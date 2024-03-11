/*
 * CPE301_DesignAssignment2_Task3_C.c
 *
 * Created: 3/9/2024 11:52:12 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL // Corrected CPU clock speed to 16MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main(){
	DDRB |= (1 << 4) | (1 << 5);		//set PB4 and PB5 as output pins
	DDRD &= ~(1 << 3);					//set PD3 as an input pin
	PORTD |= 1 << 3;					//activate pull-up resistor on PD3
	
	EICRA = 0x2;						//configure INT1 to trigger on falling edge
	PORTB ^= (1 << 4) | (1 << 5);		//toggle state of PB4 and PB5
	
	EIMSK |= (1 << INT1);				//enable external interrupt INT1
	sei();								//enable global interrupts
	
	while(1){							//infinite loop
	}
}

ISR(INT1_vect){
	PORTB ^= (1 << 4);					//toggle PB4 on interrupt
	
	_delay_ms(3000);					//delay for 3000ms or 3s
	
	PORTB ^= (1 << 4);					//toggle PB4 once again after delay
}

