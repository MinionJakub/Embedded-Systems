#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PB5 //Dioda
#define LED_DDR DDRB    //dioda ddr
#define LED_PORT PORTB  //dioda port

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define BUFFER_SIZE 64

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

static inline void diod_light(uint8_t value){
    LED_DDR  |= _BV(LED);
    if(value){
        LED_PORT |= _BV(LED);
        _delay_ms(1000);
        LED_PORT &= ~_BV(LED);
        _delay_ms(1000);    
    }
    else{
        LED_PORT |= _BV(LED);
        _delay_ms(500);
        LED_PORT &= ~_BV(LED);
        _delay_ms(500);    
    }
}

static uint8_t morse_code[] = {
    72,144,148,112,32,132,120,128,64,142,116,136,88,80,124,140,154,104,96,
    48,100,130,108,146,150,152
};

static inline char toUpper(char value){
    if(value >= 'a' && value <= 'z')
        return value - 'a' + 'A';
    return value;
}

static inline uint8_t read_letter(char value){
    if(value >= 'A' && value <= 'Z')
        return morse_code[value-'A'];
    return -1;
}

static inline void print_letter(uint8_t value){
    if (value == -1) return;
    uint8_t length = ((value)&224)>>5;
    uint8_t scan = 1<<4;
    for(uint8_t i = 0; i < length; i++){
        printf("code: %"PRIu8"\r\n",value);
        diod_light((value&scan)>>(4-i));
        scan>>=1;
    }
}

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    char buffer[BUFFER_SIZE];
    while(1){
        memset(buffer,0,BUFFER_SIZE);
        printf("Write text to encode: \r\n");
        scanf("%s",buffer);
        printf("Readed: %s\r\n",buffer);
        for(int8_t i = 0; i < BUFFER_SIZE; i++){
            if(i > 6 || !buffer[i]) break;
            print_letter(read_letter(toUpper(buffer[i])));
            printf("\n\n\n\r");
        }
    }
}