;************************************************
;		Delay microseconds
;************************************************


	PRESERVE8
	AREA DELAY,CODE,READONLY	
	ARM
	
	EXPORT GPIOR_DELAYUS
	; extern void GPIOR_DELAYUS(int)

GPIOR_DELAYUS		; CPU Cycles
	LDR R1, = 0		;	3	
_WAIT_
	SUB R0, R0, #1	;	1		
	NOP				; 	1
	NOP
	NOP
	NOP
	NOP

	NOP
	NOP
	NOP
	NOP
	NOP

	NOP
	NOP
	NOP
	NOP
	NOP

	NOP
	NOP
	NOP
	NOP
	NOP

	NOP
	NOP
	NOP
	NOP
	NOP
	CMP R0, R1	    ;	1
	BNE _WAIT_		;	3 / 1
	BX LR

	END