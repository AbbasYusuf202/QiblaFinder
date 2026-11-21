
TARGET = firmware

CC = /mnt/storage/Documents/workspace/qiblafinder/Step1/toolchain/bin/arm-none-eabi-gcc
OBJCOPY = /mnt/storage/Documents/workspace/qiblafinder/Step1/toolchain/bin/arm-none-eabi-objcopy
SIZE = /mnt/storage/Documents/workspace/qiblafinder/Step1/toolchain/bin/arm-none-eabi-size

OPENOCD = /usr/bin/openocd
OPENOCD_INTERFACE = interface/stlink.cfg
OPENOCD_TARGET = target/stm32f4x.cfg

FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp \
        -Wall -Wextra -std=gnu11 -O0 -g3 -ffreestanding \
        -ffunction-sections -fdata-sections \
        -ICMSIS \
		-IIncludes \
        -DSTM32F411xE \
        -T STM32F411VEHX_FLASH.ld -Wl,--gc-sections \
        -Wl,-Map=$(TARGET).map \
        --specs=nosys.specs -nostartfiles

C_SOURCES = $(wildcard *.c)
ASM_SOURCES = startup_stm32f411vehx.s
OBJECTS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

all: $(TARGET).elf $(TARGET).bin

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(FLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(FLAGS) $^ -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).elf
	$(OPENOCD) -f $(OPENOCD_INTERFACE) \
	           -f $(OPENOCD_TARGET) \
	           -c "program $(TARGET).elf verify reset exit"

clean:
	rm -f $(OBJECTS) *.elf *.bin *.map

.PHONY: all clean flash
