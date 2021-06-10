#include <unistd.h>
#include <fcntl.h> // for open
#include <sys/stat.h>
#include <time.h> // clock
#include <stdio.h>
#include <math.h>
#include <alloca.h>
#include <stdlib.h>
#include <alloca.h>

/**
 * @brief Generate large files.
 * 
 * @param filepath file path.
 * @param filesize file size in MB.
 */
void mkbigfile(const char* filepath, ssize_t filesize)
{
    char command[512] ={0};
    snprintf(command, sizeof command, "dd if=/dev/zero of=%s bs=%dM count=1", filepath, filesize);
    FILE* fdRes = popen(command, "r");
    char info[1024] = {0};
    fread(info, sizeof info , 1, fdRes);
    write(STDERR_FILENO, info, sizeof info);
    pclose(fdRes);
}

int main(int argc, char const *argv[])
{
    const char* biffile = "./bin/big_100m.txt";
    mkbigfile(biffile, 100);
    int fd = open(biffile,O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP |S_IWGRP);
    if (fd == -1){
        return 1;
    }
    for (ssize_t i = 0;i <= 15; ++i)
    {
        lseek(fd, 0, SEEK_SET);
        int n = floor(pow(2,i));
        char* buf = (char*)alloca(n);
        clock_t begin = clock();
        while (0 < read(fd,buf, n));
        fprintf(stderr, "bufsize: %d, spend time: %f second\n",n,  (double)(clock() - begin) / CLOCKS_PER_SEC);
    }
    close(fd);
    remove(biffile);
    return 0;
}

/**
filesize is 1GB, the result is as follows:
bufsize: 4, spend time: 100.820000 second
bufsize: 8, spend time: 50.070000 second
bufsize: 16, spend time: 25.100000 second
bufsize: 32, spend time: 12.850000 second
bufsize: 64, spend time: 6.360000 second
bufsize: 128, spend time: 3.200000 second
bufsize: 256, spend time: 1.640000 second
bufsize: 512, spend time: 0.850000 second
bufsize: 1024, spend time: 0.460000 second
bufsize: 2048, spend time: 0.290000 second
bufsize: 4096, spend time: 0.180000 second 
*/