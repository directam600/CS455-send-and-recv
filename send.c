//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - James Du

/*
   udp4-s3.c: version 3 of sender
	 - uses symbolic name for address
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
#include <sys/poll.h>

struct stopnwait{
	int seqnum;
	char buf[100];
};

struct stopnwait temp;

void reliable_sendto(int seqno, int sockfd, const void *buf, size_t len, int flags /* flags */, 
	     const struct sockaddr *dest_addr, socklen_t addrlen);

int seqno = 0;  //initial seqno set to 0

int main(int argc, char *argv[])
{
   int s;				/* s = socket */
   struct sockaddr_in in_addr;		/* Structure used for bind() */
   struct sockaddr_in sock_addr;	/* Output structure from getsockname */
   struct sockaddr_in dest_addr;	/* Destination socket */
   char line[100];
   char **p;
   int len;

   struct hostent *host_entry_ptr;     /* Structure to receive output */

   if (argc < 3)
    { printf("Usage: %s <symbolic-dest-addr> <dest-port>\n", argv[0]);
      printf("   Program sends messages to <symbolic-dest-addr> <dest-port>\n");
      exit(1);
    }

   /* -------
      Fill in destination socket - this will identify IP-host + (UDP) port
      ------- */
   dest_addr.sin_family = AF_INET;		 /* Internet domain */

   host_entry_ptr = gethostbyname(argv[1]);   /* E.g.: cssun.mathcs.emory.edu */

   if (host_entry_ptr == NULL)
    { printf("Host `%s' not found...\n", argv[1]);     /* Just for safety.... */
      exit(1);
    }

   /**********************************************************************
    * NOTE: DO NOT use htonl on the h_addr_list[0] returned by 
    * gethostbyname() !!!
    **********************************************************************/
   memcpy((char *) &(dest_addr.sin_addr.s_addr), 
		   host_entry_ptr->h_addr_list[0], 
		   host_entry_ptr->h_length);

   /**********************************************************************/

   dest_addr.sin_port = htons(atoi(argv[2]));    /* Destination (UDP) port # */

   /* ---
      Create a socket
      --- */
   s = socket(PF_INET, SOCK_DGRAM, 0);

   /* ---
      Set up socket end-point info for binding
      --- */
   memset((char *)&in_addr, 0, sizeof(in_addr));
   in_addr.sin_family = AF_INET;                   /* Protocol domain */
   in_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* Use wildcard IP address */
   in_addr.sin_port = htons(0);	           	   /* Use any UDP port */

   /* ---
      Here goes the binding...
      --- */
   if (bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)) == -1)
    { printf("Error: bind FAILED\n");
    }
   else
    { printf("OK: bind SUCCESS\n");
    }

   /* ----
      Check what port I got
      ---- */
   len = sizeof(sock_addr);
   getsockname(s, (struct sockaddr *) &sock_addr, &len);
   printf("Socket s is bind to:\n");
   printf("  addr = %u\n", ntohl(sock_addr.sin_addr.s_addr));
   printf("  port = %d\n", ntohs(sock_addr.sin_port));
  
   while(1)
    { printf("Enter a line: ");
      scanf("%[^\n]", &line);
      getchar();

      /* ----
	 sendto() will automatically use UDP layer
	 ---- */

      reliable_sendto(seqno, s, line, strlen(line)+1, 0 /* flags */, 
	     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
      seqno++;  //increment seqno for next line
    }
}

void reliable_sendto(int seqno, int sockfd, const void *buf, size_t len, int flags, 
	     const struct sockaddr *dest_addr, socklen_t addrlen){	
	int recvACK;
	int timeout = 50;  //50 milliseconds
	bool ack_recv = false;

	//Copy seqno and the message to the temp struct to be sent to rec
	temp.seqnum = seqno; 
	strcpy(temp.buf, buf);

	sendto(sockfd, &temp, sizeof(temp), flags, dest_addr, addrlen);

        struct pollfd fds[1];  //only 1 socket needed

        fds[0].fd = sockfd;  //fd to be checked
        fds[0].events = 0;   //input bit array of events to be checked
        fds[0].events = fds[0].events | POLLIN; //output bit array of ready events.  POLLIN signifies ready for input
	
	while(!ack_recv){
		if(poll(fds, 1, timeout) != 0){	//when not equal to 0, something has been received
			struct sockaddr_in sendAddr;  //used for the received message
			recvfrom(sockfd, &recvACK, sizeof(int), 0, (struct sockaddr*)&sendAddr, (int *)sizeof(sendAddr)); //obtain acknowledgement number from recv

			//printf("%d\n", recvACK);  //used for testing: prints out received ack numbers
			if(seqno == recvACK){
				ack_recv = true;  //loop exit condition
			}
			else{
				continue;
			}
		}
		else{
			sendto(sockfd, &temp, sizeof(temp), flags, dest_addr, addrlen);  //timeout: retransmit message
		}		
	}
}
