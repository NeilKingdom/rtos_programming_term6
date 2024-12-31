#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

int main(int argc, char *argv[]) {
	dispatch_t *dpp = NULL;
	FILE *fp = NULL;
	my_message_t msg;
	name_attach_t *attach;
	int rcvid;
	char status[10];
	char value[255];

	if ((attach = name_attach(dpp, "mydevice", 0)) == NULL) {
		perror("name_attach()");
		exit(EXIT_FAILURE);
	}

	if ((fp = fopen("/dev/local/mydevice", "r+")) == NULL) {
		perror("fopen()");
		name_detach(attach, 0);
		exit(EXIT_FAILURE);
	}

	memset(status, 0, sizeof(status));
	memset(value, 0, sizeof(value));
	fscanf(fp, "%10s %s", status, value);

	if (strcmp(status, "status") == 0) {
		printf("Status: %s\n", value);
		if (strcmp(value, "closed") == 0) {
			name_detach(attach, 0);
			exit(EXIT_SUCCESS);
		}

		fflush(fp);
		fclose(fp);
	}

	while (1) {
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == 0) {
			if (msg.pulse.code == MY_PULSE_CODE) {
				printf("Small Integer: %d\n", msg.pulse.value.sival_int);

				if ((fp = fopen("/dev/local/mydevice", "r+")) == NULL) {
					perror("fopen()");
					name_detach(attach, 0);
					exit(EXIT_FAILURE);
				}

				memset(status, 0, sizeof(status));
				memset(value, 0, sizeof(value));
				fscanf(fp, "%10s %s", status, value);

				if (strcmp(status, "status") == 0) {
					printf("Status: %s\n", value);
					if (strcmp(value, "closed") == 0) {
						name_detach(attach, 0);
						exit(EXIT_SUCCESS);
					}
				}

				fflush(fp);
				fclose(fp);
			} else {
				fprintf(stderr, "Unexpected pulse\n");
				name_detach(attach, 0);
				exit(EXIT_SUCCESS);
			}
		}
	}

	/* Unreachable */
	name_detach(attach, 0);

	return EXIT_SUCCESS;
}
