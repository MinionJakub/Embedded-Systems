#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


#define LED PD3
#define LED_DDR DDRD
#define LED_PORT PORTD

#define BUTTON PD2
#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD
#define BUTTON_PIN PIND


static volatile uint8_t button_pressed = 0;


// inicjalizacja SPI
void spi_init()
{
    DDRB |= _BV(DDB4);
    SPCR = _BV(SPE) | _BV(SPR1);
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data)
{
    // rozpocznij transmisję
    SPDR = data;
    // czekaj na ukończenie transmisji
    while (!(SPSR & _BV(SPIF)));
    // wyczyść flagę przerwania
    SPSR |= _BV(SPIF);

    return SPDR;
}

void interrupt_init()
{
    EICRA |= _BV(ISC00);
    EIMSK |= _BV(INT0);
}
void init()
{
    BUTTON_DDR &= ~ _BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR |= _BV(LED);
}


ISR(INT0_vect) {
    if(bit_is_set(BUTTON_PIN, BUTTON))
        button_pressed = 0;
    else
        button_pressed = 1;
}


int main()
{
    interrupt_init();
    sei();
    init();
    spi_init();

    while(1) {
        uint8_t status = spi_transfer(button_pressed);
        if(status)
        {
            LED_PORT |= _BV(LED);
        } else 
        {
            LED_PORT &= ~_BV(LED);
        }
    }
}

