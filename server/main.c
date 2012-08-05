#include "bubble.h"

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
