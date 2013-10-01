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
#define START_STATE 100

/* Strings.xml */
char* commands[] = {"LIST", "DIFF", "PULL", "LEAVE"};

const char* badCmd = "Command not recognized, exiting now.\n";
const char* badNumCmds = "Improper number of args, exiting now.\nCommand line menu usage: ./musicClient\nDirect command usage: ./musicClient <command>\n";

/* Function pointers */
int sendCommand(int);
int menuInterface(int);
void printMainMenuOptions();

/* The main function */
int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		/* Command line interface */
		menuInterface(START_STATE);

	}
	else if(argc == 2)
	{
		/* Direct command */
		if(strcmp(argv[1], "list") == 0)
		{
			sendCommand(LIST);
		}
		else if(strcmp(argv[1], "diff") == 0)
		{
			sendCommand(DIFF);
		}
		else if(strcmp(argv[1], "pull") == 0)
		{
			sendCommand(PULL);
		}
		else if(strcmp(argv[1], "leave") == 0)
		{
			sendCommand(LEAVE);
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

int sendCommand(int cmd)
{
	char* userCommand = commands[cmd];
	printf("%s\n", userCommand);

	//menuInterface(START_STATE);
}

int menuInterface(int state)
{
	if(state == START_STATE)
	{
		printf("Welcome!\n");
		printMainMenuOptions();
		//printf("%d", getchar());
		sendCommand(getchar() - 49);
	}
}

void printMainMenuOptions()
{
	printf("Please enter the number corresponding to one of the following commands:\n");
	printf("(1) LIST\n(2) DIFF\n(3) PULL\n(4) LEAVE\n");
}
