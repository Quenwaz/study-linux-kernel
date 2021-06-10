#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define random(a,b) ((rand()%(b-a))+a)

char* generate_string(ssize_t length)
{
    srand((int)time(NULL));
    char seed[]="0123456789abcdefghijklmnopqrstuvwxyz";
    const ssize_t nseed = sizeof(seed)/sizeof(seed[0]);
    char *string = (char*)malloc(sizeof(char) * (length + 1));
    for(ssize_t i = 0;i < length; ++i)
    {
        string[i] = seed[random(0, nseed)];
    }
    string[length] = '\0';
    return string;
}


int main(int argc, char const *argv[])
{
    const char * filename = "./fucku.txt";
    int fd = open(filename, O_RDWR| O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP|S_IWGRP);
    if (fd == -1){
        fprintf(stderr, "create file error: %s\n", strerror(errno));
        return 1;
    }

    const ssize_t len = 4096;
    char * string = generate_string(len);
    ssize_t actual = write(fd, string, len);
    free(string);
    if (actual == -1){
         fprintf(stderr, "write file error: %s\n", strerror(errno));
    }
    close(fd);
    unlink(filename);
    return 0;
}
