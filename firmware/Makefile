ELF_FIRMWARE=build/main.elf
BIN_FIRMWARE=build/main.bin
SYMBOLS_FIRMWARE=build/symbols.txt

ELF_BOOTLOADER=build/bootloader.elf
BIN_BOOTLOADER=build/bootloader.bin
SYMBOLS_BOOTLOADER=build/boot-symbols.txt

ARCH = arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
OPENOCD = openocd
CTAGS = /usr/local/bin/ctags

# Get a rough version number...
VERSION = $(shell git rev-list --all | wc -l)

SRC =    hw.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
	library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
	library/STM32F4xx_StdPeriph_Driver/src/misc.c \
	library/filesystem/ff.c \
	library/filesystem/diskio.c \
	library/filesystem/sdcard.c \
	 tinyprintf.c

SRC_FIRMWARE = $(SRC) main.c third/system_stm32f4xx.c
SRC_BOOTLOADER = $(SRC) bootloader.c third/system_bootloader_stm32f4xx.c

INCLUDESDIR = -I./third/ -I./library/STM32_USB_HOST_Library/Class/MSC/inc -I./library/STM32_USB_HOST_Library/Core/inc -I./library/STM32_USB_OTG_Driver/inc \
	      -I./library/STM32_USB_Device_Library/Core/inc -I./library/STM32_USB_Device_Library/Class/midi/inc -I./library/STM32F4xx_StdPeriph_Driver/inc/ \
	      -I./library/CMSIS/Include/ -I./ -I./library/STM32_USB_Device_Library/Class/msc/inc -I./library/filesystem
SMALLBINOPTS = -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-exceptions  -Wl,--gc-sections

ifeq ($(DEBUG),1)
DEFINE += -DDEBUG
endif

CFLAGS =  -O0 $(INCLUDESDIR) -c -fno-common   -g  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard $(SMALLBINOPTS) $(DEFINE) -fsigned-char
AFLAGS  = -ahls -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS  =   -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16  -nostartfiles

STARTUP = build/startup_firmware.o
OBJDIR = ./build/

OBJTMP_FIRMWARE = $(addprefix $(OBJDIR),$(notdir $(SRC_FIRMWARE:.c=.o)))
OBJ_FIRMWARE = $(OBJTMP_FIRMWARE:.cpp=.o)
LD_FIRMWARE = -Tlinker/stm32_flash.ld 

OBJTMP_BOOTLOADER = $(addprefix $(OBJDIR),$(notdir $(SRC_BOOTLOADER:.c=.o)))
OBJ_BOOTLOADER = $(OBJTMP_BOOTLOADER:.cpp=.o)
LD_BOOTLOADER = -Tlinker/stm32_bootloader.ld 

.PHONY: bootloader

all: $(BIN_FIRMWARE)

bootloader: $(BIN_BOOTLOADER)

flash: $(BIN_FIRMWARE) $(BIN_BOOTLOADER)
#	st-flash write $(BIN_FIRMWARE) 0x08000000
	st-flash write $(BIN_BOOTLOADER) 0x08000000
	st-flash write $(BIN_FIRMWARE)  0x08010000

flash-app: $(BIN_FIRMWARE)
	st-flash write $(BIN_FIRMWARE) 0x08010000

flash-boot: $(BIN_BOOTLOADER)
	st-flash write $(BIN_FIRMWARE) 0x08000000


$(BIN_FIRMWARE): $(ELF_FIRMWARE)
	$(OBJCPY) -O binary $^ $(BIN_FIRMWARE)
	ls -l $(BIN_FIRMWARE)

$(ELF_FIRMWARE): $(OBJ_FIRMWARE) $(STARTUP)
	$(CC) $(LD_FIRMWARE) $(LFLAGS) $^ -o $@  -lnosys -lm
	arm-none-eabi-nm -l -S -n $(ELF_FIRMWARE) > $(SYMBOLS_FIRMWARE)
	arm-none-eabi-readelf -S $(ELF_FIRMWARE)

$(ELF_FIRMWARE_O): $(OBJ_FIRMWARE) $(STARTUP)
	@$(CC) $(LD_FIRMWARE) $(LFLAGS) $^ -o $@
	arm-none-eabi-nm -l -S -n $(ELF_FIRMWARE_O) > $(SYMBOLS_FIRMWARE_O)
	arm-none-eabi-readelf -S $(ELF_FIRMWARE_O)

$(BIN_BOOTLOADER): $(ELF_BOOTLOADER)
	$(OBJCPY) -O binary $^ $(BIN_BOOTLOADER)
	ls -l $(BIN_BOOTLOADER)

$(ELF_BOOTLOADER): $(OBJ_BOOTLOADER) $(STARTUP)
	$(CC) $(LD_BOOTLOADER) $(LFLAGS) $^ -o $@  -lnosys -lm
	arm-none-eabi-nm -l -S -n $(ELF_BOOTLOADER) > $(SYMBOLS_BOOTLOADER)
	arm-none-eabi-readelf -S $(ELF_BOOTLOADER)

$(STARTUP): linker/startup_stm32f4xx.s
	$(AS) $(AFLAGS) -o $(STARTUP) linker/startup_stm32f4xx.s > linker/startup_stm32f4xx.lst

build/%.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: %.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: third/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: library/STM32_USB_OTG_Driver/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: library/STM32_USB_HOST_Library/Core/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: library/STM32_USB_HOST_Library/Class/MSC/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: library/STM32_USB_Device_Library/Core/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: library/STM32_USB_Device_Library/Class/msc/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: library/STM32F4xx_StdPeriph_Driver/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: library/filesystem/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@

build/%.o: midipal/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@


clean:
	@ echo ".clean"
	rm -f *.lst build/*.o

cleanall:
	@ echo ".clean all"
	rm -rf *.lst build/*
