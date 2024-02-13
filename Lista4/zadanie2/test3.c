#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PB1 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port

int main(){
    LED_DDR  |= _BV(LED);
    LED_PORT |= _BV(LED);
    while(1);
}