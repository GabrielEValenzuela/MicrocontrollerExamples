# A makefile contains a sequence of entries, each of which specifies a build target, some dependencies, and the build scripts of commands to be executed.
# This Makefile is based on the one @vpcola available on: https://github.com/vpcola/LPC1769/tree/master
# We made some changes to make it work with our project and to make it more readable.

# We define the source files that will be compiled and linked into the final binary.
# Add all the source files here, ending with \ to continue on the next line.
SRCS =	newlib_stubs.c \
		system_LPC17xx.c \
		main.c
 
	 
# Define the name of the project
# This will be the name of the final binary file

PROJ_NAME=gates-of-survival

# You should not need to change anything below this line =D

###################################################

# Some nice colors
CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
LINKCOLOR="\033[34;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

QUIET_CC      = @printf '    %b %b\n' ${CCCOLOR}CC${ENDCOLOR} ${SRCCOLOR}$@${ENDCOLOR} 1>&2;
QUIET_LINK    = @printf '    %b %b\n' ${LINKCOLOR}LINK${ENDCOLOR} ${BINCOLOR}$@${ENDCOLOR} 1>&2;
QUIET_NOTICE  = @printf '%b' ${MAKECOLOR} 1>&2;
QUIET_ENDCOLOR= @printf '%b' ${ENDCOLOR} 1>&2;

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
OBJSIZE=arm-none-eabi-size

PRETTY_CC=${QUIET_CC}${CC}

CFLAGS  = -g  -O0 -Wall -Tlpc17xx.ld
# Define the device we are using
CFLAGS += -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"
CFLAGS += -D PACK_STRUCT_END=__attribute\(\(packed\)\) 
CFLAGS += -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)	
CFLAGS += -D__USE_CMSIS
CFLAGS += -mthumb -mcpu=cortex-m3 
CFLAGS += -fno-builtin -mfloat-abi=soft	-ffunction-sections -fdata-sections -fmessage-length=0 -funsigned-char
 
ODFLAGS	= -x
LDFLAGS += -Wl,-Map,$(PROJ_NAME).map

###################################################

ROOT=$(shell pwd)
BUILD_DIR=$(ROOT)/build

# Create the build directory if it doesn't exist
$(shell mkdir -p $(BUILD_DIR))

vpath %.c src
vpath %.c $(ROOT)/lib/CMSISv2p00_LPC17xx/src 
vpath %.c $(ROOT)/lib/CMSISv2p00_LPC17xx/drivers/src

CFLAGS += -I$(ROOT)/include 
CFLAGS += -I$(ROOT)/lib/CMSISv2p00_LPC17xx/include
CFLAGS += -I$(ROOT)/lib/CMSISv2p00_LPC17xx/drivers/include

LIBS = -L$(ROOT)/lib/CMSISv2p00_LPC17xx/drivers -llpcdriver

# Modify the OBJS to place object files in the build directory
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

###################################################

.PHONY: drivers proj

all: drivers proj

drivers:
	$(MAKE) -C $(ROOT)/lib/CMSISv2p00_LPC17xx/drivers
	${QUIET_NOTICE}
	@echo "Done building library for drivers"
	${QUIET_ENDCOLOR}
	
proj: 	$(BUILD_DIR)/$(PROJ_NAME).elf

$(BUILD_DIR)/$(PROJ_NAME).elf: $(OBJS)
	$(PRETTY_CC) $(CFLAGS) $^ -o $@ $(LIBS) $(LDFLAGS)
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/$(PROJ_NAME).bin
	$(OBJDUMP) -x $@ > $(BUILD_DIR)/$(PROJ_NAME).dmp
	@$(OBJSIZE) -d $@
	@echo " "
	${QUIET_NOTICE}
	@echo "Done building ${PROJ_NAME}"
	${QUIET_ENDCOLOR}

# Compile source files to the build directory
$(BUILD_DIR)/%.o: %.c
	$(PRETTY_CC) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C $(ROOT)/lib/CMSISv2p00_LPC17xx/drivers clean
	rm -f $(BUILD_DIR)/$(PROJ_NAME).elf
	rm -f $(BUILD_DIR)/$(PROJ_NAME).hex
	rm -f $(BUILD_DIR)/$(PROJ_NAME).bin
	rm -f $(BUILD_DIR)/$(PROJ_NAME).dmp
	rm -f $(BUILD_DIR)/$(PROJ_NAME).map
	rm -f $(OBJS)
