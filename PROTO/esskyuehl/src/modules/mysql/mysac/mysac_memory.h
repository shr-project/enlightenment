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

/** @file */

#ifndef __MYSAC_MEMORY_H__
#define __MYSAC_MEMORY_H__

/**
 * This extend memory for containing complete response.
 *
 * @param mysac Should be the address of an existing MYSAC structure.
 * 
 * @return 0 if not error occured else return -1
 */
int mysac_extend_res(MYSAC *m);

void *mysac_calloc(size_t nmemb, size_t size);
void *mysac_realloc(void *ptr, size_t size);
void mysac_free(void *ptr);

#endif
