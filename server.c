/*///////////////////////////////////////////////////////////
*
* FILE:     	server.c
* AUTHOR:   	Devin Gonzalez & David Greenhalgh
* PROJECT:  	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  Network Server Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */
#include "GTmyMusic.h"

/* Macros */
#define RCV_BUF_SIZE 	512     /* The receive buffer size */
#define SND_BUF_SIZE 	512     /* The send buffer size */
#define BUFFER_SIZE		10 		/* The command can be 10 characters long. */
#define PORT_NUMBER 	2013	/* Server port number */
#define MAX_PENDING 	5		/* Maximum outstanding connection requests. */

#define NUM_FILES 10

/* Constants */
static const char* example = "Char Star";

/* Function Prototypes */
/* Params will need to be fleshed out once we define them */
int list();
int diff();
int pull();
int leave();
void handle_tcp_client(int);

int server_socket;                          /* Server Socket */
int client_socket;                          /* Client Socket */
struct sockaddr_in server_address;          /* Local address */
struct sockaddr_in client_address;          /* Client address */
unsigned short server_port;                 /* Server port */
unsigned int address_length;                /* Length of address data struct */

char command_buffer[RCV_BUF_SIZE];           /* Buff to store command from client */
char response_buffer[SND_BUF_SIZE];          /* Buff to store response from server */

size_t byte_count_in;              // Byte counter
size_t response_length;         // Output Length

int iFile;
FILE* server_files[NUM_FILES];
char* server_filenames[NUM_FILES];

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

		server_filenames[iFile] = filename;
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

    /* Announce server address for clients. */
    printf("Server IP Address: %s\n", inet_ntoa(server_address.sin_addr));
    printf("Server Port Number: %d\n", (int) ntohs(server_address.sin_port));

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

        printf("DEBUG: accepted a connection\n");

        handle_tcp_client(client_socket);
    }

    return(1);
}

/* Other functions: */

void handle_tcp_client(int client_socket) {
    /* Extract the command from the packet and store in command_buffer */
    byte_count_in = recv(server_socket, command_buffer, BUFFER_SIZE - 1, 0);
    if (byte_count_in < 0) {
        printf("recv() failed");
    }
    else if (byte_count_in == 0) {
        printf("recv()", "connection closed prematurely");
    }

    /* Interpret and execute command. */
    while (byte_count_in > 0) { // 0 indicates end of stream

        // Echo message back to client
        size_t byte_count_out = send(client_socket, response_buffer, byte_count_in, 0);
        if (byte_count_out < 0)
          printf("send() failed");
        else if (byte_count_out != byte_count_in)
          printf("send()", "sent unexpected number of bytes");
        // See if there is more data to receive
        byte_count_in = recv(client_socket, response_buffer, response_length, 0);
        if (byte_count_in < 0)
          printf("recv() failed");
    }

    close(client_socket);
}

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
