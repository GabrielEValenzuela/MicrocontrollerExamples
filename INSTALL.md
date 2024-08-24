# 🛠️ Installation Guide

This guide will help you set up the necessary tools to compile and debug the LPC1769 and STM32 microcontroller projects in this repository.

## 1. Install ARM GCC Toolchain

The ARM GCC Toolchain is required to compile code for ARM-based microcontrollers.

### Linux (Debian)

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi
```

### Windows

1. Download the ARM GCC Toolchain from [ARM Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm).
2. Follow the installation instructions provided on the website.

## 2. Install OpenOCD (for debugging)

OpenOCD is an open-source tool that provides debugging, in-system programming, and boundary-scan testing.

### Linux

```bash
sudo apt install openocd
```

### Windows

1. Download OpenOCD from the [official website](http://openocd.org/).
2. Follow the installation instructions provided on the website.

## 3. Install a Compatible IDE or Text Editor

You can use various IDEs or text editors, but the following are recommended:

- **VSCode**: A lightweight code editor with excellent support for embedded development.
- **MCUExpresso** (NXP-specific): A specialized IDE for NXP microcontrollers, recommended for LPC1769 development.

### VSCode

1. Download and install [VSCode](https://code.visualstudio.com/).
2. Install the following extensions:
   - C/C++ (Microsoft)
   - Search for official [PlatformIO IDE extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
     - Install PlatformIO IDE.
     - VSCode Extensions Manager and PlatformIO IDE auto-installer
     - [Check Quick Start guide](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start) (highly recommended).

### MCUExpresso

1. Download and install [MCUExpresso IDE](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE) from the NXP website.
2. Follow the setup instructions provided by NXP.

## 4. Install Make or CMake

Depending on your build preference, install Make or CMake:

### Make (Linux)

```bash
sudo apt install build-essential  # For Linux
```

### Make (Windows)

1. Install [MinGW](http://www.mingw.org/) and ensure the `make` utility is included.

### CMake

#### Linux

```bash
sudo apt install cmake
```

#### Windows

1. Download and install CMake from the [official website](https://cmake.org/download/).

## 5. Flashing Tools

To flash the firmware onto your microcontroller, you'll need the following tools:

- **ST-Link Utility** (STM32)
- **OpenOCD** (for both LPC1769 and STM32)

### ST-Link Utility (for STM32)

1. Download the ST-Link Utility from the [STMicroelectronics website](https://www.st.com/en/development-tools/stsw-link004.html).
2. Follow the installation instructions provided on the website.

### OpenOCD

Already covered in Step 2.

## 6. Additional Considerations for LPC1769

For LPC1769 projects:

- If using MCUExpresso, you need to copy the source files and header files from the `src` and `include` directories to your project, except for `newlib_stubs.c`.
