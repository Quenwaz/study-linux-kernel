/**
 * @file signal_coredump.c
 * @author Quenwaz (404937333@qq.com)
 * @brief core dump 处理
 * @version 0.1
 * @date 2022-03-21
 * 
 * 
 * https://blog.csdn.net/jxgz_leo/article/details/53458366#
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include <execinfo.h> 	/* for backtrace() */
 
#define BACKTRACE_SIZE   16
 
void dump_backtrace(void)
{
	int j, nptrs;
	void *buffer[BACKTRACE_SIZE];
	char **strings;
	
	nptrs = backtrace(buffer, BACKTRACE_SIZE);
	
	printf("backtrace() returned %d addresses\n", nptrs);
 
	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
 
	for (j = 0; j < nptrs; j++)
		printf("  [%02d] %s\n", j, strings[j]);
 
	free(strings);
}
 
void signal_handler(int signo)
{
	
#if 0	
	char buff[64] = {0x00};
	sprintf(buff,"cat /proc/%d/maps", getpid());
	system((const char*) buff);
#endif	
 
	printf("\n============catch signal %d============\n", signo);
	printf("<backtrace dump start>\n");
	dump_backtrace();
	printf("<backtrace dump end>\n");
 
	signal(signo, SIG_DFL); /* 恢复信号默认处理 */
	raise(signo);           /* 重新发送信号 */
}


int main(int argc, char const *argv[])
{
    // 关闭缓冲
    setbuf(stdout, NULL);
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1){
        printf("Failed to set SIGSEGV signal handler.\n");
        return 1;
    }
    char test_array[] = {'I', 'L', 'O', 'V', 'E', 'Y', 'O', 'U'};
    test_array[8192] = 'Q';

    return 0;
}
