/* AbiSource Program Utilities
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_types.h"

struct encoding_pair
{
	const char *	adb;
	UT_UCSChar		ucs;
};


class UT_AdobeEncoding
{
	public:
		UT_AdobeEncoding(const encoding_pair *ep, UT_uint32 esize);
		
		UT_UCSChar		adobeToUcs(const char * str)   const;
		const char *	ucsToAdobe(const UT_UCSChar c);
	private:
		char 			m_buff[8];
		encoding_pair *	m_pLUT;
		UT_uint32 		m_iLutSize;
};
