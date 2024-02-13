#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define SMOOTHNESS 300
#define FACTOR 125
#define BETA 14
#define GAMMA 20

static inline uint8_t gauss(int16_t value,int16_t MAX_CAP){
    int16_t exponent = ((value * FACTOR / SMOOTHNESS) - BETA)/GAMMA;
    exponent *= exponent;
    exponent = 1 + exponent + ((exponent*exponent) >> 1);
    return (uint8_t)(exponent > MAX_CAP ? MAX_CAP : exponent);
}

static inline uint8_t square_f(int16_t value, int16_t MAX_CAP){
    int16_t exponent = (value >> 3);
    exponent *= exponent;
    // exponent /= 20;
    return (uint8_t)(exponent > MAX_CAP ? MAX_CAP : exponent);
}

void timer1_init()
{
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM10);
  TCCR1B = _BV(WGM12) | _BV(CS10);
  DDRB |= _BV(PB1);
}


void timer2_init(){
  TCCR1A |= _BV(COM1B1) | _BV(COM1B0);
  DDRB |= _BV(PB2);
}

void timer3_init(){
    // PRR &= ~_BV(PRTIM2);
    TCCR2A = _BV(COM2A1) | _BV(COM2A0) 
    | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    DDRB |= _BV(PB3);
}

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

int main(){
    timer1_init();
    timer2_init();
    timer3_init();
    // OCR0A = 0;
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // adc_init();
    uart_init();
    while(1){
        // printf("%"PRIu8"\r\n",PORTB & _BV(PB0));
        for(int16_t i = 0; i <= 70; i++){
            OCR1A = square_f(2*i,0);
            OCR1B = square_f(2*i,0);
            OCR2A = square_f(2*i,0);
            _delay_ms(19);
        }
        for(int16_t i = 70; i > 0; i--){
            OCR1A = square_f(2*i,0);
            OCR1B = square_f(2*i,0);
            OCR2A = square_f(2*i,0);
            _delay_ms(19);
        }
    }
}

// #include <avr/io.h>


// int main()
// {
//   // uruchom licznik
//   timer1_init();
//   // ustaw wypełnienie 50%
//   OCR1A = 0;
//   while(1);
// }

