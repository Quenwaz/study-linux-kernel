/**
 * @file signal_handler_ex.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号处理之sigaction
 * @version 0.1
 * @date 2021-04-22
 * 
 * 此程序演示了如何使用sigaction设置信号处理程序， 
 * 设置了信号SIGINT和SIGQUIT， 其处理函数相同。 处理中将会睡眠10秒钟， 
 * 在此期间为互斥状态， 当SIGINT睡眠期间， 不能被SIGQUIT中断，若此期间触发了SIGQUIT
 * 会被进入等待状态。直到SIGINT处理函数执行完， SIGQUIT被移除阻塞列表， 将会执行一次SIGQUIT。
 * 
 * SIGQUIT同理
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <signal.h>
#include <bits/sigaction.h> // for struct sigaction
#include <unistd.h>
#include "common.h"

void sign_interrupt_handler(int signno)
{
    const char* sign_name = signno == SIGINT? "SIGINT": "SIGQUIT";
    printf("Trigger the %s(%d) signal.\n",sign_name, signno);
    sleep(10);
}

int main(int argc, char const *argv[])
{
    boolean bSetSigQuit = argc > 1;

    // 关闭缓冲
    setbuf(stdout, NULL);

    struct sigaction sa;
    sa.sa_handler = sign_interrupt_handler;
    sigemptyset(&sa.sa_mask);
    // 执行SIGINT时， 不允许被SIGQUIT中断
    sigaddset(&sa.sa_mask, SIGQUIT);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1){
        printf("Failed to set SIGINT signal handler.\n");
        return 1;
    }

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    if (bSetSigQuit && sigaction(SIGQUIT, &sa, NULL) == -1){
        printf("Failed to set SIGQUIT signal handler.\n");
        return 1;
    }

    printf("Please enter Ctrl+C to trigger the SIGINT handler.\n");
    bSetSigQuit && printf("Please enter Ctrl+\\ to trigger the SIGQUIT handler.\n");
    for (;;) pause();
    return 0;
}
