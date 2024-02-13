#define __AVR_ATtiny84__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB2        // Dioda
#define LED_DDR DDRB   // dioda ddr
#define LED_PORT PORTB // dioda port

#define BUTTON PB1
#define BUTTON_DDR DDRB
#define BUTTON_PORT PORTB
#define BUTTON_PIN PINB

#define SIZE 128/8

volatile uint8_t tab[SIZE];
volatile uint16_t num = 0;
volatile uint16_t reader = 0;
volatile uint8_t can_read = 0;

__attribute__((always_inline)) static inline void set_value(int place, int value){
  if(value) tab[place>>3] |= 1 << (place & 0x111);
  else tab[place>>3] &= ~ (1 << (place & 0x111));
}

__attribute__((always_inline)) static inline int get_value(int place){
  return (tab[place>>3] & (1 << (place & 0x111)));
} 


// FCPU 1 000 000
// -> 1000
// F_OC1A  = FCPU / (2 * N * (1 + OCR1A))

// void timer0_init(){
//     TCCR0A |= _BV(WGM00);
//     TCCR0B |= _BV(CS01);// |  _BV(CS10);
//     TIMSK0 |= _BV(OCIE0A);
//     OCR0A = 124;
// }


void timer0_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s
  TCCR0A = _BV(WGM01);            // CTC
  TCCR0B = _BV(CS02); // clk_io /1024
  OCR0A = 30;                    // 100Hz
  TIMSK0 |= _BV(OCIE0A);
}

ISR(TIM0_COMPA_vect){
  if(num >= SIZE*8) num = 0;
  if(reader >= SIZE * 8) reader = 0;
  if(num > 100) can_read = 1;
  if(get_value(num) && can_read) LED_PORT |= _BV(LED);
  else LED_PORT &= ~_BV(LED);
  uint8_t val = !(BUTTON_PIN & _BV(BUTTON));
  set_value(num,val);
  reader += can_read;
  num++;
}

int main() {
  LED_DDR  |= _BV(LED);
  LED_PORT &= ~_BV(LED);
    
  BUTTON_DDR &= ~_BV(BUTTON);
  BUTTON_PORT |= _BV(BUTTON);
  memset(tab, 0, SIZE);
  set_sleep_mode(SLEEP_MODE_IDLE);
  sei();
  timer0_init();
  while(1){
    sleep_mode();
  }
}
