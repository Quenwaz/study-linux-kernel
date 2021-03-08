#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define BUF_SIZE 32

int main(int argc, char* argv[])
{
	if (argc !=2 || strcmp(argv[1], "--help") == 0)
	{
		fprintf(stderr, "%s string\n", argv[0]);
		return EXIT_FAILURE;
	}

	int pfd[2] = { -1, -1 };
	if (pipe(pfd) == -1)
	{
		fprintf(stderr, "mkpipe failure...\n");
		return EXIT_FAILURE;
	}

	switch (fork())
	{
	case -1:
		fprintf(stderr, "fork failure...\n");
		return EXIT_FAILURE;
	case 0:
		if (close(pfd[1]) == -1)
		{
			fprintf(stderr, "child close pipe write end failure...\n");
			return EXIT_FAILURE;
		}

		for (;;)
		{
			char buf[BUF_SIZE] = { 0 };
			ssize_t numRead = read(pfd[0], buf, BUF_SIZE);
			if (numRead == -1)
			{
				fprintf(stderr, "child pipe read failure....");
				return EXIT_FAILURE;
			}

			if (numRead == 0)
			{
				break;
			}

			
			fprintf(stderr, "child recv %d byte of data from the pipe.\n", numRead);
			if (write(STDOUT_FILENO, buf, numRead) != numRead)
			{
				fprintf(stderr, "child pipe write failure...\n");
				return EXIT_FAILURE;
			}
		}

		write(STDOUT_FILENO, "\n", 1);
		if (close(pfd[0]) == -1)
		{
			fprintf(stderr, "child close pipe read end failure...\n");
			return EXIT_FAILURE;
		}
		fprintf(stderr, "child it's done...\n");
		exit(EXIT_SUCCESS);
		break;
	default:
		if(close(pfd[0]) == -1)
		{
			fprintf(stderr, "parent close pipe read end failure...\n");
			return EXIT_FAILURE;
		}
		
		if (write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1]))
		{
			fprintf(stderr, "parent write failed...\n");
			return EXIT_FAILURE;
		}

		if (close(pfd[1]) == -1)
		{
			fprintf(stderr, "parent close pipe write end failed...\n");
			return EXIT_FAILURE;
		}

		wait(NULL);
		fprintf(stderr, "parent it's done...\n");
		return EXIT_SUCCESS;
	}

	return 0;
}

