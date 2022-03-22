/**
 * @file ldpreloadtest.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 测试LD_PRELOAD
 * @version 0.1
 * @date 2022-03-22
 *  执行该程序需设置LD_PRELOAD=libldpreloaddll.so
 *  设置后其中的time接口及open接口将被劫持
 *  同时将catch  SIGSEGV信号打印对应back trace信息
 * @copyright Copyright (c) 2022 Quenwaz
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int i = 10;
    while (i--)
        printf("%d\n", rand() % 100);

    if (argc > 1){
        int fd = open(argv[1], O_RDONLY);
        if (fd != -1){
            char buff[512] = {0};
            int num_read = 0;
            while (num_read=read(fd, buff, sizeof buff), num_read!=0)
            {
                write(STDERR_FILENO, buff, num_read);
            }
            close(fd);
        }
    }

    time_t timestamp;
    time(&timestamp);
    fprintf(stderr, "\ncurrent timestamp:%ld\n", timestamp);

    char test_array[] = {'I', 'L', 'O', 'V', 'E', 'Y', 'O', 'U'};
    test_array[8192] = 'Q';
    return 0;
}
