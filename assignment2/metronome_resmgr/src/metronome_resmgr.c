#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <devctl.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdatomic.h>

#define START_CMD "start"
#define STOP_CMD  "stop"
#define PAUSE_CMD "pause"
#define QUIT_CMD  "quit"
#define SET_CMD   "set"

#define BASE 10
#define NUMDEVICES 2

#define SEC_TO_NANOSEC (1 * 1000 * 1000 * 1000)
#define SPI_TO_NANOSEC(x) ((long)(((((x) * 1000.0f) * 1000.0f)) * 1000.0f))

/* Extend the default OCB and iofunc_attr_t structs */
struct mtm_attr_s;
#define IOFUNC_ATTR_T struct mtm_attr_s
struct mtm_ocb_s;
#define IOFUNC_OCB_T struct mtm_ocb_s

/* These must come after the overrides above */
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

/* New declaration of iofunc_attr_t (must begin with an iofunc_attr_t struct */
typedef struct mtm_attr_s {
	iofunc_attr_t attr;
	char *name; 		/* Name of the device that attr is associated with */
	int bpm; 			/* Beats per minute */
	int timesig_top; 	/* The numerator of the time signature */
	int timesig_btm;	/* The denominator of the time signature */
} mtm_attr_t;

/* A list of device attributes for each device we want to open */
mtm_attr_t mtm_attrs[NUMDEVICES];

/* New declaration of iofunc_ocb_t (must begin with an iofunc_ocb_t struct */
typedef struct mtm_ocb_s {
	iofunc_ocb_t ocb;
	char *buffer;
	unsigned bufsize;
} mtm_ocb_t;

/* Base directory for the devices */
#define MTM_DIR "/dev/local/"

/* Full paths for each device */
char *devnames[NUMDEVICES] = {
	MTM_DIR "/metronome",
	MTM_DIR "/metronome-help"
};

/* Metronome time signature lookup table (yAxis = ts-bottom, xAxis = ts-top) */
float mtm_lookup[8 + 1][12 + 1] = {
/*	  0      1      2      3      4      5      6      7      8      9      10     11     12	*/
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 0 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 1 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 2 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 3 */
	{ -1.0f, -1.0f,  4.0f,  6.0f,  8.0f, 10.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 4 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 5 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 6 */
	{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }, /* 7 */
	{ -1.0f, -1.0f, -1.0f,  6.0f, -1.0f, -1.0f,  6.0f, -1.0f, -1.0f,  9.0f, -1.0f, -1.0f, 12.0f }  /* 8 */
};

typedef struct timesig_to_pattern {
	int timesig_top;
	int timesig_btm;
	const char **pattern;
} timesig_to_pattern;

const char *pattern2_4[]  = { "|1", "&", "2", "&\n" };
const char *pattern3_4[]  = { "|1", "&", "2", "&", "3", "&\n" };
const char *pattern4_4[]  = { "|1", "&", "2", "&", "3", "&", "4", "&\n" };
const char *pattern5_4[]  = { "|1", "&", "2", "&", "3", "&", "4", "-", "5", "-\n" };
const char *pattern3_8[]  = { "|1", "-", "2", "-", "3", "-\n" };
const char *pattern6_8[]  = { "|1", "&", "a", "2", "&", "a\n" };
const char *pattern9_8[]  = { "|1", "&", "a", "2", "&", "a", "3", "&", "a\n" };
const char *pattern12_8[] = { "|1", "&", "a", "2", "&", "a", "3", "&", "a", "4", "&", "a\n" };

timesig_to_pattern ttp[] = {
	{  2, 4, pattern2_4  },
	{  3, 4, pattern3_4  },
	{  4, 4, pattern4_4  },
	{  5, 4, pattern5_4  },
	{  3, 8, pattern3_8  },
	{  6, 8, pattern6_8  },
	{  9, 8, pattern9_8  },
	{ 12, 8, pattern12_8 }
};

/* General purpose function declarations */
void* metronome(void *arg);
float calc_spi(mtm_attr_t *mtm_attr);
bool parse_args(char **argv);
void print_usage(char **argv);

/* Resource Manager function overrides */
int io_read(resmgr_context_t *ctp, io_read_t *msg, mtm_ocb_t *mtm_ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, mtm_ocb_t *mtm_ocb);
mtm_ocb_t* mtm_ocb_calloc(resmgr_context_t *ctp, mtm_attr_t *mtm_attr);
void mtm_ocb_free(mtm_ocb_t *mtm_ocb);

/* Connect and I/O function override structs */
resmgr_connect_funcs_t connect_funcs;
resmgr_io_funcs_t io_funcs;

/* Functions relating to the OCB */
iofunc_funcs_t mtm_ocb_funcs = {
_IOFUNC_NFUNCS, mtm_ocb_calloc, mtm_ocb_free };

/* The mount structure which allows us to override the default iofunc_funcs_t struct */
iofunc_mount_t mtm_mount = {
	.conf = IOFUNC_PC_ACL,
	.funcs = &mtm_ocb_funcs
};

dispatch_t *dpp;
dispatch_context_t *ctp;

/* Custom message union */
typedef union {
	struct _pulse pulse;
} my_msg_t;

/* Custom pulses */
enum my_pulses {
	TIMER_PULSE = _PULSE_CODE_MINAVAIL,
	PAUSE_PULSE,
	STOP_PULSE,
	START_PULSE
};

static volatile atomic_bool running = true;
static int mtm_coid = -1;

/* Initialize the OCB */
mtm_ocb_t* mtm_ocb_calloc(resmgr_context_t *ctp, mtm_attr_t *mtm_attr) {
	mtm_ocb_t *mtm_ocb;

	if ((mtm_ocb = calloc(1, sizeof(mtm_ocb_t))) == NULL) {
		return NULL;
	}
	return mtm_ocb;
}

/* Free the OCB */
void mtm_ocb_free(mtm_ocb_t *mtm_ocb) {
	free(mtm_ocb->buffer);
	free(mtm_ocb);
}

/* Client is reading from the Resource Manager */
int io_read(resmgr_context_t *ctp, io_read_t *msg, mtm_ocb_t *mtm_ocb) {
	int status;
	size_t nb;

	status = iofunc_read_verify(ctp, msg, &mtm_ocb->ocb, NULL);
	if (status != EOK) {
		return status;
	}

	/* Ensure no special 'x' types in the message */
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		return ENOSYS;
	}

	/* Allocate the read buffer if it has not yet been allocated */
	if (mtm_ocb->buffer == NULL) {
		mtm_ocb->buffer = malloc(sizeof(char) * 2048);
		if (mtm_ocb->buffer == NULL) {
			return ENOMEM;
		}
	}

	/* If client is reading the metronome resmgr, return a metronome's status */
	if (strcmp(mtm_ocb->ocb.attr->name, devnames[0]) == 0) {
		float spi = calc_spi(mtm_ocb->ocb.attr);
		sprintf(mtm_ocb->buffer,
				"[metronome: "
				"%d beats/min, "
				"time signature %d/%d, "
				"secs-per-interval: %1.2f, "
				"nanoSecs: %ld]\n",
				mtm_ocb->ocb.attr->bpm,
				mtm_ocb->ocb.attr->timesig_top, mtm_ocb->ocb.attr->timesig_btm,
				spi,
				SPI_TO_NANOSEC(spi)
		);
	}
	/* If client is reading from metronome-help, return a help string */
	else if (strcmp(mtm_ocb->ocb.attr->name, devnames[1]) == 0) {
		strcpy(mtm_ocb->buffer,
				"Metronome Resource Manager (ResMgr)\n\n"
				"Usage: metronome <bpm> <ts-top> <ts-bottom>\n\n"
				"API:\n"
				"\tpause [1-9]                      - pause the metronome for 1-9 seconds\n"
				"\tquit                             - quit the metronome\n"
				"\tset <bpm> <ts-top> <ts-bottom>   - set the metronome to <bpm> ts-top/ts-bottom\n"
				"\tstart                            - start the metronome from stopped state\n"
				"\tstop                             - stop the metronome, use 'start' to resume\n\n"
		);
	}

	mtm_ocb->bufsize = strlen(mtm_ocb->buffer) + 1;

	/* Verify that the file pointer is within the buffer's range */
	if (mtm_ocb->ocb.offset > mtm_ocb->bufsize) {
		MsgReply(ctp->rcvid, 0, NULL, 0);
		return _RESMGR_NOREPLY ;
	}

	/* Get the lesser of the actual size of the data vs how much data the client requested to read */
	nb = min(mtm_ocb->bufsize - mtm_ocb->ocb.offset, msg->i.nbytes);

	/* Reply manually */
	if (nb) {
		MsgReply(ctp->rcvid, nb, mtm_ocb->buffer + mtm_ocb->ocb.offset, nb);
	} else {
		MsgReply(ctp->rcvid, 0, NULL, 0);
	}

	/* Update offset for next read */
	mtm_ocb->ocb.offset += nb;

	if (msg->i.nbytes > 0) {
		mtm_ocb->ocb.attr->attr.flags |= IOFUNC_ATTR_ATIME;
	}

	/* We already replied to the client, so don't send another response */
	return _RESMGR_NOREPLY ;
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, mtm_ocb_t *mtm_ocb) {
	int status;
	char const * const sep = " \n";
	char *argv[4];
	char *buffer, *cmd, *arg1, *arg2, *arg3;

	status = iofunc_write_verify(ctp, msg, &mtm_ocb->ocb, NULL);
	if (status != EOK) {
		return status;
	}

	/* Ensure no special 'x' types in the message */
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		return ENOSYS;
	}

	/* Check if message was 0 bytes */
	if (msg->i.nbytes == 0) {
		MsgReply(ctp->rcvid, 0, NULL, 0);
	}

	/* Otherwise, handle the write locally */
	else if (msg->i.nbytes <= ctp->size - sizeof(*msg)) {
		buffer = (char*)(msg + 1);

		fflush(stdout);

		/* If client is writing to the metronome resmgr, process the command */
		if (strcmp(mtm_ocb->ocb.attr->name, devnames[0]) == 0) {
			/* Get command arguments (if applicable) */
			cmd = strtok(buffer, sep);
			arg1 = strtok(NULL, sep);
			if (arg1 != NULL) {
				arg2 = strtok(NULL, sep);
			}
			if (arg2 != NULL) {
				arg3 = strtok(NULL, sep);
			}

			if (strcmp(cmd, PAUSE_CMD) == 0) {
#ifdef DEBUG
				printf("\nIn pause cmd\n");
#endif
				if (arg1 == NULL) {
					fprintf(stderr,
							"\nThe number of seconds to pause for was not specified\n");
				} else {
					if (strlen(arg1) != 1 || !isdigit((char)arg1[0])) {
						fprintf(stderr,
								"\nThe value provided for the number of seconds to pause for is invalid\n");
					} else {
						MsgSendPulse(mtm_coid, -1, PAUSE_PULSE, (int)arg1[0] - '0');
					}
				}
			} else if (strcmp(cmd, QUIT_CMD) == 0) {
#ifdef DEBUG
				printf("\nIn quit cmd\n");
#endif
				printf("\nExiting...\n");
				MsgSendPulse(mtm_coid, -1, _PULSE_CODE_MAXAVAIL, 0); /* Unblock metronome thread */
				running = false;
			} else if (strcmp(cmd, SET_CMD) == 0) {
#ifdef DEBUG
				printf("\nReceived set command\n");
#endif
				if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
					fprintf(stderr,
							"\nThe set command requires additional arguments\n");
				} else {
					argv[0] = NULL; /* parse_args() expects program name here */
					argv[1] = arg1;
					argv[2] = arg2;
					argv[3] = arg3;
					parse_args(argv);
					MsgSendPulse(mtm_coid, -1, START_PULSE, 0); /* Refresh the timer by calling start */
				}
			} else if (strcmp(cmd, START_CMD) == 0) {
#ifdef DEBUG
				printf("\nReceived start command\n");
#endif
				MsgSendPulse(mtm_coid, -1, START_PULSE, 0);
			} else if (strcmp(cmd, STOP_CMD) == 0) {
#ifdef DEBUG
				printf("\nReceived stop command\n");
#endif
				MsgSendPulse(mtm_coid, -1, STOP_PULSE, 0);
			} else {
				fprintf(stderr, "\nInvalid command\n");
			}
		}
		/* Writes are not allowed for help device */
		else if (strcmp(mtm_ocb->ocb.attr->name, devnames[1]) == 0) {
			fprintf(stderr, "\nWriting to this device is not permitted\n");
		}

		MsgReply(ctp->rcvid, msg->i.nbytes, NULL, 0);
		mtm_ocb->ocb.attr->attr.flags |= (IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME);
	}

	return _RESMGR_NOREPLY;
}

/* Calculates the seconds per interval */
float calc_spi(mtm_attr_t *mtm_attr) {
	float spb; /* Seconds per beat */
	float spm; /* Seconds per measure */
	float spi; /* Seconds per interval */

	spb = 60.0f / mtm_attr->bpm; /* 60 seconds / bpm */
	spm = spb * mtm_attr->timesig_top;
	spi = spm / mtm_lookup[mtm_attr->timesig_btm][mtm_attr->timesig_top];

	return spi;
}

/* The metronome thread */
void *metronome(void *args) {
	int chid, rcvid, prio, i, j = 0;
	long spi, seconds, nanosec;
	timer_t timer_id;
	struct sigevent timer_evt;
	struct itimerspec itime;
	struct sched_param sched_params;
	const char **pattern;
	my_msg_t msg;

	/* Create a channel to receive pulses on */
    chid = ChannelCreate(_NTO_CHF_PRIVATE);
    if (chid == -1) {
    	perror("ChannelCreate()");
    	_exit(EXIT_FAILURE);
    }

	/* Get priority */
    if (SchedGet(0, 0, &sched_params) != -1) {
       prio = sched_params.sched_priority;
    } else {
       prio = 10;
    }

    mtm_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (mtm_coid == -1) {
    	perror("ConnectAttach()");
    	_exit(EXIT_FAILURE);
    }

	timer_evt.sigev_notify = SIGEV_PULSE;
	timer_evt.sigev_coid = mtm_coid;
	timer_evt.sigev_priority = prio;
	timer_evt.sigev_code = TIMER_PULSE;

	/* Create the timer */
	timer_create(CLOCK_MONOTONIC, &timer_evt, &timer_id);

	/* NOTE: tv_nsec only supports range 0..999999999 */
	spi = SPI_TO_NANOSEC(calc_spi(&mtm_attrs[0]));
	seconds = spi / SEC_TO_NANOSEC;
	nanosec = spi % SEC_TO_NANOSEC;

	/*
		Set the initial delay and subsequent interval delay for the timer.
		NOTE: At least one of the initial times are required to be filled in.
	*/
	itime.it_value.tv_sec = seconds; 		/* Initial seconds */
	itime.it_value.tv_nsec = nanosec;		/* Initial nanoseconds */
	itime.it_interval.tv_sec = seconds; 	/* Interval seconds */
	itime.it_interval.tv_nsec = nanosec; 	/* Interval nanoseconds */

	/* Begin the timer */
	timer_settime(timer_id, 0, &itime, NULL);

	while (running) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {
			if (msg.pulse.code == TIMER_PULSE) {
				pattern = NULL;

				/* Find pattern matching our time signature */
				for (i = 0; i < (sizeof(ttp) / sizeof(timesig_to_pattern)); i++) {
					if (ttp[i].timesig_top == mtm_attrs[0].timesig_top
					&&  ttp[i].timesig_btm == mtm_attrs[0].timesig_btm) {
						pattern = ttp[i].pattern;
						break;
					}
				}

				if (pattern != NULL) {
					fputs(pattern[j], stdout);
					fflush(stdout);
					if (strstr(pattern[j++], "\n") != NULL) j = 0;
				}
			} else if (msg.pulse.code == PAUSE_PULSE) {
				spi = SPI_TO_NANOSEC(calc_spi(&mtm_attrs[0]));
				seconds = spi / SEC_TO_NANOSEC;
				nanosec = spi % SEC_TO_NANOSEC;

				itime.it_value.tv_sec = msg.pulse.value.sival_int;
				itime.it_value.tv_nsec = 0L;
				itime.it_interval.tv_sec = seconds;
				itime.it_interval.tv_nsec = nanosec;
				timer_settime(timer_id, 0, &itime, NULL); /* Re-arm the timer to go off after 'pause' seconds */
			} else if (msg.pulse.code == STOP_PULSE) {
				itime.it_value.tv_sec = 0;
				itime.it_value.tv_nsec = 0L;
				itime.it_interval.tv_sec = 0;
				itime.it_interval.tv_nsec = 0L;
				timer_settime(timer_id, 0, &itime, NULL); /* Disarm the timer */
			} else if (msg.pulse.code == START_PULSE) {
				spi = SPI_TO_NANOSEC(calc_spi(&mtm_attrs[0]));
				seconds = spi / SEC_TO_NANOSEC;
				nanosec = spi % SEC_TO_NANOSEC;

				itime.it_value.tv_sec = seconds;
				itime.it_value.tv_nsec = nanosec;
				itime.it_interval.tv_sec = seconds;
				itime.it_interval.tv_nsec = nanosec;
				timer_settime(timer_id, 0, &itime, NULL); /* Start timer */
			}
		}
	}

	/* Cleanup */
	timer_delete(timer_id);
	ConnectDetach(mtm_coid);
	ChannelDestroy(chid);

	return NULL;
}

/* Prints information regarding how the program is ran */
void print_usage(char **argv) {
	printf("Usage: %s <bpm> <ts-top> <ts-bottom>\n\n", argv[0]);
	exit(EXIT_SUCCESS);
}

/* Parse the command line arguments to ensure that they are valid */
bool parse_args(char **argv) {
	char *bpm         = argv[1];
	char *timesig_top = argv[2];
	char *timesig_btm = argv[3];
	long lbpm, ltimesig_top, ltimesig_btm;

	fflush(stdout);

	/* Validate bpm */
	lbpm = strtol(bpm, NULL, BASE);
	if (lbpm <= 0 || lbpm > INT_MAX) {
		fprintf(stderr, "\n%s: The value of bpm provided is invalid\n", strerror(errno));
		return false;
	}

	/* Validate ts-top */
	ltimesig_top = strtol(timesig_top, NULL, BASE);
	if (ltimesig_top <= 0 || ltimesig_top > 12) {
		fflush(stdout);
		fprintf(stderr, "\n%s: The value of ts-top provided is invalid\n", strerror(errno));
		return false;
	}

	/* Validate ts-bottom */
	ltimesig_btm = strtol(timesig_btm, NULL, BASE);
	if (ltimesig_btm <= 0 || ltimesig_btm > 8) {
		fprintf(stderr, "\n%s: The value of ts-bottom provided is invalid\n", strerror(errno));
		return false;
	}

	if (mtm_lookup[(int)ltimesig_btm][(int)ltimesig_top] == -1.0f) {
		fprintf(stderr, "\nThe time signature provided is not supported\n");
		return false;
	}

	/* Set device attribute members */
	for (int i = 0; i < NUMDEVICES; i++) {
		mtm_attrs[i].bpm 		 = (int)lbpm;
		mtm_attrs[i].timesig_top = (int)ltimesig_top;
		mtm_attrs[i].timesig_btm = (int)ltimesig_btm;
	}

	return true;
}

int main(int argc, char **argv) {
	int link_id;
	pthread_t tid;
	pthread_attr_t tattr;
	resmgr_attr_t resmgr_attr;
	struct sched_param param;

	/* Input validation */
	if (argc != (3 + 1)) {
		print_usage(argv);
		exit(EXIT_FAILURE);
	} else if (!parse_args(argv)) {
		exit(EXIT_FAILURE);
	}

	/* Initialize the thread attribute */
	param.sched_priority = 10;
	pthread_attr_init(&tattr);
	pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&tattr, &param);
	pthread_attr_setschedpolicy(&tattr, SCHED_RR);

	/* Create the metronome thread */
	tid = pthread_create(&tid, &tattr, metronome, NULL);

	/* Create the dispatch channel */
	dpp = dispatch_create_channel(-1, DISPATCH_FLAG_NOLOCK);
	if (dpp == NULL) {
		perror("dispatch_create_channel()");
		exit(EXIT_FAILURE);
	}

	/* Initialize resource manager attributes */
	memset(&resmgr_attr, 0, sizeof(resmgr_attr));
	resmgr_attr.nparts_max = 1;			/* Max IOV parts */
	resmgr_attr.msg_max_size = 2048; 	/* Max msg buffer size */

	/* Override the default connect and I/O function handlers */
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
				     _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	for (int i = 0; i < NUMDEVICES; i++) {
		/* Initialize the device attribute */
		iofunc_attr_init(&mtm_attrs[i].attr, (S_IFREG | 0666), NULL, NULL);
		mtm_attrs[i].attr.mount = &mtm_mount;
		mtm_attrs[i].name = strdup(devnames[i]);
		/* Attach the resource manager to the path namespace */
		link_id = resmgr_attach(
			dpp,
			&resmgr_attr,
			devnames[i],
			_FTYPE_ANY,
			0,
			&connect_funcs,
			&io_funcs,
			&mtm_attrs[i]
		);
		if (link_id == -1) {
			perror("resmgr_attach()");
			exit(EXIT_FAILURE);
		}
	}

	/* Create the dispatch context */
	ctp = dispatch_context_alloc(dpp);
	if (ctp == NULL) {
		perror("dispatch_context_alloc()");
		exit(EXIT_FAILURE);
	}

	/* Begin the dispatch handler */
	while (running) {
		if (dispatch_block(ctp) == NULL) {
			perror("dispatch_block()");
			exit(EXIT_FAILURE);
		}
		dispatch_handler(ctp);
	}

	/* Cleanup */
	resmgr_detach(dpp, link_id, _RESMGR_DETACH_ALL);
	dispatch_context_free(ctp);

	pthread_join(tid, NULL); /* Wait for metronome thread to die */

	return EXIT_SUCCESS;
}
