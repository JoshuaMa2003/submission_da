/*
 * CPE301_DesignAssignment3_Task1.c
 *
 * Created: 3/21/2024 7:26:10 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL								//define cpu clock speed at 16Mhz
#include <avr/io.h>

int main(void){
	DDRB |= (1 << 5);									//set PB5 as an output
	
	TCCR0B |= (1 << CS01) | (1 << CS00);				//initialize timer 0 with prescaler 64
	
	while(1) {											//program main loop
		
		unsigned long overflow = 0;						//overflow counter that is used for timing
		
		PORTB |= (1 << 5);								//turn on PB5 LED
		
		while(overflow < 977) {							//loop until overflow counter reaches 977 (approx. 1 second)
			
			if(TIFR0 & (1 << TOV0)){					//check if timer 0 has overflowed
				
				TIFR0 |= (1 << TOV0);					//clear timer 0 overflow flag
				
				overflow++;								//increment overflow counter
			}
		}
		
		PORTB &= ~(1 << 5);								//turn off PB5 LED
		
		overflow = 0;									//reset the overflow counter
		
		while(overflow < 977){							//timing loop repeated for LED off 
			
			if(TIFR0 & (1 << TOV0)){					//check if timer 0 has overflowed
				
			TIFR0 |= (1 << TOV0);					//clear timer 0 overflow flag
				
				overflow++;								//increment overflow counter
			}
		}
	}
	return 0;
}


