#include "i2c.h"
void i2cInit() {
  // slave mode
  TWAR = 0x7e;
  // ustaw bitrate
  // 8MHz / (16+2*TWBR*1) ~= 100kHz
  TWBR = 32;
  // uruchom TWI
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWIE);
}
void i2cWaitForComplete() {
  // czekaj na flagę TWINT
  while (!(TWCR & _BV(TWINT)))
    ;
}
uint8_t i2cReadAck() {
  // odczytaj dane, wyślij ACK
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
  i2cWaitForComplete();
  return TWDR;
}
uint8_t i2cReadNoAck() {
  // odczytaj dane, wyślij NOACK
  TWCR = _BV(TWINT) | _BV(TWEN);
  i2cWaitForComplete();
  return TWDR;
}
