#include <debug.h>
#include <kprintf.h>
#include <string.h>
#include <symbols.h>

extern "C" char symtab_start;
extern "C" char symtab_end;

struct __attribute__((packed)) symtabentry {
    uintptr_t value;
    char symname[];
};

uintptr_t syms::get_sym(const char *name) {
    if (&symtab_start == &symtab_end) { // there is no symbol table
        return 0;
    }
    struct symtabentry *ptr = (struct symtabentry *)&symtab_start;
    while (ptr->value != 0 || strlen(ptr->symname) != 0) {
        if (strcmp(ptr->symname, name) == 0) {
            return ptr->value;
        }
        ptr = (struct symtabentry *)(((uint8_t *)ptr) + sizeof(symtabentry) + strlen(ptr->symname) + 1);
    }
    kprintf(KP_ERR, "cannot find symbol %s\n", name);
    return 0;
}

std::pair<const char *, uintptr_t> syms::get_sym(size_t n) {
    std::pair<const char *, uintptr_t> ret = {nullptr, 0};
    if (&symtab_start == &symtab_end) { // there is no symbol table
        return ret;
    }
    struct symtabentry *ptr = (struct symtabentry *)&symtab_start;

    for (size_t i = 0; (i < n + 1) && (ptr->value != 0 || strlen(ptr->symname) != 0); i++) {
        if (i == n) {
            ret.first = ptr->symname;
            ret.second = ptr->value;
            return ret;
        }
        ptr = (struct symtabentry *)(((uint8_t *)ptr) + sizeof(symtabentry) + strlen(ptr->symname) + 1);
    }
    return ret;
}

std::pair<const char *, uintptr_t> syms::find_func_sym(uintptr_t addr) {
    std::pair<const char *, uintptr_t> ret = {nullptr, 0};
    if (&symtab_start == &symtab_end) { // there is no symbol table
        return ret;
    }

    struct symtabentry *ptr = (struct symtabentry *)&symtab_start;

    struct symtabentry *best_ptr = (struct symtabentry *)&symtab_start;
    uintptr_t best_addr = 0;

    while (ptr->value != 0 || strlen(ptr->symname) != 0) {
        if (ptr->value < addr) {
            if (ptr->value > best_addr) {
                best_addr = ptr->value;
                best_ptr = ptr;
            }
        }
        ptr = (struct symtabentry *)(((uint8_t *)ptr) + sizeof(symtabentry) + strlen(ptr->symname) + 1);
    }

    if (best_ptr->value != best_addr) {
        return ret;
    }

    ret.first = best_ptr->symname;
    ret.second = best_ptr->value;

    return ret;
}
