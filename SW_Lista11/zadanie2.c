#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem


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
void adc_init_ADC0()
{
    ADMUX = _BV(REFS0);
    ADMUX |= 0b0000;
    // DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}
void adc_init_ADC1()
{
    ADMUX = _BV(REFS0);
    ADMUX |= 0b0001;
    // DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}

void clock_init()
{
    TCCR1A = _BV(COM1A0) | _BV(WGM10);
    TCCR1B = _BV(CS11) | _BV(WGM13);
    OCR1A = 500;
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
}

static uint16_t opening, closing;
ISR(TIMER1_CAPT_vect) {
    ADCSRA |= _BV(ADSC);
    while (!(ADCSRA & _BV(ADIF)));
    ADCSRA |= _BV(ADIF);
    closing = (ADC * 5) / 1024;
    closing = closing * 1000;
    // printf("period %f\r\n", ADC);
}
ISR(TIMER1_OVF_vect) {
    ADCSRA |= _BV(ADSC);
    while (!(ADCSRA & _BV(ADIF)));
    ADCSRA |= _BV(ADIF);
    opening = (ADC * 5000) / 1024;
    opening = opening * 1000;
}

int main()
{   
    DDRB |= _BV(PB1);
    clock_init();
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    adc_init_ADC1();
    sei();
    while (1)
    {
        printf("opening %"PRIu16", closing %"PRIu16" \r\n", opening, closing);
        _delay_ms(1000);
    }
}