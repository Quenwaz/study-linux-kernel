/**
 * @file terminal-io-poll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief  终端poll I/O
 * @version 0.1
 * @date 2021-06-15
 * 此示例展示了多路复用I/O poll方式进行无阻塞终端的用法
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>


int main(int argc, char const *argv[])
{
    struct pollfd *pollfd;
    pollfd = calloc(1, sizeof(struct pollfd));
    memset(&pollfd[0], 0, sizeof(struct pollfd));
    pollfd[0].fd= STDIN_FILENO;
    pollfd[0].events = POLLIN;
    
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK | fcntl(STDIN_FILENO, F_GETFL, 0));

    for(;;)
    {
        const int ret = poll(pollfd, 1, 15 * 1000);
        if (ret < 0){
            return 1;
        }else if (ret == 0){
            fprintf(stderr, "error: timeout.\n");
            continue;
        }

        for (ssize_t i = 0;i < 1; ++i)
        {
            if(pollfd[i].revents & POLLIN)
            {
                char msg[1024]={0};
                if(read(STDIN_FILENO, msg, sizeof(msg)) == -1)
                {
                    fprintf(stderr, "error:%s\n", strerror(errno));
                }
                else write(STDOUT_FILENO, msg, sizeof msg);
            }
        }
    }
    free(pollfd);
    return 0;
}
