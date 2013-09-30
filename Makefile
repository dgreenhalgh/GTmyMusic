#################################################################
##
## FILE:	Makefile
## PROJECT:	CS 3251 Project 2 - Professor Traynor
## DESCRIPTION: Compile Project 2
##
#################################################################

CC=gcc

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
	LDFLAGS=-lsocket -lnsl
    endif

all: client server 

client: client.c
	$(CC) client.c -o musicClient

server: server.c
	$(CC) server.c -o musicServer -lcrypto

clean:
	    rm -f client server *.o
	    