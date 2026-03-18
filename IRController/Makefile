CC = arm-eabi-gcc
OBJCOPY = arm-eabi-objcopy
FLAGS = -mthumb -mcpu=cortex-m4
CPPFLAGS = -DSTM32F411xE
CFLAGS = $(FLAGS) -Wall -g -DNDEBUG \
				  -O2 -ffunction-sections -fdata-sections \
				  -I/opt/arm/stm32/inc \
				  -I/opt/arm/stm32/CMSIS/Include \
				  -I/opt/arm/stm32/CMSIS/Device/ST/STM32F4xx/Include \
				  -I../libs
LDFLAGS = $(FLAGS) -Wl,--gc-sections -nostartfiles\
				   -L/opt/arm/stm32/lds -Tstm32f411re.lds

vpath %.c /opt/arm/stm32/src

TARGET = IRController
OBJECTS = ${TARGET}.o startup_stm32.o delay.o gpio.o

.SECONDARY: $(TARGET).elf $(OBJECTS)
all: $(TARGET).bin
preproc: $(TARGET).pre

%.elf : $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

%.bin : %.elf
	$(OBJCOPY) $< $@ -O binary

%.pre :
	$(CC) $(CFLAGS) -E $(TARGET).c -o $@

clean :
	rm -f *.bin *.elf *.hex *.d *.o *.bak *~