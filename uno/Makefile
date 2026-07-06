# 99x-Arduino / ATmega328P bare-metal benchmark
# Build: make && make flash

CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -mmcu=atmega328p -Os -DF_CPU=16000000UL -Wall
PORT    ?= /dev/ttyACM0
BAUD    ?= 115200

all: bench.hex compute.hex

bench.elf: bench.c
	$(CC) $(CFLAGS) -o bench.elf bench.c

bench.hex: bench.elf
	$(OBJCOPY) -O ihex -R .eeprom bench.elf bench.hex

compute.elf: compute.c
	$(CC) $(CFLAGS) -o compute.elf compute.c

compute.hex: compute.elf
	$(OBJCOPY) -O ihex -R .eeprom compute.elf compute.hex

flash: compute.hex
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:compute.hex

flash-bench: bench.hex
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:bench.hex

flash-spi: spi_bitbang.hex
	avrdude -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:spi_bitbang.hex

spi_bitbang.elf: spi_bitbang.c
	$(CC) $(CFLAGS) -o spi_bitbang.elf spi_bitbang.c

spi_bitbang.hex: spi_bitbang.elf
	$(OBJCOPY) -O ihex -R .eeprom spi_bitbang.elf spi_bitbang.hex

monitor:
	python3 monitor.py

clean:
	rm -f *.o *.elf *.hex

.PHONY: all flash monitor clean
