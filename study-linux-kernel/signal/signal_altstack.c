/**
 * @file signal_altstack.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 验证信号处理函数使用备用栈
 * @version 0.1
 * @date 2021-06-22
 * 该示例在【Linux version 3.10.0-862.el7.x86_64】上未达到预期效果
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strsignal
#include <unistd.h> // for _exit
#include <alloca.h>

static void sigsegv_handler(int sig)
{
    int x;
    fprintf(stderr, "Caught signal %d (%s)\n", sig, strsignal(sig));
    fprintf(stderr, "Top of handler stack near  %10p\n", (void*)&x);
    fflush(stderr);
    _exit(EXIT_FAILURE);
}

static void overflow_stack(int callnum)
{
    char a[1000000]={0};
    fprintf(stderr, "Call %4d - top of stack near %10p \n", callnum, &a[0]);
    overflow_stack(callnum + 1);
}

int main(int argc, char const *argv[])
{
    int top = 0;
    fprintf(stderr, "Top of standard stack is near %10p\n", (void*)&top);

    stack_t sigstack;
    sigstack.ss_size = SIGSTKSZ;
    sigstack.ss_sp = malloc(sigstack.ss_size);
    sigstack.ss_flags = 0;
    if (sigaltstack(&sigstack, NULL) == -1){
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Alternate stack is at %10p-%p\n", sigstack.ss_sp, (char*)sbrk(0) - 1);

    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    if (sigaction(SIGSEGV, &sa, NULL) == -1){
        return EXIT_FAILURE;
    }
    
    overflow_stack(1);
    return 0;
}
