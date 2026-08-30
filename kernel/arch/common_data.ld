START_CONSTRUCTORS_INITARR = .;
KEEP(*(.init_array));
KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
END_CONSTRUCTORS_INITARR = .;
START_CONSTRUCTORS_CTORS = .;
KEEP(*(.ctors));
KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)));
END_CONSTRUCTORS_CTORS = .;

/* alignment to avoid padding */
. = ALIGN(8);
START_INITFN_FUNCTIONS = .;
KEEP(*(.initfn_functions));
END_INITFN_FUNCTIONS = .;

KEEP(*(.sym_table))
PROVIDE(symtab_start = .);
PROVIDE(symtab_end = .);