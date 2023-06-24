#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include "dll/common_hashtable_dll.h"


const int MAX_MSG_LEN = 1024;
const int MAX_CLIENTS = 100;

int main(int argc, char *argv[]) 
{
    hash_table_t hashtable;
    hash_table_init(&hashtable);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    fprintf(stderr, "Chat server started on port %d\n", port);

    char msg[MAX_MSG_LEN];
    socklen_t client_len = sizeof(client_addr);

    int client_fds[MAX_CLIENTS];
    int num_clients = 0;
    memset(client_fds, 0, sizeof(client_fds));
    fd_set read_fds, write_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(sock_fd, &read_fds);

    while (1) {
        fd_set tmp_read_fds = read_fds;
        fd_set tmp_write_fds = write_fds;
        int max_fd = sock_fd;

        for (int i = 0; i < num_clients; i++) {
            FD_SET(client_fds[i], &tmp_read_fds);
            if (client_fds[i] > max_fd) {
                max_fd = client_fds[i];
            }
        }

        if (select(max_fd + 1, &tmp_read_fds, &tmp_write_fds, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(sock_fd, &tmp_read_fds)) {
            int len = recvfrom(sock_fd, msg, MAX_MSG_LEN, 0, (struct sockaddr*)&client_addr, &client_len);
            if (len == -1) {
                perror("recvfrom");
                exit(1);
            }
            msg[len] = '\0';
            // fprintf(stderr, "Received message: %s\n", msg);
            
            char* recv_message_ip = inet_ntoa(client_addr.sin_addr);
            {
                char client_addr_ip[32]={0};
                snprintf(client_addr_ip, sizeof client_addr_ip, "%s:%d", recv_message_ip, ntohs(client_addr.sin_port));
                if (num_clients < MAX_CLIENTS && hash_table_get(&hashtable,client_addr_ip) == NULL) {
                    int sock = socket(AF_INET, SOCK_DGRAM, 0);
                    hash_table_put(&hashtable,client_addr_ip,(void*)(intptr_t)sock);
                    if (sock == -1) {
                        perror("socket");
                        exit(1);
                    }
                    client_fds[num_clients] = sock;
                    if (connect(client_fds[num_clients], (struct sockaddr*)&client_addr, client_len) == -1) {
                        perror("connect");
                        exit(1);
                    }
                    num_clients++;
                }
            }
            char message[2048] = {0};
            strncpy(message, recv_message_ip, strlen(recv_message_ip));
            strncat(message, "> ", 2);
            strncat(message, msg, len);
            for(void* iter = hash_table_begin(&hashtable); !hash_table_end(&hashtable,iter); hash_table_next(&hashtable, iter))
            {
                char client_ip[MAX_KEY_LEN] = {0};
                intptr_t client_fd = (intptr_t)hash_table_current_value(&hashtable,iter,client_ip);
                char* port = strrchr(client_ip, ':');
                struct sockaddr_in temp_addr;
                memcpy(&temp_addr, &client_addr, sizeof(temp_addr));
                *port = '\0';
                temp_addr.sin_addr.s_addr = inet_addr(client_ip);
                ++port;
                temp_addr.sin_port = htons(atoi(port));
                socklen_t temp_addr_len = sizeof(temp_addr);
                int n = sendto(client_fd, message, strlen(message), 0, (struct sockaddr*)&temp_addr, temp_addr_len);
                if (n == -1) {
                    perror("sendto");
                    exit(1);
                }
            }
        }

        for (int i = 0; i < num_clients; i++) {
            if (FD_ISSET(client_fds[i], &tmp_read_fds)) {
                int len = recv(client_fds[i], msg, MAX_MSG_LEN, 0);
                if (len == -1) {
                    perror("recv");
                    exit(1);
                }
                msg[len] = '\0';
                fprintf(stderr, "Received message from client: %s\n", msg);
            }
        }
    }

    return 0;
}