/*
 * CPE301_DesignAssignment2_Task2_C.c
 *
 * Created: 3/9/2024 11:48:17 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL			//cpu clock speed at 16Mhz
#include <avr/io.h>
#include <util/delay.h>

int main(void){
	DDRB |= (1 << 5);				//configures PB5 as an output
	
	PORTB |= (1 << 5);				//sets PB5 to high
	
	DDRC &= ~(1 << 2);				//configure PC2 as an output
	
	PORTC |= (1 << 2);				//enable pull-resistor for PC2
	
	while(1){
		
		//check if PC2 is pressed
		//if PC2 is pressed, condition is true
		if(!(PINC & (1 << 2))){
			
			PORTB &= ~(1 << 5);		//set PB5 to low, turning LED off
			
			_delay_ms(2000);		//delay for 2 seconds
		}
		else
		PORTB |= (1 << 5);		//if the button is not pressed, set PB5 high
	}
	return 0;
}

