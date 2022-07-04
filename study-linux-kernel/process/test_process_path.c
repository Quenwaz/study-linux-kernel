#define _GNU_SOURCE      
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PATH_MAX 
#define PATH_MAX 260
#endif

char* GetExePath()
{
    static char result[PATH_MAX] = {0};
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    char* sep = strrchr(result, '/');
    if (sep != NULL) sep[0] = 0;
    return result;
}


int main(int argc, char* argv[])
{
	fprintf(stderr, "exepath: %s\n", GetExePath());
	return EXIT_SUCCESS;
}