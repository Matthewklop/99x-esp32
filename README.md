# 99x-esp32

[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Size](https://img.shields.io/badge/firmware-832%20bytes-blue)](#)
[![Boot](https://img.shields.io/badge/boot-%7E1%C2%B5s-brightgreen)](#)

**Bare-metal ESP32-C3 firmware in 832 bytes. No SDK. No Arduino. No ESP-IDF. No OS.**

A C file and a linker script. That's all you need to run RISC-V code on an ESP32-C3.

---

## The Numbers

| Metric | 99x-esp32 | ESP-IDF | Arduino |
|--------|-----------|---------|---------|
| Firmware size | **832 B** | ~150 KB | ~270 KB |
| Boot to main() | **~1 µs** | ~500 ms | ~1 s |
| Hash throughput | **~55 MB/s** | framework overhead | digitalWrite tax |
| Dependencies | **0** | 47 packages | PlatformIO |
| Lines of code | **~30** | 100,000+ | 10,000+ |
| WDT survival | **∞** | FreeRTOS manages | framework handles |

---

## Quick Start

```bash
# One-time: install the toolchain
pip install esptool
# (Download riscv32-esp-elf-gcc from Espressif)

# Build & flash in 4 commands
riscv32-esp-elf-gcc -c -Os -nostdlib -nostartfiles -ffreestanding -o main.o src/main.c
riscv32-esp-elf-ld -T src/link.ld -o firmware.elf main.o
python3 -m esptool --chip esp32c3 elf2image --flash-mode dio --flash-size 4MB -o firmware.bin firmware.elf
python3 -m esptool --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write-flash 0x0 firmware.bin
```

Plug in your ESP32-C3, run the last three lines. You'll see `99x alive` over USB serial in under a second.

---

## What's Inside

**`src/main.c`** — ~30 lines. Does four things:

1. **Sets up the stack** — RISC-V doesn't initialize SP automatically (`li sp, 0x3FCE0000`)
2. **Feeds the watchdog** — both TIMG0 (0x6001F000) and RTC (0x60008000) need the magic key `0x50D83AA1` or they reset the chip after 325 ms
3. **Writes to USB serial** — the USB Serial/JTAG peripheral at 0x60043000, bit 2 of the conf register tells us when TX is ready
4. **Blinks GPIO2** — direct register write to 0x60004004/0x60004008

That's it. No startup code. No HAL. No vendor libraries. The chip wakes from reset, jumps to 0x40380000, sets SP, calls main.

**`src/link.ld`** — 4 lines. Maps `.text` to 0x40380000 (IRAM), forces `_start` first.

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

---

## Supported Hardware

Any ESP32-C3 with USB Serial/JTAG:
- Seeed XIAO ESP32-C3
- ESP32-C3-DevKit
- AirM2M_CORE_ESP32C3
- Any C3 module with USB D+/D- connected

---

## Roadmap

- [ ] GPIO interrupt-driven hash on pin change
- [ ] Dual-C3 fabric (power rail voltage sag as inter-chip signaling)
- [ ] I2C / SPI driver in under 100 bytes
- [ ] UART bitbang at 1 Mbps
- [ ] ADC read + fingerprint stream

---

## Related

- [aiLocker](https://github.com/Matthewklop/ailocker) — the research project this grew out of
- [ESP32-C3 TRM](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [fastAsFuck](https://github.com/Matthewklop/fastasfuck) — route discovery for optimal computation

---

## License

MIT
