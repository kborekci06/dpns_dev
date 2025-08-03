#include <stdio.h>
#include <stdlib.h> 
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "co2.h"

// === CO₂ Sensor Initialization ===
void co2_init() {
    // Initialize UART1
    uart_init(uart1, BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    

    printf("Initializing CO2 Sensor...\n");

    // Send "K 2\r\n" (set mode)
    uart_puts(uart1, "K 2\r\n");
    sleep_ms(500);

    // Read and print mode response
    uint8_t buffer[8] = {0};
    if (uart_is_readable(uart1)) {
        uart_read_blocking(uart1, buffer, sizeof(buffer));
        printf("Mode Response: %s\n", buffer);
    }

    // Send ".\r\n" (get multiplier)
    uart_puts(uart1, ".\r\n");
    sleep_ms(500);

    // Read and print multiplier response
    uint8_t buffer2[10] = {0};
    if (uart_is_readable(uart1)) {
        uart_read_blocking(uart1, buffer2, sizeof(buffer2));
        printf("Multiplier: %s\n", buffer2);
    }

    printf("CO2 Sensor Initialization Complete!\n");
}

// === Read and Print CO₂ Sensor Data ===
int co2_read_sensor_data() {
    // Send "z\r\n" to request CO₂ reading
    uart_puts(uart1, "z\r\n");
    // sleep_ms(10);  // Allow time for response

    // Read CO2 value
    uint8_t co2_buffer[10] = {0};
    if (uart_is_readable(uart1)) {
        uart_read_blocking(uart1, co2_buffer, sizeof(co2_buffer));

        // Convert buffer to a string and remove unwanted characters
        char co2_str[10] = {0};  // Temporary buffer to hold filtered value
        int j = 0;

        for (int i = 0; i < sizeof(co2_buffer); i++) {
            if (co2_buffer[i] >= '0' && co2_buffer[i] <= '9') { // Only keep digits
                co2_str[j++] = co2_buffer[i];
            }
        }

        // Convert extracted string to integer
        return atoi(co2_str);  // Return the CO₂ value instead of printing

        // // Convert extracted string to integer
        // int co2_ppm = atoi(co2_str);

        // // Print final CO₂ value as an integer
        // printf("CO2 (ppm): %d\n", co2_ppm);
    }
}