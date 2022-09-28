
; BOOT_INFO

BOTPAK	EQU		0x00280000		; 加载bootpack
DSKCAC	EQU		0x00100000		; 磁盘缓存的位置
DSKCAC0	EQU		0x00008000		; 磁盘缓存的位置（实模式）

CYLS    EQU     0x0ff0          ; 设定启动区
LEDS    EQU     0x0ff1
VMODE   EQU     0x0ff2          ; 关于颜色的数目信息.颜色的位数
SCRNX   EQU     0x0ff4          ; 分辨率的x(screen x)
SCRNY   EQU     0x0ff6          ; 分辨率的y(screen y)
VRAM    EQU     0x0ff8          ; 图像缓冲区的开始地址

        ORG     0xc200          ; 将程序装载到内存的那个地方,存到0xc200主要是因为,它原本就在0xc200
        ; ORG 0x8400            ; 存在其他地方也没事,目前没事,因为其他地方的内存还没开始使用
        MOV AL,0x13             ; VGA显卡,320x200x8位彩色
        MOV AH,0x00
        INT 0x10
        MOV     BYTE [VMODE],8
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000
        ; 用BIOS取得键盘上各种LED指示灯的状态
        MOV     AH,0x02
        INT     0x16
        MOV     [LEDS],AL

; 防止PIC接受所有中断
;	AT兼容机的规范、PIC初始化
;	然后之前在CLI不做任何事就挂起
;	PIC在同意后初始化

		MOV		AL,0xff
		OUT		0x21,AL			; 禁止主PIC的全部中断
		NOP						; 不断执行OUT指令,如果连续执行OUT指令,有些机种会无法正常运行
		OUT		0xa1,AL			; 禁止从PIC的全部中断

		CLI						; 进一步中断CPU, 禁止CPU级别的中断

; 让CPU支持1M以上内存、设置A20GATE

		CALL	waitkbdout		; 等待设备准备好
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 保护模式转换

[INSTRSET "i486p"]				; 说明使用486指令

		LGDT	[GDTR0]			; 设置临时GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 使用bit31（禁用分页）
		OR		EAX,0x00000001	; bit0到1转换（保护模式过渡）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  写32bit的段
		MOV		DS,AX			; 除了cs的段都要更新
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack传递

		MOV		ESI,bootpack	; ESI源,传送源
		MOV		EDI,BOTPAK		; EDI目标,传送目的地
		MOV		ECX,512*1024/4	; ECX是传送的数据量
		CALL	memcpy

; 传输磁盘数据,磁盘数据最终传送到它本来的地方去

; 从引导区开始

		MOV		ESI,0x7c00		; 源,传送源
		MOV		EDI,DSKCAC		; 目标,传送目的地
		MOV		ECX,512/4
		CALL	memcpy

; 剩余的全部

		MOV		ESI,DSKCAC0+512	; 源,传送源
		MOV		EDI,DSKCAC+512	; 目标,传送目的地
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 除以4得到字节数
		SUB		ECX,512/4		; IPL偏移量
		CALL	memcpy

; 由于还需要asmhead才能完成
; 完成其余的bootpack任务

; 必须由asmhead来完成的工作,至今全部完成
; 以后就交给bootpack来完成

; bootpack启动

; 和作者制作的bim2hrb有关
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 传输完成
		MOV		ESI,[EBX+20]	; 源
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 目标
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 堆栈的初始化
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		; IN		 AL,0x60		; 空读(为了清空数据接收缓冲区的垃圾数据)
		JNZ		waitkbdout		; AND结果不为0跳转到waitkbdout
		RET

; ESI 是源地址,EDI是目的地址,ECX是要传送的数据大小
memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 运算结果不为0跳转到memcpy
		RET
; memcpy地址前缀大小

		ALIGNB	16
GDT0:
		RESB	8				; 初始值
		DW		0xffff,0x0000,0x9200,0x00cf	; 写32bit位段寄存器
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行的文件的32bit寄存器（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: