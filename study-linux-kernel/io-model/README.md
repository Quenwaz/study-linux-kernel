# I/O 模型

开始学习非阻塞式I/O模型， 比如select/poll、信号驱动、epoll等。

## 目录

- [I/O 模型](#io-模型)
  - [目录](#目录)
  - [概述](#概述)
  - [技术选型](#技术选型)
  - [水平触发与边缘触发](#水平触发与边缘触发)
  - [I/O多路复用](#io多路复用)
    - [select](#select)
    - [poll](#poll)
    - [select()与poll()对比](#select与poll对比)
    - [select()与poll()存在的问题](#select与poll存在的问题)
  - [信号驱动I/O](#信号驱动io)
    - [优化信号驱动的使用](#优化信号驱动的使用)
    - [信号队列溢出的问题](#信号队列溢出的问题)
  - [epoll](#epoll)


## 概述

本篇幅涉及到的I/O模型主要是处理如下两种情形:

- 可非阻塞式检查文件描述符是否可进行I/O操作
- **同时检查多个文件描述符**是否可执行I/O操作

之前学习过以`O_NONBLOCK`方式打开文件， 此文件将是非阻塞式的。 如果I/O调用不能立刻完成，将返回错误而不是阻塞调用。非阻塞式I/O可运用到管道、FIFO、套接字、终端、伪终端以及其他设备上。

- 如果将文件设置为非阻塞方式，对多个文件进行I/O是否可操作检查，可能导致浪费CPU。且随着文件数增加，时延增大，最终导致性能下降。
- 而如果使用多进程的方式，比较耗费系统资源， 且导致资源利用率降低。
- 而多线程方式处理需要对资源进行同步处理， 使编程工作变复杂

于是出现了I/O多路复用(select/poll)、信号驱动、epoll。他们的差异如下:

- I/O多路复用可检测多个文件是否可非阻塞式执行I/O
- 而信号驱动I/O会将有数据输入或可输出时， 内核将以信号的方式通知进程。而进程本身可执行其他任务。 **信号驱动比I/O多路复用性能较好。**
- epoll为Linux 2.6及以上专有。亦可实现上述两种I/O功能， 但性能更好。


## 技术选型

- select和poll诞生较早， 所以移植性较好。但是当同时检测大量文件描述符时性能不好。仅能接收水平触发
- epoll能检查大量文件描述符， 主要缺点为Linux专有。 可支持水平(默认)与边缘触发
- 虽然其检查大量文件描述符性能相比I/O多路复用较好， 但是其处理信号较复杂。 仅支持边缘触发

## 水平触发与边缘触发

- 水平触发: 如果文件可非阻塞式执行I/O系统调用， 则触发通知
- 边缘触发: 如果文件自上次状态检测以来有新的I/O活动， 则触发通知

| I/O模型         | 水平触发     | 边缘触发 |
| --------------- | ------------ | -------- |
| select(),poll() | Yes          | No       |
| 信号驱动I/O     | No           | Yes      |
| epoll           | Yes(default) | Yes      |

- 当采用水平触发通知时，**无需尽可能多执行I/O(如尽可能多读字节数)**。因为可重复检查I/O状态，只要其可非阻塞调用总会取完。
- 而采用边缘触发时，**需尽可能多执行I/O**。因为不这么做， 只有等下一次I/O状态变化。


> 由上述可知， 当使用以上I/O模型时，**需设置文件描述符为O_NONBLOCK标志**。 
>
> 当采用边缘触发或水平触发通知时，若操作了过多的数据， 若不设置O_NONBLOCK状态将导致调用阻塞
>
> 当使用多进程或多线程时， 若同时在一个文件描述符时执行I/O， 可能某个进程或线程的I/O操作会导致其他进程或线程执行I/O操作时阻塞。 如在通知就绪和执行I/O之间，某个进程或线程操作I/O， 将导致I/O变成不就绪状态。 所以需要将文件描述符设置为O_NONBLOCK状态

## I/O多路复用

I/O多路复用主要系统调用为`select()`与`poll()`。

### select

系统调用select()阻塞至某个文件描述符称为就绪态， 即可非阻塞执行I/O。

```c
#include <sys/time.h>
#include <sys/select.h>


/**
 * @brief 检查文件描述符是否可非阻塞式执行I/O
 * 
 * @param nfds 最大文件描述符的值+1
 * @param readfds 该文件描述符集合有可非阻塞式读取数据的文件描述符时，将会通知
 * @param writefds 该描述符集合中有可非阻塞式写的文件描述符时，将会通知
 * @param exceptfds 该描述符集合中有可异常的文件描述符时，如socket收到了带外数据。将会通知
 * @param timeout 等待超时。NULL将无限等待， 0时将不等待。 否则将等待。
 * @return int 0 表示超时；1 表示出错；大于0时表示有一个或多个文件描述符就绪（1个文件若读写都就绪将返回2）
 */
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);

void FD_ZERO(fd_set* fdset); /// 清空文件描述符集合
void FD_SET(int fd, fd_set* fdset); /// 将fd添加到fdset中
void FD_CLR(int fd, fd_set* fdset); /// 将fd从fdset中移除
void FD_ISSET(int fd, fd_set* fdset); /// fd是否在fdset中
```

> 注意select第一个参数nfds为所有文件描述值加1， 便于内核仅仅检查小于等于nfds的文件描述符即可。
>
> 每次调用select需要重置fd_set及timeout


### poll

系统调用`poll()`与select()类似， 区别在于指定检查文件描述符的方式。

```c
#include <poll.h>

struct pollfd{
  int fd;         /*指定文件描述符*/
  short events;   /*位掩码指定关注事件读写*/
  short revents;  /*位掩码表示某事件发生*/
};

/**
 * @brief  检查文件描述符是否可非阻塞式执行I/O
 * 
 * @param fds 文件描述符及关注事件列表
 * @param nfds fds数组长度
 * @param timeout 超时毫秒数。-1 阻塞； 0 不阻塞； 大于0除非某文件就绪否则将阻塞指定毫秒数
 * @return int -1表示错误；0表示超时；大于0表示有文件描述符已就绪(表示fds数组有几个就绪)
 */
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

上述API中`struct pollfd`的`event`与`revent`可取如下字段:

| 位掩码     | event是否可设 | revent返回 | 描述                                                 |
| ---------- | ------------- | ---------- | ---------------------------------------------------- |
| POLLIN     | Y             | Y          | 可读取非高优先级的数据                               |
| POLLRDNORM | Y             | Y          | 等同POLLIN, 需定义_XOPEN_SOURCE                      |
| POLLRDBAND | Y             | Y          | 可读取优先级数据(Linux中不使用), 需定义_XOPEN_SOURCE |
| POLLPRI    | Y             | Y          | 可读取高优先级数据                                   |
| POLLRDHUP  | Y             | Y          | 对端套接字关闭, 需定义_GNU_SOURCE                    |
| POLLOUT    | Y             | Y          | 普通数据可写                                         |
| POLLWRNORM | Y             | Y          | 等同于POLLOUT, 需定义_XOPEN_SOURCE                   |
| POLLWRBAND | Y             | Y          | 优先数据可写, 需定义_XOPEN_SOURCE                    |
| POLLERR    | N             | Y          | 有错误发生                                           |
| POLLHUP    | N             | Y          | 出现挂断                                             |
| POLLNVAL   | N             | Y          | 文件描述符未打开，或在调用poll时文件描述符关闭了。   |
| POLLMSG    | N             | N          | Linux中不使用                                        |

基本上常用标志为：POLLIN、POLLOUT、POLLPRI、POLLRDHUP、POLLHUP及POLLERR。

如暂时不关注某文件描述符，可将events设置为0或将fd设置为文件描述符的负数，将导致event被忽略且revents总是返回0。

### select()与poll()对比

**基本差异**

- select()所使用的文件描述符集合fd_set是静态数组受FD_SETSIZE限制，修改此值需要重新编译程序。而poll()由于是动态数组，本质上没有限制。
- select()每次调用需要将fd_set重置， 因为fd_set在select()返回后保存了返回状态。而poll()以revents方式返回，避免了此操作。
- select()超时精度(微秒)比poll()超时精度(毫秒)高。
- poll()可准确由revent告知哪个文件描述符关闭了(POLLNVAL),而select()仅仅返回-1，设置错误码EBADF， 无法直接直到哪个文件描述符关闭。

**移植性差异**

- select()应用更广泛， poll()在不同实现中存在差异。

**性能差异**

- 当检查文件描述符范围小， 且集中(聚集在0到某个上限之间)时， 性能相当
- 如果文件描述符集合较稀疏的情况，poll()将优于select()。因select()循环次数增加更多(其中过多无用文件描述符)。

### select()与poll()存在的问题

- 每次调用select()和poll(), 内核需要检查所有被指定的文件描述符。
- 每次调用select()和poll(), 随着文件描述符检测数增大，需要将描述文件描述符的数据结构从用户空间拷贝到内核空间， 而内核检测到就绪态后， 又需将数据从内核空间拷贝到用户空间。 需要耗费较大CPU时间。
- select()和poll()在每次调用返回后， 需要循环检测某个文件描述符状态。

## 信号驱动I/O
与select()和poll()相比，之所以信号驱动I/O能够显著提升性能， 是因为内核能**记住**需要检查的文件描述符，且仅当某文件描述符I/O事件发生时才向程序发送信号。

对于信号驱动I/O， 在进程设定当文件描述符上可执行I/O操作时，请求内核为自己发送一个信号。在这之前进程可做其他工作。 使用信号驱动I/O基本步骤如下:
1. 为内核发送的通知信号安装一个信号处理例程。通知信号一般为`SIGIO`
2. 设定文件描述符的属主，属主就是用来告知内核， 那个进程或进程组来接收这个信号通知。一般设置为调用进程为属主。可调用`fcntl`的`F_SETOWN`来指定。
   
   ```fcntl(fd, F_SETOWN, pid)```
3. 设置文件描述符为`O_NONBLOCK`标志， 保证其非阻塞
4. 设置文件描述符`O_ASYNC`标志使能信号驱动I/O。可以与上一步一起设置:
   
   ```fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_ASYNC| O_NONBLOCK)```
5. 此时进程可做其他任务了。 当存在I/O事件时会得到通知
6. 信号驱动I/O是**边缘触发通知**。 表示一旦进程被通知I/O就绪， 需尽可能多地执行I/O(如尽可能多地读取字节)。非阻塞式I/O需读取到失败(EAGAIN或EWOULDBLOCK)为止

> 信号驱动可应用于套接字、终端、伪终端、管道和FIFO上。

### 优化信号驱动的使用

上面描述的构建信号驱动步骤中， 使用的信号为标准信号`SIGIO`， 那么标准信号会存在同类等待信号只会通知一次的情况， 而不像实时信号会排队。其次，由于标准信号处理函数没有额外数据`siginfo_t`,所以无法区分，当前信号处理是来自哪个文件描述符。

需要执行如下两个步骤来完善这些缺陷:
1. 通过`fcntl()`的`F_SETSIG`来为文件描述符指定一个实时信号, 当文件描述符上的I/O就绪时， 这个实时信号应该取代SIGIO被发送。
2. 使用`sigaction()`安装信号处理时，需指定`SA_SIGINFO`标记。 以接收`siginfo_t`信息。

```c
if (fcntl(fd, F_SETSIG, sig) == -1){
  exit(1);
}
```

当收到I/O信号后， 其中`siginfo_t`结构如下:
- si_signo: 触发信号处理的信号值
- si_fd: 发生I/O事件的文件描述符
- si_code: 表示发生事件类型的代码。如POLL_IN、POLL_OUT等，更多详见[sigaction manpage](https://man7.org/linux/man-pages/man2/sigaction.2.html)
- si_band: 一个位掩码。与系统调用poll()中返回的revents相同。

通常si_code与si_band对应值如下:
|si_code|si_band掩码|描述|
|----------|----------|----------|
|POLL_IN|POLLIN &#124; POLLRDNORM|存在输入；文件结尾情况|
|POLL_OUT|POLLOUT &#124; POLLWRNORM &#124; POLLWRBAND|可输出|
|POLL_MSG|POLLIN &#124; POLLRDNORM &#124; POLLMSG|存在输出消息(不使用)|
|POLL_ERR|POLLERR|I/O错误|
|POLL_PRI|POLLPRI &#124; POLLRDNORM|存在高优先级输入|
|POLL_HUP|POLLHUP &#124; POLLERR|出现宕机|


### 信号队列溢出的问题
学习实时信号时提到过， 实时信号的可排队数量是有限的。 如果达到上限，内核对于“I/O就绪”的通知将恢复为默认的SIGIO信号。出现这种现象表示信号队列溢出了。当出现这种情况时， 我么将失去有关文件描述符上发生I/O事件的信息， 因为SIGIO信号不会排队。


## epoll