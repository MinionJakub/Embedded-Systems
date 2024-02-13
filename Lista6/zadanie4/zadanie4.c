#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>

#define LED_DDR DDRD
#define LED_PORT PORTD

#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_MOSI PB3
#define SPI_SCK PB5
#define SPI_LA PB1
#define SPI_OE PB2

#define A 1
#define B 2
#define C 4
#define D 8
#define E 16
#define F 32
#define G 64
#define DP 128

static const uint8_t numbers[] ={
    G,                          //0
    A|D|E|F|G,                  //1
    C|F,                        //2
    E|F,                        //3
    A|D|E,                      //4
    B|E,                        //5
    B,                          //6
    D|E|G,                      //7
    0,                          //8
    E                           //9
};

void timer_init(){
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS22) | _BV(CS21) |_BV(CS20);
    OCR2A = 195;
    TIMSK2 |= _BV(OCIE2A);
}

volatile uint8_t timer = 0;
volatile uint8_t index = 0;

ISR(TIMER2_COMPA_vect){
    if(++timer >= 100){
        timer = 0;
        if(++index >= 10) index = 0;
    }
}

ISR(SPI_STC_vect){}

void spi_init(){
    //ustaw  MOSI, SCK, i ~SS jako wyjścia
    SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_LA) | _BV(SPI_OE);
    SPI_PORT &= ~_BV(SPI_OE);
    //ustaw SPI na tryb master oraz na 250 kHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPIE);
}

void load_digit(uint8_t digit){
    SPI_PORT |= _BV(SPI_LA);
    SPDR = digit|DP;
    sleep_mode();
    // while(!(SPSR & _BV(SPIF)));// sleep_mode();
    // SPSR |= _BV(SPIF);
    SPI_PORT &= ~_BV(SPI_LA);
}



int main(){
    timer_init();
    spi_init();
    sei();
    set_sleep_mode(SLEEP_MODE_IDLE);
    load_digit(~numbers[index]);
    while(1){
        load_digit(~numbers[index]);
        sleep_mode();
    }
}