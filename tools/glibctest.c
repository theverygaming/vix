#include <stdlib.h>
#include <pthread.h>

void *myThreadFun(void *vargp)
{
    printf("am pid %d\n", gettid());
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread_id;
    printf("Before Thread PID: %d\n", gettid());
    for(int i = 0; i < 1; i++) {
        pthread_create(&thread_id, NULL, myThreadFun, NULL);
    }
    waitpid();
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    return 0;
}
