#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PD2
#define LED_DDR DDRD
#define LED_PORT PORTD



int main(){
    LED_DDR  |= _BV(LED);
    LED_PORT |= _BV(LED);
    // LED_PORT |= 
    while(1);
}