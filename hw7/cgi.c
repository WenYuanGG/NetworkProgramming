#include <stdio.h>
#include <stdlib.h>

#define QUERY_STRING getenv("QUERY_STRING")
#define CONTENT_TYPE getenv("CONTENT_TYPE")
#define CONTENT_LENGTH getenv("CONTENT_LENGTH")
#define REQUEST_METHOD getenv("REQUEST_METHOD")
#define REMOTE_HOST getenv("REMOTE_HOST")
#define REMOTE_ADDR getenv("REMOTE_ADDR")
#define REMOTE_PORT getenv("REMOTE_PORT")

void print_environment(){
	//printf("<b>REQUEST_METHOD:</b> %s<br/>\n", REQUEST_METHOD);
	//printf("<b>CONTENT_TYPE:</b> %s<br/>\n", CONTENT_TYPE);
	//printf("<b>CONTENT_LENGTH:</b> %s<br/>\n", CONTENT_LENGTH);
	//printf("<b>QUERY_STRING:</b> %s<br/>\n", QUERY_STRING);
	//printf("<b>REMOTE_HOST:</b> %s<br/>\n", REMOTE_HOST);
	printf("<b>REMOTE_ADDR:</b> %s<br/>\n", REMOTE_ADDR);
	printf("<b>REMOTE_PORT:</b> %s<br/>\n", REMOTE_PORT);

	return;
}

int main(int argc, char* argv[]){
	printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	printf("<html><body>");
	printf("Hello CGI!<br/>");
	printf("<b>IP:</b> %s<br/>", REMOTE_ADDR);
	printf("<b>Port:</b> %s<br/>", REMOTE_PORT);
	print_environment();
	printf("</body></html>");

	return 0;
}