#ifndef __UARTZAD1_H
#define __UARTZAD1_H
#include <stdio.h>
#include <avr/io.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

void uart_init();
int uart_transmit(char data, FILE *stream);
int uart_receive(FILE *stream);
#endif
