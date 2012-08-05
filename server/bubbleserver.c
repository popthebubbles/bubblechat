#include "bubble.h"

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
				return 0;
			case restart:
				return 0;
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

// Shell()- returns 0 on success, 1 on program error, 2 on input error
int shell(int* pipefd, pid_t pID)
{
	char input[100];
	
	while(pval>1) {
		
		printf(">> ");
		
		/* Fork() to handle input and constantly place "nothing"
		   on the pipe so that the server (child that read()'s)
		   isn't waiting.                                     */
		if(!fork()) {
			writePipe(pipefd, nothing);
			exit(0);
		}
		else
		scanf("%s", input);
		
		if(!strcmp(input, "stop"))
			pval=stop;
		/* For now, don't bother with restart functionality
		else if(!strcmp(input, "restart"))
			pval=restart; */
		else
			pval=nothing;
			
		writePipe(pipefd, pval);
		
		if(kill(pID, 0)<0)
			break;
	}
	
	printf("Parent exiting...\n");
	return 0;
}

// Returns 0 on success, 1 on program error, 2 on input error
int main(int argc, char* argv[])
{
	int sockfd;
	int pipefd[2];
	pid_t pID;
	
	struct sockaddr_storage client_addr;
	struct addrinfo hints;
	struct addrinfo *res;
	
	socklen_t addrsize=sizeof client_addr;
	
	if(argc!=2) {
		fprintf(stderr, "Usage: bubbleserver <port>\n");
		return 2;
	}
	pval=nothing;
	system("clear");

	if((res=setupStruct(&hints, argv[1], res))==NULL)
		return 1;
	
	if(loopStructs(res, &sockfd)==-1)
		return 1;

	freeaddrinfo(res); // Free struct, we're done
	
	if(!check(listen(sockfd, BACKLOG), "listen"))
		return 1;
		
	if(!reap())
		return 1;
		
	printf("\nBubbleserver: Ready.\nWaiting for connections...\n");
	
	/* Set up pipeline and fork() */
	pipe(pipefd);
	pID=fork();
	if(pID<0) {
		perror("fork");
		return 1;
	}
	
	/* We are the parent process. The parent handles the shell
	   and pipes commands from the shell to the child. */
	if(pID>0) {
		close(pipefd[0]); // We're writing to pipe
		return shell(pipefd, pID);
	}
	
	/* Otherwise, we are the child. The child handles the server and
	   spawns further children to handle accept()'s */
	close(pipefd[1]); // We're reading from pipe
	
	if(!check(forkLoop(sockfd, (struct sockaddr *)&client_addr, pipefd), "forkLoop()"))
		return 1;
	
	/* For now, don't bother with restart functionality
	
	if(pval==restart) {
		printf("Restarting...\n");
		system("sleep 1 && ./bubbleserver 23416");
	} */
	
	if(!pID)
		printf("Child exiting...\n");

	return 0;
	
}
