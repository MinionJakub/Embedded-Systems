#define __AVR_ATmega328P__

#include <avr/io.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <inttypes.h>

#define ADC_INTERRUPT 1
#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD) - 1)

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

//inicjalizowanie ADC
void adc_init(){
    ADMUX = _BV(REFS0);
    ADMUX |= 0b1110;
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN) | _BV(ADATE);
    if(ADC_INTERRUPT) ADCSRA |= _BV(ADIE);
} 

volatile uint32_t value = 0;

ISR(ADC_vect){
    ADCSRA |= _BV(ADIF);
    value = 112640 / ADC;
}

volatile uint32_t active[128];
volatile uint32_t sleeping[128];
volatile uint64_t median_active = 0;
volatile uint64_t median_sleeping = 0;
volatile uint64_t result_active = 0;
volatile uint64_t result_sleeping = 0;

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    adc_init();
    sei();
    set_sleep_mode(SLEEP_MODE_IDLE);
    for(uint16_t i = 0; i < 128; i++){
      printf("%"PRIu16"\r\n",i);
      ADCSRA |= _BV(ADSC);
      while(!(ADCSRA & _BV(ADIF)));
      ADCSRA |= _BV(ADIF);
      value = 112640 / ADC; 
      active[i] = value;
      median_active += (uint64_t) value;
    }
    for(uint16_t i = 0; i < 128; i++){
      printf("%"PRIu16"\r\n",i);
      sleep_mode();
      sleeping[i] = value;
      median_sleeping += (uint64_t) value;
    }
    median_active >>= 7;
    median_sleeping >>= 7;
    for(int i = 0; i< 128; i++){
      result_active += (active[i] - median_active)*(active[i] - median_active);
      result_sleeping += (sleeping[i] - median_sleeping)*(sleeping[i] - median_sleeping);
    }
    result_active >>= 7;
    result_sleeping >>= 7;
    printf("wariancja aktywynego czekania:\t%"PRIu32"\r\n",(uint32_t) result_active);
    printf("wariancja spiacego czekania:\t%"PRIu32"\r\n",(uint32_t) result_sleeping);
    while(1){
    }
}