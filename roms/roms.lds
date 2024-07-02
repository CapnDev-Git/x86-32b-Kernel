OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(entry)

PHDRS
{
    code PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
	. = 0x4000 + SIZEOF_HEADERS;

	.text :
	{
		*(.text) *(.text.*)
	} : code

	.rodata :
	{
		*(.rodata) *(.rodata.*)
	} : data

	.data :
	{
		*(.data) *(.data.*)
	} : data

	.bss :
	{
		*(.bss) *(.bss.*) *(COMMON)
	} : data
}
