/*
 * calc_message.h
 *
 *  Created on: Sep 26, 2018
 *      Author: hurdleg
 */

#ifndef CALC_MESSAGE_H_
#define CALC_MESSAGE_H_

#define SHM_RES_NAME "srvr_chid"

struct client_send
{
	int left_hand;
	char operator;
	int right_hand;
} typedef client_send_t;

#define SRVR_OK 				0
#define SRVR_UNDEFINED 			1
#define SRVR_INVALID_OPERATOR 	2
#define SRVR_OVERFLOW 			3
#define SRVR_UNEXPECTED 		4

struct server_response
{
	double answer;
	int statusCode; // [OK, UNDEFINED, INVALID_OPERATOR, OVERFLOW]
	char errorMsg[128];
} typedef server_response_t;

#endif /* CALC_MESSAGE_H_ */
