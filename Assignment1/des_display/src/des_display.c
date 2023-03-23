#include "des.h"

int main(void)
{
	/* Initialize variables */
	char in_buf[BUF_SIZE];
	int status, log_fd, chid_fd, chid;
	struct _pulse pulse = { 0 };

	/* Create a channel for des_controller to connect to */
	chid = ChannelCreate(0);
	if (chid == -1)
	{
		perror("ChannelCreate()");
		exit(EXIT_FAILURE);
	}

	/* Write chid to shared memory */
	chid_fd = shm_open(DES_DISP_CHID, (O_CREAT | O_RDWR), 0777);
	if (chid_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	status = ftruncate(chid_fd, sizeof(chid));
	if (status == -1)
	{
		perror("ftruncate()");
		exit(EXIT_FAILURE);
	}
	write(chid_fd, (void *)&chid, sizeof(chid));

	/* Open the log file for reading */
	log_fd = shm_open(DES_DISP_LOG, (O_CREAT | O_RDWR), 0777);
	if (log_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}

	printf("des_display PID: %u\n", getpid());

	/* Program loop */
	while (true)
	{
		status = MsgReceivePulse(chid, &pulse, sizeof(struct _pulse), NULL);

		switch (status)
		{
			case EOK:
			{
				if (pulse.code == DES_DISP_LOG_PULSE)
				{
					/* Print the log to stdout */
					lseek(log_fd, 0L, SEEK_SET);
					read(log_fd, (void *)in_buf, sizeof(in_buf));
					fprintf(stdout, in_buf);
				}
				else
				{
					fprintf(stderr, "Received unknown pulse\n");
				}
				break;
			}
			default:
			{
				perror("MsgRecievePulse()");
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}
