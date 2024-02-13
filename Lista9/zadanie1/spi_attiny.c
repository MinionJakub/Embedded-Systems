#include <avr/io.h>
#include <inttypes.h>

void spi_init()
{
    // ustaw piny MOSI i SCK jako wyjścia
    DDRA = _BV(DDA4) | _BV(DDA5);
    // ustaw USI w trybie trzyprzewodowym (SPI)
    USICR = _BV(USIWM0);
}

uint8_t spi_transfer(uint8_t data)
{
    // załaduj dane do przesłania
    USIDR = data;
    // wyczyść flagę przerwania USI
    USISR = _BV(USIOIF);
    // póki transmisja nie została ukończona, wysyłaj impulsy zegara
    while (!(USISR & _BV(USIOIF))) {
        // wygeneruj pojedyncze zbocze zegarowe
        // zostanie wykonane 16 razy
        USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
    }
    // zwróć otrzymane dane
    return USIDR;
}

int main()
{
    spi_init();
    uint8_t v = 0;
    while(1) {
        spi_transfer(v++);
    }
}

