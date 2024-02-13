#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

int hd44780_transmit(char data, FILE *stream)
{
    LCD_WriteData(data);
    return 0;
}

FILE hd44780_file;

char line[16];

void change_lines()
{
    LCD_Clear();
    int i = 0;
    
    while(i < 16 && line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
    {
        LCD_GoTo(i, 0);
        printf("%c", line[i++]);
    }
    
}

int main()
{
    // skonfiguruj wyświetlacz
    uart_init();
    LCD_Initialize();
    LCD_Clear();
    // skonfiguruj strumienie wyjściowe
    fdev_setup_stream(&hd44780_file, hd44780_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &hd44780_file;
    // program testowy
    char character_read;
    uint8_t cursor_pos = 0;
    while (1)
    {
        LCD_GoTo(cursor_pos, 1);
        // printf("^");

        scanf("%c", &character_read);
        line[cursor_pos] = character_read;

        if(character_read == '\n' || character_read == '\r')
        {
            cursor_pos = 0;
            change_lines();
        } else 
        {
            LCD_GoTo(cursor_pos, 1);
            printf("%c", character_read);
            cursor_pos ++;
            if(cursor_pos == 16)
            {
                cursor_pos = 0;
                change_lines();
            }
        }
    }
}
