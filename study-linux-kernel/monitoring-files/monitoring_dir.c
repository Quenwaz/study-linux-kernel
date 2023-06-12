#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>



typedef void(*MatchedCall)(const char*, const char*);
static void matched_condition_call(const char* filename, const char* cmd)
{
    char command[128] ={0};
    sprintf(command, "%s ./%s", cmd != NULL ? cmd: "rm -f", filename);
    fprintf(stderr, "[matched] run `%s`\n", command);
    FILE *fp = popen(command, "r");
    if (fp == NULL){
        return;
    }

    char buffer[512] = {0};
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        printf("%s", buffer);
    }
    pclose(fp);
}


/* Read all available inotify events from the file descriptor 'fd'.
   wd is the table of watch descriptors for the directories in argv.
   argc is the length of wd and argv.
   argv is the list of watched directories.
   Entry 0 of wd and argv is unused. */

static void
handle_events(int fd, const char* folder, const char* filename, MatchedCall fnCall, const char* cmd)
{
    /* Some systems cannot read integer variables if they are not
       properly aligned. On other systems, incorrect alignment may
       decrease performance. Hence, the buffer used for reading from
       the inotify file descriptor should have the same alignment as
       struct inotify_event. */

    char buf[4096]
        __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    /* Loop while events can be read from inotify file descriptor. */

    for (;;)
    {
        /* Read some events. */
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* If the nonblocking read() found no events to read, then
           it returns -1 with errno set to EAGAIN. In that case,
           we exit the loop. */

        if (len <= 0)
            break;

        /* Loop over all events in the buffer. */
        for (char *ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event *)ptr;

            /* Print event type. */
            if (event->mask & IN_DELETE)
                printf("IN_DELETE: ");
            if (event->mask & IN_CREATE)
                printf("IN_CREATE: ");

            /* Print the name of the file. */
            // if (event->len)
            //     printf("%s", event->name);

            /* Print type of filesystem object. */

            if (!(event->mask & IN_ISDIR)){
                // printf(" [file]\n");
                if (strstr(event->name, filename) != NULL){
                    fnCall(event->name, cmd);
                }
            }
            // else{
            //     printf(" [directory]\n");
            // }
        }
    }
}


int main(int argc, char *argv[])
{
    char buf;
    int fd, i, poll_num;
    int wd;
    nfds_t nfds;
    struct pollfd fds[1];
    char* command;
    
    if (argc < 3)
    {
        printf("Usage: %s PATH FILENAME COMMAND\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc >= 3){
        command = argv[3];
    }else{
        command = NULL;
    }

    printf("Press ENTER key to terminate.\n");

    /* Create the file descriptor for accessing the inotify API. */

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
    {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }


    /* Mark directories for events
       - file was opened
       - file was closed */
    wd = inotify_add_watch(fd, argv[1],IN_DELETE | IN_CREATE | IN_MODIFY);
    if (wd == -1)
    {
        fprintf(stderr, "Cannot watch '%s': %s\n",
                argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Prepare for polling. */
    nfds = 1;

    // fds[0].fd = STDIN_FILENO; /* Console input */
    // fds[0].events = POLLIN;

    fds[0].fd = fd; /* Inotify input */
    fds[0].events = POLLIN;

    /* Wait for events and/or terminal input. */

    printf("watch %s\nListening for events.\n", argv[1]);
    while (1)
    {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (poll_num > 0)
        {

            // if (fds[0].revents & POLLIN)
            // {

            //     /* Console input is available. Empty stdin and quit. */

            //     while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
            //         continue;
            //     break;
            // }

            if (fds[0].revents & POLLIN)
            {

                /* Inotify events are available. */
                handle_events(fd, argv[1], argv[2], matched_condition_call, command);
            }
        }
    }

    printf("Listening for events stopped.\n");

    /* Close inotify file descriptor. */

    close(fd);

    exit(EXIT_SUCCESS);
}