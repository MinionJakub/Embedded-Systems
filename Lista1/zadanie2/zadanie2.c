#include <avr/io.h>
#include <util/delay.h>

#define LED PD0
#define LED_DDR DDRD
#define LED_PORT PORTD


int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    LED_DDR = 0xff;
    LED_PORT = 0;
    while (1) {
        for(int8_t i = 0; i <= 5; i++){
            if(i > 0){
                LED_PORT &= ~_BV(i-1);
            }
            if(i > 1){
                LED_PORT &= ~_BV(i-2);
            }
            LED_PORT |= _BV(i);
            LED_PORT |= _BV(i+1);
            LED_PORT |= _BV(i+2);
            _delay_ms(100);
        }
        for(int8_t i = 7;i >= 2; i--){
            if(i < 7){
                LED_PORT &= ~_BV(i+1);
            }
            if(i < 6){
                LED_PORT &= ~_BV(i+2);
            }

            LED_PORT |= _BV(i);
            LED_PORT |= _BV(i-1);
            LED_PORT |= _BV(i-2);
            _delay_ms(100);
        }
  }
}