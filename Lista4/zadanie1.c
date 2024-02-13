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

void timer1_init()
{
  // ustaw tryb licznika
  // WGM1  = 0000 -- normal
  // CS1   = 001  -- prescaler 1
  TCCR1B = _BV(CS10);
}


const static uint16_t d_time = 2000;


void operations_int8(){
    volatile int8_t num1 = 7;
    volatile int8_t num2 = 3;
    volatile uint16_t clock1;
    volatile uint16_t clock2;
    volatile int8_t result;
    clock1 = TCNT1;
    result = num1 + num2;
    clock2 = TCNT1;
    printf("Dodwanie int8_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 - num2;
    clock2 = TCNT1;
    printf("Odejmowanie int8_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 * num2;
    clock2 = TCNT1;
    printf("Mnożenie int8_t:\t%u\r\n",clock2-clock1);
    TCNT1 = 0;
    clock1 = TCNT1;
    result = num1/num2;
    clock2 = TCNT1;
    printf("Dzielenie int8_t:\t%u\r\n",clock2-clock1);
    _delay_ms(d_time);
}

void operations_int16(){
    volatile int16_t num1 = 7;
    volatile int16_t num2 = 3;
    volatile uint16_t clock1;
    volatile uint16_t clock2;
    volatile int16_t result;
    clock1 = TCNT1;
    result = num1 + num2;
    clock2 = TCNT1;
    printf("Dodwanie int16_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 - num2;
    clock2 = TCNT1;
    printf("Odejmowanie int16_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 * num2;
    clock2 = TCNT1;
    printf("Mnożenie int16_t:\t%u\r\n",clock2-clock1);
    TCNT1 = 0;
    clock1 = TCNT1;
    result = num1/num2;
    clock2 = TCNT1;
    printf("Dzielenie int16_t:\t%u\r\n",clock2-clock1);
    _delay_ms(d_time);
}
void operations_int32(){
    volatile int32_t num1 = 7;
    volatile int32_t num2 = 3;
    volatile uint16_t clock1;
    volatile uint16_t clock2;
    volatile int32_t result;
    clock1 = TCNT1;
    result = num1 + num2;
    clock2 = TCNT1;
    printf("Dodwanie int32_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 - num2;
    clock2 = TCNT1;
    printf("Odejmowanie int32_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 * num2;
    clock2 = TCNT1;
    printf("Mnożenie int32_t:\t%u\r\n",clock2-clock1);
    TCNT1 = 0;
    clock1 = TCNT1;
    result = num1/num2;
    clock2 = TCNT1;
    printf("Dzielenie int32_t:\t%u\r\n",clock2-clock1);
    _delay_ms(d_time);
}
void operations_int64(){
    volatile int64_t num1 = 7;
    volatile int64_t num2 = 3;
    volatile uint16_t clock1;
    volatile uint16_t clock2;
    volatile int64_t result;
    clock1 = TCNT1;
    result = num1 + num2;
    clock2 = TCNT1;
    printf("Dodwanie int64_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 - num2;
    clock2 = TCNT1;
    printf("Odejmowanie int64_t:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 * num2;
    clock2 = TCNT1;
    printf("Mnożenie int64_t:\t%u\r\n",clock2-clock1);
    TCNT1 = 0;
    clock1 = TCNT1;
    result = num1/num2;
    clock2 = TCNT1;
    printf("Dzielenie int64_t:\t%u\r\n",clock2-clock1);
    _delay_ms(d_time);
}
void operations_float(){
    volatile float num1 = 7;
    volatile float num2 = 3;
    volatile uint16_t clock1;
    volatile uint16_t clock2;
    volatile float result;
    clock1 = TCNT1;
    result = num1 + num2;
    clock2 = TCNT1;
    printf("Dodwanie float:\t\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 - num2;
    clock2 = TCNT1;
    printf("Odejmowanie float:\t%u\r\n",clock2-clock1);
    clock1 = TCNT1;
    result = num1 * num2;
    clock2 = TCNT1;
    printf("Mnożenie float:\t\t%u\r\n",clock2-clock1);
    TCNT1 = 0;
    clock1 = TCNT1;
    result = num1/num2;
    clock2 = TCNT1;
    printf("Dzielenie float:\t%u\r\n",clock2-clock1);
    _delay_ms(d_time);
}

int main(){
 uart_init();
 fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
 stdin = stdout = stderr = &uart_file;
 timer1_init();
 while(1){
  operations_int8();
  operations_int16();
  operations_int32();
  operations_int64();
  operations_float();
  _delay_ms(4*d_time);
 } 
}