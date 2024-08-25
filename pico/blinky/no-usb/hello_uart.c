/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

/// \tag::hello_uart[]

#define UART_ID0 uart0
#define UART_ID1 uart1
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

int main() {
    // Set up our UART with the required speed.
    uart_init(UART_ID0, BAUD_RATE);

    uart_init(UART_ID1, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART0_TX_PIN, UART_FUNCSEL_NUM(UART_ID0, UART0_TX_PIN));
    gpio_set_function(UART0_RX_PIN, UART_FUNCSEL_NUM(UART_ID0, UART0_RX_PIN));

    gpio_set_function(UART1_TX_PIN, UART_FUNCSEL_NUM(UART_ID1, UART0_TX_PIN));
    gpio_set_function(UART1_RX_PIN, UART_FUNCSEL_NUM(UART_ID1, UART0_RX_PIN));

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    // Send out a character without any conversions
    uart_putc_raw(UART_ID0, 'A');

    // Send out a character but do CR/LF conversions
    uart_putc(UART_ID0, 'B');

    while(true)
      {
    
    // Send out a string, with CR/LF conversions
	uart_puts(UART_ID0, " Hello, UART!\n");
	uart_puts(UART_ID1, " Hello, UART!\n");
	sleep_ms(500);
      }
}

/// \end::hello_uart[]
