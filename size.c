#include <ucontext.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    printf("Size of int: %zu\n", sizeof(int));
    printf("Size of ucontext_t: %zu\n", sizeof(ucontext_t));
    printf("Size of uc_flags: %zu\n", sizeof(unsigned long int));
    printf("Size of uc_link: %zu\n", sizeof(ucontext_t *));
    printf("Size of uc_sigmask: %zu\n", sizeof(sigset_t));
    printf("Size of uc_stack: %zu\n", sizeof(stack_t));
    printf("Size of uc_mcontext: %zu\n", sizeof(mcontext_t));
    printf("Size of fpstate: %zu\n", (size_t) 512);
}
