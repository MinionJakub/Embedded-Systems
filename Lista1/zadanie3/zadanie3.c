#include <avr/io.h>
#include <util/delay.h>


#define LED_DDR DDRD
#define LED_PORT PORTD

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

int main(){
    LED_DDR = 0xff;
    LED_PORT = 0xff;

    while(1){
        for(int8_t i = 0; i < 10; i++){
            LED_PORT = digits[i];
            _delay_ms(1000);
        }
    }
}
