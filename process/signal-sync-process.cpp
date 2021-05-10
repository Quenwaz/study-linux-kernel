#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static void handler(int sig)
{
    fprintf(stderr, "%ld got signal\n", getpid());

}

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);
    sigset_t blockMask, origMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1){
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    // sa.sa_sigaction = NULL;// 设置将出现错误: User defined signal 1
    if (sigaction(SIGUSR1, &sa, NULL) == -1){
        return EXIT_FAILURE;
    }

    pid_t pidChild;
    switch (pidChild = fork())
    {
    case -1:
        fprintf(stderr, "fork failed!\n");
        break;
    case 0:
        fprintf(stderr, "child[%ld] start\n", getpid());
        sleep(2);
        fprintf(stderr, "child[%ld] send signal\n", getpid());
        if (kill(getppid(), SIGUSR1) == -1){
            return EXIT_FAILURE;
        }
        fprintf(stderr, "child[%ld] over\n", getpid());

        return EXIT_SUCCESS;
    default:
        {
            fprintf(stderr, "father[%ld] start\n", getpid());

            sigset_t emptyMask;
            sigemptyset(&emptyMask);
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR){
                return EXIT_FAILURE;
            }

            fprintf(stderr, "father[%ld] got signal\n", getpid());
            if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1){
                return EXIT_FAILURE;
            }
            break;
        }
    }       
    return EXIT_SUCCESS;
}
