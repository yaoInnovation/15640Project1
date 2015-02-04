#include "pktProcess.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_PKT_SIZE 8192
#define MAX_PATHNAME 200

int openPkt(char* buf, int* err) {
	int flags;
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	mode_t mode = 0;
	char filename[MAX_PATHNAME];
	char buffer[MAX_PATHNAME]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);
	memset(filename,0,MAX_PATHNAME);
	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	strcpy(filename,buffer); filename[(int)(indexEnd-indexStart)] = '\0';

	// get flags;
	memset(buffer,0,MAX_PATHNAME);
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart,(int)(indexEnd-indexStart));
	flags = atoi(buffer);
	// get mode;
	memset(buffer,0,MAX_PATHNAME);
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart,(int)(indexEnd-indexStart));
	mode = atoi(buffer);
	//printf("Exist?:%d\n",access(filename,F_OK));
	if( (flags & O_CREAT) && access(filename,F_OK) == -1)
		reVal = open(filename,flags,mode);
	else
		reVal = open(filename,flags);

	printf("open %s flags:%d mode:%d,return:%d called\n",filename, flags, mode, reVal);
	*err = errno;
	return reVal;
}

int writePkt(char* buf, int* err) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	int size = 0;
	char buffer[MAX_PATHNAME]; // tmp buffer
	char content[MAX_PKT_SIZE]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	fd = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get size
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	size = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get content
	indexStart = indexEnd+1;
	memcpy(content,indexStart,size);
	reVal = write(fd,content,size);
	*err = errno;
	printf("write %d %d:\n%s\ncalled\nreturn:%d\n",fd, size, content,reVal);
	return reVal;
}

int readPkt(char* buf, int* err, int* payloadSize, char** content) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	int size = 0;
	char buffer[MAX_PATHNAME]; // tmp buffer
	*content = (char*)malloc(MAX_PKT_SIZE); // maximum file name
	memset(buffer,0,MAX_PATHNAME);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	fd = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get size
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	size = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get content from file
	reVal = read(fd,*content,size);

	// set errno and payload size
	*payloadSize = reVal;
	*err = errno;

	printf("read called\nreturn:%d, errno=%s\n", reVal,strerror(errno));
	return reVal;
}


int closePkt(char* buf,int* err) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	char buffer[MAX_PATHNAME]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	fd = atoi(buffer);
	reVal = close(fd);
	*err = errno;
	//printf("close %d called\n",fd);
	return reVal;
}

int unlinkPkt(char* buf, int* err) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	char buffer[MAX_PATHNAME]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);

	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	buffer[strlen(buffer)] = '\0';
	reVal = unlink(buffer);
	*err = errno;
	//printf("close %d called\n",fd);
	return reVal;
}

int lseekPkt(char* buf, int* err) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	int fd;
	off_t offset;
	int whence = 0;
	char buffer[MAX_PATHNAME]; // tmp buffer
	memset(buffer,0,MAX_PATHNAME);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	fd = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get offset
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	offset = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get offset
	indexStart = indexEnd+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer,indexStart, indexEnd-indexStart);
	whence = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// set errno and payload size
	lseek(fd,offset,whence);
	*err = errno;

	return reVal;
}

int statPkt(char* buf, int* err, int* payloadSize, char** payload) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int reVal = 0;
	char buffer[MAX_PATHNAME]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);
	struct stat* file_stat = malloc(sizeof(struct stat));
	*payload = (char*)malloc(sizeof(struct stat));

	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	buffer[strlen(buffer)] = '\0';
	
	// get stat
	reVal = stat(buffer,file_stat);

	// copy the bytes from structure to buffer
	memcpy(*payload,file_stat,sizeof(struct stat));
	*payloadSize = sizeof(file_stat);
	*err = errno;
	//printf("stat %s called\n",buf);
	return reVal;
}

int __xstatPkt(char* buf, int* err, int* payloadSize, char** payload) {
	char* indexStart = 0;
	char* indexEnd = 0;
	int ver = 0;
	int reVal = 0;
	char filename[MAX_PATHNAME] = {0};// maximum file name
	char tmp[MAX_PATHNAME] = {0}; // temp buffer for converting integers 
	memset(filename,0,MAX_PATHNAME);
	struct stat* file_stat = malloc(sizeof(struct stat));
	*payload = (char*)malloc(sizeof(struct stat));

	// get filename
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(filename, indexStart, (int)(indexEnd-indexStart));
	filename[strlen(filename)] = '\0';

	// get version
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(tmp, indexStart, (int)(indexEnd-indexStart));
	tmp[strlen(tmp)] = '\0';
	ver = atoi(tmp);

	// get __xstat
	reVal = __xstat(ver,filename,file_stat);

	// copy the bytes from structure to buffer
	memcpy(*payload,file_stat,sizeof(struct stat));
	*payloadSize = sizeof(file_stat);
	*err = errno;
	free(file_stat);
	//printf("stat %s called\n",buf);
	return reVal;
}

int gendirentriesPkt(char* buf, int* err, int* payloadSize, char** payload) {
	char* indexStart = NULL;
	char* indexEnd = NULL;
	int reVal = 0;
	int fd = 0;
	off_t basep = 0;
	size_t nbytes = 0;
	char buffer[MAX_PATHNAME]; // maximum file name
	memset(buffer,0,MAX_PATHNAME);

	*payload = (char*)malloc(MAX_PKT_SIZE);
	memset(*payload,0,MAX_PKT_SIZE);

	// get fd
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	buffer[strlen(buffer)] = '\0';
	fd = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get nbytes
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, ' ');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	buffer[strlen(buffer)] = '\0';
	nbytes = atoi(buffer); memset(buffer,0,MAX_PATHNAME);

	// get base position
	indexStart = strchr(buf,' ')+1;
	indexEnd = strchr(indexStart, '\n');
	strncpy(buffer, indexStart, (int)(indexEnd-indexStart));
	buffer[strlen(buffer)] = '\0';
	basep = atoi(buffer); 

	// get direntries
	reVal = getdirentries(fd, *payload, nbytes,&basep);
	*payloadSize = reVal;
	*err = errno;

	//printf("stat %s called\n",buf);
	return reVal;
}

char* rePktGen(int val, int err) {
	char* pkt = (char*)malloc(MAX_PATHNAME);
	sprintf(pkt,"RETURN:%d, ERRNO:%d\n", val, err);
	return pkt;
}


char* re_PLDPkt_Gen(int val, int err, int size, char* payload) {
	char* pkt = (char*)malloc(MAX_PKT_SIZE);
	sprintf(pkt,"RETURN:%d, ERRNO:%d\n", val, err);
	memcpy(pkt+strlen(pkt), payload,size);
	return pkt;
}