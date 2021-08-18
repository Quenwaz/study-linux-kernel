/**
 * @file socket_stream_base_client_v1.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 流socket 客户端基本过程
 * @version 0.1
 * @date 2021-07-18
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE // for sigaction
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>



typedef void(*FnSigHandler)(int);
void sighandler(int signo)
{
    fprintf(stderr, "recv signal SIGPIPE\n");
    abort();
}


int proces_sigpipe_signal(FnSigHandler handler)
{
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    return sigaction(SIGPIPE, &sa, NULL);
}


int main(int argc, char const *argv[])
{
    if (argc != 3){
        fprintf(stderr, "invalid input parameter.\n\nUsage: %s [IP] [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (proces_sigpipe_signal(sighandler) == -1){
        fprintf(stderr, "procee SIGPIPE failed.\n");
        return EXIT_FAILURE;
    }

    int socket_clt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_clt == -1){
        fprintf(stderr, "call socket failed:%s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    if (connect(socket_clt, (struct sockaddr*)&addr, sizeof (addr)) == -1){
        fprintf(stderr, "call connect failed:%s\n", strerror(errno));
        goto exit;
    }

    fprintf(stderr, "connect to %s:%s\n", argv[1], argv[2]);

    for(;;)
    {
        char msg[1024] = {0};
        fprintf(stderr,"please input any character: ");
        int num_read = read(STDIN_FILENO, msg, sizeof msg);
        if (num_read <= 0){
            fprintf(stderr, "call read failed:%s\n", strerror(errno));
        }

        if (send(socket_clt, msg, num_read, 0) <= 0){
            fprintf(stderr, "call send failed:%s\n", strerror(errno));
        }else{
            fprintf(stderr, "send succeed:%s\n", msg);
        }

        ssize_t n_rcv = recv(socket_clt, msg, sizeof(msg), 0);
        if (n_rcv == -1){
            fprintf(stderr, "call recv failed:%s\n", strerror(errno));
        }else if (n_rcv == 0){
            fprintf(stderr, "remote closed:%s.\n", strerror(errno));
        }else{
            fprintf(stderr, "recv succeed:%s\n", msg);
        }
    }

exit:
    close(socket_clt);
    return EXIT_SUCCESS;
}
