#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char const *argv[])
{
    char template[] = "/tmp/somestringXXXXXX";
    int fd = mkstemp(template);    
    if (fd == -1){
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Generated filename was: %s\n", template);
    char content[1024] = {0};
    sprintf(content, "%s", "fjdkalsdnvmjflkdja0efkdjladjf\n\
    fdjsalfjlakmvigtjiorhmmfptorperot456908035j32;flkmbdfaf\n\
    fdklagklgmgkofmge459uu586u0429j;mfl,crjetw['f]awe]ffjkgfds\n\
    ;fjkdajfksafjdkajskf;\n");
    write(fd, content, strlen(content));

    char cmd[64] = {0};
    sprintf(cmd, "cat -n %s", template);
    FILE* fp = popen(cmd, "r");
    if (fp != NULL){
        char data[128] = {0};
        while (fgets(data, sizeof(data), fp) != NULL)
        {
            fprintf(stderr, "%s", data);
        }
    }

    unlink(template);
    close(fd);
    return 0;
}
