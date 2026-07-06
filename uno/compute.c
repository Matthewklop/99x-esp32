// 99x-Arduino / ATmega328P compute benchmark
// Real calculations: prime sieve, fib, multiply-accumulate, mandelbrot

#include <avr/io.h>
#include <util/delay.h>

#define USART_BAUD 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUD * 8UL))) - 1)

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

void uart_puthex(unsigned long v) {
    char buf[9];
    buf[8] = 0;
    for (int i = 7; i >= 0; i--) {
        unsigned char nib = v & 0xF;
        buf[i] = nib < 10 ? '0' + nib : 'A' + nib - 10;
        v >>= 4;
    }
    uart_puts(buf);
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
// Benchmark 1: Prime sieve (Eratosthenes)
// Use heap/progmem approach — just do it with local array fitting in 1K
// ============================================================
unsigned long bench_sieve(int limit) {
    char sieve[1000];  // fits in stack
    unsigned long count = 0;
    int i, j;

    for (i = 2; i < limit; i++) sieve[i] = 1;
    sieve[0] = sieve[1] = 0;

    for (i = 2; i * i < limit; i++) {
        if (sieve[i]) {
            for (j = i * i; j < limit; j += i)
                sieve[j] = 0;
        }
    }

    for (i = 2; i < limit; i++)
        if (sieve[i]) count++;
    return count;
}

// ============================================================
// Benchmark 2: Fibonacci (iterative)
// ============================================================
unsigned long fib_iterative(int n) {
    unsigned long a = 0, b = 1, t;
    for (int i = 0; i < n; i++) {
        t = a + b;
        a = b;
        b = t;
    }
    return a;
}

// ============================================================
// Benchmark 3: Multiply-accumulate (MAC) — 32-bit
// ============================================================
unsigned long bench_mac32(unsigned long iterations) {
    unsigned long acc = 0;
    for (unsigned long i = 0; i < iterations; i++) {
        acc += i * 7 + 3;
    }
    return acc;
}

// ============================================================
// Benchmark 4: Mandelbrot set (fixed-point 8.8)
// ============================================================
unsigned int mandelbrot_point(int cr, int ci, int max_iter) {
    int zr = 0, zi = 0;
    for (int n = 0; n < max_iter; n++) {
        int zr2 = (zr * zr) >> 8;
        int zi2 = (zi * zi) >> 8;
        if (zr2 + zi2 > (4 << 8)) return n;
        int tmp = zr2 - zi2 + cr;
        zi = ((zr * zi) >> 7) + ci;
        zr = tmp;
    }
    return max_iter;
}

unsigned long bench_mandelbrot(int size, int max_iter) {
    unsigned long total = 0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cr = ((x * 3) << 8) / size - (2 << 8);
            int ci = ((y * 2) << 8) / size - (1 << 8);
            total += mandelbrot_point(cr, ci, max_iter);
        }
    }
    return total;
}

// ============================================================
// Benchmark 5: Software hash (bit rotate + xor)
// ============================================================
unsigned long bench_hash(unsigned long rounds) {
    unsigned long h = 0x6A09E667;
    for (unsigned long i = 0; i < rounds; i++) {
        h ^= h << 13;
        h ^= h >> 7;
        h ^= h << 17;
        h += 0x428A2F98;
        h ^= i;
    }
    return h;
}

// ============================================================
// Benchmark 6: Division (AVR has NO hardware divide)
// ============================================================
unsigned long bench_division(unsigned long iterations) {
    unsigned long result = 0;
    for (unsigned long i = 1; i <= iterations; i++) {
        result += 10000 / ((i & 0xFF) + 1);
    }
    return result;
}

// ============================================================
// Benchmark 7: Float (software emulation — AVR has NO FPU)
// ============================================================
float bench_float(unsigned long iterations) {
    float result = 0.0f;
    for (unsigned long i = 0; i < iterations; i++) {
        result += 3.14159f * (float)(i & 0xFF) / 180.0f;
    }
    return result;
}

// ============================================================
// Benchmark 8: 64-bit math (software emulation)
// ============================================================
unsigned long long bench_64bit(unsigned long iterations) {
    unsigned long long acc = 1;
    for (unsigned long i = 0; i < iterations; i++) {
        acc = (acc * 7 + 3) ^ (i << 5);
    }
    return (acc >> 32) ^ (acc & 0xFFFFFFFF);
}

int main(void) {
    uart_init();
    _delay_ms(500);

    uart_puts("\r\n=== 99x-Arduino Compute Benchmarks ===\r\n");
    uart_puts("ATmega328P @ 16 MHz (8-bit ALU, no FPU, no HW divide, 2KB SRAM)\r\n\r\n");

    // === Benchmark 1: Prime sieve ===
    uart_puts("Bench 1: Prime sieve (limit=1000)\r\n");
    unsigned long p = bench_sieve(1000);
    uart_puts("  Primes found: ");
    uart_putul(p);
    uart_puts(" (expected: 168)\r\n\r\n");

    // === Benchmark 2: Fibonacci iterative ===
    uart_puts("Bench 2: Fibonacci (iterative, n=45)\r\n");
    unsigned long fib45 = fib_iterative(45);
    uart_puts("  fib(45) = ");
    uart_putul(fib45);
    uart_puts(" (expected: 1134903170)\r\n\r\n");

    // === Benchmark 3: MAC 32-bit ===
    uart_puts("Bench 3: 32-bit MAC (10K iterations)\r\n");
    unsigned long mac = bench_mac32(10000);
    uart_puts("  MAC result: ");
    uart_putul(mac);
    uart_puts("\r\n\r\n");

    // === Benchmark 4: Mandelbrot ===
    uart_puts("Bench 4: Mandelbrot (16x16, 64 iterations)\r\n");
    unsigned long mandel = bench_mandelbrot(16, 64);
    uart_puts("  Sum: ");
    uart_putul(mandel);
    uart_puts("\r\n\r\n");

    // === Benchmark 5: Hash ===
    uart_puts("Bench 5: Software hash (200K rounds)\r\n");
    unsigned long h = bench_hash(200000);
    uart_puts("  Hash: 0x");
    uart_puthex(h);
    uart_puts("\r\n\r\n");

    // === Benchmark 6: Division ===
    uart_puts("Bench 6: Software division (10K iterations)\r\n");
    uart_puts("  AVR has NO hardware divider\r\n");
    unsigned long dv = bench_division(10000);
    uart_puts("  Result: ");
    uart_putul(dv);
    uart_puts("\r\n\r\n");

    // === Benchmark 7: Float ===
    uart_puts("Bench 7: Float (1000 iterations)\r\n");
    uart_puts("  AVR has NO FPU — all float = software\r\n");
    float f = bench_float(1000);
    unsigned int *fp = (unsigned int*)&f;
    uart_puts("  Result (IEEE754 hex): 0x");
    uart_puthex(fp[0]);
    uart_puthex(fp[1]);
    uart_puts("\r\n\r\n");

    // === Benchmark 8: 64-bit ===
    uart_puts("Bench 8: 64-bit math (10K iterations)\r\n");
    uart_puts("  AVR 8-bit ALU — 64-bit ops = software loops\r\n");
    unsigned long x64 = bench_64bit(10000);
    uart_puts("  64-bit result (folded): 0x");
    uart_puthex(x64);
    uart_puts("\r\n\r\n");

    // === Summary ===
    uart_puts("========================================\r\n");
    uart_puts("ATmega328P @ 16 MHz — Compute Summary\r\n");
    uart_puts("  8-bit ALU, no FPU, no HW divide, 2KB SRAM\r\n");
    uart_puts("========================================\r\n");

    DDRB |= (1 << PB5);
    while (1) {
        PORTB ^= (1 << PB5);
        for (long x = 0; x < 30000; x++) __asm__("nop");
    }
}
