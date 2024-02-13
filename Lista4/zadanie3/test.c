#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define GREEN PB1
#define RED PB2
#define BLUE PB3
#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_ARRAY (_BV(GREEN) | _BV(RED) | _BV(RED))

int main(){
    LED_DDR  |= LED_ARRAY;
    LED_PORT &= ~(LED_ARRAY);
    // LED_PORT |= 
    while(1);
}