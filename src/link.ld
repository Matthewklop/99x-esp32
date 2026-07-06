SECTIONS
{
    . = 0x40380000;
    .text : { *(.text.start) *(.text) *(.text.*) *(.rodata) *(.rodata.*) }
}
