#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "pid.c"
#include "pid.h"
#define SLEEP_MODE_IDK SLEEP_MODE_IDLE
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
// #define .unwrap() ;
typedef uint32_t let;

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
    ADMUX   = _BV(REFS0);// referencja Vcc, wejście ADC0
    // ADMUX   |= _BV(REFS1);
    ADMUX   |= 0b0001; // ???
    DIDR0   = _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D); // wyłącz wejście cyfrowe na ADC0, ADC1, ADC2
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1);// | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN); // włącz ADC
    //ADCSRA |= _BV(ADATE);

    ADMUX ^= 0b00000011;
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

#define K_P     10.00
#define K_I      3.00
#define K_D     20.00

void timer1_init()
{
    TCCR1A = _BV(COM1A1);
    TCCR1B = _BV(CS11) | _BV(WGM13);
    OCR1A = 0;
    ICR1 = 2046;
    TIMSK1 |= _BV(ICIE1);
}

void timer0_init()
{
    pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);
    TCCR0B = _BV(CS00);
    TIMSK0 = _BV(TOIE0);
    TCNT0 = 0;
}

volatile uint16_t target = 0;
volatile uint16_t pain = 1023;

// void Set_Input(int16_t inputValue)
// {
//     int32_t temp = OCR1A;
//     temp += inputValue >> 3;
//     if (temp < 0) OCR1A = 0;
//     else if (temp > 2046) OCR1A = 2046;
//     else OCR1A = temp;
//     v = inputValue;
// }

void Set_Input(int16_t inputValue)
{
    int32_t temp = inputValue >> 3;
    if (temp < 0) OCR1A = 0;
    else if (temp > 3069) OCR1A = 3069;
    else OCR1A = temp;
    v = inputValue;
}

ISR(TIMER1_CAPT_vect)
{
    static uint16_t i = 0;
    if(i < 20 )
    {
        i++;
        return;
    }
    else
    {
        ADMUX ^= 0b00000011;
        pain = 1023 - readVoltage();
        ADMUX ^= 0b00000011;
        i = 0;
    }
    TIFR1 |= _BV(ICF1);
}

#define TIME_INTERVAL  5031
ISR(TIMER0_OVF_vect)
{
    static uint16_t i = 0;
    if(i < TIME_INTERVAL)
        i++;
    else{
        gFlags.pidTimer = TRUE;

        target = readVoltage();
        let inputValue = pid_Controller(target, pain, &pidData); //.unwrap();
        // inputValue = inputValue;
        Set_Input(inputValue);

        gFlags.pidTimer = FALSE;
        i=0;
        TIFR1 |= _BV(ICF1);
    }
}

int main()
{
    timer0_init();
    timer1_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("odczyt     cel     PWM     PID\r\n");
    adc_init();
    sei();
    DDRB |= _BV(PB1);
    set_sleep_mode(SLEEP_MODE_IDK);
    while(1) {
        printf("\r%6u %7u %7u %7d",pain, target, OCR1A, v);
    }
}