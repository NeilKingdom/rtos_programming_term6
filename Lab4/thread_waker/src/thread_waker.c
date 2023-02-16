#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/stat.h>

#define UBLK_LEN (2+2) /* +1 for \n, +1 for NULL terminator */
#define BASE 10
#define SEM_VAL (unsigned)0
#define SEM_NAME "tf_sem"

int main(void) {
	int num_unblock;
	char usr_in[UBLK_LEN];
	struct stat mode;
	sem_t *psem = NULL;

	/* Initialize semaphore */
	mode.st_mode = (S_IRWXO | S_IRWXU);
	assert(SEM_VAL < SEM_VALUE_MAX);
	psem = sem_open(SEM_NAME, O_CREAT, mode.st_mode, SEM_VAL);
	if (psem == SEM_FAILED) {
		perror("sem_open()");
		exit(EXIT_FAILURE);
	}

	/* Program loop */
	while (true) {
		/* Get user input */
		printf("How many threads would you like to unblock? (Press 0 or q to quit): ");
		fflush(stdin);
		fgets(usr_in, UBLK_LEN, stdin);
		usr_in[UBLK_LEN - 1] = '\0';
		num_unblock = (int) strtol(usr_in, NULL, BASE);

		if (num_unblock == 0 || num_unblock == (int) 'q') {
			if (errno != EOK) {
				perror("strtol()");
			} else {
				printf("Exiting...\n");
			}
			sem_unlink(SEM_NAME);
			exit(EXIT_SUCCESS);
		}

		for (int i = 0; i < num_unblock; i++) {
			sem_post(psem);
		}
		printf("Unblocked %d threads\n", num_unblock);
	}
}
