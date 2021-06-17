#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

static volatile sig_atomic_t gotsigio = 0;

static void sigHandler(int sig)
{
    gotsigio = 1;
}

int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigHandler;
    if(sigaction(SIGIO, &sa, NULL) == -1){
        return 1;
    }

    if(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1){
        return 1;
    }

    int flags = fcntl(STDIN_FILENO, F_GETFL);
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1){
        return 1;
    }

    while (gotsigio == 0)
    {
        pause();
        if (gotsigio){
            char msg[1024]={0};
            ssize_t numread = read(STDIN_FILENO, msg, sizeof msg);
            if (numread > 0)
                write(STDERR_FILENO, msg,numread);
            gotsigio = 0;  
        }

       
    }
    
    return 0;
}
