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

#include "mysac.h"
#include "mysac_net.h"
#include "mysac_utils.h"
#include "mysac_decode_respbloc.h"

int mysac_set_database(MYSAC *mysac, const char *database) {
	int i;

	/* set packet number */
	mysac->buf[3] = 0;

	/* set mysql command */
	mysac->buf[4] = COM_INIT_DB;

	/* build sql query */
	i = strlen(database);
	memcpy(&mysac->buf[5], database, i);

	/* len */
	to_my_3(i + 1, &mysac->buf[0]);

	/* send params */
	mysac->send = mysac->buf;
	mysac->len = i + 5;
	mysac->qst = MYSAC_SEND_INIT_DB;
	mysac->call_it = mysac_send_database;

	return 0;
}

int mysac_send_database(MYSAC *mysac) {
	int err;
	int errcode;

	switch (mysac->qst) {

	/**********************************************************
	*
	* send query on network
	*
	**********************************************************/
	case MYSAC_SEND_INIT_DB:
		err = mysac_write(mysac->fd, mysac->send, mysac->len, &errcode);

		if (err == -1)
			return errcode;

		mysac->len -= err;
		mysac->send += err;
		if (mysac->len > 0)
			return MYERR_WANT_WRITE;
		mysac->qst = MYSAC_RECV_INIT_DB;
		mysac->readst = 0;
		mysac->read = mysac->buf;

	/**********************************************************
	*
	* receive
	*
	**********************************************************/
	case MYSAC_RECV_INIT_DB:
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_OK);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		else if (err == MYSAC_RET_OK)
			return 0;

		else {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

	case MYSAC_START:
	case MYSAC_CONN_CHECK:
	case MYSAC_READ_GREATINGS:
	case MYSAC_SEND_AUTH_1:
	case MYSAC_RECV_AUTH_1:
	case MYSAC_SEND_AUTH_2:
	case MYSAC_SEND_QUERY:
	case MYSAC_RECV_QUERY_COLNUM:
	case MYSAC_RECV_QUERY_COLDESC1:
	case MYSAC_RECV_QUERY_COLDESC2:
	case MYSAC_RECV_QUERY_EOF1:
	case MYSAC_RECV_QUERY_EOF2:
	case MYSAC_RECV_QUERY_DATA:
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

	mysac->errorcode = MYERR_BAD_STATE;
	return MYERR_BAD_STATE;
}

