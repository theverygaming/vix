#include "configask.h"
#include "configgen.h"
#include "configparser.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s -f shconfigfile [-D CONFIG_NAME value] | -g -> generates config include file from config_gen output\n", argv[0]);
        return 1;
    }

    char *inputFile = nullptr;
    
    bool gen_from_file = false;

    int opt;
    while ((opt = getopt(argc, argv, "f:D:g")) != -1) {
        switch (opt) {
        case 'D':
            if ((optind) < argc) {
                printf("define: %s %s\n", argv[optind - 1], argv[optind]);
                config_map[argv[optind - 1]].content = argv[optind];
                config_map[argv[optind - 1]].conftype = config_type::CONFIG_STRING;
                config_map[argv[optind - 1]].preset = true;
            }
            break;
        case 'f':
            inputFile = optarg;
            break;
        case 'g':
            gen_from_file = true;
            break;
        }
    }

    if(gen_from_file) {
        genConfigHeader("include/generated/config.h");
        genConfigFile("config_gen");
        return 0;
    }

    if (inputFile == nullptr) {
        printf("issue: no input file provided\n");
        return 1;
    }

    processFile(inputFile);
    configText();

    genConfigHeader("include/generated/config.h");
    genConfigFile("config_gen");
    return 0;
}
