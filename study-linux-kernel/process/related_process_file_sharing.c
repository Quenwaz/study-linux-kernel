#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);
    int fd = mkstemp();
    return 0;
}
