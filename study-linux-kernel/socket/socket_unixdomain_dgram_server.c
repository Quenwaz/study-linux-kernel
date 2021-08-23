/**
 * @file socket_unixdomain_dgram_client.c
 * @author Quenwaz (404937333@qq.com)
 * @brief UNIX domain DGRAM server测试程序
 *  - 当recvfrom 的缓冲区大小小于即将要接收的数据大小， 数据将被截断为缓冲区大小。 造成数据丢失
 *  - 当使用USE_ABSTRACT_NAME 的抽象socket路径名时， 避免使用空字符串(\0\0\0\0)。 否则将再次绑定时出现address in use错误
 * @version 0.1
 * @date 2021-08-23
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define USE_ABSTRACT_NAME

int main(int argc, char const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Invalid input parameter.\nUsage: %s [UNIX domain path].\n", argv[0]);
        return EXIT_FAILURE;
    }

    char socket_path_name[256] ={0};
#ifndef USE_ABSTRACT_NAME
    strncpy(socket_path_name, argv[1], sizeof socket_path_name);
    // make sure the file does not exist;
    unlink(socket_path_name);
#else
    // make sure the first byte is \0
    strncpy(&socket_path_name[1], argv[1], sizeof socket_path_name);
#endif // USE_ABSTRACT_NAME

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == sockfd){
        return EXIT_FAILURE;
    }

    struct sockaddr_un un;
    memset(&un, 0, sizeof (un));
    un.sun_family = AF_UNIX;
#ifndef USE_ABSTRACT_NAME
    strncpy(un.sun_path, socket_path_name, sizeof(un.sun_path) - 1);
#else
    strncpy(&un.sun_path[1], &socket_path_name[1], sizeof(un.sun_path) - 2);
    fprintf(stderr, "UNIX domain use abstract name.\n");
#endif // USE_ABSTRACT_NAME
    if (-1 == bind(sockfd, (struct sockaddr*)&un, sizeof (un)))
    {
        goto exit;
    }

    fprintf(stderr, "bind to %s...\n", argv[1]);
    for (;;)
    {
        char messgae[5] = {0};
        struct sockaddr_un un_client;
        memset(&un_client, 0, sizeof(un_client));
        socklen_t len = sizeof(un_client);
        ssize_t  num_read = recvfrom(sockfd, messgae, sizeof(messgae) - 1, 0, (struct sockaddr*)&un_client, &len);
        if (num_read <= 0){
            break;
        }

        fprintf(stderr, "recv message from (%s): %s\n", un_client.sun_path, messgae);

        if (num_read != sendto(sockfd, messgae, num_read, 0, (struct sockaddr*)&un_client, sizeof(un_client))){
            fprintf(stderr, "sendto failed: %s", strerror(errno));
        }
    }

exit:
    fprintf(stderr, "exit: %s\n", strerror(errno));
    close(sockfd);
    return EXIT_SUCCESS;
}
