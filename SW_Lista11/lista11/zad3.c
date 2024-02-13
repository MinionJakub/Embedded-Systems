#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem
#define BUFFER_SIZE 64
#define V_0 0.4
#define TemperatureCoefficient 0.010

#define LEFT PD5       
#define RIGHT PD6        
#define STEROWANIE_DDR DDRD   // dioda ddr
#define STEROWANIE_PORT PORTD // dioda port

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
    ADMUX = _BV(REFS0);
    ADMUX |= 0b0000;    // DIDR0 = _BV(ADC0D);

    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADATE) | _BV(ADIE);
    ADCSRA |= _BV(ADEN);
}

ISR(ADC_vect)
{
    ADCSRA |= _BV(ADIF);
    if(ADC < 512)
    {
        STEROWANIE_PORT = _BV(LEFT);
        OCR1A = 512 - ADC;
    } else 
    {
        STEROWANIE_PORT = _BV(RIGHT);
        OCR1A = ADC - 512;
    }
    // float V_IN = (ADC * 5.0) / 1024.0;
    // printf("POTENCJAL: %d %d\r\n", PORTD & _BV(LEFT), PORTD & _BV(RIGHT));

}

void clock_init()
{
    TCCR1A = _BV(WGM10) | _BV(WGM11) | _BV(COM1A0);
    TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);
    OCR1A = 100;
    // TIMSK1 |= _BV(OCIE1A);
}


int main()
{   
    STEROWANIE_DDR |= _BV(LEFT) | _BV(RIGHT);
    DDRB |= _BV(PB1);
    clock_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    
    adc_init();
    sei();

    ADCSRA |= _BV(ADSC);
    while (1)
    {

    }
}