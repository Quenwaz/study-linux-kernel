#include <unistd.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[])
{
	pid_t pidChild = -1;
	switch (pidChild = fork())
	{
	case -1:
		return EXIT_FAILURE;
	case 0:
		// child process
		printf("I'am child...%d.\n", getpid());
		break;
	default:
		printf("I'am father...%d.\n", getpid());
		break;
	}

	printf("%d I'm done.\n", getpid());
	
	return EXIT_SUCCESS;
}