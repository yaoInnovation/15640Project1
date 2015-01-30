#include "pktProcess.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define MAX_PKT_SIZE 8192

int openPkt(char* buf) {
	int flags;
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	mode_t mode;
	char filename[200];
	char buffer[200]; // maximum file name
	memset(buffer,0,200);
	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	strcpy(filename,buffer); filename[strlen(filename)] = '\0';

	// get flags;
	memset(buffer,0,200);
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart,(int)(indexEnd-indexStart));
	flags = atoi(buffer);
	// get mode;
	memset(buffer,0,200);
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart,(int)(indexEnd-indexStart));
	mode = atoi(buffer);
	if( flags & O_CREAT)
		reVal = open(filename,flags,mode);
	else
		reVal = open(filename,flags);

	//printf("open %s flags:%d mode:%d,%d called\n",filename, flags, mode, reVal);
	return reVal;
}

int writePkt(char* buf) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	int size = 0;
	char buffer[200]; // tmp buffer
	char content[MAX_PKT_SIZE]; // maximum file name
	memset(buffer,0,200);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	fd = atoi(buffer); memset(buffer,0,200);

	// get size
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	size = atoi(buffer); memset(buffer,0,200);

	// get content
	indexStart = indexEnd+1;
	memcpy(content,indexStart,size);
	reVal = write(fd,content,size);

	//printf("write %d %d:\n%s\ncalled\nreturn:%d\n",fd, size, content,reVal);
	return reVal;
}

int closePkt(char* buf) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	char buffer[200]; // maximum file name
	memset(buffer,0,200);

	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	fd = atoi(buffer);
	reVal = close(fd);
	//printf("close %d called\n",fd);
	return reVal;
}

char* rePktGen(int val) {
	char* pkt = (char*)malloc(200);
	sprintf(pkt,"RETURN:%d, ERRNO:%d\n", val, errno);
	return pkt;
}