#include "syscall.h"
#include "stdlib.h"

int main(int argc, char* argv[]) {
    char yes[128] = "hewwo\n";
    sys_write(1, &yes, sizeof(yes));
    int ret = sys_read(1, &yes, sizeof(yes)-1);
    sys_write(1, &yes, sizeof(yes));
    return ret;
}