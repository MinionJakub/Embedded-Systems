#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem

#define LED PB1 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port 

uint16_t pomiar(){
      uint16_t result = 0;
      LED_PORT &= ~_BV(LED);
      for(int8_t i = 0; i < 4; i++){
        ADCSRA |= _BV(ADSC);
        while(!(ADCSRA & _BV(ADIF)));
        ADCSRA |= _BV(ADIF);
        result+=ADC;
      }
      return ((result>>2)>>1)<<1;
}
static inline void migotanie(uint16_t frequency){
  if(frequency < 100){
    LED_PORT &= ~_BV(LED);
    _delay_ms(8);
  }
  else if (frequency < 200){
      LED_PORT |= _BV(LED);
    _delay_us(50);
    LED_PORT &= ~_BV(LED);
    _delay_us(750);
  }
  else if(frequency < 300){
    LED_PORT |= _BV(LED);
    _delay_us(150);
    LED_PORT &= ~_BV(LED);
    _delay_us(650);
  }
  else if(frequency < 400){
    LED_PORT |= _BV(LED);
    _delay_us(250);
    LED_PORT &= ~_BV(LED);
    _delay_us(550);
  }
  else if(frequency < 500){
    LED_PORT |= _BV(LED);
    _delay_ms(1);
    LED_PORT &= ~_BV(LED);
    _delay_ms(7);
  }
  else if(frequency < 600){
    LED_PORT |= _BV(LED);
    _delay_ms(2);
    LED_PORT &= ~_BV(LED);
    _delay_ms(6);
  }
  else if(frequency < 700){
    LED_PORT |= _BV(LED);
    _delay_ms(3);
    LED_PORT &= ~_BV(LED);
    _delay_ms(5);
  }
  else if(frequency < 800){
    LED_PORT |= _BV(LED);
    _delay_ms(4);
    LED_PORT &= ~_BV(LED);
    _delay_ms(4);
  }
  else if(frequency < 900){
    LED_PORT |= _BV(LED);
    _delay_ms(5);
    LED_PORT &= ~_BV(LED);
    _delay_ms(3);
  }    
  else if(frequency < 1000){
    LED_PORT |= _BV(LED);
    _delay_ms(6);
    LED_PORT &= ~_BV(LED);
    _delay_ms(2);
  }
  else {
      LED_PORT |= _BV(LED);
    _delay_ms(8);
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

//inicjalizowanie ADC
void adc_init(){
    ADMUX = _BV(REFS0);
    ADMUX &= ~(0b1111);
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}

int main(){
    LED_DDR  |= _BV(LED);
    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    adc_init();
    while(1){
      uint16_t v = pomiar();
      printf("Odczytano: %"PRIu16"\r\n",v);
      migotanie(v);
    }
}