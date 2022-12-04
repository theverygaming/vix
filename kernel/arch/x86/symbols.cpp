#include <arch/x86/symbols.h>
#include <stdlib.h>

extern "C" char symtab_start;
extern "C" char symtab_end;

struct __attribute__((packed)) symtabentry {
    uintptr_t value;
    char symname[];
};

uintptr_t syms::get_sym(const char *name) {
    if (&symtab_start == &symtab_end || &symtab_start == nullptr) { // there is no symbol table
        return 0;
    }
    struct symtabentry *ptr = (struct symtabentry *)&symtab_start;
    while (ptr->value != 0 || strlen(ptr->symname) != 0) {
        if (strcmp(ptr->symname, name) == 0) {
            return ptr->value;
        }
        ptr = (struct symtabentry *)(((uint8_t *)ptr) + sizeof(symtabentry) + strlen(ptr->symname) + 1);
    }
    return 0;
}
