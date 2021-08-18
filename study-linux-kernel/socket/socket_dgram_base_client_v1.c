/**
 * @file socket_dgram_base_client_v1.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 数据报socket客户端基本过程
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


/*
版本2主要处理connect，将远端socket与绑定， 往后的读写操作则无需再指定远端socket地址
*/
#define VER_2

#ifdef VER_2
#define __send(sockfd, buf, len, flag, addr, addrlen) send(sockfd, buf, len, flag)
#define __recv(sockfd, buf, len, flag, addr, addrlen) recv(sockfd, buf, len, flag)
#else
#define __send(sockfd, buf, len, flag, addr, addrlen) sendto(sockfd, buf, len, flag, addr, addrlen)
#define __recv(sockfd, buf, len, flag, addr, addrlen) recvfrom(sockfd, buf, len, flag)
#endif 

int main(int argc, char const *argv[])
{
    if (argc != 3){
        fprintf(stderr, "Invalid input parameter.\nUsage:%s [IP] [PORT]", argv[0]);
        return EXIT_FAILURE;
    }

    int socket_clt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == socket_clt){
        fprintf(stderr, "call socket failed: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
#ifdef VER_2
    if (-1 == connect(socket_clt, (struct sockaddr*)& addr, sizeof addr)){
        fprintf(stderr, "call connect failed: %s.\n", strerror(errno));
        goto exit;
    }
#endif

    for(;;)
    {

        char msg[1024] = {0};
        fprintf(stderr,"please input any character: ");
        const ssize_t num_read = read(STDIN_FILENO, msg, sizeof msg);
        if (num_read <= 0){
            fprintf(stderr, "call read failed: %s.\n", strerror(errno));
            continue;
        }

        const ssize_t n_send = __send(socket_clt, msg, num_read, 0, (struct sockaddr*)&addr, sizeof addr);
        if(-1 == n_send){
            fprintf(stderr, "call __send failed: %s.\n", strerror(errno));
            continue;
        }

        struct sockaddr_in addr_from;
        socklen_t len = sizeof (addr_from);
        memset(msg, 0, sizeof msg);
        if (-1 == __recv(socket_clt, msg, sizeof msg, 0, (struct sockaddr*)&addr_from, &len)){
            fprintf(stderr, "call __recv failed: %s.\n", strerror(errno));
            continue;
        }
#ifdef VER_2
        memcpy(&addr_from, &addr, sizeof addr);
#endif
        fprintf(stderr, "recv msg %s from %s:%d\n", msg, inet_ntoa(addr_from.sin_addr),
            ntohs(addr_from.sin_port)
        );
    }

exit:
    close(socket_clt);
    return EXIT_FAILURE;
}
