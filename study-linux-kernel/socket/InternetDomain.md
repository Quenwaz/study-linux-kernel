# 目的
学习Socket Internet Domain

# 目录
- [目的](#目的)
- [目录](#目录)
- [Internet domain socket](#internet-domain-socket)
- [网络字节序](#网络字节序)
- [Internet socket地址](#internet-socket地址)
  - [IPv4 socket 地址： struct sockaddr_in](#ipv4-socket-地址-struct-sockaddr_in)
  - [IPv6 socket 地址：struct sockaddr_in6](#ipv6-socket-地址struct-sockaddr_in6)
- [主机和服务转换函数概述](#主机和服务转换函数概述)
- [inet_pton() 和 inet_ntop()函数](#inet_pton-和-inet_ntop函数)
- [域名系统(DNS)](#域名系统dns)
- [/etc/services文件](#etcservices文件)
- [独立于协议的主机和服务转换](#独立于协议的主机和服务转换)
  - [getaddrinfo() 函数](#getaddrinfo-函数)


# Internet domain socket
Internet domain 流socket基于TCP, Internet domain 数据报socket是基于UDP。 

UDP socket 与之在 UNIX domain 中的对应实体类似， 但有如下差别:
- UNIX domain 数据报socket 是可靠的。 而UDP socket 则是不可靠的， 可能丢失、重复、失序等。
- UNIX domain 数据报socket 在接收socket的数据队列满时发送将会被**阻塞**。 而**UDP在接收队列满时， 将溢出丢弃**。


# 网络字节序
IP地址和端口号是整数值。 将这些值在网络中传输时，因不同硬件结构会以不同的顺序来存储一个多字节整数字节。一般分为**小端**字节序和**大端**字节序。

存储数据时内存低地址处存储数据的最高有效位(比如3210最高有效位为3)则为**大端序**， 相反， 最低位先存储最低有效位称为**小端序**。

如下图所示数据最高有效位为12， 最低有效位为78。
![字节序](img/byte-order.jpg)

在特定主机上使用的字节序被称为**主机字节序**。 由于端口号和IP地址必须在网络中的所有主机之间传递， 并且需要被它们所理解， 因此必须要使用一个标准字节序。 这种字节序被称为**网络字节序**， 它也是大端字节序。

C中常用的字节序转换函数如下: 

```c
#include <arpa/inet.h>

// 16字节无符号整数， 主机字节序转换为网络字节序
uint16_t htons(uint16_t host_uint16);

// 32字节无符号整数， 主机字节序转换为网络字节序
uint32_t htonl(uint32_t host_uint32);

// 16字节无符号整数， 网络字节序转换为主机字节序
uint16_t ntohs(uint16_t net_uint16);

// 32字节无符号整数， 网络字节序转换为主机字节序
uint32_t ntohl(uint32_t net_uint32);
```

# Internet socket地址

## IPv4 socket 地址： struct sockaddr_in

IPv4 socket地址存储在一个sockcaddr_in结构中， 具体定义如下:

```c
#include <netinet/in.h>

struct in_addr{
    in_addr_t s_addr; // 无符号32位整型
};

struct sockaddr_in{
    sa_family_t     sin_family;  // Address family(AF_INET)
    in_port_t       sin_port;    // Port number
    struct in_addr  sin_addr;    // IPv4 address
    unsigned char   __pad[X];
};

```


## IPv6 socket 地址：struct sockaddr_in6

IPv6地址是128位而不是32位。 一个IPv6 socket 地址会被存储在一个sockaddr_in6结构中。具体如下:
```c
#include <netinet/in.h>

struct in6_addr{
    uint8_t s6_addr[16]; // 16字节 == 128位
}；

struct sockaddr_in6{
    sa_family_t     sin6_family;
    in_port_t       sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};


```


# 主机和服务转换函数概述


# inet_pton() 和 inet_ntop()函数


# 域名系统(DNS)


# /etc/services文件

# 独立于协议的主机和服务转换

## getaddrinfo() 函数

