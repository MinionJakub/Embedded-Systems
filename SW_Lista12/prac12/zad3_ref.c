#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

void timer1_init() {
  // ustaw tryb licznika
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1000 -- PFC PWM top=ICR1
  // CS1   = 101  -- prescaler 1024
  // ICR1  = 31
  // częstotliwość 16e6/(1024*(31)) = 500 Hz
  // wzór: datasheet 20.12.3 str. 164
  ICR1 = 31;
  TCCR1A = _BV(COM1A1);
  TCCR1B = _BV(WGM13) | _BV(CS10) | _BV(CS12);
  TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

// inicjalizacja ADC
void adc_init() {
  ADMUX = _BV(REFS0); // referencja 1.1V, wejście ADC0
  DIDR0 = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS0); // preskaler 2
  ADCSRA |= _BV(ADEN) | _BV(ADIE);               // włącz ADC
}

volatile uint8_t isTop = 0, rdyAdc = 0;
volatile float adc;

ISR(TIMER1_CAPT_vect) {
  ADCSRA |= _BV(ADSC);
  isTop = 1;
}

ISR(TIMER1_OVF_vect) {
  ADCSRA |= _BV(ADSC);
  isTop = 0;
}

ISR(ADC_vect) {
  adc = ADC;
  rdyAdc = 1;
}

FILE uart_file;

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  adc_init();
  timer1_init();
  sei();
  OCR1A = 2000;
  float adcLocal = 0;
  uint8_t isTopLocal = 0;
  while (1) {
    if (rdyAdc) {
      cli();
      adcLocal = adc;
      isTopLocal = isTop;
      sei();
      rdyAdc = 0;
      adcLocal = (adcLocal * 5000) / 1024;
      if (isTopLocal) {
        printf("Closed (CAPT): %.3f mV\r\n", 5000.0 - adcLocal);
      } else {
        printf("Opened (OVF): %.3f mV\r\n", adcLocal);
      }
    }
    _delay_ms(700);
  }
  return 0;
}
