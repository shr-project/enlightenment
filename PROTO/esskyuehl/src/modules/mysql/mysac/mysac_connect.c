/*
 * Copyright (c) 2009-2011 Thierry FOURNIER
 *
 * This file is part of MySAC.
 *
 * MySAC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License
 *
 * MySAC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MySAC.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>

#include "mysac.h"
#include "mysac_net.h"
#include "mysac_utils.h"
#include "mysac_decode_respbloc.h"

#define MYSAC_CLIENT_MULTI_STATEMENTS 0x01 /* Enable/disable multi-stmt support */
#define MYSAC_CLIENT_MULTI_RESULTS    0x02 /* Enable/disable multi-results */


int mysac_connect(MYSAC *mysac) {
	int err;
	int errcode;
	int i;
	int len;

	switch (mysac->qst) {

	/***********************************************
	 network connexion
	***********************************************/
	case MYSAC_START:
		err = mysac_socket_connect(mysac->addr, &mysac->fd);
		if (err != 0) {
			mysac->qst = MYSAC_START;
			mysac->errorcode = err;
			return err;
		}
		mysac->qst = MYSAC_CONN_CHECK;
		return MYERR_WANT_READ;

	/***********************************************
	 check network connexion
	***********************************************/
	case MYSAC_CONN_CHECK:
		err = mysac_socket_connect_check(mysac->fd);
		if (err != 0) {
			close(mysac->fd);
			mysac->qst = MYSAC_START;
			mysac->errorcode = err;
			return err;
		}
		mysac->qst = MYSAC_READ_GREATINGS;
		mysac->len = 0;
		mysac->readst = 0;
		mysac->read = mysac->buf;
		mysac->read_len = mysac->bufsize;

	/***********************************************
	 read greatings
	***********************************************/
	case MYSAC_READ_GREATINGS:

		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		else if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* ok */
		else if (err != MYSAC_RET_DATA) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* decode greatings */
		i = 0;

		/* protocol */
		mysac->protocol = mysac->buf[i];
		i++;

		/* version */
		mysac->version = &mysac->buf[i];

		/* search \0 */
		while (mysac->buf[i] != 0)
			i++;
		i++;

		/* thread id */
		mysac->threadid = uint4korr(&mysac->buf[i]);

		/* first part of salt */
		strncpy(mysac->salt, &mysac->buf[i+4], SCRAMBLE_LENGTH_323);
		i += 4 + SCRAMBLE_LENGTH_323 + 1;

		/* options */
		mysac->options = uint2korr(&mysac->buf[i]);

		/* charset */
		mysac->charset = mysac->buf[i+2];

		/* server status */
		mysac->status = uint2korr(&mysac->buf[i+3]);

		/* salt part 2 */
		strncpy(mysac->salt + SCRAMBLE_LENGTH_323, &mysac->buf[i+5+13],
		        SCRAMBLE_LENGTH - SCRAMBLE_LENGTH_323);
		mysac->salt[SCRAMBLE_LENGTH] = '\0';

		/* checks */
		if (mysac->protocol != PROTOCOL_VERSION)
			return CR_VERSION_ERROR;

		/********************************
		  prepare auth packet
		********************************/

		/* set m->buf number */
		mysac->packet_number++;
		mysac->buf[3] = mysac->packet_number;

		/* set options */
		if (mysac->options & CLIENT_LONG_PASSWORD)
			mysac->flags |= CLIENT_LONG_PASSWORD;
		mysac->flags |= CLIENT_LONG_FLAG   |
		                CLIENT_PROTOCOL_41 |
		                CLIENT_SECURE_CONNECTION;
		to_my_2(mysac->flags, &mysac->buf[4]);

		/* set extended options */
		to_my_2(MYSAC_CLIENT_MULTI_RESULTS, &mysac->buf[6]);

		/* max m->bufs */
		to_my_4(0x40000000, &mysac->buf[8]);

		/* charset */
		/* 8: swedish */
		mysac->buf[12] = 8;

		/* 24 unused */
		memset(&mysac->buf[13], 0, 24);

		/* username */
		strcpy(&mysac->buf[36], mysac->login);
		i = 36 + strlen(mysac->login) + 1;

		/* password CLIENT_SECURE_CONNECTION */
		if (mysac->options & CLIENT_SECURE_CONNECTION) {

			/* the password hash len */
			mysac->buf[i] = SCRAMBLE_LENGTH;
			i++;
			scramble(&mysac->buf[i], mysac->salt, mysac->password);
			i += SCRAMBLE_LENGTH;
		}

		/* password ! CLIENT_SECURE_CONNECTION */
		else {
			scramble_323(&mysac->buf[i], mysac->salt, mysac->password);
			i += SCRAMBLE_LENGTH_323 + 1;
		}

		/* Add database if needed */
		if ((mysac->options & CLIENT_CONNECT_WITH_DB) &&
		    (mysac->database != NULL)) {
			/* TODO : debordement de buffer */
			len = strlen(mysac->database);
			memcpy(&mysac->buf[i], mysac->database, len);
			i += len;
			mysac->buf[i] = '\0';
		}

		/* len */
		to_my_3(i-4, &mysac->buf[0]);
		mysac->len = i;
		mysac->send = mysac->buf;
		mysac->qst = MYSAC_SEND_AUTH_1;

	/***********************************************
	 send paquet
	***********************************************/
	case MYSAC_SEND_AUTH_1:
		err = mysac_write(mysac->fd, mysac->send, mysac->len, &errcode);

		if (err == -1)
			return errcode;

		mysac->len -= err;
		mysac->send += err;
		if (mysac->len > 0)
			return MYERR_WANT_WRITE;

		mysac->qst = MYSAC_RECV_AUTH_1;
		mysac->readst = 0;
		mysac->read = mysac->buf;
		mysac->read_len = mysac->bufsize;

	/***********************************************
	 read response 1
	***********************************************/
	case_MYSAC_RECV_AUTH_1:
	case MYSAC_RECV_AUTH_1:
	/*
		MYSAC_RET_EOF,
		MYSAC_RET_OK,
		MYSAC_RET_ERROR,
		MYSAC_RET_DATA
	*/
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_BOTH);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* ok */
		else if (err == MYSAC_RET_OK)
			return 0;

		/*
		   By sending this very specific reply server asks us to send scrambled
		   password in old format.
		*/
		else if (mysac->packet_length == 1 && err == MYSAC_RET_EOF &&
		         mysac->options & CLIENT_SECURE_CONNECTION) {
			/* continue special paquet after conditions */
		}

		/* protocol error */
		else {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* send scrambled password in old format */

		/* set packet number */
		mysac->packet_number++;
		mysac->buf[3] = mysac->packet_number;

		/* send scrambled password in old format. */
		scramble_323(&mysac->buf[4], mysac->salt, mysac->password);
		mysac->buf[4+SCRAMBLE_LENGTH_323] = '\0';

		/* len */
		to_my_3(SCRAMBLE_LENGTH_323+1, &mysac->buf[0]);
		mysac->qst = MYSAC_SEND_AUTH_2;
		mysac->len = SCRAMBLE_LENGTH_323 + 1 + 4;
		mysac->send = mysac->buf;

	/* send scrambled password in old format */
	case MYSAC_SEND_AUTH_2:
		err = mysac_write(mysac->fd, mysac->send, mysac->len, &errcode);

		if (err == -1)
			return errcode;

		mysac->len -= err;
		mysac->send += err;
		if (mysac->len > 0)
			return MYERR_WANT_WRITE;

		mysac->qst = MYSAC_RECV_AUTH_1;
		mysac->readst = 0;
		mysac->read = mysac->buf;
		mysac->read_len = mysac->bufsize;
		goto case_MYSAC_RECV_AUTH_1;

	case MYSAC_SEND_QUERY:
	case MYSAC_RECV_QUERY_COLNUM:
	case MYSAC_RECV_QUERY_COLDESC1:
	case MYSAC_RECV_QUERY_COLDESC2:
	case MYSAC_RECV_QUERY_EOF1:
	case MYSAC_RECV_QUERY_EOF2:
	case MYSAC_RECV_QUERY_DATA:
	case MYSAC_SEND_INIT_DB:
	case MYSAC_RECV_INIT_DB:
	case MYSAC_SEND_STMT_QUERY:
	case MYSAC_RECV_STMT_QUERY:
	case MYSAC_SEND_STMT_EXECUTE:
	case MYSAC_RECV_STMT_EXECUTE:
	case MYSAC_READ_NUM:
	case MYSAC_READ_HEADER:
	case MYSAC_READ_LINE:
		mysac->errorcode = MYERR_BAD_STATE;
		return MYERR_BAD_STATE;

	}

	return 0;
}

