# 99x-esp32 / ESP8266 bare metal
# Blue LED (GPIO2) + OLED (SH1106, I2C GPIO3=SDA GPIO4=SCL)
#
# ESP8266 single-core Xtensa LX106 @ 80MHz
# ROM jumps to 0x40100000
#
# CRITICAL: ESP8266 Xtensa has short l32r range (~128KB forward only?).
# Must put literal pool BEFORE code, or use j/call0 to jump over literals.
# Pattern: j skip_lits ; .align 4 ; lits: ... ; skip_lits: code

.text
.align 4
.global call_user_start

# ============================================================
# Entry — ROM jumps here at 0x40100000
# Jump over literal pool immediately
# ============================================================
call_user_start:
    j _start

.align 4
# Literals for _start
L_gpio_en:       .word 0x6000030C
L_gpio_out_clr:  .word 0x60000308
L_gpio_out_set:  .word 0x60000304
L_gpio_in:       .word 0x60000318
L_wdt_feed:      .word 0x60000600
L_stack_top:     .word 0x3FFFC000

_start:
    l32r  a1, L_stack_top

    # === Configure GPIO ===
    l32r  a2, L_gpio_en
    movi  a3, 0x1C         # bits 2|3|4 = LED + SDA + SCL outputs
    s32i  a3, a2, 0

    # Blue LED on = GPIO2 low (active low on NodeMCU)
    l32r  a2, L_gpio_out_clr
    movi  a3, 0x04
    s32i  a3, a2, 0

    # === Init I2C and OLED ===
    call0 i2c_init
    call0 oled_init
    call0 oled_show_logo

    # ============================================================
    # Main loop: blink LED, feed WDT
    # ============================================================
loop:
    l32r  a2, L_gpio_out_set
    movi  a3, 0x04
    s32i  a3, a2, 0
    call0 delay_100ms

    l32r  a2, L_gpio_out_clr
    movi  a3, 0x04
    s32i  a3, a2, 0
    call0 delay_100ms

    # Feed WDT
    l32r  a2, L_wdt_feed
    movi  a3, 0x73
    s32i  a3, a2, 0

    j loop

# ============================================================
# Delay ~100ms at 80MHz
# ============================================================
delay_100ms:
    movi  a6, 2000000
1:  addi  a6, a6, -1
    bnez  a6, 1b
    ret

# ============================================================
# I2C bit-bang on GPIO3 (SDA) and GPIO4 (SCL)
# Registers: 0x60000304 = SET, 0x60000308 = CLEAR
# SDA bit = 0x08, SCL bit = 0x10
# ============================================================
I2C_GPIO_SET:  .word 0x60000304
I2C_GPIO_CLR:  .word 0x60000308
I2C_GPIO_IN:   .word 0x60000318

.macro i2c_high pin, reg
    l32r  a6, \reg
    movi  a7, \pin
    s32i  a7, a6, 0
.endm
.macro i2c_low pin, reg
    l32r  a6, \reg
    movi  a7, \pin
    s32i  a7, a6, 0
.endm

# Micro-delay for I2C timing
i2c_delay:
    movi  a6, 40
1:  addi  a6, a6, -1
    bnez  a6, 1b
    ret

i2c_init:
    # SDA high, SCL high (idle bus)
    l32r  a2, I2C_GPIO_SET
    movi  a3, 0x18         # both SDA+SCL high
    s32i  a3, a2, 0
    call0 i2c_delay
    ret

i2c_start:
    l32r  a2, I2C_GPIO_CLR
    movi  a3, 0x08         # SDA low while SCL high
    s32i  a3, a2, 0
    call0 i2c_delay
    l32r  a2, I2C_GPIO_CLR
    movi  a3, 0x10         # SCL low
    s32i  a3, a2, 0
    call0 i2c_delay
    ret

i2c_stop:
    l32r  a2, I2C_GPIO_CLR
    movi  a3, 0x08         # SDA low
    s32i  a3, a2, 0
    call0 i2c_delay
    l32r  a2, I2C_GPIO_SET
    movi  a3, 0x10         # SCL high
    s32i  a3, a2, 0
    call0 i2c_delay
    l32r  a2, I2C_GPIO_SET
    movi  a3, 0x08         # SDA high (stop)
    s32i  a3, a2, 0
    call0 i2c_delay
    ret

# Send byte a4 on I2C, returns ACK in a4
i2c_write_byte:
    movi  a5, 8
1:  extui a7, a4, 7, 1    # MSB first
    beqz  a7, 2f
    l32r  a6, I2C_GPIO_SET
    movi  a7, 0x08
    s32i  a7, a6, 0
    j     3f
2:  l32r  a6, I2C_GPIO_CLR
    movi  a7, 0x08
    s32i  a7, a6, 0
3:  # Clock
    l32r  a6, I2C_GPIO_SET
    movi  a7, 0x10
    s32i  a7, a6, 0
    call0 i2c_delay
    l32r  a6, I2C_GPIO_CLR
    movi  a7, 0x10
    s32i  a7, a6, 0
    sll   a4, a4, 1
    addi  a5, a5, -1
    bnez  a5, 1b
    # ACK
    l32r  a6, I2C_GPIO_SET
    movi  a7, 0x08         # release SDA
    s32i  a7, a6, 0
    l32r  a6, I2C_GPIO_SET
    movi  a7, 0x10         # clock
    s32i  a7, a6, 0
    call0 i2c_delay
    l32r  a6, I2C_GPIO_IN
    l32i  a4, a6, 0
    extui a4, a4, 3, 1    # SDA bit = ACK
    l32r  a6, I2C_GPIO_CLR
    movi  a7, 0x10
    s32i  a7, a6, 0
    call0 i2c_delay
    ret

# ============================================================
# SH1106 OLED init + display
# I2C addr 0x3C (write = 0x78)
# ============================================================
.align 4
OLED_ADDR: .word 0x78

oled_init:
    call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x00         # command mode
    call0 i2c_write_byte
    movi  a4, 0xAE; call0 i2c_write_byte  # display OFF
    movi  a4, 0xD5; call0 i2c_write_byte  # oscillator
    movi  a4, 0x80; call0 i2c_write_byte
    movi  a4, 0xA8; call0 i2c_write_byte  # mux ratio
    movi  a4, 0x3F; call0 i2c_write_byte  # 64
    movi  a4, 0xD3; call0 i2c_write_byte  # offset
    movi  a4, 0x00; call0 i2c_write_byte
    movi  a4, 0x40; call0 i2c_write_byte  # start line
    movi  a4, 0xA1; call0 i2c_write_byte  # segment remap
    movi  a4, 0xC8; call0 i2c_write_byte  # COM scan dir
    movi  a4, 0xDA; call0 i2c_write_byte  # COM pins
    movi  a4, 0x12; call0 i2c_write_byte
    movi  a4, 0x81; call0 i2c_write_byte  # contrast
    movi  a4, 0xCF; call0 i2c_write_byte
    movi  a4, 0xA4; call0 i2c_write_byte  # display on follow RAM
    movi  a4, 0xA6; call0 i2c_write_byte  # normal (not inverted)
    movi  a4, 0xAD; call0 i2c_write_byte  # charge pump
    movi  a4, 0x8B; call0 i2c_write_byte
    movi  a4, 0xAF; call0 i2c_write_byte  # display ON
    call0 i2c_stop
    ret

oled_clear:
    movi  a8, 8            # 8 pages
    movi  a9, 0            # page counter
1:  call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x00
    call0 i2c_write_byte
    movi  a4, 0xB0
    add   a4, a4, a9
    call0 i2c_write_byte
    movi  a4, 0x00
    call0 i2c_write_byte
    movi  a4, 0x10
    call0 i2c_write_byte
    call0 i2c_stop

    call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x40
    call0 i2c_write_byte
    movi  a11, 132
    movi  a4, 0x00
2:  call0 i2c_write_byte
    addi  a11, a11, -1
    bnez  a11, 2b
    call0 i2c_stop

    addi  a9, a9, 1
    addi  a8, a8, -1
    bnez  a8, 1b
    ret

# Draw a cool pattern on the OLED
oled_show_logo:
    call0 oled_clear

    # Draw bars on page 0 (top) and page 7 (bottom)
    movi  a9, 0
3:  call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x00
    call0 i2c_write_byte
    movi  a4, 0xB0
    add   a4, a4, a9
    call0 i2c_write_byte
    movi  a4, 0x00
    call0 i2c_write_byte
    movi  a4, 0x10
    call0 i2c_write_byte
    call0 i2c_stop

    call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x40
    call0 i2c_write_byte
    movi  a11, 132
    movi  a12, 0xFF        # filled bar for page 0 and 7
    movi  a13, 7
    beq   a9, a13, 5f
    bnez  a9, 4f
5:  movi  a4, 0xFF
    j     6f
4:  movi  a4, 0xAA        # checkered for middle
6:  call0 i2c_write_byte
    addi  a11, a11, -1
    bnez  a11, 6b
    call0 i2c_stop

    addi  a9, a9, 1
    movi  a13, 8
    blt   a9, a13, 3b

    # Draw text "99x" in the center (page 3-4)
    # Using simple 8x8 pixel patterns
    movi  a9, 3
    movi  a14, 4
draw_text:
    call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x00
    call0 i2c_write_byte
    movi  a4, 0xB0
    add   a4, a4, a9
    call0 i2c_write_byte
    movi  a4, 0x40         # column offset 64
    call0 i2c_write_byte
    movi  a4, 0x14
    call0 i2c_write_byte
    call0 i2c_stop

    call0 i2c_start
    l32r  a4, OLED_ADDR
    call0 i2c_write_byte
    movi  a4, 0x40
    call0 i2c_write_byte
    movi  a11, 64
7:  movi  a4, 0x99        # pattern
    call0 i2c_write_byte
    addi  a11, a11, -1
    bnez  a11, 7b
    call0 i2c_stop

    addi  a9, a9, 1
    addi  a14, a14, -1
    bnez  a14, draw_text

    ret
