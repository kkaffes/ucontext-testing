#include <ucontext.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

__thread ucontext_t uctx_main;
static ucontext_t uctx_func;

char func_stack[16384];

pthread_barrier_t barrier;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void update_uc_link(ucontext_t * ucp, ucontext_t *uc_link)
{
    unsigned long * sp = (unsigned long int *) ( ucp->uc_stack.ss_sp
                          + ucp->uc_stack.ss_size);
    sp -= 1;
    sp = (unsigned long *) ((((uintptr_t) sp) & -16L) - 8);
    sp[1] = (unsigned long) uc_link;
}

static void func(void)
{
    ucontext_t local_uctx;
    getcontext(&local_uctx);
    local_uctx.uc_link = &uctx_main;
    printf("func: started\n");
    printf("func: returning\n");
}

void * run_thread_1(void * arg)
{
    // Wait for second thread to create uctx_func2
    pthread_barrier_wait(&barrier);
    // Wait for main thread to join second thread
    pthread_barrier_wait(&barrier);

    update_uc_link(&uctx_func, &uctx_main);
    printf("run_thread_1: swapcontext(&uctx_main, &uctx_func)\n");
    if (swapcontext(&uctx_main, &uctx_func) == -1)
        handle_error("swapcontext");

    printf("run_thread_1: returning\n");
    return NULL;
}

void * run_thread_2(void * arg)
{
    if (getcontext(&uctx_func) == -1)
        handle_error("getcontext");
    uctx_func.uc_stack.ss_sp = func_stack;
    uctx_func.uc_stack.ss_size = sizeof(func_stack);
    uctx_func.uc_link = &uctx_main;
    makecontext(&uctx_func, func, 0);

    pthread_barrier_wait(&barrier);

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

    (void) pthread_join(tid2, NULL);
    pthread_barrier_wait(&barrier);
    printf("main: pthread_join(tid2, NULL) returned\n");
    (void) pthread_join(tid1, NULL);

    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
