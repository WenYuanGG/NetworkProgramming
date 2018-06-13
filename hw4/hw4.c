#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PANIC(msg) { perror(msg); exit(-1); }
#define STDIN 0

int main(int argc, char *argv[]){
	int server_fd; 
	int client_fd; 
	struct sockaddr_in server_addr; 
	struct sockaddr_in client_addr; 
	fd_set master_fds; 
	fd_set read_fds; 
	int fdmax; 
	struct timeval tv; 
	int port;
	int len;
	int i, j;
	char buf[256]; 
	int nbytes;
	int yes = 1;
	int retval;

	// Used to handle signal pipe
	struct sigaction action;
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, 0);
	
	// Set port number
	if(argc > 1) port = atoi(argv[1]);
	else port = 9999;

	// Clean up fd set
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);

	// Create a socket (IPv4, TCP)
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		PANIC("socket");

	// Let address can be reused
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		PANIC("setsockopt");

	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
	server_addr.sin_port = htons(port); // Set port number

	// Bind server_fd with specified IP and Port
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		PANIC("bind");

	if (listen(server_fd, 10) == -1) PANIC("listen");

	FD_SET(server_fd, &master_fds); // Put server_fd into master_fds
	FD_SET(STDIN, &master_fds); //Put STDIN into master_fds
	fdmax = server_fd;

	// Server Loop
	for(;;) {
		// Set the time of timeout to 60s
		tv.tv_sec = 60; 
		tv.tv_usec = 0;

		// Copy master_fds to read_fds
		read_fds = master_fds;

		// Use select to monitor many fd at the same time
		retval = select(fdmax+1, &read_fds, NULL, NULL, &tv);

		switch(retval){
			// When error occur 
			case -1:
				perror("select");
				continue;

			// When time out
			case 0:
				printf("Time Out...\n");
				for(i=3; i<= fdmax; i++)
					if (FD_ISSET(i,&master_fds))
						// Close all fd in master_fds, except to stdin, stdout, and stderr
						close(i);
				exit(0); 
		}

		for(i=0; i<=fdmax; i++) {
			if (FD_ISSET(i,&read_fds)) { 

				// Handle new connection
				if (i==server_fd) {
				//當i等於server_fd時代表有client要連線
					len = sizeof(client_addr);
					if ((client_fd=accept(server_fd,(struct sockaddr *)&client_addr,(socklen_t*)&len)) == -1) {
					//接受client端的連線，並回傳一個client_fd
						perror("accept");
						continue;
					} 

					else {
						FD_SET(client_fd, &master_fds); //把client_fd加入集合
						if (client_fd > fdmax) {
						//經過比較後，找出目前最大的fd值
							fdmax = client_fd;
						}
						printf("New connection from %s on socket %d\n",inet_ntoa(client_addr.sin_addr),client_fd);
					}
				}

				else { // Handle data from the clients and the STDIN
				//其餘的fd不是STDIN不然就是來自client

					// Read Data into buf
					if ((nbytes=read(i, buf, sizeof(buf))) > 0) {
						write(0,buf,nbytes); //再把buf的資料寫到console
						for(j=0; j<=fdmax; j++) {
							if (FD_ISSET(j, &master_fds))
								if (j!=server_fd && j!=i &&j!=0) 
								//除了自己、STDIN、送資料的client以外
								//其他的client都要送一份
									if (send(j, buf, nbytes, 0) == -1)
										perror("send");
						}//for
					} 

					else { 
						perror("recv");
						close(i); 
						FD_CLR(i, &master_fds); 
					}
				}
			}//if
		} //for scan all IO
	}//for server loop
	return 0;
}
