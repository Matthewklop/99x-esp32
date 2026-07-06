// 99x-esp32 — bare-metal ESP32-C3 firmware
// Reads ADC (GPIO2/A0) + GPIO state, streams over USB serial
// Build: make && make flash

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

static void putdec(unsigned int v) {
    char b[12]; int n = 11; b[11] = 0;
    if (v == 0) { putchar('0'); return; }
    while (v) { b[--n] = '0' + (v % 10); v /= 10; }
    puts(&b[n]);
}

// Init ADC1 on ESP32-C3
// SAR ADC at 0x60040000
// Need to: enable RTC clock, configure ADC, then read
static void adc_init(void) {
    // Enable ADC digital controller
    // RTC_CNTL: enable SAR ADC1 clock
    *(volatile unsigned int *)0x60008000 |= (1 << 24);  // Force power on
    
    // SAR ADC controller REGI = 0x60040000
    // CTRL1: enable ADC1, set clock divider
    // APB_SARADC_CTRL1 at +0x04
    *(volatile unsigned int *)0x60040004 = (1 << 22);   // ADC1 work in normal mode
    *(volatile unsigned int *)0x60040004 |= (1 << 20);   // sar1_sample_cycle 1
    *(volatile unsigned int *)0x60040008 = (2 << 0);     // sar_clk_div = 2
    
    // Wait for ADC to stabilize
    for (volatile int i = 0; i < 1000; i++) {}
}

// Read ADC1 channel 0 (GPIO2 on XIAO C3)
static unsigned int adc_read(void) {
    // CTRL2 register at +0x08: start SAR1
    volatile unsigned int *ctrl2 = (volatile unsigned int *)0x60040008;
    volatile unsigned int *data = (volatile unsigned int *)0x6004000C;
    volatile unsigned int *status = (volatile unsigned int *)0x6004001C;
    
    // Start ADC1 conversion on channel 0
    *ctrl2 = (0 << 21) |     // SAR1 sample cycle
             (1 << 19) |     // SAR1 start
             (0 << 16);      // SAR1 channel 0
    
    // Wait for done
    for (volatile int i = 0; i < 100; i++) {}
    
    // Read result
    unsigned int val = *data & 0xFFF;  // 12-bit ADC
    return val;
}

void main(void) {
    *(volatile unsigned int *)0x60004024 = (1 << 2);  // GPIO2 output for blink
    
    // Init ADC
    adc_init();
    
    puts("99x adc\n");
    
    while (1) {
        feed();
        
        // Read ADC on GPIO2 (channel 0)
        unsigned int adc_val = adc_read();
        
        // Read GPIO state
        unsigned int gpio_val = *(volatile unsigned int *)0x6000403C;
        
        // Print: "ADC=XXXX GPIO=XXXXXX"
        putchar('A'); putchar('=');
        putdec(adc_val);
        putchar(' ');
        putchar('G'); putchar('=');
        puthex(gpio_val);
        putchar('\n');
        
        // Blink
        *(volatile unsigned int *)0x60004004 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
        *(volatile unsigned int *)0x60004008 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
    }
}
