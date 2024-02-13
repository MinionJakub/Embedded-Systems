#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)

#define FIRST_VERSION 0
#define SECOND_VERSION (!FIRST_VERSION)

void uart_init(){
    //ustaw baudrate
    UBRR0 = UBRR_VALUE;
    //wyczyszczenie rejestrów UCSR0A - odpowiada za informacje dotyczące przesyłu danych
    UCSR0A = 0;
    //uruchomienie odbiornika i nadajnika
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    //ustawienie formatu 8-bitowego
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    //uruchomienie interaptów
    UCSR0B |= _BV(RXCIE0); //| _BV(TXCIE0);
}

#if(FIRST_VERSION)

volatile static char data;
// volatile static uint8_t check = 1;

//interrupt odpowiedzialny za odczyt danych
ISR(USART_RX_vect){
    //wylaczenie czytania by nadać ten sam znak
    UCSR0B &= ~_BV(RXCIE0);
    //zczytanie danych
    data = UDR0;
    //nadanie danych
    UDR0 = data;
}

//interrupt odpowiedzialny za zakonczenie przesylu danych
ISR(USART_TX_vect){
    //nadanie end_of_line po karecie
    if(data == '\r'){
        UDR0 = '\n';
        data = '\n';
    }
    //przywrocenie czytania
    UCSR0B |= _BV(RXCIE0);
}

#endif //FIRST_VERSION

#if(SECOND_VERSION)

volatile static char data;

ISR(USART_RX_vect){
    data = UDR0;
    if(data == '\r') UDR0 = '\n';
    UDR0 = data;
}
//ISR(USART_TX_vect){}

#endif //SECOND_VERSION

int main(){
    uart_init();
    sei();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while(1){
        sleep_mode();
    }
}