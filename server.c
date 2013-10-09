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
#define PORT_NUMBER 	2013	/* Server port number */
#define MAX_PENDING 	5		/* Maximum outstanding connection requests. */
#define NUM_FILES       10

/* Function Prototypes */
/* Params will need to be fleshed out once we define them */
int list(int);
int diff();
int pull();
int leave();
int get_filenames_length(char*[]);
char* serialize_filenames(char*[], char*);
size_t get_server_files_length(size_t[]);
void command_handler(void*);

int server_socket;                          /* Server Socket */
int client_socket;                          /* Client Socket */
struct sockaddr_in server_address;          /* Local address */
struct sockaddr_in client_address;          /* Client address */
unsigned short server_port;                 /* Server port */
unsigned int address_length;                /* Length of address data struct */

int i_file;
FILE* server_files[NUM_FILES];
char* server_filenames[NUM_FILES];
size_t server_file_lengths[NUM_FILES];
int filenames_count = 0;  // Manage: inc++ and decr-- as needed!!

/* pthreads */
static pthread_t *server_threads;
static int busy_threads[MAX_PENDING] = { 0 };
static int created_flag = 0;

command_handler_helper helper_struct[MAX_PENDING];

int num_bytes_recv[MAX_PENDING] = { 0 };
int total_bytes_recv[MAX_PENDING] = { 0 };
int num_bytes_sent[MAX_PENDING] = { 0 };
int total_bytes_sent[MAX_PENDING] = { 0 };

/* 
 * The main function. 
 */
int main(int argc, char *argv[])
{
    /* memset buffers */
    memset(helper_struct, 0, sizeof(helper_struct));

    /* Read in local files */
    printf("Reading local files...\n");
	DIR *dir;
	struct dirent *ent;
	char cwd[1024];
	char *wd = getcwd(cwd, sizeof(cwd));//, sizeof(cwd));	
	char* full_dir = strcat(cwd, "/serverSongs/");

	if((dir = opendir(full_dir)) != NULL) {
		int count = 0;
		while ((ent = readdir (dir)) != NULL) {
			if((strcmp(ent->d_name, ".") != 0) &&
				(strcmp(ent->d_name, "..") != 0) &&
				(strcmp(ent->d_name, ".DS_Store") != 0))
			{
				server_filenames[count] = ent->d_name;
				count++;
                filenames_count++;
    		}
		}
	}

	/* Assign port number. */
	server_port = PORT_NUMBER;

    /* Allocate thread array. */
    server_threads = malloc(sizeof(pthread_t) * MAX_PENDING);

    /* Create new TCP Socket for incoming requests. */
    printf("Creating a new TCP Socket for incoming requests...\n");
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed");
    }
    else {
        printf("Socket Created\n");
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
        printf("listen thread looping...\n");

    	address_length = sizeof(client_address);

    	/* Accept incoming connection. */
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_length);
        printf("Connection accepted\n");

        created_flag = 0;
        while(created_flag == 0){
        	int i;
            for (i=0; i<MAX_PENDING; i++) {
                if (busy_threads[i] == 0)
                {
                    helper_struct[i].socket_index = i;
                    helper_struct[i].socket = client_socket;
                    pthread_create(&server_threads[i], NULL, (void*)&command_handler, &helper_struct[i]);
                    busy_threads[i] = 1;
                    created_flag = 1;
                    break; //exit for loop
                }                
            }
        }
    }

    close(client_socket);
    return(1);
}

/* Other functions: */
void command_handler(void* helper_struct) {
    command_handler_helper* p_helper_struct = (command_handler_helper*) helper_struct;

	char command_buffer[2];
    memset(&command_buffer, 0, 2 * sizeof(char));
    num_bytes_recv[p_helper_struct->socket_index] = 0;
    total_bytes_recv[p_helper_struct->socket_index] = 0;
    while(total_bytes_recv[p_helper_struct->socket_index] < sizeof(char)) {
        num_bytes_recv[p_helper_struct->socket_index] = recv(p_helper_struct->socket, command_buffer, 1, 0);
        total_bytes_recv[p_helper_struct->socket_index] += num_bytes_recv[p_helper_struct->socket_index];
    }

	printf("Command %c\n", command_buffer[0]);

	switch(command_buffer[0] - '0')
	{
		case(LIST):
			list(p_helper_struct->socket_index);
		case(DIFF):
			diff();
		case(PULL):
			pull();
		case(LEAVE):
			leave();
		default:
			printf("%s\n", "Invalid command");
	}

    // Cleanup socket and thread.
    close(p_helper_struct->socket);
    pthread_exit(NULL);  // may be overkill/not needed?
}

/* 
 * Command: LIST
 *
 * Sends the list of files currently on the server to the requesting client
 */
int list(int thread_index)
{
	list_message new_list_message;
	new_list_message.command = (char)(((int)'0')+LIST);
    //new_list_message.command_name = (char*) malloc(5);
    //strcpy(new_list_message.command_name, "LIST");
	new_list_message.filenames_length = get_filenames_length(server_filenames);
	printf("Server filenames_length = %d\n", new_list_message.filenames_length);

    char* serialized = (char*) malloc(new_list_message.filenames_length);;
    new_list_message.serialized_server_filenames = (char*) malloc(new_list_message.filenames_length);
    

    serialized = serialize_filenames(server_filenames, new_list_message.serialized_server_filenames);
    printf("return = %s\n", serialized);
    printf("param = %s\n", new_list_message.serialized_server_filenames);

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < sizeof(new_list_message.command)) {
        num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, &new_list_message.command, sizeof(new_list_message.command), 0);
        total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
    }

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < sizeof(new_list_message.filenames_length)) {
        num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, &new_list_message.filenames_length, sizeof(new_list_message.filenames_length), 0);
        total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
    }

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < new_list_message.filenames_length) {
	   num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, new_list_message.serialized_server_filenames, new_list_message.filenames_length, 0);
       total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
	}

    // Cleanup thread?
    pthread_exit(NULL);  // may be overkill/not needed?

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
	//strcpy(new_diff_message.command_name, "DIFF");
	new_diff_message.command = (char)(((int)'0')+DIFF);
	new_diff_message.filenames_length = get_filenames_length(server_filenames);
    char* spaghetti = "";
	new_diff_message.serialized_server_filenames = serialize_filenames(server_filenames, spaghetti);

	send(server_socket, &new_diff_message.command, sizeof(new_diff_message.command), 0);
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
	//strcpy(new_pull_message_1.command_name, "PLL1");
	new_pull_message_1.command = (char)(((int)'0')+PLL1);
	new_pull_message_1.filenames_length = get_filenames_length(server_filenames);
	char* spaghetti = "";
    new_pull_message_1.serialized_server_filenames = serialize_filenames(server_filenames, spaghetti);

	send(server_socket, &new_pull_message_1.command, sizeof(new_pull_message_1.command), 0);
	send(server_socket, &new_pull_message_1.filenames_length, sizeof(new_pull_message_1.filenames_length), 0);
	send(server_socket, &new_pull_message_1.serialized_server_filenames, sizeof(new_pull_message_1.serialized_server_filenames), 0);

	/* Pull message 2 */


	/* Pull message 3 */
	pull_message_3 new_pull_message_3;
	//strcpy(new_pull_message_3.command_name, "PLL3");
	new_pull_message_3.command = (char)(((int)'0')+PLL3);
	new_pull_message_3.files_length = get_server_files_length(server_file_lengths);
	// pull_message_3.server_files is sent below

	send(server_socket, &new_pull_message_3.command, sizeof(new_pull_message_3.command), 0);
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
	//strcpy(new_leave_message.command_name, "LEAF");
	new_leave_message.command = (char)(((int)'0')+LEAVE);

	send(server_socket, &new_leave_message.command, sizeof(new_leave_message.command), 0);

	return(0); // unused for now
}

/*
 * Returns the number of bytes in the list of filenames on the server
 */
int get_filenames_length(char* filenames[])
{
	int fn_length = 0;
	int i_filename;
	for (i_filename = 0; i_filename < NUM_FILES; i_filename++)
		fn_length += strlen(filenames[i_filename]) + 1;

	return fn_length;
}

char* serialize_filenames(char* filenames[], char* spaghetti)
{
    printf("inside serialize_filenames function\n");
    printf("song count = %d\n", filenames_count);

    char* p_temp = (char*) malloc(sizeof(spaghetti));
	int i_filename;

	for(i_filename = 0; i_filename < filenames_count; i_filename++)
	{
        printf("next filename = %s\n", filenames[i_filename]);
		strcat(strcat(spaghetti, filenames[i_filename]), "\n");
        printf("serialized filenames = %s\n", spaghetti);
	}

    p_temp = spaghetti;

    printf("%s\n", p_temp);

	return p_temp;
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
