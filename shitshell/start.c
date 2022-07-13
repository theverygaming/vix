#include "types.h"
#include "stdlib.h"

extern void main(int argc, char* argv[], char* envp[]);

static void create_charptrptr(char* in, char** out) {
    size_t count = 0;
    while(1) {
        size_t strl = strlen(in);
        if(strl != 0) {
            out[count] = in;
            count++;
            in += strl+1;
            continue;
        }
        break;
    }
    
}

static size_t create_charptrptr_count(char* in) {
    size_t count = 0;
    while(1) {
        size_t strl = strlen(in);
        if(strl != 0) {
            count++;
            in += strl+1;
            continue;
        }
        break;
    }
    return count;
}

void _startc(int argc, char* argv[], char* envp_str) {
    size_t count = create_charptrptr_count(envp_str);
    char* envp[count+1];
    create_charptrptr(envp_str, envp);
    envp[count] = 0;
    main(argc, argv, envp);
}