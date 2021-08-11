#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

extern char ** environ;

int main(int argc, char const *argv[])
{
    putenv("quenwaz=zhangkunhua");
    for (char** ep = environ; *ep != NULL; ++ep){
        puts(*ep);
    }
    char* val = getenv("quenwaz");
    setenv("quenwaz","123456", 1);
    val = getenv("quenwaz");
    unsetenv("quenwaz");
    val = getenv("quenwaz");
    return 0;
}
