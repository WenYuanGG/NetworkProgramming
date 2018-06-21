#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define PANIC(msg) { perror(msg); exit(-1); }
#define STDIN 0
#define buffersize 1024

char recv_buf[buffersize];
char send_buf[buffersize];

void *read_data(void* pthreadid){
	int recvbytes;
	int sockfd = *(int*)pthreadid;
	while(1){
		if((recvbytes = recv(sockfd, recv_buf, buffersize, 0)) == -1){
			PANIC("recv");
		}
		else{
			recv_buf[recvbytes] = '\0';
			write(1, recv_buf, sizeof(recv_buf));
		}
	}
}

int main(int argc, char *argv[]){
	pthread_t child;
	int sockfd;
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9999);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		PANIC("socket");

	if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
		PANIC("connect")

	pthread_create(&child, NULL, read_data, (void*)&sockfd);

	while(1){
		memset(send_buf, 0, buffersize);
		fgets(send_buf, buffersize, stdin);

		if(send(sockfd, send_buf, buffersize, 0) == -1)
			PANIC("send");
	}

	close(sockfd);
	pthread_cancel(child);
	return EXIT_SUCCESS;
}