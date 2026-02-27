#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// **I2C Configuration**
#define ICM_20948_ADDR  0x69  // 7-bit I2C address
#define SDA_PIN 4   // SDA on GPIO 4
#define SCL_PIN 5   // SCL on GPIO 5

// #define INT1_PIN 10  // Connect INT1 pin of IMU to GPIO10

// **Register Addresses**
#define WHO_AM_I        0x00  // Should return 0xEA
#define PWR_MGMT_1      0x06  // Power management
#define PWR_MGMT_2      0x07  // Enable/disable accel & gyro
#define LP_CONFIG       0x05  // Low power mode
#define GYRO_CONFIG_1   0x01  // Gyro configuration
#define ACCEL_CONFIG    0x14  // Accelerometer configuration
#define REG_BANK_SEL    0x7F  // Register bank selection

// // **Interrupt Registers**
// #define INT_PIN_CFG      0x0F  // Interrupt pin configuration
// #define INT_ENABLE_1     0x11  // Enable interrupt sources
// #define INT_STATUS_1     0x1A  // Interrupt status (bit 0: Data Ready)


// **Accelerometer Output Registers**
#define ACCEL_XOUT_H    0x2D
#define ACCEL_YOUT_H    0x2F
#define ACCEL_ZOUT_H    0x31

#define ACCEL_LSB_PER_G 4096.0f // For ±8g range (from datasheet)
#define GYRO_LSB_PER_DPS  32.8f // For ±1000 dps range (from datasheet)

// **Gyroscope Output Registers**
#define GYRO_XOUT_H     0x33
#define GYRO_YOUT_H     0x35
#define GYRO_ZOUT_H     0x37

// **WHO_AM_I expected value**
#define ICM_20948_WHOAMI_EXPECTED 0xEA

// **Function Prototypes**
void icm20948_write_register(uint8_t reg, uint8_t value);
uint8_t icm20948_read_register(uint8_t reg);
void icm20948_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t length);
void icm20948_set_bank(uint8_t bank);
void icm20948_init();
void icm20948_read_sensor_data();

// bool icm20948_data_ready();
// void icm20948_enable_interrupt();
// void icm20948_irq_handler(uint gpio, uint32_t events);

// extern volatile bool imu_data_ready;

extern volatile float accel_x, accel_y, accel_z;
extern volatile float gyro_x, gyro_y, gyro_z;

#endif