#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

FILE uart_file;

void i2cWaitForComplete() {
  // czekaj na flagę TWINT
  while(!(TWCR & _BV(TWINT)));
}

uint8_t i2cReadAck() {
  // odczytaj dane, wyślij ACK
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA); 
    i2cWaitForComplete();
    return TWDR;
}

uint8_t TWI_slaveAddress;
uint8_t data1, data2, data3, data4;

void TWI_init()
{
    TWI_slaveAddress = 0x7f;
    TWBR = 32;
    TWCR = _BV(TWEN) | _BV(TWEA); 
    TWAR = TWI_slaveAddress << 1;    
}
int main(void)
{

    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    TWI_init();
    for (;;)
    {
        data1 = i2cReadAck();
        data2 = i2cReadAck();
        data3 = i2cReadAck();
        data4 = i2cReadAck();
        printf("read data: %"PRIu8" %"PRIu8" %"PRIu8" %"PRIu8"\r\n", data1, data2, data3, data4);
    }
}

