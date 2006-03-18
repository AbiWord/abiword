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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_bytebuf.h"
#include "ut_wctomb.h"
#include "ut_iconv.h"
#include "ut_exception.h"
#include "ut_Language.h"

#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_EncodingManager.h"

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

XAP_StringSet::XAP_StringSet(XAP_App * pApp, const XML_Char * szLanguageName)
  : m_encoding("UTF-8")
{
	m_pApp = pApp;

	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		UT_XML_cloneString((XML_Char *&)m_szLanguageName,szLanguageName);
}

XAP_StringSet::~XAP_StringSet(void)
{
	if (m_szLanguageName)
		free(const_cast<XML_Char *>(m_szLanguageName));
}

const XML_Char * XAP_StringSet::getLanguageName(void) const
{
	return m_szLanguageName;
}

bool XAP_StringSet::getValue(XAP_String_Id id, const char * inEncoding, UT_String &s) const
{
	const char * toTranslate = getValue(id);

	if(!UT_strcmp(m_encoding.c_str(),inEncoding))
	{
		s = toTranslate;
	}
	else
	{
		UT_TRY
			{
				auto_iconv cd(m_encoding.c_str(), inEncoding);
				char * translated = UT_convert_cd(toTranslate, -1, cd, NULL, NULL);
				
				UT_return_val_if_fail(translated, false);
				s = translated;
				
				free(translated);
			}
		UT_CATCH(UT_CATCH_ANY)
			{
				return false;
			}
		UT_END_CATCH
	}
	
	return true;
}

bool XAP_StringSet::getValueUTF8(XAP_String_Id id, UT_UTF8String & s) const
{	
	const char * toTranslate = getValue(id);

	if(!UT_strcmp(m_encoding.c_str(), "UTF-8"))
	{
		s = toTranslate;
	}
	else
	{
		UT_TRY
			{
				auto_iconv cd(m_encoding.c_str(), "UTF-8");
				char * translated = UT_convert_cd(toTranslate, -1, cd, NULL, NULL);
				
				UT_return_val_if_fail(translated, false);
				s = translated;
				
				free(translated);
			}
		UT_CATCH(UT_CATCH_ANY)
			{
				return false;
			}
		UT_END_CATCH
	}
	
	return true;
}

void XAP_StringSet::setEncoding(const XML_Char * inEncoding)
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

XAP_BuiltinStringSet::XAP_BuiltinStringSet(XAP_App * pApp, const XML_Char * szLanguageName)
	: XAP_StringSet(pApp,szLanguageName)
{
#define dcl(id,s)					static_cast<const XML_Char *>(s),

	static const XML_Char * s_a[] =
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

const XML_Char * XAP_BuiltinStringSet::getValue(XAP_String_Id id) const
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
	UT_sint32 kLimit = static_cast<UT_sint32>(m_vecStringsXAP.getItemCount());
	UT_sint32 k;

	for (k=kLimit-1; k>=0; k--)
	{
		XML_Char * sz = m_vecStringsXAP.getNthItem(k);
		if (sz)
			free(sz);
	}

	// we didn't create the fallback set, but we inherit ownership of it.
	DELETEP(m_pFallbackStringSet);
}

bool XAP_DiskStringSet::setLanguage(const XML_Char * szLanguageName)
{
	if (m_szLanguageName)
		free(const_cast<XML_Char *>(m_szLanguageName));
	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		UT_XML_cloneString((XML_Char *&)m_szLanguageName,szLanguageName);
	return true;
}

void XAP_DiskStringSet::setFallbackStringSet(XAP_StringSet * pFallback)
{
	m_pFallbackStringSet = pFallback;
}

bool XAP_DiskStringSet::setValue(XAP_String_Id id, const XML_Char * szString)
{
	bool bFoundMultiByte = false;
	XML_Char * szDup = NULL;
	if (szString && *szString)
	{
		UT_GrowBuf gb;
		UT_decodeUTF8string(szString,UT_XML_strlen(szString),&gb);

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

		UT_Wctomb wctomb_conv(XAP_App::getApp()->getDefaultEncoding());

		char letter_buf[20];
		int length;
		for (int k=0; k<kLimit; k++)
		{
		    if (wctomb_conv.wctomb(letter_buf,length, p[k])) {
			str.append(reinterpret_cast<const UT_Byte*>(&letter_buf[0]),length);
		    };
		}
		length = str.getLength();
		szDup = static_cast<XML_Char *>(malloc(length+1));
		if (!szDup)
			return false;
		memcpy(szDup,str.getPointer(0),length);
		szDup[length]='\0';
	}

    XML_Char* pOldValue = NULL;
	bool bResult = (m_vecStringsXAP.setNthItem(id,szDup,&pOldValue) == 0);
	UT_ASSERT(pOldValue == NULL);		// duplicate string for this id

	if (bFoundMultiByte)
	{
		UT_DEBUGMSG(("WARNING: DiskStringSet: Found Multi-Byte char in String [%s][id %d] (we mapped it to [%s])\n",szString,id,szDup));
	}

	return bResult;
}

const XML_Char * XAP_DiskStringSet::getValue(XAP_String_Id id) const
{
	UT_uint32 kLimit = m_vecStringsXAP.getItemCount();

	if (id < kLimit)
	{
		const XML_Char * szValue = m_vecStringsXAP.getNthItem(id);
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

#define dcl(id,s)					{ (const XML_Char *) #id, XAP_STRING_ID_##id },

static struct { const XML_Char * szName; XAP_String_Id id; } s_map[] =
{
#include "xap_String_Id.h"
};

#undef dcl

//////////////////////////////////////////////////////////////////

bool XAP_DiskStringSet::setValue(const XML_Char * szId, const XML_Char * szString)
{
	if (!szId || !*szId || !szString || !*szString)
		return true;

	UT_uint32 kLimit = NrElements(s_map);
	UT_uint32 k;

	// we use predefined IDs to access the strings, so there is no need to do
	// case-insensitive comparison (and it is costing us lot of time, particularly at
	// startup).
	for (k=0; k<kLimit; k++)
		if (UT_XML_strcmp(s_map[k].szName,szId) == 0)
			return XAP_DiskStringSet::setValue(s_map[k].id,szString);

	// TODO should we promote this message to a message box ??
	UT_DEBUGMSG(("Unknown ID in string file [%s=\"%s\"]\n",szId,szString));
	return false;
}

/*****************************************************************/

void XAP_DiskStringSet::startElement(const XML_Char *name, const XML_Char **atts)
{
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (strcmp(const_cast<char*>(name), "AbiStrings") == 0)
	{
		// we expect something of the form:
		// <AbiStrings app="AbiWord" ver="1.0" language="en-US">...</AbiStrings>

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(const_cast<char*>(a[0]), "app") == 0)
			{
				const char * szThisApp = m_pApp->getApplicationName();
				UT_DEBUGMSG(("Found strings file for application [%s] (this is [%s]).\n",
							a[1],szThisApp));
				if (strcmp(const_cast<char*>(a[1]),szThisApp) != 0)
				{
					UT_DEBUGMSG(("Strings file does not match this application.\n"));
					goto InvalidFileError;
				}
			}
			else if (strcmp(const_cast<char*>(a[0]), "ver") == 0)
			{
				// TODO test version number
			}
			else if (strcmp(const_cast<char*>(a[0]), "language") == 0)
			{
				UT_DEBUGMSG(("Found strings for language [%s].\n",a[1]));
				if (!setLanguage(a[1]))
					goto MemoryError;
			}

			a += 2;
		}
	}
	else if (strcmp(const_cast<char*>(name), "Strings") == 0)
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

		const XML_Char ** a;
		for (a = atts; (*a); a += 2)
		{
			// require a value for each attribute keyword
			UT_ASSERT((a[1] && *a[1]));

			if (strcmp(const_cast<char*>(a[0]), "class") == 0)
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
InvalidFileError:
	m_parserState.m_parserStatus = false;			// cause parser driver to bail
	return;
}

void XAP_DiskStringSet::endElement(const XML_Char * /* name */)
{
	// everything in this file is contained in start-tags
	return;
}

void XAP_DiskStringSet::charData(const XML_Char * /* s */, int /* len */)
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

	// we succeeded in parsing the file,
	// now check for higher-level consistency.

#ifdef DEBUG
	{
		// TODO should we promote this test to be production code
		// TODO and maybe raise a message box ??
		UT_uint32 kLimit = NrElements(s_map);
		UT_uint32 k;

		for (k=0; k<kLimit; k++)
		{
			const XML_Char * szValue = XAP_DiskStringSet::getValue(s_map[k].id);
			if (!szValue || !*szValue)
				UT_DEBUGMSG(("WARNING: Translation for id [%s] not found.\n",s_map[k].szName));
		}
	}
#endif

	bResult = true;

Cleanup:

	return bResult;
}
