// 99x-esp32 — safe mode template
// Hold GPIO0 LOW at boot → safe mode (no risky hw init)
// GPIO0 HIGH at boot → normal mode (full init)

__attribute__((section(".text.start")))
void _start(void) {
    __asm__ volatile(
        "li sp, 0x3FCE0000\n"
        "jal ra, main\n"
    );
}

// ─── Safe register write — blocks writes to protected regions ───
// Protected: DPORT clock control, RTC control, eFuse
#define PROTECTED_START 0x60026000
#define PROTECTED_END   0x60027000
#define RTC_START       0x60008000
#define RTC_END         0x60009000

static int safe_write(unsigned int addr, unsigned int val) {
    // Never write to protected clock/reset regions
    if (addr >= PROTECTED_START && addr < PROTECTED_END) return -1;
    // RTC WDT feed is OK, but block other RTC writes
    if (addr >= RTC_START && addr < RTC_END) {
        // Only allow WDT feed/write register
        if (addr != 0x600080A0 && addr != 0x600080A4) return -1;
    }
    *(volatile unsigned int *)addr = val;
    return 0;
}

static void feed(void) {
    // These are always safe
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

// ─── Hardware probe — check if it's safe to init risky peripherals ───
// Returns 1 if USB Serial TX works, 0 if not
static int probe_usb(void) {
    unsigned int *conf = (unsigned int *)0x60043004;
    // Try to send a char without waiting
    *(volatile unsigned int *)0x60043000 = '?';
    // Check if TX-ready bit changes (means USB is alive)
    for (int i = 0; i < 1000; i++) {
        if (*conf & 2) return 1;  // TX ready = USB alive
    }
    return 0;
}

// ─── Risky init wrapper ───
// Only runs if safe conditions are met
static int try_init_wifi(void) {
    if (!probe_usb()) {
        puts("USB dead, skipping WiFi init\n");
        return -1;
    }
    
    puts("WiFi init...\n");
    
    // Clock enable — protected by safe_write
    if (safe_write(0x600260D0, 0) < 0) {  // DPORT_WIFI_CLK_EN
        puts("Blocked: DPORT write\n");
        return -1;
    }
    
    // If we get here, the write went through — but it might brick USB
    // The probe_usb() already confirmed USB was alive BEFORE
    
    return 0;
}

void main(void) {
    *(volatile unsigned int *)0x60004024 = (1 << 2);
    
    // Read GPIO0 to determine boot mode (safe vs normal)
    unsigned int gpio0 = (*(volatile unsigned int *)0x6000403C >> 0) & 1;
    
    puts("99x ");
    if (gpio0 == 0) {
        puts("SAFE\n");
    } else {
        puts("NORMAL\n");
    }
    
    // In safe mode, never touch risky peripherals
    if (gpio0 == 0) {
        while (1) {
            feed();
            // Blink — safe mode only
            *(volatile unsigned int *)0x60004004 = (1 << 2);
            for (volatile int i = 0; i < 200000; i++) {}
            *(volatile unsigned int *)0x60004008 = (1 << 2);
            for (volatile int i = 0; i < 200000; i++) {}
        }
    }
    
    // Normal mode — try risky init
    // First confirm USB is alive
    if (probe_usb()) {
        puts("USB OK, proceeding\n");
    } else {
        puts("USB dead, safe mode\n");
        while (1) { feed(); }
    }
    
    // Try WiFi init (protected by safe_write)
    try_init_wifi();
    
    puts("Running\n");
    
    while (1) {
        feed();
        unsigned int gv = *(volatile unsigned int *)0x6000403C;
        puthex(gv);
        putchar('\n');
        *(volatile unsigned int *)0x60004004 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
        *(volatile unsigned int *)0x60004008 = (1 << 2);
        for (volatile int i = 0; i < 50000; i++) {}
    }
}
