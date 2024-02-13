#define __AVR_ATmega328P__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PD2
#define LED_PORT PORTD
#define LED_DDR DDRD
#define BUTTON PD5
#define BUTTON_PORT PORTD
#define BUTTON_DDR DDRD
#define size 1000

uint8_t tab[size];
volatile uint16_t num = 0;

void timer0_init(){
    TCCR0A |= _BV(WGM01);
    TCCR0B |= _BV(CS01) | _BV(CS00);
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 124;
}

ISR(TIMER0_COMPA_vect){
    if(num >= 1000) num -= 1000;
    if(tab[num]) LED_PORT |= _BV(LED);
    else LED_PORT &= ~_BV(LED);
    if(!(PIND & _BV(BUTTON))) tab[num] = 1;
    else tab[num] = 0;
    num++;
}

int main(){
    LED_DDR  |= _BV(LED);
    LED_PORT &= ~_BV(LED);
    
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);
    
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();
    timer0_init();
    while(1) sleep_mode();
}
