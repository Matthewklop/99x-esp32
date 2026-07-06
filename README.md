# 99x-esp32

Bare-metal ESP32-C3 firmware. ~800 bytes. No frameworks. No OS. No dependencies.

Minimal template for running RISC-V bare metal on ESP32-C3 with USB serial, GPIO blink, and watchdog feeding.

## Quick start

```
riscv32-esp-elf-gcc -c -Os -nostdlib -nostartfiles -ffreestanding -o main.o src/main.c
riscv32-esp-elf-ld -T src/link.ld -o firmware.elf main.o
python3 -m esptool --chip esp32c3 elf2image --flash-mode dio --flash-size 4MB -o firmware.bin firmware.elf
python3 -m esptool --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write-flash 0x0 firmware.bin
```

## Size

832 bytes.

## Benchmarks

See benchmarks/BENCHMARKS.md
