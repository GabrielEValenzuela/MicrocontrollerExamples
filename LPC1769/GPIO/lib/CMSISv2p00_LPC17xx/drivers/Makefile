# Compiler and Archiver commands
# CC: The compiler command used to compile C source files.
# AR: The archiver command used to create and manage library files (archives).
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar

###########################################

# vpath directive specifies the search path for source files.
# It tells make to look for .c files in the Src directory.
vpath %.c src 

# TARGET: Defines the name of the output file, which in this case is a static library named liblpcdriver.a.
TARGET = liblpcdriver.a
 
# Compiler Flags
# CFLAGS: Basic flags for compiling C files.
CFLAGS = -g -O2 -Wall

# Define device-specific flags
# -D__weak, -D__packed: Preprocessor macros to specify weak and packed attributes in the code.
# PACK_STRUCT_END: Macro for packing structures, ensuring no padding between members.
# ALIGN_STRUCT_END: Macro to align structures on a 4-byte boundary.
# -D__USE_CMSIS: Include CMSIS (Cortex Microcontroller Software Interface Standard) support.
# -mlittle-endian: Specifies little-endian byte ordering.
# -mthumb: Enables the Thumb instruction set (compact version of ARM instruction set).
# -mcpu=cortex-m3: Specifies the target CPU architecture (Cortex-M3).
# -mthumb-interwork: Supports interworking between ARM and Thumb code.
# -fno-builtin: Disables the use of built-in functions provided by the compiler.
# -mfloat-abi=soft: Uses software floating-point operations instead of hardware floating-point.
# -ffunction-sections, -fdata-sections: Places each function or data item in its own section.
# -fmessage-length=0: Controls the length of error messages produced by the compiler.
# -funsigned-char: Treats `char` as `unsigned char`.
CFLAGS += -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"
CFLAGS += -D PACK_STRUCT_END=__attribute\(\(packed\)\) 
CFLAGS += -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)	
CFLAGS += -D__USE_CMSIS
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
CFLAGS += -fno-builtin -mfloat-abi=soft	-ffunction-sections -fdata-sections -fmessage-length=0 -funsigned-char

# Include Paths
# -I flags specify directories to search for header files.
CFLAGS += -I./include
CFLAGS += -I../include
CFLAGS += -I../../../include/

# SRCS: Lists all the source files to be compiled into object files.
SRCS = lpc17xx_libcfg_default.c \
	 lpc17xx_pinsel.c	\
	 lpc17xx_gpio.c \
	 lpc17xx_uart.c \
	 lpc17xx_i2c.c \
	 lpc17xx_spi.c \
	 lpc17xx_clkpwr.c

# OBJS: Converts each source file name (.c) into its corresponding object file name (.o).
OBJS = $(SRCS:.c=.o)

# .PHONY: Declares targets that don't represent actual files to avoid conflicts.
.PHONY: $(TARGET)

# Default target: Builds the static library.
all: $(TARGET)

# Compilation Rule
# %.o : %.c: This pattern rule tells make how to generate an object file (.o) from a source file (.c).
# The command compiles the source file ($^) into an object file ($@) using the defined compiler (CC) and flags (CFLAGS).
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

# Linking (Library Creation)
# $(TARGET): $(OBJS): This target creates the static library (liblpcdriver.a) by archiving the object files (OBJS).
# The command uses the archiver (AR) to create or update the library file ($@, which is $(TARGET)) with the object files (OBJS).
$(TARGET): $(OBJS)
	$(AR) -r $@ $(OBJS)

# Cleaning Up
# clean: This target removes the compiled object files and the generated static library.
# The rm -f command forcefully removes (-f) all object files (OBJS) and the static library (TARGET).
clean:
	rm -f $(OBJS) $(TARGET)
