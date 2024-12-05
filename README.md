# 🚀 Microcontroller Examples

## 📝 Overview

This repository contains a collection of examples and tutorials for working with the **LPC1769** and **STM32** microcontrollers. These examples are designed to help you understand and implement embedded systems using these platforms.

## 📁 Repository Structure

- **LPC1769/**: Contains examples specific to the LPC1769 microcontroller. Each subfolder represents a module.
- **STM32/**: Contains examples for the STM32F103C8T6 microcontroller. Each subfolder represents a module.
- **EXAMS/**: Contains exams for the LPC1769 microcontroller. Each subfolder represents a year and exam number.
- **FINAL_PROJECTS/**: Contains the final projects 2024 for the LPC1769 microcontroller.

## 📦 Libraries and Frameworks

### 📚 CMSIS (Cortex Microcontroller Software Interface Standard)

CMSIS is a vendor-independent hardware abstraction layer for the Cortex-M processor series. It provides a standardized API for peripheral access, system initialization, and more. In this repository, CMSIS is used along with CMSIS drivers for the LPC1769 microcontroller to facilitate low-level programming and hardware abstraction.

### 📚 libopencm3

libopencm3 is an open-source library that provides a unified API for a variety of ARM Cortex-M microcontrollers, including the STM32 series. It abstracts the hardware-specific details, making it easier to write portable and maintainable code. In this repository, libopencm3 is used for the STM32F103C8T6 microcontroller to manage peripheral interactions, system initialization, and more.

## 🔧 Considerations and Prerequisites

Before compiling these projects, ensure you have the following:

- 🛠️ **ARM GCC Toolchain**
- 🐞 **OpenOCD** (for debugging)
- 💻 **A compatible IDE or text editor** (e.g., VSCode, Eclipse, MCUExpresso for NXP microcontroller)
- ⚙️ **Make or CMake** (depending on your build preference)

For installation instructions, please check the [INSTALL](INSTALL) file in the repository.

> [!CAUTION]
> For LPC1769 projects, we utilize `bare metal` programming. This means we develop a minimal bootloader to initialize the clock, memory, and processor before proceeding to the main program.  
> To make it work with MCUExpresso, please copy the source files and header files from the `src` and `include` directories to your projects, except for `newlib_stubs.c`.

## 🏃‍♂️ Running the Examples

After building, flash the firmware onto your LPC1769 or STM32 board using the appropriate tools (e.g., OpenOCD, ST-Link Utility).

## 🤝 Contributing

Contributions are welcome! Please fork this repository and submit a pull request with your changes.

## 📄 License

This repository is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.
