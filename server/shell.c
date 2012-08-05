#include "bubbleserver.h"

void writePipe(int* pfd, int cmdOut) // For the shell to send commands to the server
{
	write(pfd[1], &cmdOut, sizeof cmdOut);
}

// Shell()- returns 0 on success, 1 on program error, 2 on input error
int shell(int* pipefd, pid_t pID)
{
	char input[100];
	
	while(pval>1) {
		printf("> ");
		
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

