#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int
main(int argc, char **argv)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return 1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	struct sockaddr_in src;
	src.sin_addr.s_addr = inet_addr("127.0.0.1");
	src.sin_family = AF_INET;
	src.sin_port = htons(atoi(argv[1]));
	int err = bind(sockfd, &src, sizeof(src));
	if (err != 0) {
		perror("bind");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));

	int connfd = connect(sockfd, &addr, sizeof(addr));
	if (connfd < 0) {
		perror("connect");
		return 1;
	}

	printf("success\n");

	sleep(10000);

	return 0;
}
