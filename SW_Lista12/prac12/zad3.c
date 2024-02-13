#include "IAR/pid.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

uint16_t adc_read(void) {
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);
  uint16_t t = ADC;
  return t;
}

void timer1_init() {
  // ustaw tryb licznika
  // COM1A = 11   -- set up, clear down
  // WGM1  = 1000 -- PHC PWM
  // CS1   = 010  -- prescaler 8
  // ICR1  = 2048
  // częstotliwość 16e6/(8*(1+2048)) ~ 976 HZ
  // wzór: datasheet 20.12.3 str. 164
  ICR1 = 2048;
  TCCR1A = _BV(COM1A1) | _BV(COM1A0);
  TCCR1B = _BV(WGM13) | _BV(CS11);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

// inicjalizacja ADC
void adc_init() {
  ADMUX = _BV(REFS0);              // referencja AVcc, wejście ADC0
  DIDR0 = _BV(ADC0D) | _BV(ADC1D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS0); // preskaler 2
  ADCSRA |= _BV(ADEN); // włącz ADC
}

// zamykający
ISR(TIMER1_CAPT_vect) {
  TIMSK1 &= ~_BV(ICIE1);
  ADCSRA |= _BV(ADSC);
}

FILE uart_file;

int main(void) {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  adc_init();
  timer1_init();
  struct PID_DATA pid;
  pid_Init((2 * SCALING_FACTOR) >> 1, 0 * SCALING_FACTOR, 0 * SCALING_FACTOR,
           &pid);
  sei();
  int16_t pid_input, ocr1a;
  uint16_t adc_closed, target;
  /*
   * 1. Zmierz napięcie silnika - Measured value
   * 2. Zmierz potencjometr - target value
   * 3. Zapytaj PID o input na podstawie 2 poprzednich wartosci
   * 4. OCR1A = 2*PID_INPUT
   */

  // 15.01.2024 18:15 ten kod zadziałał
  while (1) {
    ADMUX &= 0xf0; // ADC0 - napięcie silnika
    TIMSK1 |= _BV(ICIE1);
    adc_closed = adc_read(); // adc_closed E (0,1023), wieksza wartosc =
                             // wolniejsze obroty
    // adc_closed = 1023 - adc_closed; ??? nie wiem czemu to nie działa

    ADMUX &= 0xf0;
    ADMUX |= 0x01;
    ADCSRA |= _BV(ADSC);
    target = adc_read();
    //target = (2 * target > 1023) ? 1023 : 2 * target;
    // uint16_t może być bo wartości z ADC są max 10-bitów
    pid_input = pid_Controller(target, adc_closed, &pid);

    // PID steruje PWM
    ocr1a = OCR1A;
    printf("adc_closed = %d, target = %d, pid_intput = %d, ocr1a = %d\r\n",
           adc_closed, target, pid_input, ocr1a);
    
    if (ocr1a + pid_input <= 0)
      OCR1A = 3;
    else if(ocr1a + pid_input > 2048)
      OCR1A = 2048;
    else
        OCR1A = ocr1a + pid_input;
  }
  return 0;
}
