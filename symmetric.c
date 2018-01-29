#include <ucontext.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static ucontext_t uctx_main, uctx_func1, uctx_func2;

char func1_stack[16384];
char func2_stack[16384];

pthread_barrier_t barrier;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void func1(void)
{
    printf("func1: started\n");
    printf("func1: swapcontext(&uctx_func1, &uctx_main)\n");
    if (swapcontext(&uctx_func1, &uctx_main) == -1)
        handle_error("swapcontext");
    printf("func1: returning\n");
}

static void func2(void)
{
    printf("func2: started\n");
    printf("func2: swapcontext(&uctx_func2, &uctx_main)\n");
    if (swapcontext(&uctx_func2, &uctx_main) == -1)
        handle_error("swapcontext");
    printf("func2: returning\n");
}

void * run_thread_1(void * arg)
{
    if (getcontext(&uctx_func1) == -1)
        handle_error("getcontext");
    uctx_func1.uc_stack.ss_sp = func1_stack;
    uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
    uctx_func1.uc_link = &uctx_main;
    makecontext(&uctx_func1, func1, 0);

    printf("run_thread_1: swapcontext(&uctx_main, &uctx_func1)\n");
    if (swapcontext(&uctx_main, &uctx_func1) == -1)
        handle_error("swapcontext");

    pthread_barrier_wait(&barrier);

    printf("run_thread_1: swapcontext(&uctx_main, &uctx_func2)\n");
    if (swapcontext(&uctx_main, &uctx_func2) == -1)
        handle_error("swapcontext");

    printf("run_thread_1: returning\n");
    return NULL;
}

void * run_thread_2(void * arg)
{
    if (getcontext(&uctx_func2) == -1)
        handle_error("getcontext");
    uctx_func2.uc_stack.ss_sp = func2_stack;
    uctx_func2.uc_stack.ss_size = sizeof(func2_stack);
    uctx_func2.uc_link = &uctx_main;
    makecontext(&uctx_func2, func2, 0);

    printf("run_thread_2: swapcontext(&uctx_main, &uctx_func2)\n");
    if (swapcontext(&uctx_main, &uctx_func2) == -1)
        handle_error("swapcontext");

    pthread_barrier_wait(&barrier);

    printf("run_thread_2: swapcontext(&uctx_main, &uctx_func1)\n");
    if (swapcontext(&uctx_main, &uctx_func1) == -1)
        handle_error("swapcontext");

    printf("run_thread_2: returning\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("main: starting\n");
    pthread_t tid1;
    pthread_t tid2;
    pthread_barrier_init(&barrier, NULL, 2);

    if (pthread_create(&tid1, NULL, run_thread_1, NULL) == -1)
	    handle_error("pthread_create");
    if (pthread_create(&tid2, NULL, run_thread_2, NULL) == -1)
	    handle_error("pthread_create");

    (void) pthread_join(tid1, NULL);
    (void) pthread_join(tid2, NULL);

    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
