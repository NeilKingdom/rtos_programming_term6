#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include "calc_message.h"

void calculate(const client_send_t, server_response_t *);
void calculate(const client_send_t msg, server_response_t *rsp_p)
{
	double left_hand = (double)msg.left_hand;
	double right_hand = (double)msg.right_hand;
	char operator = msg.operator;
	double result;

	switch (operator)
	{
		case '+':
		{
			result = left_hand + right_hand;
			if (result < INT_MIN || result > INT_MAX)
			{
				rsp_p->statusCode = SRVR_OVERFLOW;
				sprintf(rsp_p->errorMsg, "OVERFLOW: %d %c %d\n",
					msg.left_hand, msg.operator, msg.right_hand);
			}
			else
			{
				rsp_p->answer = result;
				rsp_p->statusCode = SRVR_OK;
			}
			break;
		}
		case '-':
		{
			result = left_hand - right_hand;
			if (result < INT_MIN || result > INT_MAX)
			{
				rsp_p->statusCode = SRVR_OVERFLOW;
				sprintf(rsp_p->errorMsg, "OVERFLOW: %d %c %d\n",
					msg.left_hand, msg.operator, msg.right_hand);
			}
			else
			{
				rsp_p->answer = result;
				rsp_p->statusCode = SRVR_OK;
			}
			break;
		}
		case 'x':
		{
			result = left_hand * right_hand;
			if (result < INT_MIN || result > INT_MAX)
			{
				rsp_p->statusCode = SRVR_OVERFLOW;
				sprintf(rsp_p->errorMsg, "OVERFLOW: %d %c %d\n",
					msg.left_hand, msg.operator, msg.right_hand);
			}
			else
			{
				rsp_p->answer = result;
				rsp_p->statusCode = SRVR_OK;
			}
			break;
		}
		case '/':
		{
			if (right_hand == 0)
			{
				rsp_p->statusCode = SRVR_UNDEFINED;
				sprintf(rsp_p->errorMsg, "UNDEFINED: %d %c %d\n",
					msg.left_hand, msg.operator, msg.right_hand);
				break;
			}

			result = left_hand / right_hand;
			if (result < INT_MIN || result > INT_MAX)
			{
				rsp_p->statusCode = SRVR_OVERFLOW;
				sprintf(rsp_p->errorMsg, "OVERFLOW: %d %c %d\n",
					msg.left_hand, msg.operator, msg.right_hand);
			}
			else
			{
				rsp_p->answer = left_hand / right_hand;
				rsp_p->statusCode = SRVR_OK;
			}
			break;
		}
		default:
		{
			rsp_p->statusCode = SRVR_INVALID_OPERATOR;
			sprintf(rsp_p->errorMsg, "INVALID OPERATOR: %c\n", msg.operator);
			break;
		}
	}
}

int main(void)
{
	int chid, rcvid, shm_fd, status;
	client_send_t msg;
	server_response_t rsp;

	chid = ChannelCreate(0);
	if (chid == -1)
	{
		perror("ChannelCreate()");
		exit(EXIT_FAILURE);
	}

	/* Store chid in shared memory for client to access */

	shm_fd = shm_open(SHM_RES_NAME, (O_CREAT | O_RDWR), 0777);
	if (shm_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	status = ftruncate(shm_fd, sizeof(chid));
	if (status == -1)
	{
		perror("ftruncate()");
		exit(EXIT_FAILURE);
	}
	write(shm_fd, (void *)&chid, sizeof(chid));

	/* Message passing loop */

	while (true)
	{
		rcvid = MsgReceive(chid, &msg, sizeof(client_send_t), NULL);
		switch (rcvid)
		{
			case -1:
			{
				perror("MsgRecieve()");
				exit(EXIT_FAILURE);
			}
			case 0:
			{
				printf("Received an unexpected pulse from the client\n");
				MsgError(rcvid, SRVR_UNEXPECTED);
				break;
			}
			default:
			{
				calculate(msg, &rsp);
				break;
			}
		}
		status = MsgReply(rcvid, rsp.statusCode, &rsp, sizeof(server_response_t));
		if (status == -1)
		{
			perror("MsgReply()");
			exit(EXIT_FAILURE);
		}
	}

	/* Cleanup */

	status = close(shm_fd);
	if (status == -1)
	{
		perror("close()");
	}
	status = shm_unlink("/tmp/srvr_chid");
	if (status == -1)
	{
		perror("shm_unlink()");
	}
	status = ChannelDestroy(chid);
	if (status == -1)
	{
		perror("ChannelDestroy()");
	}

	return EXIT_SUCCESS;
}
