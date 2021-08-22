#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


int main(int argc, char const *argv[])
{
    if (argc < 1){
        fprintf(stderr, "Invalid input parameter.\n Usage: %s [Unix domain path]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1){
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path) - 1);
    if (-1 == bind(sockfd, (struct sockaddr*)&addr, sizeof (struct sockaddr_un))){
        goto exit;
    }

    if (-1 == listen(sockfd, 4)){
        goto exit;
    }

    fprintf(stderr, "listening to %s", argv[1]);
    for (;;)
    {
        int sockfd_client = accept(sockfd, NULL, NULL);
        if (sockfd_client ==  -1){
            continue;
        }

        fprintf(stderr, "accept a connection:%d \n", sockfd_client);

        char msg[1024] = {0};
        ssize_t num_read = read(sockfd_client, msg, sizeof msg);
        if (num_read < 0){
            fprintf(stderr, "occur error: %s\n", strerror(errno));
            break;
        }else if (num_read == 0){
            fprintf(stderr, "remote closed.\n");
            break;
        }

        fprintf(stderr, "recv msg: %s\n", msg);
        write(sockfd_client, msg, num_read);
        close(sockfd_client);
    }

exit:
    close(sockfd);
    return EXIT_SUCCESS;
}
