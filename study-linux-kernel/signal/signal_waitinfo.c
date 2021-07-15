/**
 * @file signal_waitinfo.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 此示例演示了sigwaitinfo的用法
 * @version 0.1
 * @date 2021-07-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

int main(int argc, char const *argv[])
{
    sigset_t setblock;
    sigfillset(&setblock);

    // 阻塞除SIGKILL和SIGSTOP的所有标准信号
    if(sigprocmask(SIG_SETMASK, &setblock, NULL) == -1){
        return EXIT_FAILURE;
    }

    for (;;)
    {
        siginfo_t si;
        memset(&si, 0, sizeof(si));
        int sig = sigwaitinfo(&setblock, &si);
        if (sig == -1){
            fprintf(stderr, "occur error: %s\n", strerror(errno));
            break;
        }

        assert(sig == si.si_signo);
        assert(SI_KERNEL == si.si_code || SI_QUEUE == si.si_code);
        fprintf(stderr, "Received signal %s(%d)\n", strsignal(sig), sig);
    }

    return 0;
}
