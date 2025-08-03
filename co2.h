#ifndef CO2_H
#define CO2_H

#include "pico/stdlib.h"
#include "hardware/uart.h"

// === Pin Configuration ===
#define UART1_TX_PIN 8   // TX for CO2 sensor
#define UART1_RX_PIN 9   // RX for CO2 sensor
#define BAUD_RATE 9600   // Baud rate for CO2 sensor

// === Function Prototypes ===
void co2_init();
int co2_read_sensor_data();

#endif // CO2_H