//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - James Du

/*
   udp4-r2.c: bind to wildcard IP address
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#define MAXSIZE 100

struct stopnwait{
	int seqnum;
	char buf[MAXSIZE];
};

struct stopnwait temp;

int reliable_recvfrom(int seqno, int sockfd, void *buf, size_t len, int flags /* flags */, 
	     const struct sockaddr *src_addr, socklen_t *addrlen);

/* -----------------------------------------------------------------
    paddr: print the IP address in a standard decimal dotted format
   ----------------------------------------------------------------- */
void paddr(unsigned char *a)
{
   printf("%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
}

int seqno = 0;  //initial seqno set to 0

main(int argc, char *argv[])
{
   int s;			   /* s = socket */
   struct sockaddr_in in_addr;	   /* Structure used for bind() */
   struct sockaddr_in sock_addr;   /* Output structure from getsockname */
   struct sockaddr_in src_addr;    /* Used to receive (addr,port) of sender */
   int src_addr_len;		   /* Length of src_addr */
   int len;			   /* Length of result from getsockname */
   char line[MAXSIZE];
int seqno;

   if (argc == 1)
    { printf("Usage: %s port\n", argv[0]);
      exit(1);
    }

   /* ---
      Create a socket
      --- */
   s = socket(PF_INET, SOCK_DGRAM, 0);

   /* ---
      Set up socket end-point info for binding
      --- */
   memset((char *)&in_addr, 0, sizeof(in_addr));
   in_addr.sin_family = AF_INET;                   /* Protocol family */
   in_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* Wildcard IP address */
   in_addr.sin_port = htons(atoi(argv[1]));	   /* Use any UDP port */

   /* ---
      Here goes the binding...
      --- */
   if (bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)) == -1)
    { perror("bind: ");
      printf("- Note: this program can be run on any machine...\n");
      printf("        but you have to pick an unsed port#\n");
      exit(1);
    }
   else
    { printf("OK: bind SUCCESS\n");
    }

   /* --------------------------------------------------------
      **** Print socket "name" (which is IP-address + Port#)
      -------------------------------------------------------- */
   len = sizeof(sock_addr);
   getsockname(s, (struct sockaddr *) &sock_addr, &len);
   printf("Socket is bind to:\n");
   printf("  addr = %u\n", ntohl(sock_addr.sin_addr.s_addr) );
   printf("  port = %u\n", ntohs(sock_addr.sin_port) );

   while(1)
    { src_addr_len = sizeof(src_addr);
      len = reliable_recvfrom(seqno, s, line, MAXSIZE, 0 /* flags */,
                    (struct sockaddr *) &src_addr, &src_addr_len);
      seqno++;
      strcpy(line, temp.buf);
      printf("Msg from (");
      paddr( (void*) &src_addr.sin_addr);
      printf("/%u): `%s' (%u bytes)\n", src_addr.sin_port, line, len);
    }
}

int reliable_recvfrom(int seqno, int sockfd, void *buf, size_t len, int flags, 
	     const struct sockaddr *src_addr, socklen_t *addrlen){
	bool ack_recv = false;
	while(!ack_recv){
		recvfrom(sockfd, &temp, sizeof(temp), 0, (struct sockaddr*)src_addr, addrlen);  //receive seqno + buf from send
		if(seqno == temp.seqnum){  //check for matching sequence numbers and if true, send ack to send and then copy send's temp.buf to recv's buf and return its length
			sendto(sockfd, &(temp.seqnum), sizeof(int), flags, (struct sockaddr*)src_addr, *addrlen);
			ack_recv = true;  //loop exit condition
		}	
		else{
			sendto(sockfd, &(temp.seqnum), sizeof(int), flags, (struct sockaddr*)src_addr, *addrlen);	//timeout: retransmit sequence number
		}
	}
	return strlen(temp.buf);
}
