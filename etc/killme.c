#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void handler(int sig)
{
	exit(sig);
}


int main(int argc, char *argv[])
{
	int duration;
	if (argc > 1) 
	{
		duration = atoi(argv[1]);
		printf("Sleeping for %ds\n", duration);
		sleep(duration);
		exit(EXIT_SUCCESS);
	}
	if(signal(SIGQUIT, handler) == SIG_ERR)
		exit(EXIT_FAILURE);
	
	for (;;)
		pause();
}
