#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem

#define REF_VOLT 5.053f
#define R 10000.0f
#define THERMISTOR_R0 4700.0f
#define THERMISTOR_T0 298.15f
#define THERMISTOR_B 4100

#define TO_CELCIUS(kelvinTemp) (kelvinTemp - 273.15f)

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

// ADC

void setup_ADC(){
    ADMUX = _BV(REFS0); // setup ref vol (here it is 5V)

    ADMUX &= ~(0b1111);// setup input pins

    DIDR0 = _BV(ADC0D); // disable digital input on pin adc0
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //setup prescaler
    ADCSRA |= _BV(ADEN); // enables the ADC
}

void printFloat(float number) {
    number = round(number * 100) / 100; /* round off to 2 decimal places */
    uart_transmit('0' + number / 10 - 25, stdin); /* tens place */
    uart_transmit('0' + number - 10 * floor(number / 10), stdin); /* ones */
    uart_transmit('.', stdin);
    uart_transmit('0' + (number * 10) - floor(number) * 10, stdin); /* tenths */
    /* hundredths place */
    uart_transmit('0' + (number * 100) - floor(number * 10) * 10, stdin);
}

int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);

    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    
    setup_ADC();

    while(1){

        ADCSRA |= _BV(ADSC); // start conversion

        while(!(ADCSRA & _BV(ADIF)));

        ADCSRA |= _BV(ADIF);

        float vIn = (float)ADC * (REF_VOLT / 1024.0f);

        printf("Measured voltage: ");    
        printFloat(vIn);
        printf("\r\n");

        float r = ((REF_VOLT - vIn)/(vIn)) * R;
        float temperature = TO_CELCIUS(
            THERMISTOR_B / (log(r / THERMISTOR_R0) + THERMISTOR_B / THERMISTOR_T0)
        );

        printf("Current temperature: ");    
        printFloat(temperature);
        printf("\r\n");

        _delay_ms(1000);
    }
}
