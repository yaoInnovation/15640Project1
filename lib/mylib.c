#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <err.h>
#include "dirtree.h"
//#include <unistd.h>

// The following line declares a function pointer with the same prototype as the open function.  
int (*orig_open)(const char *pathname, int flags, ...);  // mode_t mode is needed when flags includes O_CREAT
int (*orig_read)(int fd, void* buffer, int nbyte, ...);  
int (*orig_write)(int fd, void* buffer, int nbyte, ...);
int (*orig_lseek)(int fd, off_t offset, int whence, ...);
int (*orig_stat)(int ver, const char * path, struct stat * stat_buf, ...);
struct dirtreenode* (*orig_getdirtree)( const char *path );
void (*orig_freedirtree)( struct dirtreenode* dt );
int (*orig_close)(int fd, ...);

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

int sendMsg(int sockfd, const char* msg) {
	// send message to server
	//printf("client sending to server: %s\n", msg);
	send(sockfd, msg, strlen(msg), 0);	// send message; should check return value
	return 0;
}

int readMsg(int sockfd, const char* msg) {
	// read message from server
	//printf("client reading from server\n");
	send(sockfd, msg, strlen(msg), 0);
	return 0;
}

/** @brief our repleace ment for the close function from libc
 *	@param fd, file descriptor pointed to open file
 *  @The function returns 0 if successful, -1 to indicate an error, 
 *	with errno set appropriately.
 */
int close(int fd, ...) {
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"close");
	val = orig_close(sock);
	val = orig_close(fd);
	//printf("mylib: close called\n");
	return val;
}

// This is our replacement for the open function from libc.
int open(const char *pathname, int flags, ...) {
	// init network
	int sock = connectServer();
	int val = 0;
	// send request
	sendMsg(sock,"open");
	// we just print a message, then call through to the original open function (from libc)
	orig_close(sock);
	val = orig_open(pathname,flags);
	//printf("mylib: open called for path %s\n", pathname);
	return val;
}

/** @brief our repleace ment for the read function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the space where read content to 
 *	@param the size which is about to read
 *  @return the actual bytes that have been read
 */ 
int read(int fd, void *buffer, int nbyte, ... ) {
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
int write(int fd, void* buffer, int nbyte, ...) {
	int val = 0;
	// init network
	int sock = connectServer();
	
	// send request
	sendMsg(sock,"write");
	orig_close(sock);
	val = orig_write(fd,buffer,nbyte);
	//printf("mylib: write called\n");
	return val;
}

int __xstat(int ver, const char * path, struct stat * stat_buf) {
	int val = 0;
	// init network
	int sock = connectServer();
	// send request
	sendMsg(sock,"stat");
	orig_close(sock);
	val = __xstat(ver,path,stat_buf);
	//printf("mylib: lseek called\n");
	return val;
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


/** @brief our repleace ment for the write function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the buffer which contents the content to be written 
 *	@param the size which is about to write
 *  @return the actual bytes that have been written
 */
int lseek(int fd, off_t offset, int whence, ...){
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

// This function is automatically called when program is started
void _init(void) {
	// set function pointer orig_open to point to the original open function
	orig_open = dlsym(RTLD_NEXT, "open");
	orig_lseek = dlsym(RTLD_NEXT, "lseek");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_stat = dlsym(RTLD_NEXT, "__xstat");
	orig_getdirtree = dlsym(RTLD_NEXT, "getdirtree");
	orig_freedirtree = dlsym(RTLD_NEXT, "freedirtree");
	//printf("Init mylib\n");
}


