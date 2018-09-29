SECTIONS
{
    .text ALIGN (0x4) : { _ftext = . ; *(.text) *(.text.*) }
    _etext = .;
    .rodata ALIGN (0x8) : { . = .; *(.rodata) *(.rodata.*) }
    .data ALIGN (0x8) : { _fdata = . ; *(.data) *(.data.*) }
    _gp = ALIGN(0x100) ;
    .sdata ALIGN (0x4) : { . = .; *(.sdata) *(.sdata.*) }

	. = ALIGN(16) ;

    _edata = .;
    __bss_start = .; _fbss = .;
	.sbss ALIGN (0x8) : { . = .; *(.sbss) *(.sbss.*) }
    .bss ALIGN (0x8) : { *(.bss) *(.bss.*) }
    stack ALIGN (0x8) : { *(COMMON) }
	__bss_end = .;
    . = ALIGN(4); _end = .; PROVIDE (end = .);
	__heap1 = ALIGN(0x10) ;
}
