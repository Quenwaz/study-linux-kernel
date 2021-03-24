#include <signal.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char  *argv[])
{
    
    setbuf(stdout, NULL);    
    pid_t pidChild;
    switch (pidChild = fork())
    {
    case -1:
        return EXIT_FAILURE;
    case 0:
        printf("Child pid=%ld exiting\n", getpid());
        _exit(EXIT_SUCCESS);
    default:
        break;
    }

    sleep(3);
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "ps | grep %s", basename(argv[0]));
    system(cmd);

    if (kill(pidChild, SIGKILL) == -1){
        printf("kill child %ld failed.\n", pidChild);
        return EXIT_FAILURE;
    }

    sleep(3);
    printf("After sending SIGKILL to zombie(pid=%ld)\n", pidChild);
    system(cmd);
    return EXIT_SUCCESS;
}
