#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if (argc < 4){
        fprintf(stderr, "invalid input parameter!\nUsage: %s [host] [port] [message]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sock_clt = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_clt == -1){
        return EXIT_FAILURE;
    }


    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(atoi(argv[2]));
    svr_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (connect(sock_clt, (struct sockaddr*)& svr_addr, sizeof(struct sockaddr_in)) == -1){
        goto exit;
    }

    for (size_t i = 3; i < argc; ++i)
    {
        if (-1 == sendto(sock_clt, argv[i], strlen(argv[i]), 0, NULL, 0)){
            fprintf(stderr, "error: message not sendto successfully! %s\n", strerror(errno));
            break;
        }

        char message[1024] ={0};
        if (-1 == recvfrom(sock_clt, message, sizeof(message), 0, NULL, NULL)){
            break;
        }

        fprintf(stderr, "%s ", message);
    }

    fprintf(stderr, "\n");
exit:
    close(sock_clt);
    return 0;
}
