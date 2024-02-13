#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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
    ADMUX = _BV(REFS0);
    ADMUX &= ~(0b1111);
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}

#define SMOOTHNESS 100
#define FACTOR 125
#define BETA 14
#define GAMMA 20

static inline int16_t gauss(int16_t value){
    int16_t exponent = ((value * FACTOR / SMOOTHNESS) - BETA)/GAMMA;
    exponent *= exponent;
    exponent = 1 + exponent + ((exponent*exponent) >> 1);
    return exponent > 255 ? 255 : exponent;
}

int main(){
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // adc_init();
    uart_init();
    while(1){
        for(int16_t i = 0; i <= SMOOTHNESS; i++){
            printf("Gauss od %"PRId16":\t %"PRId16"\r\n",i,gauss(i));
            _delay_ms(100);
        }

        for(int16_t i = SMOOTHNESS; i > 0; i--){
            printf("Gauss od %"PRId16":\t %"PRId16"\r\n",i,gauss(i));
            _delay_ms(100);
        }
    }
}