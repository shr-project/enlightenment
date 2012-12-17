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
#include "mysac_memory.h"
#include "mysac_utils.h"

inline
void mysac_reset_res(MYSAC_RES *res) {
	res->nb_cols = 0;
	res->nb_lines = 0;
	INIT_LIST_HEAD(&res->data);
}

inline
MYSAC_RES *mysac_init_res(char *buffer, int len) {
	MYSAC_RES *res;

	/* check minimu length */
	if ((unsigned int)len < sizeof(MYSAC_RES))
		return NULL;

	res = (MYSAC_RES *)buffer;
	res->extend_bloc_size = 0;
	res->max_len = len;
	res->do_free = 0;
	res->buffer = buffer + sizeof(MYSAC_RES);
	res->buffer_len = len - sizeof(MYSAC_RES);
	INIT_LIST_HEAD(&res->list);

	mysac_reset_res(res);

	return res;
}

MYSAC_RES *mysac_new_res(int chunk_size, int extend)
{
	MYSAC_RES *res;

	res = mysac_calloc(1, chunk_size);
	if (res == NULL)
		return NULL;

	if (mysac_init_res((char *)res, chunk_size) == NULL)
		return NULL;
	if (extend)
		res->extend_bloc_size = chunk_size;
	res->do_free = 1;

	return res;
}

void mysac_add_res(MYSAC *mysac, MYSAC_RES *res)
{
	list_add_tail(&res->list, &mysac->all_res);
}

void mysac_del_res(MYSAC_RES *res)
{
	list_del(&res->list);
}

void mysac_free_res(MYSAC_RES *r)
{
	if (r && r->do_free == 1)
		mysac_free(r);
}

MYSAC_RES *mysac_get_res(MYSAC *mysac) {
	return mysac->res;
}

MYSAC_RES *mysac_get_first_res(MYSAC *mysac) {
	if (mysac->all_res.next == &mysac->all_res)
		return NULL;
	return mysac_list_first_entry(&mysac->all_res, MYSAC_RES, list);
}

MYSAC_RES *mysac_get_next_res(MYSAC *mysac, MYSAC_RES *last) {
	if (last->list.next == &mysac->all_res)
		return NULL;
	return mysac_list_next_entry(&last->list, MYSAC_RES, list);
}

unsigned int mysac_field_count(MYSAC_RES *res) {
	return res->nb_cols;
}

unsigned long mysac_num_rows(MYSAC_RES *res) {
	return res->nb_lines;
}

MYSAC_ROW *mysac_fetch_row(MYSAC_RES *res) {

	/* empty list */
	if (res->data.next == &res->data) {
		res->cr = NULL;
		return NULL;
	}

	/* start list */
	if (res->cr == NULL)
		res->cr = mysac_list_first_entry(&res->data, MYSAC_ROWS, link);

	/* next item */
	else
		res->cr = mysac_list_next_entry(&res->cr->link, MYSAC_ROWS, link);

	/* last item */
	if (&res->data == &res->cr->link) {
		res->cr = NULL;
		return NULL;
	}

	return res->cr->data;
}

void mysac_first_row(MYSAC_RES *res) {
	res->cr = NULL;
}

MYSAC_ROW *mysac_cur_row(MYSAC_RES *res) {
	if (res->cr == NULL)
		return NULL;
	return res->cr->data;
}

