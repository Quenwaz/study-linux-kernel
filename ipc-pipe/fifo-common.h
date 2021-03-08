#pragma once
#include <sys/types.h>

const char* const SERVER_PIPE_PATH = "./fifo-server-pipe";
const char* const CLIENT_PIPE_PATH = "./fifo-client-pipe";


enum
{
    MSG_LEN = 16
};

struct request {                /* Request (client --> server) */
    pid_t pid;                  
    char msg[MSG_LEN];
};

struct response {               /* Response (server --> client) */
    char msg[MSG_LEN];
};