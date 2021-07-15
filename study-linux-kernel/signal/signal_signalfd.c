#define _GNU_SOURCE
#include <signal.h>
#include <sys/signalfd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>



int main(int argc, char const *argv[])
{
    sigset_t setblock;
    sigfillset(&setblock);

    // 阻塞除SIGKILL和SIGSTOP的所有标准信号
    if(sigprocmask(SIG_SETMASK, &setblock, NULL) == -1){
        return EXIT_FAILURE;
    }

    int fd = signalfd(-1, &setblock, 0);
    if (fd == -1){
        fprintf(stderr, "occur error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    for(;;)
    {
        struct signalfd_siginfo siginfo;
        if (-1 == read(fd, &siginfo, sizeof(struct signalfd_siginfo))){
            fprintf(stderr, "occur error: %s\n", strerror(errno));
            break;
        }

        fprintf(stderr, "Received signal %s(%d)\n", strsignal(siginfo.ssi_signo), siginfo.ssi_signo);
    }

    return EXIT_SUCCESS;
}


