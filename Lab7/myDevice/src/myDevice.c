#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>

char data[255];
int server_coid;

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {
	if ((server_coid = name_open("mydevice", 0)) == -1) {
		perror("name_open failed");
		return EXIT_FAILURE;
	}
	return iofunc_open_default(ctp, msg, handle, extra);
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nb = 0;

	if (data == NULL)
		return 0;

	nb = strlen(data);

	// test to see if we have already send the whole message
	if (ocb->offset == nb)
		return 0;

	// we will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	// set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	// copy data into reply buffer
	SETIOV(ctp->iov, data, nb);

	ocb->offset += nb;

	// update offset into our data used to determine start position for next read
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return _RESMGR_NPARTS(1);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nb = 0;

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		/* have all the data */
		char *buf;
		char *alert_msg;
		int i, small_integer;
		buf = (char*) (msg + 1);

		if (strstr(buf, "alert") != NULL) {
			for (i = 0; i < 2; i++) {
				alert_msg = strsep(&buf, " ");
			}
			small_integer = atoi(alert_msg);
			if (small_integer >= 1 && small_integer <= 99) {
				//FIXME :: replace getprio() with SchedGet()
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
						_PULSE_CODE_MINAVAIL, small_integer);
			} else {
				printf("Integer is not between 1 and 99.\n");
			}
		} else {
			strcpy(data, buf);
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int main(int argc, char *argv[]) {
	dispatch_t *dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch context\n", argv[0]);
		return EXIT_FAILURE;
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	if ((id = resmgr_attach(dpp, NULL, "/dev/local/mydevice", _FTYPE_ANY, 0,
			&connect_funcs, &io_funcs, &ioattr)) == -1) {
		fprintf(stderr, "%s: Unable to attach name\n", argv[0]);
		return EXIT_FAILURE;
	}

	ctp = dispatch_context_alloc(dpp);
	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}

	return EXIT_SUCCESS;
}
