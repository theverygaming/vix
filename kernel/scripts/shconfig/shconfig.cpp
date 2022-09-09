#include "configgen.h"
#include "configparser.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s shconfigfile\n", argv[0]);
        return 1;
    }

    processFile(argv[1]);
    genConfigHeader("include/generated/config.h");
    return 0;
}
