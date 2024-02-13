#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

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

// uint16_t now;
// uint16_t elapsed;
// uint16_t exec_time;
float first;
float second;
float result;

// int8_t add()

int main(){
    // sys_clock_1();
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    while(1){
        // printf("Podaj dwie liczby obie w zakrese -128 - +127\r\n");
        scanf("%f",&first);
        scanf("%f",&second);
        float value = first + second;
        printf("Wynik operacji %c:\t%f""\r\n",'+',value);

        value = first - second;
        printf("Wynik operacji %c:\t%f""\r\n",'-',value);


        value = first * second;
        printf("Wynik operacji %c:\t%f""\r\n",'*',value);


        value = first / second;
        printf("Wynik operacji %c:\t%f""\r\n",'/',value);
    }
}

