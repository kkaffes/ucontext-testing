#define _GNU_SOURCE

#include <ucontext.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>


#include <sys/types.h>
#include <sys/resource.h>

static ucontext_t uctx_main, uctx_func;

char func_stack[16384];

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int run_on_core(int core_id)
{
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

static void func(void)
{
    while (1)
        if (swapcontext(&uctx_func, &uctx_main) == -1)
            handle_error("swapcontext");
}

int main(int argc, char *argv[])
{
    struct timespec start, end;
    uint64_t start64, end64;
    int i, which, priority;
    id_t pid;

    run_on_core(1);

    which = PRIO_PROCESS;
    priority = -20;
    pid = getpid();
    if (setpriority(which, pid, priority) == -1)
        handle_error("set_priority");

    if (getcontext(&uctx_func) == -1)
        handle_error("getcontext");
    uctx_func.uc_stack.ss_sp = func_stack;
    uctx_func.uc_stack.ss_size = sizeof(func_stack);
    uctx_func.uc_link = &uctx_main;
    makecontext(&uctx_func, func, 0);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i=0; i < 100000; i++)
        if (swapcontext(&uctx_main, &uctx_func) ==  -1)
            handle_error("swapcontext");

    clock_gettime(CLOCK_MONOTONIC, &end);

    start64 = 10e9 * start.tv_sec + start.tv_nsec;
    end64 = 10e9 * end.tv_sec + end.tv_nsec;

    printf("Average context switch time is: %lu ns\n",
           (end64 - start64)/200000);
    exit(EXIT_SUCCESS);
}
