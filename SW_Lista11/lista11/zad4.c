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

#define GRZALA PB5        // Dioda
#define GRZALA_DDR DDRB   // dioda ddr
#define GRZALA_PORT PORTB // dioda port

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
    ADMUX |= 0b0000;
    // DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADATE) | _BV(ADIE);
    ADCSRA |= _BV(ADEN);
}

ISR(ADC_vect)
{
    ADCSRA |= _BV(ADIF);
    OCR1A = ADC / 5;

}

void clock_init()
{
    TCCR1A = _BV(WGM10) | _BV(WGM11) | _BV(COM1A0);
    TCCR1B = _BV(CS12) | _BV(WGM12) | _BV(WGM13);
    OCR1A = 200;
    // TIMSK1 |= _BV(OCIE1A);
}


int main()
{   
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