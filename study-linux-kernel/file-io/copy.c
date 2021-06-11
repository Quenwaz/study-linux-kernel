/**
 * @file copy.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 模拟编写shell命令copy
 * @version 0.1
 * @date 2021-06-11
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> // for read and write

int main(int argc, char const *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage: %s [src] [dst]\n", argv[0]);
        return 1;
    }

    int fdsrc= open(argv[1], O_RDONLY);
    if (fdsrc == -1){
        fprintf(stderr, "open file [%s] failed.\n", argv[1]);
        return 1;
    }

    int fddst = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IWOTH);
    if(fddst == -1){
        fprintf(stderr, "create file [%s] failed.\n", argv[2]);
        return 1;
    }

    char buffer[1024] = {0};
    ssize_t numread = 0;
    while ((numread = read(fdsrc, buffer, sizeof buffer)) > 0)
    {
        if (write(fddst, buffer, numread) < numread){
            fprintf(stderr, "The specified buffer length not written.\n");
            break;
        }
    }

    if (numread == -1){
        fprintf(stderr, "The specified buffer length not written.\n");
    }

    close(fdsrc);
    close(fddst);
    return 0;
}
