/* AbiWord
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
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_bytebuf.h"
#include "ut_wctomb.h"
#include "ap_Strings.h"
#include "xap_EncodingManager.h"

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in (english) strings
// (there will only be one instance of this sub-class)
//////////////////////////////////////////////////////////////////

AP_BuiltinStringSet::AP_BuiltinStringSet(XAP_App * pApp, const XML_Char * szLanguageName)
	: XAP_BuiltinStringSet(pApp,szLanguageName)
{
#define dcl(id,s)					s,

	static const XML_Char * s_a[] =
	{
		dcl(__FIRST__,0)			// bogus entry for zero
#include "ap_String_Id.h"
		dcl(__LAST__,0)				// bogus entry for end
	};

	m_arrayAP = s_a;

#undef dcl
}

AP_BuiltinStringSet::~AP_BuiltinStringSet(void)
{
}

const XML_Char * AP_BuiltinStringSet::getValue(XAP_String_Id id) const
{
	// if it's in our range, we fetch it.
	// otherwise, we hand it down to the base class.
	
	if ( (id > AP_STRING_ID__FIRST__) && (id < AP_STRING_ID__LAST__) )
		return m_arrayAP[id-AP_STRING_ID__FIRST__];

	return XAP_BuiltinStringSet::getValue(id);
}

#ifdef DEBUG
static void s_dumpXMLpair(FILE * fp, const XML_Char *szID, const XML_Char *sz)
{
	fprintf(fp,"%s=\"",szID);

	for (; *sz; ++sz) 
	{
		switch (*sz) 
		{
		case '&':
			fputs("&amp;", fp);
			break;
		case '<':
			fputs("&lt;", fp);
			break;
		case '>':
			fputs("&gt;", fp);
			break;
		case '"':
			fputs("&quot;", fp);
			break;
		case 9:
		case 10:
		case 13:
			fprintf(fp, "&#%d;", *sz);
			break;
		default:
			putc(*sz, fp);
			break;
		}
	}

	fprintf(fp,"\"\n");
}

bool AP_BuiltinStringSet::dumpBuiltinSet(const char * szFilename) const
{
	// Dump a full set of english strings.  The resulting file
	// can then be translated and later loaded as a DiskStringSet
	// for the other language.
	
	bool bResult = false;			// assume failure
	FILE * fp = NULL;
	fp = fopen(szFilename, "w");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open String file [%s].\n",szFilename));
		return false;
	}

	UT_DEBUGMSG(("Dumping English strings into String file [%s].\n",szFilename));
	
	// most translators need to explicitly set an encoding, so provide a sample

	fprintf(fp,"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
	fprintf(fp,"\n");

	// write a comment block as a prolog.
	// NOTE: this is human readable information only.
	
	fprintf(fp,"<!-- ==============================================================  -->\n");
	fprintf(fp,"<!-- This file contains AbiWord Strings.  AbiWord is an Open Source  -->\n");
	fprintf(fp,"<!-- word processor developed by AbiSource, Inc.  Information about  -->\n");
	fprintf(fp,"<!-- this application can be found at http://www.abisource.com       -->\n");
	fprintf(fp,"<!-- This file contains the string translations for one language.    -->\n");
	fprintf(fp,"<!-- This file is covered by the GNU Public License (GPL).           -->\n");
	fprintf(fp,"<!-- ==============================================================  -->\n");
	fprintf(fp,"\n");
	fprintf(fp,"<!-- _Language_ translations provided by _name_ <_email_> -->\n");
	fprintf(fp,"\n");

	// end of prolog.
	// now we begin the actual document.

	//////////////////////////////////////////////////////////////////
	// declare a static table of all the ids and strings
	//////////////////////////////////////////////////////////////////

#define dcl(id,s)				{ #id,s },

	static struct { const XML_Char * szId; const XML_Char * szString; } s_mapXAP[] =
	{
#include "xap_String_Id.h"
	};
	
	static struct { const XML_Char * szId; const XML_Char * szString; } s_mapAP[] =
	{
#include "ap_String_Id.h"
	};

#undef dcl

	fprintf(fp,"\n<AbiStrings app=\"%s\" ver=\"%s\" language=\"%s\">\n",
			m_pApp->getApplicationName(), "1.0", getLanguageName());
	{
		UT_uint32 k;
		
		fprintf(fp,"\n<Strings\tclass=\"XAP\"\n");
		for (k=0; k<NrElements(s_mapXAP); k++)
			s_dumpXMLpair(fp,s_mapXAP[k].szId,s_mapXAP[k].szString);
		fprintf(fp,"/>\n");

		fprintf(fp,"\n<Strings\tclass=\"AP\"\n");
		for (k=0; k<NrElements(s_mapAP); k++)
			s_dumpXMLpair(fp,s_mapAP[k].szId,s_mapAP[k].szString);
		fprintf(fp,"/>\n");
	}

	fprintf(fp,"\n</AbiStrings>\n");
	
	if (fp)
		fclose(fp);
	return bResult;
}

#endif // DEBUG

//////////////////////////////////////////////////////////////////
// a sub-class to deal with disk-based string sets (translations)
// (a unique one of these will be instantiated for each language
// that we load -- or rather one for each time the user switches
// languages and we load another one from disk)
//////////////////////////////////////////////////////////////////

AP_DiskStringSet::AP_DiskStringSet(XAP_App * pApp)
	: XAP_DiskStringSet(pApp)
{
	setValue(AP_STRING_ID__FIRST__,0);			// bogus zero element
}

AP_DiskStringSet::~AP_DiskStringSet(void)
{
	UT_sint32 kLimit = (UT_sint32)m_vecStringsAP.getItemCount();
	UT_sint32 k;

	for (k=kLimit-1; k>=0; k--)
	{
		XML_Char * sz = (XML_Char *)m_vecStringsAP.getNthItem(k);
		if (sz)
			free(sz);
	}
}

bool AP_DiskStringSet::setValue(XAP_String_Id id, const XML_Char * szString)
{
	if (id < AP_STRING_ID__FIRST__)
		return XAP_DiskStringSet::setValue(id,szString);

	bool bFoundMultiByte = false;
	XML_Char * szDup = NULL;
	if (szString && *szString)
	{
		UT_GrowBuf gb;
		UT_decodeUTF8string(szString,UT_XML_strlen(szString),&gb);

		// TODO The strings that we use (for dialogs and etc) are currently
		// TODO limited to single-byte encodings by the code below.  

		int kLimit=gb.getLength();
		UT_uint16 * p=gb.getPointer(0);
		UT_ByteBuf str;		
		UT_Wctomb wctomb_conv;
		char letter_buf[20];
		int length;
		for (int k=0; k<kLimit; k++)
		{
		    if (wctomb_conv.wctomb(letter_buf,length,(wchar_t)p[k])) {
			str.append((UT_Byte*)letter_buf,length);
		    };
		}
		length = str.getLength();
		szDup = (XML_Char *)malloc(length+1);
		if (!szDup)
			return false;
		memcpy(szDup,str.getPointer(0),length);
		szDup[length]='\0';
	}

	void * pOldValue = NULL;
	bool bResult = (m_vecStringsAP.setNthItem(id-AP_STRING_ID__FIRST__,szDup,&pOldValue) == 0);
	UT_ASSERT(pOldValue == NULL);		// duplicate string for this id

	if (bFoundMultiByte)
	{
		UT_DEBUGMSG(("WARNING: DiskStringSet: Found Multi-Byte char in String [%s][id %d] (we mapped it to [%s])\n",szString,id,szDup));
	}
	
	return bResult;
}

const XML_Char * AP_DiskStringSet::getValue(XAP_String_Id id) const
{
	// dispatch to XAP code if not in our range
	
	if (id < AP_STRING_ID__FIRST__)
		return XAP_DiskStringSet::getValue(id);

	// if it is in our range, look it up in our table
	
	UT_uint32 kLimit = m_vecStringsAP.getItemCount();

	if (id-AP_STRING_ID__FIRST__ < kLimit)
	{
		const XML_Char * szValue = (const XML_Char *) m_vecStringsAP.getNthItem(id-AP_STRING_ID__FIRST__);
		if (szValue)
			return szValue;
	}

	// if no entry in our table for this string, fallback to the builtin value (if provided).

	if (m_pFallbackStringSet)
		return m_pFallbackStringSet->getValue(id);
	
	return NULL;
}

//////////////////////////////////////////////////////////////////
// build a static table to map id by names into numbers
//////////////////////////////////////////////////////////////////

#define dcl(id,s)					{ #id, AP_STRING_ID_##id },

static struct { const XML_Char * szName; XAP_String_Id id; } s_map[] =
{
#include "ap_String_Id.h"
};
			
#undef dcl

//////////////////////////////////////////////////////////////////

bool AP_DiskStringSet::setValue(const XML_Char * szId, const XML_Char * szString)
{
	if (!szId || !*szId || !szString || !*szString)
		return true;
	
	UT_uint32 kLimit = NrElements(s_map);
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
		if (UT_XML_stricmp(s_map[k].szName,szId) == 0)
			return setValue(s_map[k].id,szString);

	// the name (szId) is not in our table, see if the base class knows about it.

	return XAP_DiskStringSet::setValue(szId,szString);
}

bool AP_DiskStringSet::loadStringsFromDisk(const char * szFilename)
{
	if (!XAP_DiskStringSet::loadStringsFromDisk(szFilename))
		return false;

#ifdef DEBUG	
	{
		// TODO should we promote this test to be production code
		// TODO and maybe raise a message box ??
		UT_uint32 kLimit = NrElements(s_map);
		UT_uint32 k;

		for (k=0; k<kLimit; k++)
		{
			const XML_Char * szValue = AP_DiskStringSet::getValue(s_map[k].id);
			if (!szValue || !*szValue)
				UT_DEBUGMSG(("WARNING: Translation for id [%s] not found.\n",s_map[k].szName));
		}
	}
#endif

	return true;
}
