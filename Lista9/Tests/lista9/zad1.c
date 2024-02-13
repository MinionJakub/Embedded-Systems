#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PB2
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BUTTON PA7
#define BUTTON_DDR DDRA
#define BUTTON_PORT PORTA
#define BUTTON_PIN PINA

volatile uint8_t circular_buffer[126];


static volatile uint16_t current_time = 0;
static volatile uint8_t button_pressed = 1;

void clock_init()
{

    TCCR0A |= _BV(WGM01);
    TCCR0B |= _BV(CS01) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 124;
}
void interrupt_init()
{
    MCUCR |= _BV(ISC00);
    GIMSK |=  _BV(PCIE0);
    PCMSK0 |= _BV(PCINT0);
}
void init()
{
    BUTTON_DDR &= ~ _BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR |= _BV(LED);
}

ISR(PCINT0_vect) {

    if(bit_is_set(BUTTON_PIN, BUTTON))
        button_pressed = 0;
    else
        button_pressed = 1;
}
ISR(TIM0_COMPA_vect) {

    if(button_pressed)
        circular_buffer[current_time >> 3] |= (1 << (7 - (current_time & 7)));


    if(current_time++ == 1001) current_time = 0;

    light(current_time);
}


void light(uint16_t ind)
{
    uint16_t ind_tab = ind >> 3;
    uint16_t ind_bite = 7 - (ind & 7);

    if(bit_is_set(circular_buffer[ind_tab], ind_bite))
    {
        LED_PORT |= _BV(LED);
        circular_buffer[ind_tab] &= ~(1 << ind_bite);
    } else 
    {
        LED_PORT &= ~_BV(LED);
    }
}


int main()
{
    clock_init();
    interrupt_init();
    sei();
    init();


    set_sleep_mode(SLEEP_MODE_IDLE);


    while(1) 
    {
        sleep_mode();
    }
}
