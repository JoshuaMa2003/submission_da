;
; CPE301_DesignAssignment2_Task3_ASM.asm
;
; Created: 3/9/2024 11:57:08 PM
; Author : joshu
;

#define F_CPU 16000000UL
.include "m328pbdef.inc"

.org 0x00												//begin at address 0x00
    JMP BEGIN											//jump to begginning of program
.org 0x04												//program counter for INT1
    JMP ISR_INT1										//jump to ISR for INT1

BEGIN:
    LDI R16, (1 << 4)									//load immediate value to set PB4 as output
	OUT DDRB, R16										//set DDRB to make PB4 an output
   

    LDI R17, (1 << 3)									//load immediate value to enable pull-up on PD3
    OUT PORTD, R17										//enable pull-up for PD3
    LDI R18, (1 << ISC11) | (1 << ISC10)				//INT1 to trigger on rising edge
    STS EICRA, R18										//external interrupt control register A

    LDI R19, (1 << INT1)								//enable INT1
    STS EIMSK, R19										//external interrupt mask register
	
    SEI													//enable global interrupts

LOOP:
    IN R20, PIND										//read state of PORTD into R20
    ANDI R20, (1 << 3)									//mask all but 3 bits
    CPI R20, (1 << 3)									//compare masked value with expected bit
    BRNE ISR_INT1										//not equal, jump to ISR
    LDI R21, (1 << 4)									//load value to set PB4 high
    OUT PORTB, R21										//set PB4 high
    RJMP LOOP											//jump to loop

ISR_INT1:
	IN R22, PORTB										//read current state of PORTB into R22
	ORI R22, ~(1 << 4)									//clear bit 4 of PORTB
	ANDI R22, (1 << 5)									//manipulate R22
	OUT PORTB, R22										//output value to PORTb
	CBI PORTB, 5										//clear bit 5 of PORTB
	RCALL DELAY											//delay function
	RETI


DELAY:
    LDI  R18, 255										//outer loop
    LDI  R19, 255										//middle loop
    LDI  R20, 23										//inner loop
L1: DEC  R20											//decrement R20
    BRNE L1												//if R20, not zero, loop back
    DEC  R19											//decrement R19
    BRNE L1												//if R19, not zero, loop back
    DEC  R18											//decrement R18
    BRNE L1												//if R18, not zero, loop back

	