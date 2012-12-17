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

#include "mysac.h"
#include "mysac_utils.h"

void *mysac_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

void mysac_free(void *ptr)
{
	return free(ptr);
}

void *mysac_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

int mysac_extend_res(MYSAC *m)
{
	MYSAC_RES *res = m->res;
	MYSAC_ROWS *row;
	struct mysac_list_head *stop;
	struct mysac_list_head *run;
	struct mysac_list_head *next;
	struct mysac_list_head *prev;
	long long int offset;
	int i;

	if (res->extend_bloc_size == 0) {
		m->errorcode = MYERR_BUFFER_OVERSIZE;
		return -1;
	}

	res = mysac_realloc(m->res, res->max_len + res->extend_bloc_size);
	if (res == NULL) {
		m->errorcode = MYERR_SYSTEM;
		return -1;
	}

	/* clear new memory */
	memset((char *)res + res->max_len, 0, res->extend_bloc_size);

	mysac_print_audit(m, "mysac realloc memory: from_ptr=%p, to_ptr=%p, from=%d to=%d",
	                  m->res, res, res->max_len, res->max_len + res->extend_bloc_size);

	/* update lengths */
	res->buffer_len += res->extend_bloc_size;
	res->max_len += res->extend_bloc_size;
	m->read_len += res->extend_bloc_size;

	/* if the pointer of the data block does not change, the operation is ended */
	if (res == m->res)
		return 0;

	/* if the pointer of the dat bloc changed, update links */

	/* compute offset between old and new memory bloc */
	offset = (unsigned long int)res - (unsigned long int)m->res;

	/* update pointers */
	res->buffer = (char *)((char *)res->buffer + offset);
	res->cr = (MYSAC_ROWS *)((char *)res->cr + offset);
	m->read = (char *)((char *)m->read + offset);

	/* update first cell */
	next = (struct mysac_list_head *)((char *)res->data.next + offset);
	prev = (struct mysac_list_head *)((char *)res->data.prev + offset);
	res->data.next = next;
	res->data.prev = prev;
	stop = &res->data;

	/* update row names */
	res->cols = (MYSQL_FIELD *)((char *)res->cols + offset);
	for (i=0; i<res->nb_cols; i++) {
		if (res->cols[i].name != NULL)
			res->cols[i].name = res->cols[i].name + offset;
		if (res->cols[i].org_name != NULL)
			res->cols[i].org_name = res->cols[i].org_name + offset;
		if (res->cols[i].table != NULL)
			res->cols[i].table = res->cols[i].table + offset;
		if (res->cols[i].org_table != NULL)
			res->cols[i].org_table = res->cols[i].org_table + offset;
		if (res->cols[i].db != NULL)
			res->cols[i].db = res->cols[i].db + offset;
		if (res->cols[i].catalog != NULL)
			res->cols[i].catalog = res->cols[i].catalog + offset;
		if (res->cols[i].def != NULL)
			res->cols[i].def = res->cols[i].def + offset;
	}

	/* parcours la liste */
	run = res->data.next;
	while (1) {
		if (run == stop)
			break;
		next = (struct mysac_list_head *)((char *)run->next + offset);
		prev = (struct mysac_list_head *)((char *)run->prev + offset);
		row = mysac_container_of(run, MYSAC_ROWS, link);

		/* upadate data pointer */
		row->lengths = (unsigned long *)((char *)row->lengths + offset);
		row->data = (MYSAC_ROW *)((char *)row->data + offset);

		/* struct tm */
		for (i=0; i<res->nb_cols; i++) {
			switch(res->cols[i].type) {

			/* apply offset on data pointer */
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_BLOB:
				if (row->data[i].string != NULL)
					row->data[i].string = row->data[i].string + offset;
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
			case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_ENUM:
			case MYSQL_TYPE_SET:
			case MYSQL_TYPE_GEOMETRY:
			default:
				break;
			}
		}

		run->next = next;
		run->prev = prev;
		run = run->next;
	}

	/* update neighbor list link */
	res->list.prev->next = &res->list;
	res->list.next->prev = &res->list;

	/* update resource pointer */
	m->res = res;

	return 0;
}

