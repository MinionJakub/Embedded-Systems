#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include "i2c.h"

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

const uint8_t eeprom_addr = 0xa0;

uint8_t read_addr(uint8_t addr)
{
    i2cStart();
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cSend(addr & 0xff);
    i2cStart();
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    uint8_t data = i2cReadNoAck();
    i2cStop();
    return data;
}

void write_addr(uint8_t addr, uint8_t data)
{
    i2cStart();
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cSend(addr & 0xff);
    i2cSend(data);
    i2cStop();
}

void read_addr_seq(uint8_t addr, uint8_t len, uint8_t* dest)
{
    i2cStart();
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cSend(addr & 0xff);
    i2cStart();
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    for (uint8_t i=1; i<len; i++)
    {
        dest[i-1]=i2cReadAck();
    }
    dest[len-1] = i2cReadNoAck();
    i2cStop();
}

void write_addr_seq(uint8_t addr, uint8_t len, uint8_t* src)
{
    uint8_t i = 0;
    uint8_t j = 0;
    while (i < len)
    {
        if (j==0)
        {
            i2cStart();
            i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
            i2cSend(addr & 0xff);
        }

        i2cSend(src[i]);
        i++;
        j++;
        addr++;

        if (j==4)
        {
            j = 0;
            i2cStop();
            _delay_ms(20);
        }
    }
    if(j!=0)
    {
        i2cStop();
        _delay_ms(20);
    }
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
    uint8_t buf[21];
    while(1) {
        printf("ready\r\n");
        scanf("%c",&command);
        if (command == 'r')
        {
            printf("read\r\n");
            uint16_t addr;
            scanf("%d",&addr);
            printf("odczytano: %d\r\n",addr);
            printf("odczytano %.3x: %c\r\n",addr,read_addr(addr));
        }
        else if (command == 'w')
        {
            printf("write\r\n");
            uint16_t addr;
            char data;
            scanf("%d %c",&addr, &data);
            printf("odczytano: %d, %d\r\n",addr, data);
            write_addr(addr, data);
            printf("zapisano %.3x: %d\r\n",addr,read_addr(addr));
        }
        else if (command == 't')
        {
            printf("sequential read\r\n");
            uint16_t addr;
            uint16_t len;
            scanf("%d %d",&addr, &len);
            if (len > 20) len = 20;
            printf("odczytano: %d %d\r\n",addr,len);
            read_addr_seq(addr, len, buf);
            printf("odczytano %.3x: ",addr);
            for(uint8_t i = 0; i<len;i++)
            {
                printf("%c",buf[i]);
            }
            printf("\r\n");
        }
        else if (command == 'e')
        {
            printf("sequential write\r\n");
            uint16_t addr;
            uint16_t len;
            scanf("%d %d",&addr, &len);
            if (len > 20) len = 20;
            scanf("%s",buf);
            printf("odczytano: %d %d\r\n",addr,len);
            write_addr_seq(addr, len, buf);
            read_addr_seq(addr, len, buf);
            printf("odczytano %.3x: ",addr);
            for(uint8_t i = 0; i<len;i++)
            {
                printf("%c",buf[i]);
            }
            printf("\r\n");
        }
        for(uint8_t i = 0; i<20; i++)
        {
            buf[i] = ' ';
        }
        printf("\r\n");
    }
}
