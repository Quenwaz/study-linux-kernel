/**
 * @file stdio_buffer.c
 * @author Quenwaz (404937333@qq.com)
 * @brief stdio缓冲区设置
 * @version 0.1
 * @date 2021-06-09 Quenwaz
 * - setvbuf
 * - setbuf 构建在setvbuf之上，相当于 setvbuf(fp, buf, (buf == NULL)? _IONBF: _IOFBF, BUFSIZE)
 * - setbuffer 类似setvbuf,  相当于 setvbuf(fp, buf, (buf == NULL)? _IONBF: _IOFBF, size)
 * 
 * 以下示例当设置OPT为SETVBUF或SETBUFFER时只能打印一行，
 * 5秒后打印剩余的。 如果设置为SETBUF， 则5秒后全部打印
 * @copyright Copyright (c) 2021
 * 
 */
#define _BSD_SOURCE // for setbuffer, 非标准api
#include <stdio.h>

#define BUF_SIZE 32

#define SETVBUF 0
#define SETBUF 1
#define SETBUFFER 2

// choose API
#define OPT SETBUFFER

#if OPT == SETBUF
#define BUF_SIZE BUFSIZ
#endif

int main(int argc, char const *argv[])
{
    char buffer[BUF_SIZE] = {0};
    int ret = 0;
    switch (OPT)
    {
    case 0:
        ret = setvbuf(stdout, buffer,_IOFBF, sizeof buffer);
        break;
    case 1:
        setbuf(stdout, buffer);
        break;
    case 2:
        setbuffer(stdout, buffer, sizeof buffer);
        break;
    default:
        break;
    }

    if(0!= ret){
        fprintf(stderr, "setvbuf failed.\n");
        return 1;
    }

    fprintf(stdout, "1234567890\n");
    fprintf(stdout, "abcdefghijklmn\n");
    fprintf(stdout, "opqrstuvwxyz\n");
    sleep(5);
    fflush(stdout);
    return 0;
}
