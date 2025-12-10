/* alignment to avoid padding */
. = ALIGN(8);
START_INITFN_FUNCTIONS = .;
KEEP(*(.initfn_functions));
END_INITFN_FUNCTIONS = .;
