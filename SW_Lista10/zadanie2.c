#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

int hd44780_transmit(char data, FILE *stream)
{
    LCD_WriteData(data);
    return 0;
}

FILE hd44780_file;

char loading_char[5] = {
    0xf0,
    0xf8,
    0xfc,
    0xfe,
    0xff
};

void LCD_CGRAM_SET()
{
    for(int cgram_addr = 0; cgram_addr < 5; cgram_addr++)
    {
        for(int level = 0; level < 8; level++)
        {
            LCD_WriteCommand(HD44780_CGRAM_SET | (cgram_addr << 3) | level);
            LCD_WriteData(loading_char[cgram_addr]);
        }
    }
    

}
int main()
{
    // skonfiguruj wyświetlacz
    LCD_Initialize();
    LCD_CGRAM_SET();
    LCD_Clear();
    // skonfiguruj strumienie wyjściowe
    fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
    stdout = stderr = &hd44780_file;

    while (1)
    {
        LCD_Clear();
        LCD_GoTo(0,0);
        printf("LOADING...");
        _delay_ms(100);
        for(int i = 0; i < 16; i++)
        {
            for(int load = 0; load < 5; load++)
            {
                LCD_GoTo(i, 1);
                printf("%c", (char)(load));
                _delay_ms(100);
            }
        }
        LCD_Clear();
        printf("HURRA SUKCES!!!");
        _delay_ms(1000);
    }
}
