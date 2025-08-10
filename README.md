# Digital Phrenic Nerve Project: 
## PNS Device Firmware — Embedded Software for Digital Phrenic Nerve Data Collection

### Overview

This project contains the embedded C firmware for a custom-designed device that collects synchronized physiological data from:

- An Inertial Measurement Unit (IMU) via I²C  
- A CO₂ sensor (SprintIR-WF-20) via UART  
- Logs data to an SD card via SPI using the FatFS library  

The purpose of this device is to record high-resolution motion (IMU) and respiratory gas concentration (CO₂) data from healthy individuals. This dataset will later be used to develop a predictive model (i.e. Machine Learning, Deep Learning) capable of estimating CO₂ levels from IMU data alone. The long-term application is a "Digital Phrenic Nerve" — a closed-loop control signal to autonomously adjust stimulation parameters for phrenic nerve implants in patients with respiratory insufficiency, removing the need for manual caregiver intervention.

### System Architecture

- **Microcontroller**: Raspberry Pi Pico (RP2040)
- **Sensors**:
  - ICM-20948 (IMU) over I²C
  - SprintIR-WF-20 (CO₂ sensor) over UART @ 20 Hz
- **Storage**: SD card via SPI using FatFS
- **I/O**:
  - Start/Stop buttons (GPIO)
  - Status LEDs (for IMU & CO₂ activity)
  - PWM speaker alert on stop
- **Multicore support**: One core dedicated to sensor acquisition, one to SD logging

### File Structure and Descriptions

| File                | Description |
|---------------------|-------------|
| `pns_dev.c`         | Main application logic. Initializes peripherals, handles start/stop logic, orchestrates data acquisition and logging. |
| `co2.c`, `co2.h`    | CO₂ sensor interface. Reads UART data from the SprintIR-WF-20 and parses it into readable values. |
| `imu.c`, `imu.h`    | IMU interface. Initializes ICM-20948 via I²C and handles configuration and data acquisition. |
| `hw_config.c`       | Hardware configuration. Sets up GPIO, PWM for speaker, and LED control. Also includes utility functions for pin configuration. |
| `CMakeLists.txt`    | Build configuration. Specifies project settings, includes source files, links Pico SDK and external FatFS SD card library. |
| `pico_sdk_import.cmake` | Imports Pico SDK for CMake build environment (auto-generated, required for VS Code integration). |

External dependencies:
- [`lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico`](https://github.com/majekw/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico): FatFS implementation for SD card writing over SPI.


### Future Development

The current system is a foundational prototype aimed at collecting synchronized IMU and CO₂ data. Future iterations of this project are envisioned to expand the capabilities of the system significantly, leading to the creation of a closed-loop phrenic nerve control platform. Planned areas of development include:

#### 1. Real-Time Timestamping

Implement a Real-Time Clock (RTC) module to enable precise timestamping of each data sample. This will ensure high temporal fidelity and support future multi-device synchronization and cross-modality data alignment (e.g., video, audio).

#### 2. Wireless Streaming Architecture

Develop a modular wireless architecture where:

- **Peripheral Nodes** (IMU devices) are placed on different parts of the body and stream real-time motion data.
- A **Central Hub Unit** includes the CO₂ sensor and a more powerful microcontroller (e.g., Raspberry Pi 5 or ESP32-S3) responsible for:
  - Synchronizing all data sources
  - Running a predictive inference model (see below)
  - Logging and/or transmitting data to a host system for analysis or control

This will support distributed sensing and real-time multi-channel data fusion.

#### 3. EMG Signal Acquisition

Extend the sensing capabilities of peripheral devices to include surface Electromyography (EMG). EMG signals from respiratory-related muscles (e.g., diaphragm, intercostals) can provide additional neuromuscular information to complement IMU data and improve predictive accuracy.

This will enable the creation of a richer, multimodal dataset for model training.

#### 4. Predictive Model Development

With sufficient training data from healthy subjects, develop a predictive model capable of estimating real-time CO₂ levels using IMU and/or EMG input alone. Candidate model types include:

- Time-series regressors (e.g., LSTM, TCN, Transformers)
- Lightweight embedded ML models for deployment
- Activity-aware models to contextualize predictions

The model will be validated using ground truth CO₂ data, then optimized for real-time inference.

#### 5. Embedded Inference Device

Once a reliable model has been trained and validated:

- A new second-generation device will be developed that omits the CO₂ sensor.
- The trained predictive model will be embedded directly into the firmware or run on a more powerful embedded processor.
- This device will continuously collect IMU (and optionally EMG) data and produce a real-time control signal representing estimated respiratory demand.

This marks the transition from data collection to real-time physiological state estimation.

#### 6. Closed-Loop Phrenic Nerve Control System

The final stage of the project involves integrating the predictive control signal into a next-generation phrenic nerve implant. This system will:

- Receive real-time control data from the inference device
- Dynamically adjust stimulation parameters (e.g., frequency, pulse width, amplitude)
- Operate autonomously without manual tuning or caregiver input

This represents a novel closed-loop neuromodulation system for patients with impaired respiratory function.

---

This roadmap highlights the evolution from a data acquisition platform to a real-time, intelligent therapeutic system capable of autonomously regulating respiratory support via targeted neuromodulation.


### Build Instructions

#### Prerequisites

- Raspberry Pi Pico SDK installed and configured
- CMake ≥ 3.13
- GNU Arm Toolchain
- FatFS library cloned into `lib/`

#### Compile

```bash
mkdir build
cd build
cmake ..
make
