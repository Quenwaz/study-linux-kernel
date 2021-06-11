/**
 * @file close_on_exec.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 测试O_CLOEXEC标志
 * @version 0.1
 * @date 2021-06-11
 * 当前示例展示当使用O_CLOEXEC时对应的sleep程序将不会继承创建的文件句柄
 * 否则不加标签O_CLOEXEC将存在文件句柄， 查看文件句柄使用命令ls -l /proc/PID/fd
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE // for O_CLOEXEC
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // for fork
#include <stdio.h>
#include <stdlib.h>  // for exit

#include <errno.h>  // errno
#include <string.h> // strerror
#define err_sys(fmt, arg...) \
do { \
    printf(fmt, ##arg);\
    printf("\nerrno:%d %s\n", errno, strerror(errno));\
    exit(0);\
} while (0)


// #define _O_CLOEXEC

int main(int argc, char const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage: %s [filename].\n", argv[0]);
        return 1;
    }

    int oflags = O_RDWR | O_CREAT | O_TRUNC;
#ifdef _O_CLOEXEC
    oflags |= O_CLOEXEC;
#endif
    int fd = open(argv[1], oflags);
    if (fd == -1){
        fprintf(stderr, "open file [%s] failed.\n",argv[1]);
        return 1;
    }

    pid_t pid = -1;
    switch ((pid=fork()))
    {
    case -1:
        err_sys("fork failed");
        break;
    case 0:
        if (execl("/bin/sleep", "sleep", "120", (void*)0) < 0)
            err_sys("execl error");
        break;
    default:
        sleep(120);
        break;
    }

    close(fd);
    return 0;
}
