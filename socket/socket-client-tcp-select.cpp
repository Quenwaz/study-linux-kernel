
#include <memory>
#include<unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

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
	
	for (;;)
	{
		fprintf(stderr, "\n>:");
		char szMsg[1024] = { 0 };
		ssize_t _n_read = read(STDIN_FILENO, szMsg, sizeof(szMsg));
		if (_n_read <= 0)
		{
			continue;
		}

		if (_n_read == 2 && szMsg[0] == 'q')
		{

			fprintf(stderr, "\ngoodbye.");
			break;
		}
		
		_n_read = send(sock, szMsg, strlen(szMsg), 0);
		if (_n_read == -1)
		{
			fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
			break;
		}
		
		fprintf(stderr, "send %d bytes msg: %s\n", _n_read, szMsg);
		memset(szMsg, 0, _n_read);
		_n_read = recv(sock, szMsg, sizeof(szMsg), 0);
		if (_n_read == -1)
		{
			fprintf(stderr, "Error: recv failed: %s\n", strerror(errno));
			continue;
		}

		fprintf(stderr, "recv %d bytes msg: %s\n", _n_read, szMsg);
	}

	close(sock);

	return EXIT_SUCCESS;
}
