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

#include <ctype.h>

#include "mysac.h"
#include "mysac_decode_respbloc.h"

enum my_expected_response_t check_action(const char *request, int len, MYSAC *mysac)
{
	const char *parse;

	/* jump blank char '\r', '\n', '\t' and ' ' */
	parse = request;
	while (1) {
		if (!isspace(*parse))
			break;

		/* if no more chars in string */
		len--;
		if (len <= 0)
			return MYSAC_EXPECT_OK;

		parse++;
	}

	/* check request type */
	if ( (len > 6) && ( strncasecmp(parse, "SELECT", 5) == 0) )
		return MYSAC_EXPECT_DATA;

	else if ( (len > 5) && ( strncasecmp(parse, "CALL", 4) == 0) ) {
		mysac->status = RESPONSE_MULTI_RESULTS;
		return MYSAC_EXPECT_DATA;
	}

	return MYSAC_EXPECT_OK;
}

