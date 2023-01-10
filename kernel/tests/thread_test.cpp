#include <arch/thread.h>
#include <tests/testprint.h>
#include <types.h>

#define THREAD_COUNT 1 // max 10

static void thread(void *arg) {
    /*
    asm volatile("cli");
    printf("arg: 0x%p\n", arg);
    asm volatile("sti");
    uintptr_t n = (uintptr_t)arg;
    // char str[] = "thread #0";
    // str[8] = '0' + n;
    asm volatile("cli");
    TEST("thread", "thread 0", true);
    asm volatile("sti");
    asm volatile("mov $1, %eax; mov $2, %ebx; int $0x80; .loop:;hlt; jmp .loop;");
    */
}

void thread_test() {
    for (int i = 0; i < THREAD_COUNT; i++) {
        // std::thread t(thread, (void *)i);
    }
}
