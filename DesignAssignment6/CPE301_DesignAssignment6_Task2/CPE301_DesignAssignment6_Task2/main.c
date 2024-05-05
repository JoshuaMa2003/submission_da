/*
 * CPE301_DesignAssignment6_Task2.c
 *
 * Created: 5/4/2024 10:46:33 PM
 * Author : joshu
 */ 

#define F_CPU 16000000UL              //define the clock frequency as 16 mhz
#include <avr/io.h>                   //include the input/output library for avr
#include <util/delay.h>               //include the delay functions
#include <stdio.h>                    //include the standard input/output library
#include <avr/interrupt.h>            //include the interrupt library

#define ENABLE 3                      //define enable pin as pb3
#define MTR_1 1                       //define motor 1 pin as pb1
#define MTR_2 2                       //define motor 2 pin as pb2
#define SW (PIND & (1<<7))            //read switch state from pd7

#define BAUDRATE 9600                 //define default baud rate for serial communication
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)  //calculate prescaler for uart

//function declarations
void read_adc(void);
void adc_init(void);
void USART_init(unsigned int ubrr);
void USART_tx_string(char *data);
void InitTimer1(void);
void InitExtInter(void);
void StartTimer1(void);

//global variables for adc, timer counts, etc.
volatile unsigned int adc_temp, revTickAvg, revCtr, ticks, timeTicks, timeCount;
volatile uint32_t tickv, T1Ovs2;
char outs[20];
char please[5] = "1.1\n\0";

//timer 0 overflow interrupt service routine
ISR(TIMER0_OVF_vect) {
	timeTicks++;
	if (timeTicks > 980) {
		snprintf(outs, sizeof(outs), "%.2d\n", ticks / 24);  //format ticks to string
		please[0] = outs[0];
		please[2] = outs[1];
		USART_tx_string("RPS: ");    //send rpm data via uart
		USART_tx_string(please);
		ticks = 0;
		timeCount = 0;
		timeTicks = 0;
	}
	TCNT0 = 0;  //reset timer counter
}

//external interrupt 1 service routine for counting motor revolutions
ISR(INT1_vect) {
	ticks++;
}

//adc initialization
void adc_init(void) {
	ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0);
	ADCSRA = (1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0);
}

//read adc value
void read_adc(void) {
	unsigned char i = 4;
	adc_temp = 0;
	while (i--) {
		ADCSRA |= (1<<ADSC);
		while (ADCSRA & (1<<ADSC));
		adc_temp += ADC;
		_delay_ms(50);
	}
	adc_temp = adc_temp / 4;  //average four samples
	adc_temp = adc_temp * 50 / 1024; //convert to voltage
}

//timer 1 initialization
void InitTimer1(void) {
	TCCR0A = 0;
	TCNT0 = 255;
	TIMSK0 = (1 << TOIE0);  //enable timer0 overflow interrupt
	TCCR0B |= (1 << CS01) | (1 << CS00);  //set prescaler to 64
	sei();  //enable global interrupts
}

//external interrupt setup
void InitExtInter(void) {
	DDRD &= ~(1 << 2);  //set pd2 as input
	PORTD |= (1 << 1) | (1 << 2);  //enable pull-ups
	EICRA = (1 << 2);  //set int1 to trigger on falling edge
	EIMSK = (1 << INT1);  //enable int1
	sei();  //enable global interrupts
}

//usart initialization
void USART_init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0);  //enable transmitter
	UCSR0C = (3 << UCSZ00);  //set frame format: 8 data bits, no parity, 1 stop bit
}

//send string over usart
void USART_tx_string(char *data) {
	while (*data != '\0') {
		while (!(UCSR0A & (1 << UDRE0)));  //wait for empty transmit buffer
		UDR0 = *data++;  //put data into buffer, sends the data
	}
}

int main(void) {
	InitExtInter();
	InitTimer1();
	adc_init();                      //initialize adc
	USART_init(BAUD_PRESCALLER);     //initialize usart
	USART_tx_string("Connected!\r\n");  //send connection message
	_delay_ms(125);                  //wait a bit

	PORTD |= (1 << 7);               //enable pull-up on pd7
	DDRB |= 0b00001110;              //set pb3, pb1, pb2 as outputs
	PORTB &= ~(1 << ENABLE);         //disable motor initially
	PORTB &= ~(1 << MTR_1);          //motor 1 off
	PORTB &= ~(1 << MTR_2);          //motor 2 off
	DDRB |= (1 << 3);                //set oc2a as output
	OCR2A = 50;                      //set output compare for pwm
	TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);  //configure for fast pwm
	TCCR2B = 0x02;                   //set prescaler to 8

	while (1) {
		read_adc();  //read adc value continuously
		PORTD |= (1 << ENABLE);  //enable motor
		if (SW != 0) {  //if switch is on
			_delay_ms(20);
			PORTB |= (1 << MTR_1);  //motor 1 on for clockwise rotation
			PORTB &= ~(1 << MTR_2);  //motor 2 off
			} else {
			_delay_ms(20);
			PORTB &= ~(1 << MTR_1);  //motor 1 off
			PORTB |= (1 << MTR_2);  //motor 2 on for counter-clockwise rotation
		}
		OCR2A = adc_temp * 15;  //adjust pwm based on adc result
	}
}


