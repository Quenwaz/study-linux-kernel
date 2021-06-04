#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage: race-condition numprocess");
        return EXIT_FAILURE;
    }

    size_t num_process = atoi(argv[1]);
    pid_t pidChild = -1;
    for (size_t i = 0;i < num_process; ++i){
        switch (pidChild = fork())
        {
        case -1:
            fprintf(stderr, "fork failed, error: %s", strerror(errno));
            return EXIT_FAILURE;
        case 0:
            fprintf(stderr, "%d child\n", i);
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "%d father\n", i);
            break;
        } 

    }
    return EXIT_SUCCESS;
}
