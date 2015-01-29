# Some variables
CC 		= gcc
CFLAGS	= -g -Wall -DDEBUG -O3
LDFLAGS = lm

INCLUDES = -I../include
LIBRARY = -L../lib

all: mylib.so server

mylib.o: mylib.c
	gcc -Wall -fPIC -DPIC $(INCLUDES) -c mylib.c

mylib.so: mylib.o
	ld -shared $(LIBRARY) -o mylib.so mylib.o -ldl

server: server.o
	gcc server.o -o server

server.o: server.c
	gcc -Wall -fPIC -DPIC -c server.c

clean:
	rm -f *.o *.so server

