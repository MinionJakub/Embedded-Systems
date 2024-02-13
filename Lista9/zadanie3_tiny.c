#define __AVR_ATtiny84__

#include "USI_TWI_Master.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

uint8_t addr = 0x7e;

void i2c_send(uint8_t data) {
  char buf[2];
  buf[0] = addr;
  buf[1] = data;
  USI_TWI_Start_Transceiver_With_Data(buf, 2);
}

int main(void) {
  USI_TWI_Master_Initialise();
  uint8_t data = 0;
  while (1) {
    i2c_send(0x30 + data);
    data++;
    data %= 10;
    _delay_ms(1000);
  }
}
