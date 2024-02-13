#define __AVR_ATmega328P__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>


#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem


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

#define BUTTON PD2
#define BUTTON_PORT PORTD
#define BUTTON_DDR DDRD
#define BUTTON_PIN PIND

#define REF_VOLT 5
#define R 10000

volatile float num = 0;

void initInterrupt0(void) {
  EIMSK |= (1 << INT0); /* enable INT0 */
  EICRA |= (1 << ISC00); /* trigger when button changes */
}

ISR(INT0_vect) {
  if(bit_is_clear(BUTTON_PIN, BUTTON)){
  }
}

ISR(ADC_vect){
    uint16_t v = ADC;
    float u = (v*5.0)/1024.0;
    num = u *1000.0 /(5.0-u);
}

void setup_ADC(){
  ADMUX = _BV(REFS0); // setup ref vol (here it is 5V)

  ADMUX &= ~(0b1111);// setup input pins

  DIDR0 = _BV(ADC0D); // disable digital input on pin adc0
  ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //setup prescaler
  ADCSRA |= _BV(ADEN); // enables the ADC
  ADCSRA |= _BV(ADIE) | _BV(ADATE); // enable adc complete interrupt

  ADCSRB |= _BV(ADTS1); // enable auto-trigger when external input 0 positive edge
}

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);

    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);
    
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();
    initInterrupt0();
    setup_ADC();

    while(1){
    //   printf("Last value: %"PRIu32"\r\n", lastPhotoValue);
    printf("Last readed value:\t%f\r\n",num);
    _delay_ms(200);
    }
}