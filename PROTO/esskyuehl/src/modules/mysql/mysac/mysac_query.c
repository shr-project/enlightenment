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
#include "mysac_memory.h"
#include "mysac_decode_respbloc.h"
#include "mysac_decode_field.h"
#include "mysac_decode_row.h"

static inline
int mysac_set_query_params(MYSAC *mysac, MYSAC_RES *res, int len) {

	/* set packet number */
	mysac->buf[3] = 0;

	/* set mysql command */
	mysac->buf[4] = COM_QUERY;

	/* request type */
	mysac->expect = check_action(&mysac->buf[5], len, mysac);

	/* unset statement result */
	mysac->stmt_id = (void *)0;

	/* len */
	to_my_3(len + 1, &mysac->buf[0]);

	/* add resource */
	if (res != NULL)
		mysac->res = res;
	else {
		mysac->res = mysac_get_first_res(mysac);
		if (mysac->res == NULL) {
			mysac->errorcode = MYERR_NO_RES;
			return -1;
		}
	}

	/* send params */
	mysac->send = mysac->buf;
	mysac->len = len + 5;
	mysac->qst = MYSAC_SEND_QUERY;
	mysac->call_it = mysac_send_query;

	return 0;
}

inline
int mysac_b_set_query(MYSAC *mysac, MYSAC_RES *res, const char *query, unsigned int len) {

	/* build sql query */
	if (mysac->bufsize - 5 < len) {
		mysac->errorcode = MYERR_BUFFER_TOO_SMALL;
		mysac->len = 0;
		return -1;
	}
	memcpy(&mysac->buf[5], query, len);

	/* build request */
	return mysac_set_query_params(mysac, res, len);
}

int mysac_s_set_query(MYSAC *mysac, MYSAC_RES *res, const char *query) {
	return mysac_b_set_query(mysac, res, query, strlen(query));
}

inline
int mysac_v_set_query(MYSAC *mysac, MYSAC_RES *res, const char *fmt, va_list ap) {
	unsigned int len;

	/* build sql query */
	len = vsnprintf(&mysac->buf[5], mysac->bufsize - 5, fmt, ap);
	if (len >= mysac->bufsize - 5) {
		mysac->errorcode = MYERR_BUFFER_TOO_SMALL;
		mysac->len = 0;
		return -1;
	}

	return mysac_set_query_params(mysac, res, len);
}

int mysac_set_query(MYSAC *mysac, MYSAC_RES *res, const char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = mysac_v_set_query(mysac, res, fmt, ap);
	va_end(ap);

	return ret;
}

int mysac_send_query(MYSAC *mysac) {
	int err;
	int errcode;
	int i;
	int len;
	unsigned int nb_cols;

	switch (mysac->qst) {

	/**********************************************************
	*
	* send query on network
	*
	**********************************************************/
	case MYSAC_SEND_QUERY:
		err = mysac_write(mysac->fd, mysac->send, mysac->len, &errcode);

		if (err == -1)
			return errcode;

		mysac->len -= err;
		mysac->send += err;
		if (mysac->len > 0)
			return MYERR_WANT_WRITE;

		/* prepare first resource */
		mysac->read = mysac->res->buffer;
		mysac->read_len = mysac->res->buffer_len;

	/**********************************************************
	*
	* receive
	*
	**********************************************************/

	case_MYSAC_RECV_QUERY_COLNUM:
	mysac->qst = MYSAC_RECV_QUERY_COLNUM;
	mysac->readst = 0;

	/* prepare struct

	 +---------------+-----------------+
	 | MYSQL_FIELD[] | char[]          |
	 | resp->nb_cols | all fields name |
	 +---------------+-----------------+

	 */

	case MYSAC_RECV_QUERY_COLNUM:

		/* if the last EOF return RESPONSE_MULTI_RESULTS flag, 
		 * expect data or ok. This use previous resource.
		 */
		if (mysac->status & RESPONSE_MULTI_RESULTS) {
			err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_BOTH);

			/* want read */
			if (err == MYERR_WANT_READ)
				return MYERR_WANT_READ;

			/* error */
			if (err == MYSAC_RET_ERROR)
				return mysac->errorcode;

			/* end of request */
			if (err == MYSAC_RET_OK)
				return 0;

			/* use next resource */
			if (mysac->all_res.next != &mysac->all_res)
				mysac->res = mysac_get_next_res(mysac, mysac->res);
			if (mysac->res == NULL) {
				mysac->res = mysac_get_first_res(mysac);
				if (mysac->res == NULL) {
					mysac->errorcode = MYERR_NO_RES;
					return mysac->errorcode;
				}
			}

			/* update resource size */
			while ((unsigned int)mysac->read_len < mysac->packet_length)
				if (mysac_extend_res(mysac) != 0)
					return MYSAC_RET_ERROR;

			/* copy data */
			memcpy(mysac->res->buffer, mysac->read, mysac->packet_length);
			mysac->read = mysac->res->buffer;
			mysac->read_len = mysac->res->buffer_len;
			mysac->len = mysac->packet_length;
		}

		else {
			err = mysac_decode_respbloc(mysac, mysac->expect);

			/* want read */
			if (err == MYERR_WANT_READ)
				return MYERR_WANT_READ;

			/* error */
			if (err == MYSAC_RET_ERROR)
				return mysac->errorcode;

		}

		/* ok ( for insert or no return data command ) */
		if (mysac->expect == MYSAC_EXPECT_OK) {
			if (err == MYSAC_RET_OK)
				return 0;
			else {
				mysac->errorcode = MYERR_PROTOCOL_ERROR;
				return mysac->errorcode;
			}
		}

		/* invalide expect */
		else if (mysac->expect != MYSAC_EXPECT_DATA) {
			mysac->errorcode = MYERR_INVALID_EXPECT;
			return mysac->errorcode;
		}

		/* protocol error */
		if (err != MYSAC_RET_DATA) {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* get nb col TODO: pas sur que ce soit un byte */
		nb_cols = (unsigned char)mysac->read[0];
		mysac->read_id = 0;
		mysac->qst = MYSAC_RECV_QUERY_COLDESC1;

		/* reset resource */
		mysac_reset_res(mysac->res);

		/* prepare cols space */

		/* check for avalaible size in buffer */
		while ((unsigned int)mysac->read_len < sizeof(MYSQL_FIELD) * nb_cols)
			if (mysac_extend_res(mysac) != 0)
				return mysac->errorcode;

		mysac->res->nb_cols = nb_cols;
		mysac->res->cols = (MYSQL_FIELD *)mysac->read;
		memset((char *)mysac->res->cols, 0, sizeof(MYSQL_FIELD) * mysac->res->nb_cols);
		mysac->read += sizeof(MYSQL_FIELD) * mysac->res->nb_cols;
		mysac->read_len -= sizeof(MYSQL_FIELD) * mysac->res->nb_cols;

	/**********************************************************
	*
	* receive column description
	*
	**********************************************************/

	case_MYSAC_RECV_QUERY_COLDESC1:
	mysac->readst = 0;

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

		/* decode mysql packet with field desc, use packet buffer for storing
		   mysql data (field name) */
#if 0
		for (i=0; i<mysac->packet_length; i++) {
			fprintf(stderr, "%02x ", (unsigned char)mysac->read[i]);
		}
		fprintf(stderr, "\n");
#endif
		len = mysac_decode_field(mysac->read, mysac->packet_length,
		                         &mysac->res->cols[mysac->read_id]);

		if (len < 0) {
			mysac->errorcode = len * -1;
			return mysac->errorcode;
		}
		mysac->read += len;
		mysac->read_len += mysac->packet_length - len;

		mysac->read_id++;
		if (mysac->read_id < mysac->res->nb_cols)
			goto case_MYSAC_RECV_QUERY_COLDESC1;

		mysac->readst = 0;
		mysac->qst = MYSAC_RECV_QUERY_EOF1;

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

		mysac->qst = MYSAC_RECV_QUERY_DATA;

	/**********************************************************
	*
	* read data lines
	*
	**********************************************************/
	case_MYSAC_RECV_QUERY_DATA:

	/*
	   +-------------------+----------------+-----------------+----------------+
	   | struct mysac_rows | MYSAC_ROW[]    | unsigned long[] | struct tm[]    |
	   |                   | mysac->nb_cols | mysac->nb_cols  | mysac->nb_time |
	   +-------------------+----------------+-----------------+----------------+
	 */

	/* check for avalaible size in buffer */
	while ((unsigned int)mysac->read_len < sizeof(MYSAC_ROWS) + ( mysac->res->nb_cols * (
	                         sizeof(MYSAC_ROW) + sizeof(unsigned long) ) ) )
		if (mysac_extend_res(mysac) != 0)
			return mysac->errorcode;

	mysac->read_len -= sizeof(MYSAC_ROWS) + ( mysac->res->nb_cols * (
	                   sizeof(MYSAC_ROW) + sizeof(unsigned long) ) );

	/* reserve space for MYSAC_ROWS and add it into chained list */
	mysac->res->cr = (MYSAC_ROWS *)mysac->read;
	list_add_tail(&mysac->res->cr->link, &mysac->res->data);
	mysac->read += sizeof(MYSAC_ROWS);

	/* space for each field definition into row */
	mysac->res->cr->data = (MYSAC_ROW *)mysac->read;
	mysac->read += sizeof(MYSAC_ROW) * mysac->res->nb_cols;

	/* space for length table */
	mysac->res->cr->lengths = (unsigned long *)mysac->read;
	mysac->read += sizeof(unsigned long) * mysac->res->nb_cols;

	/* struct tm */
	for (i=0; i<mysac->res->nb_cols; i++) {
		switch(mysac->res->cols[i].type) {

		/* date type */
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_DATE:
			while ((unsigned int)mysac->read_len < sizeof(struct tm))
				if (mysac_extend_res(mysac) != 0)
					return mysac->errorcode;

			mysac->res->cr->data[i].tm = (struct tm *)mysac->read;
			mysac->read += sizeof(struct tm);
			mysac->read_len -= sizeof(struct tm);
			memset(mysac->res->cr->data[i].tm, 0, sizeof(struct tm));
			break;

		/* do nothing for other types */
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_NULL:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_NEWDATE:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_GEOMETRY:
			break;
		}
	}

	/* set state at 0 */
	mysac->readst = 0;

	case MYSAC_RECV_QUERY_DATA:
		err = mysac_decode_respbloc(mysac, MYSAC_EXPECT_DATA);

		if (err == MYERR_WANT_READ)
			return MYERR_WANT_READ;

		/* error */
		else if (err == MYSAC_RET_ERROR)
			return mysac->errorcode;

		/* EOF */
		else if (err == MYSAC_RET_EOF) {
			list_del(&mysac->res->cr->link);
			mysac->res->cr = NULL;

			/* check for multiquery flag. If is set, jump to start */
			if (mysac->status & RESPONSE_MULTI_RESULTS)
				goto case_MYSAC_RECV_QUERY_COLNUM;

			return 0;
		}

		/* read data in string type */
		if (mysac->stmt_id == (void *)0) {
#if 0
			for (i=0; i<mysac->packet_length; i+=20) {
				int j;

				for(j=i;j<i+20;j++)
					fprintf(stderr, "%02x ", (unsigned char)mysac->read[j]);

				for(j=i;j<i+20;j++)
					if (isprint(mysac->read[j]))
						fprintf(stderr, "%c", (unsigned char)mysac->read[j]);
					else
						fprintf(stderr, ".");

				fprintf(stderr, "\n");
			}
			fprintf(stderr, "\n\n");
#endif
			len = mysac_decode_string_row(mysac->read, mysac->packet_length,
			                              mysac->res, mysac->res->cr);
			if (len < 0) {
				mysac->errorcode = len * -1;
				return mysac->errorcode;
			}
			mysac->read += len;
			mysac->read_len += mysac->packet_length - len;
		}

		/* read data in binary type */
		else if (mysac->stmt_id == (void *)1) {
#if 0
			for (i=0; i<mysac->packet_length; i++) {
				fprintf(stderr, "%02x ", (unsigned char)mysac->read[i]);
			}
			fprintf(stderr, "\n");
#endif
			len = mysac_decode_binary_row(mysac->read, mysac->packet_length,
			                              mysac->res, mysac->res->cr);
			if (len == -1) {
				mysac->errorcode = MYERR_BINFIELD_CORRUPT;
				return mysac->errorcode;
			}
			mysac->read += len;
			mysac->read_len += mysac->packet_length - len;
		}

		/* protocol error */
		else {
			mysac->errorcode = MYERR_PROTOCOL_ERROR;
			return mysac->errorcode;
		}

		/* next line */
		mysac->res->nb_lines++;
		goto case_MYSAC_RECV_QUERY_DATA;

	case MYSAC_START:
	case MYSAC_CONN_CHECK:
	case MYSAC_READ_GREATINGS:
	case MYSAC_SEND_AUTH_1:
	case MYSAC_RECV_AUTH_1:
	case MYSAC_SEND_AUTH_2:
	case MYSAC_SEND_INIT_DB:
	case MYSAC_RECV_INIT_DB:
	case MYSAC_SEND_STMT_QUERY:
	case MYSAC_RECV_STMT_QUERY:
	case MYSAC_SEND_STMT_EXECUTE:
	case MYSAC_RECV_STMT_EXECUTE:
	case MYSAC_READ_NUM:
	case MYSAC_READ_HEADER:
	case MYSAC_READ_LINE:
	case MYSAC_RECV_QUERY_COLDESC2:
	case MYSAC_RECV_QUERY_EOF2:
		mysac->errorcode = MYERR_BAD_STATE;
		return MYERR_BAD_STATE;
	}

	mysac->errorcode = MYERR_BAD_STATE;
	return MYERR_BAD_STATE;
}

