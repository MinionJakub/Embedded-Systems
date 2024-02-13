#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) 

#define LED PB1
#define LED_DDR DDRB
#define LED_PORT PORTB 

#define MAX_VALUE 10000

void light(uint16_t value){
  if(value < 100){
    OCR1A = MAX_VALUE;
  }
  else if(value < 200){
    OCR1A = MAX_VALUE - 625;
  }
  else if(value < 300){
    OCR1A = MAX_VALUE - 1406;
  }
  else if(value < 400){
    OCR1A = MAX_VALUE - 2500;
  }
  else if(value < 500){
    OCR1A = MAX_VALUE - 3906;
  }
  else if(value < 600){
    OCR1A = MAX_VALUE - 5625;
  }
  else if(value < 700){
    OCR1A = MAX_VALUE - 7656;
  }
  else{
    OCR1A = 0;
  }
}

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
    ADMUX = _BV(REFS0);
    ADMUX &= ~(0b1111);
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}

uint16_t read_ADC(){
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
  // printf("%u\r\n",v>>3);
  return v >> 3;
}

void timer1_init()
{
  ICR1 = 15624;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS00);// | _BV(CS10) | _BV(CS12);
  DDRB |= _BV(PB1);
}


int main(){
  adc_init();
  UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
  uart_init();
  timer1_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // DDRB |= _BV(PB1);
  while(1){
    uint16_t value = read_ADC();
    //printf("%u\r\n",value*value);
    value = value <= 100 ? value : 100;
    // OCR1A = 0;
    OCR1A = MAX_VALUE - value * value;
    printf("%u\t%u\r\n",value,OCR1A);
    _delay_ms(100);
  }
}