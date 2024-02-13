#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>

#define SPI_PORT PORTB
#define SPI_DDR DDRB
#define SPI_SS PB2
#define SPI_MOSI PB3
#define SPI_MISO PB4
#define SPI_SCK PB5

#define LED PD6        // Dioda
#define LED_DDR DDRD   // dioda ddr
#define LED_PORT PORTD // dioda port

#define BUTTON PC5
#define BUTTON_DDR DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN PINC
#define BUTTON_PRESSED ((!(BUTTON_PIN & _BV(BUTTON))) == 1 ? 0xFF : 0x00)

void init(void) {
  BUTTON_DDR &= ~_BV(BUTTON); // button input
  BUTTON_PORT |= _BV(BUTTON); // button pull-up
  LED_DDR |= _BV(LED);        // led output
}

void spi_init() {
  // mcu spi stuff set as input, miso as output
  uint8_t a;
  SPI_DDR |= _BV(SPI_MISO);
  SPCR = _BV(SPE) | _BV(SPIE);
  SPCR &= ~_BV(MSTR);
  a = SPSR;
  a = SPDR;
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data) {
  // rozpocznij transmisję
  uint8_t a;
  a = SPDR;
  SPDR = data;
  // zwróć otrzymane dane
  return a;
}

int main(void) {
  init();
  spi_init();
  // odmaskuj przerwania
  uint8_t a=0;
  while (1) {
    a = spi_transfer(BUTTON_PRESSED);
    if (a)
      LED_PORT |= _BV(LED);
    else
      LED_PORT &= ~_BV(LED);
  }
}
