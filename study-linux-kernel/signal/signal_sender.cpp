/**
 * @file signal_sender.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号发送器
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>  // for atoi

int main(int argc, char const *argv[])
{
    if (argc < 4){
        fprintf(stderr, "Usage: signal_sender [pid] [signal number] [times].\n");
        return 1;
    }

    const pid_t pid = atoi(argv[1]);
    const int signo = atoi(argv[2]);
    const int times = atoi(argv[3]);
    
    fprintf(stderr, "ready sending signal %d to process %ld %d times.\n", signo, pid, times);

    for (size_t i =0;i < times; ++i){
        if (kill(pid, signo) == -1){
            fprintf(stderr, "send signal %d failed, stop...\n", signo);
            return 1;
        }
    }

    fprintf(stderr, "finish send, will interrupt process %d.\n", pid);
    kill(pid, SIGINT);
    return 0;
}
