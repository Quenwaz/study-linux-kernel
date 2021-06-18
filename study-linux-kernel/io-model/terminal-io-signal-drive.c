/**
 * @file terminal-io-signal-drive.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 终端输入之信号驱动I/O
 * @version 0.1
 * @date 2021-06-18
 * 
 * 此示例演示了使用信号驱动I/O方式对终端输入文件I/O的异步通知
 * 需使用实时信号， 否则不会排队造成丢失。
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define SIG_DRIVE_IO (SIGRTMIN+2)

static void sigHandler(int sig, siginfo_t * siginfo, void * data)
{
    char msg[1024]={0};
    ssize_t numread = read(siginfo->si_fd, msg, sizeof msg);
    if (numread > 0)
        write(STDERR_FILENO, msg,numread);
}

int main(int argc, char const *argv[])
{   
    fprintf(stderr, "非阻塞终端输入之信号驱动测试程序\n\n");
    /// step 1: 设置信号处理器
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART |SA_SIGINFO;
    sa.sa_sigaction = sigHandler;
    if(sigaction(SIG_DRIVE_IO, &sa, NULL) == -1){
        return 1;
    }

    /// step 2: 指定文件描述符的信号及信号接收进程
    if( fcntl(STDIN_FILENO, F_SETSIG, SIG_DRIVE_IO) ==-1 ||
        fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1){
        return 1;
    }

    /// step 3: 设置文件描述符异步非阻塞
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1){
        return 1;
    }

    // step 4：循环阻塞非SIG_DRIVE_IO信号， 以等待SIG_DRIVE_IO信号
    sigset_t set_block, setold;
    sigemptyset(&set_block);
    sigaddset(&set_block, SIG_DRIVE_IO);
    sigprocmask(SIG_BLOCK, &set_block, &setold);
    for(;;)
    {
        sigsuspend(&setold);
        // sigwaitinfo();
        // pause();
    }
    sigprocmask(SIG_UNBLOCK, &set_block, NULL);
    return 0;
}
