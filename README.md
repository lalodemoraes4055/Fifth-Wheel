# Fifth-Wheel

## Overview

**Fifth-Wheel** is an embedded systems project developed in the C programming language using the **CodeWarrior** development environment. The system is based on the **FRDM-KL25Z** microcontroller and was designed from scratch with the objective of providing an affordable market option with enhanced safety, control, and monitoring capabilities for a fifth-wheel electric platform or vehicle.

## Features

- **Manual control** via a potentiometer (connected to PTB0).
- **Mobile control** using bluetooth through a custom Android application developed in **MIT App Inventor**, enabling:
  - Speed adjustment.
  - Real-time system monitoring (temperature, current, battery level, etc).
- **Sensor integration** for safety and system feedback:
  - **Accelerometer**: For orientation and movement detection.
  - **Temperature sensor**: For overheating detection (PTB1).
  - **Voltage sensor**: For battery monitoring (PTB2).
  - **Current sensor**: For current overload protection (PTB3).
- **Safety systems**:
  - Active buzzer (PTC9) for alert conditions.
  - Battery LED indicator (PTC8).
  - Emergency switch (PTC11).
- **UART communication** with the mobile application through a bluetooth module.
- **PWM and ADC** functionality for precise control and sensing.

## Technologies Used

- **Microcontroller**: NXP FRDM-KL25Z (ARM Cortex-M0+).
- **Language**: C.
- **IDE**: CodeWarrior.
- **Mobile App**: MIT App Inventor (Bluetooth communication).
- **Communication Protocol**: UART (9600 baud rate).

## Functional Summary

The system reads analog values from various sensors via ADC, processes them, and controls a DAC output to modulate speed. It uses PIT timers and TPM input capture to measure parameters like RPM. Safety features automatically cut off the DAC output in case of overheating or overcurrent, indicated by the system's sensors.

The main loop remains idle while all logic is handled via interrupts:

- **PIT Interrupt**: Initiates ADC conversions in sequence.
- **ADC Interrupt**: Reads and processes sensor data, controls safety logic, and communicates values over UART.
- **UART Interrupt**: Receives manual speed commands from the mobile app and parses values to control the DAC.
- **TPM Interrupt**: Calculates frequency (RPM) from an input capture pin.

## Safety Features

- **Overheating Protection**: Disables DAC output when temperature exceeds a set threshold.
- **Low Battery Warning**: Activates LED and buzzer when voltage drops below a threshold.
- **Overcurrent Protection**: Cuts off output to prevent hardware damage.

## Mobile App Functions

- Sends speed commands via Bluetooth.
- Displays current sensor values.
- Sends end-of-command symbol `;` to finalize data input.

## Getting Started

To compile and deploy this project, follow these steps:

1. **Install CodeWarrior** for the FRDM-KL25Z platform.
2. **Load the source code**:
   - Open the project in CodeWarrior.
   - Locate and review `main.c` for customization or understanding.
3. **Compile and flash** the binary to the **FRDM-KL25Z** development board.
4. **Connect the required sensors** to their corresponding pins:
   - PTB0: Potentiometer (manual control)
   - PTB1: Temperature sensor
   - PTB2: Voltage sensor
   - PTB3: Current sensor
   - PTC11: Emergency switch
   - PTC9: Buzzer
   - PTC8: Battery LED indicator
5. **Pair the KL25Z with a mobile phone** via Bluetooth.
6. **Open the MIT App Inventor mobile application**:
   - Begin monitoring real-time sensor data.
   - Control speed and system behavior remotely.
## Future Improvements

- Integrate I2C or SPI-based sensors for higher accuracy.
- Improve the mobile user interface to provide more detailed feedback and control.
- Add data logging functionality to enable performance monitoring and analysis over time.
