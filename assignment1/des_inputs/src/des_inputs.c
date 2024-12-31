#include "des.h"

void print_usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s <server_PID>\n", prog_name);
}

int main(int argc, char **argv)
{
	/* Initialize variables */
	char *tmp_buf = NULL;
	long line_limit;
	int shm_fd, coid, srv_chid;
	pid_t srv_pid;
	inputs_msg_t msg = { 0 };
	inputs_rsp_t rsp = { 0 };

	/* Input validation */
	if (argc != 2)
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Retrieve chid of des_controller */
	shm_fd = shm_open(DES_CTL_CHID, (O_CREAT | O_RDWR), 0777);
	if (shm_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	read(shm_fd, (void *)&srv_chid, sizeof(srv_chid));

	/* Connect to des_controller */
	srv_pid = (pid_t)atoi(argv[1]);
	coid = ConnectAttach(ND_LOCAL_NODE, srv_pid, srv_chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror("ConnectAttach()");
		exit(EXIT_FAILURE);
	}

	/* Allocate memory for user input buffer */
	line_limit = sysconf(_SC_LINE_MAX);
	tmp_buf = malloc(line_limit + 1);
	if (tmp_buf == NULL)
	{
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* Print list of valid commands */
	printf(
			"Valid commands:\n"
			"\t%s = Left Scan <punch_card_ID>\n"
			"\t%s = Right Scan <punch_card_ID>\n"
			"\t%s = Left door open\n"
			"\t%s = Right door open\n"
			"\t%s = Left door close\n"
			"\t%s = Right door close\n"
			"\t%s = Left guard unlock\n"
			"\t%s = Right guard unlock\n"
			"\t%s = Left guard lock\n"
			"\t%s = Right guard lock\n"
			"\t%s = Weight scale <weight>\n"
			"\t%s = Exit the program\n",
			DES_CTL_LEFT_SCAN, DES_CTL_RIGHT_SCAN,
			DES_CTL_LEFT_OPEN, DES_CTL_RIGHT_OPEN, DES_CTL_LEFT_CLOSE,
			DES_CTL_RIGHT_CLOSE, DES_CTL_LG_UNLOCK, DES_CTL_RG_UNLOCK,
			DES_CTL_LG_LOCK, DES_CTL_RG_LOCK, DES_CTL_WEIGHT,
			DES_CTL_EXIT
		  );

	/* Program loop */
	while (true)
	{
		/* Get user input */
		fgets(tmp_buf, line_limit + 1, stdin);
		tmp_buf[strlen(tmp_buf) - 1] = '\0';
		strncpy(msg.data, tmp_buf, sizeof(msg.data));
		msg.extra = 0;

		MsgSend(coid, &msg, sizeof(inputs_msg_t), &rsp, sizeof(inputs_rsp_t)); /* SEND blocked */

		switch (rsp.code) /* We switch on rsp.code rather than using return status of MsgSend() */
		{
			case -1: /* Error in MsgSend() */
			{
				perror("MsgSend()");
				break;
			}
			case DES_CTL_OK: /* Server processed the message successfully */
			{
				break;
			}
			case DES_CTL_UNEXPECTED: /* Sending a pulse failed */
			{
				fprintf(stderr, "Server was not prepared to handle our pulse\n");
				break;
			}
			case DES_CTL_EXTRA: /* Send additional information to server (e.g., punch card ID or weight) */
			{
				assert(rsp.notif != NULL);
				fprintf(stdout, rsp.notif);

				/* We assume the user inputs a valid number. No type checking required */
				fgets(tmp_buf, line_limit + 1, stdin);
				tmp_buf[strlen(tmp_buf) - 1] = '\0';
				msg.extra = (unsigned)atoi(tmp_buf);

				MsgSend(coid, &msg, sizeof(inputs_msg_t), &rsp, sizeof(inputs_rsp_t));
				break;
			}
			case DES_CTL_INVAL: /* Invalid input */
			{
				printf("Invalid input\n");
				break;
			}
			case DES_CTL_DISCONNECT: /* Server wants to disconnect. Terminating this process will allow it to exit. */
			{
				/* Cleanup */
				printf("%s exiting...\n", argv[0]);
				free(tmp_buf);
				ConnectDetach(coid);
				exit(EXIT_SUCCESS);
			}
			default: /* Unknown response code */
			{
				fprintf(stderr, "Received unknown response code\n");
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}
