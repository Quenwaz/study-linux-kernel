#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "invalid parameter.\nUsage: %s [PORT]",argv[0]);
        return EXIT_FAILURE;
    }

    int sock_svr = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_svr == -1){
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr_v4;
    addr_v4.sin_family = AF_INET;
    addr_v4.sin_port = htons(atoi(argv[1]));
    addr_v4.sin_addr.s_addr = INADDR_ANY;
    if (-1 == bind(sock_svr, (struct sockaddr*)&addr_v4, sizeof(struct sockaddr_in))){
        goto exit;
    }

    for(;;)
    {
        char message[1024] = {0};
        struct sockaddr_in remote_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        ssize_t recv_num = recvfrom(sock_svr, message, sizeof(message), 0, (struct sockaddr*)&remote_addr, &len);
        if (recv_num == -1){
            break;
        }

        if (-1 == sendto(sock_svr, message, recv_num, 0, (struct sockaddr*)&remote_addr, len)){
            break;
        }
    }

exit:
    close(sock_svr);
    return EXIT_SUCCESS;
}
