#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "fifo-common.h"

int main(int argc, char* argv[])
{
	// if (argc != 3 || strcmp(argv[1], "--help") == 0)
	// {
	// 	fprintf(stderr, "%s SERVER_FIFO CLIENT_FIFO-PID\n", argv[0]);
	// 	return EXIT_FAILURE;
	// }

	umask(0);
	if (mkfifo(SERVER_PIPE_PATH, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
	{
		fprintf(stderr, "mkfifo %s failed...\n", SERVER_PIPE_PATH);
		return EXIT_FAILURE;
	}

	const int server_fd = open(SERVER_PIPE_PATH, O_RDONLY);
	if (server_fd == -1 )
	{
		fprintf(stderr, "open %s failed...\n", SERVER_PIPE_PATH);
		return EXIT_FAILURE;
	}

	/* Open an extra write descriptor, so that we never see EOF */
	open(SERVER_PIPE_PATH, O_WRONLY);
	if (server_fd == -1)
	{
		fprintf(stderr, "open %s failed...\n", SERVER_PIPE_PATH);
		return EXIT_FAILURE;
	}

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		fprintf(stderr, "signal SIGPIPE to SIG_IGN failed...\n");
		return EXIT_FAILURE;
	}


	enum { MAX_PATH = 260 };
	for (;;)
	{
		struct request req;
		if (read(server_fd, &req, sizeof(struct request)) != sizeof(struct request))
		{
			fprintf(stderr, "Error reading request; discarding...\n");
			continue;
		}


		char client_pipe_path[MAX_PATH] = { 0 };
		snprintf(client_pipe_path, MAX_PATH, "%s-%ld", CLIENT_PIPE_PATH, req.pid);
		int client_fd = open(client_pipe_path, O_WRONLY);
		if (client_fd == -1)
		{
			fprintf(stderr, "open %s failed...\n", client_pipe_path);
			continue;
		}
		
		struct response resp;
		snprintf(resp.msg, MSG_LEN, "%s", req.msg);
		if (write(client_fd, &resp, sizeof(struct response)) != sizeof(struct response))
		{
			fprintf(stderr, "Error writing to FIFO %s\n", client_pipe_path);
		}

		if (close(client_fd) == -1)
		{
			fprintf(stderr, "close FIFO %s failed...\n", client_pipe_path);
		}
	}
	
	return EXIT_SUCCESS;
}


