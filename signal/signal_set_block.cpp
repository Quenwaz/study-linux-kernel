/**
 * @file signal_set_block.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号集API使用
 * @version 0.1
 * @date 2021-05-25
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <signal.h>


static void signal_handler(int signo)
{
    fprintf(stderr, "Trigger the signal SIGQUIT(%d).\n", signo);
    sigset_t sigset_pendding;
    sigemptyset(&sigset_pendding);
    if(sigpending(&sigset_pendding) == 0){
        fprintf(stderr, "SIGINT is %s.\n", sigismember(&sigset_pendding, SIGINT)? "pendding":"not peddidng");
    }
}

int main(int argc, char const *argv[])
{
    /*Initialize*/
    sigset_t sigset_block, sigset_last;
    sigemptyset(&sigset_block);
    sigaddset(&sigset_block, SIGINT);

    /*Block SIGINT*/
    if (sigprocmask(SIG_BLOCK, &sigset_block, &sigset_last) == -1){
        fprintf(stderr, "The system call sigprocmask filed.\n");
        return 1;
    }


    /*Set up SIGQUIT signal processing.*/
    signal(SIGQUIT, signal_handler);

    // Suspend the process until the signal arrives.
    for(;;) pause();
    return 0;
}