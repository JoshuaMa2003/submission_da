/*
 * CPE301_DesignAssignment3_Task3.c
 *
 * Created: 3/21/2024 10:24:20 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL				//define cpu clock speed at 16Mhz
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned int overflow = 0;			//volatile variable to track number of overflows

int main(void){
	DDRB |= (1 << 3) | (1 << 5);			//setting PB3 and PB5 as outputs
	
	PORTB ^= (1 << 5);						//initialize PB5 LED off
	
	TCCR2B |= (1 << CS22);					//set prescaler for timer 2 to 64
	
	TIMSK2 |= (1 << TOIE2);					//enable timer 2 overflow interrupt
	
	sei();									//enable global interrupts
	
	while(1){								//loop does nothing, waits for interrupt
		
	}
	return 0;
}

ISR(TIMER2_OVF_vect){
	
	overflow++;								//increment overflow counter each time timer 2 overflows
	
	if(overflow >= 1953){					//check if overflow reaches 1953 (approx. 2 seconds)
		
		PORTB ^= (1 << 3);					//toggle the LED at PB3
		
		overflow = 0;						//resetting overflow counter
	}
}
