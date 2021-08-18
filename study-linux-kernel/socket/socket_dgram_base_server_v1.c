/**
 * @file socket_dgram_base_server.v1.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 数据报socket 服务端基础API使用
 * @version 0.1
 * @date 2021-07-18
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>



int main(int argc, char const *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Invalid input parameter.\n Usage: %s [PORT]\n\n", argv[0]);
        return EXIT_FAILURE;
    }

    int socket_svr = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_svr == -1){
        fprintf(stderr, "call socket failed: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));
    if (-1 == bind(socket_svr, (struct sockaddr*)&addr, sizeof(addr))){
        fprintf(stderr, "call bind failed: %s.\n", strerror(errno));
        goto exit;
    }

    fprintf(stderr, "binded address %s:%s...", inet_ntoa(addr.sin_addr), argv[1]);

    for(;;)
    {
        struct sockaddr_in addr_from;
        socklen_t len = sizeof(addr_from);
        char msg[1024] ={0};
        ssize_t n_recv = recvfrom(socket_svr,msg, sizeof(msg), 0, (struct sockaddr*)&addr_from, &len);
        if (n_recv == -1){
            fprintf(stderr, "call recvfrom failed: %s.\n", strerror(errno));
            continue;
        }

        fprintf(stderr, "recv msg: %s\n", msg);

        const ssize_t n_send = sendto(socket_svr, msg, n_recv, 0, (struct sockaddr*)& addr_from, sizeof(addr_from));
        if (-1 == n_send){
            fprintf(stderr, "call sendto failed: %s.\n", strerror(errno));
            continue;
        }

        fprintf(stderr, "sendto msg succeed: %s\n", msg);
    }

exit:
    close(socket_svr);
    return EXIT_SUCCESS;
}
