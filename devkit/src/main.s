.text
.global _start
_start:
# Disable TIMG0 WDT
    movi.n a2, 0x3F; slli a2, a2, 16
    movi.n a3, 0xF5; slli a3, a3, 8; or a2, a2, a3; addi a2, a2, 0x64
    movi.n a3, 0x50; slli a3, a3, 24
    movi.n a4, 0xD8; slli a4, a4, 16; or a3, a3, a4
    movi.n a4, 0x3A; slli a4, a4, 8; or a3, a3, a4; addi a3, a3, 0xA1
    s32i.n a3, a2, 0
    addi a2, a2, -28; movi.n a3, 0; s32i.n a3, a2, 0

# Disable TIMG1 WDT (0x3FF60064)
    movi.n a2, 0x3F; slli a2, a2, 16
    movi.n a3, 0xF6; slli a3, a3, 8; or a2, a2, a3; addi a2, a2, 0x64
    movi.n a3, 0x50; slli a3, a3, 24
    movi.n a4, 0xD8; slli a4, a4, 16; or a3, a3, a4
    movi.n a4, 0x3A; slli a4, a4, 8; or a3, a3, a4; addi a3, a3, 0xA1
    s32i.n a3, a2, 0
    addi a2, a2, -28; movi.n a3, 0; s32i.n a3, a2, 0

# Disable RTC WDT (0x3FF480A4)
    movi.n a2, 0x3F; slli a2, a2, 16
    movi.n a3, 0xF4; slli a3, a3, 8; or a2, a2, a3
    addi a2, a2, 0x80; addi a2, a2, 0x24
    movi.n a3, 0x50; slli a3, a3, 24
    movi.n a4, 0xD8; slli a4, a4, 16; or a3, a3, a4
    movi.n a4, 0x3A; slli a4, a4, 8; or a3, a3, a4; addi a3, a3, 0xA1
    s32i.n a3, a2, 0
    addi a2, a2, -24; movi.n a3, 0; s32i.n a3, a2, 0

# UART FIFO
    movi.n a5, 0x3F; slli a5, a5, 16
    movi.n a2, 0x40; slli a2, a2, 8; or a5, a5, a2

    movi.n a3, 69; s32i.n a3, a5, 0
    movi.n a3, 10; s32i.n a3, a5, 0

loop:
    j loop
