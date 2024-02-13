#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#define GRZEJNIK PB5
#define GRZEJNIK_DDR DDRB
#define GRZEJNIK_PORT PORTB
#define BUFF 20
#define STDIN_FILENO 0
#define TEMP_EPS 0.3
#define V_SUB 0.4
#define MCP9700_SCALE 0.0195

FILE uart_file;

void grzejnik_init(void) { GRZEJNIK_DDR = _BV(GRZEJNIK); }

void timer_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s
  TCCR0A = _BV(WGM01);            // CTC
  TCCR0B = _BV(CS00) | _BV(CS02); // clk_io /1024
  OCR0A = 30;                     // 100Hz
  TIMSK0 |= _BV(OCIE0A);
}

void adc_init() {
  ADMUX = _BV(REFS0) | _BV(REFS1); // referencja 1.1V, wejście ADC0
  DIDR0 = _BV(ADC0D);              // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN) | _BV(ADIE);               // włącz ADC i interrupt
  // ADCSRB = _BV(ADTS0) | _BV(ADTS1);
}

ISR(TIMER0_COMPA_vect) { ADCSRA |= _BV(ADSC); }

volatile float temp = 0.0, settemp = 20.0;
volatile uint16_t adc;
ISR(ADC_vect) {
  GRZEJNIK_PORT &= ~_BV(GRZEJNIK);
  adc = ADC;
  GRZEJNIK_PORT |= _BV(GRZEJNIK);
  temp = (adc * 1.1) / 1024;
  temp = (temp - V_SUB) / MCP9700_SCALE;
  if (temp <= settemp - TEMP_EPS) {
    GRZEJNIK_PORT |= _BV(GRZEJNIK);
  } else if (temp >= settemp)
    GRZEJNIK_PORT &= ~_BV(GRZEJNIK);
}

uint8_t readline(char *line) {
  uint8_t c = 0;
  while (c < BUFF) {
    line[c] = getchar();
    if (line[c++] == 0x0d)
      break;
  }
  line[c] = '\0';
  return c;
}

int main(void) {
  char cmd[BUFF];
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  grzejnik_init();
  timer_init();
  adc_init();

  sei();
  float tmpsettemp;
  char *strpart;
  while (1) {
    readline(cmd);
    strpart = strtok(cmd, " ");
    if (!strncmp(strpart, "temp", 4)) {
      printf("Current temp: %.2f\r\n", temp);
    } else if (!strncmp(strpart, "change", 6)) {
      strpart = strtok(NULL, " ");
      tmpsettemp = atof(strpart);
      settemp = tmpsettemp;
      printf("Set temp: %.2f\r\n", tmpsettemp);
    }
  }

  return 0;
}
