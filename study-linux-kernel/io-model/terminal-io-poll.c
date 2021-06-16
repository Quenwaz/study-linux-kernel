/**
 * @file terminal-io-poll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief  终端poll I/O
 * @version 0.1
 * @date 2021-06-15
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>



int main(int argc, char const *argv[])
{
    struct pollfd *pollfd;
    pollfd = calloc(1, sizeof(struct pollfd));
    memset(&pollfd[0], 0, sizeof(struct pollfd));
    pollfd[0].fd= STDIN_FILENO;
    pollfd[0].events = POLLIN;
    for(;;)
    {
        int ret = poll(pollfd, 1, 15 * 1000);
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
                read(STDIN_FILENO, msg, sizeof(msg));
                write(STDOUT_FILENO, msg, sizeof msg);
            }
        }
    }
    free(pollfd);
    return 0;
}
