#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// #define THREADLOCALTEST

#ifndef THREADLOCALTEST

void *thread(void *vargp) {
    while (true) {
        printf("PID: %d TID: %d\n", getpid(), gettid());
    }
    return NULL;
}

__thread int t_local = 69420;

int main(int argc, char *argv[]) {
    printf("t_local value: %d address: %p\n", t_local, &t_local);
    pthread_t thread_id;
    printf("Before Thread PID: %d\n", gettid());
    for (int i = 0; i < 3; i++) {
        fprintf(stderr, "pthread_create\n");
        pthread_create(&thread_id, NULL, thread, NULL);
        fprintf(stderr, "pthread_create_end\n");
    }
    printf("waitpid\n");
    waitpid();
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    return 0;
}

#else

__thread int t_local = 69420;

int main(int argc, char *argv[]) {
    printf("t_local value: %d address: %p\n", t_local, &t_local);
    return 0;
}

#endif
