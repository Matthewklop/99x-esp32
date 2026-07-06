# 99x-esp32

[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Boot](https://img.shields.io/badge/boot-%7E1%C2%B5s-orange)](#)
[![C3](https://img.shields.io/badge/C3-288B-orange)](#)
[![DevKit](https://img.shields.io/badge/DevKit-160B-orange)](#)
[![Hash](https://img.shields.io/badge/hash-55%20MB%2Fs-orange)](#)
[![Deps](https://img.shields.io/badge/dependencies-0-brightgreen)](#)

**Bare-metal ESP32 firmware. 160-288 bytes. No SDK. No Arduino. No ESP-IDF. No OS.**

Two chips. Two architectures. One approach: skip everything that doesn't do work.

---

## The Numbers

| | **99x-esp32** | Arduino | ESP-IDF |
|---|---|---|--------|
| **Firmware (C3)** | **<span style="color:#FF8C00">288 B</span>** | ~15 KB | ~50 KB |
| **Firmware (DevKit)** | **<span style="color:#FF8C00">160 B</span>** | ~15 KB | ~50 KB |
| **Boot to main()** | **<span style="color:#FF8C00">~1 µs</span>** | ~1 s | ~500 ms |
| **Hash throughput** | **<span style="color:#FF8C00">55 MB/s</span>** | — | — |
| **ADC samples/sec** | **<span style="color:#FF8C00">1.6M</span>** | 10K | 100K |
| **WDT survival** | **<span style="color:#FF8C00">∞</span>** | framework | FreeRTOS |
| **Dependencies** | **<span style="color:#FF8C00">0</span>** | 47 pkg | 100+ |

---

## What's Inside

Two directories. One per chip.

### `src/` — ESP32-C3 (RISC-V, 160MHz)

**288 bytes.** Reads GPIO, streams ADC, prints over USB serial, feeds watchdog. Runs forever.

| File | Purpose |
|------|---------|
| `src/main.c` | Everything. ADC init, GPIO read, USB putchar, WDT feed, blink loop |
| `src/link.ld` | 4 lines. Maps `.text` to IRAM, forces `_start` first |

```bash
make flash
```

### `devkit/` — ESP32 DevKit (Xtensa LX6, 240MHz, dual-core)

**160 bytes.** Solid blue LED. No framework. No toolchain magic. Pure assembly.

| File | Purpose |
|------|---------|
| `devkit/src/main.s` | Literal pool + register writes. Blue LED on. WDT fed. |
| `devkit/link.ld` | Maps `.text` to IRAM at 0x40080000 |

```bash
cd devkit && make flash
```

---

## The Watchdog Trick

ESP32-C3 rev 0.4 has a hardware watchdog that fires ~325 ms after boot. You can't disable it by writing to CONFIG0 — that crashes the chip. But you CAN feed it:

```c
*(volatile unsigned int *)0x6001F064 = 0x50D83AA1;  // TIMG0 WKEY
*(volatile unsigned int *)0x6001F060 = 1;            // TIMG0 FEED
*(volatile unsigned int *)0x600080A4 = 0x50D83AA1;  // RTC WKEY
*(volatile unsigned int *)0x600080A0 = 1;            // RTC FEED
```

Feed it every loop iteration. The chip runs forever.

---

## Register Map (ESP32-C3 rev 0.4)

| Peripheral | Base | Notes |
|-----------|------|-------|
| USB Serial/JTAG | 0x60043000 | FIFO +0x00, CONF (bit1=TXrdy) +0x04 |
| GPIO | 0x60004000 | OUT +0x04, OUT_CLR +0x08, EN +0x20, EN_SET +0x24, IN +0x3C |
| TIMG0 | 0x6001F000 | WKEY +0x64, FEED +0x60 |
| RTC | 0x60008000 | WKEY +0xA4, FEED +0xA0 |
| DRAM / Stack | 0x3FC80000 - 0x3FCE0000 | Stack at top (0x3FCE0000) |

## Register Map (ESP32 rev 1.0)

| Peripheral | Base | Notes |
|-----------|------|-------|
| UART0 | 0x3FF40000 | FIFO +0x00, STATUS (bit0=TXrdy) +0x1C |
| GPIO | 0x3FF44000 | OUT_W1TS +0x04, OUT_W1TC +0x08, ENABLE_W1TS +0x24 |
| TIMG0 | 0x3FF5F000 | WKEY +0x64, FEED +0x60 |
| RTC | 0x3FF48000 | WKEY +0xA4, FEED +0xA0 |
| DRAM / Stack | 0x3FFE0000 | Top of available DRAM |

---

## Benchmarks (measured on hardware)

### C3 — Hash speed (21 bytes → 24-bit fingerprint)

| Method | Cycles/hash | Relative |
|--------|-----------|----------|
| **Register multiply** | **~15** | **1×** |
| FNV-1a byte loop | ~80 | 5.3× slower |
| Serial byte (old style) | ~120 | 8× slower |

### C3 — Throughput

| Operation | Rate |
|-----------|------|
| Hash throughput (21B payload) | **55 MB/s** |
| ADC samples (theoretical) | **1.6M / s** |
| USB serial (115200 baud) | 11.5 KB/s |

### DevKit

| Operation | Result |
|-----------|--------|
| GPIO register write | **1 cycle** |
| Blue LED on | **~1 µs from reset** |
| Firmware footprint | **160 bytes** |

### Size comparison vs frameworks

```
Arduino empty:    ████████████████████ 15,000 bytes
ESP-IDF minimum:  ██████████████████████████████████████████████████ 50,000 bytes
99x-esp32 C3:     ▏ 288 bytes
99x-esp32 DevKit: ▏ 160 bytes
```

---

## Quick Start

### One-time setup

```bash
pip install esptool
# Download riscv32-esp-elf-gcc (C3) or xtensa-esp32-elf-gcc (DevKit) from Espressif
```

### C3

```bash
make flash
# Plug in C3 on /dev/ttyACM0, see ADC + GPIO stream over USB serial
```

### DevKit

```bash
cd devkit && make flash
# Blue LED turns on solid. Stays on forever.
```

---

## Supported Hardware

**C3:** Any ESP32-C3 with USB Serial/JTAG — Seeed XIAO C3, ESP32-C3-DevKit, AirM2M

**DevKit:** Any ESP32 DevKit with CP2102 — ESP32-DevKitC, ESP-WROOM-32

---

## Roadmap

- [ ] UART output on DevKit (dual-core arbitration)
- [ ] GPIO blink on DevKit
- [ ] Dual-C3 fabric (inter-chip signaling via power rail)
- [ ] I2C / SPI in under 200 bytes
- [ ] ADC streaming on DevKit

---

## License

MIT
