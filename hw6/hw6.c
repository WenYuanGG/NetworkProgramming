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
#include <fcntl.h>
#include <sys/stat.h> 

void PANIC(char* msg);
#define PANIC(msg){perror(msg); exit(-1);}

#define BUFF_SIZE 2048
#define WorkingPath "./www/"

#define SupportFileType	\
{".html",".HTML",".htm",".HTM",".png",".PNG",".jpeg",".JPEG",".jpg",".JPG",".gif",".GIF"}

#define SupportResponeMessage	\
{"text/html","text/html","text/html","text/html","image/png","image/png","image/jpeg","image/jpeg"	\
"image/jpeg","image/jpeg","image/gif","image/gif"}

char *FileTypeList[] = SupportFileType;
char *ResponeMessageList[] = SupportResponeMessage;
char NowWorkingPath[BUFF_SIZE];

char* Get_ResponeFileType(char *FileType){
	char *p = NULL;
	int i;
	for(i=0;i<12;i++)
		if(!strcmp(FileType,FileTypeList[i]))
			p = ResponeMessageList[i];
	return p;
}

void http(struct sockaddr_in *client_addr){
	char recvBuff[BUFF_SIZE];
	int len;
	char *const parmList[] = {"/bin/ls", "-l", NULL, NULL};

	if((len = recv(0, recvBuff , BUFF_SIZE, 0)) > 0 ){
		char FileName[BUFF_SIZE] = {0};
		char FileType[BUFF_SIZE] = {0};
		char client_req[BUFF_SIZE] = {0};
		sscanf(recvBuff,"GET %s HTTP", client_req);

		/*Client GET root dir，return Fou.html*/
		if (!strcmp(client_req,"/")){
			printf("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
			printf("Hello CGI!\n");
			printf("The following is the results of \"ls -l\"\n\n");
			execv("/bin/ls", parmList);
		}
		/* Client GET other files.*/
		else{
			//讀取到的檔名放到FileName，副檔名放到FileType
			sscanf(client_req,"%[^.]%s",FileName,FileType);
			int reqFile_fd;
			sprintf(NowWorkingPath,".%s", client_req); //NowWorkingPath
			reqFile_fd = open(NowWorkingPath, O_RDONLY);
			if (  reqFile_fd != -1 ) {
				char ACK_MESSAGE[BUFF_SIZE];
				memset(ACK_MESSAGE,0,BUFF_SIZE);
				sprintf(ACK_MESSAGE,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n",Get_ResponeFileType(FileType));
				write(0, ACK_MESSAGE, strlen(ACK_MESSAGE));

				char Buffer[BUFF_SIZE];
				long i;
				while( (i = read(reqFile_fd, Buffer, sizeof(Buffer)) ) > 0 )
				{
					write(0, Buffer, i);
					memset(Buffer, 0, sizeof(Buffer));
				}
			}
		}
	}
}

int main(int argc, char* argv[]){
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
				close(sd);

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

	return 0;
}