OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(k_entry)

SECTIONS
{
	. = 0x100000 + SIZEOF_HEADERS;

	.text :
	{
		*(.multiboot) *(.text) *(.text.*)
	}

	.rodata :
	{
		*(.rodata) *(.rodata.*)
	}

	.data :
	{
		*(.data) *(.data.*)
	}

	.bss :
	{
		*(.bss) *(.bss.*) *(COMMON)
	}

	PROVIDE(_end = .);
}
