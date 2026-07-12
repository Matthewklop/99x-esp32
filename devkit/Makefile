# Makefile for ESP32 RAM execution workflow
#
# Targets:
#   make ram    — build .s file → .bin ready for RAM loading
#   make run    — build + load to ESP32 + read UART output
#   make flash  — build + write to flash (for boot)
#   make clean  — remove build artifacts
#
# Required: a single .s file at src/$(SRC).s
# Uses trampoline.s + src/ram.ld automatically.
#
# Quick start:
#   SRC=flash_min3 make run PORT=/dev/ttyUSB2

AS      = /home/s1/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-as
LD      = /home/s1/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-ld
CC      = /home/s1/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-gcc
OBJCOPY = /home/s1/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-objcopy
LIBGCC  = /home/s1/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/lib/gcc/xtensa-esp-elf/13.2.0/esp32/libgcc.a

PORT    ?= /dev/ttyUSB2
SRC     ?= flash_min3
RAM_LD  ?= src/ram.ld

# Detect source type
SRC_S   = src/$(SRC).s
SRC_C   = src/$(SRC).c

# Output files
OBJ     = $(SRC).o
ELF     = $(SRC).elf
BIN     = $(SRC)_ram.bin

all: $(BIN)

# Assemble trampoline (3-byte jump stub)
trampoline.o: src/trampoline.s
	$(AS) --abi-call0 -o trampoline.o src/trampoline.s

# Compile or assemble the main firmware
$(SRC).o: $(SRC_S)
	$(AS) --abi-call0 -o $@ $(SRC_S)

$(SRC).o: $(SRC_C)
	$(CC) -c -O2 -nostdlib -nostartfiles -ffreestanding -mabi=call0 -o $@ $(SRC_C)

# Link with trampoline first, then literals, then code
$(ELF): trampoline.o $(OBJ)
	$(LD) -T $(RAM_LD) -o $(ELF) trampoline.o $(OBJ) $(LIBGCC)

# Generate RAM-only binary image
$(BIN): $(ELF)
	python3 -m esptool --chip esp32 elf2image --ram-only-header -o $(BIN) $(ELF)
	@echo "---"
	@python3 -c "import esptool.bin_image; img=esptool.bin_image.LoadFirmwareImage('esp32', open('$(BIN)','rb').read()); print(f'Binary: {len(img.segments)} seg(s), entry=0x{img.entrypoint:08x}, {sum(len(s.data) for s in img.segments)} bytes')"
	@ls -la $(BIN)

# Load to RAM and read UART output
run: $(BIN)
	python3 ramrun.py $(PORT) $(BIN)

# Legacy: build for flash boot (not RAM)
flash_bin: $(ELF)
	python3 -m esptool --chip esp32 elf2image --flash-mode dio --flash-size 4MB -o $(SRC).bin $(ELF)

flash: flash_bin
	python3 -m esptool --chip esp32 --port $(PORT) --baud 115200 write-flash 0x1000 $(SRC).bin

# Show disassembly of the ELF
dis: $(ELF)
	$(OBJCOPY) -O binary $(ELF) /dev/stdout | xxd | head -20
	@echo "---"
	$(LD) --print-map -T $(RAM_LD) trampoline.o $(OBJ) 2>/dev/null | grep -E "\.text|\.literal|\.rodata|_start|trampoline" | head -20

# Quick test: monitor UART without loading anything
monitor:
	python3 -c "import serial, sys; s=serial.Serial('$(PORT)',115200,timeout=10); [sys.stdout.buffer.write(s.read(4096)) or sys.stdout.flush() for _ in range(100)]"

clean:
	rm -f *.o *.elf *.bin *_ram.bin

.PHONY: all run flash clean dis monitor flash_bin
