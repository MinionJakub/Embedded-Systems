#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include "pid.c"
#include "pid.h"


#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem
#define BUFFER_SIZE 64
#define V_0 0.4
#define TemperatureCoefficient 0.0195
#define TIME_INTERVAL   457
#define K_P     1.000
#define K_I     0.300
#define K_D     0.120

volatile int16_t v = 0;

struct GLOBAL_FLAGS {
    //! True when PID control loop should run one time
    uint8_t pidTimer:1;
    uint8_t dummy:7;
} gFlags = {0, 0};

volatile struct PID_DATA pidData;


volatile uint16_t target = 155;
volatile uint16_t value = 1023;

#define GRZALKA PB5        // Dioda
#define GRZALKA_DDR DDRB   // dioda ddr
#define GRZALKA_PORT PORTB // dioda port

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
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

FILE uart_file;

// inicjalizowanie ADC
void adc_init()
{
    // ADMUX = _BV(REFS0) | _BV(REFS1);
    // ADMUX |= 0b0000;
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);// | _BV(ADATE) | _BV(ADIE);
    ADCSRA |= _BV(ADEN);
}

// void adc_init()
// {
//   DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
//   // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
//   ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
//   ADCSRA |= _BV(ADEN); // włącz ADC
//   //ADCSRA |= _BV(ADATE);
// }

static float ReadTemperature;
static float TargetTemperature = 25.0;
static float Th = 1;
static bool isCooling = false;
ISR(ADC_vect)
{
    ADCSRA |= _BV(ADIF);
    float V_IN = (ADC * 1.1) / 1024;
    ReadTemperature = (V_IN - V_0) / TemperatureCoefficient; // z noty katalogowej termometra

    if (isCooling)
    {
        if (ReadTemperature < TargetTemperature - Th)
        {
            isCooling = false;
            GRZALKA_PORT |= _BV(GRZALKA);
        }
    }
    else
    {
        if (ReadTemperature >= TargetTemperature)
        {
            isCooling = true;
            GRZALKA_PORT &= ~_BV(GRZALKA);
        }
    }
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

int16_t Get_Reference()
{
    return target;
}

int16_t Get_Measurement()
{
    return value;
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
        value = readVoltage();
        printf("\r%6u %7u %7u %7d",value, target, OCR1A, v);
        i = 0;
    }
}


int main()
{
    GRZALKA_DDR |= _BV(GRZALKA);
    GRZALKA_PORT |= _BV(GRZALKA);

    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    
    adc_init();
    
    sei();

    ADCSRA |= _BV(ADSC);
    while (1)
    {

        char command[BUFFER_SIZE];
        scanf("%s", command);

        if (strcmp("show", command) == 0)
        {
            printf("Current Temperature: %f\r\n", ReadTemperature);
        } else if (strcmp("change", command) == 0)
        {   
            float newTargetTemperature;
            scanf("%f", &newTargetTemperature);
            TargetTemperature = newTargetTemperature;
            printf("Value Changed to %f\r\n", TargetTemperature);
        } else 
        {
            printf("Current Temperature: %f\r\n", ReadTemperature);

        }
    }
}