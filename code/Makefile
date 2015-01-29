# Some variables
CC 		= gcc
CFLAGS	= -g -Wall -DDEBUG -O3
LDFLAGS = lm

INCLUDES = -I../include
LIBRARY = -L../lib

all: mylib.so server

mylib.o: mylib.c pktGenerate.h
	gcc -Wall -fPIC -DPIC $(INCLUDES) -c mylib.c

mylib.so: mylib.o pktGenerate.o
	ld -shared $(LIBRARY) -o mylib.so mylib.o pktGenerate.o -ldl

server: server.o
	gcc server.o -o server

server.o: server.c
	gcc -Wall -fPIC -DPIC -c server.c

pktGenerate.o: pktGenerate.c pktGenerate.h
	gcc -Wall -fPIC -DPIC -c pktGenerate.c

clean:
	rm -f *.o *.so server

handin:
	tar czvf ../mysolution.tgz Makefile mylib.c server.c 

