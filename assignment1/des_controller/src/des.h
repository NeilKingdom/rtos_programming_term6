#ifndef DES_MVA_H
#define DES_MVA_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

/*** DES_INPUTS ***/

/* Function decls */
void print_usage(char *);

/* Commands */
#define DES_CTL_LEFT_SCAN   "ls"
#define DES_CTL_RIGHT_SCAN  "rs"
#define DES_CTL_LEFT_OPEN   "lo"
#define DES_CTL_RIGHT_OPEN  "ro"
#define DES_CTL_LEFT_CLOSE  "lc"
#define DES_CTL_RIGHT_CLOSE "rc"
#define DES_CTL_LG_LOCK     "gll"
#define DES_CTL_RG_LOCK     "grl"
#define DES_CTL_LG_UNLOCK   "glu"
#define DES_CTL_RG_UNLOCK   "gru"
#define DES_CTL_WEIGHT      "ws"
#define DES_CTL_EXIT		"exit"

/* Message struct (client -> server) */
typedef struct
{
	char     data[100];	/* User input from des_input */
	unsigned extra;	    /* Additional data for punch card ID or weight */
} inputs_msg_t;

/* Response struct (server -> client) */
typedef struct
{
	int  code;		 	/* The exit status code */
	char notif[100]; 	/* The response message */
} inputs_rsp_t;

/* Exit status codes */
enum
{
	DES_CTL_UNEXPECTED = (_PULSE_CODE_MAXAVAIL + 1),	/* Unexpected pulse message */
	DES_CTL_OK,											/* Server processed message successfully */
	DES_CTL_INVAL,										/* The user's input was invalid */
	DES_CTL_EXTRA,										/* The server requires additional info */
	DES_CTL_DISCONNECT									/* Indication that the client wants to disconnect */
} inputs_rsp_codes;

/*** DES_CONTROLLER ***/

#define DES_CTL_CHID "des_ctl_chid"

typedef struct
{
	bool left_door_open;
	bool right_door_open;
	bool left_guard_locked;
	bool right_guard_locked;
	int users_waiting_left;
	int users_waiting_right;
	int user_weight;
	int user_id;
} mva_ctx_t;

/* Function decls */
void print_usage(char *);
void update_display(int, int);
void get_first_state(void);

int state_start(char *, int, int, int);
int state_gll_alpha(char *, int, int, int);
int state_grl_alpha(char *, int, int, int);
int state_lc_alpha(char *, int, int, int);
int state_rc_alpha(char *, int, int, int);
int state_opened(char *, int, int, int);
int state_weight(char *, int, int, int);
int state_lc_beta(char *, int, int, int);
int state_rc_beta(char *, int, int, int);
int state_guard_locked(char *, int, int, int);
int state_glu(char *, int, int, int);
int state_gru(char *, int, int, int);
int state_lo(char *, int, int, int);
int state_ro(char *, int, int, int);
int state_lc_gamma(char *, int, int, int);
int state_rc_gamma(char *, int, int, int);
int state_exit(void);

/*** DES_DISPLAY ***/

#define DES_DISP_CHID "des_disp_chid"
#define DES_DISP_LOG "des_disp_log"
#define DES_DISP_LOG_PULSE (_PULSE_CODE_MINAVAIL + 1)
#define BUF_SIZE 1000

#endif /* DES_MVA_H */
