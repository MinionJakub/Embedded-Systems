#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PD2
#define LED_DDR DDRD
#define LED_PORT PORTD
#define true 1
#define false 0
int main()
{   
    LED_DDR |= _BV(LED);
    LED_PORT |= _BV(LED);
    while(true){
    }
}