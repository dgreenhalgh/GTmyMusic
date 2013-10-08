/*///////////////////////////////////////////////////////////
*
* FILE:     	client.c
* AUTHOR:   	Devin Gonzalez & David Greenhalgh
* PROJECT:  	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  Network Client Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */
#include "GTmyMusic.h"

/* Constants */
#define LIST 0
#define DIFF 1
#define PULL 2
#define LEAVE 3
#define START_STATE 100
#define ERROR_STATE -1

#define ASCII_CORRECTOR (-49)

#define RCVBUFSIZE 512		    /* The receive buffer size */
#define SNDBUFSIZE 512		    /* The send buffer size */

#define MAX_NUM_FILES 10

/* Function pointers */
int send_command(int);
int switch_state(int);
void print_main_menu_options();
void init_connection(char*, unsigned short);
void create_tcp_socket(int*);
char* recieve_message();
int compare_files(char*, char*);

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "LEAF"};

const char* bad_command = "Command not recognized, exiting now.\n";
const char* bad_number_of_commands = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Socket info */
int client_sock, i_file, num_files; // no camel case
struct sockaddr_in serv_addr;

char rcv_buffer[RCVBUFSIZE];
char send_buffer[SNDBUFSIZE];

char* server_ip = "127.0.0.1"; 		// temp
unsigned short server_port = 2013; 	// temp

FILE* local_files[MAX_NUM_FILES];
char* local_filenames[MAX_NUM_FILES];

size_t local_file_lengths[MAX_NUM_FILES]; // replace NUM_FILES

/*
 * The main function
 */
int main(int argc, char *argv[])
{
	/* Read in local files */
	printf("Reading local files...\n");
	for(i_file = 0; i_file < MAX_NUM_FILES; i_file++)
	{
		char filename[20];
	 	sprintf(filename, "song%d%s", i_file, ".mp3");
	 	printf("%s\n", filename);

	 	local_filenames[i_file] = filename;
	 	local_files[i_file] = fopen(filename, "r");

	 	fseek(local_files[i_file], sizeof(local_files[i_file])*i_file, SEEK_END); // seg fault
	 	printf("%d\n", i_file);
	 	local_file_lengths[i_file] = ftell(local_files[i_file]);

	 	fclose(local_files[i_file]); // maybe?
	}

	if(argc == 1)
	{
		/* Command line interface */
		switch_state(START_STATE);

	}
	else if(argc == 2)
	{
		/* Direct command */
		if(strcmp(argv[1], "list") == 0)
		{
			send_command(LIST);
		}
		else if(strcmp(argv[1], "diff") == 0)
		{
			send_command(DIFF);
		}
		else if(strcmp(argv[1], "pull") == 0)
		{
			send_command(PULL);
		}
		else if(strcmp(argv[1], "leave") == 0)
		{
			send_command(LEAVE);
		}
		else
		{
			printf("%s", bad_command);
		}
	}
	else if(argc == 3)
	{
		printf("%d\n", compare_files(argv[1], argv[2]));
	}
	else
	{
		printf("%s", bad_number_of_commands);
	}
}

/*
 * Sends a command to the server.
 *
 * Param: cmd - integer identifier for the command being sent
 * Returns: cmd - The state identifier of the command being sent
 */
int send_command(int cmd)
{
	char* user_command = commands[cmd];
	printf("%s\n", user_command);

	init_connection(server_ip, server_port);
	printf("Connected\n");

	size_t echo_string_len = strlen(user_command);
	printf("%zu", echo_string_len);

	/* Send command string to the server */
	size_t num_bytes = send(client_sock, user_command, echo_string_len, 0); 
	printf("%zu", num_bytes);

	if((num_bytes < 0) || (num_bytes != echo_string_len))
		switch_state(ERROR_STATE);

	/* Receive command back */
	unsigned int total_bytes_rcvd = 0;

	// Note: all commands are 4 chars in length
	size_t command_length = sizeof(char)*4;
	size_t num_command_bytes;

	/* Read command name */
	char command_name_buffer[sizeof(char)*4];
	while(total_bytes_rcvd < command_length)
	{
		num_command_bytes = recv(client_sock, command_name_buffer, command_length, 0);
	}

	while(total_bytes_rcvd < echo_string_len)
	{
		char buffer[RCVBUFSIZE];
		num_bytes = recv(client_sock, buffer, RCVBUFSIZE - 1, 0);

		if(num_bytes <= 0)
			switch_state(ERROR_STATE);
		
		total_bytes_rcvd += num_bytes;
		buffer[num_bytes] = '\0';

		fputs(buffer, stdout); // temp
	}

	return cmd;
}

/*
 * Presenter for the command interface
 *
 * Param: The state identifier to present to the user.
 * Returns: The state identifier
 */
int switch_state(int state)
{
	if(state == START_STATE)
	{
		printf("Welcome!\n");
		print_main_menu_options();
		
		send_command(getchar() + ASCII_CORRECTOR); 

		close(client_sock);

	}
	else if(state = ERROR_STATE)
	{
		printf("Connection lost, exiting now.\n");	
		exit(1);
	}

	return state;
}

/*
 * Prints out the formatted main menu of command options to the user
 */
void print_main_menu_options()
{
	printf("Please enter the number corresponding to one of the following commands:\n");
	printf("(1) LIST\n(2) DIFF\n(3) PULL\n(4) LEAVE\n");
}

/*
 * Initializes a connection with the server
 */
void init_connection(char* serv_ip, unsigned short serv_port)
{
	memset(&send_buffer, 0, SNDBUFSIZE);
	memset(&rcv_buffer, 0, RCVBUFSIZE);
	printf("memset\n");

	create_tcp_socket(&client_sock);
	printf("Created TCP socket\n");

	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	printf("Constructed server address structure\n");

	/* Convert address */
	int ret_val = inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr.s_addr);

	if(ret_val <= 0)
		switch_state(ERROR_STATE);

	serv_addr.sin_port = htons(serv_port);

	printf("Convert address\n");

	/* Establish connection */
	if(connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		switch_state(ERROR_STATE);

	printf("Establish connection\n");
}

/*
 * Creates a TCP socket on the client side
 *
 * Param: The client socket
 */
void create_tcp_socket(int* p_client_socket)
{
	*p_client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(p_client_socket < 0)
		switch_state(ERROR_STATE);
}

int compare_files(char* filename_a, char* filename_b)
{
	FILE* file_a = fopen(filename_a, "r");
	FILE* file_b = fopen(filename_b, "r");

	char file_buffer_a[100000]; // Assuming we won't have a song file length over 100MB
	char file_buffer_b[100000];

	int ret_val = 1, var = 0;

	while(((fgets(file_buffer_a, 1000, file_a)) && (fgets(file_buffer_b, 1000, file_b))))
	{
		var = strcmp(file_buffer_a, file_buffer_b);
		if(var != 0)
		{
			printf("Files differ\n");
			fclose(file_a);
			fclose(file_b);
			ret_val = 0;
			exit(0);
		}
	}

	printf("Same files");

}
