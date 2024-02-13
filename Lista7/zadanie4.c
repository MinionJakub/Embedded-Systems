#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem

#define SS_CONTROLLER PD4
#define MOSI_CONTROLLER PD5
#define MISO_CONTROLLER PD6
#define SCK_CONTROLLER PD7

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

// inicjalizacja UART
void uart_init()
{
  UBRR0 = UBRR_VALUE;

  UCSR0A = 0;
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


FILE uart_file;

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

// a transfer has been completed
ISR(SPI_STC_vect){
    uint8_t value = reverse(SPDR);
    printf("Received: "BYTE_TO_BINARY_PATTERN"\r\n", BYTE_TO_BINARY(value));
}

void setup_spi(){
    // configure SPI as slave
    SPCR |= _BV(SPIE) | _BV(SPE);

    // Turn on interruSPI as slavepts
    SPSR |= _BV(SPIF);
}

/*
We write the byte by manually writing the bits :))
*/
void send_byte(uint8_t byte){

    PORTD &= ~_BV(SS_CONTROLLER);

    // go over each bit and show it by high or low mosi value
    for(int i = 0; i<8; i++){

        if(byte & 1)
            PORTD |= _BV(MOSI_CONTROLLER); 
        else
            PORTD &= ~_BV(MOSI_CONTROLLER); 

        byte >>= 1;

        // sck rising edge
        PORTD |= _BV(SCK_CONTROLLER);

        PORTD &= ~_BV(SCK_CONTROLLER);
    }

    PORTD |= _BV(SS_CONTROLLER);

}

int main(){
    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    DDRD |= _BV(SCK_CONTROLLER) | _BV(MOSI_CONTROLLER) | _BV(SS_CONTROLLER);
    PORTD |= _BV(SS_CONTROLLER);


    sei();
    setup_spi();

    uint8_t counter = 0;

    while(1){
        send_byte(counter++);

        _delay_ms(1000);
    }
}
