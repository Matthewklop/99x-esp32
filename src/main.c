// 99x-esp32 — C3 bare metal: ADC, GPIO, USB serial, WDT
// 288 bytes. No SDK. No Arduino. No ESP-IDF. No OS.

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
    for (int i = 28; i >= 0; i -= 4)
        putchar("0123456789abcdef"[(v >> i) & 0xF]);
}

static void putdec(unsigned int v) {
    char b[12]; int n = 11; b[11] = 0;
    if (v == 0) { putchar('0'); return; }
    while (v) { b[--n] = '0' + (v % 10); v /= 10; }
    puts(&b[n]);
}

static void adc_init(void) {
    *(volatile unsigned int *)0x60040004 = (1 << 22);
    *(volatile unsigned int *)0x60040004 |= (1 << 20);
    *(volatile unsigned int *)0x60040008 = (2 << 0);
}

static unsigned int adc_read(void) {
    volatile unsigned int *ctrl2 = (volatile unsigned int *)0x60040008;
    volatile unsigned int *data = (volatile unsigned int *)0x6004000C;
    *ctrl2 = (0 << 21) | (1 << 19) | (0 << 16);
    for (volatile int i = 0; i < 100; i++) {}
    return *data & 0xFFF;
}

void main(void) {
    *(volatile unsigned int *)0x60004024 = (1 << 2);
    adc_init();
    
    puts("99x\n");
    
    while (1) {
        feed();
        
        unsigned int adc = adc_read();
        unsigned int gpio = *(volatile unsigned int *)0x6000403C;
        
        putchar('A'); putchar('='); putdec(adc);
        putchar(' '); putchar('G'); putchar('='); puthex(gpio);
        putchar('\n');
        
        *(volatile unsigned int *)0x60004004 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
        *(volatile unsigned int *)0x60004008 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
    }
}
