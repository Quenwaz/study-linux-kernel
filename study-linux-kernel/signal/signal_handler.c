/**
 * @file signal_handler.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 信号处理之signal
 * @version 0.1
 * @date 2021-04-22
 * 
 * 此程序演示了如何使用signal设置信号处理
 * 
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

void sign_interrupt_handler(int signno)
{
    const char* sign_name = signno == SIGINT? "SIGINT": "SIGQUIT";
    printf("Trigger the %s(%d) signal.\n",sign_name, signno);
}

int main(int argc, char const *argv[])
{
    boolean bSetSigQuit = argc > 1;

    // 关闭缓冲
    setbuf(stdout, NULL);
    void (*oldHandler) (int) = signal(SIGINT, sign_interrupt_handler);
    if (oldHandler == SIG_ERR){
        printf("Failed to set SIGINT signal handler.\n");
        return 1;
    }

    if (bSetSigQuit && signal(SIGQUIT, sign_interrupt_handler) == SIG_ERR){
        printf("Failed to set SIGQUIT signal handler.\n");
        return 1;
    }

    printf("Please enter Ctrl+C to trigger the SIGINT handler.\n");
    bSetSigQuit && printf("Please enter Ctrl+\\ to trigger the SIGQUIT handler.\n");
    for (;;) pause();
    return 0;
}
