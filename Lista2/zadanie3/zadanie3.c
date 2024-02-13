#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PD0 //Dioda
#define LED_DDR DDRD    //dioda ddr
#define LED_PORT PORTD  //dioda port

#define BUTTON PB0
#define BUTTON2 PB1
#define BUTTON3 PB2
#define BUTTON_DDR DDRB
#define BUTTON_PORT PORTB

uint8_t graycodes[] = {0, 1, 3, 2, 6, 7, 5, 4, 12, 
13, 15, 14, 10, 11, 9, 8, 24, 25, 27, 26, 30, 31, 
29, 28, 20, 21, 23, 22, 18, 19, 17, 16};

int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    LED_DDR = 0xff;
    LED_PORT = 0;

    BUTTON_DDR &= ~(_BV(BUTTON) | _BV(BUTTON2) | _BV(BUTTON3));
    BUTTON_PORT |= _BV(BUTTON) | _BV(BUTTON2) | _BV(BUTTON3);
    uint8_t counter = 0;
    while(1){
        if(!(PINB&_BV(BUTTON))){
            counter = 0;
        }
        else if(!(PINB&_BV(BUTTON2))){
            if(counter == 31) counter = 0;
            else counter++;
        }
        else if(!(PINB&_BV(BUTTON3))){
            if(counter == 0) counter = 31;
            else counter--;
        }
        LED_PORT = graycodes[counter];
        _delay_ms(100);
    }
}