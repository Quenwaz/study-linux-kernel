/**
 * @file signal_realtime.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 此示例演示了处理实时信号和发送实时信号的过程
 * @version 0.1
 * @date 2021-07-06
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#define MY_REALTIME_SIGNAL  (SIGRTMIN+3)


void handler(int sig,  siginfo_t * siginfo, void * data)
{
    assert(siginfo->si_signo == sig);
    assert(siginfo->si_code == SI_QUEUE);
    assert(siginfo->si_pid == getpid());

    fprintf(stderr, "recv real-time signal %d\n",siginfo->si_value.sival_int);
}

int main(int argc, char const *argv[])
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(MY_REALTIME_SIGNAL, &act, NULL) == -1){
        return 1;
    }

    union  sigval sv;
    sv.sival_int = 123456;
    if(sigqueue(getpid(), MY_REALTIME_SIGNAL, sv) == -1){
        return 1;
    }
    
    return 0;
}
