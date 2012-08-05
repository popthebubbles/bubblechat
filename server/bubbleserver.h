#ifndef bubbleserver_h
#define bubbleserver_h

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
	
void sigchld_handler(int s);
void writePipe(int* pfd, int cmdOut);
void readPipe(int* pfd);

void* get_addr(struct sockaddr* addr);

int reap();
int check(int i, const char* msg);
int loopStructs(struct addrinfo *results, int* s);
int forkLoop(int lsock, struct sockaddr *client_addr, int* pipefd);
int shell(int* pipefd, pid_t pID);

struct addrinfo* setupStruct(struct addrinfo* input, const char* port, struct addrinfo* results);

	
#endif

