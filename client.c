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
const char* badNumCmds = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Function pointers */
int directCall(int);
int menuInterface(int);

/* The main function */
int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		/* Command line interface */

	}
	else if(argc == 2)
	{
		/* Direct command */
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
	else
	{
		printf("%s", badNumCmds);
	}
}

int directCall(int cmd)
{
	char* userCommand = commands[cmd];
	printf("%s\n", userCommand);
}


