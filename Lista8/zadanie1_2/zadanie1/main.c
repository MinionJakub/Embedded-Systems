/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/
#define __AVR_ATmega328P__

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>


#include <stdio.h>
// #include "uart.h"


/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_counter_TASK_PRIORITY   2

#define mainBUTTON_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vLED_counter(void* pvParameters);

static void vButton(void* pvParameters);

/******************************************************************************
 * Public function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn int main(void)
 *
 * \brief Main function.
 *
 * \return
 ******************************************************************************/
int main(void)
{
    // Create task.
    xTaskHandle blink_handle;
    xTaskHandle serial_handle;

    xTaskCreate
        (
         vLED_counter,
         "blink",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainLED_counter_TASK_PRIORITY,
         &blink_handle
        );

    xTaskCreate
        (
         vButton,
         "serial",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainBUTTON_TASK_PRIORITY,
         &serial_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************//**
 * \fn static vApplicationIdleHook(void)
 *
 * \brief
 ******************************************************************************/
void vApplicationIdleHook(void)
{

}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn static void vLED_counter(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/

// #define LED_DDR DDRD
// #define LED_PORT PORTD


// #define LED PD0
#define LED_DDR DDRD
#define LED_PORT PORTD

static void vLED_counter(void* pvParameters)
{
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    LED_DDR = 0xff;
    LED_PORT = 0;
    while (1) {
        for(int8_t i = 0; i <= 5; i++){
            if(i > 0){
                LED_PORT &= ~_BV(i-1);
            }
            if(i > 1){
                LED_PORT &= ~_BV(i-2);
            }
            LED_PORT |= _BV(i);
            LED_PORT |= _BV(i+1);
            LED_PORT |= _BV(i+2);
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        for(int8_t i = 7;i >= 2; i--){
            if(i < 7){
                LED_PORT &= ~_BV(i+1);
            }
            if(i < 6){
                LED_PORT &= ~_BV(i+2);
            }

            LED_PORT |= _BV(i);
            LED_PORT |= _BV(i-1);
            LED_PORT |= _BV(i-2);
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
  }
}


/**************************************************************************//**
 * \fn static void vButton(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
#define LED PB1
#define LED_DDR2 DDRB
#define LED_PORT2 PORTB

#define BUTTON PC4
#define BUTTON_PORT PORTC
#define BUTTON_DDR DDRC

#define CIRCLE_SIZE 256/8

static uint8_t cirular_buffer[CIRCLE_SIZE];

static inline uint8_t get_place(uint8_t value){
    return (value >> 3);
}

static inline uint8_t get_bit(uint8_t value){
    return (value&7);
}

static inline void light(uint8_t value){
    if(value)
        LED_PORT2 |= _BV(LED);
    else
        LED_PORT2 &= ~_BV(LED);
}


static void vButton(void* pvParameters)
{
    BUTTON_DDR &= ~_BV(BUTTON);
    BUTTON_PORT |= _BV(BUTTON);

    LED_DDR2  |= _BV(LED);
    uint8_t crossed_100 = 0;
    uint8_t place;
    uint8_t bit;
    uint8_t writer = 0;
    uint8_t reader = 0;
    // while(1) LED_PORT2 |= _BV(LED);
    while(1){
        if(reader == 255)
            reader = 0;
        if(writer == 255)
            writer = 0;
        if(!crossed_100 && writer == 100)
            crossed_100 = 1;
        place = get_place(writer);
        bit = get_bit(writer);
        if(!(PINC&_BV(BUTTON))){
            cirular_buffer[place] |= _BV(bit);
        }
        else{
            cirular_buffer[place] &= ~_BV(bit);
        }
        writer++;
        if(crossed_100){
            place = get_place(reader);
            bit = get_bit(reader);
            light(cirular_buffer[place]&_BV(bit));
            reader++;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
