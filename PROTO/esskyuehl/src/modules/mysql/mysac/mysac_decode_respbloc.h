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

#ifndef __MYSAC_DECODE_RESPBLOC_H__
#define __MYSAC_DECODE_RESPBLOC_H__

#include "mysac.h"

#define RESPONSE_MULTI_RESULTS 0x0008

enum mysac_decode_respbloc_t {
	MYSAC_RET_EOF = 1000,
	MYSAC_RET_OK,
	MYSAC_RET_ERROR,
	MYSAC_RET_DATA
};

/**
 * This decode mysql bloc header.
 *
 * @param mysac Should be the address of an existing MYSAC structure.
 * @param expect specifie is data is expected or boolean (ok)
 *
 * @return 
 *    Expected return values are:<br />
 *    MYSAC_RET_OK if OK bloc is sent by mysql server. (only in mode MYSAC_EXPECT_OK)<br />
 *    MYSAC_RET_EOF if EOF bloc is sent by mysql server (only in mode MYSAC_EXPECT_DATA)<br />
 *    MYSAC_RET_DATA if DATA bloc is sent by mysql server (only in mode MYSAC_EXPECT_DATA)<br />
 *    MYERR_WANT_READ if the library need more data on the network<br />
 *    <br />
 *    Unexpected return values:<br />
 *    MYSAC_RET_ERROR if technical error is occured (memory, protocol, ..) you can see<br />
 *                    the error in the field "errorcode" of the mysac struct<br />
 *    MYERR_SERVER_LOST if the serveur closed the connection<br />
 */
int mysac_decode_respbloc(MYSAC *m, enum my_expected_response_t expect);

#endif
