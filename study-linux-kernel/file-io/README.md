# 目的
学习通用文件I/O

# 目录
- [目的](#目的)
- [目录](#目录)
- [概述](#概述)
  - [标准文件描述符](#标准文件描述符)
- [基本文件操作](#基本文件操作)
  - [打开(或创建)](#打开或创建)
  - [读写](#读写)
  - [关闭](#关闭)
- [文件描述符和打开文件之间的关系](#文件描述符和打开文件之间的关系)
- [/dev/fd目录](#devfd目录)
- [创建临时文件](#创建临时文件)

# 概述
所有执行I/O操作的系统调用都以文件描述符，一个非负整数来指代代开的文件。文件描述符可表示管道、FIFO、socket、终端、设备和普通文件。针对每个进程， 文件描述符都自成一套。
## 标准文件描述符
当程序从shell终端打开时， 就继承了3个标准文件描述符。 

|文件描述符|用途|POSIX名称|stdio流|
|---------|----|---------|-------|
|0|标准输入|STDIN_FIFENO|stdin|
|1|标准输出|STDOUT_FIFENO|stdout|
|2|标准错误|STDERR_FIFENO|stderr|

# 基本文件操作

执行文件I/O操作的主要系统调用有如下四个:
- fd=`open`(pathname, flags, mode)：打开或创建文件
- numread=`read`(fd, buffer, count)：返回实际读取到的字节数， 当遇到文件结尾符EOF时返回0
- numwritten=`write`(fd, buffer, count)：write返回的值可能小于count值
- status=`close`(fd)： 释放文件描述符及相关内核资源


## 打开(或创建)
```c
#include <sys/stat.h>
#include <fcntl.h>

// 返回非负数则成功， -1则失败
int open(const char* pathname, int flags, .../*mode_t mode*/);
```
flags为位掩码， 用于指定文件的访问模式， 可选值如下：
- O_RDONLY：只读
- O_WRONLY：只写
- O_RDWR: 可读可写

出来以上三种访问模式外， 还可设置其他操作标志:
- O_APPEND: 追加数据
- O_ASYNC: 对文件描述符实施I/O操作时， 会产生信号通知进程。称为`信号驱动I/O`, 仅对特定文件有效，如终端、FIFO及socket。 open()中指定不会产生效果， 需调用`fcntl()`的F_SETFL来启动O_ASYNC标志
- O_CLOEXEC: linux 2.6.23开始支持，用于在open()创建文件时指定， 若后续执行exec系统调用将自动关闭句柄。 若不指定将继承句柄。放在open()中而不是fcntl()中指定主要时为了避免竞争态出现。open()中为原子操作。 fcntl()需要先F_GETFD再F_SETFD来设置，就可能出现竞争状态。
- O_CREAT: 如果文件不存在则创建一个空文件。**若指定了这个标志必须指定mode， 否则权限将随机**。
- O_DIRECT：无系统缓冲的文件I/O操作。 
- O_DIRECTORY: 若pathname参数非目录，错误errno将为ENOTDIR
- O_DSYNC：linux 2.6.23开始支持，系统缓冲将同步I/O读写数据完整性。
- O_EXCL: 与O_CREAT标志结合使用时， 若文件存在将不会打开文件。且errno为EEXIST。内核保证检查文件存在及创建文件两个步骤为原子操作。
- O_LARGEFILE： 支持以大文件方式打开文件。在32位操作系统中使用此标志， 64位系统将无效。
- O_NOATIME：读文件时不更新文件最近访问时间。


其中第三个参数用来指定用户权限mode

## 读写


## 关闭


# 文件描述符和打开文件之间的关系


# /dev/fd目录

对于每个进程， 内核提供一个特殊的虚拟目录`/dev/fd`。 该目录包含`/dev/fd/n`形式的文件名， 其中n是与进程中的打开文件描述符响应编号。

# 创建临时文件
创建临时文件推荐接口为: `mkstemp()` 和 `tmpfile()`

先来看函数`mkstemp()`:
```c
#include <stdlib.h>

/**
 * @brief 生成一个唯一文件名并打开该文件， 返回一个可用于I/O调用的文件描述符
 * 
 * @param template 模板字符串
 * @return int 返回文件描述符则成功， -1则失败
 * @note template 其中最后六个字符必须为XXXXXX, 此六个字符将会被替换成随机串以保证唯一性
 */
int mkstemp(char* template);
```

> `mkstemp` 将文件权限设为仅调用者可读写且以O_EXCL标志打开文件， 以保证调用者以独占方式访问文件。
> 通常在打开临时文件不久， 程序就会使用`unlink`将其删除， 当文件描述符被关闭时， 文件将自动删除。


另一个函数`tmpfile()`, 会创建一个名称唯一的临时文件， 并以读写方式将其打开。与`mkstemp()`一样用O_EXCL方式打开。
```c
#include <stdio.h>

/**
 * @brief 创建一个临时文件
 * 
 * @return FILE* 返回文件流供stdio库函数使用。文件流关闭后将自动删除临时文件。
 */
FILE *tmpfile(void);

```


