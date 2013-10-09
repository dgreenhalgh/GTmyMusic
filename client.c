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

#define MAX_NUM_FILES 25

/* Function Prototypes */
int send_command(int);
int switch_state(int);
void print_main_menu_options();
void init_connection(char*, unsigned short);
void create_tcp_socket(int*);
char* recieve_message();
int compare_files(char*);
int get_filenames_length(int, char*[]);
void serialize_filenames(int , char*[], char*);

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "PLL1", "PLL2", "PLL3", "LEAF"};

const char* bad_command = "Command not recognized, exiting now.\n";
const char* bad_number_of_commands = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Socket info */
int client_sock, i_file, num_files;
struct sockaddr_in serv_addr;

int num_bytes_recv = 0;
int total_bytes_recv = 0;
int num_bytes_sent = 0;
int total_bytes_sent = 0;

/* Send and Receive buffers */
char command_name_buffer[2];
char filenames_length_buffer[sizeof(int)];
char files_length_buffer[sizeof(size_t)];  // not sure what for?
off_t file_lengths[MAX_NUM_FILES];

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
	DIR *dir;
	struct dirent *ent;

	char cwd[1024];
	char *wd = getcwd(cwd, sizeof(cwd));
	
	char* full_dir = strcat(cwd, "/clientSongs/");

	struct stat st;

	if((dir = opendir(full_dir)) != NULL)
	{
		int count = 0;
		while ((ent = readdir (dir)) != NULL)
		{
			if((strcmp(ent->d_name, ".") != 0) &&
				(strcmp(ent->d_name, "..") != 0) &&
				(strcmp(ent->d_name, ".DS_Store") != 0))
			{
	    		long f_size;

	    		/*stat(ent->d_name, &st);
	    		file_lengths[count] = st.st_size;*/

	    		local_filenames[count] = ent->d_name;
	    		count++;
    		}
    		num_files = count;
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
			printf("LIST\n");
			send_command(LIST);
		}
		else if(strcmp(argv[1], "diff") == 0)
		{
			printf("DIFF\n");
			send_command(DIFF);
		}
		else if(strcmp(argv[1], "pull") == 0)
		{
			printf("PULL\n");
			send_command(PULL);
		}
		else if(strcmp(argv[1], "leave") == 0)
		{
			printf("LEAVE\n");
			send_command(LEAVE);
		}
		else
		{
			printf("%s", bad_command);
		}
	}
	else if((argc == 3) && (strcmp(argv[1], "-c") == 0))
	{
		compare_files(argv[2]);
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
	size_t command_length = sizeof(char);
	char user_command = (char)(((int)'0')+cmd);

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
	memset(&command_name_buffer, 0, sizeof(char));
	num_bytes_recv = 0;
	total_bytes_recv = 0;
	while(total_bytes_recv < command_length) {
		num_bytes_recv = recv(client_sock, command_name_buffer, sizeof(char), 0);
		total_bytes_recv += num_bytes_recv;
	}

	switch(command_name_buffer[0] - '0')
	{
		case(LIST):
		{
			memset(&filenames_length_buffer, 0, sizeof(int));
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < sizeof(int)) {
				num_bytes_recv = recv(client_sock, filenames_length_buffer, sizeof(int), 0);
				total_bytes_recv += num_bytes_recv;
			}

			int server_filenames_length = *(int*) filenames_length_buffer;

			char* serialized_client_filenames_buffer = (char*) malloc(server_filenames_length);
			memset(serialized_client_filenames_buffer, 0, server_filenames_length);
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < server_filenames_length) {
				num_bytes_recv = recv(client_sock, serialized_client_filenames_buffer, server_filenames_length, 0);
				total_bytes_recv += num_bytes_recv;
			}
			
			/* Tokenize filenames */
		  	char* server_filenames[100];

		    char s[2000];
		    strcpy(s, serialized_client_filenames_buffer);
		    char* t = strtok(s, "\n");
		    int c = 0;
		    while(t != NULL)
		    {
		        server_filenames[c] = t;
		        t = strtok(NULL, "\n");
		        c++;
		    }

		    /* List filenames */
		    int x;
		    for(x = 0; x < c; x++)
		    	printf("%s\n", server_filenames[x]);

		    break; // Necessary for Case:
		}
		case(DIFF):
		{			
			memset(&filenames_length_buffer, 0, sizeof(int));
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < sizeof(int)) {
				num_bytes_recv = recv(client_sock, filenames_length_buffer, sizeof(int), 0);
				total_bytes_recv += num_bytes_recv;
			}

			int server_filenames_length = *(int*) filenames_length_buffer;

			char* serialized_client_filenames_buffer = (char*) malloc(server_filenames_length);
			memset(serialized_client_filenames_buffer, 0, server_filenames_length);
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < server_filenames_length) {
				num_bytes_recv = recv(client_sock, serialized_client_filenames_buffer, server_filenames_length, 0);
				total_bytes_recv += num_bytes_recv;
			}

			/* Tokenize filenames */
	      	char* server_filenames[100];

		    char s[2000];
		    strcpy(s, serialized_client_filenames_buffer);
		    char* t = strtok(s, "\n");
		    int c = 0;
		    while(t != NULL)
		    {
			    server_filenames[c] = t;
			    t = strtok(NULL, "\n");
			    c++;
		    }

		    // diff against local filenames
		    char* diff[100];

		    int i_server, diff_id = 0, diff_len = 0;
		    for(i_server = 0; i_server < 10; i_server++)  // replace with num_serv_files
		    {
		        int i_local, no_match = 1;
		        for(i_local = 0; i_local < num_files; i_local++)
		        	no_match *= strcmp(local_filenames[i_local], server_filenames[i_server]);

		        if(no_match != 0)
		        {
		        	diff[diff_id] = server_filenames[i_server];
		            diff_id++;
		        }

		        diff_len = diff_id;
	      	}

	       	// print diff
	      	int i_diff;
	      	for(i_diff = 0; i_diff < diff_len; i_diff++) 
	        	printf("%s\n", diff[i_diff]);

    		break; // Necessary for Case:
		}
		case(PULL):
		{	
			memset(&filenames_length_buffer, 0, sizeof(int));
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < sizeof(int)) {
				num_bytes_recv = recv(client_sock, filenames_length_buffer, sizeof(int), 0);
				total_bytes_recv += num_bytes_recv;
			}

			int server_filenames_length = *(int*) filenames_length_buffer;

			char* serialized_client_filenames_buffer = (char*) malloc(server_filenames_length);
			memset(serialized_client_filenames_buffer, 0, server_filenames_length);
			num_bytes_recv = 0;
			total_bytes_recv = 0;
			while(total_bytes_recv < server_filenames_length) {
				num_bytes_recv = recv(client_sock, serialized_client_filenames_buffer, server_filenames_length, 0);
				total_bytes_recv += num_bytes_recv;
			}

			/* Tokenize filenames */
	      	char* server_filenames[100];

	      	char s[2000];
	       	strcpy(s, serialized_client_filenames_buffer);
	       	char* t = strtok(s, "\n");
	       	int c = 0;
	       	while(t != NULL)
	       	{
		        server_filenames[c] = t;
		        t = strtok(NULL, "\n");
		        c++;
	       	}

	      	/* Diff against local filenames */
	      	char* diff[100];

	      	int i_server, diff_id = 0, diff_len = 0;
	      	for(i_server = 0; i_server < 10; i_server++)  // replace with num_diff_files
	      	{
	        	int i_local, no_match = 1;
	        	for(i_local = 0; i_local < num_files; i_local++)
	          		no_match *= strcmp(local_filenames[i_local], server_filenames[i_server]);

	        	if(no_match != 0)
	        	{
	          		diff[diff_id] = server_filenames[i_server];
	          		diff_id++;
	        	}

	        	diff_len = diff_id;
	      	}

	      	/* Send diff to server (PLL2) */
	      	pull_message_2 new_pull_message_2;
		    new_pull_message_2.command = (char)(((int)'0')+PLL2);
		    new_pull_message_2.filenames_length = get_filenames_length(diff_len, diff);
		    //printf("Server filenames_length = %d\n", new_pull_message_2.filenames_length);

		    //char* serialized = (char*) malloc(new_pull_message_2.filenames_length);;
		    new_pull_message_2.serialized_client_filenames = (char*) malloc(new_pull_message_2.filenames_length);
			serialize_filenames(diff_len, diff, new_pull_message_2.serialized_client_filenames);

		    num_bytes_sent = 0;
		    total_bytes_sent = 0;
		    while(total_bytes_sent < sizeof(new_pull_message_2.command)) {
		        num_bytes_sent = send(client_sock, &new_pull_message_2.command, sizeof(new_pull_message_2.command), 0);
		        total_bytes_sent += num_bytes_sent;
		    }

		    num_bytes_sent = 0;
		    total_bytes_sent = 0;
		    while(total_bytes_sent < sizeof(new_pull_message_2.filenames_length)) {
		        num_bytes_sent = send(client_sock, &new_pull_message_2.filenames_length, sizeof(new_pull_message_2.filenames_length), 0);
		        total_bytes_sent += num_bytes_sent;
		    }

		    num_bytes_sent = 0;
		    total_bytes_sent = 0;
		    while(total_bytes_sent < new_pull_message_2.filenames_length) {
		       num_bytes_sent = send(client_sock, new_pull_message_2.serialized_client_filenames, new_pull_message_2.filenames_length, 0);
		       total_bytes_sent += num_bytes_sent;
		    }


		    /* Pull Message 3 */


		    break; // Necessary for Case:
		}
		case(PLL3):
		{
			printf("PLL3 ??\n");
		    break; // Necessary for Case:  // Never reached?
		}
		case(COMP):
		{

			break;
		}
		case(LEAVE):
		{
			// do anything with connection?

			exit(1);
		    break; // Necessary for Case:  // Never reached?
		}
		default:
			switch_state(ERROR_STATE);

		    break; // Necessary for Case:
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
		
		char command = getchar();
		switch (command - '0') {
			case(ONE):
			{
				printf("LIST\n");
				break;
			}
			case(TWO):
			{
				printf("DIFF\n");
				break;
			}
			case(THREE):
			{
				printf("PULL\n");
				break;
			}
			case(FOUR):
			{
				printf("LEAVE\n");
				break;
			}
			default:
				break;
		}
		send_command(command + ASCII_CORRECTOR); 

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

// Not hash compare
int compare_files(char* local_filename)
{
	long f_size;
	char* f_buffer;
	struct stat st;

	/* FILE* to CHAR* */
	FILE* l_file = fopen(local_filename, "r");
	/*fseek(l_file, 0L, SEEK_END);
	f_size = ftell(l_file);
	rewind(l_file);*/

	stat(local_filename, &st);
	f_size = st.st_size;

	printf("File: %s Size: %lu\n", local_filename, f_size);

	f_buffer = calloc(1, f_size);

	if(!f_buffer)
	{
		fclose(l_file);
		printf("eh\n");
		printf("alloc fails\n");
		exit(1);
	}
	

	if(1 != fread(f_buffer, f_size, 1, l_file))
	{
		fclose(l_file);
		free(f_buffer);
		printf("fread fails\n");
		exit(1);
	}

	printf("pased\n");
	hash_compare_message new_hash_compare_message;
	new_hash_compare_message.command = COMP;
	new_hash_compare_message.client_file_length = f_size;
	new_hash_compare_message.client_file_hash = hash(f_buffer);
	//new_hash_compare_message.client_file_hash_length = sizeof(new_hash_compare_message.client_file_hash_length);

	fclose(l_file);
	free(f_buffer);

	size_t command_length = sizeof(char);
	char user_command = (char)(((int)'0')+COMP);

	init_connection(server_ip, server_port);
	printf("Connected\n");

	/* Send command string to the server */
	num_bytes_sent = 0;
	total_bytes_sent = 0;
	//memset(&user_command, 0, sizeof(cmd) + 1);
	while(total_bytes_sent < command_length) {
		num_bytes_sent = send(client_sock, &new_hash_compare_message.command, command_length, 0); 
		total_bytes_sent += num_bytes_sent;
	}
	if(num_bytes_sent != command_length) {
		switch_state(ERROR_STATE);
	}

	num_bytes_sent = 0;
	total_bytes_sent = 0;
	char* serialized_client_file_len = (char*) malloc(new_hash_compare_message.client_file_length);
	while(total_bytes_sent < sizeof(new_hash_compare_message.client_file_length)) {
        num_bytes_sent = send(client_sock, &new_hash_compare_message.client_file_length, sizeof(new_hash_compare_message.client_file_length), 0);
        total_bytes_sent += num_bytes_sent;
    }

    /*num_bytes_sent = 0;
	total_bytes_sent = 0;
    char* serialized_client_file_hash_len = (char*) malloc(new_hash_compare_message.client_file_hash_length);
    while(total_bytes_sent < sizeof(new_hash_compare_message.client_file_hash_length))
    {
    	num_bytes_sent = send(client_sock, &new_hash_compare_message.client_file_hash_length, sizeof(new_hash_compare_message.client_file_hash_length), 0);
    	total_bytes_sent += num_bytes_sent;
    }*/

    num_bytes_sent = 0;
	total_bytes_sent = 0;
	char* serialized_hash = (char*) malloc(new_hash_compare_message.client_file_hash);
	while(total_bytes_sent < sizeof(new_hash_compare_message.client_file_hash)) {
       num_bytes_sent = send(client_sock, &new_hash_compare_message.client_file_hash, sizeof(new_hash_compare_message.client_file_hash), 0);
       total_bytes_sent += num_bytes_sent;
    }

    //TODO: serverside hash_cmp

	return(1);
}

/*
 * Returns the number of bytes in the list of filenames on the server
 */
int get_filenames_length(int file_count, char* filenames[])
{
	int fn_length = 0;
	int i_filename;
	for (i_filename = 0; i_filename < file_count; i_filename++)
		fn_length += strlen(filenames[i_filename]) + 1;

	return fn_length;
}

void serialize_filenames(int file_count, char* filenames[], char* spaghetti)
{
	int i_filename;
	for(i_filename = 0; i_filename < file_count; i_filename++)
	{
		strcat(strcat(spaghetti, filenames[i_filename]), "\n");
	}
}

unsigned hash(char *s)
{
	unsigned hashval;

	for(hashval = 0;  *s != '\0'; s++)
		hashval = *s + 31 * hashval;

	return hashval % HASHSIZE;
}
