#include "i2c.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

#define READ_NOACK 0x88

ISR(TWI_vect) {}

FILE uart_file;

int main(void) {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  set_sleep_mode(SLEEP_MODE_IDLE);

  // ustaw uart git
  uint8_t data;
  i2cInit();
  while (1) {
    sei();
    sleep_mode();
    cli();
    while (TWSR != READ_NOACK) {
      data = i2cReadAck();
      i2cReadAck();
      i2cReadAck();
      printf("read: 0x%x\r\n", data);
    }
    TWCR |= _BV(TWIE);
  }
}
