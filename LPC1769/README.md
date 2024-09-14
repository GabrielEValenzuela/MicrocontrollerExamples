# 🛠️ LPC1769 Microcontroller Examples

## 🌟 Overview

The LPC1769 is a powerful ARM Cortex-M3 based microcontroller developed by NXP. It is designed for high-performance applications that require fast processing and advanced features like USB, CAN, and Ethernet interfaces.

### 🔑 Key Features:

- 🚀 ARM Cortex-M3 core running at up to 120 MHz
- 💾 Up to 512 KB on-chip flash and 64 KB SRAM
- 🧩 Rich peripheral set including USB 2.0, CAN, and Ethernet
- 📊 12-bit ADC, 10-bit DAC, PWM, Timers, and more
- 🖥️ Integrated with LPCXpresso and MCUXpresso development environments

## 📚 Projects

| 📂 Project Name     | 📝 Description                                                                                                                         |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------- |
| [GPIO](GPIO)       | Provides an introduction to GPIO operations, such as configuring pin modes, reading inputs, and controlling outputs.                     |
| [SYSTICK](SYSTICK) | Covers topics such as configuring the SysTick timer, setting up interrupts, and using the timer for precise timing and delay operations. |
| [INT](INT)         | This project demonstrates how to utilize the interrupt capabilities of the LPC1769 microcontroller and priority.                         |
| [EINT](EINT)       | This project demonstrates how to utilize the external interrupt capabilities of the LPC1769                                              |
| [TIMER](TIMER)     | This project demonstrates how to configure and use the timer peripherals of the LPC1769 for various timing and counting applications.    |
| [ADC](ADC)         | This project demonstrates how to configure and use the Analog-to-Digital Converter (ADC) of the LPC1769.                                 |

Each project is designed to showcase a specific feature or peripheral of the LPC1769, providing hands-on experience in embedded systems development.

## 🚀 How to Use

To build and run any of the projects:

1. Navigate to the desired project directory.
2. Run `make` to build the project.
3. Flash the firmware to your LPC1769 board using OpenOCD or a similar tool.
4. Use a terminal or debugger to interact with the project as needed.

For more detailed instructions, please refer to the main [INSTALL](../INSTALL.md) file.
