/**
 * @file terminal-io-select.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 终端select I/O
 * @version 0.1
 * @date 2021-06-15
 * 此示例展示了无阻塞终端的用法
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    fd_set readfds, writefds;
    struct timeval timeout;


    for (;;)
    {
        memset(&timeout, 0, sizeof(struct timeval));
        timeout.tv_sec = 15;

        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(STDOUT_FILENO, &writefds);
        const int nfds = STDOUT_FILENO + 1;
        int ret = select(nfds, &readfds, NULL, NULL, &timeout);
        if (ret < 0){
            fprintf(stderr, "error: %s\n", strerror(errno));
            return 1;
        }else if(ret == 0)
        {
            fprintf(stderr, "error: timeout.\n");
            continue;
        }

        char msg[1024]={0};
        for (int fd = 0; fd < nfds; ++fd)
        {
            if (FD_ISSET(fd,&readfds))
            {
                read(STDIN_FILENO, msg, sizeof(msg));
            }else if (FD_ISSET(fd,&writefds) && strlen(msg) > 0){
                write(STDOUT_FILENO, msg, sizeof msg);
            }
        }
    }


    return 0;
}
