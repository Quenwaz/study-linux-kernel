/**
 * @file signal_sigsuspend.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 此示例演示等待信号sigsuspend的调用
 * @version 0.1
 * @date 2021-07-07
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

static void handler(int sig)
{
    fprintf(stderr, "Received signal %d\n", sig);

    sigset_t setwait;
    sigemptyset(&setwait);
    sigpending(&setwait);

    for (int i = SIGHUP;i <= SIGUNUSED; ++i)
    {
        if(sigismember(&setwait, i)){
            fprintf(stderr, "Signal %s(%d) is pending\n", strsignal(i), i);
        }
    }
}

int main(int argc, char const *argv[])
{
    fprintf(stderr, "此示例演示等待信号sigsuspend的调用，程序设置了SIGINT的处理程序，等待过程中阻塞了SIGQUIT信号。\n");

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1){
        return EXIT_FAILURE;
    }

    for(;;)
    {
        sigset_t setblock;
        sigemptyset(&setblock);
        sigaddset(&setblock, SIGQUIT);
        if(sigsuspend(&setblock) == -1 && errno != EINTR){
            return EXIT_FAILURE;
        }else{
            fprintf(stderr, "Already trigger signal\n");
        }
    }

    return EXIT_SUCCESS;
}
