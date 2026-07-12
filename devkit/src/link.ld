SECTIONS
{
    . = 0x40080000;
    .text.start : { *(.text.start) *(.trampoline) }
    .literal : { *(.literal) *(.literal.*) }
    .text : { *(.text) *(.text.*) }
    .rodata : { *(.rodata) *(.rodata.*) }
    .bss : { *(.bss) *(.bss.*) *(COMMON) }
}
