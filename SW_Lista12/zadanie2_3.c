#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyÅ›Ä‡ rejestr UCSR0A
  UCSR0A = 0;
  // wÅ‚Ä…cz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aÅ¼ transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aÅ¼ znak dostÄ™pny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}


void adc_init()
{
  ADMUX   = _BV(REFS0);// referencja 1,1V, wejÅ›cie ADC0
  ADMUX  |= _BV(REFS1);
  //ADMUX   |= 0b0000; // ???
  DIDR0   = _BV(ADC0D); // wyÅ‚Ä…cz wejÅ›cie cyfrowe na ADC0
  // czÄ™stotliwoÅ›Ä‡ zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // wÅ‚Ä…cz ADC
  //ADCSRA |= _BV(ADATE);
}

uint16_t readVoltage(void) {
	_delay_ms(1);
	ADCSRA |= _BV(ADSC);
	while( ADCSRA & _BV( ADSC) ) ;
	uint8_t low  = ADCL;
	uint8_t high = ADCH;
	uint16_t adc = (high << 6) | low;		// 0<= result <=1023
	//uint8_t vccx10 = (uint8_t) ( (11 * 1024) / adc);
	return adc;
}

FILE uart_file;

void timer_init()
{
    TCCR1B = _BV(CS11) | _BV(WGM12);
    OCR1A = 39999;
    TIMSK1 = _BV(OCIE1A);
}

volatile uint16_t target = 5;
volatile uint16_t pain = 1023;
ISR(TIMER1_COMPA_vect)
{
    pain = readVoltage();
    if (pain < target - 5) PORTB |= _BV(PB5);
    else if (pain > target + 5) PORTB &= ~_BV(PB5);
    printf("\r%5d %6d",pain,target);
}

int main()
{
    timer_init();
    uart_init();
    // skonfiguruj strumienie wejÅ›cia/wyjÅ›cia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("VALUE TARGET\n\r");
    adc_init();
    sei();
    DDRB |= _BV(PB5);
    while(1) {
        scanf("%d",&target);
    }
}