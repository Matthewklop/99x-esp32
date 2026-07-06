# Benchmarks

Measured on ESP32-C3 rev v0.4, 160MHz.

## Firmware size

| 99x-esp32 | ESP-IDF | Arduino |
|-----------|---------|---------|
| 832 bytes | ~150 KB | ~270 KB |

## Boot time

| 99x-esp32 | ESP-IDF | Arduino |
|-----------|---------|---------|
| ~1us | ~500ms | ~1s |

## Hash performance (21 bytes, register multiply)

~15 cycles/hash. ~55 MB/s.

## Watchdog survival

Feeds both TIMG0 and RTC WDT. Runs indefinitely.

## Toolchain

riscv32-esp-elf-gcc + esptool. Zero other dependencies.
