SECTIONS
{
    . = 0x40100000;
    .literal : { *(.literal) *(.literal.*) }
    .text : { *(.text) *(.text.*) }
    .rodata : { *(.rodata) *(.rodata.*) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
