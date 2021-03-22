#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>


int main(int argc, char* argv[])
{

	struct sockaddr_un addr;

	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1) {
		return 1;
	}

	const char* SV_SOCK_PATH = "/tmp/us_xfr";

	printf("hello socket\n");
	return 0;
}
