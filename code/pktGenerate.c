#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pktGenerate.h"

char* openPktGen(const char *pathname, int flags, ...) {
	char* pkt = (char*)malloc(200);
	strcpy(pkt,"OPEN ");
	strcpy(pkt+5,pathname);
	strcat(pkt," ");
	sprintf(pkt+strlen(pkt), "%d", flags);
	strcat(pkt,"\n\n");
	printf("OPEN PKT:\n%s\n",pkt);
	return pkt;
}

char* writePktGen(int fd, void* buffer, int nbyte) {
	char* pkt = (char*)malloc(MAX_PKT_SIZE);
	strcpy(pkt, "WRITE ");
	sprintf(pkt+strlen(pkt), "%d", fd);
	strcat(pkt," ");
	sprintf(pkt+strlen(pkt), "%d", nbyte);
	strcat(pkt,"\n");
	strncpy(pkt+strlen(pkt),buffer,nbyte);
	printf("WRITE PKT:\n%s\n",pkt);
	return pkt;
}

char* closePktGen(int fd) {
	char* pkt = (char*)malloc(200);
	strcpy(pkt,"CLOSE ");
	sprintf(pkt+strlen(pkt), "%d", fd);
	strcat(pkt,"\n\n");
	printf("CLSOE PKT:\n%s\n",pkt);
	return pkt;
}