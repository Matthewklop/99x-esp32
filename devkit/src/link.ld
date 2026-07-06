SECTIONS
{
<<<<<<< HEAD
    . = 0x40380000;
    .text : { *(.text.start) *(.text) *(.text.*) *(.rodata) *(.rodata.*) }
=======
    . = 0x40080000;
    .text : { *(.text) }
>>>>>>> 4f761e4 (ESP32 DevKit bare metal: blue LED, 320 bytes, dual-core WIP)
}
