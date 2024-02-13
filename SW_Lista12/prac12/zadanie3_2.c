#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include "pid.c"

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

void adc_init() {
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADEN);
  DIDR0  = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0
  DIDR0 |= _BV(ADC1D);  // wyłącz wejście cyfrowe na ADC1
  
//   ADCSRA |= _BV(ADIE);
  set_sleep_mode(SLEEP_MODE_IDLE);
}

ISR(ADC_vect) {}

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

void timer1_init() {
  TCCR1A = _BV(COM1A1);
  TCCR1B = _BV(WGM13) | _BV(CS10);
  TIMSK1 = _BV(ICIE1);

  // 500 Hz = 16e6 Hz / 2 / 1 / 16000
  ICR1 = 16000;
  OCR1A = 0;
  DDRB |= _BV(PB1);
}

int16_t Get_Reference();
int16_t Get_Measurement();
void Set_Input(int16_t);
volatile int16_t referenceValue = 0, measurementValue = 0, inputValue = 0;
ISR(TIMER1_CAPT_vect)
{
//   cli();
  TIMSK1 &= ~_BV(ICIE1);
  measurementValue = Get_Measurement();
  referenceValue = Get_Reference();
  inputValue = pid_Controller(referenceValue, measurementValue, &pidData);
  Set_Input(inputValue);
  TIMSK1 |= _BV(ICIE1);
//   sei();
}

void Init(void)
{
  pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);
}

uint16_t adc_v = 0;
int16_t Get_Reference(void)
{
  ADMUX &= ~_BV(MUX0);
  ADCSRA |= _BV(ADSC);
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);
  adc_v = ADC;
  return adc_v;
}

int16_t Get_Measurement(void)
{
  ADMUX |= _BV(MUX0);
  ADCSRA |= _BV(ADSC);
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);
  adc_v = ADC;
  return 1023 - adc_v;
}

void Set_Input(int16_t inputValue)
{
  if (inputValue < 0 && OCR1A < -inputValue)
    OCR1A = 0;

  else if (inputValue > 0 && OCR1A >= ICR1 - inputValue)
    OCR1A = ICR1;

  else
    OCR1A += inputValue;
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

  while (1) {
    _delay_ms(100);
    printf("\rrV: %4d, mV: %4d, iV: %5d, OCR1A: %6u", referenceValue, measurementValue, inputValue, OCR1A);
  }
}