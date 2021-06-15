#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> // for memset
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

enum {
	MSG_LEN = 1024
};

struct __msgbuf
{
    __syscall_slong_t mtype;	/* type of received/sent message */
    char mtext[MSG_LEN];		/* text of the message */
  };

int main(int argc, char* argv[])
{
	key_t key = ftok(argv[0], 1);
	int msgId = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
	pid_t pidChild = -1;
	switch (pidChild = fork())
	{
	case -1:
		return EXIT_FAILURE;
	case 0:	
	{
		msgId = msgget(key, IPC_EXCL | S_IRUSR | S_IWUSR);
		if (msgId == -1){
			printf("get msg failed\n");
			return EXIT_FAILURE;
		}
		printf("pid: %d, msg id :%d\n", getpid(), msgId);
		struct __msgbuf buf;
		memset(&buf, 0, sizeof(struct __msgbuf));
		const char* msg = "test msg queue";
		if (argc > 1) msg = argv[1];
		strncpy(buf.mtext, msg, strlen(msg));
		buf.mtype = 1024;
		msgsnd(msgId, &buf, sizeof(struct __msgbuf), 0);
		printf("pid: %d, send msg done\n", getpid());
	}
		
		exit(0);
	default:
		break;
	}	

	printf("msg key: %d\n", key);
	printf("pid: %d, msg id :%d\n", getpid(), msgId);
	struct __msgbuf msg;
	msg.mtype = 1024;
	// The call will block until the specified type of message is received.
	msgrcv(msgId,&msg, sizeof(msg), msg.mtype, 0);
	printf("pid: %d, recv msg:%s\n", getpid(), msg.mtext);

	// Delete message queue
	msgctl(msgId, IPC_RMID, NULL);
	return EXIT_SUCCESS;
}
