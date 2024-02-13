#ifndef __UARTZAD1LISTA10_H
#define __UARTZAD1LISTA10_H
#include <avr/io.h>
#include <stdio.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

void uart_init();
int uart_transmit(char data, FILE *stream);
int uart_receive(FILE *stream);
#endif
