// 99x-esp32 v2 — known-working version
// This exact code printed "=== fastC3 v6 ===" before

__attribute__((section(".text.start")))
void _start(void) {
    __asm__ volatile(
        "li sp, 0x3FCE0000\n"
        "jal ra, main\n"
    );
}

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

static void puthex(unsigned int v) {
    for (int i = 28; i >= 0; i -= 4) putchar("0123456789abcdef"[(v >> i) & 0xF]);
}

void main(void) {
    *(volatile unsigned int *)0x60004024 = (1 << 2);
    
    // Initial delay — let USB stabilize
    for (volatile int i = 0; i < 1000; i++) {}
    
    puts("99x\n");
    
    while (1) {
        feed();
        puthex(*(volatile unsigned int *)0x6000403C);
        putchar('\n');
        for (volatile int i = 0; i < 50000; i++) {}
        *(volatile unsigned int *)0x60004004 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
        *(volatile unsigned int *)0x60004008 = (1 << 2);
    }
}
