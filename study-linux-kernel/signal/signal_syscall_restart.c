#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static void sigalarm_handler(int sig)
{

}

int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigalarm_handler;
    if(sigaction(SIGALRM, &sa, NULL) == -1){
        return EXIT_FAILURE;
    }
    
    struct itimerval tmval;
    memset(&tmval, 0, sizeof (struct itimerval));
    tmval.it_interval.tv_sec = 1;
    setitimer(ITIMER_REAL, &tmval, NULL);

    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flag | O_NONBLOCK);
    for(;;)
    {
        fd_set fdread;
        FD_ZERO(&fdread);
        FD_SET(STDIN_FILENO, &fdread);
        if (select(STDIN_FILENO +1, &fdread, NULL, NULL, NULL) == -1){
            continue;
        }

        FD_ISSET(STDIN_FILENO);
    }

    return 0;
}
