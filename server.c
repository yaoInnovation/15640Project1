#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include "pktProcess.h"


#define MAXMSGLEN 1500

int main(int argc, char**argv) {
	char buf[MAXMSGLEN];
	char *serverport;
	unsigned short port;
	int sockfd, sessfd, rv;
	struct sockaddr_in srv, cli;
	socklen_t sa_size;
	
	// Get environment variable indicating the port of the server
	serverport = getenv("serverport15440");
	if (serverport) port = (unsigned short)atoi(serverport);
	else port=15440;
	
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// TCP/IP socket
	if (sockfd<0) err(1, 0);			// in case of error
	
	// setup address structure to indicate server port
	memset(&srv, 0, sizeof(srv));			// clear it first
	srv.sin_family = AF_INET;			// IP family
	srv.sin_addr.s_addr = htonl(INADDR_ANY);	// don't care IP address
	srv.sin_port = htons(port);			// server port

	// bind to our port
	rv = bind(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
	if (rv<0) err(1,0);
	
	// start listening for connections
	rv = listen(sockfd, 5);
	if (rv<0) err(1,0);
	
	while(1) {
		
		// wait for next client, get session socket
		sa_size = sizeof(struct sockaddr_in);
		sessfd = accept(sockfd, (struct sockaddr *)&cli, &sa_size);
		if (sessfd<0) err(1,0);
		// get messages and send replies to this client, until it goes away
		if( (rv=recv(sessfd, buf, MAXMSGLEN, 0)) > 0) {
			int reVal = 0;
			int err = 0;
			int payloadSize = 0;
			char* payload = NULL;
			char* rePkt = NULL;

			// check first line
			if(strstr(buf,"\n") != NULL) {
				//check pkt type
				if(strncmp(buf, "OPEN",4) == 0) 
					reVal = openPkt(buf,&err);
				if(strncmp(buf, "CLOSE",5) == 0) 
					reVal = closePkt(buf,&err);
				if(strncmp(buf, "WRITE",5) == 0) 
					reVal = writePkt(buf,&err);
				if(strncmp(buf, "READ", 4) == 0)
					reVal = readPkt(buf,&err, &payloadSize, &payload);
				if(strncmp(buf, "UNLINK", 6) ==0)
					reVal = unlinkPkt(buf, &err);
				if(strncmp(buf, "LSEEK", 5) == 0)
					reVal = lseekPkt(buf, &err);
				if(strncmp(buf, "STAT", 4) == 0)
					reVal = statPkt(buf, &err, &payloadSize, &payload);
				if(strncmp(buf, "__XSTAT", 7) == 0) 
					reVal = __xstatPkt(buf, &err, &payloadSize, &payload);
				if(strncmp(buf, "GENDIRENTRIES", strlen("GENDIRENTRIES")) ==0 )
					reVal = gendirentriesPkt(buf, &err, &payloadSize, &payload);
			}

			// send return val back
			if(payload == NULL) {
				rePkt = rePktGen(reVal,err);
				send(sessfd,rePkt,strlen(rePkt),0);
				free(rePkt);
			}
			else {
				rePkt =re_PLDPkt_Gen(reVal,err,payloadSize,payload);
				send(sessfd,rePkt,payloadSize+(int)(strstr(rePkt,"\n\n")-rePkt)+strlen("\n\n"),0);
				free(rePkt);
				free(payload);
			}
			memset(buf,0,MAXMSGLEN);
		}

		// either client closed connection, or error
		if (rv<0) err(1,0);
		close(sessfd);
	}
	
	// close socket
	close(sockfd);

	return 0;
}

