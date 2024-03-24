/*
 * CPE301_DesignAssignment3_Task2.c
 *
 * Created: 3/21/2024 7:53:33 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL							//define clock speed at 16Mhz
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned long timer = 0;					//volatile variable to count the number of interrupts


int main(void)
{
	DDRB |= (1 << 4) | (1 << 5);					//set PB4 and PB5 as output pins
	
	PORTB ^= (1 << 5);								//initialize LED at PB5 off
	
	TCCR1B |= (1 << WGM12);							//Configure timer 1 for CTC mode
	
	OCR1A = 124;									//setting Output Compare Register for Timer 1 channel A to 124
													//Will cause timer to reset every 0.5 ms ((124 + 1) * prescaler / 16Mhz)
													//prescaler in this case would be set to 64
	
	TIMSK1 |= (1 << OCIE1A);						//enable Timer 1 Output Compare A Match Interrupt
	
	TCCR1B |= (1 << CS11) | (1 << CS10);			//setting prescaler to 64
	
	sei();											//enable global interrupts
	
	while(1){										//loop does nothing, waits for interrupts
		
	}
	return 0;
}

ISR(TIMER1_COMPA_vect){
	timer++;										//increment timer variable every time interrupts occurs
	
	if(timer >= 6000){								//check if the timer has reached 6000 (approx. 3 seconds)
		
		PORTB ^= (1 << 4);							//toggling the LED at PB4
		
		timer = 0;									//resetting the timer variable
	}
}


