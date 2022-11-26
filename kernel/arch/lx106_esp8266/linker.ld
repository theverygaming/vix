/* well this seems to work... It runs at least.. Is it right? i have no clue */
ENTRY(_kentry)

MEMORY
{
	dport0_0_seg : org = 0x3FF00000, len = 0x10
	dram0_0_seg : org = 0x3FFE8000, len = 0x14000
	iram1_0_seg : org = 0x40100000, len = 0x8000
	irom0_0_seg : org = 0x40210000, len = 0x5C000
}

SECTIONS
{
	.rodata : ALIGN(4)
	{
		_rodata_start = ABSOLUTE(.);
		*(.rodata)
		START_CONSTRUCTORS = .;
		KEEP(*( .ctors ));
		KEEP(*(SORT_BY_INIT_PRIORITY( .ctors.* )));
		END_CONSTRUCTORS = .;
		_rodata_end = ABSOLUTE(.);
	}>dram0_0_seg

	.data : ALIGN(4)
	{
		_data_start = ABSOLUTE(.);
		*(.data)
		_data_end = ABSOLUTE(.);
	}>dram0_0_seg

	.bss ALIGN(8) (NOLOAD) : ALIGN(4)
	{
		. = ALIGN (8);
		_bss_start = ABSOLUTE(.);
		*(.bss)
		_bss_end = ABSOLUTE(.);
	}>dram0_0_seg

	.text : ALIGN(4)
	{
		_text_start = ABSOLUTE(.);
		*(.entry)
		*(.text)
		_text_start = ABSOLUTE(.);
	}>iram1_0_seg
}
