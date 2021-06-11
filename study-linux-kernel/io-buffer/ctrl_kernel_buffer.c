/**
 * @file ctrl_kernel_buffer.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 验证内核缓冲同步
 * @version 0.1
 * @date 2021-06-11
 * 以下测试暂未得到预想结果， 每次输出立即被同步了(数据与元数据)
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
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
    const char * filename = "./test.txt";
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
    fsync(fd);
    close(fd);
    unlink(filename);
    return 0;
}
