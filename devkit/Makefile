<<<<<<< HEAD
# 99x-esp32: quick start, benchmarks, register map, watchdog trick
# Build: make && make flash

CC      = riscv32-esp-elf-gcc
LD      = riscv32-esp-elf-ld
CFLAGS  = -c -Os -nostdlib -nostartfiles -ffreestanding
LFLAGS  = -T src/link.ld
PORT    ?= /dev/ttyACM0
BAUD    ?= 460800
CHIP    ?= esp32c3

all: firmware.bin

main.o: src/main.c
	$(CC) $(CFLAGS) -o main.o src/main.c

firmware.elf: main.o
	$(LD) $(LFLAGS) -o firmware.elf main.o
=======
CC      = xtensa-esp32-elf-gcc
AS      = xtensa-esp32-elf-as
LD      = xtensa-esp32-elf-ld
OBJCOPY = xtensa-esp32-elf-objcopy
CFLAGS  = -c -Os -nostdlib -nostartfiles -ffreestanding
PORT    ?= /dev/ttyUSB0
BAUD    ?= 460800
CHIP    ?= esp32

all: firmware.bin

main.o: src/main.s
	$(AS) -o main.o src/main.s

firmware.elf: main.o
	$(LD) -T src/link.ld -o firmware.elf main.o
>>>>>>> 4f761e4 (ESP32 DevKit bare metal: blue LED, 320 bytes, dual-core WIP)

firmware.bin: firmware.elf
	python3 -m esptool --chip $(CHIP) elf2image --flash-mode dio --flash-size 4MB -o firmware.bin firmware.elf

flash: firmware.bin
<<<<<<< HEAD
	python3 -m esptool --chip $(CHIP) --port $(PORT) --baud $(BAUD) write-flash 0x0 firmware.bin

monitor:
	python3 -c "import serial,time; s=serial.Serial('$(PORT)',115200,timeout=0); s.setDTR(0);s.setRTS(0);[print(s.read(999).decode(errrs='replace'),end='')or time.sleep(.1) for _ in range(100)]"
=======
	python3 -m esptool --chip $(CHIP) --port $(PORT) --baud $(BAUD) write-flash 0x1000 firmware.bin
>>>>>>> 4f761e4 (ESP32 DevKit bare metal: blue LED, 320 bytes, dual-core WIP)

clean:
	rm -f *.o *.elf *.bin

<<<<<<< HEAD
.PHONY: all flash monitor clean
=======
.PHONY: all flash clean
>>>>>>> 4f761e4 (ESP32 DevKit bare metal: blue LED, 320 bytes, dual-core WIP)
