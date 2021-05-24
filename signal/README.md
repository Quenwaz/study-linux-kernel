# 信号

## 概述
信号是事件发生时对进程的通知机制。有时也称之为软件中断。信号与硬件中断相似之处在于打断了程序执行的正常流程。大多数情况下， 无法预测信号到达的精确时间。

进程可向其他进程发送信号， 亦可向自身发送信号。然而发往进程的诸多信号， 通常源于内核。 引发内核为进程产生信号的事件通常有:
- 硬件异常。如被0除、引用了无法访问的内存区域。
- 键盘输入中断。 如输入中断字符(Ctrl + C)、暂停字符(Ctrl + Z)
- 发生了软件事件。如定时器到期、文件描述符输出变有效等

针对每个信号都有唯一的整数表示， 从1开始。 以SIGxxxx形式定义在文件<signal.h>中。其值因系统而异。 

信号因某些事件产生。 然后被传递给某一进程， 进程会对之作出响应。 在产生和到达期间，信号处于等待(pending)状态。 通常一旦内核要调度该进程运行， 等待信号会马上送达。

有时需要确保一段代码不为传递来的信号所中断， 需将信号添加到进程的信号掩码中。此时信号将会被阻塞， 进入等待状态， 直到其从信号掩码中移除为止。进程可使用各种系统调用对其信号掩码添加和移除信号。


信号到达后， 因不同信号有如下默认操作之一:
- 忽略信号: 内核将信号丢弃
- 终止进程: 异常终止， 而非exit() 正常退出
- 产生核心转储文件， 同时终止进程: 核心转储文件包含对进程的虚拟内存的镜像， 可将其加载到调试器中以检测进程终止时的状态。
- 停止进程: 暂停进程
- 恢复暂停进程

除了上述默认行为之外， 程序也能改变信号到达时的响应行为， 可有如下响应：
- 采取默认行为
- 忽略信号
- 执行信号处理程序



## 信号类型

| 名称      | 描述                                                         | 默认   |
| --------- | ------------------------------------------------------------ | ------ |
| SIGABRT   | 中止进程,产生核心转储文件。 这实现了abort()预期的目标， 产生核心转储文件用于调试。 | core   |
| SIGALRM   | 实时定时器过期                                               | term   |
| SIGBUS    | 内存访问错误                                                 | core   |
| SIGCHLD   | 当某一子进程终止时将向父进程发送该信号；或某一子进程因信号停止或恢复时， 有可能会向父进程发送该信号。 | ignore |
| SIGCONT   | 若停止则继续                                                 | cont   |
| SIGEMT    | 硬件错误                                                     | term   |
| SIGFPE    | 算术异常                                                     | core   |
| SIGHUP    | 挂起                                                         | term   |
| SIGILL    | 非法指令                                                     | core   |
| SIGINT    | 终端中断(Control+C)                                          | term   |
| SIGIO     | 利用fcntl()系统调用， 即可与特定类型(终端和套接字)的可打开文件描述符发生I/O时产生该信号。 | term   |
| SIGPOLL   | 从System V派生而来， 与Linux中的SIGIO信号同义                |        |
| SIGKILL   | 确保杀死                                                     | term   |
| SIGPIPE   | 当某一进程试图向管道、FIFO或套接字写入信息时， 如果这些设备并未响应的读取进程。 系统产生此信号。 | term   |
| SIGPROF   | 性能分析定时器国企                                           | term   |
| SIGPWR    | 电量行将耗尽                                                 | term   |
| SIGQUIT   | 终端退出， Control+\                                         | core   |
| SIGSEGV   | 无效的内存引用， 通常为段错误                                | core   |
| SIGSTKFLT | 协处理器栈错误                                               | term   |
| SIGSTOP   | 确保停止                                                     | stop   |
| SIGSYS    | 无效的系统调用                                               | core   |
| SIGTERM   | 终止进程， kill和killall命令所发送的默认信号。               | term   |
| SIGTRAP   | 跟踪/断点陷阱                                                | core   |
| SIGTSTP   | 终端停止                                                     | stop   |
| SIGTTIN   | BG从终端中读取                                               | stop   |
| SIGTTOU   | BG向终端写入                                                 | stop   |
| SIGURG    | 系统发送毅哥信号给进程， 表示套接字上存在带外(紧急)数据      | ignore |
| SIGUSR1   | 用户自定义信号1                                              | term   |
| SIGUSR2   | 用户自定义信号2                                              | term   |
| SIGVTALRM | 虚拟定时器过期                                               | term   |
| SIGWINCH  | 终端窗口尺寸变换                                             | ignore |
| SIGXCPU   | 突破对CPU时间限制                                            | core   |
| SIGXFSZ   | 突破对文件大小的限制                                         | core   |

## 信号处理

UNIX提供了signal()和sigaction()来改变信号处置。signal的行为在不同的UNIX实现间存在差异， 因此考虑可移植性， sigaction()是建立信号处理器的首选API。

```c++
#include <signal.h>

// 若成功返回上一个信号处理函数， 否则返回SIG_ERR
void (*signal(int sig, void(*handler)(int)))(int);
```
在调用signal() 时， 可使用如下值来代替函数地址:
- SIG_DEL：将信号重置为默认处理程序
- SIG_IGN：忽略该信号

如下设置终端中断处理程序:
```c++
#include <stdio.h>
#include <signal.h>

void sign_interrupt_handler(int signno)
{
    printf("Trigger the SIGINT(%d) signal.\n", signno);
}

int main(int argc, char const *argv[])
{
    // 关闭缓冲
    setbuf(stdout, NULL);
    void (*oldHandler) (int) = signal(SIGINT, sign_interrupt_handler);
    if (oldHandler == SIG_ERR){
        printf("Failed to set SIGINT signal handler.\n");
        return 1;
    }

    printf("Please enter Ctrl+C to trigger the interrupt handler.\n");
    while (1);
    return 0;
}
```

下图为信号到达及处理过程:

<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="538px" height="251px" viewBox="-0.5 -0.5 538 251" content="&lt;mxfile host=&quot;Electron&quot; modified=&quot;2021-05-24T08:35:46.181Z&quot; agent=&quot;5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) draw.io/13.9.9 Chrome/85.0.4183.121 Electron/10.1.5 Safari/537.36&quot; etag=&quot;cipPx83KeM0zOgldTxf_&quot; version=&quot;13.9.9&quot; type=&quot;device&quot;&gt;&lt;diagram id=&quot;iWch4RwDsjf-bw-bIRMe&quot; name=&quot;Page-1&quot;&gt;7Vptb5s6FP41lroPdwLMi/MR8tJp0qRJ1dW2jyx4xBrBEThLul9/bbAh2E6hV0mqrpOq1j5+AZ7nnOPnQAGcb4/3VbrbfKIZLoDnZEcAF8DzXAeF/I+wPLYW5EhDXpFMTuoND+Q3ViuldU8yXA8mMkoLRnZD45qWJV6zgS2tKnoYTvtBi+FVd2mODcPDOi1M6xeSsY18isDp7R8wyTfqyq4jR7apmiwN9SbN6OHEBJcAzitKWdvaHue4EOApXNp1qzOj3Y1VuGRTFnw9/PTzf51P92T3sShLmO3uV//AdpdfabGXDyxvlj0qBCq6LzMsNnEBTA4bwvDDLl2L0QPnnNs2bFvIYbkdrhg+nr1Pt3t67jaYbjGrHvkUtSCUgEmP8VT/0OPfzdmcYN8hnUrO827vHhbekMg8AyXfRGkZgRgBlIBlAJIYoFXTmAPkiEYciSEdSY4JG8JVs4r+xHNa0IpbSlrymckPUhSaKS1IXvLummOIuT0RCBPupbEc2JIsE5ex8tMz6FyIIqRRFJkUBTaGrkWQ6477cc5h2J19eJk/0u9quvNcULzZEBToWBBwg2tBgMYRwGUWi5QoHKlI65qsh96YpfWmC/R2Nc6M9Njj8QQVJkojrqFsFS5SRn4NL2oDS17hMyX8dvopNd1Xayytp9lQTZTsRBoPLK1yzIx1DRXdU/x/dmZvm53u8B+umEqWETVXZsv1DLrwkbC7d28npfv+S6d0y6H73JCZECVKUzYONSaUbh43x2HcqJNFT16jcaRIdrWNwhvHVWBweufyoOKayQfJCsSuEE9cSyVR00Ag4UP891L8jMfe9JiqcE1+y8NeRNBOPHIDQpCAYCH22jNat+XIhYStzqIlwtBNIywcj7Bbi38/OCOixsS/618NpsiESbgrd8vEUga8di/1VJAqbQ9f+iCwKFtruoh9gHzBCBfiKPwDqfGRRo2l7Jrdkhq18VMZhFUkLfOzQuZU4JKKV2CECpwPuGa65rWIqAuA2mUYlXFcS8ax5WX3erCa8tOAdUz6CL36Vfjm+0D2vp2MLI7SbZvOo+qU/O7bRTCMlOFbY0BeoAz94qY3WP0ZV4SDIGLnRJlOUWCthpngbKNS7eTt2gtoNai/G3l20aOiHWobwduKNW/Cu8FLCfCXIsvXNZleaE4mS98Iubcl6wLVUp8yfAFYlzSc954bPZ04eEeP/JNkEkRomEw8BP8mk0n+qb9T1Au2qf6pZyU4g7f1T0vl57WVXyDEGlc2yxDEjpDVQtwlIIai8uMSj4s4UQKuwOxUbjelIXcjIfd8YfTCQiiG7xVv5aL1RkUi9DSmbRUUtLisfrxcTs6YdeYdbLnnxM/Ex5KWaTSfxJmFafNjTOdDkfAqlY1fMa9GKggCk1f3przaCuM2Un3BBI/X2et/a6Of7L5nxpNvgf161YFZD9/5bTidCQO0lC8s4oUIuVnYNCKAPJDMbOEUioCJPRV7C0ugvnZaoaaufVvZd6Fw4t3+M397ovb/LAGX/wE=&lt;/diagram&gt;&lt;/mxfile&gt;" style="background-color: rgb(255, 255, 255);"><defs/><g><rect x="87" y="30" width="160" height="220" rx="24" ry="24" fill="#ffffff" stroke="#000000" pointer-events="all"/><rect x="107" y="40" width="50" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 48px; height: 1px; padding-top: 50px; margin-left: 108px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: normal; word-wrap: normal; ">程序开始</div></div></div></foreignObject><text x="132" y="54" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">程序开始</text></switch></g><path d="M 217 70 L 217 133.63" fill="none" stroke="#000000" stroke-miterlimit="10" stroke-dasharray="3 3" pointer-events="stroke"/><path d="M 217 138.88 L 213.5 131.88 L 217 133.63 L 220.5 131.88 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><path d="M 217 150 L 217 213.63" fill="none" stroke="#000000" stroke-miterlimit="10" stroke-dasharray="3 3" pointer-events="stroke"/><path d="M 217 218.88 L 213.5 211.88 L 217 213.63 L 220.5 211.88 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><rect x="107" y="210" width="50" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 48px; height: 1px; padding-top: 220px; margin-left: 108px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: normal; word-wrap: normal; ">exit()</div></div></div></foreignObject><text x="132" y="224" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">exit()</text></switch></g><path d="M 7 140 L 80.63 140" fill="none" stroke="#000000" stroke-miterlimit="10" pointer-events="stroke"/><path d="M 85.88 140 L 78.88 143.5 L 80.63 140 L 78.88 136.5 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><rect x="7" y="110" width="80" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 120px; margin-left: 47px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">(1) 信号到达</div></div></div></foreignObject><text x="47" y="124" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">(1) 信号到达</text></switch></g><rect x="377" y="70" width="160" height="140" rx="21" ry="21" fill="#ffffff" stroke="#000000" pointer-events="all"/><rect x="142" y="0" width="50" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 10px; margin-left: 167px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">主程序</div></div></div></foreignObject><text x="167" y="14" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">主程序</text></switch></g><rect x="412" y="40" width="90" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 50px; margin-left: 457px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">信号处理程序</div></div></div></foreignObject><text x="457" y="54" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">信号处理程序</text></switch></g><path d="M 227 80 L 407 140 L 227 200 Z" fill="none" stroke="#000000" stroke-miterlimit="10" stroke-dasharray="3 3" transform="rotate(180,317,140)" pointer-events="all"/><path d="M 317 110 L 334.98 103.24" fill="none" stroke="#000000" stroke-miterlimit="10" pointer-events="stroke"/><path d="M 339.89 101.39 L 334.57 107.13 L 334.98 103.24 L 332.11 100.58 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><path d="M 407 120 L 407 144.63" fill="none" stroke="#000000" stroke-miterlimit="10" pointer-events="stroke"/><path d="M 407 149.88 L 403.5 142.88 L 407 144.63 L 410.5 142.88 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><path d="M 331.04 173.96 L 309.09 167.77" fill="none" stroke="#000000" stroke-miterlimit="10" pointer-events="stroke"/><path d="M 304.04 166.34 L 311.72 164.88 L 309.09 167.77 L 309.82 171.61 Z" fill="#000000" stroke="#000000" stroke-miterlimit="10" pointer-events="all"/><rect x="247" y="70" width="130" height="30" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 85px; margin-left: 312px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">(2) 内核代表进程调用<br />信号处理程序</div></div></div></foreignObject><text x="312" y="89" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">(2) 内核代表进程调用...</text></switch></g><rect x="417" y="125" width="110" height="30" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 140px; margin-left: 472px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">(3) 执行信号处理<br />程序代码</div></div></div></foreignObject><text x="472" y="144" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">(3) 执行信号处理...</text></switch></g><rect x="407" y="190" width="40" height="20" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 200px; margin-left: 427px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">返回</div></div></div></foreignObject><text x="427" y="204" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">返回</text></switch></g><rect x="257" y="180" width="110" height="30" fill="none" stroke="none" pointer-events="all"/><g transform="translate(-0.5 -0.5)"><switch><foreignObject style="overflow: visible; text-align: left;" pointer-events="none" width="100%" height="100%" requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"><div xmlns="http://www.w3.org/1999/xhtml" style="display: flex; align-items: unsafe center; justify-content: unsafe center; width: 1px; height: 1px; padding-top: 195px; margin-left: 312px;"><div style="box-sizing: border-box; font-size: 0; text-align: center; "><div style="display: inline-block; font-size: 12px; font-family: Helvetica; color: #000000; line-height: 1.2; pointer-events: all; white-space: nowrap; ">(4) 程序从中断点<br />恢复执行</div></div></div></foreignObject><text x="312" y="199" fill="#000000" font-family="Helvetica" font-size="12px" text-anchor="middle">(4) 程序从中断点...</text></switch></g></g><switch><g requiredFeatures="http://www.w3.org/TR/SVG11/feature#Extensibility"/></switch></svg>



## 发送信号
`kill()` 系统调用用于发送一个信号，与shell中的kill命令类似。
```c
#include <signal.h>

// 0表示成功， -1 表示失败
int kill(pid_t pid, int sig);
```

当pid设置为以下值时， 将会对不同的进程发送sig信号:
- 大于0: 发送信号给指定pid的进程
- 等于0: 发送信号给与调用进程同组的每个进程
- 小于-1: 会向id等于pid绝对值的进程组内所有下属进程发送信号
- 等于-1: 发送信号给除调用进程以及init(ID为1)进程外的所有进程， 如果时特权级进程发起的这一调用， 那么会发给系统中除前述外的两个进程外的所有进程。

进程发送信号需要一定的权限:
- 特权级(CAP_KILL)进程可给任何进程发送信号
- 同用户的进程间可以发送信号
- 非特权进程可向会话中任何进程发送SIGCONT信号

`kill()` 发送给多个进程信号时， 只要一个发送成功则调用成功。

> 可使用kill检测进程是否存在， 只要将参数sig指定为0(空信号)， 则无信号发送。 再根据返回值和errno判断进程是否存在。若返回0或-1且errno为EPERM(无权限)时表示进程存在，否则进程不存在。

其他发送信号的系统调用有`raise()` 及`killpg()` 
```c
#include <signal.h>


/**
 * @brief 给当前进程发送信号
 * 
 * @param sig 信号值
 * @return int 0则发送成功， 非0出现错误
 * @note 相当于kill(getpid(), sig)
 */
int raise(int sig);

/**
 * @brief 给进程组发送信号
 * 
 * @param pgrp 进程组id 
 * @param sig 信号值
 * @return int 0 表示成功， -1表示失败
 * @note 相当于kill(-pgrp, sig)
 */
int killpg(pid_t pgrp, int sig);

```

显示信号描述的系统调用`char* strsignal(int sig)`




