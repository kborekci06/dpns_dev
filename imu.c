#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/time.h"  
#include "imu.h"

// 8.2
// **Function Definitions**

void icm20948_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = { reg, value };
    i2c_write_blocking(i2c_default, ICM_20948_ADDR, buffer, 2, false);
}

uint8_t icm20948_read_register(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, ICM_20948_ADDR, &value, 1, false);
    return value;
}

void icm20948_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
    i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, ICM_20948_ADDR, buffer, length, false);
}

void icm20948_set_bank(uint8_t bank) {
    icm20948_write_register(REG_BANK_SEL, (bank << 4));
}

void icm20948_init() {
    uint8_t who_am_i = icm20948_read_register(WHO_AM_I);
    if (who_am_i != ICM_20948_WHOAMI_EXPECTED) {
        printf("Error: ICM-20948 not detected! (Read: 0x%X)\n", who_am_i);
        return;
    }
    printf("ICM-20948 detected! WHO_AM_I = 0x%X\n", who_am_i);

    icm20948_write_register(PWR_MGMT_1, 0x80);
    sleep_ms(50);  

    icm20948_write_register(PWR_MGMT_1, 0x01);
    icm20948_write_register(PWR_MGMT_2, 0x00);

    icm20948_set_bank(0);

    icm20948_write_register(LP_CONFIG, 0x00);
    icm20948_write_register(ACCEL_CONFIG, 0x00);  // ±2g
    icm20948_write_register(GYRO_CONFIG_1, 0x00); // ±250°/s

    printf("ICM-20948 Initialization Complete!\n");
}


// Interrupt version (without printing)
void icm20948_read_sensor_data(float *accel_x_g, float *accel_y_g, float *accel_z_g,
    float *gyro_x_dps, float *gyro_y_dps, float *gyro_z_dps) {

    icm20948_set_bank(0);

    uint8_t data[12];
    icm20948_read_bytes(ACCEL_XOUT_H, data, 12);

    int16_t accel_x = (int16_t)((data[0] << 8) | data[1]);
    int16_t accel_y = (int16_t)((data[2] << 8) | data[3]);
    int16_t accel_z = (int16_t)((data[4] << 8) | data[5]);

    int16_t gyro_x = (int16_t)((data[6] << 8) | data[7]);
    int16_t gyro_y = (int16_t)((data[8] << 8) | data[9]);
    int16_t gyro_z = (int16_t)((data[10] << 8) | data[11]);

    // Convert raw values to meaningful physical values
    *accel_x_g = accel_x / 16384.0f;
    *accel_y_g = accel_y / 16384.0f;
    *accel_z_g = accel_z / 16384.0f;

    *gyro_x_dps = gyro_x / 131.0f;
    *gyro_y_dps = gyro_y / 131.0f;
    *gyro_z_dps = gyro_z / 131.0f;
}

// // Task 8.1 Implement Hardware Interrupt - Testing - Interrupt Triggers but no writing
// volatile bool imu_data_ready = false;  // Global flag

// // **Function Definitions**

// void icm20948_write_register(uint8_t reg, uint8_t value) {
//     uint8_t buffer[2] = { reg, value };
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, buffer, 2, false);
// }

// uint8_t icm20948_read_register(uint8_t reg) {
//     uint8_t value;
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
//     i2c_read_blocking(i2c_default, ICM_20948_ADDR, &value, 1, false);
//     return value;
// }

// void icm20948_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
//     i2c_read_blocking(i2c_default, ICM_20948_ADDR, buffer, length, false);
// }

// bool icm20948_data_ready() {
//     uint8_t status = icm20948_read_register(INT_STATUS_1);
//     return (status & 0x01);  // Bit 0 = 1 when new data is available
// }

// void icm20948_set_bank(uint8_t bank) {
//     icm20948_write_register(REG_BANK_SEL, (bank << 4));
// }

// void icm20948_init() {
//     uint8_t who_am_i = icm20948_read_register(WHO_AM_I);
//     if (who_am_i != ICM_20948_WHOAMI_EXPECTED) {
//         printf("Error: ICM-20948 not detected! (Read: 0x%X)\n", who_am_i);
//         return;
//     }
//     printf("ICM-20948 detected! WHO_AM_I = 0x%X\n", who_am_i);

//     icm20948_write_register(PWR_MGMT_1, 0x80);
//     sleep_ms(50);  

//     icm20948_write_register(PWR_MGMT_1, 0x01);
//     icm20948_write_register(PWR_MGMT_2, 0x00);

//     icm20948_set_bank(0);

//     icm20948_write_register(LP_CONFIG, 0x00);
//     icm20948_write_register(ACCEL_CONFIG, 0x00);  // ±2g
//     icm20948_write_register(GYRO_CONFIG_1, 0x00); // ±250°/s

//     printf("ICM-20948 Initialization Complete!\n");
// }

// void icm20948_read_sensor_data() {
//     icm20948_set_bank(0);

//     uint64_t timestamp_us = time_us_64();
//     float timestamp_ms = timestamp_us / 1000.0;

//     if (!icm20948_data_ready()) {
//         printf("No new data available...\n");
//         return;
//     }

//     uint8_t data[12];
//     icm20948_read_bytes(ACCEL_XOUT_H, data, 12);

//     int16_t accel_x = (int16_t)((data[0] << 8) | data[1]);
//     int16_t accel_y = (int16_t)((data[2] << 8) | data[3]);
//     int16_t accel_z = (int16_t)((data[4] << 8) | data[5]);

//     int16_t gyro_x = (int16_t)((data[6] << 8) | data[7]);
//     int16_t gyro_y = (int16_t)((data[8] << 8) | data[9]);
//     int16_t gyro_z = (int16_t)((data[10] << 8) | data[11]);

//     float accel_x_g = accel_x / 16384.0f;
//     float accel_y_g = accel_y / 16384.0f;
//     float accel_z_g = accel_z / 16384.0f;

//     float gyro_x_dps = gyro_x / 131.0f;
//     float gyro_y_dps = gyro_y / 131.0f;
//     float gyro_z_dps = gyro_z / 131.0f;

//     printf("[%.3f ms] Accel (g): X=%.3f, Y=%.3f, Z=%.3f\n", timestamp_ms, accel_x_g, accel_y_g, accel_z_g);
//     printf("[%.3f ms] Gyro (°/s): X=%.3f, Y=%.3f, Z=%.3f\n", timestamp_ms, gyro_x_dps, gyro_y_dps, gyro_z_dps);
// }

// void icm20948_enable_interrupt() {
//     icm20948_write_register(INT_ENABLE_1, 0x01);  // Enable RAW_DATA_RDY_INT
//     icm20948_write_register(INT_PIN_CFG, 0x32);   // Active High, Push-Pull

//     // Configure GPIO for interrupt
//     gpio_set_dir(INT1_PIN, GPIO_IN);
//     gpio_pull_up(INT1_PIN);
//     gpio_set_irq_enabled_with_callback(INT1_PIN, GPIO_IRQ_EDGE_RISE, true, &icm20948_irq_handler);

//     // // Debugging
//     // uint8_t int_enable = icm20948_read_register(INT_ENABLE_1);
//     // uint8_t pin_cfg = icm20948_read_register(INT_PIN_CFG);
//     // printf("Interrupt Config: INT_ENABLE_1 = 0x%02X, INT_PIN_CFG = 0x%02X\n", int_enable, pin_cfg);
// }


// // void icm20948_irq_handler(uint gpio, uint32_t events) {
// //     if (gpio == INT1_PIN) {
// //         uint8_t status = icm20948_read_register(INT_STATUS_1);

// //         printf("Interrupt Triggered! INT_STATUS_1 = 0x%02X\n", status);
        
// //         bool data_ready = icm20948_data_ready();
// //         printf("icm20948_data_ready() = %d\n", data_ready);

// //         // if (status & 0x01) {
// //         //     icm20948_read_sensor_data();  // Read IMU data
// //         // }
// //     }
// // }


// void icm20948_irq_handler(uint gpio, uint32_t events) {
//     if (gpio == INT1_PIN) {
//         uint8_t status = icm20948_read_register(INT_STATUS_1);
//         printf("Interrupt Triggered! INT_STATUS_1 = 0x%02X\n", status);

//         imu_data_ready = true;  // Set flag instead of reading data
//     }
// }


// // Task 6 – Working

// // **I2C Configuration**
// // #define ICM_20948_ADDR  0x69  // 7-bit I2C address

// // // **Register Addresses**
// // #define WHO_AM_I        0x00  // Should return 0xEA
// // #define PWR_MGMT_1      0x06  // Power management
// // #define PWR_MGMT_2      0x07  // Enable/disable accel & gyro
// // #define LP_CONFIG       0x05  // Low power mode
// // #define GYRO_CONFIG_1   0x01  // Gyro configuration
// // #define ACCEL_CONFIG    0x14  // Accelerometer configuration
// // #define REG_BANK_SEL    0x7F  // Register bank selection
// // #define INT_STATUS_1    0x1A  // Data Ready status register

// // // **Accelerometer Output Registers**
// // #define ACCEL_XOUT_H    0x2D
// // #define ACCEL_YOUT_H    0x2F
// // #define ACCEL_ZOUT_H    0x31

// // // **Gyroscope Output Registers**
// // #define GYRO_XOUT_H     0x33
// // #define GYRO_YOUT_H     0x35
// // #define GYRO_ZOUT_H     0x37

// // // **WHO_AM_I expected value**
// // #define ICM_20948_WHOAMI_EXPECTED 0xEA

// // **Function Definitions**

// void icm20948_write_register(uint8_t reg, uint8_t value) {
//     uint8_t buffer[2] = { reg, value };
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, buffer, 2, false);
// }

// uint8_t icm20948_read_register(uint8_t reg) {
//     uint8_t value;
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
//     i2c_read_blocking(i2c_default, ICM_20948_ADDR, &value, 1, false);
//     return value;
// }

// void icm20948_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
//     i2c_write_blocking(i2c_default, ICM_20948_ADDR, &reg, 1, true);
//     i2c_read_blocking(i2c_default, ICM_20948_ADDR, buffer, length, false);
// }

// void icm20948_set_bank(uint8_t bank) {
//     icm20948_write_register(REG_BANK_SEL, (bank << 4));
// }

// void icm20948_init() {
//     uint8_t who_am_i = icm20948_read_register(WHO_AM_I);
//     if (who_am_i != ICM_20948_WHOAMI_EXPECTED) {
//         printf("Error: ICM-20948 not detected! (Read: 0x%X)\n", who_am_i);
//         return;
//     }
//     printf("ICM-20948 detected! WHO_AM_I = 0x%X\n", who_am_i);

//     icm20948_write_register(PWR_MGMT_1, 0x80);
//     sleep_ms(50);  

//     icm20948_write_register(PWR_MGMT_1, 0x01);
//     icm20948_write_register(PWR_MGMT_2, 0x00);

//     icm20948_set_bank(0);

//     icm20948_write_register(LP_CONFIG, 0x00);
//     icm20948_write_register(ACCEL_CONFIG, 0x00);  // ±2g
//     icm20948_write_register(GYRO_CONFIG_1, 0x00); // ±250°/s

//     printf("ICM-20948 Initialization Complete!\n");
// }

// void icm20948_read_sensor_data() {
//     icm20948_set_bank(0);

//     uint64_t timestamp_us = time_us_64();
//     float timestamp_ms = timestamp_us / 1000.0;

//     uint8_t data[12];
//     icm20948_read_bytes(ACCEL_XOUT_H, data, 12);

//     int16_t accel_x = (int16_t)((data[0] << 8) | data[1]);
//     int16_t accel_y = (int16_t)((data[2] << 8) | data[3]);
//     int16_t accel_z = (int16_t)((data[4] << 8) | data[5]);

//     int16_t gyro_x = (int16_t)((data[6] << 8) | data[7]);
//     int16_t gyro_y = (int16_t)((data[8] << 8) | data[9]);
//     int16_t gyro_z = (int16_t)((data[10] << 8) | data[11]);

//     float accel_x_g = accel_x / 16384.0f;
//     float accel_y_g = accel_y / 16384.0f;
//     float accel_z_g = accel_z / 16384.0f;

//     float gyro_x_dps = gyro_x / 131.0f;
//     float gyro_y_dps = gyro_y / 131.0f;
//     float gyro_z_dps = gyro_z / 131.0f;

//     printf("[%.3f ms] Accel (g): X=%.3f, Y=%.3f, Z=%.3f\n", timestamp_ms, accel_x_g, accel_y_g, accel_z_g);
//     printf("[%.3f ms] Gyro (°/s): X=%.3f, Y=%.3f, Z=%.3f\n", timestamp_ms, gyro_x_dps, gyro_y_dps, gyro_z_dps);
// }