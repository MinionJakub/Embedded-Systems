#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PB5 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port

#define BUTTON PD2
#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD

int main(){
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR  |= _BV(LED);
    while(1){
        if(PIND&_BV(PD2))
            LED_PORT |= _BV(LED);
        else
            LED_PORT &= ~_BV(LED);
    }
}