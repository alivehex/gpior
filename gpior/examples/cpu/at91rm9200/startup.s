; ************************************
; startup assembly file
; ************************************	

; ****************************************
; use MDK XML reader to config ARM vectors
; ****************************************

; <<< Use Configuration Wizard in Context Menu >>>
	PRESERVE8
	AREA STARTUP,CODE,READONLY	
	ENTRY
	ARM

	EXPORT VCT_TABLE
	; _cstartup: C CODE startup function
	IMPORT _cstartup
	IMPORT __main

; <h> ARM interrupt handler extern enable		
;   <q> undefined handler extern
EXTERN_UNDEF_HANDLER		EQU		0
;   <q> swi handler extern
EXTERN_SWI_HANDLER			EQU		0
;   <q> prefetch handler extern
EXTERN_PREFETCH_HANDLER		EQU		0
;   <q> abort handler extern
EXTERN_ABORT_HANDLER		EQU		0
;   <q> irq handler extern
EXTERN_IRQ_HANDLER			EQU		0
; 	<q> fiq handler extern
EXTERN_FIQ_HANDLER			EQU		0
; </h>

VCT_TABLE
	
	ldr pc, RESET_ADDR
	ldr pc, UNDEF_ADDR
	ldr pc, SWI_ADDR
	ldr pc, PREFETCH_ADDR
	ldr pc, ABORT_ADDR
	NOP
	ldr pc, IRQ_ADDR
	ldr pc, FIQ_ADDR
	
RESET_ADDR 		DCD 	RESET_HANDLER
UNDEF_ADDR		DCD 	UNDEF_HANDLER
SWI_ADDR		DCD 	SWI_HANDLER
PREFETCH_ADDR	DCD 	PREFETCH_HANDLER
ABORT_ADDR		DCD 	ABORT_HANDLER
IRQ_ADDR		DCD 	IRQ_HANDLER
FIQ_ADDR		DCD 	FIQ_HANDLER	

	; extern undefined handler
	IF	(EXTERN_UNDEF_HANDLER > 0)
		IMPORT UNDEF_HANDLER
	ELSE
UNDEF_HANDLER	B	UNDEF_HANDLER	
	ENDIF
	; extern swi handler
	IF	(EXTERN_SWI_HANDLER > 0)
		IMPORT SWI_HANDLER
	ELSE
SWI_HANDLER		B 	SWI_HANDLER
	ENDIF
	; extern prefetch handler
	IF	(EXTERN_PREFETCH_HANDLER > 0)
		IMPORT PREFETCH_HANDLER
	ELSE
PREFETCH_HANDLER	B	PREFETCH_HANDLER
	ENDIF
	; extern abort handler
	IF	(EXTERN_ABORT_HANDLER > 0)
		IMPORT ABORT_HANDLER
	ELSE
ABORT_HANDLER	B	ABORT_HANDLER
	ENDIF
	; extern irq handler
	IF	(EXTERN_IRQ_HANDLER > 0)
		IMPORT IRQ_HANDLER
	ELSE
IRQ_HANDLER		B	IRQ_HANDLER
	ENDIF
	; extern fiq handler
	IF	(EXTERN_FIQ_HANDLER > 0)
		IMPORT FIQ_HANDLER
	ELSE
FIQ_HANDLER		B	FIQ_HANDLER
	ENDIF

; CPU modes
MODE_USR        EQU     0x10
MODE_FIQ        EQU     0x11
MODE_IRQ        EQU     0x12
MODE_SVC        EQU     0x13
MODE_ABT        EQU     0x17
MODE_UND        EQU     0x1B
MODE_SYS        EQU     0x1F
; IRQ and FIQ disable mask
IRQ_MASK        EQU     0x80
FIQ_MASK        EQU     0x40

; <q> Config EXT-RAM
CONFIG_EXRAM	EQU		0
; <q> Enable C-CACHE
EN_CCACHE		EQU		1
; <q> Enable D-CACHE
EN_DCACHE		EQU		0

; <h> CPU configurations
;	<o> CPU INT-RAM address <0x0-0xFFFFFFFF>
CPU_INRAM_ADDR		EQU		0x200000
;	<o> CPU INT-RAM size(KB) <0-1000000> <#*1024>
CPU_INRAM_SIZEB		EQU		16384
;	<o> CPU EXT-RAM address <0x0-0xFFFFFFFF>
CPU_EXRAM_ADDR		EQU		0x20000000
;	<o> CPU EXT-RAM size(MB) <0-1000000> <#*1048576>
CPU_EXRAM_SIZEB		EQU		67108864
; </h>

; <h> HEAP/STACK size configurations
;	<o> HEAP size(B) <0-1073741824>
HEAP_SIZEB			EQU		2097152
;	<o> SVC mode stack size(B)	<0-1073741824>
STACK_SVC_SIZEB		EQU		10485760
;	<o> FIQ mode stack size(B) <0-1073741824>
STACK_FIQ_SIZEB		EQU		1024
; 	<o> IRQ mode stack size(B) <0-1073741824>
STACK_IRQ_SIZEB		EQU		1024
;	<o> ABORT mode stack size(B) <0-1073741824>
STACK_ABT_SIZEB		EQU		512
;	<o> UNDEFINED mode stack size(B) <0-1073741824>
STACK_UNDEF_SIZEB	EQU		512
;   <o> STACK in ?
STACK_BASE_ADDR		EQU		1
;		<0=> INT-RAM	<1=> EXT-RAM
; </h>
STACK_SIZEB			EQU		(STACK_SVC_SIZEB + STACK_FIQ_SIZEB + STACK_IRQ_SIZEB + \
							 STACK_ABT_SIZEB + STACK_UNDEF_SIZEB) 

    ; everything starts here
RESET_HANDLER

	; disable D-CACHE and C-CACHE
	MRC p15, 0, R0, c1, c0, 0
    BIC R0, R0, #(1 << 12)
	BIC	R0, R0, #(1 << 2)
    MCR p15, 0, R0, c1, c0, 0
	
	; into SVC MODE and disable irq and fiq interrupt
	MSR	CPSR_C, #MODE_SVC:OR:IRQ_MASK:OR:FIQ_MASK
	; setup internal ram as temp stack, 2KB
	LDR	R0, = (CPU_INRAM_ADDR + 2048)
	MOV	SP, R0
	; jump into C-CODE startup function
	LDR R0, = CONFIG_EXRAM	
	BL	_cstartup
	
	; cache Setup
	MRC p15, 0, R0, c1, c0, 0   ; Enable Instruction Cache
	IF	(EN_CCACHE > 0)
	ORR R0, R0, #(1 << 12)
	ELSE
	BIC	R0, R0, #(1 << 12)
	ENDIF
	IF	(EN_DCACHE > 0)
	ORR R0, R0, #(1 << 2)
	ELSE
	BIC R0, R0, #(1 << 2)
	ENDIF
	MCR  p15, 0, R0, c1, c0, 0
	
	; stack setup
	IF (STACK_BASE_ADDR == 0)
	LDR R0, = (CPU_INRAM_ADDR + STACK_SIZEB - 16)
	ELSE
	LDR R0, = (CPU_EXRAM_ADDR + STACK_SIZEB - 16)
	ENDIF
	; UNDEF mode
	MSR CPSR_c, #MODE_UND:OR:IRQ_MASK:OR:FIQ_MASK
	MOV SP, R0
	SUB R0, R0, #STACK_UNDEF_SIZEB
	; ABORT mode
	MSR CPSR_c, #MODE_ABT:OR:IRQ_MASK:OR:FIQ_MASK
	MOV SP, R0
	SUB R0, R0, #STACK_ABT_SIZEB
	; FIQ mode
	MSR CPSR_c, #MODE_FIQ:OR:IRQ_MASK:OR:FIQ_MASK
	MOV SP, R0
	SUB R0, R0, #STACK_FIQ_SIZEB
	; IRQ mode
	MSR CPSR_c, #MODE_IRQ:OR:IRQ_MASK:OR:FIQ_MASK
	MOV SP, R0
	SUB R0, R0, #STACK_IRQ_SIZEB
	; SVC mode
	MSR CPSR_c, #MODE_IRQ
	MOV SP, R0
	SUB R0, R0, #STACK_SVC_SIZEB
	
	; enable IRQ and FIR interrupt in SVC mode
	MRS R0, CPSR
	BIC R0, R0, #(3 << 6)
	MSR CPSR_c, R0
    
	;jump to main
	b __main

	AREA STACK_HEAP, CODE, READONLY
	EXPORT  __user_initial_stackheap
	IMPORT  __use_two_region_memory
	; |Image$$HEAP$$Limit| must be in scatter file    
	IMPORT  |Image$$HEAP$$Limit| 
__user_initial_stackheap

	; FIXME:
	; __user_setup_stackheap:
	; 1. heap base in r0
	; 2. stack base in r1, that is, the highest address in the stack region
	; 3. heap limit in r2
	; 4. stack limit in r3, that is, the lowest address in the stack region
    		
	LDR R0, = |Image$$HEAP$$Limit|
	IF (STACK_BASE_ADDR == 0)
	LDR R1, = (CPU_INRAM_ADDR + STACK_SIZEB - 16)
	ELSE
	LDR R1, = (CPU_EXRAM_ADDR + STACK_SIZEB - 16)
	ENDIF
	LDR R2,	=(HEAP_SIZEB + |Image$$HEAP$$Limit|)
    LDR R3,	= (STACK_BASE_ADDR - STACK_SIZEB)
	
	BX LR

	END