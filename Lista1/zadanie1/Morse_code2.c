#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define BUFFER_SIZE 64

#define LED PB5    			        // dioda
#define LED_DDR DDRB			    // dioda ddr
#define LED_PORT PORTB			    // dioda port

void signal_letter(int8_t letter) {
    int8_t n = (letter | 224) >> 5; // odczytujemy długość litery
    LED_DDR |= _BV(LED);
    for (int8_t i = 0; i < n; i++) {
        LED_DDR |= _BV(LED);

        if (letter & (32 >> i))
            _delay_ms(1000);
        else _delay_ms(2000);

        LED_DDR &= ~_BV(LED);
    }
}

char toUpper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    } else {
        return c;
    }
}

int alphabetPosition(char c) {
    c = toUpper(c);

    if (c >= 'A' && c <= 'Z') {
        int position = c - 'A';
        return position;
    } else {
        return -1;
    }
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

int main()
{
    uint8_t morse_code[] = {
            80, 142, 138, 108, 48, 154, 100, 158, 88, 144, 104, 150, 64, 72, 96, 146, 132, 116, 124, 32, 120, 156, 112, 140, 136, 134
    };

    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // program testowy
    char buffer[BUFFER_SIZE];
    while(1) {
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
        printf("write text to encode: \r\n");
        scanf("%s", &buffer);
        printf("Odczytano: %s\r\n", buffer);
        for (int8_t i = 0; i < BUFFER_SIZE; ++i) {
            if (i > 6 || !buffer[i]) break;
            int8_t pos = alphabetPosition(buffer[i]);
            if (pos == -1) {
                printf("Bad letter found. Skipping...\r\n");
            }
            else {
                uint8_t code = morse_code[pos];
                uint8_t n = (code >> 5) & 7; // odczytujemy długość litery
                printf("n: %d \r\n", n);
                LED_DDR |= _BV(LED);
                for (int8_t i = 0; i < n; i++) {
                    LED_PORT |= _BV(LED);
                    printf("code: %"PRIu8"\r\n", code);
                    printf(" %"PRIu8, code & (32 >> i));
                    if (code & (32 >> i))
                        _delay_ms(500);
                    else
                        _delay_ms(1000);

                    LED_PORT &= ~_BV(LED);

                    if (code & (32 >> i))
                        _delay_ms(500);
                    else
                        _delay_ms(1000);
                }
            }
            printf("\n\n\n\r");
        }

    }
}
