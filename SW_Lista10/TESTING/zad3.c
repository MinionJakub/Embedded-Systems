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
#define TemperatureCoefficient 0.0195

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
    ADMUX = _BV(REFS0) | _BV(REFS1);
    ADMUX |= 0b0000;
    // DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADATE) | _BV(ADIE);
    ADCSRA |= _BV(ADEN);
}

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
            GRZALA_PORT |= _BV(GRZALA);
        }
    }
    else
    {
        if (ReadTemperature >= TargetTemperature)
        {
            isCooling = true;
            GRZALA_PORT &= ~_BV(GRZALA);
        }
    }
}

int main()
{
    GRZALA_DDR |= _BV(GRZALA);
    GRZALA_PORT |= _BV(GRZALA);

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