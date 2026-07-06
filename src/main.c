// 99x-esp32 — minimal bare-metal ESP32-C3 firmware
// Compile: riscv32-esp-elf-gcc -c -Os -nostdlib -nostartfiles -ffreestanding -o main.o src/main.c
// Link:    riscv32-esp-elf-ld -T src/link.ld -o firmware.elf main.o
// Image:   python3 -m esptool --chip esp32c3 elf2image --flash-mode dio --flash-size 4MB -o firmware.bin firmware.elf
// Flash:   python3 -m esptool --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write-flash 0x0 firmware.bin

static void feed(void) {
    *(volatile unsigned int *)0x6001F064 = 0x50D83AA1;
    *(volatile unsigned int *)0x6001F060 = 1;
    *(volatile unsigned int *)0x600080A4 = 0x50D83AA1;
    *(volatile unsigned int *)0x600080A0 = 1;
}

static void putchar(char c) {
    while (!((*(volatile unsigned int *)0x60043004) & 2)) {}
    *(volatile unsigned int *)0x60043000 = c;
}

static void puts(const char *s) {
    while (*s) putchar(*s++);
}

void _start(void) {
    __asm__ volatile("li sp, 0x3FCE0000\njal ra, main\n");
    while (1) __asm__ volatile("wfi");
}

void main(void) {
    puts("99x alive\n");
    while (1) feed();
}
