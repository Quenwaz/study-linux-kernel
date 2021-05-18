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
#include <fcntl.h>
#include <sys/select.h>
#include <set>
#include <deque>
#include <utility>
#include <netdb.h>
#include <signal.h>

// inet_ntoa <=> inet_addr
// ntohs <=> htons

pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;

void* _thread_func(void * pData)
{
	std::set<int> * pSetFdAccepted = (std::set<int>*)(pData);
	std::deque<std::pair<int, std::string>> queMsgs;
	for(;;)
	{
		if (pSetFdAccepted->empty()){
			sleep(0);
			continue;
		}

		fd_set setrd, setwd;
		FD_ZERO(&setrd);
		FD_ZERO(&setwd);

		int maxfd = STDIN_FILENO;
		FD_SET(STDIN_FILENO, &setrd);
		for (auto iter = pSetFdAccepted->begin(); iter != pSetFdAccepted->end(); ++iter){
			FD_SET(*iter, &setrd);
			FD_SET(*iter, &setwd);
			if (*iter > maxfd){
				maxfd = *iter + 1;
			}
		}

		int ret = select(maxfd, &setrd, &setwd, NULL, NULL);
		if (ret <= 0){
			continue;
		}


		if (FD_ISSET(STDIN_FILENO, &setrd)){
			queMsgs.push_back(std::make_pair(pSetFdAccepted->size(), ""));
			queMsgs.back().second.resize(1024, 0);
			read(STDIN_FILENO, &queMsgs.back().second[0], 1024);
		}


		pthread_mutex_lock(&g_mtx);
		for (auto iter = pSetFdAccepted->begin(); iter != pSetFdAccepted->end(); ){
			bool bClosed = false;
			if (FD_ISSET(*iter, &setwd))
			{
				for (int i =queMsgs.size() - 1;i >= 0; --i)
				{
					if (queMsgs.at(i).first <= 0){
						continue;
					}
					
					auto msg = queMsgs.at(i).second.c_str();
					ssize_t _n_read = send(*iter, msg, strlen(msg), 0);
					if (_n_read == -1)
					{
						fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
						bClosed = true;
						break;
					}
					--queMsgs.at(i).first;
					fprintf(stderr, "send %d bytes msg: %s\n", _n_read, msg);
				}
			}


			if (FD_ISSET(*iter, &setrd)){
				char msg[1024]= {0};
				ssize_t _n_read = recv(*iter, msg, sizeof(msg), 0);
				if (_n_read <= 0)
				{
					fprintf(stderr, "Error: recv failed: %s\n", strerror(errno));
					bClosed = true;		
				}else{
					fprintf(stderr, "recv msg %d bytes: %s\n", _n_read, msg);
				}
			}

						
			if (bClosed){
				close(*iter);
				iter = pSetFdAccepted->erase(iter);
			}else ++iter;
		}
		pthread_mutex_unlock(&g_mtx);

		for (auto iter = queMsgs.begin(); iter != queMsgs.end();)
		{
			if (iter->first <= 0){
				iter = queMsgs.erase(iter);
			}else ++iter;
		}
		
	}

	fprintf(stderr, "\n[%lu] goodbye\n", pthread_self());
	return NULL;
}


std::string GetHostInfo()
{
	char szHost[128] = { 0 };
	gethostname(szHost, sizeof(szHost));


	hostent* hostInfo = gethostbyname(szHost);
	return std::string(inet_ntoa(*((struct in_addr*)hostInfo->h_addr_list[0])));
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: socket-server-tcp-select [port] [backlog:options,default 4]\n");
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
	
	
	fcntl(STDIN_FILENO,F_SETFL,  fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
	fprintf(stderr, "listen to: 0.0.0.0:%s\n", argv[1]);

	std::set<int> setFdAccepted;
	pthread_t thread;
	if( 0 > pthread_create(&thread, NULL, _thread_func, &setFdAccepted))
	{
		fprintf(stderr, "Error: pthread_create failed: %s\n", strerror(errno));
	}
	else {
		for (;;)
		{
			struct sockaddr_in addr_client;
			socklen_t _n_addr_client = 0;
			const int sock_client = accept(sock, (struct sockaddr*)&addr_client, &_n_addr_client);
			if (-1 == sock_client)
			{
				continue;
			}
			
			fcntl(sock_client,F_SETFL,  fcntl(sock_client, F_GETFL) | O_NONBLOCK);
			pthread_mutex_lock(&g_mtx);
			setFdAccepted.insert(sock_client);
			pthread_mutex_unlock(&g_mtx);

			fprintf(stderr, "accept %s connect\n", inet_ntoa(addr_client.sin_addr));
		}
		pthread_detach(thread);
	}

	close(sock);

	return EXIT_SUCCESS;
}

