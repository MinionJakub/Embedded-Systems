#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD) - 1)

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

const uint8_t time_addr = 0b11010000;



void getData(){
  i2cStart();
  i2cSend(time_addr);
  i2cSend(4);
  i2cStop();
  i2cStart();
  i2cSend(time_addr | 0x1);
  uint8_t day = i2cReadAck();
  uint8_t mon = i2cReadAck();
  uint8_t year = i2cReadNoAck();
  i2cStop();
  printf("%d%d-%d%d-%d%d%d%d\r\n", (day>>4), day&0b1111, (mon>>4)&0b1, mon&0b1111, (mon>>7)+1, 9*((mon>>7)^1), year>>4, year&0b1111);
}

void setData(uint8_t day, uint8_t month, uint8_t year){
  i2cStart();
  i2cSend(time_addr);
  i2cSend(4);
  year = year - 8;
  uint8_t century = year/1000 - 1;
  i2cSend((day/10)<<4 | day%10);
  i2cSend(century<<7 | (month/10)<<4 | month%10);
  i2cSend((year%100)/10<<4 | year%10);
  i2cStop();
}

void getTime(){
  i2cStart();
  i2cSend(time_addr);
  i2cSend(0);
  i2cStop();
  i2cStart();
  i2cSend(time_addr|0x1);
  uint8_t sec = i2cReadAck();
  uint8_t min = i2cReadAck();
  uint8_t hour = i2cReadNoAck();
  i2cStop();
  uint8_t d;
  uint8_t j;
  if(hour & 0b01000000){
    uint8_t value = hour&0b00001111 + 10*(((hour)>>4)&1) + 12*(((hour)>>5)&1);
    d = value/10;
    j = value%10;
  }
  else
  {
    d = 2*((hour&0b00100000)>>5) + ((hour&0b00010000)>>4);
    j = hour&0b00001111;
  }
  printf("%d%d:%d%d:%d%d\n\r", d, j, min>>4, min&0b00001111, sec>>4, sec&0b00001111);
}

void setTime(uint8_t h, uint8_t m, uint8_t s)
{
  i2cStart();
  i2cSend(time_addr);
  i2cSend(0);

  i2cSend(((s/10)<<4) | s%10);
  i2cSend(((m/10)<<4) | m%10);
  i2cSend(((h/10)<<4) | h%10);
  i2cStop();
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  // program testowy
  char command;
  printf("gettime = t, settime = w, getdate = r, setdate = e, status = s\r\n");
  while(1)
  {
    printf("ready\r\n");
    scanf("%c",&command);
    if (command == 't')
    {
      printf("time\r\n");
      getTime();
    }
    else if (command == 'r')
    {
      printf("date\r\n");
      getData();
    }
    else if (command == 'w')
    {
      printf("set time\r\n");
      uint16_t a, b, c;
      scanf("%d %d %d",&a, &b, &c);
      setTime(a, b, c);
      getTime();
    }
    else if (command == 'e')
    {
      printf("set date\r\n");
      uint16_t a, b, c;
      scanf("%d %d %d",&a, &b, &c);
      setData(a, b, c);
      getData();
    }
    else if (command == 's')
    {
      printf("status\r\n");
      getData();
      getTime();
    }
    printf("\r\n");
    _delay_ms(1000);
  }
}
