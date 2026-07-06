# 99x-Arduino / ATmega328P bare-metal benchmark
# Build: make && make flash

CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -mmcu=atmega328p -Os -DF_CPU=16000000UL -Wall
PORT    ?= /dev/ttyACM0
BAUD    ?= 115200

all: bench.hex

bench.elf: bench.c
	$(CC) $(CFLAGS) -o bench.elf bench.c

bench.hex: bench.elf
	$(OBJCOPY) -O ihex -R .eeprom bench.elf bench.hex

flash: bench.hex
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:bench.hex

monitor:
	python3 monitor.py

clean:
	rm -f *.o *.elf *.hex

.PHONY: all flash monitor clean
