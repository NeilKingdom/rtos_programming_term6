#include "des.h"

typedef int(*fn_ptr)(char *, int, int, int);
static fn_ptr next_state;
static mva_ctx_t context = { 0 };
static inputs_rsp_t rsp = { 0 };

int state_start(char *data, int id, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state START\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LEFT_SCAN) == 0)
	{
		if (id == 0)
		{
			strcpy(rsp.notif, "Please enter the user's ID: ");
			return DES_CTL_EXTRA;
		}

		context.user_id = id;
		update_display(coid, log_fd);
		next_state = &state_gll_alpha;
		return DES_CTL_OK;
	}
	else if (strcmp(data, DES_CTL_RIGHT_SCAN) == 0)
	{
		if (id == 0)
		{
			strcpy(rsp.notif, "Please enter the user's ID: ");
			return DES_CTL_EXTRA;
		}

		context.user_id = id;
		update_display(coid, log_fd);
		next_state = &state_grl_alpha;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_gll_alpha(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state GLL (alpha)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LG_UNLOCK) == 0)
	{
		context.left_guard_locked = false;
		update_display(coid, log_fd);
		next_state = &state_lc_alpha;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_grl_alpha(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state GRL (alpha)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RG_UNLOCK) == 0)
	{
		context.right_guard_locked = false;
		update_display(coid, log_fd);
		next_state = &state_rc_alpha;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_lc_alpha(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state LC (alpha)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LEFT_OPEN) == 0)
	{
		context.left_door_open = true;
		update_display(coid, log_fd);
		next_state = &state_opened;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_rc_alpha(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state RC (alpha)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RIGHT_OPEN) == 0)
	{
		context.right_door_open = true;
		update_display(coid, log_fd);
		next_state = &state_opened;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_opened(char *data, int weight, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state OPENED\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_WEIGHT) == 0)
	{
		if (weight == 0)
		{
			strcpy(rsp.notif, "Please enter the user's weight: ");
			return DES_CTL_EXTRA;
		}

		context.user_weight = weight;
		update_display(coid, log_fd);
		next_state = &state_weight;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_weight(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state WEIGHT\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LEFT_CLOSE) == 0
			&& context.left_door_open)
	{
		context.left_door_open = false;
		update_display(coid, log_fd);
		next_state = &state_lc_beta;
		return DES_CTL_OK;
	}
	if (strcmp(data, DES_CTL_RIGHT_CLOSE) == 0
			&& context.right_door_open)
	{
		context.right_door_open = false;
		update_display(coid, log_fd);
		next_state = &state_rc_beta;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_lc_beta(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state LC (beta)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LG_LOCK) == 0)
	{
		context.left_guard_locked = true;
		update_display(coid, log_fd);
		next_state = &state_guard_locked;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_rc_beta(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state RC (beta)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RG_LOCK) == 0)
	{
		context.right_guard_locked = true;
		update_display(coid, log_fd);
		next_state = &state_guard_locked;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_guard_locked(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state GUARD LOCKED\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LG_UNLOCK) == 0)
	{
		context.left_guard_locked = false;
		update_display(coid, log_fd);
		next_state = &state_glu;
		return DES_CTL_OK;
	}
	else if (strcmp(data, DES_CTL_RG_UNLOCK) == 0)
	{
		context.right_guard_locked = false;
		update_display(coid, log_fd);
		next_state = &state_gru;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_glu(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state GLU\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LEFT_OPEN) == 0)
	{
		context.left_door_open = true;
		update_display(coid, log_fd);
		next_state = &state_lo;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_gru(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state GRU\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RIGHT_OPEN) == 0)
	{
		context.right_door_open = true;
		update_display(coid, log_fd);
		next_state = &state_ro;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_lo(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state LO\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LEFT_CLOSE) == 0)
	{
		context.left_door_open = false;
		update_display(coid, log_fd);
		next_state = &state_lc_gamma;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_ro(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state RO\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RIGHT_CLOSE) == 0)
	{
		context.right_door_open = false;
		update_display(coid, log_fd);
		next_state = &state_rc_gamma;
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_lc_gamma(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state LC (gamma)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_LG_LOCK) == 0)
	{
		context.left_guard_locked = true;
		update_display(coid, log_fd);

		get_first_state(); /* Effectively restart the program */
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_rc_gamma(char *data, int unused, int coid, int log_fd)
{
#ifdef DEBUG
	printf("In state RC (gamma)\n");
#endif

	if (strcmp(data, DES_CTL_EXIT) == 0)
	{
		return state_exit();
	}
	else if (strcmp(data, DES_CTL_RG_LOCK) == 0)
	{
		context.right_guard_locked = true;
		update_display(coid, log_fd);

		get_first_state(); /* Effectively restart the program */
		return DES_CTL_OK;
	}

	return DES_CTL_INVAL;
}

int state_exit(void)
{
	return DES_CTL_DISCONNECT;
}

void get_first_state(void)
{
	context.left_door_open = false;
	context.right_door_open = false;
	context.left_guard_locked = true;
	context.right_guard_locked = true;
	context.user_id = 0;
	context.user_weight = 0;

	next_state = &state_start;
}

void update_display(int coid, int log_fd)
{
	int status;
	char out_buf[BUF_SIZE];
	char id_buf[100];
	char weight_buf[100];
	itoa(context.user_id, id_buf, 10);
	itoa(context.user_weight, weight_buf, 10);

	/* Copy context members to buffer */
	sprintf(out_buf,
			"State Summary:\n"
			"=======================================\n"
			"User's ID number = %s\n"
			"User's weight = %s\n"
			"Left door status = %s\n"
			"Right door status = %s\n"
			"Left guard status = %s\n"
			"Right guard status = %s\n"
			"=======================================\n\n",
			(context.user_id == 0) ? "Unknown" : id_buf,
			(context.user_weight) == 0 ? "Unknown" : weight_buf,
			(context.left_door_open) ? "Opened" : "Closed",
			(context.right_door_open) ? "Opened" : "Closed",
			(context.left_guard_locked) ? "Left guard locked" : "Left guard unlocked",
			(context.right_guard_locked) ? "Right guard locked" : "Right guard unlocked");

	/* Write to file */
	lseek(log_fd, 0L, SEEK_SET);
	write(log_fd, (void *)out_buf, sizeof(out_buf));

	/* Send pulse indicating that display can update */
	status = MsgSendPulse(coid, -1, DES_DISP_LOG_PULSE, 0);
	if (status == -1)
	{
		perror("MsgSendPulse()");
	}
}

void print_usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s <server_PID>\n", prog_name);
}

int main(int argc, char **argv)
{
	/* Initialize variables */
	int status, log_fd, chid_fd, coid,
	srv_pid, chid, srv_chid, rcvid;
	inputs_msg_t msg = { 0 };

	/* Input validation */
	if (argc != 2)
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Retrieve chid of des_display */
	chid_fd = shm_open(DES_DISP_CHID, (O_CREAT | O_RDWR), 0777);
	if (chid_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	read(chid_fd, (void *)&srv_chid, sizeof(srv_chid));

	/* Open the log file for read/write */
	log_fd = shm_open(DES_DISP_LOG, (O_CREAT | O_RDWR), 0777);
	if (log_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}

	/* Connect to des_display */
	srv_pid = (pid_t)atoi(argv[1]);
	coid = ConnectAttach(ND_LOCAL_NODE, srv_pid, srv_chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror("ConnectAttach()");
		exit(EXIT_FAILURE);
	}

	/* Create channel for des_inputs to connect to */
	chid = ChannelCreate(0);
	if (chid == -1)
	{
		perror("ChannelCreate()");
		exit(EXIT_FAILURE);
	}

	/* Store chid in shared memory */
	chid_fd = shm_open(DES_CTL_CHID, (O_CREAT | O_RDWR), 0777);
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

	/* Print PID and return first state of TT */
	printf("des_controller PID: %u\n", getpid());
	get_first_state();

	/* Program loop */
	while (true)
	{
		rcvid = MsgReceive(chid, &msg, sizeof(inputs_msg_t), NULL); /* RECEIVE blocked */

		switch (rcvid)
		{
			case -1: /* Error in MsgReceive() */
			{
				perror("MsgRecieve()");
				rsp.code = -1;
				break;
			}
			case 0: /* Received a pulse */
			{
				fprintf(stderr, "Received unexpected pulse\n");
				rsp.code = DES_CTL_UNEXPECTED;
				break;
			}
			default: /* Transition to the next state in the TT */
			{
				assert(next_state != NULL);
				rsp.code = (*next_state)(msg.data, msg.extra, coid, log_fd);
				break;
			}
		}

		/* Reply to des_inputs with response struct */
		status = MsgReply(rcvid, 0, &rsp, sizeof(inputs_rsp_t)); /* REPLY blocked */
		if (status == -1)
		{
			perror("MsgReply()");
		}
	}

	return EXIT_SUCCESS;
}
