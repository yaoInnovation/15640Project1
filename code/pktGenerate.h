#ifndef _PKTGENERATE_H
#define _PKTGENERATE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PKT_SIZE 8192

char* openPktGen(const char *pathname, int flags, ...);
char* writePktGen(int fd, void* buffer, int nbyte);
char* closePktGen(int fd);


#endif