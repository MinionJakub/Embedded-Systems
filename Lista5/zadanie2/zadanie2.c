#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) 

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

FILE uart_file;

static inline void adc_init(){
    ADMUX = _BV(REFS0); //REFRENCE AVCC 
    ADMUX &= ~(0b1111); //INPUT ADC0
    DIDR0 = _BV(ADC0D); //TURN OFF DIGITAL INPUT ADC0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //PRESKALER 128
    ADCSRA |= _BV(ADEN); //TURN ON ADC
    ADCSRA |= _BV(ADATE) | _BV(ADIE); //AUTO TRIGGER
    ADCSRB |= _BV(ADTS1); //INT0 - EXTERNAL INTERRUPT REQUEST 0 - PB0
}


void timer0_init(){
    TCCR0A |= _BV(WGM01);
    TCCR0B |= _BV(CS01) | _BV(CS00);
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 124;
}

volatile uint32_t num = 0;
// volatile float num = 0;

ISR(INT0_vect){}

ISR(ADC_vect){
    uint16_t v = ADC;
    uint32_t num = (((uint32_t)v * 625 * 5) >> 8);
    num = 10000 - num;
}

// ISR(ADC_vect){
//     uint16_t v = ADC;
//     float u = (v*5.0)/1024.0;
//     num = u *10000.0 /(5.0-u);
// }


ISR(TIMER0_COMPA_vect){}

int main(){
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    set_sleep_mode(SLEEP_MODE_IDLE);
    adc_init();
    sei();
    // timer0_init();
    EICRA = _BV(ISC01) | _BV(ISC00);
    EIMSK = _BV(INT0);
    while(1){
        printf("value\r\n");
        printf("%"PRIu32"\r\n",num);
        // printf("%f\r\n",num);
        // sleep_mode();
        _delay_ms(100);
    }
}