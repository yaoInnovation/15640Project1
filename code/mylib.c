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

#define MAXMSGLEN 8192

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
	char buffer[MAX_PATHNAME] = {0};
	char tmp[MAX_PATHNAME] = {0};
	char* indexStart;
	char* indexEnd;
	
	// read message from server
	if((nbyte = recv(sockfd,buffer,MAX_PATHNAME,0)) > 0) {
		// get return value
		indexStart = strstr(buffer,"RETURN:") + strlen("RETURN:");
		indexEnd = strchr(indexStart,' ');
		strncpy(tmp,indexStart,indexEnd-indexStart); tmp[strlen(tmp)] = '\0';
		*reVal = atoi(tmp);memset(tmp,0,MAX_PATHNAME);

		// get errno number
		indexStart = strstr(indexEnd+1,"ERRNO:")+strlen("ERRNO:");
		indexEnd = strchr(indexStart,'\n');
		strncpy(tmp,indexStart,indexEnd-indexStart); //tmp[strlen(tmp)] = '\0';
		*err = atoi(tmp);
	}
	return nbyte;
}

int readMsgWithPatyload(int sockfd, int* reVal, int*err, void* p) {
	
	char* indexStart = NULL;
	char* indexEnd = NULL;
	char* buf = malloc(MAXMSGLEN);
	int nbyte = 0;
	char tmp[20]; // this buffer is used to convert integers
	
	// read message from server
	if((nbyte = recv(sockfd,buf,MAX_PATHNAME,0)) > 0) {

		// get return value
		indexStart = strchr(buf,':')+1;
		indexEnd = strchr(indexStart,' ');
		strncpy(tmp,indexStart, indexEnd-indexStart);
		*reVal = atoi(tmp); memset(tmp,0,20);

		// get errno
		indexStart = strchr(indexEnd + 1,':')+1;
		indexEnd = strchr(indexStart,'\n');
		strncpy(tmp,indexStart,indexEnd-indexStart);
		*err = atoi(tmp);memset(tmp,0,20);

		// get content
		indexStart = indexEnd+1;
		if(*reVal != nbyte) {
			//printf("Weird! nbyte diff from client to server!\n");
		}
			memcpy(p,indexStart,*reVal);
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
	orig_close(sock);
	errno = err;

	printf("mylib: close called%d\n", fd);
	return reVal;
}

// This is our replacement for the open function from libc.
int open(const char *pathname, int flags, ...) {
	
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
	orig_close(sock);
	errno = err;
	printf("mylib: open called for path %s\n", pathname);
	return reVal;
}

/** @brief our repleace ment for the read function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the space where read content to 
 *	@param the size which is about to read
 *  @return the actual bytes that have been read
 */ 
int read(int fd, void *buffer, int nbyte) {
	if(nbyte == 0)
		return 0; // return 0 if requested size was 0
	
	// init network
	int sock = connectServer();
	int reVal = 0;
	int err = 0;

	// send request
	char* pkt = readPktGen(fd,nbyte);

	sendMsg(sock, pkt); free(pkt);

	readMsgWithPatyload(sock, &reVal, &err, buffer);

	orig_close(sock);
	errno = err;
	printf("mylib: read called\n");
	return reVal;
}

/** @brief our repleace ment for the write function from libc
 *	@param fd, file descriptor pointed to the file
 *	@param the buffer which contents the content to be written 
 *	@param the size which is about to write
 *  @return the actual bytes that have been written
 */
int write(int fd, void* buffer, int nbyte) {
	int reVal = 0;
	int err = 0;
	// init network
	int sock = connectServer();
	// contruct pkt
	char* pkt = writePktGen(fd,buffer,nbyte);
	// send request

	send(sock,pkt,nbyte+strchr(pkt,'\n')-pkt+1,0); free(pkt);
	//printf("with \\0, size:%d\n", size);
	// get feedback
	readMsg(sock,&reVal,&err);
	orig_close(sock);
	errno = err;
	printf("mylib: write called,return %d\n", reVal);
	return reVal;
}

int stat(const char *path, struct stat* buf) {
	int reVal = 0;
	int err =0;
	// init network
	int sock = connectServer();
	char* pkt = statPktGen(path);
	// send request
	sendMsg(sock,pkt);

	// get return value and errno
	readMsgWithPatyload(sock, &reVal, &err, buf);

	orig_close(sock);
	printf("mylib: stat called\n");
	errno = err;
	return reVal;
}

int __xstat(int ver, const char * path, struct stat * stat_buf) {
	int reVal = 0;
	int err = 0;
	// init network
	int sock = connectServer();
	char* pkt = __xstatPktGen(ver, path);
	// send request
	sendMsg(sock,pkt);

	// get return value, errno and content
	readMsgWithPatyload(sock, &reVal, &err, stat_buf);
	orig_close(sock);
	
	printf("mylib: __xstat called\n");
	errno = err;
	return reVal;
}

int getdirentries(int fd, char *buf, size_t nbytes, off_t *basep) {
	int reVal = 0;
	int err = 0;

	// init network
	int sock = connectServer();
	char* pkt = getdirentriesPktGen(fd, nbytes, *basep);

	// send request
	sendMsg(sock,pkt);
	
	//get return msg
	readMsgWithPatyload(sock, &reVal, &err, buf);
	*basep = *basep + reVal;
	orig_close(sock);
	
	errno = err;
	printf("mylib: getdirentries called\n");
	return reVal;
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
	int reVal = 0;
	int err = 0;
	// init network
	int sock = connectServer();
	char* pkt = lseekPktGen(fd, offset, whence);

	// send request
	sendMsg(sock,pkt); free(pkt);
	// get retur value and errno
	readMsg(sock,&reVal, &err);
	
	orig_close(sock);
	errno = err;
	printf("mylib: lseek called\n");
	return reVal;
}

int unlink(const char *path) {
	int reVal = 0;
	int err = 0;
	// init network
	int sock = connectServer();
	char* pkt = unlinkPktGen(path);

	// send request
	sendMsg(sock,pkt); free(pkt);
	// get retur value and errno
	readMsg(sock,&reVal, &err);
	
	orig_close(sock);
	errno = err;
	//printf("mylib: unlink called\n");
	return reVal;	
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


