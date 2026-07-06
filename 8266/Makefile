# 99x-esp32 / ESP8266 bare metal C
# Blue LED (GPIO2) + OLED (SH1106, I2C GPIO3=SDA GPIO4=SCL)
# Build: make && make flash

CC      = xtensa-lx106-elf-gcc
LD      = xtensa-lx106-elf-ld
OBJCOPY = xtensa-lx106-elf-objcopy
CFLAGS  = -c -Os -nostdlib -nostartfiles -ffreestanding -mno-serialize-volatile
PORT    ?= /dev/ttyUSB0
BAUD    ?= 460800
CHIP    ?= esp8266

all: firmware.bin

main.o: main.c
	$(CC) $(CFLAGS) -o main.o main.c

firmware.elf: main.o
	$(LD) -T link.ld -o firmware.elf main.o

firmware.bin: firmware.elf
	python3 -m esptool --chip $(CHIP) elf2image -o firmware.bin firmware.elf

flash: firmware.bin
	python3 -m esptool --chip $(CHIP) --port $(PORT) --baud $(BAUD) write-flash 0x0 firmware.bin

monitor:
	python3 -c "\
import serial,time,sys; \
s=serial.Serial('$(PORT)',74880,timeout=0.1); \
s.setDTR(0);s.setRTS(0); \
try:\
 while 1:\
  d=s.read(4096);\
  if d: sys.stdout.buffer.write(d); sys.stdout.flush()\
 except: pass"

clean:
	rm -f *.o *.elf *.bin

.PHONY: all flash monitor clean
