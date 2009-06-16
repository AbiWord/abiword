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

#include <stdlib.h>

#include "ut_Language.h"
#include "ut_string.h"

#include "xap_App.h"
#include "xap_Strings.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

// Please keep the list below alphabetised by the lang; even though 
// this is not required for it to work, it will make it easier to maintain.

// to add a new language:
// (1) check on this list it is not already there;
// (2) add it to the list in xap_String_Id.h 
// (3) add it here, using the ID corresponding to the one
//     from xap_String_Id.h

static UT_LangRecord s_Table[] = {{}};

/*!
 Compare function used by qsort()

 \param a left side of comparison
 \param b right side of comparison
 \return negative, 0, or positive

 Special "no proofing" language will always be sorted to the
  top of the list
 */
static int s_compareQ(const void * a, const void *b)
{
	const UT_LangRecord * A = static_cast<const UT_LangRecord *>(a);
	const UT_LangRecord * B = static_cast<const UT_LangRecord *>(b);
#if 0
	// as long as bsearch is used searching for lang codes this is wrong
	if (B->m_nID == XAP_STRING_ID_LANG_0)
		return 1;
	else if (A->m_nID == XAP_STRING_ID_LANG_0)
		return -1;

	return g_utf8_collate(A->m_szLangName, B->m_szLangName);
#else

	return strcmp(A->m_szLangCode, B->m_szLangCode);
#endif
}

/*!
 Compare function used by bsearch()

 \param l langauge code
 \param e language table entry
 \return negative, 0, or positive
 */
static int s_compareB(const void * l, const void *e)
{
	const gchar * L   = static_cast<const gchar *>(l);
	const UT_LangRecord * E = static_cast<const UT_LangRecord *>(e);

#if 0
	// as long as bsearch is used searching for lang codes this is wrong
	if (E->m_nID == XAP_STRING_ID_LANG_0)
		return 1;
	else if (L == s_Table[0].m_szLangName)
		return -1;
	else if (strcmp(L, s_Table[0].m_szLangName) == 0)
		return -1;

	return g_utf8_collate(L, E->m_szLangName);
#else
	// make the comparison case insensitive to cope with buggy systems 
	return g_ascii_strcasecmp(L, E->m_szLangCode);
#endif
}

/*!
 Set the language name attribut in the language table. It takes the localized
 names from the available stringset. This function is supplied to set the names
 after the stringset is known to the application.
*/
void UT_Language_updateLanguageNames() 
{
    const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
    UT_return_if_fail(pSS);
    
    for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
    {
        s_Table[i].m_szLangName;
    }

    qsort(&s_Table[0], G_N_ELEMENTS(s_Table), sizeof(UT_LangRecord), s_compareQ);
}


bool UT_Language::s_Init = true;

/*!
 The constructor looks up the translations for the language code and sorts the table
  alphabetically by the language name
 */
UT_Language::UT_Language()
{
	if(s_Init) // do this once here
    {
        UT_Language_updateLanguageNames();
        s_Init = false;
    }
}

UT_uint32 UT_Language::getCount()
{
	return (G_N_ELEMENTS(s_Table));
}

const gchar * UT_Language::getNthLangCode(UT_uint32 n)
{
	return (s_Table[n].m_szLangCode);
}

const gchar * UT_Language::getNthLangName(UT_uint32 n)
{
	return (s_Table[n].m_szLangName);
}

UT_uint32 UT_Language::getNthId(UT_uint32 n)
{
	return (s_Table[n].m_nID);
}

const gchar * UT_Language::getCodeFromName(const gchar * szName)
{
	for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
	{
		// make the comparison case insensitive to cope with buggy systems 
		if(!g_ascii_strcasecmp(szName, s_Table[i].m_szLangName))
			return s_Table[i].m_szLangCode;
	}

	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", szName));
	return NULL;
}

UT_uint32 UT_Language::getIndxFromCode(const gchar * szCode)
{
	for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
	{
		// make the comparison case insensitive to cope with buggy systems 
		if(!g_ascii_strcasecmp(szCode, s_Table[i].m_szLangCode))
			return i;
	}

	// see if our tables contain short version of this code, e.g., hr instead of hr-HR
	static gchar szShortCode[7];
	strncpy(szShortCode, szCode,6);
	szShortCode[6] = 0;

	char * dash = strchr(szShortCode, '-');
	if(dash)
	{
		*dash = 0;

		for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
		{
			// make the comparison case insensitive to cope with buggy systems 
			if(!g_ascii_strcasecmp(szShortCode, s_Table[i].m_szLangCode))
				return i;
		}
	}
		
	
	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", szCode));
	return 0;
}

UT_uint32 UT_Language::getIdFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = getLangRecordFromCode(szCode);

	if(e)
		return e->m_nID;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return 0;
	}
	
}

// This function is not as useless as might seem; it takes a pointer
// to a property string, finds the same property in the static table
// and returns the pointer to it. This is used by fp_TextRun to set its
// m_pLanguage member; by always refering into the static table it is
// possible to compare the language property by simply comparing the
// pointers, rather than having to use strcmp()

const gchar * UT_Language::getCodeFromCode(const gchar * szName)
{
	const UT_LangRecord * e = getLangRecordFromCode(szName);
	
	if(e)
		return e->m_szLangCode;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szName));
		return 0;
	}
}

UT_LANGUAGE_DIR UT_Language::getDirFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = getLangRecordFromCode(szCode);
	
	if(e)
		return e->m_eDir;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return UTLANG_LTR;
	}
}

const UT_LangRecord * UT_Language::getLangRecordFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = static_cast<UT_LangRecord *>(bsearch(szCode, s_Table,
																   G_N_ELEMENTS(s_Table),
																   sizeof(UT_LangRecord), s_compareB));
	if(!e)
	{
		// see if our tables contain short version of this code, e.g., hr instead of hr-HR
		static gchar szShortCode[7];
		strncpy(szShortCode, szCode,6);
		szShortCode[6] = 0;

		char * dash = strchr(szShortCode, '-');
		if(dash)
		{
			*dash = 0;
			e = static_cast<UT_LangRecord *>(bsearch(szShortCode, s_Table,
													 G_N_ELEMENTS(s_Table),
													 sizeof(UT_LangRecord), s_compareB));

			if(e)
				return e;
		}
		
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return 0;
	}

	return e;
}
	
