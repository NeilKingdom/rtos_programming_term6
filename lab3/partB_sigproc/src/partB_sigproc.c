#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <sys/wait.h>

#define CHLD_LEN (2+1) /* +1 for NULL terminator */
#define BASE 10

static volatile atomic_int i;
static volatile sig_atomic_t usr1Happened = false;
static pid_t *chld_list = NULL;

/* Function declarations */

void chld_sig_hndlr(int);
void prnt_sig_hndlr(int);
void cleanup(void);

void chld_sig_hndlr(int signo)
{
	usr1Happened = true;
}

void prnt_sig_hndlr(int signo)
{
	cleanup();
}

void cleanup(void)
{
	int j;
	for (j = 0; j < i; j++) /* Manually terminate remaining children */
		kill(chld_list[j], SIGUSR1);
	while (waitpid(-1, NULL, WNOHANG) != -1) ;; /* Block until all children are terminated */
	printf("PID = %d: Children finished, parent exiting.\n", getpid());
	exit(EXIT_FAILURE);
}

int main(void)
{
	/* Declare variables */
	pid_t pid;
	char usr_in[CHLD_LEN];
	int numChildren;

	/* Initialize variables */
	const struct sigaction chld_sig_act = { .sa_handler = chld_sig_hndlr };
	const struct sigaction prnt_sig_act = { .sa_handler = prnt_sig_hndlr };

	/* Get user input */
	printf("Enter the number of children: ");
	do
	{
		fgets(usr_in, CHLD_LEN, stdin);
		fflush(stdin);
		usr_in[CHLD_LEN-1] = '\0';
	} while((numChildren = (int)strtol(usr_in, NULL, BASE)) == 0);
	chld_list = malloc(sizeof(pid_t) * numChildren); /* Allocate space for child list */

	printf("Num children: %d\n", numChildren);
	printf("PID = %d: Parent running...\n", getpid());

	if (sigaction(SIGUSR1, &prnt_sig_act, NULL) == -1) /* Attach signal handler */
	{
		fprintf(stderr, "PID = %d: sigaction() failed. %s\n", getpid(), errno);
		exit(EXIT_FAILURE);
	}

	/* Invoke fork() for each child requested */
	for (i = 0; i < numChildren; i++)
	{
		pid = fork();
		if (pid != -1)
		{
			if (pid == 0) /* Child process */
			{
				pid_t chld_pid = getpid();
				printf("PID = %d: Child running...\n", chld_pid);
				if (sigaction(SIGUSR1, &chld_sig_act, NULL) == -1) /* Attach signal handler */
				{
					fprintf(stderr, "PID = %d: sigaction() failed. %s\n", chld_pid, errno);
					_exit(EXIT_FAILURE);
				}
				while (!usr1Happened) ;; /* Block until SIGUSR1 is received */
				printf("PID = %d: Child received SIGUSR1.\n", chld_pid);
				printf("PID = %d: Child exiting.\n", chld_pid);
				_exit(EXIT_SUCCESS);
			}
			else /* Parent process */
			{
				pid_t chld_pid = pid;
				chld_list[i] = chld_pid; /* Add child to child process list */
			}
		}
		else /* fork() failed. Abort. */
		{
			fprintf(stderr, "PID = %d: fork() failed. %s\n", getpid(), errno);
			cleanup();
		}
	}

	while (waitpid(-1, NULL, WNOHANG) != -1) ;; /* Block until all children are terminated */
	printf("PID = %d: Children finished, parent exiting.\n", getpid());
	return EXIT_SUCCESS;
}
