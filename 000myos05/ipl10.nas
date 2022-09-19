; TAB=4
; day03

CYLS	EQU		10      ; EQU就是equal,意思就是CYLS=10,定义了一个变量,读取的柱面数

        ORG     0x7c00          ;指明程序的装载地址

; 标准FAT12格式软盘的专用代码,Stand FAT12 format floppy code

        JMP     entry
        DB      0x90
        DB      "HARIBOTE"      ; 启动扇区的名称（8字节）
        DW      512             ; 每个扇区（sector）大小（必须512字节）
        DB		1				; 簇（cluster）大小（必须为1个扇区）
        DW		1				; FAT起始位置（一般为第一个扇区）
        DB		2				; FAT个数（必须为2）
        DW		224				; 根目录大小（一般为224项）
        DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
        DB		0xf0			; 磁盘类型（必须为0xf0）
        DW		9				; FAT的长度（必??9扇区）
        DW		18				; 一个磁道（track）有几个扇区（必须为18）
        DW		2				; 磁头数（必??2）
        DD		0				; 不使用分区，必须是0
        DD		2880			; 重写一次磁盘大小
        DB		0,0,0x29		; 意义不明（固定）
        DD		0xffffffff		; （可能是）卷标号码
		DB		"HARIBOTEOS "	; 磁盘的名称（必须为11字?，不足填空格）        
		DB		"FAT12   "		; 磁盘格式名称（必??8字?，不足填空格）
        RESB	18				; 先空出18字节

; 程序主体

entry:
        MOV     AX,0            ; 初始化寄存器
        MOV     SS,AX
        MOV     SP,0x7c00
        MOV     DS,AX

; 读磁盘

        MOV     AX,0x0820
        MOV     ES,AX
        MOV     CH,0            ; 柱面0
        MOV     DH,0            ; 磁头0
        MOV     CL,2            ; 扇区2
readloop:
        MOV     SI,0            ; 记录失败次数的寄存器
retry:
        MOV     AH,0x02         ; AH=0x02:读入磁盘
        MOV     AL,1            ; 读取一个扇区
        MOV     BX,0            ; ES:BX是存放读取数据的内存地址
        MOV     DL,0x00         ; A驱动器
        INT     0x13            ; 调用磁盘BIOS
        JNC     next             ; 成功就跳转到fin
        ADD     SI,1            
        CMP     SI,5            ; 重试5次
        JAE     error           ; 失败超过5次就转到error
        MOV     AH,0x00         ; 磁盘系统复位
        MOV     DL,0x00         ; A驱动器
        INT     0x13            ; 重置驱动器
        JMP     retry

next:
        MOV     AX,ES           ;ES = 0x0820
        ADD     AX,0x0020       ; 把内存地址后移0x200(0x200 = 512o),这里是0x0020是因为ES为短地址
        MOV     ES,AX
        ADD     CL,1
        CMP     CL,18           ; 一共读 18个扇区
        JBE     readloop        ; 扇区小于等于18就继续读
        MOV     CL,1            ; 读磁盘的反面,也就是另一个磁头,重新开始读
        ADD     DH,1            ; DH代表磁头,磁头加一,就代表读的反面
        CMP     DH,1            ; 磁头小于等于1就继续读
        JBE     readloop
        MOV     DH,0
        ADD     CH,1            ; 柱面加一,接着读
        CMP     CH,CYLS         ; CYLS就是前边定义的变量,就是要读取的柱面数
        JB      readloop        ; 小于才跳转,大于等于就不跳转了

; 读完后运行haribote.sys
        MOV     [0x0ff0],CH     ;将CYLS的值写道内存地址0x0ff0中
        JMP     0xc200          ; 跳到0xc200是因为软盘



error: 
        MOV     SI,msg
putloop:
        MOV     AL,[SI]         ; AL是要显示的字符
        ADD     SI,1            ; 指向要显示的下一个字符
        CMP     AL,0            ; 如果读到的AL为0就停下,跳转到fin
        JE      fin
        MOV     AH,0x0e         ; AH=0x0e:表示要显示一个字符
        MOV     BX,15           ; 指定字符的颜色
        INT     0x10            ; 调用BIOS
        JMP     putloop
fin:
        HLT                     ; 让CPU停止,等待指令
        JMP     fin             ; 无限循环
msg:
        DB      0x0a, 0x0a      ;两个换行
        DB      "load error"
        DB      0x0a
        DB      0

        RESB    0x7dfe-$        ;用 0x00 填充到 0x7dfe 的指令

        DB      0x55, 0xaa




        