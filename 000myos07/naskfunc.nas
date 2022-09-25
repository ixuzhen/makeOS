; tab 4
[FORMAT "WCOFF"]            	; 制作目标文件的模式
[INSTRSET "i486p"]				; 不指定这个nask就认为,该汇编是8086模式,只有16位,他认识EAX
[BITS 32]                   	; 制作32位模式用的机械语言

; 制作目标文件的信息
[FILE "naskfunc.nas"]       	; 源文件名信息
        GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		EXTERN	_inthandler21, _inthandler27, _inthandler2c

; 实际的函数
[SECTION .text]             	; 目标文件中写了这些之后再写程序
_io_hlt:                    	; 相当于 void io_hlt(void);
        HLT
        RET

_io_cli:		; void io_cli(void)		# 禁止中断
	CLI	
	RET

_io_sti:		; void io_sti(void)		# 允许中断
	STI
	RET

_io_stihlt:		; void io_in8(void)
	STI
	HLT
	RET

_io_in8:		; int io_in8(int port);
	MOV		EDX,[ESP+4]
	MOV		EAX,0
	IN		AL,DX
	RET

_io_in16:		; int io_in16(int port);
	MOV		EDX,[ESP+4]
	MOV		EAX,0
	IN		AX,DX
	RET


_io_in32:		; int io_in32(int port);
	MOV		EDX,[ESP+4]
	MOV		EAX,0
	IN		EAX,DX
	RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET


_io_load_eflags:	;int io_load_eflags(void);
		PUSHFD			; PUSH EFLAGS
		POP		EAX
		RET

_io_store_eflags:	;void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD			; POP EFLAGS
		RET

_load_gdtr:			; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]
		MOV 	[ESP+6],AX
		LGDT 	[ESP+6]
		RET

_load_idtr:			; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]
		MOV 	[ESP+6],AX
		LIDT 	[ESP+6]
		RET

_asm_inthandler21:
		PUSH 	ES
		PUSH 	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH 	EAX
		MOV		AX,SS		; 关于在DS和ES中放入SS值的部分，因为C语言自以为是地认为“DS也好，ES也好，SS也好，
		MOV		DS,AX		; 它们都是指同一个段”，所以如果不按照它的想法设定的话，函数
		MOV 	ES,AX		; inthandler21就不能顺利执行。所以，虽然麻烦了一点，但还是要这样做。
		CALL 	_inthandler21
		POP		EAX
		POPAD
		POP 	DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH 	ES
		PUSH 	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH 	EAX
		MOV		AX,SS		; 关于在DS和ES中放入SS值的部分，因为C语言自以为是地认为“DS也好，ES也好，SS也好，
		MOV		DS,AX		; 它们都是指同一个段”，所以如果不按照它的想法设定的话，函数
		MOV 	ES,AX		; inthandler21就不能顺利执行。所以，虽然麻烦了一点，但还是要这样做。
		CALL 	_inthandler27
		POP		EAX
		POPAD
		POP 	DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH 	ES
		PUSH 	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH 	EAX
		MOV		AX,SS		; 关于在DS和ES中放入SS值的部分，因为C语言自以为是地认为“DS也好，ES也好，SS也好，
		MOV		DS,AX		; 它们都是指同一个段”，所以如果不按照它的想法设定的话，函数
		MOV 	ES,AX		; inthandler21就不能顺利执行。所以，虽然麻烦了一点，但还是要这样做。
		CALL 	_inthandler2c
		POP		EAX
		POPAD
		POP 	DS
		POP		ES
		IRETD