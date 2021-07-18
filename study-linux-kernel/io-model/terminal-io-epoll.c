/**
 * @file terminal-io-epoll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-16
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    fprintf(stderr, "epoll I/O 管道测试\n\n");
	int pfd[2] = {-1, -1};
	if (pipe(pfd) == -1)
	{
		fprintf(stderr, "mkpipe failure...\n");
		return EXIT_FAILURE;
	}
    fprintf(stderr, ">>>创建管道成功\n");

    pid_t pid_child = -1;
    switch ((pid_child=fork()))
    {
    case -1:
        /* code */
        break;
    case 0:
    {
        fprintf(stderr, ">>>子进程创建成功\n");
        if (close(pfd[0]) == -1){
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, ">>>子进程关闭管道读取端\n");

        
        char msg[1024]={0};
        for (int readnum = 0;
             ;)
        {
            fprintf(stderr, ">>>子进程开始从终端读入字符\n");
            if ((readnum = read(STDIN_FILENO, msg, sizeof msg)) == -1){
                break;
            }

            
            if (write(pfd[1], msg, readnum) == -1){
                break;
            }
            fprintf(stderr, ">>>子进程将读取到的字符写入管道成功\n");
        }

        fprintf(stderr, ">>>子进程关闭管道写入端并退出进程\n");
        close(pfd[1]);
        exit(EXIT_SUCCESS);
    }
        break;
    default:
        break;
    }

    if (close(pfd[1]) == -1){
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, ">>>主进程关闭管道写入端\n");

    int epoll_fd = epoll_create(8);
    if (epoll_fd == -1){
        return EXIT_FAILURE;
    }

    fprintf(stderr, ">>>主进程创建epoll实例\n");

    struct epoll_event epollev;
    epollev.events = EPOLLIN;
    epollev.data.fd = pfd[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, epollev.data.fd,&epollev) == -1){
        return EXIT_FAILURE;
    }

    fprintf(stderr, ">>>主进程将管道读取端加入epoll兴趣列表\n");

    struct epoll_event epollwt[8];
    for(;;)
    {
        fprintf(stderr, ">>>主进程epoll等待管道读取端数据...\n");
        int ready = 0;
        if(-1 == (ready = epoll_wait(epoll_fd, epollwt, 
                            sizeof(epollwt)/sizeof(epollwt[0]), -1))){
            break;                        
        }

        for (size_t i = 0; i < ready; i++)
        {   
            char msg[1024] = {0};
            int readnum = 0;
            if (epollwt[i].events == EPOLLIN &&
                (readnum=read(epollwt[i].data.fd, msg, sizeof msg)) > 0){
                fprintf(stderr, ">>>主进程收到管道数据: ");
                write(STDERR_FILENO, msg,readnum);
            }
        }
    }

    fprintf(stderr, ">>>主进程关闭epoll实例并关闭管道读取端\n");
    close(epoll_fd);
    close(pfd[0]);

    return 0;
}
