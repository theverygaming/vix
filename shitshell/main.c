#include "types.h"
#include "syscall.h"
#include "stdlib.h"

static void replace_char(char* str, size_t len,  char c, char r) {
    while(len--) {
        if(*str == c) {
            *str = r;
            return;
        }
        str++;
    }
}

static int str_remove_until(char* out, const char* strin, char c) {
    bool remove = true;
    while(*strin) {
        if(!remove) {
            *out = *strin;
            out++;
            strin++;
            continue;
        }
        if(*strin == c) { remove = false; }
        *strin++;
    }
    if(remove) { return -1; }
    return 0;
}

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

char input_buf[100];

char args_buf[100];

int main(int argc, char* argv[], char* envp[]) {
    while(1) {
        size_t cntr = 0;
        while(envp[cntr]) {
            sys_write(1, envp[cntr], strlen(envp[cntr]));
            sys_write(1, "\n", 1);
            cntr++;
        }
        
        sys_write(1, "# ", 2);
        sys_read(1, input_buf, sizeof(input_buf));
        replace_char(input_buf, sizeof(input_buf), '\n', '\0');
        str_remove_until(args_buf, input_buf, ' ');
        replace_char(args_buf, sizeof(args_buf), ' ', '\0');
        //sys_write(1, args_buf, sizeof(args_buf));
        //create_charptrptr(args_buf, args_ptr, sizeof(args_buf));
        //sys_write(1, args_ptr[0], 100);
        pid_t forked = 1;
        if(!forked) {
            sys_execve(input_buf, 0, 0);
            sys_write(1, "execve failed!\n", 15);
            return 1; // execve failed
        }
        sys_waitpid(forked, 0, 0);
    }

    return 0;
}