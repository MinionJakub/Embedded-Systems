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

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

static inline char decode(int8_t len, int8_t sign){
    if(len == 1){
        if(sign&1) return 'T';
        return 'E';
    }
    else if(len == 2){
        if(sign&1){
            if(sign&2) return 'M';
            else return 'N';
        }
        else{
            if(sign&2) return 'A';
            else return 'I';
        }
    }
    else if(len == 3){
        if(sign&1){
            if(sign&2){
                if(sign&4) return 'O';
                else return 'G';
            }
            else{
                if(sign&4) return 'K';
                else return 'D';
            }
        }
        else{
            if(sign&2){
                if(sign&4) return 'W';
                else return 'R';
            }
            else{
                if(sign&4) return 'U';
                else return 'S';
            }
        }
    }
    else if(len == 4){
        if(sign == 8+4) return 'Z';
        if(sign == 8+2+1) return 'Y';
        if(sign == 8+2) return 'C';
        if(sign == 8+1) return 'X';
        if(sign == 8) return 'B';
        if(sign == 7) return 'J';
        if(sign == 6) return 'P';
        if(sign == 4) return 'L';
        if(sign == 2) return 'F';
        if(sign == 1) return 'V';
        if(sign == 0) return 'H';
        return '?';
    }
    return '?';
}

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

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR  |= _BV(LED);

    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("test\r\n");
    int8_t long_sign = 0;
    int8_t count_time = 0;
    int8_t decoding = 0;
    int8_t first_time = 0;
    int8_t reading;
    int8_t readed_signs = 0;
    int8_t signs = 0;
    char val;
    while(1){
        reading = !(PIND&(_BV(BUTTON))); //read_state();
        if(decoding && !reading){
            count_time = 0;
            first_time = 1;
            decoding = 0;
        }
        if(!reading ){
            // count_time 
            LED_PORT &= ~_BV(LED);
            if(first_time){
                count_time = 0;
                first_time = 0;
                if(readed_signs < 4){
                signs |= (long_sign*(_BV(readed_signs)));
                readed_signs ++;}
            }
            else count_time++;
            if(count_time == 8){
                val = decode(readed_signs,signs);
                if(readed_signs != 0)printf("%c:\tlen:%"PRId8"\tval:%"PRId8"\r\n",val,readed_signs,signs);
                readed_signs = 0;
                signs = 0;
            }
            if(count_time == 0xff) count_time --;
        }
        else if(reading){
            if(decoding == 0) first_time = 1;
            decoding = 1;
            if(first_time){
                if(count_time != 8){
                    LED_PORT &= ~_BV(LED);
                    long_sign = 0;
                }
                count_time = 0;
                first_time = 0;
            }
            else count_time++;
            if(count_time == 8)
            {
                long_sign = 1;
                LED_PORT |= _BV(LED);
            }
            if (count_time == 0xff) count_time--;
        }
        _delay_ms(100);
    }
}