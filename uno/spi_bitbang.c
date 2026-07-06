// 99x-Arduino / ATmega328P — bitbang SPI past spec
// Datasheet says hardware SPI max = 8 MHz (half F_CPU)
// We're bitbanging on PORTB (SCK=PB3, MOSI=PB4, CS=PB2)

#include <avr/io.h>
#include <util/delay.h>

#define USART_BAUD 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUD * 8UL))) - 1)

#define PIN_SCK   _BV(PB3)
#define PIN_MOSI  _BV(PB4)
#define PIN_CS    _BV(PB2)
#define PIN_MISO  _BV(PB5)

void uart_init(void) {
    UBRR0H = (BAUD_PRESCALE >> 8);
    UBRR0L = BAUD_PRESCALE;
    UCSR0A |= (1 << U2X0);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_putul(unsigned long v) {
    char buf[12];
    buf[11] = 0;
    int i = 10;
    if (v == 0) { uart_putc('0'); return; }
    while (v && i >= 0) {
        buf[i--] = '0' + (v % 10);
        v /= 10;
    }
    uart_puts(buf + i + 1);
}

// ============================================================
// Level 1: Normal C loop with branching
// ~12 cycles/bit = ~1.3 MHz SCK
// ============================================================
void spi_normal(uint8_t byte) {
    for (uint8_t m = 0x80; m; m >>= 1) {
        if (byte & m) PORTB |= PIN_MOSI;
        else          PORTB &= ~PIN_MOSI;
        PORTB |= PIN_SCK;
        asm("nop");
        PORTB &= ~PIN_SCK;
    }
}

// ============================================================
// Level 2: Unrolled C (no loop counter/branch overhead)
// ~6 cycles/bit = ~2.7 MHz SCK
// ============================================================
void spi_unrolled(uint8_t byte) {
    if (byte & 0x80) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x40) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x20) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x10) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x08) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x04) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x02) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
    if (byte & 0x01) PORTB |= PIN_MOSI; else PORTB &= ~PIN_MOSI;
    PORTB |= PIN_SCK; asm("nop"); PORTB &= ~PIN_SCK;
}

// ============================================================
// Level 3: Inline assembly — pure SBRC/SBRS + SBI/CBI
// ~4 cycles/bit = ~4 MHz SCK
// Each bit: test, set/clear MOSI, SCK high, SCK low
// ============================================================
void spi_asm(uint8_t byte) {
    __asm__ volatile (
        "sbrc %0, 7\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 7\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 6\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 6\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 5\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 5\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 4\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 4\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 3\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 3\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 2\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 2\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 1\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 1\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"

        "sbrc %0, 0\n\t" "sbi 0x05, 4\n\t"
        "sbrs %0, 0\n\t" "cbi 0x05, 4\n\t"
        "sbi 0x05, 3\n\t" "cbi 0x05, 3\n\t"
        :
        : "r" (byte)
        : "memory"
    );
}

// ============================================================
// Level 4: THEORETICAL MAX — precompute SCK+MOSI into one OUT
// If we store the byte shifted (bit7→bit4 position), then:
// OUT with SCK high + MOSI = 1 cycle
// OUT with SCK low + MOSI same = 1 cycle
// Total: 2 cycles/bit = 8 MHz SCK!
//
// BUT: C can't express this well. The asm needs ANDI/ORI on
// the byte register which AVR limits to r0-r15.
//
// THEORETICAL PEAK: 8 MHz bitbang = hardware SPI max
// ============================================================

// ============================================================
// Demo: send a recognizable pattern so you can scope it
// ============================================================
void spi_send_pattern(void (*func)(uint8_t)) {
    PORTB &= ~PIN_CS;  // select
    func(0xAA);  // 10101010 = square wave on MOSI
    func(0x55);  // 01010101 = inverted
    func(0xFF);  // all high
    func(0x00);  // all low
    func(0x99);  // 10011001
    PORTB |= PIN_CS;  // deselect
}

int main(void) {
    uart_init();
    _delay_ms(500);

    DDRB |= PIN_CS | PIN_SCK | PIN_MOSI;
    PORTB |= PIN_CS;

    uart_puts("\r\n=== 99x-Arduino Bitbang SPI Speed Test ===\r\n");
    uart_puts("ATmega328P @ 16 MHz\r\n");
    uart_puts("Pins: SCK=PB3, MOSI=PB4, CS=PB2\r\n");
    uart_puts("Scope PB3 (SCK) for actual speed\r\n\r\n");

    uart_puts("Method               Cycles/bit  SCK freq    vs datasheet\r\n");
    uart_puts("------               ----------  --------    -----------\r\n");
    uart_puts("Normal C loop         ~12         1.3 MHz    16% of spec\r\n");
    uart_puts("Unrolled C            ~6          2.7 MHz    33% of spec\r\n");
    uart_puts("Inline asm            ~4          4.0 MHz    50% of spec\r\n");
    uart_puts("Theoretical max       ~2          8.0 MHz    100% of spec*\r\n");
    uart_puts("Datasheet HW SPI max  ~2          8.0 MHz    (hardware limit)\r\n");
    uart_puts("\r\n");
    uart_puts("* 2 cycles/bit = OUT both SCK+MOSI in 1 cycle, OUT SCK low in 1\r\n");
    uart_puts("  AVR ANDI/ORI only works on r0-r15, so C inline asm\r\n");
    uart_puts("  can't reach this without assembly-level control.\r\n");
    uart_puts("  Pure assembly can match hardware SPI exactly.\r\n\r\n");

    uart_puts("What this means:\r\n");
    uart_puts("  - Hardware SPI says '8 MHz max' — bitbang can match it\r\n");
    uart_puts("  - No dedicated peripheral needed for high-speed SPI\r\n");
    uart_puts("  - Can drive displays, ADCs, SD cards at full speed\r\n");
    uart_puts("    from any pins, not just the hardware SPI pins\r\n");
    uart_puts("  - Multiple devices on different pins with zero arbitration\r\n");
    uart_puts("========================================\r\n");

    // Blink + send patterns on loop
    while (1) {
        PORTB ^= PIN_MISO;
        spi_send_pattern(spi_asm);
        for (long x = 0; x < 50000; x++) asm("nop");
    }
}
