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

/* Function pointers */
/* Params will need to be fleshed out once we define them */
int list();
int diff();
int pull();
int leave();
size_t get_filenames_length(char*[]);
char* serialize_filenames(char*[]);
size_t get_server_files_length(size_t[]);
void* command_handler(void*);


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

int i_file;
FILE* server_files[NUM_FILES];
char* server_filenames[NUM_FILES];

size_t server_file_lengths[NUM_FILES];


/* pthreads */
static pthread_t *server_threads;
static int busy_threads[MAX_PENDING] = { 0 };


/* 
 * The main function. 
 */
int main(int argc, char *argv[])
{
    /* Read in local files */
    printf("Reading local files...\n");
	for(i_file = 0; i_file < NUM_FILES; i_file++)
	{
		char filename[20];
	 	sprintf(filename, "song%d%s", i_file, ".mp3");
	 	printf("%s\n", filename);

	 	server_filenames[i_file] = filename;
	 	server_files[i_file] = fopen(filename, "r");

	 	fseek(server_files[i_file], sizeof(server_files[i_file])*i_file, SEEK_END);
	 	printf("%d\n", i_file);
	 	server_file_lengths[i_file] = ftell(server_files[i_file]);

	 	fclose(server_files[i_file]); // maybe?
	}

	/* Assign port number. */
	server_port = PORT_NUMBER;

    /* Allocate arrays */
    server_threads = malloc(sizeof(pthread_t) * MAX_PENDING);

    /* Create new TCP Socket for incoming requests. */
    printf("Creating a new TCP Socket for incoming requests...\n");
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed");
    }
    else {
        printf("Socket Created");
    }

    /* Construct local address structure. */
    memset(&server_address, 0, sizeof(server_address));     // zero out structure
    server_address.sin_family = AF_INET;                    // IPv4 address family
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);     // Any incoming interface
    server_address.sin_port = htons(server_port);        	// Local port

    /* Bind to local address structure. */
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        printf("bind() failed\n");
    }

	/* Listen for incoming connections. */
    if (listen(server_socket, MAX_PENDING) < 0) {
        printf("listen() failed\n");
    }
    else {
        printf("Listening for incoming connections...\n");
    }

    /* Loop server forever. */
    while(1){
        printf("looping...\n");

    	address_length = sizeof(client_address);

    	/* Accept incoming connection. */
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_length);
        printf("Connection accepted\n");

        if (client_socket < 0) {
            printf("accept() failed\n");
        }
        else {
            printf("Accepted incoming connection.\n");
        }

        pthread_t* new_connection_thread;
        //pthread_create(&new_connection_thread, NULL, &command_handler, &command_handler_helper);

        /* Extract the command from the packet and store in command_buffer */
        byte_count = recv(server_socket, command_buffer, BUFFER_SIZE - 1, 0);
        if (byte_count < 0) {
            printf("recv() failed\n");
        }
        else if (byte_count == 0) {
            printf("recv()", "connection closed prematurely\n");
        }

        /* Interpret and execute command. */
        /* TODO */

        /* Return response to client */
        response_length = strlen(response_buffer);

        byte_count = send(server_socket, response_buffer, response_length, 0);
        if (byte_count < 0) {
            printf("send() failed\n");
        }
        else if (byte_count != response_length) {
            printf("send()", "sent unexpected number of bytes\n");
        }

    }

    close(client_socket);
    return(1);
}

/* Other functions: */
void* command_handler(void* helper_struct)
{
	char command[4];
	//recv((helper_struct).socket, command, 4, 0); // fix me
	printf("%s\n", command);

	if(strcmp(command, "LIST") == 0)
	{
		list();
	}
	else if(strcmp(command, "DIFF") == 0)
	{
		diff();
	}
	else if(strcmp(command, "PULL") == 0)
	{
		pull();
	}
	else if(strcmp(command, "LEAF") == 0)
	{
		leave();
	}
	else
	{
		printf("%s\n", "Invalid command string.");
	}
}

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
	/* Pull message 1 */
	pull_message_1 new_pull_message_1;
	strcpy(new_pull_message_1.command_name, "PLL1");
	new_pull_message_1.filenames_length = get_filenames_length(server_filenames);
	new_pull_message_1.serialized_server_filenames = serialize_filenames(server_filenames);

	send(server_socket, &new_pull_message_1.command_name, sizeof(new_pull_message_1.command_name), 0);
	send(server_socket, &new_pull_message_1.filenames_length, sizeof(new_pull_message_1.filenames_length), 0);
	send(server_socket, &new_pull_message_1.serialized_server_filenames, sizeof(new_pull_message_1.serialized_server_filenames), 0);

	/* Pull message 2 */


	/* Pull message 3 */
	pull_message_3 new_pull_message_3;
	strcpy(new_pull_message_3.command_name, "PLL3");
	new_pull_message_3.files_length = get_server_files_length(server_file_lengths);
	// pull_message_3.server_files is sent below

	send(server_socket, &new_pull_message_3.command_name, sizeof(new_pull_message_3.command_name), 0);
	send(server_socket, &new_pull_message_3.files_length, sizeof(new_pull_message_3.files_length), 0);

	int i_server_file;
	for(i_server_file = 0; i_server_file < NUM_FILES; i_server_file++)
		send(server_socket, &new_pull_message_3.server_files[i_server_file], sizeof(new_pull_message_3.server_files[i_server_file]), 0);

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
	int i_filename;
	for (i_filename = 0; i_filename < NUM_FILES; i_filename++)
		fn_length += sizeof(filenames[i_filename]);

	return fn_length;
}

char* serialize_filenames(char* filenames[])
{
	char* spaghetti = "";
	int i_filename;
	for(i_filename = 0; i_filename < get_filenames_length(filenames); i_filename++)
	{
		spaghetti = malloc(strlen(spaghetti)+strlen(filenames[i_filename])+1);
		strcat(spaghetti, filenames[i_filename]);
	}

	return spaghetti;
}

size_t get_server_files_length(size_t server_file_length_list[])
{
	size_t total_server_files_length = 0;
	int i_file_length;
	for(i_file_length = 0; i_file_length < NUM_FILES; i_file_length++)
	{
		total_server_files_length += server_file_length_list[i_file_length];
	}

	return total_server_files_length;
}
