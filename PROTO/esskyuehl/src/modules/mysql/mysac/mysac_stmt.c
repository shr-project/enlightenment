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
#include "mysac_utils.h"
#include "mysac_net.h"
#include "mysac_encode_values.h"
#include "mysac_decode_respbloc.h"

int mysac_b_set_stmt_prepare(MYSAC *mysac, unsigned int *stmt_id,
                             const char *request, int len) {

	/* set packet number */
	mysac->buf[3] = 0;

	/* set mysql command */
	mysac->buf[4] = COM_STMT_PREPARE;

	/* check len */
	if (mysac->bufsize - 5 < (unsigned int)len)
		return -1;

	/* build sql query */
	memcpy(&mysac->buf[5], request, len);

	/* request type */
	mysac->expect = check_action(&mysac->buf[5], len, mysac);

	/* l */
	to_my_3(len + 1, &mysac->buf[0]);

	/* send params */
	mysac->send = mysac->buf;
	mysac->len = len + 5;
	mysac->qst = MYSAC_SEND_STMT_QUERY;
	mysac->call_it = mysac_send_stmt_prepare;
	mysac->stmt_id = stmt_id;

	return 0;
}

int mysac_s_set_stmt_prepare(MYSAC *mysac, unsigned int *stmt_id,
                             const char *request) {
	return mysac_b_set_stmt_prepare(mysac, stmt_id, request, strlen(request));
}

int mysac_v_set_stmt_prepare(MYSAC *mysac, unsigned int *stmt_id,
                             const char *fmt, va_list ap) {
	int len;

	/* set packet number */
	mysac->buf[3] = 0;

	/* set mysql command */
	mysac->buf[4] = COM_STMT_PREPARE;

	/* build sql query */
	len = vsnprintf(&mysac->buf[5], mysac->bufsize - 5, fmt, ap);
	if ((unsigned int)len >= mysac->bufsize - 5)
		return -1;

	/* request type */
	mysac->expect = check_action(&mysac->buf[5], len, mysac);

	/* len */
	to_my_3(len + 1, &mysac->buf[0]);

	/* send params */
	mysac->send = mysac->buf;
	mysac->len = len + 5;
	mysac->qst = MYSAC_SEND_STMT_QUERY;
	mysac->call_it = mysac_send_stmt_prepare;
	mysac->stmt_id = stmt_id;

	return 0;
}

int mysac_set_stmt_prepare(MYSAC *mysac, unsigned int *stmt_id,
                           const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	return mysac_v_set_stmt_prepare(mysac, stmt_id, fmt, ap);
}

int mysac_send_stmt_prepare(MYSAC *mysac) {
	int err;
	int errcode;

	switch (mysac->qst) {

	/**********************************************************
	*
	* send query on network
	*
	**********************************************************/
	case MYSAC_SEND_STMT_QUERY:
		err = mysac_write(mysac->fd, mysac->send, mysac->len, &errcode);

		if (err == -1)
			return errcode;

		mysac->len -= err;
		mysac->send += err;
		if (mysac->len > 0)
			return MYERR_WANT_WRITE;
		mysac->qst = MYSAC_RECV_STMT_QUERY;
		mysac->readst = 0;
		mysac->read = mysac->buf;

	/**********************************************************
	*
	* receive
	*
	**********************************************************/
	case MYSAC_RECV_STMT_QUERY:
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_OK);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		if (err != MYSAC_RET_OK) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* 0: don't care */

		/* 1-4: get statement id */
		*mysac->stmt_id = uint4korr(&mysac->buf[1]);

		/* if data expected, set MSB */
		if (mysac->expect == MYSAC_EXPECT_DATA)
			(*mysac->stmt_id) |= 0x80000000;

		/* 5-6: get nb of columns */
		mysac->nb_cols = uint2korr(&mysac->buf[5]);

		/* 7-8: Number of placeholders in the statement */
		mysac->nb_plhold = uint2korr(&mysac->buf[7]);

		/* 9-.. don't care ! */

		/* if no data expected */
		if (mysac->expect == MYSAC_EXPECT_OK)
			return 0;

		if (mysac->nb_plhold > 0)
			mysac->qst = MYSAC_RECV_QUERY_COLDESC1;
		else {
			mysac->qst = MYSAC_RECV_QUERY_COLDESC2;
			goto case_MYSAC_RECV_QUERY_COLDESC2;
		}

	/**********************************************************
	*
	* receive place holder description
	*
	**********************************************************/
	case_MYSAC_RECV_QUERY_COLDESC1:
	mysac->readst = 0;
	mysac->read = mysac->buf;

	case MYSAC_RECV_QUERY_COLDESC1:

		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		else if (err != MYSAC_RET_DATA) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* XXX for a moment, dont decode columns
		 * names and types
		 */
		mysac->nb_plhold--;
		if (mysac->nb_plhold != 0)
			goto case_MYSAC_RECV_QUERY_COLDESC1;

		mysac->readst = 0;
		mysac->qst = MYSAC_RECV_QUERY_EOF1;
		mysac->read = mysac->buf;

	/**********************************************************
	*
	* receive EOF
	*
	**********************************************************/
	case MYSAC_RECV_QUERY_EOF1:
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		else if (err != MYSAC_RET_EOF) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		mysac->qst = MYSAC_RECV_QUERY_COLDESC2;

	/**********************************************************
	*
	* receive column description
	*
	**********************************************************/
	case_MYSAC_RECV_QUERY_COLDESC2:
	mysac->readst = 0;
	mysac->read = mysac->buf;

	case MYSAC_RECV_QUERY_COLDESC2:

		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		else if (err != MYSAC_RET_DATA) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* XXX for a moment, dont decode columns
		 * names and types
		 */
		mysac->nb_cols--;
		if (mysac->nb_cols != 0)
			goto case_MYSAC_RECV_QUERY_COLDESC2;

		mysac->readst = 0;
		mysac->qst = MYSAC_RECV_QUERY_EOF2;
		mysac->read = mysac->buf;

	/**********************************************************
	*
	* receive EOF
	*
	**********************************************************/
	case MYSAC_RECV_QUERY_EOF2:
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* protocol error */
		else if (err != MYSAC_RET_EOF) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		return 0;

	case MYSAC_START:
	case MYSAC_CONN_CHECK:
	case MYSAC_READ_GREATINGS:
	case MYSAC_SEND_AUTH_1:
	case MYSAC_RECV_AUTH_1:
	case MYSAC_SEND_AUTH_2:
	case MYSAC_SEND_QUERY:
	case MYSAC_RECV_QUERY_COLNUM:
	case MYSAC_RECV_QUERY_DATA:
	case MYSAC_SEND_INIT_DB:
	case MYSAC_RECV_INIT_DB:
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

int mysac_set_stmt_execute(MYSAC *mysac, MYSAC_RES *res, unsigned int stmt_id,
                           MYSAC_BIND *values, int nb) {
	int i;
	int nb_bf;
	int desc_off;
	unsigned int vals_off;
	unsigned int len = 3 + 1 + 1 + 4 + 1 + 4;
	int ret;

	/* check len */
	if (mysac->bufsize < len) {
		mysac->errorcode = MYERR_BUFFER_TOO_SMALL;
		mysac->len = 0;
		return -1;
	}

	/* 3 : set packet number */
	mysac->buf[3] = 0;

	/* 4 : set mysql command */
	mysac->buf[4] = COM_STMT_EXECUTE;

	/* dat aexpected */
	if ((stmt_id & 0x80000000) == 0)
		mysac->expect = MYSAC_EXPECT_OK;
	else {
		stmt_id &= 0x7fffffff;
		mysac->expect = MYSAC_EXPECT_DATA;
	}
	mysac->stmt_id = (void *)1;

	/* 5-8 : build sql query */
	to_my_4(stmt_id, &mysac->buf[5]);

	/* 9 : flags (unused) */
	mysac->buf[9] = 0;

	/* 10-13 : iterations (unused) */
	to_my_4(1, &mysac->buf[10]);

	/* number of bytes for the NULL values bitfield */
	nb_bf = ( nb / 8 ) + 1;
	desc_off = len + nb_bf + 1;
	vals_off = desc_off + ( nb * 2 );

	/* check len */
	if (mysac->bufsize < vals_off) {
		mysac->errorcode = MYERR_BUFFER_TOO_SMALL;
		mysac->len = 0;
		return -1;
	}

	/* init bitfield: set 0 */
	memset(&mysac->buf[len], 0, nb_bf);

	/* build NULL bitfield and values type */
	for (i=0; i<nb; i++) {

		/***********************
		 *
		 * NULL bitfield
		 *
		 ***********************/
		if (values[i].is_null != 0)
			mysac->buf[len + (i << 3)] |= 1 << (i & 0xf);

		/***********************
		 *
		 * Value type
		 *
		 ***********************/
		mysac->buf[desc_off + ( i * 2 )] =  values[i].type;
		mysac->buf[desc_off + ( i * 2 ) + 1] = 0x00; /* ???? */

		/***********************
		 *
		 * set values data
		 *
		 ***********************/
		ret = mysac_encode_value(&values[i], &mysac->buf[vals_off],
		                         mysac->bufsize - vals_off);
		if (ret < 0) {
			mysac->errorcode = MYERR_BUFFER_TOO_SMALL;
			mysac->len = 0;
			return -1;
		}
		vals_off += ret;
	}

	/* 01 byte ??? */
	mysac->buf[len + nb_bf] = 0x01;

	/* 0-2 : len
	 * 4 = packet_len + packet_id
	 */
	to_my_3(vals_off - 4, &mysac->buf[0]);

	/* send params */
	mysac->res = res;
	mysac->send = mysac->buf;
	mysac->len = vals_off;
	mysac->qst = MYSAC_SEND_QUERY;
	mysac->call_it = mysac_send_stmt_execute;

	return 0;
}

int mysac_send_stmt_execute(MYSAC *mysac) {
	return mysac_send_query(mysac);
}

