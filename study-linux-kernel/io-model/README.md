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
    - [多线程中使用信号驱动I/O](#多线程中使用信号驱动io)
  - [epoll](#epoll)
    - [创建epoll实例](#创建epoll实例)
    - [修改epoll兴趣列表](#修改epoll兴趣列表)
      - [感兴趣文件描述符上限](#感兴趣文件描述符上限)
    - [事件等待](#事件等待)
    - [epoll事件](#epoll事件)
      - [EPOLLNESHOT标志](#epollneshot标志)
    - [深入epoll语义](#深入epoll语义)
    - [epoll与I/O多路复用的性能对比](#epoll与io多路复用的性能对比)
    - [边缘触发通知](#边缘触发通知)
      - [边缘触发时避免出现文件描述符饥饿现象](#边缘触发时避免出现文件描述符饥饿现象)


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
| si_code  | si_band掩码                                 | 描述                   |
| -------- | ------------------------------------------- | ---------------------- |
| POLL_IN  | POLLIN &#124; POLLRDNORM                    | 存在输入；文件结尾情况 |
| POLL_OUT | POLLOUT &#124; POLLWRNORM &#124; POLLWRBAND | 可输出                 |
| POLL_MSG | POLLIN &#124; POLLRDNORM &#124; POLLMSG     | 存在输出消息(不使用)   |
| POLL_ERR | POLLERR                                     | I/O错误                |
| POLL_PRI | POLLPRI &#124; POLLRDNORM                   | 存在高优先级输入       |
| POLL_HUP | POLLHUP &#124; POLLERR                      | 出现宕机               |


### 信号队列溢出的问题
学习实时信号时提到过， 实时信号的可排队数量是有限的。 如果达到上限，内核对于“I/O就绪”的通知将恢复为默认的SIGIO信号。出现这种现象表示信号队列溢出了。当出现这种情况时， 将失去有关文件描述符上发生I/O事件的信息， 因为SIGIO信号不会排队。

> 一个设计良好的采用`F_SETSIG`来建立实时信号作为“I/O就绪”通知的程序必须也要为信号SIGIO安装处理程序。 如果发送了`SIGIO`信号， 应用程序可先通过`sigwaitinfo()`将队列中的实时信号全部获取，然后临时切换到`select()`或`poll()`， 通过它们获取剩余的发生I/O事件的文件描述符列表。


### 多线程中使用信号驱动I/O

从内核2.6.32开始， Linux 提供了两个新的非标准`fcntl()`操作， 可用于设定接收I/O就绪信号的目标。 它们是`F_SETOWN_EX`和`F_GETOWN_EX`

`fcntl()`的第三个参数指向如下结构体指针:
```c
struct f_owner_ex{
  int type;
  pid_t pid;
};
```

当type设置为`F_OWNER_TID`时， 字段pid指定了作为接收I/O就绪信号的线程ID。

## epoll

epoll为Linux专用接口， 主要有如下优点:
- 当检查大量文件描述符时， epoll的性能延展性比select()和poll()高
- epoll既支持水平触发也支持边缘触发。
- 避免信号驱动的复杂处理流程（避免信号驱动的队列溢出问题）
- 灵活性高，可指定我们希望检查的事件类型

epoll API核心数据结构称为epoll实例， 它与一个打开的文件描述符相关联。而这个文件描述符不是用来做I/O操作的， 它时内核数据结构句柄，内核数据结构实现了两个目的
1. 记录在进程中声明过感兴趣文件描述符列表----interest list(兴趣列表)
2. 维护处于I/O就绪态的文件描述符列表----ready list(就绪列表)

> ready list中的成员是interest list的子集。

epoll API由以下三个系统调用组成:
1. epoll_create()： 创建一个epoll实例， 返回代表该实例的文件描述符。
2. epoll_ctl()：操作同epoll实例相关联的兴趣列表。 可新增删除文件描述符， 或修改文件描述符上的事件类型掩码。
3. epoll_wait(): 返回与epoll实例相关联的就绪列表的成员。

### 创建epoll实例

```c
#include <sys/epoll.h>

/**
 * @brief 创建epoll实例
 * 
 * @param size 指定检查文件描述符个数。此值不是一个上限。而是告诉内核的一个初始值 
 * @return int 成功返回文件描述符， 失败返回-1 
 * @note size在 Linux 2.6.8之后被忽略不用。
 * 对于返回值的文件描述符， 不用时应调用close()进行关闭。
 */
int epoll_create(int size);
```

### 修改epoll兴趣列表

```c
#include <sys/epoll.h>

struct epoll_event
{
    uint32_t events;    /*epoll events(bit mask)*/
    epoll_data_t data;  /*User data*/
};

/**
 * @brief 修改用文件描述符epfd所代表的兴趣列表
 * 
 * @param epfd 由epoll_create创建的文件描述符
 * @param op 操作选项
 * @param fd 感兴趣的文件描述符， 
 *           可以是管道、FIFO、套接字、POSIX消息队列、inotify实例、终端、设备，
 *           甚至可以是另一个epoll文件描述符。不能是普通文件或目录的文件描述符
 * @param ev 位掩码，指定待检测描述符所感兴趣事件集合
 * @return int 返回0表示成功， -1 表示出错
 */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev);
```

其中`op`可选值如下:
- EPOLL_CTL_ADD: 添加文件描述符到感兴趣列表。 添加一个已存在的将出现EEXIST错误
- EPOLL_CTL_MOD：修改描述符上设定的事件， 修改不在感兴趣列表中的文件描述符， 将出现ENOENT错误
- EPOLL_CTL_DEL: 将文件描述符从感兴趣列表删除， 将忽略ev参数。 试图移除一个不存在的文件描述符，将出现错误ENOENT。**关闭文件描述符会自动将其从epoll感兴趣列表移除**。

#### 感兴趣文件描述符上限
epoll实例上的文件描述符需占用一段不能被交换的内核内存空间。因此内核提供了接口用来定义每个用户可注册到epoll实例上的文件描述符总数。 这个上限可通过`max_user_watches`查看和修改。文件位于`/proc/sys/fs/epoll`目录下。**默认上限值可根据系统内存来计算得出**

### 事件等待

```c
#include <sys/epoll.h>

/**
 * @brief 等待文件描述符就绪
 * 
 * @param epfd epoll文件描述符实例
 * @param evlist [in/out] 返回就绪态的文件描述符信息, 用户需先分配内存。
 * @param maxevents  evlist列表长度， 每次将从就绪列表中取出指定大小的就绪信息
 * @param timeout -1 表示阻塞等待; 0表示非阻塞检查; >0 表示阻塞毫秒数或就绪或信号发生
 * @return int >0 返回就绪的文件描述符数量; =0 超时; -1 表示错误发生
 */
int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);
```

### epoll事件

| 位掩码      | 作为epoll_ctl()的输入 | 由epoll_wait()返回 | 描述                       |
| ----------- | --------------------- | ------------------ | -------------------------- |
| EPOLLIN     | Y                     | Y                  | 可读取非高优先级的数据     |
| EPOLLPRI    | Y                     | Y                  | 可读取高优先级数据         |
| EPOLLRDHUP  | Y                     | Y                  | 套接字对端关闭             |
| EPOLLOUT    | Y                     | Y                  | 普通数据可写               |
| EPOLLET     | Y                     | N                  | 采用边缘触发事件通知       |
| EPOLLNESHOT | Y                     | N                  | 在完成事件通知之后禁用检查 |
| EPOLLERR    | N                     | Y                  | 有错误发生                 |
| EPOLLHUP    | N                     | Y                  | 出现挂断                   |

#### EPOLLNESHOT标志
此标志将会在`epoll_wait()`获取到其就绪通知后处于非激活状态， 下次将不会得到通知。直到利用`epoll_ctl()`使用`EPOLL_CTL_MOD`标志重新激活对这个文件描述符的检查。**不能用EPOLL_CTL_ADD标志， 因为文件描述符仍然在兴趣列表中处于非激活状态**


### 深入epoll语义

由于`epoll_create()`创建一个epoll实例时， 内核在内存中创建一个新的`i-node`及打开的文件句柄。随后在调用进程中为打开的文件句柄分配一个新的文件描述符。 同epoll兴趣列表关联的是打开的文件句柄， 而非epoll文件描述符。

> 注意`[文件描述符]`、`[打开的文件句柄]`及`[i-node]`之间的关系

由于以上关系， 就会出现如下情况:
- 当利用`dup()`或类似函数复制了epoll文件描述符。 将共用epoll兴趣列表和就绪列表。 两个文件描述符指向相同的兴趣列表和就绪列表。 利用`epoll_ctl`修改任意一个文件描述符，将导致另一个关联的兴趣列表和就绪列表更改。
- 同时对于`fork()`也会出现以上情况

当利用`epoll_ctl()`的`EPOLL_CTL_ADD`在`epoll`兴趣列表中添加一个元素， 这个元素同时记录了需要检查的文件描述符数量**以及对应文件描述符的引用**。所以上文[修改epoll兴趣列表](#修改epoll兴趣列表)提到的关闭兴趣列表中某个文件描述符，将会自动从兴趣列表移除。更严谨地是：**当所有打开的文件句柄的文件描述符都关闭后， 这个打开的文件句柄才会从epoll兴趣列表中移除**。


### epoll与I/O多路复用的性能对比
实验证明， 随着文件描述符数量的上升， `poll()`和`select()`的性能表现越来越差。 而对于`epoll()`， 当文件描述符数量增长很大的值时， 性能几乎不受影响。之所以`epll()`能保持性能稳定， 主要因为有如下几点:
- 对于`select()`和`poll()`， 内核必须检查所有在调用中指定的文件描述符。而epoll()只要指定了文件描述符， 内核会在打开的文件句柄相关的兴趣列表中与之关联。 之后每执行I/O操作使得文件描述符成为就绪态时， 内核就在epoll描述符的就绪列表中添加一个元素。(单个打开的文件句柄I/O就绪会导致与之关联的文件描述符就绪)
- 对于`select()`和`poll()`，标记待监视的文件描述符及事件的数据结构， 需要在内核态与进程态之间至少传递两次。而`epoll()`在内核空间中建立数据结构， 该结构将待监视的文件描述符记录在兴趣列表。之后只需获取就绪的文件描述符。

### 边缘触发通知
默认情况下`epoll()`提供的时水平触发通知。 要使用边缘触发通知， 需调用`epoll_ctl()`时在`ev.events`字段中指定`EPOLLET`标志。

```c
struct epoll_event ev;
ev.data.fd = fd;
ev.events = EPOLLIN|EPOLLET;
if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev) == -1){
  exit(1);
}
```

采用epoll的边缘通知机制的程序基本框架为:
1. 将待监视的文件描述符设为非阻塞的
2. 通过`epoll_ctl()`构建epoll兴趣列表
3. 通过如下循环处理I/O事件
   1. 通过`epoll_wait()`取得处于就绪态的描述符列表
   2. 针对每个就绪态文件描述符， 不断进行I/O直到相关系统调用返回`EAGAIN`或`EWOULDBLOCK`错误


#### 边缘触发时避免出现文件描述符饥饿现象

所谓饥饿现象的一般场景是： 当监控了多个文件描述符， 此时其中一个文件描述符上有大量的输入存在(持续的流数据)。 如果检测到此文件描述符处于就绪态后， 通过非阻塞读取直到出现错误`EAGAIN`或`EWOULDBLOCK`。 将会导致其他文件描述符处于无法读取的状态。

面临此场景， 一贯的处理办法是:
- 在用户空间维护一个列表A， 用于存储就绪的文件描述符
- 调用epoll_wait()监视文件描述符， 将处于就绪态的文件描述符添加到列表A中。 若这个文件已存在在文件列表A中， 则将等待超时事件设置为较小的值， 尽量避免在epoll_wait()上等待，继而对列表A进行I/O操作。
- **旋转循环**(而不是每次从头开始)列表A， 在循环到的文件描述符上进行**一定限度**的I/O操作。 当I/O操作出错`EAGAIN`或`EWOULDBLOCK`时，则将文件描述符从列表中A移除。
