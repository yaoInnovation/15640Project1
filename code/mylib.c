#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include "pktGenerate.h"
#include "dirtree.h"
//#include <unistd.h>

// The following line declares a function pointer with the same prototype as the open function.  
int (*orig_open)(const char *pathname, int flags, ...);  // mode_t mode is needed when flags includes O_CREAT
int (*orig_read)(int fd, void* buffer, int nbyte);  
int (*orig_write)(int fd, void* buffer, int nbyte);
int (*orig_lseek)(int fd, off_t offset, int whence);
int (*orig___xstat)(int ver, const char * path, struct stat * stat_buf);
int (*orig_stat)(const char *path, struct stat *buf);
int (*orig_unlink)(const char *path);
int (*orig_getdirentries)(int fd, char *buf, size_t nbytes, off_t *basep);
struct dirtreenode* (*orig_getdirtree)( const char *path );
void (*orig_freedirtree)( struct dirtreenode* dt );
int (*orig_close)(int fd);

#define MAXMSGLEN 1024

int sockfd = 0;



// Network Function
int connectServer() {
	char *serverip;
	char *serverport;
	unsigned short port;
	int sockfd, rv;
	struct sockaddr_in srv;
	
	// Get environment variable indicating the ip address of the server
	serverip = getenv("server15440");
	if (serverip != NULL) {
		//printf("Got environment variable server15440: %s\n", serverip);
	}
	else {
		//printf("Environment variable server15440 not found.  Using 127.0.0.1\n");
		serverip = "127.0.0.1";
	}
	
	// Get environment variable indicating the port of the server
	serverport = getenv("serverport15440");
	if (serverport != NULL) {
		//printf("Got environment variable serverport15440: %s\n", serverport);
	}
	else {
		//printf("Environment variable serverport15440 not found.  Using 15440\n");
		serverport = "15440";
	}
	port = (unsigned short)atoi(serverport);
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// TCP/IP socket
	if (sockfd<0) err(1, 0);			// in case of error
	
	// setup address structure to point to server
	memset(&srv, 0, sizeof(srv));			// clear it first
	srv.sin_family = AF_INET;			// IP family
	srv.sin_addr.s_addr = inet_addr(serverip);	// IP address of server
	srv.sin_port = htons(port);			// server port

	// actually connect to the server
	rv = connect(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
	if (rv<0)  {
		printf("failed to connect server!\n");
		err(1,0);
	}
	return sockfd;
}

int sendMsg(int sockfd, char* msg) {
	// send message to server
	send(sockfd, msg, strlen(msg), 0);	// send message; should check return value
	return 0;
}

int readMsg(int sockfd, int* reVal, int* err) {
	int nbyte = 0;
	char buffer[200];
	char tmp[200];
	char* indexStart;
	char* indexEnd;
	
	// read message from server
	if((nbyte = recv(sockfd,buffer,200,0)) > 0) {

		// get return value
		indexStart = strstr(buffer,"RETURN:") + strlen("RETURN:");
		indexEnd = strchr(indexStart,' ');
		strncpy(tmp,indexStart,indexEnd-indexStart); tmp[strlen(tmp)] = '\0';
		*reVal = atoi(tmp);

		// get errno number
		indexStart = strstr(indexEnd+1,"ERRNO:")+strlen("ERRNO:");
		indexEnd = strchr(indexStart,'\n');
		strncpy(tmp,indexStart,indexEnd-indexStart); tmp[strlen(tmp)] = '\0';
		*err = atoi(tmp);
	}
	return nbyte;
}

/** @brief our repleace ment for the close function from libc
 *	@param fd, file descriptor pointed to open file
 *  @The function returns 0 if successful, -1 to indicate an error, 
 *	with errno set appropriately.
 */
int close(int fd) {
	int reVal, err = 0;
	// init network
	int sock = connectServer();
	// contruct pkt
	char* pkt = closePktGen(fd);
	// send request
	sendMsg(sock,pkt); free(pkt);
	// get feedback
	readMsg(sock,&reVal,&err);
	errno = err;
	
	orig_close(sock);
	return reVal;
}

// This is our replacement for the open function from libc.
int open(const char *pathname, int flags, ...) {
	
	printf("pathname:%s\n", pathname );
	mode_t m=0;
	if (flags & O_CREAT) {
		va_list a;
		va_start(a, flags);
		m = va_arg(a, mode_t);
		va_end(a);
	}

	// init network
	int sock = connectServer();
	int reVal = 0;
	int err = 0;
	// contruct pkt
	char* pkt = openPktGen(pathname,flags,m);
	// send request
	sendMsg(sock,pkt); free(pkt);
	// get feedback
	readMsg(sock,&reVal,&err);
	errno = err;
	
	orig_close(sock);
	//printf("mylib: open called for path %s\n", pathname);
	return reVal;
}

/** @brief our repleace ment for the read function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the space where read content to 
 *	@param the size which is about to read
 *  @return the actual bytes that have been read
 */ 
int read(int fd, void *buffer, int nbyte) {
	// init network
	int sock = connectServer();
	int val = 0;
	// send request
	sendMsg(sock,"read");
	orig_close(sock);
	val = orig_read(fd,buffer,nbyte);
	//printf("mylib: read called\n");
	return val;
}

/** @brief our repleace ment for the write function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the buffer which contents the content to be written 
 *	@param the size which is about to write
 *  @return the actual bytes that have been written
 */
int write(int fd, void* buffer, int nbyte) {
	int reVal, err =0;
	// init network
	int sock = connectServer();
	// contruct pkt
	char* pkt = writePktGen(fd,buffer,nbyte);
	// send request

	send(sock,pkt,nbyte+strchr(pkt,'\n')-pkt+1,0); free(pkt);
	//printf("with \\0, size:%d\n", size);
	// get feedback
	readMsg(sock,&reVal,&err);
	errno = err;

	orig_close(sock);
	//printf("mylib: write called,return %d\n", reVal);
	return reVal;
}

int stat(const char *path, struct stat *buf) {
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"stat");
	orig_close(sock);
	val = orig_stat(path,buf);
	//printf("mylib: lseek called\n");
	return val;
}

int __xstat(int ver, const char * path, struct stat * stat_buf) {
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"__xstat");
	orig_close(sock);
	val = orig___xstat(ver,path,stat_buf);
	//printf("mylib: lseek called\n");
	return val;
}

int getdirentries(int fd, char *buf, size_t nbytes, off_t *basep) {
	int size = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"getdirentries");
	orig_close(sock);
	size = orig_getdirentries(fd, buf, nbytes, basep);
	//printf("mylib: lseek called\n");
	return size;
}


struct dirtreenode* getdirtree( const char *path ) {
	struct dirtreenode* node = NULL;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"getdirtree");
	orig_close(sock);
	node = orig_getdirtree(path);
	//printf("mylib: lseek called\n");
	return node;
}

void freedirtree( struct dirtreenode* dt ) {
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"freedirtree");
	orig_close(sock);
	orig_freedirtree(dt);
	//printf("mylib: lseek called\n");
	return;
}


/** @brief our repleace ment for the lseak function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the offsize
 *	@param the start position
 *  @return the actual bytes that have been moved
 */
int lseek(int fd, off_t offset, int whence){
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"lseek");
	orig_close(sock);
	val = orig_lseek(fd,offset,whence);
	//printf("mylib: lseek called\n");
	return val;
}

int unlink(const char *path) {
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"unlink");
	orig_unlink(path);
	val = orig_unlink(path);
	//printf("mylib: lseek called\n");
	return val;	
}

// This function is automatically called when program is started
void _init(void) {
	// set function pointer orig_open to point to the original open function
	orig_open = dlsym(RTLD_NEXT, "open");
	orig_lseek = dlsym(RTLD_NEXT, "lseek");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_stat = dlsym(RTLD_NEXT, "stat");
	orig___xstat = dlsym(RTLD_NEXT, "__xstat");
	orig_unlink = dlsym(RTLD_NEXT, "unlink");
	orig_getdirentries = dlsym(RTLD_NEXT, "getdirentries");
	orig_getdirtree = dlsym(RTLD_NEXT, "getdirtree");
	orig_freedirtree = dlsym(RTLD_NEXT, "freedirtree");
	//printf("Init mylib\n");
}


