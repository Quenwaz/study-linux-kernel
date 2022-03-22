
/**
 * @file ldpreloaddll.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 生成动态链接库， 用于LD_PRELOAD
 * @version 0.1
 * @date 2022-03-22
 * 
 * 1. 伪装成C中的open, 获取进程打开文件信息
 * 2. 伪装成C中的rand() 时随机数不随机
 * 3. 给异常信号添加信号处理， 便于打印堆栈信息
 * 
 * @copyright Copyright (c) 2022 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#define ADDR_MAX_NUM 64

typedef int(*orig_open_f_type)(const char*pathname,int flags);
int open(const char*pathname,int flags,...)
{
    fprintf(stderr, "\ntry to open the file:%s\n", pathname);
    orig_open_f_type orig_open;
    orig_open =(orig_open_f_type)dlsym(RTLD_NEXT,"open");
    return orig_open(pathname,flags);
}

int rand()
{
    return 520;
}

typedef time_t(*orig_time_type)(time_t *__timer);
time_t time(time_t *__timer)
{
    fprintf(stderr, "\ntry to call time\n");
    orig_time_type orig_time=(orig_time_type)dlsym(RTLD_NEXT,"time");
    time_t ret = orig_time(__timer);
    ret -= 3600;
    if (__timer != NULL) *__timer = ret;
    return ret;
}


void sighandler(int signo)
{
#if 0
    _exit(signo);
#else
    fprintf(stderr, "\n=======================CATCH SIGNAL:%d=======================\n", signo);
    void *buffer[ADDR_MAX_NUM] = {0};
    int i_addr_num = backtrace(buffer, ADDR_MAX_NUM);
    fprintf(stderr, "BACKTRACE: NUMBER OF ADDRESSES IS:%d\n\n", i_addr_num);
    char ** symbols = backtrace_symbols(buffer, i_addr_num);
    if (symbols == NULL) {
        fprintf(stderr, "BACKTRACE: CANNOT GET BACKTRACE SYMBOLS\n");
        return;
    }
    for (int i = 0; i < i_addr_num; ++i) {
        fprintf(stderr, "%03d %s\n", i_addr_num-i, symbols[i]);
    }
    fprintf(stderr, "\n");
    free(symbols);
    symbols = NULL;
    _exit(1); // QUIT PROCESS. IF NOT, MAYBE ENDLESS LOOP.
#endif
}


/**
 * @brief __attribute__((constructor)) 使方法在main之前执行
 * 
 */
static __attribute__((constructor))  void  before()
{
    int sigs[] = {
        SIGILL, SIGFPE, SIGABRT, SIGBUS, SIGSEGV, SIGHUP, SIGINT, SIGQUIT, SIGTERM
    };

    struct sigaction sa;
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    for(int i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if(sigaction(sigs[i], &sa, NULL) == -1) {
            perror("Could not set signal handler");
        }
    }
}
 
 /**
 * @brief __attribute__((destructor)) 使方法在main之后执行
 * 
 */
static __attribute__((destructor)) void after()
{
    fprintf(stderr,"\n-----------debug end-----------\n");
}
