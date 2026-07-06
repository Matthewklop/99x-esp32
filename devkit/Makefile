# 99x-esp32 devkit: bare metal ESP32 (Xtensa LX6, dual-core)
# Build: make && make flash

AS      = xtensa-esp32-elf-as
LD      = xtensa-esp32-elf-ld
OBJCOPY = xtensa-esp32-elf-objcopy
PORT    ?= /dev/ttyUSB0
BAUD    ?= 460800
CHIP    ?= esp32

all: firmware.bin

main.o: src/main.s
	$(AS) -o main.o src/main.s

firmware.elf: main.o
	$(LD) -T src/link.ld -o firmware.elf main.o

firmware.bin: firmware.elf
	python3 -m esptool --chip $(CHIP) elf2image --flash-mode dio --flash-size 4MB -o firmware.bin firmware.elf

flash: firmware.bin
	python3 -m esptool --chip $(CHIP) --port $(PORT) --baud $(BAUD) write-flash 0x1000 firmware.bin

clean:
	rm -f *.o *.elf *.bin

.PHONY: all flash clean
