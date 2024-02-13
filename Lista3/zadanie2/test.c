#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

void uart_init()
{
  UBRR0 = UBRR_VALUE;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int uart_transmit(char data, FILE *stream)
{
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

int uart_receive(FILE *stream)
{
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  ADMUX   |= 0b1110; // ???
  //DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

uint8_t readVccVoltage(void) {
    _delay_ms(1);
    ADCSRA |= _BV(ADSC);                // Start a conversion
    while( ADCSRA & _BV( ADSC) ) ;
    uint8_t low  = ADCL;
    uint8_t high = ADCH;
    uint16_t adc = (high << 8) | low;        // 0<= result <=1023
    uint8_t vccx10 = (uint8_t) ( (11 * 1024) / adc);

    return vccx10 ;

}

FILE uart_file;

int main(void)
{
    adc_init();
    DDRB |= _BV(PORTB5);
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    uint8_t led = 0;
    while(1)
    {
        if (led==0) PORTB |= _BV(PORTB5);
        else PORTB &= ~_BV(PORTB5);
        led = !led;
        _delay_ms(1000);
        uint8_t vccx10 = readVccVoltage();

        printf("Odczytano: %0d\r\n",vccx10);
        _delay_ms(1000);

    }
}
