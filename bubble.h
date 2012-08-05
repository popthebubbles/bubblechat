#ifndef __bubble_h___
	#define __bubble_h__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10

#define true 1
#define false 0

/*    Pipeval values:    */
/*    0 = stop server    */
/*    1 = nothing        */
/*    2 = ...            */
enum pipeval {
	stop, restart, nothing
	} pval;

/* IMPORTANT: Returns 1 ON SUCCESS */
int check(int i, const char* msg)
{
	printf("Checking %s...", msg);
	if(i==-1) {
		perror(msg);
		return 0;
	}
	
	printf("No error.\n");
	return 1;
}

// Returns pipeval sent by shell
int readPipe(int* pfd) // For the server to receive commands from shell
{	
	read(pfd[0], &pval, sizeof pval);
}

void writePipe(int* pfd, int cmdOut) // For the shell to send commands to the server
{
	write(pfd[1], &cmdOut, sizeof cmdOut);
}

// Used in process handling
void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int reap() {
	struct sigaction sa;

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (!check(sigaction(SIGCHLD, &sa, NULL), "sigaction"))
		return 0;
}
#endif

