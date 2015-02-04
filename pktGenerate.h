#ifndef _PKTGENERATE_H
#define _PKTGENERATE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX_PKT_SIZE 1500
#define MAX_PATHNAME 200

char* openPktGen(const char *pathname, int flags, mode_t);
char* writePktGen(int fd, void* buffer, int nbyte);
char* readPktGen(int fd, int nbyte);
char* unlinkPktGen(const char* pathname);
char* lseekPktGen(int fd, off_t offset, int whence);
char* statPktGen(const char* path);
char* __xstatPktGen(int ver, const char* path);
char* getdirentriesPktGen(int fd, size_t nbytes, off_t basep);
char* closePktGen(int fd);


#endif