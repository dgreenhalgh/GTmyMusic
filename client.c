/*///////////////////////////////////////////////////////////
*
* FILE:     	client.c
* AUTHOR:   	Devin Gonzalez & David Greenhalgh
* PROJECT:  	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  Network Client Code
*
*////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>

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

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "LEAVE"};

const char* bad_command = "Command not recognized, exiting now.\n";
const char* bad_number_of_commands = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Socket info */
int client_sock;
struct sockaddr_in serv_addr;

char rcv_buffer[RCVBUFSIZE];
char send_buffer[SNDBUFSIZE];

char* server_ip = "127.0.0.1"; 		// temp
unsigned short server_port = 2013; 	// temp

/* Function pointers */
int send_command(int);
int switch_state(int);
void print_main_menu_options();
void init_connection(char*, unsigned short);
void create_tcp_socket(int);

/*
 * The main function
 */
int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		/* Command line interface */
		switch_state(ERROR_STATE);

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

	/* Send command string to the server */
	size_t num_bytes = send(client_sock, user_command, strlen(user_command), 0); // client sock has no value yet

	if((num_bytes < 0) || (num_bytes != strlen(user_command)))
		switch_state(ERROR_STATE);

	//menuInterface(START_STATE);
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

	create_tcp_socket(client_sock); // client_sock has no value yet

	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	/* Convert address */
	int ret_val = inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr.s_addr);

	if(ret_val <= 0)
		switch_state(ERROR_STATE);


	serv_addr.sin_port = htons(serv_port);

	/* Establish connection */
	if(connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		switch_state(ERROR_STATE);
}

/*
 * Creates a TCP socket on the client side
 *
 * Param: The client socket
 */
void create_tcp_socket(int client_socket)
{
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(client_socket < 0)
		switch_state(ERROR_STATE);
}

