#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void Usage(const char* pszAppName)
{
    fprintf(stderr, "Usage: %s [-cx] [-m maxmsg] [-s msgsize] mq-name [octal-perms]\n", pszAppName);
    fprintf(stderr, "\t-c\tCreate queue (O_CREAT)\n");
    fprintf(stderr, "\t-m maxmsg\tSet maximum # of message\n");
    fprintf(stderr, "\t-s msgsize\t Set maximum message size\n");
    fprintf(stderr, "\t-x\tCreate exclusively (O_EXCL)\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt =0;
    int flags = O_RDWR;
    struct mq_attr attr, *attrp;
    while ((opt = getopt(argc, argv, "cm:s:x")) != -1)
    {
        switch (opt)
        {
        case 'c':
            flags |= O_CREAT;
            break;
        case 'm':
            attr.mq_maxmsg = atoi(optarg);
            attrp = &attr;
            break;
        case 's':
            attr.mq_msgsize = atoi(optarg);
            attrp = &attr;
            break;
       case 'x':
            flags |= O_EXCL;
            break;
        default:
            Usage(argv[0]);
            break;
        }
    }

    if (optind >= argc){
        Usage(argv[0]);
    }

    char *endptr;
    mode_t perms=((argc <= optind + 1) ? (S_IRUSR | S_IWUSR) : strtol(argv[optind + 1], &endptr, 8));
    mqd_t mqd = mq_open(argv[optind],flags, perms, attrp);
    if (mqd == -1){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
