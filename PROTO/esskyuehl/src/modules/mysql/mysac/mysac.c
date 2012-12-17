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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <mysql/my_global.h>

#include "mysac_decode_respbloc.h"
#include "mysac_decode_field.h"
#include "mysac_encode_values.h"
#include "mysac_decode_row.h"
#include "mysac.h"
#include "mysac_memory.h"
#include "mysac_net.h"
#include "mysac_utils.h"

const char *mysac_type[] = {
	[MYSQL_TYPE_DECIMAL]     = "MYSQL_TYPE_DECIMAL",
	[MYSQL_TYPE_TINY]        = "MYSQL_TYPE_TINY",
	[MYSQL_TYPE_SHORT]       = "MYSQL_TYPE_SHORT",
	[MYSQL_TYPE_LONG]        = "MYSQL_TYPE_LONG",
	[MYSQL_TYPE_FLOAT]       = "MYSQL_TYPE_FLOAT",
	[MYSQL_TYPE_DOUBLE]      = "MYSQL_TYPE_DOUBLE",
	[MYSQL_TYPE_NULL]        = "MYSQL_TYPE_NULL",
	[MYSQL_TYPE_TIMESTAMP]   = "MYSQL_TYPE_TIMESTAMP",
	[MYSQL_TYPE_LONGLONG]    = "MYSQL_TYPE_LONGLONG",
	[MYSQL_TYPE_INT24]       = "MYSQL_TYPE_INT24",
	[MYSQL_TYPE_DATE]        = "MYSQL_TYPE_DATE",
	[MYSQL_TYPE_TIME]        = "MYSQL_TYPE_TIME",
	[MYSQL_TYPE_DATETIME]    = "MYSQL_TYPE_DATETIME",
	[MYSQL_TYPE_YEAR]        = "MYSQL_TYPE_YEAR",
	[MYSQL_TYPE_NEWDATE]     = "MYSQL_TYPE_NEWDATE",
	[MYSQL_TYPE_VARCHAR]     = "MYSQL_TYPE_VARCHAR",
	[MYSQL_TYPE_BIT]         = "MYSQL_TYPE_BIT",
	[MYSQL_TYPE_NEWDECIMAL]  = "MYSQL_TYPE_NEWDECIMAL",
	[MYSQL_TYPE_ENUM]        = "MYSQL_TYPE_ENUM",
	[MYSQL_TYPE_SET]         = "MYSQL_TYPE_SET",
	[MYSQL_TYPE_TINY_BLOB]   = "MYSQL_TYPE_TINY_BLOB",
	[MYSQL_TYPE_MEDIUM_BLOB] = "MYSQL_TYPE_MEDIUM_BLOB",
	[MYSQL_TYPE_LONG_BLOB]   = "MYSQL_TYPE_LONG_BLOB",
	[MYSQL_TYPE_BLOB]        = "MYSQL_TYPE_BLOB",
	[MYSQL_TYPE_VAR_STRING]  = "MYSQL_TYPE_VAR_STRING",
	[MYSQL_TYPE_STRING]      = "MYSQL_TYPE_STRING",
	[MYSQL_TYPE_GEOMETRY]    = "MYSQL_TYPE_GEOMETRY"
};

void mysac_init(MYSAC *mysac, char *buffer, unsigned int buffsize) {

	/* init */
	memset(mysac, 0, sizeof(MYSAC));
	mysac->qst = MYSAC_START;
	mysac->buf = buffer;
	mysac->bufsize = buffsize;
	INIT_LIST_HEAD(&mysac->all_res);
}

MYSAC *mysac_new(int buffsize) {
	MYSAC *m;
	char *buf;

	/* struct memory */
	m = mysac_calloc(1, sizeof(MYSAC));
	if (m == NULL)
		return NULL;

	/* buff memory */
	buf = mysac_calloc(1, buffsize);
	if (buf == NULL) {
		free(m);
		return NULL;
	}

	/* init */
	m->free_it = 1;
	m->qst = MYSAC_START;
	m->buf = buf;
	m->bufsize = buffsize;

	return m;
}

void mysac_setup(MYSAC *mysac, const char *my_addr, const char *user,
                 const char *passwd, const char *db,
                 unsigned long client_flag) {
	mysac->addr     = my_addr;
	mysac->login    = user;
	mysac->password = passwd;
	mysac->database = db;
	mysac->flags    = client_flag;
	mysac->call_it  = mysac_connect;
}

void mysac_close(MYSAC *mysac) {
	if (mysac->free_it == 1)
		free(mysac);
}

int mysac_get_fd(MYSAC *mysac) {
	return mysac->fd;
}

int mysac_io(MYSAC *mysac) {
	if (mysac == NULL || mysac->call_it == NULL)
		return MYERR_BAD_STATE;
	return mysac->call_it(mysac);
}

unsigned long mysac_insert_id(MYSAC *m) {
	return m->insert_id;
}

unsigned int mysac_errno(MYSAC *mysac) {
	return mysac->errorcode;
}

const char *mysac_error(MYSAC *mysac) {
	return mysac_errors[mysac->errorcode];
}

const char *mysac_advance_error(MYSAC *mysac) {
	if (mysac->errorcode == MYERR_MYSQL_ERROR)
		return mysac->mysql_error;
	else if (mysac->errorcode == MYERR_SYSTEM)
		return strerror(errno);
	else
		return mysac_errors[mysac->errorcode];
}

void mysac_set_audit_fcn(MYSAC *mysac, void *arg, mysac_audit ma) {
	mysac->ma = ma;
	mysac->ma_arg = arg;
}
