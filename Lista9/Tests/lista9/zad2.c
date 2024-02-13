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

static volatile uint8_t button_pressed = 0;


void spi_init()
{
    // ustaw piny MOSI i SCK jako wyjścia
    DDRA |= _BV(DDA4) | _BV(DDA5);
    // ustaw USI w trybie trzyprzewodowym (SPI)
    USICR = _BV(USIWM0);
}

uint8_t spi_transfer(uint8_t data)
{

    // załaduj dane do przesłania
    USIDR = data;
    // wyczyść flagę przerwania USI
    USISR = _BV(USIOIF);

    
    // póki transmisja nie została ukończona, wysyłaj impulsy zegara
    while (!(USISR & _BV(USIOIF))) {
        // wygeneruj pojedyncze zbocze zegarowe
        // zostanie wykonane 16 razy
        USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
    }
    
    // zwróć otrzymane dane
    return USIDR;
}

void interrupt_init()
{
    MCUCR |= _BV(ISC00);
    GIMSK |=  _BV(PCIE0);
    PCMSK0 |= _BV(PCINT7);
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
int main()
{
    interrupt_init();
    sei();
    spi_init();
    init();
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

