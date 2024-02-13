#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#define LED PB2        // Dioda
#define LED_DDR DDRB   // dioda ddr
#define LED_PORT PORTB // dioda port

#define BUTTON PA7
#define BUTTON_DDR DDRA
#define BUTTON_PORT PORTA
#define BUTTON_PIN PINA
#define BUTTON_PRESSED ((!(BUTTON_PIN & _BV(BUTTON))) == 1 ? 0xFF : 0x00)
#define SPI_SS PA3
#define SPI_SS_DDR DDRA
#define SPI_SS_PORT PORTA

void init(void) {
  BUTTON_DDR &= ~_BV(BUTTON); // button input
  BUTTON_PORT |= _BV(BUTTON); // button pull-up
  LED_DDR |= _BV(LED);        // led output
}

void spi_init() {
  // ustaw piny MOSI i SCK jako wyjścia
  DDRA = _BV(DDA4) | _BV(DDA5);
  // ustaw USI w trybie trzyprzewodowym (SPI)
  USICR = _BV(USIWM0);
}

uint8_t spi_transfer(uint8_t data) {
  // załaduj dane do przesłania
  USIDR = data;
  // wyczyść flagę przerwania USI
  USISR = _BV(USIOIF);
  // póki transmisja nie została ukończona, wysyłaj impulsy zegara
  while (!(USISR & _BV(USIOIF))) {
    // wygeneruj pojedyncze zbocze zegarowe
    // zostanie wykonane 16 razy
    USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
  }
  // zwróć otrzymane dane
  return USIDR;
}

int main() {
  init();
  spi_init();
  uint8_t a;
  while (1) {
    a = spi_transfer(BUTTON_PRESSED);
    if (a)
      LED_PORT |= _BV(LED);
    else
      LED_PORT &= ~_BV(LED);
  }
}
