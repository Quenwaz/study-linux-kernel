# 目的
学习Linux 套接字编程

# 目录
- [目的](#目的)
- [目录](#目录)
- [Socket概述](#socket概述)
  - [通信domain](#通信domain)
  - [socket 类型](#socket-类型)
- [socket系统调用](#socket系统调用)
  - [创建socket](#创建socket)
  - [socket绑定到地址](#socket绑定到地址)
  - [监听接入连接：listen()](#监听接入连接listen)
- [流socket](#流socket)

# Socket概述
socket是一个各应用间允许通信的“设备”

## 通信domain
socket 存在与一个通信domain中， 它可确定:
- 识别出一个socket的方法(即socket"地址"格式)
- 通信范围
  - 同一主机不同进程
  - 不同主机之间的进程

现代操作系统至少支撑如下domain:

| Domain            | 执行的通信 | 应用程序间的通信           | 地址格式                 | 地址结构     |
| ----------------- | ---------- | -------------------------- | ------------------------ | ------------ |
| AF_UNIX(AF_LOCAL) | 内核中     | 同一主机                   | 路径名                   | sockaddr_un  |
| AF_INET           | 通过IPv4   | 通过IPv4网络连接起来的主机 | 32为IPv4地址+16位端口号  | sockaddr_in  |
| AF_INET6          | 通过IPv6   | 通过IPv6网络连接起来的主机 | 128为IPv6地址+16位端口号 | sockaddr_in6 |

## socket 类型

| 特性             | 流(SOCK_STREAM)  | 数据报(SOCK_DGRAM) |
| ---------------- | --- | ------ |
| 是否可靠传输     | 是  | 否     |
| 消息边界是否保留 | 否  | 是     |
| 面向连接         | 是  | 否     |

流socket(SOCK_STREAM)提供了一个可靠的双向的字节流通信信道:
- 可靠的: 可保证发送者传输的数据会完整无缺第到达接收应用程序或收到一个传输失败通知。
- 双向的：表示数据可在两个socket之间任意方向上传输
- 字节流：表示与管道一样不存在详细边界的概念

数据包socket(SOCK_DGRAM)允许数据以被称为数据报的消息的形式进行交换。在数据报socket中消息边界得到了保留， 但数据传输是不可靠的。消息的到达**可能是无需的、重复的或丢失的**。

# socket系统调用
- socket()：创建socket
- bind()： 将socket绑定的一个地址上。
- listen()：允许一个流socket接受来自其他socket的接入连接。
- accept()：在一个监听流socket上接受来自一个对等应用程序的连接。
- connect()：建立与另一个socket之间的连接。

> socket I/O可使用传统的read()和write()系统调用或使用一组socket特有的系统调用(如send()、reccv()、sendto()及recvfrom())来完成。默认情况下这些系统调用在I/O操作时会被阻塞。可使用`fcntl()` `F_SETFL`来启用`O_NONBLOCK`标志便以执行非阻塞I/O.

## 创建socket

```c
#include <sys/socket.h>

/**
 * @brief 创建一个套接字
 * 
 * @param domain 协议族。通信domain. AF_UNIX|AF_INET|AF_INET6
 * @param type socket 类型. SOCK_STREAM|SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK
 * @param protocol 传输协议。通常为0。可指定 IPPROTO_TCP|IPPROTO_UDP....
 * @return int 返回套接字， -1则创建失败
 */
int socket(int domain, int type, int protocol);
```

## socket绑定到地址

```c
#include <sys/socket.h>

/**
 * @brief 将socket绑定到地址
 * 
 * @param sockfd socket
 * @param addr addr地址结构
 * @param addrlen addr长度
 * @return int 0表示成功， -1 则失败
 * @note 通常服务端调用
 */
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
```

## 监听接入连接：listen()

```c
#include <sys/socket.h>

/**
 * @brief 将文件描述符sockfd引用的流socket标记为被动。 这个socket用来接受来自其他socket的连接
 * 
 * @param sockfd socket
 * @param backlog 可接受未决连接数的数量
 * @return int 返回0表示成功， -1则出错
 */
int listen(int sockfd, int backlog);
```

理解参数`backlog`:

![backlog](img/backlog.jpg)

# 流socket

#