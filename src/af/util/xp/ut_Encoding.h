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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef UT_ENCODING_H
#define UT_ENCODING_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

struct enc_entry
{
	gchar const ** encs;
	const gchar * desc;
	UT_uint32  id;
};

class ABI_EXPORT UT_Encoding
{
public:
	UT_Encoding();

	UT_uint32	getCount();
	const gchar * 	getNthEncoding(UT_uint32 n);
	const gchar * 	getNthDescription(UT_uint32 n);
	const gchar * 	getEncodingFromDescription(const gchar * desc);
	UT_uint32 	getIndxFromEncoding(const gchar * enc);
	UT_uint32 	getIdFromEncoding(const gchar * enc);

private:
	static bool	s_Init;
	static UT_uint32	s_iCount;
};

#endif
