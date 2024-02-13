#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PB5 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port

#define BUTTON PD2
#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD

#define CIRCLE_SIZE 256/8

uint8_t cirular_buffer[CIRCLE_SIZE];

static inline uint8_t get_place(uint8_t value){
    return (value >> 3);
}

static inline uint8_t get_bit(uint8_t value){
    return (value&7);
}

static inline void light(uint8_t value){
    if(value)
        LED_PORT |= _BV(LED);
    else
        LED_PORT &= ~_BV(LED);
}

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR  |= _BV(LED);
    uint8_t crossed_100 = 0;
    uint8_t place;
    uint8_t bit;
    uint8_t writer = 0;
    uint8_t reader = 0;
    while(1){
        if(reader == 255)
            reader = 0;
        if(writer == 255)
            writer = 0;
        if(!crossed_100 && writer == 100)
            crossed_100 = 1;
        

        place = get_place(writer);
        bit = get_bit(writer);
        if(!(PIND&_BV(PD2))){
            cirular_buffer[place] |= _BV(bit);
        }
        else{
            cirular_buffer[place] &= ~_BV(bit);
        }
        writer++;
        if(crossed_100){
            place = get_place(reader);
            bit = get_bit(reader);
            light(cirular_buffer[place]&_BV(bit));
            reader++;
        }
        _delay_ms(10);
    }
}