/* AbiSource Application Framework
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

#include <stdlib.h>
#include <stdio.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_Strings.h"

#define NrElements(a)		(sizeof(a) / sizeof(a[0]))
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

XAP_StringSet::XAP_StringSet(XAP_App * pApp, const XML_Char * szLanguageName)
{
	m_pApp = pApp;
	
	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		UT_XML_cloneString((XML_Char *&)m_szLanguageName,szLanguageName);
}

XAP_StringSet::~XAP_StringSet(void)
{
	if (m_szLanguageName)
		free((XML_Char *)m_szLanguageName);
}

const XML_Char * XAP_StringSet::getLanguageName(void) const
{
	return m_szLanguageName;
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in (english) strings
// (there will only be one instance of this sub-class)
//////////////////////////////////////////////////////////////////

XAP_BuiltinStringSet::XAP_BuiltinStringSet(XAP_App * pApp, const XML_Char * szLanguageName)
	: XAP_StringSet(pApp,szLanguageName)
{
#define dcl(id,s)					s,

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
	: XAP_StringSet(pApp,NULL)
{
	m_pFallbackStringSet = NULL;
	
	setValue(XAP_STRING_ID__FIRST__,0);			// bogus zero element
}

XAP_DiskStringSet::~XAP_DiskStringSet(void)
{
	UT_sint32 kLimit = (UT_sint32)m_vecStringsXAP.getItemCount();
	UT_sint32 k;

	for (k=kLimit-1; k>=0; k--)
	{
		XML_Char * sz = (XML_Char *)m_vecStringsXAP.getNthItem(k);
		if (sz)
			free(sz);
	}

	// we didn't create the fallback set, but we inherit ownership of it.
	DELETEP(m_pFallbackStringSet);
}

UT_Bool XAP_DiskStringSet::setLanguage(const XML_Char * szLanguageName)
{
	if (m_szLanguageName)
		free((XML_Char *)m_szLanguageName);
	m_szLanguageName = NULL;
	if (szLanguageName && *szLanguageName)
		UT_XML_cloneString((XML_Char *&)m_szLanguageName,szLanguageName);
	return UT_TRUE;
}

void XAP_DiskStringSet::setFallbackStringSet(XAP_StringSet * pFallback)
{
	m_pFallbackStringSet = pFallback;
}

UT_Bool XAP_DiskStringSet::setValue(XAP_String_Id id, const XML_Char * szString)
{
	XML_Char * szDup = NULL;
	if (szString && *szString)
		if (!UT_XML_cloneString(szDup,szString))
			return UT_FALSE;

	void * pOldValue = NULL;
	UT_Bool bResult = (m_vecStringsXAP.setNthItem(id,szDup,&pOldValue) == 0);
	UT_ASSERT(pOldValue == NULL);		// duplicate string for this id

	return bResult;
}

const XML_Char * XAP_DiskStringSet::getValue(XAP_String_Id id) const
{
	UT_uint32 kLimit = m_vecStringsXAP.getItemCount();

	if (id < kLimit)
	{
		const XML_Char * szValue = (const XML_Char *)m_vecStringsXAP.getNthItem(id);
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

#define dcl(id,s)					{ #id, XAP_STRING_ID_##id },

static struct { const XML_Char * szName; XAP_String_Id id; } s_map[] =
{
#include "xap_String_Id.h"
};
			
#undef dcl

//////////////////////////////////////////////////////////////////

UT_Bool XAP_DiskStringSet::setValue(const XML_Char * szId, const XML_Char * szString)
{
	if (!szId || !*szId || !szString || !*szString)
		return UT_TRUE;
	
	UT_uint32 kLimit = NrElements(s_map);
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
		if (UT_XML_stricmp(s_map[k].szName,szId) == 0)
			return XAP_DiskStringSet::setValue(s_map[k].id,szString);

	// TODO should we promote this message to a message box ??
	UT_DEBUGMSG(("Unknown ID in string file [%s=\"%s\"]\n",szId,szString));
	return UT_FALSE;
}

/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	XAP_DiskStringSet * pDisk = (XAP_DiskStringSet *)userData;
	pDisk->_startElement(name,atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	XAP_DiskStringSet * pDisk = (XAP_DiskStringSet *)userData;
	pDisk->_endElement(name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	XAP_DiskStringSet * pDisk = (XAP_DiskStringSet *)userData;
	pDisk->_charData(s,len);
}
/*****************************************************************/

void XAP_DiskStringSet::_startElement(const XML_Char *name, const XML_Char **atts)
{
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (UT_XML_stricmp(name, "AbiStrings") == 0)
	{
		// we expect something of the form:
		// <AbiStrings app="AbiWord" ver="1.0" language="EnUS">...</AbiStrings>

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0], "app") == 0)
			{
				const char * szThisApp = m_pApp->getApplicationName();
				UT_DEBUGMSG(("Found strings file for application [%s] (this is [%s]).\n",
							a[1],szThisApp));
				if (UT_XML_stricmp(a[1],szThisApp) != 0)
				{
					UT_DEBUGMSG(("Strings file does not match this application.\n"));
					goto InvalidFileError;
				}
			}
			else if (UT_XML_stricmp(a[0], "ver") == 0)
			{
				// TODO test version number
			}
			else if (UT_XML_stricmp(a[0], "language") == 0)
			{
				UT_DEBUGMSG(("Found strings for language [%s].\n",a[1]));
				if (!setLanguage(a[1]))
					goto MemoryError;
			}

			a += 2;
		}
	}
	else if (UT_XML_stricmp(name, "Strings") == 0)
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
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0],"class") == 0)
				continue;
			
			if (!setValue(a[0],a[1]))
			{
				UT_DEBUGMSG(("UNKNOWN StringId [%s] value [%s]\n",a[0],a[1]));
			}
		}
	}

	// successful parse of tag...

	return;								// success

MemoryError:
	UT_DEBUGMSG(("Memory error parsing strings file.\n"));
InvalidFileError:
	m_parserState.m_parserStatus = UT_FALSE;			// cause parser driver to bail
	return;
}

void XAP_DiskStringSet::_endElement(const XML_Char * /* name */)
{
	// everything in this file is contained in start-tags
	return;
}

void XAP_DiskStringSet::_charData(const XML_Char * /* s */, int /* len */)
{
	// everything in this file is contained in start-tags
	return;
}

UT_Bool XAP_DiskStringSet::loadStringsFromDisk(const char * szFilename)
{
	UT_Bool bResult = UT_FALSE;			// assume failure
	FILE * fp = NULL;
	XML_Parser parser = NULL;
	int done = 0;
	char buf[4096];

	m_parserState.m_parserStatus = UT_TRUE;

	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("Invalid pathname for Strings file.\n"));
		goto Cleanup;
	}

	fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open Strings file [%s].\n",szFilename));
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	if (!parser)
	{
		UT_DEBUGMSG(("Could not create parser for Strings file [%s].\n",szFilename));
		goto Cleanup;
	}
	
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						XML_ErrorString(XML_GetErrorCode(parser)),
						XML_GetCurrentLineNumber(parser)));
			goto Cleanup;
		}

		if (!m_parserState.m_parserStatus)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
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

	bResult = UT_TRUE;

Cleanup:
	if (parser)
		XML_ParserFree(parser);
	if (fp)
		fclose(fp);
	return bResult;
}
