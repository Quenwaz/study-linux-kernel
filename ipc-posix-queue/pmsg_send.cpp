#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void Usage(const char* pszAppName)
{
    fprintf(stderr, "Usage: %s [-n] name msg [prio]\n", pszAppName);
    fprintf(stderr, "\t-n\t Use O_NONBLOCK flag\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt = 0;
    int flags= O_WRONLY;
    while ((opt=getopt(argc, argv, "n"))!= -1)
    {
        switch(opt)
        {
           case 'n':  flags |= O_NONBLOCK;
            break;
           default: Usage(argv[0]); break;
        }
    }

    if (optind + 1 > argc){
        Usage(argv[0]);
    }

    mqd_t mqd=mq_open(argv[optind], flags);
    if (mqd == (mqd_t) -1){
        return EXIT_FAILURE;
    }

    unsigned int prio = (argc > optind + 2) ? atoi(argv[optind + 2]): 0;
    if (mq_send(mqd, argv[optind + 1], strlen(argv[optind + 1]), prio) == -1){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
