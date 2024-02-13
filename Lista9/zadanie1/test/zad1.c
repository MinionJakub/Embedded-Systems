#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#define LED PB2        // Dioda
#define LED_DDR DDRB   // dioda ddr
#define LED_PORT PORTB // dioda port

#define BUTTON PB1
#define BUTTON_DDR DDRB
#define BUTTON_PORT PORTB
#define BUTTON_PIN PINB
#define BUTTON_PRESSED !(BUTTON_PIN & _BV(BUTTON))

#define BUFFER_SIZE (128 / 8)

void init(void) {
  BUTTON_DDR &= ~_BV(BUTTON); // button input
  BUTTON_PORT |= _BV(BUTTON); // button pull-up
  LED_DDR |= _BV(LED);        // led output
}

void timer_init() {
  // ctc mode tak zeby przerwanie było co 1/100 s
  TCCR0A = _BV(WGM01);            // CTC
  TCCR0B = _BV(CS02); // clk_io /1024
  OCR0A = 30;                    // 100Hz
  TIMSK0 |= _BV(OCIE0A);
}

volatile uint8_t circular_buffer[BUFFER_SIZE];
volatile uint8_t read_ptr = 0, exec_ptr = 0, execute_flag = 0;
volatile uint8_t extra_time = 0;

// procedura obsługi przerwania przepełnienia licznika
ISR(TIM0_COMPA_vect) {
  // save state
  if (BUTTON_PRESSED)
    circular_buffer[read_ptr / 8] |= _BV(read_ptr & 7);
  else
    circular_buffer[read_ptr / 8] &= ~_BV(read_ptr & 7);
  read_ptr = (read_ptr + 1) & 127;
  // if 1s passed, exec state
  if (extra_time == 100) {
    if (circular_buffer[exec_ptr / 8] & _BV(exec_ptr & 7)) {
      LED_PORT |= _BV(LED);
      circular_buffer[exec_ptr / 8] &= ~_BV(exec_ptr & 7);
    } else
      LED_PORT &= ~_BV(LED);
    exec_ptr = (exec_ptr + 1) & 127;
  } else
    extra_time++;
}

int main() {
  // zainicjalizuj UART
  // zainicjalizuj licznik 2
  memset(circular_buffer, 0, BUFFER_SIZE);
  init();
  timer_init();
  // ustaw tryb uśpienia na tryb bezczynności
  set_sleep_mode(SLEEP_MODE_IDLE);
  // odmaskuj przerwania
  sei();
  // program testowy
  while (1) {
    sleep_mode();
  }
}
