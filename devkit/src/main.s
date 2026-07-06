.text
.global _start
_start:
    # ============================================================
    # ESP32 DevKit bare metal firmware
    # Blue LED (GPIO2) + UART0 "Hi!" at 115200
    # Key fix: halt APP CPU immediately to prevent UART contention
    # ============================================================

    # === Step 1: Halt APP CPU ===
    # APP CPU CTRL at 0x3FF48800, write 0 to halt
    # Do this FIRST before any register access
    l32r  a2, appcpu_ctrl
    movi  a3, 0
    s32i  a3, a2, 0

    # === Step 2: Configure UART0 ===
    # Set CLKDIV for 115200 baud @ 80MHz APB
    # Formula: CLKDIV = APB_CLK / (16 * baud) = 80000000 / (16 * 115200) = 43.4
    # Integer part = 43, Fraction part = 0.4 * 256 = 102
    l32r  a2, uart_clkdiv
    # Set integer=43, frac=102 for ~115200 baud
    movi  a3, (43 << 0) | (102 << 10)
    s32i  a3, a2, 0

    # === Step 3: Enable UART TX ===
    # UART_CONF0 at 0x3FF40020, set bit 0 (parity) disabled, bits 2-1 flow ctrl disabled
    # UART_CONF1 at 0x3FF40024, set TX FIFO empty threshold to 0
    # Actually: configure TX_FIFO_RESET bit 2 in CONF0 for 0
    l32r  a2, uart_conf0
    movi  a3, 0x0
    s32i  a3, a2, 0

    # === Step 4: Set GPIO2 as output (blue LED) ===
    l32r  a2, gpio_en
    movi.n a3, 4           # bit 2
    s32i.n a3, a2, 0

    # Turn blue LED on (GPIO2 low = active low)
    l32r  a2, gpio_clr
    movi.n a3, 4
    s32i.n a3, a2, 0

    # === Step 5: Main loop - send "Hi!" every ~500ms ===
loop:
    # Send 'H'
    l32r  a2, uart_fifo
    movi  a3, 0x48
    s32i  a3, a2, 0
    # Wait for TX to complete
    jal   wait_tx

    # Send 'i'
    l32r  a2, uart_fifo
    movi  a3, 0x69
    s32i  a3, a2, 0
    jal   wait_tx

    # Send '!'
    l32r  a2, uart_fifo
    movi  a3, 0x21
    s32i  a3, a2, 0
    jal   wait_tx

    # Send newline
    l32r  a2, uart_fifo
    movi  a3, 0x0A
    s32i  a3, a2, 0
    jal   wait_tx

    # Feed WDT
    l32r  a2, wdt_key
    l32r  a3, magic
    s32i.n a3, a2, 0
    l32r  a2, wdt_feed
    movi.n a3, 1
    s32i.n a3, a2, 0
    l32r  a2, rtc_key
    l32r  a3, magic
    s32i.n a3, a2, 0
    l32r  a2, rtc_feed
    movi.n a3, 1
    s32i.n a3, a2, 0

    # Blink LED quickly to show life
    l32r  a2, gpio_clr
    movi.n a3, 4
    s32i.n a3, a2, 0
    # Short delay loop
    movi  a4, 50000
delay_on:
    addi  a4, a4, -1
    bnez  a4, delay_on

    l32r  a2, gpio_set
    movi.n a3, 4
    s32i.n a3, a2, 0
    # Short delay loop
    movi  a4, 50000
delay_off:
    addi  a4, a4, -1
    bnez  a4, delay_off

    j loop

# ============================================================
# Wait for TX FIFO empty
# ============================================================
wait_tx:
    l32r  a5, uart_status
    movi  a6, 0x20        # Bit 5 = TX_FIFO_EMPTY
wait_tx_loop:
    l32i  a7, a5, 0
    and   a7, a7, a6
    beqz  a7, wait_tx_loop
    ret

# ============================================================
# Literal pool
# ============================================================
.align 4
uart_fifo:    .word 0x3FF40000
uart_status:  .word 0x3FF4001C
uart_clkdiv:  .word 0x3FF40014
uart_conf0:   .word 0x3FF40020
gpio_en:      .word 0x3FF44024
gpio_set:     .word 0x3FF44004
gpio_clr:     .word 0x3FF44008
wdt_key:      .word 0x3FF5F064
wdt_feed:     .word 0x3FF5F060
rtc_key:      .word 0x3FF480A4
rtc_feed:     .word 0x3FF480A0
appcpu_ctrl:  .word 0x3FF48800
magic:        .word 0x50D83AA1
