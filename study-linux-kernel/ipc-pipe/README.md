## 管道

### 概述

什么是管道？就是一个允许数据从一个进一端流向另一端的一个通道。

### 特性

- 管道是一个字节流

  管道**没有消息边界**的概念， 因为它是一个字节流。从管道中可以读取进程可以读取的任意大小的数据块**。在管道中无法使用lseek()来随机访问数据。**

- 读取管道中的数据

  试图从一个空管道中读取数据将会被阻塞直到至少有一个字节数据被写入到管道中。**如果管道写入段被关闭了， 从管道中读取完所有数据后将会收到文件结束(即read()返回0)**

- 管道是单向的

  在管道中数据的传递是单向的。 管道的一端用于写入另一端则用于读取。

  在其他一些UNIX实现上----如System V Release 4中管道是双向的。但是双向管道未在任何UNIX标准中进行规定。

- 当写入不超过PIPE_BUF字节时操作为原子的

  多个进程写入同一管道， 当在一时刻写入的数据量不超过PIPE_BUF, 则可以确保写入的数据不会混合。

  当写入管道的数据块超过了PIPE_BUF字节， 内核会将数据分割成数个较小片段来传输，在管道数据被读取后在发送后续片段(**write调用会阻塞直到所有数据写入到管道为止**)。如果write阻塞时被信号中断， 那么write将直接返回实际传输到管道的字节数(即部分写入)。

- 管道的容量是有限的

  **管道其实是一个在内核内存中维护的缓冲器**。这个缓冲器存储能力是有限的。

  从Linux 2.6.34后可以调用fcntl(fd, F_SETPIPE_SZ, size)调整缓冲器大小。非特权程序可以修改小于/proc/sys/fs/pipe-max-size的任意值。 特权程序则可以覆盖这个值。

### 注意事项

1. 如果当前进程仅仅为读取数据， 则关闭写入描述符。 这样当其他进程写入完成后关闭文件描述符， 读取进程才能看到文件结束。否则， 如果读者进程未关闭写入描述符， 即使其他进程已经关闭， 读取进程会一直阻塞读取不到文件结束(**类似引用计数**)。

2. 当一个进程向一个管道写入数据， 但没有进程拥有该管道的读取文件描述符， 则内核会向进程发送SIGPIPE信号， 该信号会将信号弄死。当然进程可以捕获或忽略该信号。 所以对于一个只想写入管道的进程来说， 关闭读取文件描述符， 能有效检测到此类状态。

   同时， 如果写入进程没有关闭读取端， 而其他进程已关闭读取端， 此进程会将管道写满后进入阻塞。

3. 关闭未使用文件描述符最后一个原因是只有当所有进程中所有引用一个管道的文件描述符都被关闭之后词汇销毁该管道以释放该管道占用的资源以供其他进程复用。

