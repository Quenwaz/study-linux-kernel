#include <unistd.h>



int main(int argc, char const *argv[])
{
    write(STDOUT_FILENO, "fucku", 5);
    getchar();
    return 0;
}
