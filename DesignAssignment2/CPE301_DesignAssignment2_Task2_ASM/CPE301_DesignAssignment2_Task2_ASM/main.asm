;
; CPE301_DesignAssignment2_Task2_ASM.asm
;
; Created: 3/9/2024 11:37:48 PM
; Author : joshu
;

#define F_CPU 160000000UL
.include "m328pbdef.inc"

; Initialize
.org 0x00

CBI DDRC, 2					//set PC2 as an output
SBI DDRB, 5					//set PB5 as an input

BEGIN:
		LDI R16, 6			//loading loop counter for delay
		SBI PORTB, 5		//turn on the led at PB5
LOOP:	
		SBIC PINC, 2		//wait for the button/switch on PC2
		RJMP LOOP			//stay in loop if button/switch not activated
		CBI PORTB, 5		//turn off the led at PB5

		C_DELAY:
		CALL DELAY			//call delay subroutine
		DEC R16				//decrement the loop counter
		BRNE C_DELAY		//repeat if it is not zero
		SBI PORTB, 5		//turn on the led at PB5 again


DELAY:
	LDI R18, 28				//delay loop: outer counter
	LDI R19, 8				//delay loop: middle counter
	LDI R20, 103			//delay loop: inner counter
L1: 
	DEC R20					//decrement the inner counter
	BRNE L1					//continue inner counter if not zero
	DEC R19					//decrement middle counter
	BRNE L1					//continue middle loop if not zero
	DEC R18					//decrement outer counter
	BRNE L1					//continue outer loop if not zero
	NOP						//does nothing, only ensures delay timing

RET
