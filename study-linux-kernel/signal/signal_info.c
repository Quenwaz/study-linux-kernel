/**
 * @file signal_info.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 打印信号信息
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#define _GNU_SOURCE  // for NSIG and strsignal
#include <stdio.h>
#include <signal.h>
#include <string.h>

int main(int argc, char const *argv[])
{   
    // Linux定义标准信号和实时信号在 [1, NSIG)之间
    for (int i = 1;i < NSIG; ++i){
        fprintf(stderr, "Signal(%d)： %s\n", i, strsignal(i));
    }
    return 0;
}

