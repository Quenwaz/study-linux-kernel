/**
 * @file terminal-io-epoll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-16
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

/**
 * @brief 等待文件描述符就绪
 * 
 * @param epfd epoll文件描述符实例
 * @param evlist [in/out] 返回就绪态的文件描述符信息, 用户需先分配内存
 * @param maxevents  evlist列表长度
 * @param timeout -1 表示阻塞等待; 0表示非阻塞检查; >0 表示阻塞毫秒数或就绪或信号发生
 * @return int >0 返回就绪的文件描述符数量; =0 超时; -1 表示错误发生
 */
int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);

int main(int argc, char const *argv[])
{
    
    return 0;
}
