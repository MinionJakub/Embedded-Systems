#define __AVR_ATmega328P__

#include "FreeRTOS.h"
#include "task.h"
#include <avr/io.h>
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
#include <avr/interrupt.h>


#define mainECHO   2
#define mainBLINKER 1

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

static void vBlinker(void* pvParameters);
static void vECHO(void* pvParameters);

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

// inicjalizacja UART
void uart_init(void)
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(UDRIE0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
  //wlaczenie przerwan
  UCSR0B |= _BV(RXCIE0) | _BV(TXCIE0);
}

QueueHandle_t tx_buffer;
QueueHandle_t rx_buffer;


ISR(USART_UDRE_vect){
  uint8_t value;
  if(xQueueReceiveFromISR(tx_buffer,&value,0) == pdPASS){
    UDR0 = value;
  }
  // BaseType_t checkIfYieldRequired;
  // checkIfYieldRequired = xTaskResumeFromISR(mainECHO);
  // taskYIELD_FROM_ISR(checkIfYieldRequired);
}

ISR(USART_RX_vect){
  char received = UDR0;
  xQueueSendFromISR(rx_buffer,&received,NULL);
  // BaseType_t checkIfYieldRequired;
  // checkIfYieldRequired = xTaskResumeFromISR(mainECHO);
  // taskYIELD_FROM_ISR(checkIfYieldRequired);
}

int uart_transmit(char c, FILE *stream) {
  xQueueSend(tx_buffer, &c, portMAX_DELAY);
  // UCSR0B |= _BV(UDRIE0);
  // vTaskSuspend(NULL);
  // while (!(UCSR0A & _BV(UDRE0))) taskYIELD();
  // UDR0 = c;
  return 0;
}

int uart_receive(FILE *stream) {
  uint8_t data;
  if(xQueueReceive(rx_buffer,&data,portMAX_DELAY) == pdPASS){
    return data;
  }
  // vTaskSuspend(NULL);
  // while (!(UCSR0A & _BV(RXC0))) taskYIELD();
  // return _FDEV_EOF;
}


int main(void)
{
    tx_buffer = xQueueCreate(100,sizeof(uint8_t));
    rx_buffer = xQueueCreate(100,sizeof(uint8_t));

    // Create task.
    xTaskHandle blinker_handle;
    xTaskHandle echo_handle;

    xTaskCreate
        (
         vBlinker,
         "blink",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainBLINKER,
         &blinker_handle
        );

    xTaskCreate
        (
         vECHO,
         "serial",
         600,
         NULL,
         mainECHO,
         &echo_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}


void vApplicationIdleHook(void)
{

}

static void vBlinker(void* pvParameters)
{
    DDRB |= _BV(PB2);

    for ( ;; )
    {
        PORTB ^= _BV(PB2);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


static void vECHO(void* pvParameters)
{
    uart_init();
    stdin = stdout = stderr = &uart_file;

    char input;

    for ( ;; )
    {
        // puts("Hello World\r\n");
        input = getchar();
        putchar(input);
        // printf("You wrote %c\r\n", input); 
        // vTaskDelay(500);
    }
}