// Libraries
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "imu.h"
#include "co2.h"
#include "sd_card.h"
#include "ff.h"

// GPIO pins
#define IMU_LED_PIN         26
#define CO2_LED_PIN         27
#define SD_LED_PIN          28
#define START_PB            6
#define STOP_PB             7

// Timing
#define IMU_FREQ_US         10000  // 10ms = 100Hz
#define DEBOUNCE_DELAY_US   50000  // 50ms debounce for stop button

// Buffers
#define BUFFER_LINES 500
#define LINE_LENGTH 128

char buffer_A[BUFFER_LINES][LINE_LENGTH];
char buffer_B[BUFFER_LINES][LINE_LENGTH];
volatile bool buffer_A_ready = false;
volatile bool buffer_B_ready = false;
volatile bool buffer_A_writing = false;
volatile bool buffer_B_writing = false;
volatile int buffer_index = 0;
volatile bool writing_buffer_A = true;

// Global Variables
FATFS fs;
FIL fil;
FRESULT fr;
char log_filename[20];

volatile float accel_x, accel_y, accel_z;
volatile float gyro_x, gyro_y, gyro_z;
volatile int co2_value;
volatile bool imu_data_ready = false;
volatile bool latest_co2_available = false;
volatile uint64_t imu_timestamp_us = 0;
volatile uint64_t start_time = 0;
volatile bool is_collecting = false;
volatile uint64_t last_stop_press = 0;
volatile bool stop_requested = false;
volatile int imu_sample_counter = 0;
volatile uint64_t next_imu_alarm_us = 0;

// Function Prototypes
void io_init(void);
void core1_entry();
void imu_timer_callback(__unused uint alarm_num);
bool co2_timer_callback(struct repeating_timer *t);
void stop_button_callback(uint gpio, uint32_t events);
void close_sd_card(void);
void generate_unique_filename(char *filename, size_t size);

// MAIN
int main() {
    stdio_init_all();
    for (int i = 5; i > 0; i--) {
        printf("USB setup... %d\n", i);
        sleep_ms(1000);
    }

    // Init
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    io_init();
    icm20948_init();
    co2_init();

    // Set up CO₂ Timer (still software timer at 20Hz)
    struct repeating_timer co2_timer;
    add_repeating_timer_ms(-50, co2_timer_callback, NULL, &co2_timer);

    // Start multicore
    multicore_launch_core1(core1_entry);

    // Mount SD card
    while ((fr = f_mount(&fs, "0:", 1)) != FR_OK) {
        printf("Mount failed (%d), retrying...\n", fr);
        gpio_put(SD_LED_PIN, !gpio_get(SD_LED_PIN));
        sleep_ms(50);
    }
    printf("SD card mounted!\n");

    // Create file
    generate_unique_filename(log_filename, sizeof(log_filename));
    fr = f_open(&fil, log_filename, FA_WRITE | FA_OPEN_APPEND);
    if (fr != FR_OK) {
        printf("File open failed (%d)\n", fr);
        return 1;
    }
    if (f_size(&fil) == 0) {
        f_printf(&fil, "Time(ms),Acc_x,Acc_y,Acc_z,Gyro_x,Gyro_y,Gyro_z,CO2\n");
        f_sync(&fil);
    }

    // Startup LEDs
    printf("Waiting for Start Button...\n");
    while (gpio_get(START_PB)) {
        gpio_put(SD_LED_PIN, true);
        gpio_put(IMU_LED_PIN, true);
        gpio_put(CO2_LED_PIN, true);
    }
    gpio_put(SD_LED_PIN, false);
    gpio_put(IMU_LED_PIN, false);
    gpio_put(CO2_LED_PIN, false);
    is_collecting = true;
    printf("Starting!\n");

    hardware_alarm_set_callback(0, imu_timer_callback);
    next_imu_alarm_us = time_us_64() + IMU_FREQ_US;
    hardware_alarm_set_target(0, next_imu_alarm_us);
    
    start_time = time_us_64() / 1000;


    // Main Loop
    while (true) {
        if (is_collecting) {
            if (imu_data_ready) {
                imu_data_ready = false;
                imu_sample_counter++;

                if (writing_buffer_A && !buffer_A_writing) {
                    if (imu_sample_counter % 5 == 0 && latest_co2_available) {
                        snprintf(buffer_A[buffer_index], LINE_LENGTH,
                                 "%llu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",
                                 (imu_timestamp_us/1000) - start_time,
                                 accel_x, accel_y, accel_z,
                                 gyro_x, gyro_y, gyro_z,
                                 co2_value);
                        latest_co2_available = false;
                        gpio_put(CO2_LED_PIN, !gpio_get(CO2_LED_PIN));
                    } else {
                        snprintf(buffer_A[buffer_index], LINE_LENGTH,
                                 "%llu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,0\n",
                                 (imu_timestamp_us/1000) - start_time,
                                 accel_x, accel_y, accel_z,
                                 gyro_x, gyro_y, gyro_z);
                    }
                } else if (!writing_buffer_A && !buffer_B_writing) {
                    if (imu_sample_counter % 5 == 0 && latest_co2_available) {
                        snprintf(buffer_B[buffer_index], LINE_LENGTH,
                                 "%llu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",
                                 (imu_timestamp_us/1000) - start_time,
                                 accel_x, accel_y, accel_z,
                                 gyro_x, gyro_y, gyro_z,
                                 co2_value);
                        latest_co2_available = false;
                        gpio_put(CO2_LED_PIN, !gpio_get(CO2_LED_PIN));
                    } else {
                        snprintf(buffer_B[buffer_index], LINE_LENGTH,
                                 "%llu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,0\n",
                                 (imu_timestamp_us/1000) - start_time,
                                 accel_x, accel_y, accel_z,
                                 gyro_x, gyro_y, gyro_z);
                    }
                }

                gpio_put(IMU_LED_PIN, !gpio_get(IMU_LED_PIN));
                buffer_index++;

                if (buffer_index >= BUFFER_LINES) {
                    if (writing_buffer_A) buffer_A_ready = true;
                    else buffer_B_ready = true;
                    writing_buffer_A = !writing_buffer_A;
                    buffer_index = 0;
                }
            }

            if (stop_requested) {
                printf("Stopping...\n");
                if (buffer_index > 0) {
                    if (writing_buffer_A) {
                        for (int i = 0; i < buffer_index; i++) f_printf(&fil, "%s", buffer_A[i]);
                    } else {
                        for (int i = 0; i < buffer_index; i++) f_printf(&fil, "%s", buffer_B[i]);
                    }
                    f_sync(&fil);
                }
                close_sd_card();
                break;
            }

            tight_loop_contents();
        }
    }

    gpio_put(SD_LED_PIN, false);
    gpio_put(IMU_LED_PIN, false);
    gpio_put(CO2_LED_PIN, false);
    printf("Data collection stopped.\n");
    return 0;
}

// Core 1
void core1_entry() {
    while (true) {
        if (buffer_A_ready && !buffer_A_writing) {
            buffer_A_writing = true;
            for (int i = 0; i < BUFFER_LINES; i++) {
                f_printf(&fil, "%s", buffer_A[i]);
            }
            f_sync(&fil);
            buffer_A_ready = false;
            buffer_A_writing = false;
        }
        if (buffer_B_ready && !buffer_B_writing) {
            buffer_B_writing = true;
            for (int i = 0; i < BUFFER_LINES; i++) {
                f_printf(&fil, "%s", buffer_B[i]);
            }
            f_sync(&fil);
            buffer_B_ready = false;
            buffer_B_writing = false;
        }
        sleep_ms(1);
    }
}

// Hardware Alarm 0 callback
void imu_timer_callback(__unused uint alarm_num) {
    imu_timestamp_us = next_imu_alarm_us;  // Use target time as reference
    next_imu_alarm_us += IMU_FREQ_US;      // Schedule next time *based on the last scheduled*
    hardware_alarm_set_target(0, next_imu_alarm_us);

    icm20948_read_sensor_data(&accel_x, &accel_y, &accel_z, &gyro_x, &gyro_y, &gyro_z);
    imu_data_ready = true;
    // return true;
}

// CO2 Timer callback (still repeating timer)
bool co2_timer_callback(struct repeating_timer *t) {
    co2_value = co2_read_sensor_data();
    latest_co2_available = true;
    return true;
}

// Stop button interrupt
void stop_button_callback(uint gpio, uint32_t events) {
    static uint64_t last_press_time_us = 0;
    uint64_t now = time_us_64();
    if ((now - last_press_time_us) > DEBOUNCE_DELAY_US) {
        stop_requested = true;
    }
    last_press_time_us = now;
}

// IO Initialization
void io_init(void) {
    gpio_init(IMU_LED_PIN);
    gpio_set_dir(IMU_LED_PIN, GPIO_OUT);
    gpio_init(CO2_LED_PIN);
    gpio_set_dir(CO2_LED_PIN, GPIO_OUT);
    gpio_init(SD_LED_PIN);
    gpio_set_dir(SD_LED_PIN, GPIO_OUT);
    gpio_init(START_PB);
    gpio_set_dir(START_PB, GPIO_IN);
    gpio_pull_up(START_PB);
    gpio_init(STOP_PB);
    gpio_set_dir(STOP_PB, GPIO_IN);
    gpio_pull_up(STOP_PB);
    gpio_set_irq_enabled_with_callback(STOP_PB, GPIO_IRQ_EDGE_FALL, true, &stop_button_callback);
}

// Close SD card
void close_sd_card(void) {
    f_sync(&fil);
    f_close(&fil);
    f_mount(NULL, "0:", 0);
    printf("SD card unmounted.\n");
}

// Generate unique filename
void generate_unique_filename(char *filename, size_t size) {
    int file_number = 1;
    do {
        snprintf(filename, size, "log_%02d.csv", file_number++);
        FIL temp_fil;
        if (f_open(&temp_fil, filename, FA_READ) != FR_OK) break;
        f_close(&temp_fil);
    } while (file_number < 100);
}