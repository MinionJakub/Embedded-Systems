#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)

#define BUFFER_SIZE 256

volatile char sending[BUFFER_SIZE];
volatile char receiving[BUFFER_SIZE];
volatile uint8_t current_sending = 0;
volatile uint8_t last_sending = 0;
volatile uint8_t current_receiving = 0;
volatile uint8_t last_receiving = 0;

void uart_init(){
    // ustawienie baudrate
    UBRR0 = UBRR_VALUE;
    // wyczyszczenie rejestru UCSR0A
    UCSR0A = 0;
    // uruchomienie czytania i transmiji
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustawienie formatu 8-bitowych znakow
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    // uruchomienie interruptów
    UCSR0B |= _BV(RXCIE0);
}

//transmisja jednego znaku
int uart_transmit(char data, FILE *stream){
    while(current_sending == (last_sending + 1));
    sending[last_sending++] = data;
    // last_sending %= BUFFER_SIZE;
    //uruchomienie interrupta
    UCSR0B |= _BV(UDRIE0);
    return 0;
}

//odczyt jednego znaku
int uart_receive(FILE *stream){
    while(current_receiving == last_receiving);
    char return_val = receiving[current_receiving++];
    // current_receiving %= BUFFER_SIZE;
    //uruchomienie interrupta
    UCSR0B |= _BV(UDRIE0);
    return return_val;
}

ISR(USART_RX_vect){
    receiving[last_receiving++] = UDR0;
}

ISR(USART_UDRE_vect){
    UDR0 = sending[current_sending++];
    if(current_sending == last_sending) UCSR0B &= ~_BV(UDRIE0);
}

FILE uart_file;

int main(){
    uart_init();
    sei();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    volatile char sign;
    while(1){
        scanf("%c",&sign);
        printf("Odczytano :\t %c\r\n",sign);
    }
}