#define _GNU_SOURCE  //for sigaction
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
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
    if (argc < 1){
        fprintf(stderr, "Invalid input parameter.\n Usage: %s [Unix domain path]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (proces_sigpipe_signal(sighandler) == -1){
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == sockfd){
        return EXIT_FAILURE;
    }

    struct sockaddr_un un;
    memset(&un, 0, sizeof (struct sockaddr_un));
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, argv[1], sizeof(un.sun_path) - 1);
    if(-1 == connect(sockfd, (struct sockaddr*)&un, sizeof(un))){
        goto exit;
    }

    for(;;)
    {
        fprintf(stderr, "please input msg: ");
        char msg[1024] ={0};
        int num_read = read(STDIN_FILENO, msg, sizeof(msg));
        if (num_read <= 0){
            continue;
        }

        if (write(sockfd, msg, num_read) <= 0){
            fprintf(stderr, "occur error : %s\n", strerror(errno));
            break;
        }

        memset(msg, 0, sizeof msg);
        if (0 >= read(sockfd, msg, sizeof (msg))){
            fprintf(stderr, "occur error : %s\n", strerror(errno));
            break;
        }
        
        fprintf(stderr, "recv msg: %s\n", msg);
    }

exit:
    fprintf(stderr, "exit: %s\n", strerror(errno));
    close(sockfd);
    return EXIT_SUCCESS;
}
