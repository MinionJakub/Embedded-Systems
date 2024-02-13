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

#define LED_DDR DDRD
#define LED_PORT PORTD

static const LED_DIGITS[] = {
192 , //0
249 , //1
164 , //2
176 , //3
153 , //4
146 , //5
130 , //6
248 , //7
128 , //8
144   //9
};

static void vLED_counter(void* pvParameters)
{
    static uint8_t counter = 0;
    LED_DDR = 0xff;
    LED_PORT = 0xff;
    while(1){
        LED_PORT = LED_DIGITS[counter++];
        counter %= 10;
        vTaskDelay(1000/portTICK_PERIOD_MS);
        // vTaskDelay(500/portTICK_PERIOD_MS);
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
