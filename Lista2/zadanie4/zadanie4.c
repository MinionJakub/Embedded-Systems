#include <avr/io.h>
#include <util/delay.h>


#define LED_DDR DDRD
#define LED_PORT PORTD

#define T_DDR DDRC
#define T_PORT PORTC

int8_t digits [] ={
192 , //0
249 , //1
164 , //2
176 , //3
153 , //4
146 , //5
130 , //6
248 , //7
128 , //8
144   //9
};

void mig(int8_t dioda1, int8_t dioda2){
    T_PORT &= ~(_BV(PC0) | _BV(PC1));
    T_PORT |= _BV(PC0);
    LED_PORT = digits[dioda1];
    _delay_ms(10);
    T_PORT &= ~_BV(PC0);
    LED_PORT = digits[dioda2];
    T_PORT |= _BV(PC1);
    _delay_ms(10);
}

int main(){
    // UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    LED_DDR = 0xff;
    LED_PORT = 0xff;
    T_DDR |= _BV(PC0) | _BV(PC1);
    T_PORT &= ~(_BV(PC0) | _BV(PC1));
    int8_t counter_1 = 0;
    int8_t counter_2 = 0;
    while(1){
        if(counter_2 == 10){
            counter_1++;
            counter_2 = 0;
        }
        if(counter_1 == 6) counter_1 = 0;
        for(int8_t i = 0; i < 50; i++)
            mig(counter_2,counter_1);
        counter_2++;
    }
}