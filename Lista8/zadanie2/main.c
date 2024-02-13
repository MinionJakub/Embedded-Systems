#define __AVR_ATmega328P__
#ifndef F_CPU
#define F_CPU 16000000UL
#endif


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <avr/io.h>
#include <stdio.h>

#define mainBlinker     2
#define mainWriter      1

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

static void vBlinker(void* pvParameters);
static void vWriter(void* pvParameters);

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
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int uart_transmit(char c, FILE *stream) {
  while (!(UCSR0A & _BV(UDRE0))) taskYIELD();
  UDR0 = c;
  return 0;
}

int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0))) taskYIELD();
  return UDR0;
}

QueueHandle_t xQueue;
int main(void)
{
    xQueue = xQueueCreate(10,sizeof(uint16_t));
    uart_init();
    stdin = stdout = stderr = &uart_file;
    xTaskHandle writer_hendler;
    xTaskHandle blinker_hendler;
    xTaskCreate(vBlinker,"blink",600,NULL,mainBlinker,&blinker_hendler);
    xTaskCreate(vWriter,"writer",600,NULL,mainWriter,&writer_hendler);
    
    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}


void vApplicationIdleHook(void)
{

}

static void vWriter(void* pvParameters)
{
    BaseType_t xStatus;
    uint16_t SendValue;
    printf("Wpisz wartosc\r\n");
    while(1){
        scanf("%"SCNu16,&SendValue);
        printf("Send:\t%"PRIu16"\r\n",SendValue);
        xStatus = xQueueSendToBack(xQueue,&SendValue,0);
        if(xStatus != pdPASS) printf("COULDN'T SEND VALUE\r\n");
    }
}

#define LED PB2
#define LED_DDR DDRB
#define LED_PORT PORTB

static void vBlinker(void* pvParameters)
{
    uart_init();
    stdin = stdout = stderr = &uart_file;
    BaseType_t xStatus;
    uint16_t value;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    LED_DDR |= _BV(LED);
    while(1){
        xStatus = xQueueReceive(xQueue,&value,100/portTICK_PERIOD_MS);
        if(xStatus == pdPASS){
            printf("Read:\t%"PRIu16"\r\n",value);
            LED_PORT |= _BV(LED);
            uint16_t timing = value/portTICK_PERIOD_MS;
            vTaskDelay(timing);
            // _delay_ms(value);
            LED_PORT &= ~_BV(LED);
        }
    }
}
