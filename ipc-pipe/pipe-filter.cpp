

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char*
currTime(const char* format)
{
	enum
	{
		BUF_SIZE = 2014
	};
	static char buf[BUF_SIZE];  /* Nonreentrant */
	time_t t;
	size_t s;
	struct tm* tm;

	t = time(NULL);
	tm = localtime(&t);
	if (tm == NULL)
		return NULL;

	s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);

	return (s == 0) ? NULL : buf;
}



int main(int argc, char* argv[])
{
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
		if (close(pfd[0]) == -1)
		{
			fprintf(stderr, "child close pipe read end failure...\n");
			return EXIT_FAILURE;
		}

		if (pfd[1] != STDOUT_FILENO)
		{
			if (dup2(pfd[1], STDOUT_FILENO) == -1)
			{
				fprintf(stderr, "child bound write end of pipe to stdout failed...\n");
			}

			if (close(pfd[1]) == -1)
			{
				fprintf(stderr, "child close pipe write end failure...\n");
				return EXIT_FAILURE;
			}
		}
		
		execlp("ls", "ls", NULL);
		fprintf(stderr, "child it's done...\n");
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
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

		if (pfd[0] != STDIN_FILENO)
		{
			if (dup2(pfd[0], STDIN_FILENO) == -1)
			{
				fprintf(stderr, "child bound read end of pipe to stdin failed...\n");
			}

			if (close(pfd[0]) == -1)
			{
				fprintf(stderr, "child close pipe read end failure...\n");
				return EXIT_FAILURE;
			}
		}

		execlp("wc", "wc", "-l", NULL);
		fprintf(stderr, "child it's done...\n");
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	if (close(pfd[0]) == -1)
	{
		fprintf(stderr, "parent close pipe read end failure...\n");
		return EXIT_FAILURE;
	}

	if (close(pfd[1]) == -1)
	{
		fprintf(stderr, "parent close pipe write end failure...\n");
		return EXIT_FAILURE;
	}

	wait(NULL);

	fprintf(stderr, "parent it's done...\n");
	return EXIT_SUCCESS;
}

