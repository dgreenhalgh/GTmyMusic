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
#define PORT_NUMBER 	1500	/* Server port number */
#define MAX_PENDING 	5		/* Maximum outstanding connection requests. */

#define MAX_NUM_FILES 10

/* Constants */
static const char* example = "Char Star";

/* Function pointers */
/* Params will need to be fleshed out once we define them */
int list();
int diff();
int pull();
int leave();
size_t get_filenames_length(char*[]);
char* serialize_filenames(char*[]);


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

int iFile, num_files;
FILE* server_files[MAX_NUM_FILES];
char* server_filenames[MAX_NUM_FILES];

/* 
 * The main function. 
 */
int main(int argc, char *argv[])
{
	/* Read in local files */
	for(iFile = 0; iFile < MAX_NUM_FILES; iFile++)
	{
		char filename[20];
		sprintf(filename, "song%d", iFile);

		server_filenames[iFile] = filename;
		server_files[iFile] = fopen(filename, "r");
	}

	num_files = iFile;

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
	list_message new_list_message;
	strcpy(new_list_message.command_name, "LIST");
	new_list_message.filenames_length = get_filenames_length(server_filenames);
	new_list_message.serialized_server_filenames = serialize_filenames(server_filenames);

	send(server_socket, &new_list_message.command_name, sizeof(new_list_message.command_name), 0);
	send(server_socket, &new_list_message.filenames_length, sizeof(new_list_message.filenames_length), 0);
	send(server_socket, &new_list_message.serialized_server_filenames, sizeof(new_list_message.serialized_server_filenames), 0);
	
	return(0); // unused for now
}

/* 
 * Command: DIFF
 *
 * Sends the list of files currently on the server to the requesting client so 
 * that it can be compared to the list of files on the client machine.
 */
int diff()
{
	diff_message new_diff_message;
	strcpy(new_diff_message.command_name, "DIFF");
	new_diff_message.filenames_length = get_filenames_length(server_filenames);
	new_diff_message.serialized_server_filenames = serialize_filenames(server_filenames);

	send(server_socket, &new_diff_message.command_name, sizeof(new_diff_message.command_name), 0);
	send(server_socket, &new_diff_message.filenames_length, sizeof(new_diff_message.filenames_length), 0);
	send(server_socket, &new_diff_message.serialized_server_filenames, sizeof(new_diff_message.serialized_server_filenames), 0);

	return(0); // unused for now
}

/* 
 * Command: PULL
 *
 * Diffs and downloads the files not on the client machine to the client 
 * machine
 */
int pull()
{
	

	return(0); // unused for now
}

/* 
 * Command: LEAVE
 *
 * Client machine and closes its connection with the server
 */
int leave()
{
	leave_message new_leave_message;
	strcpy(new_leave_message.command_name, "LEAF");

	send(server_socket, &new_leave_message.command_name, sizeof(new_leave_message.command_name), 0);

	return(0); // unused for now
}

/*
 * Returns the number of bytes in the list of filenames on the server
 */
size_t get_filenames_length(char* filenames[])
{
	size_t fn_length = 0;
	int iFilename;
	for (iFilename = 0; iFilename < num_files; iFilename++)
		fn_length += sizeof(filenames[iFilename]);

	return fn_length;
}

char* serialize_filenames(char* filenames[])
{
	char* spaghetti = "";
	int iFilename;
	for(iFilename = 0; iFilename < get_filenames_length(filenames); iFilename++)
	{
		spaghetti = malloc(strlen(spaghetti)+strlen(filenames[iFilename])+1);
		strcat(spaghetti, filenames[iFilename]);
	}

	return spaghetti;
}
