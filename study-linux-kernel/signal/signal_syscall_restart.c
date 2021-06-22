/**
 * @file signal_syscall_restart.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 测试SA_RESTART对系统调用是否重启
 * @version 0.1
 * @date 2021-06-22
 * 该示例验证对信号处理添加SA_RESTART标志后， select调用将返回EINTR, read读取终端返回实际字节数
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define TEST_INVALID_SA_RESTART 1


static void sigalarm_handler(int sig)
{
    fprintf(stderr, "One second is up.\n");
}

int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigalarm_handler;
    if(sigaction(SIGALRM, &sa, NULL) == -1){
        return EXIT_FAILURE;
    }
    
    struct itimerval tmval;
    memset(&tmval, 0, sizeof (struct itimerval));
    tmval.it_interval.tv_sec = 1;
    tmval.it_value.tv_sec = 1;
    if(setitimer(ITIMER_REAL, &tmval, NULL) == -1){
        return EXIT_FAILURE;
    }

    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flag | O_NONBLOCK);
    for(;;)
    {
        fd_set fdread;
        FD_ZERO(&fdread);
        FD_SET(STDIN_FILENO, &fdread);
        int nfds = STDIN_FILENO + 1;
#if TEST_INVALID_SA_RESTART
        if (select(nfds, &fdread, NULL, NULL, NULL) == -1){
            fprintf(stderr, "select error: %s.\n", strerror(errno));
            continue;
        }
#endif // TEST_INVALID_SA_RESTART
        for (ssize_t i = 0;i <  nfds; ++i)
        {
            char msg[1024]={0};
            const ssize_t readnum = read(STDIN_FILENO, msg, sizeof msg);
            if (FD_ISSET(i, &fdread) &&  readnum> 0){
                write(STDERR_FILENO, msg, readnum);
            }
        }
    }

    return 0;
}
