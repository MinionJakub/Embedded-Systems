#include <avr/io.h>
#include <util/delay.h>

void timer1_init(){
  // ustaw tryb licznika
  OCR1A = 421;
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(ICES1) | _BV(CS10);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB2);
}

int main(){
  timer1_init();
  uint16_t x = ICR1;
  OCR1B = OCR1A / 2;
  DDRB |= _BV(PB5);
  while(1) {
    PORTB &= ~_BV(PB5);
    if (x != ICR1) {
      PORTB |= _BV(PB5);
      x = ICR1;
    }
    DDRB |= _BV(PB2);
    _delay_us(600);
    DDRB &= ~_BV(PB2);
    _delay_us(600);
  }
}