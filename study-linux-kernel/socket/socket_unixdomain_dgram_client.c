/**
 * @file socket_unixdomain_dgram_client.c
 * @author Quenwaz (404937333@qq.com)
 * @brief UNIX domain DGRAM client测试程序
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
    if (argc < 3){
        fprintf(stderr, "Invalid input parameter.\nUsage: %s [UNIX domain pair end path] [UNIX domain path].\n", argv[0]);
        return EXIT_FAILURE;
    }

    char socket_path_name_s[256] ={0};
    char socket_path_name_c[256] ={0};
#ifndef USE_ABSTRACT_NAME
    strncpy(socket_path_name_s, argv[1], sizeof socket_path_name_s);
    strncpy(socket_path_name_c, argv[2], sizeof socket_path_name_c);
    // make sure the file does not exist;
    unlink(socket_path_name_c);
#else
    // make sure the first byte is \0
    strncpy(&socket_path_name_s[1], argv[1], sizeof socket_path_name_s);
    strncpy(&socket_path_name_c[1], argv[2], sizeof socket_path_name_c);
#endif // USE_ABSTRACT_NAME

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == sockfd){
        return EXIT_FAILURE;
    }

    struct sockaddr_un un;
    memset(&un, 0, sizeof (un));
    un.sun_family = AF_UNIX;
#ifndef USE_ABSTRACT_NAME
    strncpy(un.sun_path, socket_path_name_c, sizeof(un.sun_path) - 1);
#else
    strncpy(&un.sun_path[1], &socket_path_name_c[1], sizeof(un.sun_path) - 2);
    fprintf(stderr, "UNIX domain use abstract name.\n");
#endif 
    if (-1 == bind(sockfd, (struct sockaddr*)&un, sizeof (un)))
    {
        goto exit;
    }

    fprintf(stderr, "bind to %s...\n", argv[2]);
    for (;;)
    {
        fprintf(stderr, "Please input msg:");
        char messgae[1024] = {0};
        int num_read = read(STDIN_FILENO, messgae, sizeof (messgae));
        if (num_read <= 0){
            continue;
        }

        struct sockaddr_un un_server;
        memset(&un_server, 0, sizeof(un_server));
        un_server.sun_family = AF_UNIX;
#ifndef USE_ABSTRACT_NAME
        strncpy(un_server.sun_path, socket_path_name_s, sizeof(un_server.sun_path) - 1);
#else
        strncpy(&un_server.sun_path[1], &socket_path_name_s[1], sizeof(un_server.sun_path) - 2);
#endif
        if (num_read != sendto(sockfd, messgae, num_read, 0, (struct sockaddr*)&un_server, sizeof(un_server))){
            break;
        }

        memset(messgae, 0, sizeof (messgae));
        memset(&un_server, 0, sizeof(un_server));
        socklen_t len = sizeof(un_server);
        if (recvfrom(sockfd, messgae, sizeof messgae, 0, (struct sockaddr*)&un_server, &len) <= 0){
            break;
        }

        fprintf(stderr, "recv message from (%s): %s\n", un_server.sun_path, messgae);
    }

exit:
    fprintf(stderr, "exit: %s\n", strerror(errno));
    close(sockfd);
    return EXIT_SUCCESS;
}
