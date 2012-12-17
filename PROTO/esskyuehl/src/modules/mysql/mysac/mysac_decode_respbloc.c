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
#include "mysac_memory.h"
#include "mysac_net.h"
#include "mysac_decode_respbloc.h"

enum read_state {
	RDST_INIT = 0,
	RDST_READ_LEN,
	RDST_READ_DATA,
	RDST_DECODE_DATA
};

int mysac_decode_respbloc(MYSAC *m, enum my_expected_response_t expect) {
	int i;
	int err;
	int errcode;
	char *read;
	unsigned long len;
	unsigned long rlen;
	char nul;

	switch ((enum read_state)m->readst) {

	case RDST_INIT:
		m->len = 0;
		m->readst = RDST_READ_LEN;

	/* read length */
	case RDST_READ_LEN:

		/* check for avalaible size in buffer */
		while (m->read_len < 4)
			if (mysac_extend_res(m) != 0)
				return MYSAC_RET_ERROR;

		err = mysac_read(m->fd, m->read + m->len,
		                 4 - m->len, &errcode);
		if (err == -1) {
			m->errorcode = errcode;
			return errcode;
		}

		m->len += err;
		if (m->len < 4) {
			m->errorcode = MYERR_WANT_READ;
			return MYERR_WANT_READ;
		}

		/* decode */
		m->packet_length = uint3korr(&m->read[0]);

		/* packet number */
		m->packet_number = m->read[3];

		/* update read state */
		m->readst = RDST_READ_DATA;
		m->len = 0;

	/* read data */
	case RDST_READ_DATA:

		/* check for avalaible size in buffer */
		while ((unsigned int)m->read_len < m->packet_length)
			if (mysac_extend_res(m) != 0)
				return MYSAC_RET_ERROR;

		err = mysac_read(m->fd, m->read + m->len,
		                 m->packet_length - m->len, &errcode);
		if (err == -1)
			return errcode;

		m->len += err;
		if ((unsigned int)m->len < m->packet_length) {
			m->errorcode = MYERR_WANT_READ;
			return MYERR_WANT_READ;
		}
		m->read_len -= m->packet_length;

		/* re-init eof */
		m->readst = RDST_DECODE_DATA;
		m->eof = 1;

	/* decode data */
	case RDST_DECODE_DATA:

		/* error */
		if ((unsigned char)m->read[0] == 255) {

			/* defined mysql error */
			if (m->packet_length > 3) {

				/* read error code */
				// TODO: voir quoi foutre de ca plus tard
				// m->errorcode = uint2korr(&m->read[1]);

				/* read mysal code and message */
				for (i=3; i<3+5; i++)
					m->read[i] = m->read[i+1];
				m->read[8] = ' ';
				m->mysql_error = &m->read[3];
				m->read[m->packet_length] = '\0';
				m->errorcode = MYERR_MYSQL_ERROR;
			}

			/* unknown error */
			else
				m->errorcode = MYERR_PROTOCOL_ERROR;

			return MYSAC_RET_ERROR;
		}

		/* reponse is expectig sucess and onmly success */
		if (expect == MYSAC_EXPECT_OK || expect == MYSAC_EXPECT_BOTH) {

			/* not a sucess code */
			if ((unsigned char)m->read[0] == 0xff) {
				m->errorcode = MYERR_PROTOCOL_ERROR;
				return MYSAC_RET_ERROR;
			}

			/* is sucess */
			if ((unsigned char)m->read[0] == 0) {

				read = &m->read[1];
				rlen = m->packet_length - 1;

				/* affected rows */
				len = my_lcb(read, &m->affected_rows, &nul, rlen);
				rlen -= len;
				read += len;
				/* m->affected_rows = uint2korr(&m->read[1]); */

				/* insert id */
				len = my_lcb(read, &m->insert_id, &nul, rlen);
				rlen -= len;
				read += len;

				/* server status */
				m->status = uint2korr(read);
				read += 2;

				/* server warnings */
				m->warnings = uint2korr(read);

				/* copy va	lues into resource */
				if (m->res != NULL) {
					m->res->affected_rows = m->affected_rows;
					m->res->insert_id     = m->insert_id;
					m->res->warnings      = m->warnings;
				}
				return MYSAC_RET_OK;
			}
		}

		/* response is expecting data. Maybe contain an EOF */
		if (expect == MYSAC_EXPECT_DATA || expect == MYSAC_EXPECT_BOTH) {

			/* EOF marker: marque la fin d'une serie
				(la fin des headers dans une requete) */
			if ((unsigned char)m->read[0] == 254) {
				m->warnings = uint2korr(&m->read[1]);
				m->status = uint2korr(&m->read[3]);
				m->eof = 1;
				return MYSAC_RET_EOF;
			}

			else
				return MYSAC_RET_DATA;
		}

		/* the expect code is not valid */
		m->errorcode = MYERR_INVALID_EXPECT;
		return MYSAC_RET_ERROR;

	default:
		m->errorcode = MYERR_UNEXPECT_R_STATE;
		return MYSAC_RET_ERROR;
	}

	m->errorcode = MYERR_PACKET_CORRUPT;
	return MYSAC_RET_ERROR;
}

