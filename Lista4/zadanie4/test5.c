#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PB5 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port

void timer1_init()
{ 
  ICR1 = 210;
  TCCR1A = _BV(COM1A1) | _BV(WGM11); //| _BV(WGM10);
  TCCR1B = _BV(WGM13) |_BV(WGM12) | _BV(CS10);
  DDRB |= _BV(PB1);
}

int main(){
  LED_DDR |= _BV(LED);
  timer1_init();
  OCR1A = ICR1/2;
  while(1){
    int8_t value = PINB & _BV(PB0);
    if(value) PORTB |= _BV(LED);
    else PORTB &= ~_BV(LED);
    _delay_ms(6);
    _delay_us(600);
    ICR1 = 0;
    _delay_ms(100);
    ICR1 = 210;
  }
}