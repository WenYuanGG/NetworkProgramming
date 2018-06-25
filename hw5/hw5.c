#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

static unsigned short checksum(unsigned short *buf, int bufsz){
	unsigned long sum = 0xffff;

	while(bufsz > 1){
		sum += *buf;
		buf++;
		bufsz -= 2;
	}

	if(bufsz == 1)
		sum += *(unsigned char*)buf;

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int main(int argc, char *argv[]){
	int sockfd;

	struct icmphdr hdr;
	struct sockaddr_in addr;
	int n;

	char buf[2000];
	struct icmphdr *icmphdrptr;
	struct iphdr *iphdrptr;

	if(argc != 2){
		printf("usage : %s IPADDR\n", argv[0]);
		return 1;
	}

	addr.sin_family = PF_INET;

	n = inet_pton(PF_INET, argv[1], &addr.sin_addr);
	if(n < 0){
		perror("inet_pton");
		return -1;
	}

	sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockfd < 0){
		perror("socket");
		return -1;
	}

	memset(&hdr, 0, sizeof(hdr));

	hdr.type = ICMP_ECHO;
	hdr.code = 0;
	hdr.checksum = 0;
	hdr.un.echo.id = 0;
	hdr.un.echo.sequence = 0;

	hdr.checksum = checksum((unsigned short*)&hdr, sizeof(hdr));

	n = sendto(sockfd, (char*)&hdr, sizeof(hdr), 0, (struct sockaddr*)&addr, sizeof(addr));
	if(n < 1){
		perror("sendto");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	
	memset(buf, 0, sizeof(buf));

	n = recv(sockfd, buf, sizeof(buf), 0);
	if(n < 1){
		perror("recv");
		return -1;
	}

	iphdrptr = (struct iphdr*)buf;

	icmphdrptr = (struct icmphdr*)(buf + (iphdrptr->ihl) * 4);

	if(icmphdrptr->type != 0){
		if(icmphdrptr->type == 3){
			printf("received ICMP %d, and the code is %d\n", icmphdrptr->type, icmphdrptr->code);			
		}
		else{
			printf("received ICMP %d\n", icmphdrptr->type);
			printf("The host %s is alive\n", argv[1]);
		}		
	}
	else if(icmphdrptr->type == 0){
		printf("The host %s is alive\n", argv[1]);
	}

	close(sockfd);
	return 0;
}