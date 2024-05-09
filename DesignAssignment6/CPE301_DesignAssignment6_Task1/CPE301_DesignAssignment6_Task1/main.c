#define F_CPU 16000000UL              //define the clock frequency as 16 mhz
#include <avr/io.h>                   //include the input/output library for avr
#define ENABLE 3                      //define enable pin as pb3
#define MTR_1 1                       //define motor 1 pin as pb1
#define MTR_2 2                       //define motor 2 pin as pb2
#define SW (PIND & (1<<7))            //read switch state from pd7
#include <util/delay.h>               //include the delay functions
#include <stdio.h>                    //include the standard input/output library
#include <avr/interrupt.h>            //include the interrupt library

//function declarations
void read_adc(void);
void adc_init(void);

volatile unsigned int adc_temp;      //variable to store adc result

//initialize adc settings
void adc_init(void)
{
	ADMUX = (0<<REFS1)|               //use avcc as reference voltage
	(1<<REFS0)|
	(0<<ADLAR)|               //right adjust adc result
	(0<<MUX2)|                //use adc0 (pc0, pin23)
	(0<<MUX1)|
	(0<<MUX0);
	ADCSRA = (1<<ADEN)|               //enable adc
	(0<<ADSC)|               //do not start conversion
	(0<<ADATE)|              //disable auto trigger
	(0<<ADIF)|               //clear adc interrupt flag
	(0<<ADIE)|               //disable adc interrupt
	(1<<ADPS2)|              //set adc prescaler to 128
	(0<<ADPS1)|
	(1<<ADPS0);
}

//read adc value
void read_adc(void)
{
	unsigned char i = 4;
	adc_temp = 0;
	while (i--)
	{
		ADCSRA |= (1<<ADSC);          //start conversion
		while (ADCSRA & (1<<ADSC));   //wait for conversion to finish
		adc_temp += ADC;              //sum up results
		_delay_ms(50);                //wait 50 ms
	}
	adc_temp = adc_temp / 4;          //average four samples
	adc_temp = adc_temp * 50 / 1024;  //convert to voltage scale
}

int main(void)
{
	adc_init();                       //initialize adc
	PORTD |= (1<<7);                  //enable pull-up resistor on pd7
	DDRB |= 0b00001110;               //set pb3, pb1, pb2 as outputs
	PORTB &= ~(1<<ENABLE);            //disable motor initially
	PORTB &= ~(1<<MTR_1);             //turn off mtr_1
	PORTB &= ~(1<<MTR_2);             //turn off mtr_2
	DDRB |= (1<<3);                   //set oc2a as output
	OCR2A = 50;                       //set output compare for pwm
	
	//configure timer 2 for fast pwm
	TCCR2A = (1<<COM2A1)|(1<<WGM21)|(1<<WGM20);
	TCCR2B = 0x02;                    //set prescaler to 8

	while (1)
	{
		read_adc();                   //read adc value
		PORTD |= (1<<ENABLE);         //enable motor
		if(SW != 0)                   //if switch is on
		{
			_delay_ms(20);            //delay for 20ms
			PORTB |= (1<<MTR_1);      //drive mtr_1 high for clockwise rotation
			PORTB &= ~(1<<MTR_2);     //keep mtr_2 low
		}
		else
		{
			_delay_ms(20);            //delay for 20ms
			PORTB &= ~(1<<MTR_1);     //drive mtr_1 low
			PORTB |= (1<<MTR_2);      //drive mtr_2 high for counter-clockwise rotation
		}
		OCR2A = adc_temp * 15;       //adjust pwm based on adc result
	}
}
