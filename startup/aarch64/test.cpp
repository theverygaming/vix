void puts(char *str) {
    while (*str) {
        while ((*((unsigned volatile int*)0x3F201018)) & 0x20) {}
        *((unsigned volatile int*)0x3F201000) = *str;
        if (*str == '\n') {
            while ((*((unsigned volatile int*)0x3F201018)) & 0x20) {}
            *((unsigned volatile int*)0x3F201000) = '\r';
        }
        str++;
    }
}

char c = 0;

void main() {
    puts("hello world!\n");
    char a[] = "a";
    while (1) {
        a[0] = c;
        puts(a);
        c++;
    }
}

// run with make clean MAKE_ARCH=aarch64 && make MAKE_ARCH=aarch64  && qemu-system-aarch64 -M raspi3b -kernel startup/aarch64/startup -serial stdio
