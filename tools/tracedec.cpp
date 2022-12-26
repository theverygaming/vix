#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <map>
#include <string>
#include <vector>

struct symbol {
    std::string name;
    uint32_t address = 0;
    size_t callcount = 0;
};

bool symsort(const struct symbol a, const struct symbol b) {
    return a.callcount > b.callcount;
}

struct symbol *findsym(uint32_t address, std::vector<struct symbol> *symtab) {
    for (size_t i = 0; i < symtab->size(); i++) {
        if ((*symtab)[i].address == address) {
            return &(*symtab)[i];
        }
    }
    return nullptr;
}

struct __attribute__((packed)) ftrace_entry {
    char type;
    uint32_t func;
    uint32_t caller;
};

int main(int argc, char *argv[]) {
    std::string inputfile = "";
    std::string symtabfile = "";

    int parsed_args = 0;

    int opt;
    while ((opt = getopt(argc, argv, "i:s:")) != -1) {
        switch (opt) {
        case 'i': { // input file
            inputfile = optarg;
            parsed_args++;
        } break;
        case 's': { // symtab file
            symtabfile = optarg;
            parsed_args++;
        } break;
        }
    }

    if (parsed_args != 2) {
        fprintf(stderr, "usage: %s -i [input file] -s [symbol table]\nexample: nm -C --format=bsd -n kernel/kernel.o | ./tracedec -s /dev/stdin -i /tmp/ftrace.bin\n", argv[0]);
        return 1;
    }

    std::ifstream input_f(inputfile, std::ios::binary);
    if (!input_f.is_open()) {
        fprintf(stderr, "could not open input file\n");
        return 1;
    }

    std::ifstream symtab_f(symtabfile);
    if (!symtab_f.is_open()) {
        fprintf(stderr, "could not open symtab file\n");
        return 1;
    }

    std::vector<struct symbol> symtab;
    std::string currentLine;
    while (std::getline(symtab_f, currentLine)) {
        char name[1000];
        if (currentLine.length() > 1000) {
            fprintf(stderr, "input too long\n");
            return 1;
        }
        char c;
        unsigned int address;
        sscanf(currentLine.c_str(), "%x %c %[^\n]", &address, &c, name);
        // fprintf(stderr, "%x -> %s\n", address, name);
        symtab.push_back({.name = name, .address = address, .callcount = 0});
    }
    symtab_f.close();

    struct ftrace_entry entry;
    while (input_f.read((char *)&entry, sizeof(struct ftrace_entry))) {
        if (entry.type == 'x') {
            continue;
        }
        if (entry.type != 'e' && entry.type != 'x') {
            //fprintf(stderr, "parser error\n");
            continue;
        }
        struct symbol *sym = findsym(entry.func, &symtab);
        if (sym == nullptr) {
            //printf("unresolved symbol %x(%c)\n", entry.func, entry.type);
            continue;
        }
        // printf("%s\n", sym->name.c_str());
        sym->callcount++;
    }

    std::sort(symtab.begin(), symtab.end(), symsort);
    printf("top called functions\n");
    for (size_t i = 0; i < std::min(symtab.size(), (size_t)100); i++) {
        printf("%zu -> %s\n", symtab[i].callcount, symtab[i].name.c_str());
    }

    input_f.close();
    return 0;
}
