#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define NUM_THREADS 10
#define SYSCALL_WAIT_QUEUE 450

void *enter_wait_queue(void *thread_id)
{
    fprintf(stderr, "enter wait queue thread_id: %d\n", *(int *)thread_id);

    // your syscall here 
    syscall(SYSCALL_WAIT_QUEUE, 1);

    fprintf(stderr, "exit wait queue thread_id: %d\n", *(int *)thread_id);
    return NULL;
}

void *clean_wait_queue()
{
    // your syscall here 
    syscall(SYSCALL_WAIT_QUEUE, 2);
    return NULL;
}

int main()
{
    void *ret;
    pthread_t id[NUM_THREADS];
    int thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;
        pthread_create(&id[i], NULL, enter_wait_queue, (void *)&thread_args[i]);
    }

    sleep(1);

    fprintf(stderr,  "start clean queue ...\n");
    clean_wait_queue();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(id[i], &ret);
    }

    return 0;
}
