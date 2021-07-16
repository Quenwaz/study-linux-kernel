/**
 * @file terminal-io-signal-drive.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 终端输入之信号驱动I/O
 * @version 0.1
 * @date 2021-06-25
 * 
 * 此示例尝试解决信号驱动I/O中出现的实时信号队列问题。 
 * 按照书中的说明:
 * 1. 设置SIGIO信号处理程序
 * 2. 设置文件描述符的实时信号
 * 3. 当获取到SIGIO信号时， 利用sigwaitinfo获取等待队列中的信号(实时信号)
 * 4. 获取完之后， 利用IO多路复用(select/poll)获取剩余未处理的I/O事件
 * 
 * 此尝试并未成功， 因为设置了实时信号作为文件描述符的I/O就绪信号， 就无法接收
 * 到SIGIO信号
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define SIG_DRIVE_IO (SIGRTMIN+2)

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

    /// step 2: 指定文件描述符的信号及信号接收进程
    if( fcntl(STDIN_FILENO, F_SETSIG, SIG_DRIVE_IO) == -1 ||
        fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1){
        return 1;
    }

    /// step 3: 设置文件描述符异步非阻塞
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1){
        return 1;
    }


    // 阻塞除SIGKILL和SIGSTOP的所有标准信号
    // 避免信号在获取等待队列之前到达，而执行默认行为
    sigset_t setblock;
    sigfillset(&setblock);
    sigdelset(&setblock, SIGIO);
    if(sigprocmask(SIG_SETMASK, &setblock, NULL) == -1){
        return 1;
    }

    for (;;)
    {
        for (size_t i = 0;i < (1e8); ++i) 
            continue;

        if(got_sigio)
        {
            siginfo_t si;
            memset(&si, 0, sizeof(si));
            const int sig = sigwaitinfo(&setblock, &si);
            if (sig == -1){
                fprintf(stderr, "occur error: %s\n", strerror(errno));
                break;
            }

            assert(sig == si.si_signo);
            assert(SI_KERNEL == si.si_code || SI_QUEUE == si.si_code);
            fprintf(stderr, "Received signal %s(%d)\n", strsignal(sig), sig);

            if (si.si_fd == STDIN_FILENO)
            {
                char msg[1024]={0};
                const ssize_t numread = read(STDIN_FILENO, msg, sizeof msg);
                if (numread > 0)
                    write(STDERR_FILENO, msg,numread);
            }
            got_sigio = 0;
        }
    }

    return 0;
}
