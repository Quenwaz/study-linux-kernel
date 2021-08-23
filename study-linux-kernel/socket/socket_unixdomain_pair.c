/**
 * @file socket_unixdomain_pair.c
 * @author Quenwaz (404937333@qq.com)
 * @brief socketpair 学习， 类似管道。 用于IPC
 * @version 0.1
 * @date 2021-08-23
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>


#define SWITCH_TO_DGRAM
#ifdef SWITCH_TO_DGRAM
#define SOCK_TYPE SOCK_DGRAM
#else
#define SOCK_TYPE SOCK_STREAM
#endif

static void strhelper_toupper(char* string)
{
    while ((*string) != '\0')
    {
        if (isalpha((*string)))
            (*string) = toupper(*string);
        ++string;
    }
}


int main(int argc, char const *argv[])
{
    int sock_pair[2] ={0};
    if (socketpair(AF_UNIX, SOCK_TYPE, 0, sock_pair) == -1){
        return EXIT_FAILURE;
    }

    pid_t pid_child = -1;
    switch ((pid_child = fork()))
    {
    case -1:
        fprintf(stderr, "occur error: %s\n", strerror(errno));
        break;
    case 0:
    {
        close(sock_pair[0]);
        int socket_cur_hold = sock_pair[1];
        const pid_t pid = getpid();
        for(;;)
        {
            fprintf(stderr, "[%d]please input message:", pid);
            char msg[1024] ={0};
            ssize_t num_read = read(STDIN_FILENO, msg, sizeof msg);
            if (num_read <= 0){
                continue;
            }
            if (write(socket_cur_hold, msg, num_read) <= 0){
                break;
            }

            memset(msg, 0, sizeof msg);
            num_read = read(socket_cur_hold, msg, sizeof msg);
            if (num_read <= 0){
                break;
            }

            fprintf(stderr, "[%d]recv msg: %s\n", pid, msg);
        }

        fprintf(stderr, "[%d] close socket: %s", pid, strerror(errno));
        close(socket_cur_hold);
        exit(EXIT_SUCCESS);
    }
        break;
    default:
        break;
    }

    close(sock_pair[1]);
    int socket_cur_hold = sock_pair[0];
    pid_t pid = getpid();
    for(;;)
    {
        char msg[1024] ={0};
        ssize_t num_read = read(socket_cur_hold, msg, sizeof msg);
        if (num_read <= 0){
            break;
        }
        
        
        fprintf(stderr, "[%d]recv msg: %s\n", pid, msg);
        strhelper_toupper(msg);
        if (write(socket_cur_hold, msg, num_read) <= 0){
            break;
        }
    }

    fprintf(stderr, "[%d] close socket: %s", pid, strerror(errno));
    close(socket_cur_hold);
    exit(EXIT_SUCCESS);
    return 0;
}
