/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef UT_ENCODING_H
#define UT_ENCODING_H

#include "ut_types.h"
#include "ut_xml.h"

typedef struct
	{
		XML_Char ** encs;
		XML_Char * desc;
		UT_uint32  id;
	} enc_entry;

class UT_Encoding
{
public:
	UT_Encoding();

	UT_uint32	getCount();
	const XML_Char * 	getNthEncoding(UT_uint32 n);
	const XML_Char * 	getNthDescription(UT_uint32 n);
	const XML_Char * 	getEncodingFromDescription(const XML_Char * desc);
	const XML_Char * 	getEncodingFromEncoding(const XML_Char * enc); //see the cpp file for explanation
	UT_uint32 	getIndxFromEncoding(const XML_Char * enc);
	UT_uint32 	getIdFromEncoding(const XML_Char * enc);

protected:
	static bool	s_Init;
	static UT_uint32	s_iCount;
};

#endif
