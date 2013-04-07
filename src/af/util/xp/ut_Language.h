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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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

enum UT_LANGUAGE_DIR
{
	UTLANG_LTR,
	UTLANG_RTL,
	UTLANG_VERTICAL
};

struct UT_LangRecord
{
	const gchar * m_szLangCode;
	const gchar * m_szLangName;
	UT_uint32  m_nID;
    UT_LANGUAGE_DIR m_eDir;
};


class ABI_EXPORT UT_Language
{
public:
	UT_Language();

	UT_uint32	getCount();
	const gchar * 	getNthLangCode(UT_uint32 n);
	const gchar * 	getNthLangName(UT_uint32 n);
	UT_uint32  	getNthId(UT_uint32 n);
	const gchar * 	getCodeFromName(const gchar * szName);
	const gchar * 	getCodeFromCode(const gchar * szCode); //
																  //see the cpp file for explanation
	const UT_LangRecord* getLangRecordFromCode(const gchar * szCode);
	UT_uint32 	        getIndxFromCode(const gchar * szCode);
	UT_uint32 	        getIdFromCode(const gchar * szCode);
	UT_LANGUAGE_DIR		getDirFromCode(const gchar * szCode);

private:
	static bool	s_Init;
};

ABI_EXPORT void UT_Language_updateLanguageNames();
#endif
