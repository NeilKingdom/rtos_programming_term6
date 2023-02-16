#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define TAG "PID = %d : "

static volatile sig_atomic_t usr1Happened = false;

static void sig_hndlr(int);
static void sig_hndlr(int signo)
{
	usr1Happened = true;
}

int main(void)
{
	pid_t pid = getpid();
	const struct sigaction sig_act = {
      .sa_handler = sig_hndlr,
      .sa_flags = SA_SIGINFO, /* Queue signals if multiple */
	};

	printf(TAG "Running... \n", pid);

	if (sigaction(SIGUSR1, &sig_act, NULL) == -1) /* Attach signal handler */
	{
		fprintf(stderr, "sigaction failed. %s\n", errno);
		exit(EXIT_FAILURE);
	}
	while (!usr1Happened) ;; /* Block */

	printf(TAG "Received SIGUSR1.\n", pid);
	printf(TAG "Exiting.\n", pid);
	return EXIT_SUCCESS;
}
