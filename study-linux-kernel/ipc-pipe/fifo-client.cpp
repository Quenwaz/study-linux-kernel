#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "fifo-common.h"

namespace 
{
	enum { MAX_PATH = 260 };
	char g_client_pipe_path[MAX_PATH] = { 0 };
}

static void             /* Invoked on exit to delete client FIFO */
removeFifo(void)
{
	unlink(g_client_pipe_path);
}

int main(int argc, char* argv[])
{
	if (argc != 2 || strcmp(argv[1], "--help") == 0)
	{
		fprintf(stderr, "%s MSG\n", argv[0]);
		return EXIT_FAILURE;
	}

	
	umask(0);
	
	const char* MSG = argv[1];

	snprintf(g_client_pipe_path, MAX_PATH, "%s-%ld", CLIENT_PIPE_PATH,getpid());

	if (mkfifo(g_client_pipe_path, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
	{
		fprintf(stderr, "mkfifo %s failed...\n", g_client_pipe_path);
		return EXIT_FAILURE;
	}

	if (atexit(removeFifo) != 0)
	{
		return EXIT_FAILURE;
	}


	const int server_fd = open(SERVER_PIPE_PATH, O_WRONLY);
	if (server_fd == -1)
	{
		fprintf(stderr, "open %s failed...\n", SERVER_PIPE_PATH);
		return EXIT_FAILURE;
	}

	struct request req;
	req.pid = getpid();
	strncpy(req.msg, MSG, MSG_LEN);
	if (write(server_fd, &req, sizeof(struct request)) != sizeof(struct request))
	{
		fprintf(stderr, "Error writing request; discarding...\n");
		return EXIT_FAILURE;
	}

	if (close(server_fd) == -1)
	{
		fprintf(stderr, "close FIFO %s failed...\n", SERVER_PIPE_PATH);
	}

	int client_fd = open(g_client_pipe_path, O_RDONLY);
	if (client_fd == -1)
	{
		fprintf(stderr, "open %s failed...\n", g_client_pipe_path);
		return EXIT_FAILURE;
	}

	struct response resp;
	if (read(client_fd, &resp, sizeof(struct response)) != sizeof(struct response))
	{
		fprintf(stderr, "Error writing to FIFO %s\n", g_client_pipe_path);
	}
	else
	{
		fprintf(stdout, "%s\n", resp.msg);
	}
	
	if (close(client_fd) == -1)
	{
		fprintf(stderr, "close FIFO %s failed...\n", g_client_pipe_path);
	}
	

	return EXIT_SUCCESS;
}


