#include <avr/io.h>
#include <util/delay.h>

#define BUZZ PB5
#define BUZZ_DDR DDRB
#define BUZZ_PORT PORTB

#define C 26
#define D 29
#define E 33
#define F 35
#define G 39



static inline void TONE(uint16_t step, uint16_t delay){
    // uint16_t step2 = step;
    if(step == 0){
        BUZZ_PORT &= ~_BV(BUZZ);
        _delay_us(4*step);
        return;
    }
    for (uint16_t i = 0; i < (uint32_t)1000 * (delay) / (step) / 2; i++){
        BUZZ_PORT |= _BV(BUZZ);
        _delay_us(step);
        BUZZ_PORT &= ~_BV(BUZZ);
        _delay_us(step);
    }
}

int main() {
  BUZZ_DDR |= _BV(BUZZ);
  while (1) {
    TONE(0, 1000);
    // TONE(C*10, 500);
    // TONE(D*10,500);
    // TONE(E*10,500);
    // TONE(F*10,500);
    // TONE(G*10,500);
    // TONE(C*10, 250);
    // TONE(D*10,250);
    // TONE(E*10,250);
    // TONE(F*10,250);
    // TONE(G*10,250);
  }
}
