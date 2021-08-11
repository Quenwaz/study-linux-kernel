#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>


static jmp_buf env;

static void f2(void)
{
    longjmp(env, 2);
}

static void f1(int argc)
{
    if (argc == 1){
        longjmp(env,1);
    }
    f2();
}

int main(int argc, char const *argv[])
{
    switch (setjmp(env))
    {
    case 0:
        printf("Calling f1() after initial setjmp()\n");
        f1(argc);
        break;
    case 1:
        printf("We jumped back from f1()\n");
        break;
    case 2:
        printf("We jumped back from f2()\n");
        break;
    
    default:
        break;
    }
    return 0;
}
