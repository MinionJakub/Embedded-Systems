#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem

#define IR_DETECTOR PB0
#define IR_DETECTOR_DDR DDRB
#define IR_DETECTOR_PORT PORTB
#define IR_DETECTOR_PIN PINB


#define min(x, y) (x < y ? x : y)

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

void timer_init()
{
    TCCR1A |= _BV(COM1A1); 

    TCCR1A |= _BV(WGM11);
    TCCR1B |= _BV(WGM12) | _BV(WGM13);

    ICR1 = 210;

    TCCR1B |= _BV(CS10);

    //DDRB |= _BV(PB1);
}

void send_signal(){

    for(int i = 0; i<6; i++){
        DDRB |= _BV(PB1);
        _delay_us(600);
        DDRB &= ~_BV(PB1);
        
        if(i < 5)
            _delay_us(600);
    }

    DDRB &= ~_BV(PB1);
}

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);

    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    
    timer_init();

    // button is input, toggle pull-up resistor
    IR_DETECTOR_DDR &= ~_BV(IR_DETECTOR); // unnecessary, but guarantees input
    IR_DETECTOR_PORT |= _BV(IR_DETECTOR);

    while(1){

        send_signal();

        if(bit_is_clear(IR_DETECTOR_PIN, PB2)){
            printf("p\r\n");
        }
        else{
            printf("n\r\n");
        }

        _delay_ms(100);
    }
}