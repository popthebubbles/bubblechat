#include "bubbleserver.h"

// Used in process handling
void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Returns pipeval sent by shell
void readPipe(int* pfd) // For the server to receive commands from shell
{	
	read(pfd[0], &pval, sizeof pval);
}

// Returns pointer to struct filled out by getaddrinfo(), NULL on error
struct addrinfo* setupStruct(struct addrinfo* input, const char* port, struct addrinfo* results)
{
	memset(input, 0, sizeof *input);
	input->ai_family=AF_UNSPEC;
	input->ai_socktype=SOCK_STREAM;
	input->ai_flags=AI_PASSIVE;
	
	int status=getaddrinfo(NULL, port, input, &results);
	if(status!=0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return NULL;
	}
	
	return results;
}

// Returns socket on success, -1 on error
int loopStructs(struct addrinfo *results, int* s)
{
	struct addrinfo* p; int yes=1;
	
	for(p=results; p!=NULL; p=p->ai_next)
	{
		*s=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(!check(*s, "server: socket"))
			continue;
		
		if(!check(setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)), "setsockopt"))
			return -1;
		
		if(!check(bind(*s, p->ai_addr, (socklen_t)p->ai_addrlen), "server: bind")) {
			close(*s);
			continue;
		}
		
		break;		
	}
	if(p==NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}
	
	return *s;
}

// Returns pointer for either sin_addr or sin6_addr
void* get_addr(struct sockaddr* addr)
{
	if(addr->sa_family==AF_INET)
		return &(((struct sockaddr_in *)addr)->sin_addr);
		
	return &(((struct sockaddr_in6 *)addr)->sin6_addr);
}

// Returns 0 on success, -1 on error
int forkLoop(int lsock, struct sockaddr *client_addr, int* pipefd)
{
	char client[INET6_ADDRSTRLEN];
	char* buffer;
	char* msg="Hello World!\n";
	int asock, status, len;
	socklen_t addrsize=sizeof *client_addr;
	
	while(pval) {
		readPipe(pipefd); // Check to see if any commands came in from the shell
		switch(pval) {
			case stop:
				exit(0);
			case restart:
				exit(0);
			case nothing:
				break;
			default: 
				return -1;
		}
		
		asock=accept(lsock, client_addr, &addrsize);
		if(!check(asock, "accept"))
			return -1;
		
		inet_ntop(client_addr->sa_family, get_addr(client_addr), client, sizeof client);
		printf("Received connection from %s\n", client);
		
		if(!fork()) { // fork() returns 0- child
			close(lsock);
			len=strlen(msg);
			status=send(asock, msg, len, 0);
			if(!check(status, "send"))
				return -1;
			close(asock);
			return 0;
		}
		else // parent or error
			close(asock); // Don't need this socket descriptor
			
	}
	return 0;
}

