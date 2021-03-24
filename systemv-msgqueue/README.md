# System V 消息队列

### 概述

什么是System V 消息队列？它是早期的System V UNIX系统上的一种进程间通讯机制。 所谓消息队列， 即它是一个队列，与管道类似都是先进先出的数据结构。那么区别是什么呢？有如下几点:

- 消息队列存在边界(管道不存在边界)
- 消息队列具有一个整形type字段

消息队列进行通信是面向消息的， 即读者接收到由写者写入的整条消息。 **读取一条消息的一部分而让剩余部分遗留在队列中或一次性读取多条消息是不可能的。**

### 系统调用

- **创建或打开一个消息队列**

```c
#include <sys/types.h>
#include <sys/msg.h>

// 如果调用成功， 返回消息队列的ID, 否则返回-1
int msgget(key_t key, int msgflg);
```

其中key参数可以指定为IPC_PRIVATE使其生成一个键， 或调用`ftok()`返回一个唯一键(大概率上)。

而msgflg参数可指定权限或检查一个已有队列的权限位掩码， 包括如下参数:

**IPC_CREAT:** 如果没有指定的key对应的消息队列， 则创建一个新队列

**IPC_EXCL:** 如果同时还指定了IPC_CREAT并且指定key的队列已存在， 则将返回一个EEXIST错误

还可以指定用户权限:  S_IRUSR、S_IWUSR等

- **发送消息**

```c
#include <sys/types.h>
#include <sys/msg.h>

// 返回0表示成功， 或返回-1表示出错
int msgsnd(int msgid, const void* msgp, size_t msgsz, int msgflg);
```

其中msgp是由程序员进行定义， 在sys/msg.h中定义了一个模板， 如下:

```c
/* Template for struct to be used as argument for `msgsnd' and `msgrcv'.  */
struct msgbuf
{
    __syscall_slong_t mtype;	/* type of received/sent message */
    char mtext[1];				/* text of the message */
};
```

由一个type和消息内容组成， 消息长度可自定义。其中type值需要指定为一个大于0的值， 建议指定为进程id， 这样可以避免竞争读取同一消息。

msgflg 只定义了一个标记**IPC_NOWAIT**,即执行一个非阻塞的发送操作。当消息队列满使， 若不指定此标志则会被阻塞， 否则不阻塞并返回一个EAGAIN错误

- **接收消息**

```c
#include <sys/types.h>
#include <sys/msg.h>

// 返回复制到msgp结构中mtext的字节数， 否则返回-1
ssize_t msgrcv(int msgid, void* msgp, size_t maxmsgsz, long msgtyp, int msgflg);
```

msgrcv() 系统调用从消息队列中读取(以及删除)一条消息并将其内容复制进msgp指向的缓冲区中。

其中msgp中的mtext最大可用空间通过maxmsgsz来指定。 如果读取的消息体超过了maxmsgsz字节， 将返回错误， 且不会删除消息。

**读取消息的顺序无需于消息被发送的一致**， 可根据msgtyp字段来选择消息， 具体选择如下:

当msgtyp等于0， 将删除并返回队列中第一条消息

当msgtyp大于0，将会从队列中取出mtype等于msgtyp的第一条消息并删除。所以当指定大于0的mtype时可避免多进程之间读取同一消息的竞争状态。

当msgtyp小于0，那么消息队列类似优先队列， 会优先取出小于等于msgtyp绝对值的第一条消息， 如果不存在时再取消息将会被阻塞。

而msgflg有如下定义:

**IPC_NOWAIT**指定了此标志位后， 如果没有匹配的消息时将不会阻塞调用。

**MSG_EXCEPT**此标志仅仅在msgtyp大于0时有效， 它会优先取不等于指定msgtyp的第一条消息。

**MSG_NOERROR** 默认情况下， 当指定的maxmsgsz字段小于该消息的长度时，将不会调用失败， 而是删除并返回截断后的消息。

- 消息队列控制操作

```c
#include <sys/types.h>
#include <sys/msg.h>

// 返回0表示成功， 否则返回-1 表示失败
int msgctl(int msgid, int cmd, struct msqid_ds * buf);
```

cmd参数有如下选项:

**IPC_RMID**

**IPC_STAT**
**IPC_SET**





