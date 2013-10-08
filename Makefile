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

all: musicClient musicServer

musicClient: client.c 
	$(CC) client.c -o musicClient -include GTmyMusic.h

musicServer: server.c
	$(CC) server.c -o musicServer -include GTmyMusic.h -lpthread -lcrypto

clean:
	    rm -f musicClient musicServer

