#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>



int main(int argc, char const *argv[])
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags =  AI_ADDRCONFIG | AI_V4MAPPED |AI_NUMERICSERV |AI_NUMERICHOST;
    struct addrinfo * result = 0;
    if (getaddrinfo("192.168.60.94", "22", &hints, &result) != 0){
        fprintf(stderr, "occur error: %s\n", strerror(errno));
    }

    if (result->ai_addrlen == sizeof(struct sockaddr_in)){
        struct sockaddr_in addr;
        memcpy(&addr, result->ai_addr, result->ai_addrlen);
        char ip_addr[32] ={0};
        inet_ntop(AF_INET, &addr.sin_addr, ip_addr, sizeof(ip_addr));
        fprintf(stderr, "IP: %s, PORT:%d\n", ip_addr, ntohs(addr.sin_port));

        char service_name[16] ={0};
        int ret = getnameinfo((struct sockaddr*)&addr, result->ai_addrlen, ip_addr, sizeof(ip_addr), 
                                                    service_name, sizeof(service_name), NI_NAMEREQD);

        fprintf(stderr, "HOST: %s, SERVICE: %s\n", ip_addr, service_name);
    }else if (result->ai_addrlen == sizeof(struct sockaddr_in6)){
        struct sockaddr_in6 addr = *(struct sockaddr_in6*)&result->ai_addr;
    }

    freeaddrinfo(result);
    return 0;
}
