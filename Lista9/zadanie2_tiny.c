#define __AVR_ATtiny84__

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
#define BUTTON_PRESSED (!(BUTTON_PIN & _BV(BUTTON)))
#define SPI_SS PA3
#define SPI_SS_DDR DDRA
#define SPI_SS_PORT PORTA
#define SLAVE_DESELECT SPI_SS_PORT |= _BV(SPI_SS)
#define SLAVE_SELECT SPI_SS_PORT &= ~_BV(SPI_SS)

void init(void) {
  BUTTON_DDR &= ~_BV(BUTTON); // button input
  BUTTON_PORT |= _BV(BUTTON); // button pull-up
  LED_DDR |= _BV(LED);        // led output
  SPI_SS_DDR |= _BV(SPI_SS);
}

volatile uint8_t DATA_TRANSFER = 0, DATA_READ = 0, COUNTER = 0;

ISR(TIM0_COMPA_vect) {
  if (COUNTER < 8) {
    DATA_TRANSFER = (DATA_TRANSFER << 1) | BUTTON_PRESSED;
    if (DATA_READ & 0x80)
      LED_PORT |= _BV(LED);
    else
      LED_PORT &= ~_BV(LED);
    DATA_READ <<= 1;
    COUNTER++;
  }
}

void timer_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s (na oko)
  TCCR0A = _BV(WGM01); // CTC
  TCCR0B = _BV(CS02);  // clk_io /256
  OCR0A = 30;          // 100Hz
  TIMSK0 |= _BV(OCIE0A);
}

void spi_init() {
  // ustaw piny MOSI i SCK jako wyjścia
  DDRA = _BV(DDA4) | _BV(DDA5);
  // ustaw USI w trybie trzyprzewodowym (SPI)
  USICR = _BV(USIWM0);
  SLAVE_DESELECT;
}

void spi_transfer(void) {
  SLAVE_SELECT;
  _delay_us(10);
  // załaduj dane do przesłania
  USIDR = DATA_TRANSFER;
  // wyczyść flagę przerwania USI
  USISR = _BV(USIOIF);
  // póki transmisja nie została ukończona, wysyłaj impulsy zegara
  while (!(USISR & _BV(USIOIF))) {
    // wygeneruj pojedyncze zbocze zegarowe
    // zostanie wykonane 16 razy
    USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
  }
  SLAVE_DESELECT;
  // zwróć otrzymane dane
  DATA_READ = USIDR;
}

int main() {
  init();
  spi_init();
  timer_init();
  // ustaw tryb uśpienia na tryb bezczynności
  set_sleep_mode(SLEEP_MODE_IDLE);
  // odmaskuj przerwania
  sei();
  // program testowy
  while (1) {
    sleep_mode();
    if (COUNTER >= 8) {
      spi_transfer();
      COUNTER = 0;
    }
  }
}
