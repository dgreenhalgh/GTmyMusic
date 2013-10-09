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
int diff(int);
int pull(int);
int comp(int);
int leave(int);
int get_filenames_length(char*[]);
char* serialize_filenames(char*[], char*);
size_t get_files_length(int, size_t[]);
void command_handler(void*);
void serialize_files(int, FILE*[], char*);

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

    //FILE* fp;
    off_t file_lengths[NUM_FILES];
    struct stat st;

	if((dir = opendir(full_dir)) != NULL) {
		int count = 0;
		while ((ent = readdir (dir)) != NULL) {
			if((strcmp(ent->d_name, ".") != 0) &&
				(strcmp(ent->d_name, "..") != 0) &&
				(strcmp(ent->d_name, ".DS_Store") != 0))
			{
                long f_size;

				server_filenames[count] = ent->d_name;

                printf("%s\n", ent->d_name);
                stat(ent->d_name, &st);
                file_lengths[count] = st.st_size;
                printf("%zd\n", file_lengths[count]);


                //server_files[count] = (FILE*)malloc(sizeof(fopen(ent->d_name, "r")));
                /*server_files[count] = fopen(ent->d_name, "r");
                fseek(server_files[count], 0L, SEEK_END);
                f_size = ftell(server_files[count]);
                rewind(server_files[count]);*/

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

    closedir(dir);
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

	switch(command_buffer[0] - '0')
	{
		case(LIST):
			list(p_helper_struct->socket_index);
		case(DIFF):
			diff(p_helper_struct->socket_index);
		case(PULL):
			pull(p_helper_struct->socket_index);
        case(COMP):
            comp(p_helper_struct->socket_index);
		case(LEAVE):
			leave(p_helper_struct->socket_index);
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
	new_list_message.filenames_length = get_filenames_length(server_filenames);

    char* serialized = (char*) malloc(new_list_message.filenames_length);
    new_list_message.serialized_server_filenames = (char*) malloc(new_list_message.filenames_length);
    serialized = serialize_filenames(server_filenames, new_list_message.serialized_server_filenames);

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
int diff(int thread_index)
{
    list_message new_list_message;
    new_list_message.command = (char)(((int)'0')+DIFF);
    new_list_message.filenames_length = get_filenames_length(server_filenames);

    char* serialized = (char*) malloc(new_list_message.filenames_length);
    new_list_message.serialized_server_filenames = (char*) malloc(new_list_message.filenames_length);
    serialized = serialize_filenames(server_filenames, new_list_message.serialized_server_filenames);

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
 * Command: PULL
 *
 * Diffs and downloads the files not on the client machine to the client 
 * machine
 */
int pull(int thread_index)
{
	/* Pull message 1 */ // aka LIST
    list_message new_list_message;
    new_list_message.command = (char)(((int)'0')+PULL);
    new_list_message.filenames_length = get_filenames_length(server_filenames);

    char* serialized = (char*) malloc(new_list_message.filenames_length);
    new_list_message.serialized_server_filenames = (char*) malloc(new_list_message.filenames_length);
    

    serialized = serialize_filenames(server_filenames, new_list_message.serialized_server_filenames);

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


	/* Pull message 2 */
    char filenames_length_buffer[sizeof(int)];
    memset(&filenames_length_buffer, 0, sizeof(int));
    num_bytes_recv[thread_index] = 0;
    total_bytes_recv[thread_index] = 0;
    while(total_bytes_recv[thread_index] < sizeof(int)) {
        num_bytes_recv[thread_index] = recv(helper_struct[thread_index].socket, filenames_length_buffer, sizeof(int), 0);
        total_bytes_recv[thread_index] += num_bytes_recv[thread_index];
    }

    int requested_filenames_length = *(int*) filenames_length_buffer;

    char* serialized_client_filenames_buffer = (char*) malloc(requested_filenames_length);
    memset(serialized_client_filenames_buffer, 0, requested_filenames_length);
    num_bytes_recv[thread_index] = 0;
    total_bytes_recv[thread_index] = 0;
    while(total_bytes_recv[thread_index] < requested_filenames_length) {
        num_bytes_recv[thread_index] = recv(helper_struct[thread_index].socket, serialized_client_filenames_buffer, requested_filenames_length, 0);
        total_bytes_recv[thread_index] += num_bytes_recv[thread_index];
    }
    
    /* Tokenize filenames */
    char* requested_filenames[100];

    char s[2000];
    strcpy(s, serialized_client_filenames_buffer);
    char* t = strtok(s, "\n");
    int diff_count = 0;
    while(t != NULL)
    {
        requested_filenames[diff_count] = t;
        t = strtok(NULL, "\n");
        diff_count++;
    }

printf("debug\n");

	/* Pull message 3 */
	pull_message_3 new_pull_message_3;
	new_pull_message_3.command = (char)(((int)'0')+PLL3);
	// new_pull_message_3.files_length = get_files_length(diff_count, server_file_lengths); // fix
    // new_pull_message_3.server_files = (FILE*) malloc(new_pull_message_3.files_length);

    // Get files from requested filenames.
    // Convert to one byte stream.
    FILE* requested_files[diff_count];
    char* serialized_files1 = (char*) malloc(1);
    char* serialized_files2 = (char*) malloc(1);
    int i;
    for (i=0; i<diff_count; i++) {
printf("debug\n");        

        char* path = strcat("./serverSongs/", requested_filenames[i]);
        requested_files[i] = fopen(path, "r");

        char* next_byte = (char*) malloc(1);

        while (*next_byte != EOF) {
printf("debug\n");            

            serialized_files1 = malloc(strlen(serialized_files2));
            *serialized_files1 = *serialized_files2;
            serialized_files2 = malloc(strlen(serialized_files1) + 1);

            fscanf(requested_files[i], "%c", next_byte);
            serialized_files2 = strcat(serialized_files1, next_byte);
        }
printf("debug\n");

        serialized_files1 = malloc(strlen(serialized_files2 + 1));
        *serialized_files1 = *serialized_files2;
        serialized_files2 = malloc(strlen(serialized_files1) + 1);

        serialized_files2 = strcat(serialized_files1, "'EOF'");
        fclose(requested_files[i]);
    }

    new_pull_message_3.files_length = strlen(serialized_files2);
    new_pull_message_3.server_files = serialized_files2;
    printf("%s\n", serialized_files2);

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < sizeof(new_pull_message_3.command)) {
        num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, &new_pull_message_3.command, sizeof(new_pull_message_3.command), 0);
        total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
    }

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < sizeof(new_pull_message_3.files_length)) {
        num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, &new_pull_message_3.files_length, sizeof(new_pull_message_3.files_length), 0);
        total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
    }

    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_sent[thread_index] < new_pull_message_3.files_length) {
       num_bytes_sent[thread_index] = send(helper_struct[thread_index].socket, new_pull_message_3.server_files, new_pull_message_3.files_length, 0);
       total_bytes_sent[thread_index] += num_bytes_sent[thread_index];
    }
		
	return(0); // unused for now
}

int comp(int thread_index)
{
    /* client_file_length*/
    char* client_file_length_buffer[sizeof(int)];
    memset(&client_file_length_buffer, 0, sizeof(int));

    num_bytes_recv[thread_index] = 0;
    total_bytes_recv[thread_index] = 0;
    while(total_bytes_recv[thread_index] < sizeof(int)) {
        num_bytes_recv[thread_index] = recv(helper_struct[thread_index].socket, client_file_length_buffer, sizeof(int), 0);
        total_bytes_recv[thread_index] += num_bytes_recv[thread_index];
    }

    int client_file_length = *(int*) client_file_length_buffer;

    /* client_file_hash_length */
    /*char* client_file_hash_length_buffer[sizeof(int)];
    memset(&client_file_hash_length_buffer, 0, sizeof(int));

    num_bytes_recv[thread_index] = 0;
    total_bytes_recv[thread_index] = 0;
    while(total_bytes_recv[thread_index] < sizeof(int)) {
        num_bytes_recv[thread_index] = recv(helper_struct[thread_index].socket, client_file_hash_length_buffer, sizeof(int), 0);
        total_bytes_recv[thread_index] += num_bytes_recv[thread_index];
    }

    int client_file_hash_length = *(int*) client_file_hash_length_buffer;*/

    /* client_file_hash */
    char* client_file_hash_buffer[sizeof(unsigned)];
    memset(client_file_hash_buffer, 0, sizeof(unsigned));
    num_bytes_sent[thread_index] = 0;
    total_bytes_sent[thread_index] = 0;
    while(total_bytes_recv[thread_index] < sizeof(unsigned))
    {
        num_bytes_recv[thread_index] = recv(helper_struct[thread_index].socket, client_file_hash_buffer, sizeof(unsigned), 0);
        total_bytes_recv[thread_index] += num_bytes_recv[thread_index];
    }

    unsigned client_file_hash = *(unsigned*) client_file_hash_buffer;

    /* Check lengths */
    /*int i_len;
    for(i_len = 0; i_len < NUM_FILES; i_len++)
    {
        long f_size = len(server_files[i_len]); // figure this out
        if(f_size == client_file_length)
        {
            char* f_buffer = calloc(1, f_size + 1);
            if(!f_buffer)
            {
                fclose(server_files[i_len]);
                printf("alloc fails\n");
                exit(1);
            }

            if(1 != fread(f_buffer, f_size, 1, server_files[i_len]))
            {
                fclose(server_files[i_len]);
                free(f_buffer);
                printf("fread fails\n");
                exit(1);
            }

            unsigned server_file_hash = hash(f_buffer);
            if(server_file_hash == client_file_hash)
            {
                // send filename to client
            }

        }
    }*/
}

/* 
 * Command: LEAVE
 *
 * Client machine and closes its connection with the server
 */
int leave(int thread_index)
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

void serialize_files(int file_count, FILE* files[], char* spaghetti)
{
    // for(int i_file = 0; i_file < file_count; i_file++) {
    //     strcat(strcat(spaghetti, files[i_file]), "\n");
    // }
}

char* serialize_filenames(char* filenames[], char* spaghetti)
{
    char* p_temp = (char*) malloc(sizeof(spaghetti));
	int i_filename;

	for(i_filename = 0; i_filename < filenames_count; i_filename++) {
		strcat(strcat(spaghetti, filenames[i_filename]), "\n");
	}

    p_temp = spaghetti;

	return p_temp;
}

size_t get_files_length(int file_count, size_t files_list[])
{
	size_t total_files_length = 0;
	int i_file_length;
	for(i_file_length = 0; i_file_length < file_count; i_file_length++)
	{
		total_files_length += files_list[i_file_length];
	}

	return total_files_length;
}

unsigned hash(char *s)
{
    unsigned hashval;

    for(hashval = 0;  *s != '\0'; s++)
        hashval = *s + 31 * hashval;

    return hashval % HASHSIZE;
}
