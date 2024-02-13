#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "IAR/pid.h"
#include "task.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
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

#define mainTRACE_TASK_PRIORITY 2
#define mainSHIFTING_TASK_PRIORITY 2

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

void grzejnik_init(void) { GRZEJNIK_DDR = _BV(GRZEJNIK); }

void timer_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s
  TCCR0A = _BV(WGM01);            // CTC
  TCCR0B = _BV(CS00) | _BV(CS02); // clk_io /1024
  OCR0A = 30;                     // 100Hz
  TIMSK0 |= _BV(OCIE0A);
}

void timer2_init() {
  TCCR2A = _BV(WGM21) | _BV(COM2A0); // CTC
  TCCR2B = _BV(CS20);                // no prescaler
  OCR2A = 30;
  DDRB |= _BV(PB3); // OC2A output
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

volatile uint16_t settemp = 20;
volatile uint16_t adc;
volatile uint8_t adc_bool = 0;
ISR(ADC_vect) {
  adc = ADC;
  // temp = (adc + (adc / 10)) / 1024;
  // temp = (temp - V_SUB) / MCP9700_SCALE;
  // temp = (int16_t)(((adc + (adc / 10)) / 20) - 21);
  adc_bool = 1;
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

#define stack_size ((unsigned short)300)

static void pidControllerTask(void *pvParameters);
static void uartControllerTask(void *pvParameters);

int main(void) {
  uart_init();
  stdin = stdout = stderr = &uart_file;

  xTaskHandle trace_handle;
  xTaskHandle shifting_handle;

  xTaskCreate(pidControllerTask, "trace", stack_size, NULL,
              mainTRACE_TASK_PRIORITY, &trace_handle);

  xTaskCreate(uartControllerTask, "shift", stack_size, NULL,
              mainSHIFTING_TASK_PRIORITY, &shifting_handle);

  vTaskStartScheduler();

  return 0;
}

int16_t sum_pid(struct PID_DATA *pid_data) {
  return pid_data->P_Factor + pid_data->D_Factor + pid_data->I_Factor;
}

void vApplicationIdleHook(void) {}
#define RISING 1
#define FALLING 0
static void pidControllerTask(void *pvParameters) {
  timer2_init();
  timer_init();
  adc_init();
  struct PID_DATA pid_data;
  //                 0.5                     0                   0
  pid_Init((2 * SCALING_FACTOR) >> 1, 0 * SCALING_FACTOR, 0 * SCALING_FACTOR,
           &pid_data);
  sei();
  int16_t input, ocr2a = OCR2A;
  uint16_t cnt, mapped_temp;
  while (1) {
    if (adc_bool) {

      // Jeśli settemp E (0,35) to takie cos zmapuje go na (0,255)
      // adc E (0,1023) więc adc/4 bedzie tez w (0,255)
      input = pid_Controller((settemp * 255) / 35, adc / 4, &pid_data);
      //input = -input;
      if (cnt % 300 == 0) {
        printf("Pid controller value: %d, OCR2A %d\r\n", input, ocr2a);
        cnt = 0;
      }
      cnt++;
      ocr2a = OCR2A;
      if (ocr2a + input <= 0)
        OCR2A = 0;
      else if (ocr2a + input > 255)
        OCR2A = 255;
      else
        OCR2A = ocr2a + input;
      adc_bool = 0;
    }
  }
}

static void uartControllerTask(void *pvParameters) {
  char cmd[BUFF];
  int16_t tmpsettemp;
  char *strpart;
  float temp;
  while (1) {
    readline(cmd);
    strpart = strtok(cmd, " ");
    if (!strncmp(strpart, "temp", 4)) {
      temp = (adc * 1.1) / 1024.0;
      temp = (temp - V_SUB) / MCP9700_SCALE;
      printf("Current temp: %.2f\r\n", temp);
    } else if (!strncmp(strpart, "change", 6)) {
      strpart = strtok(NULL, " ");
      tmpsettemp = atoi(strpart);
      settemp = tmpsettemp;
      printf("Set temp: %d\r\n", tmpsettemp);
    }
  }

  return 0;
}
