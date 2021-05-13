// #define _BSD_SOURCE
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
// inet_ntoa <=> inet_addr
// ntohs <=> htons


void* _thread_func(void * pData)
{
	int sock = *(int*)(pData);
	for(;;)
	{
		char szMsg[1024] = { 0 };
		ssize_t _n_read = recv(sock, szMsg, sizeof(szMsg), 0);
		if (_n_read == -1)
		{
			fprintf(stderr, "Error: recv failed: %s\n", strerror(errno));
			break;
		}
		fprintf(stderr, "recv %d bytes msg: %s\n", _n_read, szMsg);
		fprintf(stderr, "\n>:");

		memset(szMsg, 0, _n_read);
		_n_read = read(STDIN_FILENO, szMsg, sizeof(szMsg));
		if (_n_read <= 0)
		{
			continue;
		}

		if (_n_read == 2 && szMsg[0] == 'q')
		{
			break;
		}

		// 使用MSG_NOSIGNAL flag 避免对端socket关闭后再发送导致进程kill
		_n_read = send(sock, szMsg, strlen(szMsg), 0);
		if (_n_read == -1)
		{
			fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
			break;
		}

		fprintf(stderr, "[%lu] send %d bytes msg: %s\n", pthread_self(), _n_read, szMsg);
	}

	fprintf(stderr, "\n[%lu] goodbye\n", pthread_self());
	close(sock);
	return NULL;
}

#include <netdb.h>
std::string GetHostInfo()
{
	char szHost[128] = { 0 };
	gethostname(szHost, sizeof(szHost));


	hostent* hostInfo = gethostbyname(szHost);
	return std::string(inet_ntoa(*((struct in_addr*)hostInfo->h_addr_list[0])));
}
#include <signal.h>
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: socket-server-tcp [port] [backlog:options,default 4]\n");
		return EXIT_FAILURE;
	}


	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return EXIT_FAILURE;
	}


	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(atoi(argv[1]));
	_addr.sin_addr.s_addr = INADDR_ANY; // inet_addr(argv[1]);
	if (-1 == bind(sock, (struct sockaddr*)&_addr, sizeof(_addr)))
	{
		fprintf(stderr, "Error: bind failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	const int backlog = (argc == 3) ? atoi(argv[2]) : 4;
	if(-1 == listen(sock, backlog))
	{
		fprintf(stderr, "Error: listen failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// 忽略SIGPIPE信号， 避免对端socket关闭后， 触发此信号导致进程终止
	signal(SIGPIPE, SIG_IGN);
	
	fprintf(stderr, "listen to: 0.0.0.0:%s\n", argv[1]);
	for (;;)
	{
		struct sockaddr_in addr_client;
		socklen_t _n_addr_client = 0;
		int sock_client = accept(sock, (struct sockaddr*)&addr_client, &_n_addr_client);
		if (-1 == sock_client)
		{
			continue;
		}
		
		fprintf(stderr, "accept %s connect\n", inet_ntoa(addr_client.sin_addr));
		
		pthread_t thread;
		if( 0 > pthread_create(&thread, NULL, _thread_func, &sock_client))
		{
			
			continue;
		}

		pthread_detach(thread);
	}

	close(sock);

	return EXIT_SUCCESS;
}
