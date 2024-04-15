/*
 * CPE301_DesignAssignment4_C.c
 *
 * Created: 4/9/2024 8:35:41 AM
 * Author : joshu
 */ 


#define F_CPU 16000000UL	//define cpu clock frequency       
#define BAUDRATE 9600		//define baud rate for usart 
#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1)	//define baud rate prescaler value

//include needed avr headers
#include <avr/io.h>                   
#include <stdlib.h>                    
#include <string.h>                    
#include <util/delay.h>                
#include <avr/interrupt.h>            

char out_str[51];		//define buffer for output string             

int adc_init(void)
{
	ADMUX |= (1 << REFS0);				//refernce voltage for adc
	ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	//adc configuration
	ADCSRB = (1 << ADTS2) | (1 << ADTS1);	//adc trigger source
	
	while(ADCSRA & (1 << ADSC));	//wait for adc
	return ADC;						//return adc value
}

void timer_init(void)
{
	TCNT1 = 65380;			//preload timer1 for delay adjustments 
	TCCR1B |= (1 << CS11);	//set prescaler to 8
	TIMSK1 = (1 << TOIE1);	//enable timer1 overflow interrupt
	sei();					//enable global interrupt
}

void usart_init(void)
{
	UBRR0H = (uint8_t)(BAUD_PRESCALE >> 8);	// set baud rate high
	UBRR0L = (uint8_t)(BAUD_PRESCALE);		//set baud rate low
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);	//enable reciever and transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);	//set frame format
	
}

int main(void)
{
	memset(out_str, ' ', 49);	//initialize out_str with spaces
	out_str[50] = '\0';			//null terminate string
	
	timer_init();				//timer initialization
	usart_init();				//usart initialization
	adc_init();					//adc initialization
	
	while(1)
	{
		//infinite loop
	}
	return 0;
}

void usart_send(unsigned char ch)
{
	while(!(UCSR0A & (1 << UDRE0)));	//wait for data register to be empty
	UDR0 = ch;		//send character
}

//print a string over usart
void usart_print(char* str)
{
	int i = 0;
	while(str[i] != '\0')
	{
		usart_send(str[i]);
		i++;
	}
}

//send string over to usart
void usart_putstring(char* StringPtr)
{
	while((*StringPtr) != '\0')
	{
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = *StringPtr;
		StringPtr++;
	}
}

ISR(TIMER1_OVF_vect)
{
	ADCSRA |= (1 << ADSC);					//start adc conversion
	while((ADCSRA & (1 << ADIF)) == 0);		//wait for conversion to complete
	ADCSRA |= (1 << ADIF);					//clear adc interrupt flag
	
	int adcVal = ADCL;						//read low byte
	adcVal = adcVal | (ADCH << 8);			//read high byte, combine with low byte
	int8_t index = adcVal/20.48;			//convert adc value to index
	out_str[index] = '*';					//mark current position in buffer
	usart_putstring(out_str);				//send buffer over to usart
	usart_putstring("\n");					//send newline to usart
	out_str[index] = ' ';					//clear marked position in buffer
	TCNT1 = 65380;							//reload timer
}
