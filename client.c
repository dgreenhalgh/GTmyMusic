/* Devin Gonzalez & David Greenhalgh */
/* CS 3251, Project 2 */
/* client.c */

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

/* Constants */
#define LIST 0
#define DIFF 1
#define PULL 2
#define LEAVE 3

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "LEAVE"};

const char* badCmd = "Command not recognized, exiting now.\n";

/* Function pointers */
int directCall(int);

/* The main function */
int main(int argc, char *argv[])
{
	//printf("%d", argc);
	/* Command line interface */
	if(argc == 1)
	{

	}

	/* Direct command */
	if(argc == 2)
	{
		if(strcmp(argv[1], "list") == 0)
		{
			directCall(LIST);
		}
		else if(strcmp(argv[1], "diff") == 0)
		{
			directCall(DIFF);
		}
		else if(strcmp(argv[1], "pull") == 0)
		{
			directCall(PULL);
		}
		else if(strcmp(argv[1], "leave") == 0)
		{
			directCall(LEAVE);
		}
		else
		{
			printf("%s", badCmd);
		}


	}
}

int directCall(int cmd)
{
	char* userCommand = commands[cmd];
	printf("%s", userCommand);
}
