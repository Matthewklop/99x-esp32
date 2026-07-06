.text
.global _start
_start:
    # Halt APP CPU immediately using PRID
    rsr.prid a2
    extui a2, a2, 0, 8
    beqz a2, pro_only
    j app_halt
    
pro_only:
    j code
    
    .align 4
stack:  .word 0x3FFE0000
gpio_en: .word 0x3FF44024
gpio_clr: .word 0x3FF44008
gpio_set: .word 0x3FF44004
uart_fifo: .word 0x3FF40000
uart_st:  .word 0x3FF4001C
wdt_feed: .word 0x3FF5F060
wdt_key:  .word 0x3FF5F064
magic:    .word 0x50D83AA1
rtc_feed: .word 0x3FF480A0
rtc_key:  .word 0x3FF480A4
h_val:    .word 0x48
i_val:    .word 0x69
e_val:    .word 0x21
n_val:    .word 0x0A

code:
    l32r a2, stack
    mov sp, a2
    l32r a2, gpio_en
    movi.n a3, 4
    s32i.n a3, a2, 0

    # Feed all WDTs
    l32r a2, wdt_key
    l32r a3, magic
    s32i.n a3, a2, 0
    l32r a2, wdt_feed
    movi.n a3, 1
    s32i.n a3, a2, 0
    l32r a2, rtc_key
    l32r a3, magic
    s32i.n a3, a2, 0
    l32r a2, rtc_feed
    movi.n a3, 1
    s32i.n a3, a2, 0

    # Write '!' to UART (no baud change, use ROM's config)
    l32r a2, uart_st
txw:l32i.n a3, a2, 0
    bbci a3, 0, txw
    l32r a2, uart_fifo
    l32r a3, h_val
    s32i.n a3, a2, 0

    l32r a2, uart_st
tx2:l32i.n a3, a2, 0
    bbci a3, 0, tx2
    l32r a2, uart_fifo
    l32r a3, i_val
    s32i.n a3, a2, 0

    l32r a2, uart_st
tx3:l32i.n a3, a2, 0
    bbci a3, 0, tx3
    l32r a2, uart_fifo
    l32r a3, e_val
    s32i.n a3, a2, 0

    l32r a2, uart_st
tx4:l32i.n a3, a2, 0
    bbci a3, 0, tx4
    l32r a2, uart_fifo
    l32r a3, n_val
    s32i.n a3, a2, 0

    # Blink + feed WDT
blk:
    l32r a2, gpio_clr
    movi.n a3, 4
    s32i.n a3, a2, 0
    # Feed WDT in delay
    movi.n a4, 0
d1: addi a4, a4, 1
    movi.n a5, 16
    blt a4, a5, d1
    # Feed
    l32r a2, wdt_key
    l32r a3, magic
    s32i.n a3, a2, 0
    l32r a2, wdt_feed
    movi.n a3, 1
    s32i.n a3, a2, 0
    l32r a2, rtc_key
    l32r a3, magic
    s32i.n a3, a2, 0
    l32r a2, rtc_feed
    movi.n a3, 1
    s32i.n a3, a2, 0

    l32r a2, gpio_set
    movi.n a3, 4
    s32i.n a3, a2, 0
    movi.n a4, 0
d2: addi a4, a4, 1
    blt a4, a5, d2
    j blk

app_halt:
    j app_halt
