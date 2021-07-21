/**
 * @file terminal-io-epoll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief epoll I/O模型实现边缘触发
 * @version 0.1
 * @date 2021-07-16
 * 此示例演示如何解决边缘触发中出现的饥饿问题, 基本思路是:
 * 1. 构造就绪列表
 * 2. epoll_wait()获取就绪文件描述符后， 加入到就绪列表
 * 3. 循环执行就绪列表中的I/O，当列表不为空时， 超时设置为0， 否则为空时， 设置为-1阻塞等待。
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


typedef struct ready_io_file_descriptor{
    int size;
    int capacity;
    int curpos;
    struct epoll_event *ep_events;
}ready_io_fds;


int fd_exist_list(ready_io_fds* ready_fds, struct epoll_event * fd)
{
    for(size_t i =0;i < ready_fds->size; ++i)
    {
        if (ready_fds->ep_events[i].data.fd == fd->data.fd){
            return 1;
        }
    }

    return 0;
}

int add_fd_to_list(ready_io_fds* ready_fds, struct epoll_event * fd)
{
    if (ready_fds == NULL){
        return -1;
    }

    if (fd_exist_list(ready_fds, fd) == 1){
        return 0;
    }

    if (ready_fds->capacity <= ready_fds->size){
        if(ready_fds->capacity == 0 && ready_fds->size == 0){
            ready_fds->capacity = 1;
        }

        ready_fds->capacity *=2;
        struct epoll_event* temp_fds = malloc(ready_fds->capacity * sizeof(struct epoll_event));
        if (temp_fds == NULL){
            return -1;
        }

        memcpy(temp_fds, ready_fds->ep_events, sizeof(struct epoll_event) *  ready_fds->size);
        free(ready_fds->ep_events);
        ready_fds->ep_events = temp_fds;
    }

    memcpy(&ready_fds->ep_events[ready_fds->size], fd, sizeof(struct epoll_event));
    return ++ready_fds->size;
}

struct epoll_event* get_cur_file_descriptor(ready_io_fds* ready_fds)
{
    if (ready_fds == NULL || ready_fds->size < 1 || 
        ready_fds->curpos >= ready_fds->size){
        return NULL;
    }

    return &ready_fds->ep_events[ready_fds->curpos++];
}




int remove_file_descriptor(ready_io_fds* ready_fds, struct epoll_event * fd)
{
    if (fd_exist_list(ready_fds, fd) == 0){
        return 1;
    }

    struct epoll_event* temp_fds = malloc(ready_fds->capacity * sizeof(struct epoll_event));
    if (temp_fds == NULL){
        return -1;
    }

    for(size_t i =0;i < ready_fds->size; ++i)
    {
        if (ready_fds->ep_events[i].data.fd == fd->data.fd){
            continue;
        }

        memcpy(&temp_fds[i], &ready_fds->ep_events[i], sizeof(struct epoll_event));
    }

    free(ready_fds->ep_events);
    ready_fds->ep_events = temp_fds;
    memcpy(ready_fds->ep_events, temp_fds, sizeof(struct epoll_event) * ready_fds->capacity);
    --ready_fds->size;
    if (ready_fds->curpos == ready_fds->size){
        ready_fds->curpos = 0;
    }
    return 0;
}


int main(int argc, char const *argv[])
{
    int fd_pipe[2]={0};
    if (pipe(fd_pipe) == -1){
        return EXIT_FAILURE;
    }

    pid_t pid_child = -1;
    switch ((pid_child = fork()))
    {
    case -1:
        exit(EXIT_FAILURE);
    case 0:
    {
        if (close(fd_pipe[0]) == -1){
            exit(EXIT_FAILURE);
        }

        fd_set set_read;
        int nfds = STDIN_FILENO + 1;
        for(;;)
        {
            FD_ZERO(&set_read);
            FD_SET(STDIN_FILENO, &set_read);
            int nready = select(nfds, &set_read, NULL, NULL, NULL);
            if (nready <= 0)
                continue;

            fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO,F_GETFL) | O_NONBLOCK);

            char msg[512]={0};
            ssize_t numread = -1;
            if( FD_ISSET(STDIN_FILENO, &set_read) && 
                (numread=read(STDIN_FILENO, msg, sizeof msg)) > 0)
            {
                fprintf(stderr, "pid[%d] read number of byte: %d, error:%s\n", getpid(), numread, strerror(errno));
                if (-1 == write(fd_pipe[1], msg, numread)){
                    
                    break;
                }
            }
        }
        fprintf(stderr, "%d: exit.\n", getpid());
        close(fd_pipe[1]);
        exit(EXIT_SUCCESS);
    }
        break;
    default:
        break;
    }

    if (close(fd_pipe[1]) == -1){
        exit(EXIT_FAILURE);
    }

    int fd_epoll = epoll_create(2);
    if (fd_epoll == -1){
        return EXIT_FAILURE;
    }

    fcntl(fd_pipe[0], F_SETFL, fcntl(fd_pipe[0],F_GETFL) | O_NONBLOCK);

    struct epoll_event event_epoll;
    event_epoll.data.fd = fd_pipe[0];
    event_epoll.events = EPOLLIN | EPOLLET;
    if (-1 == epoll_ctl(fd_epoll, EPOLL_CTL_ADD, event_epoll.data.fd, &event_epoll)){
        return EXIT_FAILURE;
    }


    ready_io_fds ready_fds;
    memset(&ready_fds, 0, sizeof(ready_fds));
    int timeout = -1;
    for (;;)
    {
        struct epoll_event event_ready[8];
        int ready = epoll_wait(fd_epoll, event_ready, sizeof event_ready / sizeof event_ready[0], timeout);
        if (ready == -1){
            break;
        }

        for (size_t i = 0;i < ready; ++i){
            add_fd_to_list(&ready_fds, &event_ready[i]);
        }

        for (size_t i = 0; i < ready_fds.size; i++)
        {
            char msg[4]={0};
            int numread = numread=read(ready_fds.ep_events[i].data.fd, msg, sizeof msg);
            fprintf(stderr, "\npid[%d] fd[%d] read number of byte: %d\n",getpid(),ready_fds.ep_events[i].data.fd, numread);
            if (numread < 0 && errno == EAGAIN){
                remove_file_descriptor(&ready_fds, &ready_fds.ep_events[i]);
                continue;
            }
            write(STDERR_FILENO, msg, numread);
        }

        if (ready_fds.size == 0)
            timeout = -1;
        else timeout = 0;
    }
    


    return 0;
}
