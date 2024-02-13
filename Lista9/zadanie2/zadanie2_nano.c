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
#define SLAVE_SELECT SPI_PORT &= ~(1 << SPI_SS)
#define SLAVE_DESELECT SPI_PORT |= (1 << SPI_SS)

#define LED PC4        // Dioda
#define LED_DDR DDRC   // dioda ddr
#define LED_PORT PORTC // dioda port

#define BUTTON PC5
#define BUTTON_DDR DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN PINC
#define BUTTON_PRESSED (!(BUTTON_PIN & _BV(BUTTON)))

void init(void) {
  BUTTON_DDR &= ~_BV(BUTTON); // button input
  BUTTON_PORT |= _BV(BUTTON); // button pull-up
  LED_DDR |= _BV(LED);        // led output
}

volatile uint8_t dataT = 0, dataR = 0, cnt = 0, slave_transmit = 0;

ISR(TIMER2_COMPA_vect) {
  if (cnt < 8) {
    dataT = (dataT << 1) | BUTTON_PRESSED;
    if (dataR & 0x80)
      LED_PORT |= _BV(LED);
    else
      LED_PORT &= ~_BV(LED);
    dataR <<= 1;
    cnt++;
  }
}

// SS held low, master expects data or transfer completed
ISR(SPI_STC_vect) {
  LED_PORT |= _BV(LED);
  dataR = SPDR;
  SPDR = dataT;
  cnt = 0;
  SPCR &= ~_BV(SPIE);
}

void timer_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s
  TCCR2A = _BV(WGM21);                        // CTC
  TCCR2B = _BV(CS22) | _BV(CS20) | _BV(CS21); // clk_io /1024
  OCR2A = 95;                                 // 100Hz
  TIMSK2 |= _BV(OCIE2A);
}

void spi_init() {
  // mcu spi stuff set as input, miso as output
  uint8_t a;
  SPI_PORT |= _BV(SPI_SS);
  SPI_DDR |= _BV(SPI_MISO);
  SPCR = _BV(SPE);
  SPCR &= ~_BV(MSTR);
  a = SPSR;
  a = SPDR;
}

int main(void) {
  init();
  spi_init();
  timer_init();
  if (SPI_PORT & _BV(SPI_SS))
    LED_PORT |= _BV(LED);
  // ustaw tryb uśpienia na tryb bezczynności
  set_sleep_mode(SLEEP_MODE_IDLE);
  // odmaskuj przerwania
  sei();
  // program testowy
  while (1) {
    sleep_mode();
    if (cnt >= 8) {
      SPCR |= _BV(SPIE);
    }
  }
}