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
#define START_STATE 100
#define ERROR_STATE -1

#define ASCII_CORRECTOR (-49)

#define MAX_NUM_FILES 10

/* Function Prototypes */
int send_command(int);
int switch_state(int);
void print_main_menu_options();
void init_connection(char*, unsigned short);
void create_tcp_socket(int*);
char* recieve_message();
int compare_files(char*, char*);

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "PLL1", "PLL2", "PLL3", "LEAF"};

const char* bad_command = "Command not recognized, exiting now.\n";
const char* bad_number_of_commands = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Socket info */
int client_sock, i_file, num_files;
struct sockaddr_in serv_addr;

int num_bytes_rcvd = 0;
int total_bytes_rcvd = 0;
int num_bytes_sent = 0;
int total_bytes_sent = 0;

/* Send and Receive buffers */
char command_name_buffer[2];
char filenames_length_buffer[sizeof(int)];
char files_length_buffer[sizeof(size_t)];  // not sure what for?

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
	printf("sizeof size_t = %zu\n", sizeof(size_t));
	/* Read in local files */
	printf("Reading local files...\n");
	DIR *dir;
	struct dirent *ent;

	char cwd[1024];
	char *wd = getcwd(cwd, sizeof(cwd));
	
	char* full_dir = strcat(cwd, "/clientSongs/");

	if((dir = opendir(full_dir)) != NULL)
	{
		int count = 0;
		while ((ent = readdir (dir)) != NULL)
		{
			if((strcmp(ent->d_name, ".") != 0) &&
				(strcmp(ent->d_name, "..") != 0) &&
				(strcmp(ent->d_name, ".DS_Store") != 0))
			{
	    		local_filenames[count] = ent->d_name;
	    		count++;
    		}
		}
	}

	if(argc == 1) {
		/* Command line interface */
		switch_state(START_STATE);
	}
	else if(argc == 2)
	{
		/* Direct command */
		if(strcmp(argv[1], "list") == 0)
		{
			send_command(LIST);
			printf("Command Sent!\n");
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

	printf("End of Client MAIN.\n");
}

/*
 * Sends a command to the server.
 *
 * Param: cmd - integer identifier for the command being sent
 * Returns: cmd - The state identifier of the command being sent
 */
int send_command(int cmd)
{
	/* memset buffers */
	//memset(&send_buffer, 0, SNDBUFSIZE); // example


	size_t command_length = sizeof(char);
	char user_command = (char)(((int)'0')+cmd);
	printf("%c\n", user_command);

	init_connection(server_ip, server_port);
	printf("Connected\n");

	/* Send command string to the server */
	num_bytes_sent = 0;
	total_bytes_sent = 0;
	//memset(&user_command, 0, sizeof(cmd) + 1);
	while(total_bytes_sent < command_length) {
		num_bytes_sent = send(client_sock, &user_command, command_length, 0); 
		total_bytes_sent += num_bytes_sent;
	}
	if(num_bytes_sent != command_length) {
		switch_state(ERROR_STATE);
	}

	/* Get response */


	/* Read command name */
	printf("Read command name\n");
	memset(&command_name_buffer, 0, sizeof(char));
	num_bytes_rcvd = 0;
	total_bytes_rcvd = 0;
	while(total_bytes_rcvd < command_length) {
		num_bytes_rcvd = recv(client_sock, command_name_buffer, sizeof(char), 0);
		total_bytes_rcvd += num_bytes_rcvd;
	}

	printf("we have a command\n");
	printf("%c\n", command_name_buffer[0]);

	switch(command_name_buffer[0] - '0')
	{
		case(LIST):
		{
			printf("inside of LIST\n");
			
			memset(&filenames_length_buffer, 0, sizeof(int));
			num_bytes_rcvd = 0;
			total_bytes_rcvd = 0;
			while(total_bytes_rcvd < sizeof(int)) {
				num_bytes_rcvd = recv(client_sock, filenames_length_buffer, sizeof(int), 0);
				total_bytes_rcvd += num_bytes_rcvd;
			}

			// FIX
			int server_filenames_length = 100; //atoi(filenames_length_buffer);
			printf("server_filenames_length = %d\n", server_filenames_length);


			char serialized_server_filenames_buffer[server_filenames_length];
			memset(&serialized_server_filenames_buffer, 0, server_filenames_length);
			num_bytes_rcvd = 0;
			total_bytes_rcvd = 0;
			while(total_bytes_rcvd < server_filenames_length) {
				num_bytes_rcvd = recv(client_sock, serialized_server_filenames_buffer, server_filenames_length, 0);
				total_bytes_rcvd += num_bytes_rcvd;
				printf("total_bytes_rcvd = %d\n", total_bytes_rcvd);
				printf("serialized_server_filenames_buffer = %s\n", serialized_server_filenames_buffer);
			}
			
			printf("FINAL serialized buffer = %s\n", serialized_server_filenames_buffer);


			// tokenize filenames
			// print filenames
		}
		case(DIFF):
		{
			recv(client_sock, filenames_length_buffer, sizeof(size_t), 0);

			char serialized_server_filenames_buffer[sizeof(filenames_length_buffer)];
			recv(client_sock, serialized_server_filenames_buffer, sizeof(serialized_server_filenames_buffer), 0);

			// tokenize filenames
			// diff filenames
			// print diff
		}
		case(PLL1):
		{
			recv(client_sock, filenames_length_buffer, sizeof(size_t), 0);

			char serialized_server_filenames_buffer[sizeof(filenames_length_buffer)];
			recv(client_sock, serialized_server_filenames_buffer, sizeof(serialized_server_filenames_buffer), 0);

			// tokenize filenames
			// diff filenames
			// send diff to server
		}
		case(PLL3):
		{
			recv(client_sock, files_length_buffer, sizeof(size_t), 0);

			// receive files
		}
		case(LEAVE):
		{
			// do anything with connection?

			exit(1);
		}
		default:
			switch_state(ERROR_STATE);
	}


	/*if(strcmp(command_name_buffer, "LIST") == 0)
	{
		recv(client_sock, filenames_length_buffer, sizeof(size_t), 0);

		char serialized_server_filenames_buffer[sizeof(filenames_length_buffer)];
		recv(client_sock, serialized_server_filenames_buffer, sizeof(serialized_server_filenames_buffer), 0);

		printf("%s\n", strtok(serialized_server_filenames_buffer, ".mp3")); // for testing

		//char* reinflated_server_filenames[MAX_NUM_FILES] = strtok(serialized_server_filenames_buffer, ".mp3");
		int i_filename;
		for(i_filename = 0; i_filename < MAX_NUM_FILES; i_filename++)
		{
			if(reinflated_server_filenames[i_filename])
				printf("%s\n", reinflated_server_filenames[i_filename]);
		}
	/
	else if(strcmp(command_name_buffer, "DIFF") == 0)
	{
		recv(client_sock, filenames_length_buffer, sizeof(size_t), 0);

		char serialized_server_filenames_buffer[sizeof(filenames_length_buffer)];
		recv(client_sock, serialized_server_filenames_buffer, sizeof(serialized_server_filenames_buffer), 0);

		// have to actually diff them

	}
	else if(strcmp(command_name_buffer, "PLL1") == 0)
	{
		recv(client_sock, filenames_length_buffer, sizeof(size_t), 0);

		char serialized_server_filenames_buffer[sizeof(filenames_length_buffer)];
		recv(client_sock, serialized_server_filenames_buffer, sizeof(serialized_server_filenames_buffer), 0);
	}
	else if(strcmp(command_name_buffer, "PLL3") == 0)
	{
		recv(client_sock, files_length_buffer, sizeof(size_t), 0);

		int i_transmitted_file;
		for (i_transmitted_file = 0; i_transmitted_file < MAX_NUM_FILES; i_transmitted_file++)
		{
			//recv(client_sock, )
			// how long should socket stay open?
		}
	}
	else if(strcmp(command_name_buffer, "LEAF") == 0)
	{
		//probably have to close connection

		exit(1);
	}
	else
	{
		switch_state(ERROR_STATE);
	}*/


	/*while(total_bytes_rcvd < echo_string_len)
	{
		char buffer[RCVBUFSIZE];
		num_bytes = recv(client_sock, buffer, RCVBUFSIZE - 1, 0);

		if(num_bytes <= 0)
			switch_state(ERROR_STATE);
		
		total_bytes_rcvd += num_bytes;
		buffer[num_bytes] = '\0';

		fputs(buffer, stdout); // temp
	}*/

	//printf("End of send command.\n");

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
	else if(state == ERROR_STATE)
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
	create_tcp_socket(&client_sock);
	printf("Created TCP socket\n");

	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	printf("Constructed server address structure\n");

	/* Convert address */
	int ret_val = inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr.s_addr);
	if(ret_val <= 0) {
		switch_state(ERROR_STATE);
	}
	serv_addr.sin_port = htons(serv_port);
	printf("Converted address\n");

	/* Establish connection */
	if(connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("conn fail\n");
		switch_state(ERROR_STATE);
	}
	printf("Established connection\n");
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
			return ret_val;
			exit(0);
		}
	}

	printf("Same files");
	return(1);
}
