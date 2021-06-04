/**
 * @file tty_info.cpp
 * @author Quenwaz (404937333@qq.com)
 * @brief 终端信息
 * @version 0.1
 * @date 2021-06-02
 * 判断给定的文件描述符是否为终端， 是则打印其名称
 * @copyright Copyright (c) 2021 Quenwaz
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> // for atoi
#include "common.h"

boolean isdigit(const char *pszInput)
{
    if (pszInput == NULL)
    {
        return FALSE;
    }

    while (*pszInput != '\0' && *pszInput > '0' && *pszInput < '9')
        ++pszInput;

    return *pszInput == '\0';
}

int main(int argc, char const *argv[])
{
    if (argc != 2 || !isdigit(argv[1]))
    {
        fprintf(stderr, "Usage: %s [tty file descriptor]\n", argv[0]);
        return 1;
    }

    const int fd = atoi(argv[1]);
    const boolean istty = isatty(fd);
    fprintf(stderr, "[%s] is%s tty.\n", argv[1], istty ? "" : " not a");
    istty && fprintf(stderr, "and the name is: [%s]\n", ttyname(fd));
    return 0;
}
