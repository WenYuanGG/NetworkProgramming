#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

void PANIC(char* msg);

#define PANIC(msg){perror(msg); exit(-1);}

void http(struct sockaddr_in *client_addr){
	char buffer[1512];
	char port[10];

	char *argv[3] = {"./cgi", "./cgi", 0};

	read(0, buffer, sizeof(buffer));
	//printf("%s\n", buffer);

	setenv("REMOTE_ADDR", inet_ntoa(client_addr->sin_addr), 1);
	sprintf(port, "%d", ntohs(client_addr->sin_port));
	setenv("REMOTE_PORT", port, 1);

	if(!fork())
		execvp(argv[0], argv);
}

int main(int argc, char *argv[]){
	int sd;
	int port;
	int yes = 1;
	int new_fd;
	socklen_t len;
	struct sockaddr_in server_addr, client_addr;

	struct sigaction action1;
	action1.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action1, 0);

	struct sigaction action2;
	action2.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &action2, 0);

	if(argc > 1) port = atoi(argv[1]);
	else port = 80;

	sd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));

	// Let address can be reused
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		PANIC("setsockopt");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
		PANIC("bind");
	if(listen(sd, 20) != 0)
		PANIC("listen");

	while(1){
		len = sizeof(client_addr);

		new_fd = accept(sd, (struct sockaddr*)&client_addr, &len);
		printf("%d, Client from %s:%d\n", new_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		if(new_fd < 0)
			perror("accept");
		else{
			if(!fork()){
				dup2(new_fd, 0);
				dup2(new_fd, 1);
				dup2(new_fd, 2);
				close(new_fd);

				http(&client_addr);

				exit(0);
			}
			close(new_fd);
		}
	}
	close(sd);
	return 0;
}
