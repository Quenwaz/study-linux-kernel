/**
 * @file signal_pause_bug.c
 * @author Quenwaz (404937333@qq.com)
 * @brief 模拟出等待信号到达调用存在的bug
 * @version 0.1
 * @date 2021-07-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h> // for pause



static void handler(int sig)
{
    fprintf(stderr, "Received signal %d\n", sig);
}

int main(int argc, char const *argv[])
{
    sigset_t set_block, set_previous;
    sigemptyset(&set_block);
    sigaddset(&set_block, SIGINT);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1){
        return 1;
    }

    if (sigprocmask(SIG_BLOCK, &set_block, &set_previous) == -1){
        return 1;
    }

    // 阻塞信号， 避免打断此处的代码执行

    if (sigprocmask(SIG_SETMASK, &set_previous, NULL) == -1){
        return 1;
    }

    // 此处模拟， 解除阻塞后在此间隙收到了信号。 
    // 将会导致代码逻辑不会按照预期进行。 
    // 预期是调用pause()后信号才到达。
    kill(getpid(), SIGINT);

    pause();
    return 0;
}
