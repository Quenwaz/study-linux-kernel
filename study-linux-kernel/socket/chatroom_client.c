#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

const int MAX_MSG_LEN = 1024;

int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n",argv[0] );
        exit(1);
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

	fcntl(sock_fd, F_SETFL, O_NONBLOCK | fcntl(sock_fd, F_GETFL));
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK | fcntl(STDIN_FILENO, F_GETFL));

    while (1) {
		fd_set setrd, setwd;
		FD_ZERO(&setrd);
		FD_ZERO(&setwd);
		FD_SET(STDIN_FILENO, &setrd);
		FD_SET(sock_fd, &setrd);
		FD_SET(sock_fd, &setwd);
		int nSel = select(sock_fd + 1, &setrd, &setwd, NULL, NULL);
		if (nSel <= 0){
			break;
		}

        char msg[MAX_MSG_LEN];
        // fprintf(stderr, "Enter message to send: ");
		if (FD_ISSET(STDIN_FILENO, &setrd)){
            fgets(msg, MAX_MSG_LEN, stdin);
            int len = strlen(msg);
            msg[len - 1] = '\0';

            int n = sendto(sock_fd, msg, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if (n == -1) {
                perror("sendto");
                exit(1);
            }

            // fprintf(stderr, "Sent message: %s\n",msg);
        }

        if (FD_ISSET(sock_fd, &setrd)){
            int recv_len = recv(sock_fd, msg, MAX_MSG_LEN, 0);
            if (recv_len == -1) {
                perror("recv");
                exit(1);
            }
            msg[recv_len] = '\0';
            fprintf(stderr, "%s\n",msg);
        }
    }

    return 0;
}