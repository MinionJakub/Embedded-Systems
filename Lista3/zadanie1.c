#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BUZZ PB5
#define BUZZ_DDR DDRB
#define BUZZ_PORT PORTB

void my_delay_us(uint16_t value){
    while(value){
        _delay_us(1);
        value--;
    }
}

static inline void TONE(uint16_t step, uint16_t delay){
    // uint16_t step2 = step;
    if(step == 0){
        BUZZ_PORT &= ~_BV(BUZZ);
        _delay_us(2*step);
        return;
    }
    for (uint16_t i = 0; i < (uint32_t)1000 * (delay) / (step) / 2; i++){
        BUZZ_PORT |= _BV(BUZZ);
        my_delay_us(step);
        BUZZ_PORT &= ~_BV(BUZZ);
        my_delay_us(step);
    }
}

// _delay_us()
#define C 26
#define D 29
#define E 33
#define F 35
#define G 39

#define P 0

#define f 100
#define h 50
#define q 25
// #define e 125

static const uint8_t notes[]  PROGMEM = {
    C,D,E,D,C,E,D,E,F,E,D,P,
    D,E,F,E,D,F,C,D,E,D,C,P,
    C,D,E,D,C,E,D,E,F,E,D,P,
    D,E,F,E,D,F,E,D,G,F,E,P
};
static const uint8_t length[] PROGMEM = {
    q,q,q,q,h,h,q,q,q,q,h,f
};

int main(){
    BUZZ_DDR |= _BV(BUZZ);
    uint16_t value;
    uint16_t len;
    while(1){
        for(int8_t i = 0; i < 48; i++){
            value = (uint16_t)pgm_read_byte(&notes[i]) * 10;
            len = (uint16_t)pgm_read_byte(&length[i%12]) * 10;
            TONE(value,len);
        }
    }
}


