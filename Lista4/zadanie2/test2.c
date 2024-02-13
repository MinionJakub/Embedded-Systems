#include <avr/io.h>
#include <util/delay.h>
void timer1_init()
{
  ICR1 = 15624;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS00);// | _BV(CS10) | _BV(CS12);
  DDRB |= _BV(PB1);
}

int main()
{
  // uruchom licznik
  timer1_init();
  // ustaw wypełnienie 50%
  // OCR1A = (870)*(870);
  OCR1A = ICR1/2;
  while(1){
    // _delay_ms(2000);
    // OCR1A = 0;
  };
}
