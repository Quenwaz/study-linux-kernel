#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char*
currTime(const char* format)
{
	enum
	{
		BUF_SIZE = 2014
	};
	static char buf[BUF_SIZE]; /* Nonreentrant */
	time_t t;
	size_t s;
	struct tm* tm;

	t = time(nullptr);
	tm = localtime(&t);
	if (tm == nullptr)
		return nullptr;

	s = strftime(buf, BUF_SIZE, (format != nullptr) ? format : "%c", tm);

	return (s == 0) ? nullptr : buf;
}


int main(int argc, char* argv[])
{
	if (argc < 2 || strcmp(argv[1], "--help") == 0)
	{
		fprintf(stderr, "%s sleep-time...\n", argv[0]);
		return EXIT_FAILURE;
	}

	setbuf(stdout, nullptr); // Do not buffer stdout, output immediately

	int pfd[2] = {-1, -1};
	if (pipe(pfd) == -1)
	{
		fprintf(stderr, "mkpipe failure...\n");
		return EXIT_FAILURE;
	}

	for (int i = 1; i < argc; ++i)
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


			sleep(atoi(argv[i]));
			fprintf(stderr, "%s child %d (PID=%ld) closing pipe\n", currTime("%T"), i, getpid());

			if (close(pfd[1]) == -1)
			{
				fprintf(stderr, "child close pipe write end failure...\n");
				return EXIT_FAILURE;
			}

			fprintf(stderr, "child it's done...\n");
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
		}

	if (close(pfd[1]) == -1)
	{
		fprintf(stderr, "parent close pipe write end failure...\n");
		return EXIT_FAILURE;
	}

	char dummy = '0';
	if (read(pfd[0], &dummy, 1) != 0)
	{
		fprintf(stderr, "parent read failed...\n");
		return EXIT_FAILURE;
	}

	if (close(pfd[0]) == -1)
	{
		fprintf(stderr, "parent close pipe read end failed...\n");
		return EXIT_FAILURE;
	}

	fprintf(stderr, "%s parent ready to go...\n", currTime("%T"));
	return EXIT_SUCCESS;
}
