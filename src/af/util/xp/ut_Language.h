/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
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

#ifndef UT_LANGUAGE_H
#define UT_LANGUAGE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

enum UT_LANGUAGE_ORDER
{
	UTLANG_LTR,
	UTLANG_RTL,
	UTLANG_VERTICAL
};


typedef struct
{
	XML_Char * prop;
	XML_Char * lang;
	UT_uint32  id;
    UT_LANGUAGE_ORDER order;
} lang_entry;

class ABI_EXPORT UT_Language
{
public:
	UT_Language();

	UT_uint32	getCount();
	const XML_Char * 	getNthProperty(UT_uint32 n);
	const XML_Char * 	getNthLanguage(UT_uint32 n);
	const XML_Char * 	getPropertyFromLanguage(const XML_Char * lang);
	const XML_Char * 	getPropertyFromProperty(const XML_Char * prop); //see the cpp file for explanation
	UT_uint32 	        getIndxFromProperty(const XML_Char * prop);
	UT_uint32 	        getIdFromProperty(const XML_Char * prop);

	UT_LANGUAGE_ORDER   getOrderFromProperty(const XML_Char * prop);

private:
	static bool	s_Init;
};

#endif
