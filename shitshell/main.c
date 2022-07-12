#include "syscall.h"
#include "stdlib.h"

char input_buf[100];

void replace_char(char* str, size_t len,  char c, char r) {
    while(len--) {
        if(*str == c) {
            *str = r;
            return;
        }
        str++;
    }
}

int main(int argc, char* argv[]) {
    sys_write(1, "# ", 2);
    sys_read(1, input_buf, sizeof(input_buf));
    replace_char(input_buf, sizeof(input_buf), '\n', '\0');
    pid_t forked = sys_fork();
    if(!forked) {
        sys_execve(input_buf, 0, 0);
        return 1; // execve failed
    }
    sys_waitpid(forked, 0, 0);

    return 0;
}