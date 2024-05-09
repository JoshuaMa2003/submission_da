/*
 * CPE301_DesignAssignment6_Task3.c
 *
 * Created: 5/4/2024 10:53:31 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL              //define clock frequency as 16 MHz for delay calculations
#include <avr/io.h>                   //include AVR device-specific IO definitions
#include <util/delay.h>               //include functions for delay loops
#include <stdio.h>                    //include standard input and output functions
#include <avr/interrupt.h>            //include interrupt handling functionality

#define ENABLE 3                      //define enable pin number for motor control
#define MTR_1 1                       //define motor 1 control pin
#define MTR_2 2                       //define motor 2 control pin
#define SW (PIND&(1<<7))              //define switch input using pin PD7
#define SHIFT_REGISTER DDRB           //define shift register direction register
#define SHIFT_PORT PORTB              //define shift register port
#define DATA (1<<PB3)                 //define data input pin for SPI (MOSI)
#define LATCH (1<<PB2)                //define latch pin for SPI (SS)
#define CLOCK (1<<PB5)                //define clock pin for SPI (SCK)

#define BAUDRATE 9600                 //define baud rate for USART
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)  //calculate USART baud rate prescaler

//function prototypes
void read_adc(void);
void adc_init(void);
void USART_init(unsigned int ubrr);
void USART_tx_string(char *data);
void init_IO(void);
void init_SPI(void);
void spi_send(unsigned char byte);

//global variables
volatile unsigned int adc_temp, revTickAvg, ticks, recTicks, recTicks2, timeTicks, timeCount;
char outs[20];
char please[5] = "1.1\n\0";
const uint8_t SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90}; //segment byte maps for numbers 0-9
const uint8_t SEGMENT_SELECT[] = {0xF1, 0xF2, 0xF4, 0xF8}; //byte maps to select digit 1-4
volatile int alternator = 0;

//setup IO for SPI and shift register
void init_IO(void){
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK); //set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK); //initialize control pins to low
}

//initialize SPI in Master mode
void init_SPI(void){
	SPCR0 = (1<<SPE) | (1<<MSTR); //enable SPI and set as Master
}

//send byte via SPI
void spi_send(unsigned char byte){
	SPDR0 = byte; //load byte to SPI data register
	while(!(SPSR0 & (1<<SPIF))); //wait until SPI transmission is complete
}

//ISR for TIMER0 overflow
ISR(TIMER0_OVF_vect) {
	timeTicks++;
	if(timeTicks > 980){
		recTicks = ticks;
		ticks = 0;
		timeCount = 0;
		timeTicks = 0;
	}
	TCNT0 = 0;
}

//ISR for external interrupt 0
ISR(INT0_vect){
	ticks++;
}

//initialize ADC
void adc_init(void){
	ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0); //select AVcc as reference, right adjust, select ADC0
	ADCSRA = (1<<ADEN)|(0<<ADSC)|(0<<ADATE)|(0<<ADIF)|(0<<ADIE)|(1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0); //enable ADC, set prescaler to 128
}

//read ADC value
void read_adc(void){
	unsigned char i = 4;
	adc_temp = 0;
	while (i--){
		ADCSRA |= (1<<ADSC); //start ADC conversion
		while(ADCSRA & (1<<ADSC)); //wait for conversion to complete
		adc_temp += ADC; //accumulate converted result
		_delay_ms(50);
	}
	adc_temp = adc_temp / 4; //average over four samples
	adc_temp = adc_temp * 50 / 1024; //convert to voltage
}

//initialize timer0 for periodic interrupts
void InitTimer1(void) {
	TCCR0A = 0;
	TCNT0 = 255; //preload timer counter
	TIMSK0 = (1 << TOIE0); //enable overflow interrupt
	TCCR0B |= (1 << CS01) | (1 << CS00); //set prescaler to 64
	sei(); //enable global interrupts
}

//setup external interrupts
void InitExtInter(void){
	DDRD &= ~(1 << 2); //set PD2 as input
	PORTD |= (1 << 1) | (1<<2); //enable pull-up resistors
	EICRA = (1<<2); //trigger INT0 on falling edge
	EIMSK = (1<<INT0); //enable external interrupt 0
	sei(); //enable global interrupts
}

//initialize USART for serial communication
void USART_init(unsigned int ubrr){
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0); //enable transmitter
	UCSR0C = (3 << UCSZ00); //set frame format: 8 data bits, no parity, 1 stop bit
}

//transmit string via USART
void USART_tx_string(char *data){
	while ((*data != '\0')){
		while (!(UCSR0A & (1 <<UDRE0))); //wait for empty transmit buffer
		UDR0 = *data; //send character
		data++;
	}
}

int main(void){
	InitExtInter();
	InitTimer1();
	adc_init(); //initialize ADC
	init_IO();
	init_SPI();
	DDRB |= 0b00001110; //set PB3, PB1, PB2 as outputs for motor control
	DDRD |= (1<<ENABLE) | (1<<5) | (1<<6); //set pins as outputs
	OCR2A = 50; //set PWM duty cycle
	TCCR2A = (1<<COM2B1)|(1<<WGM21)|(1<<WGM20); //configure Timer2 for fast PWM
	TCCR2B = 0x02; //set prescaler to 8

	while(1){
		read_adc();
		PORTD |= (1<<ENABLE); //enable motor
		PORTD |= (1<<5); //set MTR_1 high
		PORTD &= ~(1<<6); //set MTR_2 low

		OCR2B = adc_temp * 15; //adjust PWM based on ADC reading

		// Display handling in the SPI block
		if(alternator == 1){
			recTicks2 = recTicks;
			SHIFT_PORT &= ~LATCH; //pull LATCH low to start SPI transfer
			if(recTicks2/24 >= 10){
				spi_send((unsigned char)SEGMENT_MAP[1] - 0x80); //display digit on the 7-segment
				}else{
				spi_send((unsigned char)SEGMENT_MAP[0] - 0x80);
			}
			spi_send((unsigned char)0xF2); //select digit
			SHIFT_PORT |= LATCH; //toggle latch to copy data to storage register
			SHIFT_PORT &= ~LATCH;
			alternator = 0;
			}else{
			spi_send((unsigned char)SEGMENT_MAP[(recTicks2/24) % 10]);
			spi_send((unsigned char)0xF4);
			SHIFT_PORT |= LATCH;
			SHIFT_PORT &= ~LATCH;
			alternator = 1;
		}
	}
}


