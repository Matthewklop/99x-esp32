// 99x-esp32 / ESP8266 bare metal
// Blue LED (GPIO2) + OLED SH1106 (I2C GPIO3=SDA, GPIO4=SCL)
//
// ESP8266 single-core Xtensa LX106 @ 80MHz
// ROM jumps to call_user_start at 0x40100000
//
// GPIO registers on ESP8266:
// 0x60000304 = GPIO_OUT_SET, 0x60000308 = GPIO_OUT_CLEAR
// 0x6000030C = GPIO_ENABLE,  0x60000318 = GPIO_IN
// 0x60000600 = WDT feed
//
// I2C: bit-banged, GPIO3 (bit 3) = SDA, GPIO4 (bit 4) = SCL

#define GPIO_OUT_SET   ((volatile unsigned int*)0x60000304)
#define GPIO_OUT_CLR   ((volatile unsigned int*)0x60000308)
#define GPIO_ENABLE    ((volatile unsigned int*)0x6000030C)
#define GPIO_IN        ((volatile unsigned int*)0x60000318)
#define WDT_FEED       ((volatile unsigned int*)0x60000600)

#define SDA 0x08  // bit 3
#define SCL 0x10  // bit 4
#define LED 0x04  // bit 2

#define OLED_ADDR 0x78  // 0x3C << 1

static void delay(unsigned int count) {
    while (count--) {
        __asm__ volatile("nop");
    }
}

static void i2c_delay(void) {
    delay(80);  // ~1us at 80MHz
}

static void i2c_init(void) {
    *GPIO_OUT_SET = SDA | SCL;
    i2c_delay();
}

static void i2c_start(void) {
    *GPIO_OUT_CLR = SDA;  // SDA low while SCL high
    i2c_delay();
    *GPIO_OUT_CLR = SCL;  // SCL low
    i2c_delay();
}

static void i2c_stop(void) {
    *GPIO_OUT_CLR = SDA;   // SDA low
    i2c_delay();
    *GPIO_OUT_SET = SCL;   // SCL high
    i2c_delay();
    *GPIO_OUT_SET = SDA;   // SDA high (stop condition)
    i2c_delay();
}

static int i2c_write_byte(unsigned char byte) {
    int i;
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            *GPIO_OUT_SET = SDA;
        } else {
            *GPIO_OUT_CLR = SDA;
        }
        byte <<= 1;
        *GPIO_OUT_SET = SCL;  // clock high
        i2c_delay();
        *GPIO_OUT_CLR = SCL;  // clock low
        i2c_delay();
    }
    // Release SDA for ACK
    *GPIO_OUT_SET = SDA;
    *GPIO_OUT_SET = SCL;  // clock high
    i2c_delay();
    int ack = (*GPIO_IN & SDA) ? 1 : 0;  // read SDA
    *GPIO_OUT_CLR = SCL;  // clock low
    i2c_delay();
    return ack;
}

static void oled_command(unsigned char cmd) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x00);  // command mode
    i2c_write_byte(cmd);
    i2c_stop();
}

static void oled_data_start(void) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x40);  // data mode
}

static void oled_data_end(void) {
    i2c_stop();
}

static void oled_set_page(int page) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x00);
    i2c_write_byte(0xB0 | page);  // set page
    i2c_write_byte(0x00);         // column low
    i2c_write_byte(0x10);         // column high
    i2c_stop();
}

static void oled_init(void) {
    oled_command(0xAE);  // display OFF
    oled_command(0xD5);  // oscillator
    oled_command(0x80);
    oled_command(0xA8);  // mux ratio
    oled_command(0x3F);  // 64
    oled_command(0xD3);  // offset
    oled_command(0x00);
    oled_command(0x40);  // start line
    oled_command(0xA1);  // segment remap
    oled_command(0xC8);  // COM scan direction
    oled_command(0xDA);  // COM pins
    oled_command(0x12);
    oled_command(0x81);  // contrast
    oled_command(0xCF);
    oled_command(0xA4);  // display on follow RAM
    oled_command(0xA6);  // normal display
    oled_command(0xAD);  // charge pump
    oled_command(0x8B);
    oled_command(0xAF);  // display ON
}

static void oled_clear(void) {
    int page;
    for (page = 0; page < 8; page++) {
        oled_set_page(page);
        oled_data_start();
        int col;
        for (col = 0; col < 132; col++) {
            i2c_write_byte(0x00);
        }
        oled_data_end();
    }
}

static void oled_show_logo(void) {
    oled_clear();
    delay(500000);

    // Draw top and bottom bars, checkerboard middle
    int page;
    for (page = 0; page < 8; page++) {
        oled_set_page(page);
        oled_data_start();
        unsigned char pattern;
        if (page == 0 || page == 7) {
            pattern = 0xFF;  // solid bar
        } else {
            pattern = 0xAA;  // checkerboard
        }
        int col;
        for (col = 0; col < 132; col++) {
            i2c_write_byte(pattern);
        }
        oled_data_end();
    }

    // Draw "99x" pattern in center (page 3, column 60)
    oled_set_page(3);
    oled_data_start();
    int i;
    for (i = 0; i < 60; i++) i2c_write_byte(0x00);
    // '9' char: 0x7C,0x82,0x82,0x7C,0x02,0x02,0x7C,0x00
    i2c_write_byte(0x7C); i2c_write_byte(0x82); i2c_write_byte(0x82); i2c_write_byte(0x7C);
    i2c_write_byte(0x02); i2c_write_byte(0x02); i2c_write_byte(0x7C); i2c_write_byte(0x00);
    // '9' again
    i2c_write_byte(0x7C); i2c_write_byte(0x82); i2c_write_byte(0x82); i2c_write_byte(0x7C);
    i2c_write_byte(0x02); i2c_write_byte(0x02); i2c_write_byte(0x7C); i2c_write_byte(0x00);
    // 'x' char: 0x00,0x42,0x24,0x18,0x18,0x24,0x42,0x00
    i2c_write_byte(0x00); i2c_write_byte(0x42); i2c_write_byte(0x24); i2c_write_byte(0x18);
    i2c_write_byte(0x18); i2c_write_byte(0x24); i2c_write_byte(0x42); i2c_write_byte(0x00);
    for (i = 0; i < 48; i++) i2c_write_byte(0x00);
    oled_data_end();
}

void call_user_start(void) {
    // Set stack pointer
    __asm__ volatile("movi a1, 0x3FFFC000");

    // Enable GPIO2 (LED), GPIO3 (SDA), GPIO4 (SCL)
    *GPIO_ENABLE = LED | SDA | SCL;

    // Turn LED on (GPIO2 low = active low)
    *GPIO_OUT_CLR = LED;

    // Init peripherals
    i2c_init();
    oled_init();
    oled_show_logo();

    // Main loop
    while (1) {
        // Blink LED
        *GPIO_OUT_SET = LED;
        delay(2000000);

        *GPIO_OUT_CLR = LED;
        delay(2000000);

        // Feed WDT
        *WDT_FEED = 0x73;
    }
}
