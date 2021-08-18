/**
 * @file socket_stream_base_server_v1.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 流socket 服务端基本过程
 * @version 0.1
 * @date 2021-07-18
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    if (argc != 2){
        fprintf(stderr, "invalid input parameter.\n\nUsage: %s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int socket_svr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_svr == -1){
        fprintf(stderr, "call socket failed:%s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));
    if (bind(socket_svr, (struct sockaddr*)&addr, sizeof (struct sockaddr_in)) == -1){
        fprintf(stderr, "call bind failed:%s\n", strerror(errno));
        goto exit;
    }

    if (listen(socket_svr, 4) == -1){
        fprintf(stderr, "call listen failed:%s\n", strerror(errno));
        goto exit;
    }

    fprintf(stderr, "listening to %s....\n", argv[1]);

    for(;;)
    {
        struct sockaddr_in addr_from_remote;
        socklen_t len = sizeof(struct sockaddr_in);
        memset(&addr_from_remote, 0, len);
        int socket_clt = accept(socket_svr, (struct sockaddr*)&addr_from_remote, &len);
        if (socket_clt == -1){
            fprintf(stderr, "call accept failed:%s\n", strerror(errno));
            continue;
        }

        fprintf(stderr, "accept connect from %s\n", inet_ntoa(addr_from_remote.sin_addr));

        char msg[1024] = {0};
        ssize_t n_rcv = recv(socket_clt, msg, sizeof(msg), 0);
        if (n_rcv == -1){
            fprintf(stderr, "call recv failed:%s\n", strerror(errno));
        }else if (n_rcv == 0){
            fprintf(stderr, "remote closed.\n");
        }else if (send(socket_clt, msg, n_rcv, 0) == -1){
            fprintf(stderr, "call send failed:%s\n", strerror(errno));
        }else 
            fprintf(stderr, "send succeed:%s\n",msg);

        close(socket_clt);
    }

exit:
    close(socket_svr);
    return EXIT_SUCCESS;
}
