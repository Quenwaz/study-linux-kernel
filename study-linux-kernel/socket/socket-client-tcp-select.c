
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdbool.h>



// inet_ntoa <=> inet_addr
// ntohs <=> htons

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: socket-client-tcp-select [ip] [port]\n");
		return EXIT_FAILURE;
	}

	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return EXIT_FAILURE;
	}

	
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(atoi(argv[2]));
	_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	if (-1 == connect(sock, (struct sockaddr*)&_addr, sizeof(_addr)))
	{
		fprintf(stderr, "Error: connect failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	signal(SIGPIPE, SIG_IGN);
	fcntl(sock, F_SETFL, O_NONBLOCK | fcntl(sock, F_GETFL));
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK | fcntl(STDIN_FILENO, F_GETFL));
	for (;;)
	{
		fd_set setrd, setwd;
		FD_ZERO(&setrd);
		FD_ZERO(&setwd);
		FD_SET(STDIN_FILENO, &setrd);
		FD_SET(sock, &setrd);
		FD_SET(sock, &setwd);
		int nSel = select(sock + 1, &setrd, &setwd, NULL, NULL);
		if (nSel <= 0){
			break;
		}
		
		bool bDisconnect = false;
		char szMsg[1024] = { 0 };
		ssize_t _n_read = 0;
		if (FD_ISSET(STDIN_FILENO, &setrd)){
			_n_read = read(STDIN_FILENO, szMsg, sizeof(szMsg));
			if (_n_read == 2 && szMsg[0] == 'q')
			{
				bDisconnect = true;
			}else if (_n_read > 0 && FD_ISSET(sock, &setwd) ){
				_n_read = send(sock, szMsg, _n_read, 0);
				if (_n_read == -1){
					fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
				}
			}
		}
						
		if (FD_ISSET(sock, &setrd)){
			memset(szMsg, 0, sizeof(szMsg));
			_n_read = recv(sock, szMsg, sizeof(szMsg), 0);
			if (_n_read <= 0)
			{
				fprintf(stderr, "Error: recv failed: %s\n", strerror(errno));
				bDisconnect = true;
			}else{
				fprintf(stderr, "recv %d bytes msg: %s\n", _n_read, szMsg);
			}
		}

		if (bDisconnect){
			fprintf(stderr, "\ngoodbye. %s\n", strerror(errno));
			break;
		}
	}

	close(sock);

	return EXIT_SUCCESS;
}
