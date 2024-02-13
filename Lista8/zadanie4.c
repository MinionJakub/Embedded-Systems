#define __AVR_ATmega328P__
#include "FreeRTOS.h"
#include "task.h"
#include <avr/io.h>
#include <stdio.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>
#include "semphr.h"
#include <avr/interrupt.h>

#define mainADC_TASK_1   1
#define mainADC_TASK_2   1
#define mainADC_TASK_3   1

static void vADC_READ1(void *pvParameters);
static void vADC_READ2(void *pvParameters);
static void vADC_READ3(void *pvParameters);

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

void uart_init() {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0); /* Enable RX and TX */
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

SemaphoreHandle_t semADC, semADCComplete, printSEM;

void adc_init(){
    ADMUX = _BV(REFS0); // referencja AVcc
    DIDR0 = _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D); // wyłącz wejście cyfrowe na ADC0, ADC1, ADC2
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN); // włącz ADC
    ADCSRA |= _BV(ADIE); // włącz interupty nad ADC
}

int main(void)
{
    xTaskHandle adc1,adc2,adc3;
    uart_init();
    stdin = stdout = stderr = &uart_file;
    adc_init();

    sei();
    semADC = xSemaphoreCreateBinary();
    // semADCComplete = xSemaphoreCreateBinary();
    // printSEM = xSemaphoreCreateBinary();

    xSemaphoreGive(semADC);
    // xSemaphoreGive(printSEM);

    xTaskCreate(vADC_READ1,"ADC1",400,(void*)0,mainADC_TASK_1,&adc1);
    xTaskCreate(vADC_READ2,"ADC2",400,(void*)(0xe),mainADC_TASK_2,&adc2);
    xTaskCreate(vADC_READ3,"ADC3",400,(void*)(_BV(MUX0)),mainADC_TASK_3,&adc3);

    vTaskStartScheduler();

    return 0;
}
void vApplicationIdleHook(void)
{

}

ISR(ADC_vect) {xSemaphoreGive(semADCComplete);}

// __attribute((always_inline)) 
static inline uint16_t readADC(uint8_t mux){
    xSemaphoreTake(semADC,portMAX_DELAY);
    uint16_t adc_state;
    ADMUX &= 0xf0;
    ADMUX |= mux;
    ADCSRA |= _BV(ADSC);
    // xSemaphoreTake(semADCComplete,portMAX_DELAY);
    adc_state = ADC;
    xSemaphoreGive(semADC);
    return adc_state;   
}

static void vADC_READ1(void *pvParameters){
    uint8_t input = (uint8_t)pvParameters;
    uint16_t adc;
    char buf[10];
    while(1){
        adc = readADC(input);
        // xSemaphoreTake(printSEM,portMAX_DELAY);
        itoa(adc,buf,10);
        printf("ADC1:\t%s\r\n",buf);
        fflush(stdout);
        // xSemaphoreGive(printSEM);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}


static void vADC_READ2(void *pvParameters){
    uint8_t input = (uint8_t)pvParameters;
    uint16_t adc;
    char buf[10];
    while(1){
        adc = readADC(input);
        // xSemaphoreTake(printSEM,portMAX_DELAY);
        itoa(adc,buf,10);
        printf("ADC2:\t%s\r\n",buf);
        fflush(stdout);
        // xSemaphoreGive(printSEM);
        vTaskDelay(400/portTICK_PERIOD_MS);
    }
}


static void vADC_READ3(void *pvParameters){
    uint8_t input = (uint8_t)pvParameters;
    uint16_t adc;
    char buf[10];
    while(1){
        adc = readADC(input);
        // xSemaphoreTake(printSEM,portMAX_DELAY);
        itoa(adc,buf,10);
        printf("ADC3:\t%s\r\n",buf);
        fflush(stdout);
        // xSemaphoreGive(printSEM);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}