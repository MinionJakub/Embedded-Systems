#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"


#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

int hd44780_transmit(char data, FILE *stream)
{
  LCD_WriteData(data);
  return 0;
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

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

static inline void do_new_line(char* first_raw, char* text, uint8_t written){
//  LCD_GoTo(0,0);
  LCD_Clear();
  for(uint8_t i = 0; i < written; i++){
    text[0] = first_raw[i];
    LCD_WriteText(text);        
  }
  written = 0;
  LCD_GoTo(0,1);
}

FILE hd44780_file;
FILE uart_file;
int main()
{
  // skonfiguruj wyświetlacz
  uart_init();
  LCD_Initialize();
  LCD_Clear();
  // skonfiguruj strumienie wyjściowe
  fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
  stdout = stderr = &hd44780_file;
  fdev_setup_stream(&uart_file,NULL,uart_receive,_FDEV_SETUP_READ);
  stdin = &uart_file;
  LCD_WriteCommand(0b1111);
  char sign;
  char* text = "0";
  char first_raw[16];
  uint8_t value = 0;
  uint8_t written = 0;
  while(1){
    sign = getchar();
    // if(sign == '\r'){}
    if(sign == '\r' || sign == '\n' || sign == EOF){
      do_new_line(first_raw,text,written);
      written = 0;
    }
    else{
      text[0] = sign;
      LCD_WriteText(text);
      first_raw[written++] = sign;
      if(written == 16){
        do_new_line(first_raw,text,written);
        written = 0;
      }
    }
  }
}