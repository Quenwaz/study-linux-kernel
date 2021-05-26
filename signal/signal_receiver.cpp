/**
 * @file signal_receiver.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号接收器
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> // for sleep

static int g_statistics[NSIG] = {0};
static volatile sig_atomic_t g_interrupt = 0;

static void signal_handler(int signo)
{
    if (signo == SIGINT){
        g_interrupt = 1;
    }
    else ++g_statistics[signo];
}

static void print_pendding_signal(FILE* fd, sigset_t* set)
{
    for (size_t i = 1; i < NSIG; ++i)
    {
        if(sigismember(set, i)){
            fprintf(fd, "%d ", i);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage: %s [sleep second].\n", argv[0]);
        return 1;
    }

    fprintf(stderr, "[pid=%ld] ready receive signal from [1~%d].\n", getpid(), NSIG);

    for (size_t i = 1; i < NSIG; ++i)
    {
        signal(i, signal_handler);
    }

    int sleep_second = atoi(argv[1]);
    if (sleep_second > 0){
        sigset_t sigset_block;
        sigfillset(&sigset_block);
        if (sigprocmask(SIG_SETMASK, &sigset_block, NULL) == -1){
            fprintf(stderr, "sigprocmask call failed.\n");
            return 1;
        }

        fprintf(stderr, "begin sleeping for %d seconds.\n", sleep_second);
        sleep(sleep_second);
        fprintf(stderr, "end sleeping for %d seconds.\n", sleep_second);
        

        sigset_t sigset_pedding;
        sigemptyset(&sigset_pedding);
        if(sigpending(&sigset_pedding) == -1){
            fprintf(stderr, "sigpending call failed.\n");
            return 1;
        }

        fprintf(stderr, "current pedding signal: ");
        print_pendding_signal(stderr, &sigset_pedding);
        fprintf(stderr, "\n");

        sigset_t sigset_empty;
        sigemptyset(&sigset_empty);
        if (sigprocmask(SIG_SETMASK, &sigset_empty, NULL) == -1){
            fprintf(stderr, "sigprocmask call failed.\n");
            return 1;
        }
    }

    // loop the process until the SIGINT signal arrives.
    for(;g_interrupt == 0;) continue;
    
    for(size_t i = 1;i < NSIG; ++i){
        if (g_statistics[i] != 0){
            fprintf(stderr, "signal %d trigger %d times.\n", i, g_statistics[i]);
        }
    }

    return 0;
}

