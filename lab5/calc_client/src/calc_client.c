#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include "calc_message.h"

#define BASE     10
#define PID_LEN  (10 + 1)
#define OPND_LEN (10 + 1)
#define OPTR_LEN (1 + 1)

void print_usage(char *);
void process_args(char **argv, pid_t *srv_pid_p, int *operand1_p, int *operand2_p, char *operator_p);

void print_usage(char *prog_name)
{
	printf("Usage: %s <calc-server-PID> left_hand operator right_hand\n", prog_name);
}

void process_args(char **argv, pid_t *srv_pid_p, int *operand1_p, int *operand2_p, char *operator_p)
{
	char pid_buf[PID_LEN];
	char operand1_buf[OPND_LEN];
	char operand2_buf[OPND_LEN];
	long pid_long, op1_long, op2_long;

	/* Process PID */
	strncpy(pid_buf, argv[1], PID_LEN);
	if (pid_buf[PID_LEN-1] != '\0')
	{
		fprintf(stderr, "Invalid PID\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	pid_long = strtol(pid_buf, NULL, BASE);
	if (errno != EOK)
	{
		perror("strtol()");
		exit(EXIT_FAILURE);
	}
	else if (pid_long < 1 || pid_long > UINT_MAX)
	{
		fprintf(stderr, "Invalid PID\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	*srv_pid_p = (pid_t)pid_long;

	/* Process operand 1 */
	strncpy(operand1_buf, argv[2], OPND_LEN);
	if (operand1_buf[OPND_LEN-1] != '\0')
	{
		fprintf(stderr, "Maximum number of placeholder digits allowed for operand is %d\n", OPND_LEN - 1);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	op1_long = strtol(operand1_buf, NULL, BASE);
	if (errno != EOK)
	{
		perror("strtol()");
		exit(EXIT_FAILURE);
	}
	else if (op1_long > INT_MAX)
	{
		fprintf(stderr, "Operand cannot be greater than %d\n", INT_MAX);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	else if (op1_long < INT_MIN)
	{
		fprintf(stderr, "Operand cannot be less than %d\n", INT_MIN);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	*operand1_p = (int)op1_long;

	/* Process operand 2 */
	strncpy(operand2_buf, argv[4], OPND_LEN);
	if (operand2_buf[OPND_LEN-1] != '\0')
	{
		fprintf(stderr, "Maximum number of placeholder digits allowed for operand is %d\n", OPND_LEN - 1);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	op2_long = strtol(operand2_buf, NULL, BASE);
	if (errno != EOK)
	{
		perror("strtol()");
		exit(EXIT_FAILURE);
	}
	else if (op2_long > INT_MAX)
	{
		fprintf(stderr, "Operand cannot be greater than %d\n", INT_MAX);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	else if (op2_long < INT_MIN)
	{
		fprintf(stderr, "Operand cannot be less than %d\n", INT_MIN);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	*operand2_p = (int)op2_long;

	/* Process operator (server checks if it is actually valid) */
	if (strlen(argv[3]) > 1)
	{
		fprintf(stderr, "Operator is invalid\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	*operator_p = (char)argv[3][0];
}

int main(int argc, char **argv)
{
	pid_t srv_pid;
	int operand1, operand2;
	int coid, chid, shm_fd, status;
	char operator;
	client_send_t msg;
	server_response_t rsp;

	if (argc != 5)
	{
		fprintf(stderr, "Incorrect number of arguments\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	process_args(argv, &srv_pid, &operand1, &operand2, &operator);
	msg.left_hand = operand1;
	msg.right_hand = operand2;
	msg.operator = operator;

	/* Get server's chid */

	shm_fd = shm_open(SHM_RES_NAME, (O_CREAT | O_RDWR), 0777);
	if (shm_fd == -1)
	{
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	read(shm_fd, (void *)&chid, sizeof(chid));

	/* Begin message passing */

	coid = ConnectAttach(ND_LOCAL_NODE, srv_pid, chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1)
	{
		perror("ConnectAttach()");
		exit(EXIT_FAILURE);
	}

	status = MsgSend(coid, &msg, sizeof(client_send_t), &rsp, sizeof(server_response_t));
	switch (status)
	{
		case -1:
		{
			perror("MsgSend()");
			exit(EXIT_FAILURE);
		}
		case SRVR_UNEXPECTED:
		{
			printf("Server was not prepared to receive our pulse\n");
			exit(EXIT_FAILURE);
		}
		default:
		{
			switch (rsp.statusCode)
			{
				case SRVR_OK:
				{
					printf("The server calculated the result of %u %c %u as %.2f\n",
							msg.left_hand, msg.operator, msg.right_hand, rsp.answer);
					break;
				}
				case SRVR_UNDEFINED:
				case SRVR_INVALID_OPERATOR:
				case SRVR_OVERFLOW:
				{
					printf("Error message from server: %s", rsp.errorMsg);
					break;
				}
				default:
				{
					fprintf(stderr, "Unexpected status code was returned by the server\n");
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	status = ConnectDetach(coid);
	if (status == -1)
	{
		perror("ConnectDetach()");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
