/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * BIDI Copyright (C) 2001,2002 Tomas Frydrych
 * Copyright (C) 2002 Dom Lachowicz
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_bytebuf.h"
#include "ut_wctomb.h"
#include "ut_iconv.h"
#include "ut_Language.h"

#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_EncodingManager.h"

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

XAP_StringSet::XAP_StringSet(XAP_App * pApp, const gchar * szLanguageName)
  : m_encoding("UTF-8")
{
	m_pApp = pApp;

	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		m_szLanguageName = g_strdup(szLanguageName);
}

XAP_StringSet::~XAP_StringSet(void)
{
	if (m_szLanguageName)
		g_free(const_cast<gchar *>(m_szLanguageName));
}

const gchar * XAP_StringSet::getLanguageName(void) const
{
	return m_szLanguageName;
}

bool XAP_StringSet::getValue(XAP_String_Id id, const char * inEncoding, std::string &s) const
{
	const char * toTranslate = getValue(id);

	UT_return_val_if_fail(toTranslate != NULL, false);

	if(!strcmp(m_encoding.c_str(),inEncoding))
	{
		s = toTranslate;
	}
	else
	{
	        UT_iconv_t conv = UT_iconv_open(inEncoding, m_encoding.c_str());
		UT_return_val_if_fail(UT_iconv_isValid(conv), false);
	  
		char * translated = UT_convert_cd(toTranslate, strlen (toTranslate)+1, conv, NULL, NULL);
		
		UT_iconv_close(conv);
		
		UT_return_val_if_fail(translated, false);
		s = translated;
		
		g_free(translated);
	}
	
	return true;
}

bool XAP_StringSet::getValueUTF8(XAP_String_Id id, std::string & s) const
{	
	return getValue(id, "UTF-8", s);
}

void XAP_StringSet::setEncoding(const gchar * inEncoding)
{
  UT_return_if_fail(inEncoding != 0);
  m_encoding = inEncoding;
}

const char * XAP_StringSet::getEncoding() const
{
  return m_encoding.c_str();
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in (english) strings
// (there will only be one instance of this sub-class)
// (since these strings are English, we will not bother with any
// (bidi processing, we only need this in the disk stringset)
//////////////////////////////////////////////////////////////////

XAP_BuiltinStringSet::XAP_BuiltinStringSet(XAP_App * pApp, const gchar * szLanguageName)
	: XAP_StringSet(pApp,szLanguageName)
{
#define dcl(id,s)					static_cast<const gchar *>(s),

	static const gchar * s_a[] =
	{
		dcl(__FIRST__,0)			// bogus entry for zero
#include "xap_String_Id.h"
		dcl(__LAST__,0)				// bogus entry for end
	};

	m_arrayXAP = s_a;

#undef dcl
}

XAP_BuiltinStringSet::~XAP_BuiltinStringSet(void)
{
}

const gchar * XAP_BuiltinStringSet::getValue(XAP_String_Id id) const
{
	if ( (id > XAP_STRING_ID__FIRST__) && (id < XAP_STRING_ID__LAST__) )
		return m_arrayXAP[id];

	return NULL;
}

//////////////////////////////////////////////////////////////////
// a sub-class to deal with disk-based string sets (translations)
// (a unique one of these will be instantiated for each language
// that we load -- or rather one for each time the user switches
// languages and we load another one from disk)
//////////////////////////////////////////////////////////////////

XAP_DiskStringSet::XAP_DiskStringSet(XAP_App * pApp)
	: XAP_StringSet(pApp,NULL), 
	  m_vecStringsXAP(XAP_STRING_ID__LAST__ - XAP_STRING_ID__FIRST__ + 1, 4, true)
{
	m_pFallbackStringSet = NULL;

	XAP_DiskStringSet::setValue(XAP_STRING_ID__FIRST__,0);			// bogus zero element
}

XAP_DiskStringSet::~XAP_DiskStringSet(void)
{
	UT_sint32 kLimit = m_vecStringsXAP.getItemCount();
	UT_sint32 k;

	for (k=kLimit-1; k>=0; k--)
	{
		gchar * sz = m_vecStringsXAP.getNthItem(k);
		if (sz)
			g_free(sz);
	}

	// we didn't create the fallback set, but we inherit ownership of it.
	DELETEP(m_pFallbackStringSet);
}

bool XAP_DiskStringSet::setLanguage(const gchar * szLanguageName)
{
	if (m_szLanguageName)
		g_free(const_cast<gchar *>(m_szLanguageName));
	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		m_szLanguageName = g_strdup(szLanguageName);
	return true;
}

void XAP_DiskStringSet::setFallbackStringSet(XAP_StringSet * pFallback)
{
	m_pFallbackStringSet = pFallback;
}

bool XAP_DiskStringSet::setValue(XAP_String_Id id, const gchar * szString)
{
	bool bFoundMultiByte = false;
	gchar * szDup = NULL;
	int length;
	const void* ptr;
	
	if (szString && *szString)
	{
		UT_GrowBuf gb;
		UT_decodeUTF8string(szString,strlen(szString),&gb);

		// TODO The strings that we use (for dialogs and etc) are currently
		// TODO limited to single-byte encodings by the code below.

		int kLimit=gb.getLength();
		UT_UCS4Char * p = reinterpret_cast<UT_UCS4Char*>(gb.getPointer(0));
		UT_ByteBuf str;

		// now we run this string through fribidi
		if(XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_NONE)
		{
			if (p && *p)
			{
				UT_UCS4Char * fbdStr2  = new UT_UCS4Char [kLimit + 1];
				UT_ASSERT(fbdStr2);

				UT_sint32 i;

				// Testing the first char is not good enough; we really need to get this from the
				// language
				// FriBidiCharType fbdDomDir = fribidi_get_type(fbdStr[0]);
				UT_BidiCharType iDomDir = UT_BIDI_LTR;
				UT_Language l;
				if(UTLANG_RTL == l.getDirFromCode(getLanguageName()))
				   iDomDir = UT_BIDI_RTL;

				UT_bidiReorderString(p, kLimit, iDomDir, fbdStr2);

				for(i = 0; i < kLimit; i++)
				{
					p[i] = fbdStr2[i];
				}

				UT_ASSERT_HARMLESS(p[i] == 0);
				delete[] fbdStr2;
			}
		}				
		
		if(strcmp(getEncoding(), "UTF-8"))
		{
			UT_Wctomb wctomb_conv(getEncoding());
	
			char letter_buf[20];
			for (int k=0; k<kLimit; k++)
			{
			    if (wctomb_conv.wctomb(letter_buf,length, p[k])) {
				str.append(reinterpret_cast<const UT_Byte*>(&letter_buf[0]),length);
			    };
			}
			length = str.getLength();			
			ptr = str.getPointer(0);
		} else
		{	
			length = strlen(szString);
			ptr = szString;
		}
		szDup = static_cast<gchar *>(g_try_malloc(length+1));
		if (!szDup)
				return false;
		memcpy(szDup,ptr,length);
		szDup[length]='\0';	
	}

    gchar* pOldValue = NULL;
	bool bResult = (m_vecStringsXAP.setNthItem(id,szDup,&pOldValue) == 0);
	UT_ASSERT(pOldValue == NULL);		// duplicate string for this id

	if (bFoundMultiByte)
	{
		UT_DEBUGMSG(("WARNING: DiskStringSet: Found Multi-Byte char in String [%s][id %d] (we mapped it to [%s])\n",szString,id,szDup));
	}

	return bResult;
}

const gchar * XAP_DiskStringSet::getValue(XAP_String_Id id) const
{
	UT_uint32 kLimit = m_vecStringsXAP.getItemCount();

	if (id < kLimit)
	{
		const gchar * szValue = m_vecStringsXAP.getNthItem(id);
		if (szValue)
			return szValue;
	}

	if (m_pFallbackStringSet)
		return m_pFallbackStringSet->getValue(id);

	return NULL;
}

//////////////////////////////////////////////////////////////////
// build a static table to map id by names into numbers
//////////////////////////////////////////////////////////////////

#define dcl(id,s)					{ (const gchar *) #id, XAP_STRING_ID_##id },

static struct { const gchar * szName; XAP_String_Id id; } s_map[] =
{
#include "xap_String_Id.h"
};

#undef dcl

//////////////////////////////////////////////////////////////////

bool XAP_DiskStringSet::setValue(const gchar * szId, const gchar * szString)
{
	if (!szId || !*szId || !szString || !*szString)
		return true;

	gchar *id;
	// Build a hash table the first time that the function is called
 	if (m_hash.size() == 0) {
 		UT_uint32 k, kLimit = G_N_ELEMENTS(s_map);

		for (k=0; k<kLimit; k++) {
 			id = g_ascii_strdown (s_map[k].szName, -1);
 			m_hash[ std::string(id) ] = k + 1;
 			FREEP(id);
 		}
	}

 	id = g_ascii_strdown (szId, -1); 	
	std::map<std::string, UT_uint32>::iterator  iter = m_hash.find( id );
 	FREEP(id);

	if (iter != m_hash.end())
		return setValue(s_map[iter->second - 1].id, szString);	

	// TODO should we promote this message to a message box ??
	UT_DEBUGMSG(("Unknown ID in string file [%s=\"%s\"]\n",szId,szString));
	return false;
}

/*****************************************************************/

void XAP_DiskStringSet::startElement(const gchar *name, const gchar **atts)
{
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (strcmp(name, "AbiStrings") == 0)
	{
		// we expect something of the form:
		// <AbiStrings ver="1.0" language="en-US">...</AbiStrings>

		const gchar ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(a[0], "ver") == 0)
			{
				// TODO test version number
			}
			else if (strcmp(a[0], "language") == 0)
			{
				UT_DEBUGMSG(("Found strings for language [%s].\n",a[1]));
				if (!setLanguage(a[1]))
					goto MemoryError;
			}

			a += 2;
		}
	}
	else if (strcmp(name, "Strings") == 0)
	{
		// we found a set of strings.  we expect something of the form:
		// <Strings class="type" n0="v0" n1="v1" ... />
		// where the [nk,vk] are arbitrary name/value pairs that mean
		// something to the application.  each of the nk's corresponds
		// to an XAP_STRING_ID_nk or AP_STRING_ID_nk.
		//
		// class="type" is a way of indicating whether the string is an
		// XAP or AP string.  this information is strictly for the
		// convenience of translators -- we don't care.  on input, we
		// allow there to be as many <Strings.../> as the translator
		// wants.  hopefully, this will make it easier to deal with
		// multiple applications and to deal with multiple language
		// dialects.
		//
		// if there are duplicated name/value pairs our behavior is
		// undefined -- we remember the last one that the XML parser
		// give us.

		const gchar ** a;
		for (a = atts; (*a); a += 2)
		{
			if (strcmp(a[0], "class") == 0)
				continue;

			if (!setValue(a[0], a[1]))
			{
				UT_DEBUGMSG(("UNKNOWN StringId [%s] value [%s]\n", a[0], a[1]));
			}
		}
	}

	// successful parse of tag...

	return;								// success

MemoryError:
	UT_DEBUGMSG(("Memory error parsing strings file.\n"));
	m_parserState.m_parserStatus = false;			// cause parser driver to bail
	return;
}

void XAP_DiskStringSet::endElement(const gchar * /* name */)
{
	// everything in this file is contained in start-tags
	return;
}

void XAP_DiskStringSet::charData(const gchar * /* s */, int /* len */)
{
	// everything in this file is contained in start-tags
	return;
}


bool XAP_DiskStringSet::loadStringsFromDisk(const char * szFilename)
{
	bool bResult = false;			// assume failure

	m_parserState.m_parserStatus = true;

	UT_XML parser;

	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("Invalid pathname for Strings file.\n"));
		goto Cleanup;
	}

	parser.setListener (this);
	if ((parser.parse (szFilename) != UT_OK) || (!m_parserState.m_parserStatus))
	{
		UT_DEBUGMSG(("Problem reading document\n"));
		goto Cleanup;
	}

	bResult = true;

Cleanup:

	return bResult;
}
