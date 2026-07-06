.text
.global _start
_start:
    # Minimal: just turn on the blue LED and stay on
    # GPIO2 on ESP32 DevKit = GPIO_EN at 0x3FF44024, set bit 2
    # GPIO2 low = LED on (active low on most devkits)
    # Use literal pool
    j code
    .align 4
gpio_en: .word 0x3FF44024
gpio_clr: .word 0x3FF44008
wdt_key:  .word 0x3FF5F064
wdt_feed: .word 0x3FF5F060
rtc_key:  .word 0x3FF480A4
rtc_feed: .word 0x3FF480A0
magic:    .word 0x50D83AA1
code:
    l32r a2, gpio_en
    movi.n a3, 4
    s32i.n a3, a2, 0
    # Turn LED on (GPIO2 low)
    l32r a2, gpio_clr
    movi.n a3, 4
    s32i.n a3, a2, 0
    # Feed WDT forever
loop:
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
    j loop
