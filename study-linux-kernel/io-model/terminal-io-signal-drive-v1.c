/**
 * @file terminal-io-signal-drive.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 终端输入之信号驱动I/O
 * @version 0.1
 * @date 2021-06-18
 * 
 * 此示例演示了使用信号驱动I/O方式对终端输入文件I/O的异步通知
 * 由于SIGIO是标准信号。 会存在同类信号丢失的情况， 而且信号处理函数无法区分来自哪个文件描述符的信号
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termio.h>
#include <string.h>

static volatile sig_atomic_t got_sigio = 0;


static void sigHandler(int sig)
{
    got_sigio = 1;
}

int main(int argc, char const *argv[])
{   
    fprintf(stderr, "非阻塞终端输入之信号驱动测试程序\n\n");
    /// step 1: 设置信号处理器
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigHandler;
    if(sigaction(SIGIO, &sa, NULL) == -1){
        return 1;
    }

    /// step 2: 指定文件描述符信号接收进程
    if(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1){
        return 1;
    }

    /// step 3: 设置文件描述符异步非阻塞
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1){
        return 1;
    }

    for (;;)
    {
        for (size_t i = 0;i < (1e8); ++i) continue;
        if(got_sigio){
            char msg[1024]={0};
            ssize_t numread = read(STDIN_FILENO, msg, sizeof msg);
            if (numread > 0)
                write(STDERR_FILENO, msg,numread);
            got_sigio = 0;
        }
    }

    return 0;
}
