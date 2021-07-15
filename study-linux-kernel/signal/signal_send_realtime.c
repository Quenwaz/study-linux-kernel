/**
 * @file signal_send_realtime.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 发送实时信号
 * @version 0.1
 * @date 2021-07-07
 *  由于kill只能发送标准信号， 所以写一个发送实时信号的工具
 * @copyright Copyright (c) 2021 Quemwaz
 * 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc != 4){
        fprintf(stderr, "Usage: %s [pid] [signal number] [attach].\n\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = atoi(argv[1]);
    int sig = atoi(argv[2]);

    union sigval sv;
    sv.sival_int = atoi(argv[3]);
    if (sigqueue(pid, sig, sv) == -1){
        fprintf(stderr, "send failed! %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr, "send successfully!\n");
    return 0;
}
