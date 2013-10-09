/*///////////////////////////////////////////////////////////
*
* FILE:     	GTmyMusic.h
* AUTHOR:   	Devin Gonzalez & David Greenhalgh
* PROJECT:  	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  Custom header file for GTmyMusic.
*
*////////////////////////////////////////////////////////////

#ifndef GTmyMusic	/* Include guard */
#define GTmyMusic

	/* Included libraries */
	#include <stdio.h>          /* for printf() and fprintf() */
	#include <sys/socket.h>     /* for socket(), connect(), send(), and recv() */
	#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
	#include <stdlib.h>         /* supports all sorts of functionality */
	#include <unistd.h>         /* for close() */
	#include <string.h>         /* support any string ops */
	#include <openssl/evp.h>    /* for OpenSSL EVP digest libraries/SHA256 */
	#include <pthread.h>		/* for parallel processing */
	#include <dirent.h>

	/* Commands */
	#define LIST 0
	#define DIFF 1
	#define PULL 2
	#define PLL1 3
	#define PLL2 4
	#define PLL3 5
	#define LEAVE 6

	/* Message Structures */
	typedef struct {
		char command;
		int filenames_length;
		char* serialized_server_filenames;
	} list_message;

	typedef struct {
		char command;
		int filenames_length;
		char* serialized_server_filenames;
	} diff_message;

	typedef struct {
		char command;
		int filenames_length;
		char* serialized_server_filenames;
	} pull_message_1;

	typedef struct {
		char command;
		int filenames_length;
		char* serialized_client_filenames;
	} pull_message_2;

	typedef struct {
		char command;
		int files_length;
		FILE* server_files;
	} pull_message_3;

	typedef struct {
		char command;
	} leave_message;

	typedef struct {
		int socket;
		int socket_index;
	} command_handler_helper;

#endif
