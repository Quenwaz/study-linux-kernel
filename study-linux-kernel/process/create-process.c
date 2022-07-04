#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef PATH_MAX 
#define PATH_MAX 260
#endif



int main(int argc, char* argv[])
{
	FILE * fp = popen("/home/tnbs/dev-projects/study-linux-kernel/bin/static/test_process_path", "r");
	char msg[1024] = {0};
	fread(msg, sizeof msg, 1, fp);
	fprintf(stderr, msg);
	pclose(fp);
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