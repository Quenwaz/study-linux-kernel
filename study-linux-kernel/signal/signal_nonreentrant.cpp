/**
 * @file signal_nonreentrant.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号处理不可重入实验
 * @version 0.1
 * @date 2021-06-02
 * 此示例演示了当调用不可重入方法crypt()时发生的有趣现象。
 * 同一个字符串加密后在不触发SIGINT时，其值相等。 触发后竟然不相等了。
 * 说明crypt()中使用了静态变量， 再次调用后改变了静态变量的值。 而主进程中返回了
 * 信号处理程序中加密的值。 导致不相等。
 * 解决这个问题的方法是添加同步锁。SYNC_LOCK
 * @copyright Copyright (c) 2021
 * 
 */
#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <pthread.h>


static const char* g_str = NULL;
static int g_handled = 0;

/// 同步互斥锁
// #define SYNC_LOCK
#ifdef SYNC_LOCK
static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif 

void sig_handler(int signo)
{
#ifdef SYNC_LOCK
    pthread_mutex_lock(&g_mtx);
#endif 
    crypt(g_str, "xx");

#ifdef SYNC_LOCK
    pthread_mutex_unlock(&g_mtx);
#endif 
    ++g_handled;
}


int main(int argc, char const *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage: %s [str1] [str2]\n", argv[0]);
        return 1;
    }

    g_str = argv[2];
    char* cr1 = strdup(crypt(argv[1], "xx"));
    if (cr1 == NULL){
        return 1;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sig_handler;
    if(sigaction(SIGINT, &sa, NULL) == -1){
        return 1;
    }

    fprintf(stderr, "Enter Ctrl+C repeatedly to view the output, and enter Ctrl+\ to exit.\n");
    for (int call = 1, mismatch=0;; ++call)
    {   
#ifdef SYNC_LOCK
        pthread_mutex_lock(&g_mtx);
#endif
        const char* pszcr2 = crypt(argv[1], "xx");
#ifdef SYNC_LOCK
        pthread_mutex_unlock(&g_mtx);
#endif
        if (strcmp(pszcr2, cr1) != 0){
            ++mismatch;
            fprintf(stderr, "Mismatch on call %d (mismatch=%d handled=%d)\n",
                call, mismatch, g_handled);
        }
       
    }
    free(cr1);
#ifdef SYNC_LOCK
    pthread_mutex_destroy(&g_mtx);
#endif
    return 0;
}
