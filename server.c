/*///////////////////////////////////////////////////////////
*
* FILE:     	server.c
* AUTHOR:   	Devin Gonzalez & David Greenhalgh
* PROJECT:  	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  Network Server Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */
#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
#include <stdlib.h>         /* supports all sorts of functionality */
#include <unistd.h>         /* for close() */
#include <string.h>         /* support any string ops */
#include <openssl/evp.h>    /* for OpenSSL EVP digest libraries/SHA256 */
#include <pthread.h>		/* for parallel processing */

/* Macros */
#define RCV_BUF_SIZE 	512     /* The receive buffer size */
#define SND_BUF_SIZE 	512     /* The send buffer size */
#define BUFFER_SIZE		10 		/* The command can be 10 characters long. */
#define PORT_NUMBER 	1500	/* Server port number */
#define MAX_PENDING 	5		/* Maximum outstanding connection requests. */

#define NUM_FILES 10

/* Constants */
static const char* example = "Char Star";

/* Function pointers */
/* Params will need to be fleshed out once we define them */
int list();
int diff();
int pull();
int leave();


int server_socket;                          /* Server Socket */
int client_socket;                          /* Client Socket */
struct sockaddr_in server_address;          /* Local address */
struct sockaddr_in client_address;          /* Client address */
unsigned short server_port;                 /* Server port */
unsigned int address_length;                /* Length of address data struct */

char command_buffer[RCV_BUF_SIZE];           /* Buff to store command from client */
char response_buffer[SND_BUF_SIZE];          /* Buff to store response from server */

size_t byte_count;              // Byte counter
size_t response_length;         // Output Length

int iFile;
FILE* server_files[NUM_FILES];

/* 
 * The main function. 
 */
int main(int argc, char *argv[])
{
	/* Read in local files */
	for(iFile = 0; iFile < NUM_FILES; iFile++)
	{
		char filename[20];
		sprintf(filename, "song%d", iFile);
		server_files[iFile] = fopen(filename, "r");
	}

	/* Assign port number. */
	server_port = PORT_NUMBER;

    /* Create new TCP Socket for incoming requests. */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket() failed");
    }

    /* Construct local address structure. */
    memset(&server_address, 0, sizeof(server_address));     // zero out structure
    server_address.sin_family = AF_INET;                    // IPv4 address family
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);     // Any incoming interface
    server_address.sin_port = htons(server_port);        	// Local port

    /* Bind to local address structure. */
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        printf("bind() failed");
    }

	/* Listen for incoming connections. */
    if (listen(server_socket, MAX_PENDING) < 0) {
        printf("listen() failed");
    }

    /* Loop server forever. */
    while(1){
    	address_length = sizeof(client_address);

    	/* Accept incoming connection. */
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_length);
        if (client_socket < 0) {
            printf("accept() failed");
        }

        /* Extract the command from the packet and store in command_buffer */
        byte_count = recv(server_socket, command_buffer, BUFFER_SIZE - 1, 0);
        if (byte_count < 0) {
            printf("recv() failed");
        }
        else if (byte_count == 0) {
            printf("recv()", "connection closed prematurely");
        }

        /* Interpret and execute command. */
        /* TODO */

        /* Return response to client */
        response_length = strlen(response_buffer);

        byte_count = send(server_socket, response_buffer, response_length, 0);
        if (byte_count < 0) {
            printf("send() failed");
        }
        else if (byte_count != response_length) {
            printf("send()", "sent unexpected number of bytes");
        }

    }

    close(client_socket);
    return(1);
}

/* Other functions: */

/* 
 * Command: LIST
 *
 * Sends the list of files currently on the server to the requesting client
 */
int list()
{
	return(0);
}

/* 
 * Command: DIFF
 *
 * Sends the list of files currently on the server to the requesting client so 
 * that it can be compared to the list of files on the client machine.
 */
int diff()
{
	return(0);
}

/* 
 * Command: PULL
 *
 * Diffs and downloads the files not on the client machine to the client 
 * machine
 */
int pull()
{
	return(0);
}

/* 
 * Command: LEAVE
 *
 * Client machine and closes its connection with the server
 */
int leave()
{
	return(0);
}
