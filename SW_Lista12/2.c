#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pid.c"
#include "pid.h"

//Michał Łukasik 330623

// #define 復用器 ADMUX
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


volatile int16_t v = 0;

struct GLOBAL_FLAGS {
    //! True when PID control loop should run one time
    uint8_t pidTimer:1;
    uint8_t dummy:7;
} gFlags = {0, 0};

volatile struct PID_DATA pidData;

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}


void adc_init()
{
  ADMUX   = _BV(REFS0);// referencja 1,1V, wejście ADC0
  ADMUX  |= _BV(REFS1);
  //ADMUX   |= 0b0000; // ???
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
  //ADCSRA |= _BV(ADATE);
}

uint16_t readVoltage(void) {
	_delay_ms(1);
	ADCSRA |= _BV(ADSC);
	while( ADCSRA & _BV( ADSC) ) ;
	uint8_t low  = ADCL;
	uint8_t high = ADCH;
	uint16_t adc = (high << 8) | low;		// 0<= result <=1023
	//uint8_t vccx10 = (uint8_t) ( (11 * 1024) / adc);
	return adc;
}

FILE uart_file;

//-------------------------------------------------------------------------------------------------------------------------------------------------------

#define K_P     1.200
#define K_I     0.300
#define K_D     0.120

void timer1_init()
{
    TCCR1A = _BV(COM1A1) | _BV(WGM11);
    TCCR1B = _BV(CS10) | _BV(WGM12) | _BV(WGM13);
    ICR1 = 32767;
    OCR1A = 0;
}

void timer0_init()
{
    pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);
    TCCR0B = _BV(CS00);
    TIMSK0 = _BV(TOIE0);
    TCNT0 = 0;
}

volatile uint16_t target = 155;
volatile uint16_t pain = 1023;

int16_t Get_Reference()
{
    return target;
}

int16_t Get_Measurement()
{
    return pain;
}

void Set_Input(int16_t inputValue)
{
    if (inputValue < 0) OCR1A = 0;
    else if (inputValue > 32767) OCR1A = 32767;
    else OCR1A = inputValue;
    v = inputValue;
}

void timer2_init()
{
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);
    TIMSK2 = _BV(OCIE2A);
}

ISR(TIMER2_COMPA_vect)
{
    static uint16_t i = 0;
    if(i < 20000)
    {
        i++;
        return;
    }
    else
    {
        pain = readVoltage();
        printf("\r%6u %7u %7u %7d",pain, target, OCR1A, v);
        i = 0;
    }
}

#define TIME_INTERVAL   457
ISR(TIMER0_OVF_vect)
{
    static uint16_t i = 0;
    if(i < TIME_INTERVAL)
        i++;
    else{
        gFlags.pidTimer = TRUE;

        target = Get_Reference();

        int16_t inputValue = pid_Controller(target << 6, pain << 6, &pidData);

        Set_Input(inputValue);

        gFlags.pidTimer = FALSE;
        i=0;
    }
}

int main()
{
    timer0_init();
    timer1_init();
    timer2_init();
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("odczyt     cel     PWM     PID\r\n");
    adc_init();
    sei();
    DDRB |= _BV(PB1);
    while(1) {
        if(scanf("%d",&target))
        {
            pid_Reset_Integrator(&pidData);
        }
    }
}
