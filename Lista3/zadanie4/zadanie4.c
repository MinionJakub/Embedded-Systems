#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define REF_VOLT 5.100f
#define R 10000.0f
#define THERMISTOR_R0 4700.0f
#define THERMISTOR_T0 298.15f
#define THERMISTOR_B 5560

#define TO_CELISIUS(Temp) (Temp - 273.15f)
#define GET_TEMP(Temp) (THERMISTOR_B / (log(Temp / THERMISTOR_R0) + THERMISTOR_B / THERMISTOR_T0))

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem

// #define LED PB5 //Dioda
// #define LED_DDR DDRB    //dioda ddr
// #define LED_PORT PORTB  //dioda port 

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

//inicjalizowanie ADC
static inline void adc_init(){
    ADMUX = _BV(REFS0);
    ADMUX &= ~(0b1111);
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN);
}

static inline void printfloat(float number){
    number = round(number * 100) / 100;
    uart_transmit('0'+number /10 - 29,stdin);
    uart_transmit('0'+number - 10 * floor(number/10),stdin);
    uart_transmit('.',stdin);
    uart_transmit('0'+(number*10) - 10 * floor(number),stdin);
    uart_transmit('0'+(number*100) - 10 * floor(number*10),stdin);
}



int main(){
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
    uart_init();
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    adc_init();
    while(1){
        ADCSRA |= _BV(ADSC);
        while(!(ADCSRA & _BV(ADIF)));
        ADCSRA |= _BV(ADIF);
        float value = (float)ADC * (REF_VOLT / 1024.0f);        
        float temperature = ((REF_VOLT - value)/value) * R;
        float cels = TO_CELISIUS(GET_TEMP(temperature));
        printf("Temperature:\t");
        printfloat(cels);
        printf("\r\n");
        _delay_ms(1000);
    }
}
