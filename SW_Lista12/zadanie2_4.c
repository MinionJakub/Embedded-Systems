#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include "pid.c"

// uart


#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem
#define BUFFER_SIZE 64
#define V_0 0.4
#define TemperatureCoefficient 0.0195

#define GRZALKA PB5        // Dioda
#define GRZALKA_DDR DDRB   // dioda ddr
#define GRZALKA_PORT PORTB // dioda port

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
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

FILE uart_file;


void adc_init() {
  ADMUX = _BV(REFS1) | _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADATE) | _BV(ADSC) | _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0 = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0
}

#define K_P     1.00
#define K_I     0.00
#define K_D     0.00

struct GLOBAL_FLAGS {
  //! True when PID control loop should run one time
  uint8_t pidTimer:1;
  uint8_t dummy:7;
} gFlags = {0, 0};

//! Parameters for regulator
struct PID_DATA pidData;

#define TIME_INTERVAL   157

int16_t Get_Reference();
int16_t Get_Measurement();
void Set_Input(int16_t);

// #pragma vector = TIMER0_OVF_vect
volatile int16_t referenceValue, measurementValue, inputValue;
ISR(TIMER0_OVF_vect)
{
  static uint16_t i = 0;
  if(i < TIME_INTERVAL)
    i++;
  else{
    // gFlags.pidTimer = TRUE;

      referenceValue = Get_Reference();
      measurementValue = Get_Measurement();

      inputValue = pid_Controller(referenceValue, measurementValue, &pidData);

      Set_Input(inputValue);
    i = 0;
  }
}
void timer1_init() {
  TCCR1A = _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

  ICR1 = 0xFFFF;
  OCR1B = 0;
  DDRB |= _BV(PB2);
}

void Init(void)
{
  pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);

  // Set up timer, enable timer/counte 0 overflow interrupt
  TCCR0B = _BV(CS00);
  TIMSK0 = _BV(TOIE0);
  TCNT0 = 0;
}

volatile uint8_t temp = 0;
volatile uint8_t T = 25;
int16_t Get_Reference(void)
{
  return T;
}

int16_t Get_Measurement(void)
{
  return temp;
}

void Set_Input(int16_t inputValue)
{
  if (inputValue < 0 && OCR1B < -inputValue)
    OCR1B = 0;

  else if (inputValue > 0 && OCR1B >= ICR1 - inputValue)
    OCR1B = ICR1;

  else OCR1B += inputValue;
}

// V_out = T_c * T_a + V_0
// ==>
// T_a = (V_out - V_0) / T_c
//
// V_0 = 500mV
// T_c = 10mV/C°
// ==>
// T_a = (V_out - 500mV) / 10mv/C°
// (5.0 * ADC / 1023 - 0.5) * 100
ISR(ADC_vect) {
  temp = (110ull * ADC / 1023 - 50);
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy
  adc_init();
  timer1_init();
  Init();

  sei();

  char c;
  while (1) {
    c = getchar();
    if (c == 'r') {
      printf("\rmT: %5u, rT: %5u, iV: %5d, OCR1B: %5u", temp, T, inputValue, OCR1B);
    } else if (c == 'w') {
      scanf("%hhu", &T);
    }
  }
}
