/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* Abiword
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_hash.h"
#include "ut_string_class.h"

#include "xap_EncodingManager.h"
#ifdef ENABLE_RESOURCE_MANAGER
#include "xap_ResourceManager.h"
#endif

#include "pd_Document.h"

#include "ap_Prefs.h"

#include "ie_imp_XML.h"
#include "ie_types.h"

/*****************************************************************/
/*****************************************************************/

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

extern "C" { // for MRC compiler (Mac)
	static int s_str_compare (const void * a, const void * b)
	{
		const char * name = (const char *)a;
		const xmlToIdMapping * id = (const xmlToIdMapping *)b;

		return UT_strcmp (name, id->m_name);
	}
}

int IE_Imp_XML::_mapNameToToken (const char * name,
								 struct xmlToIdMapping * idlist, int len)
{
	static UT_StringPtrMap tokens(30);

	xmlToIdMapping * id = NULL;

	const void * pEntry = tokens.pick (name);

	if (pEntry)
	{
		return (int)pEntry;
	}

	id = (xmlToIdMapping *)bsearch (name, idlist, len,
									sizeof (xmlToIdMapping), s_str_compare);
	if (id)
    {
		tokens.insert (name, (void *)id->m_type);
		return id->m_type;
    }
	return -1;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_XML::importFile(const char * szFilename)
{
	m_szFileName = szFilename;

	UT_XML parser;
	parser.setListener (this);
	if (m_pReader) parser.setReader (m_pReader);
	UT_Error err =parser.parse (szFilename);
	if ((err != UT_OK) && (err != UT_IE_SKIPINVALID))
	{
		m_error = UT_IE_BOGUSDOCUMENT;
	}
	if (m_error)
	{
		UT_DEBUGMSG(("Problem reading document\n"));
		if(m_error != UT_IE_SKIPINVALID)
		{
			goto Cleanup;
		}
	}
	m_error = UT_OK;
Cleanup:
	m_szFileName = 0;

	return m_error;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_XML::~IE_Imp_XML()
{
	FREEP(m_currentDataItemName);
	FREEP(m_currentDataItemMimeType);
}

IE_Imp_XML::IE_Imp_XML(PD_Document * pDocument, bool whiteSignificant)
	: IE_Imp(pDocument), m_pReader(NULL), m_error(UT_OK),
          m_parseState(_PS_Init), m_bLoadIgnoredWords(false),
	  m_lenCharDataSeen(0), m_lenCharDataExpected(0),
	  m_iOperationCount(0), m_bSeenCR(false),
	  m_bWhiteSignificant(whiteSignificant), m_bWasSpace(false),
	  m_currentDataItemName(NULL), m_currentDataItemMimeType(NULL)
{
	XAP_App *pApp = getDoc()->getApp();
	UT_return_if_fail(pApp);
	XAP_Prefs *pPrefs = pApp->getPrefs();
	UT_return_if_fail(pPrefs);

	pPrefs->getPrefsValueBool((XML_Char *)AP_PREF_KEY_SpellCheckIgnoredWordsLoad,
							  &m_bLoadIgnoredWords);
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XML::startElement (const XML_Char * /*name*/, const XML_Char ** /*atts*/)
{
	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	m_error = UT_IE_UNSUPTYPE;
	UT_DEBUGMSG(("you must override virtual method IE_Imp_XML::startElement\n"));
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void IE_Imp_XML::endElement (const XML_Char * /*name*/)
{
	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	m_error = UT_IE_UNSUPTYPE;
	UT_DEBUGMSG(("you must override virtual method IE_Imp_XML::endElement\n"));
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void IE_Imp_XML::charData(const XML_Char *s, int len)
{
	// TODO XML_Char is defined in the xml parser
	// TODO as a 'char' not as a 'unsigned char'.
	// TODO does this cause any problems ??

	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed

	switch (m_parseState)
	{
	default:
	{
		xxx_UT_DEBUGMSG(("charData DISCARDED [length %d]\n",len));
		return;
	}

	case _PS_Field:
	{
		// discard contents of the field - force recalculation
		// this gives us a higher chance of correcting fields
		// with the wrong values
		return;
	}

	case _PS_Block:
	case _PS_IgnoredWordsItem:
	case _PS_Meta:
	case _PS_Revision:
	{
		UT_UCS4String buf(s,static_cast<size_t>(len),!m_bWhiteSignificant);

		// flush out the buffer

		if (buf.size() == 0 )
		  return;

		switch (m_parseState)
		  {
		  case _PS_Block:
		    X_CheckError(getDoc()->appendSpan(buf.ucs4_str(), buf.size()));
		    break;
		  case _PS_IgnoredWordsItem:
		    if (m_bLoadIgnoredWords)
		      {
			X_CheckError(getDoc()->appendIgnore(buf.ucs4_str(), buf.size()));
		      }
		    break;
		  case _PS_Meta:
		    {
		      // ugly hack
		      UT_String data(UT_UTF8String(buf).utf8_str());
		      getDoc()->setMetaDataProp(m_currentMetaDataName, data);
		      UT_DEBUGMSG(("Storing metadata: %s=%s\n", m_currentMetaDataName.c_str(), data.c_str()));
		      break;
		    }
		  case _PS_Revision:
			{
				X_CheckError(getDoc()->addRevision(m_currentRevisionId, buf.ucs4_str(), buf.size()));
			}
			break;
			

		  default:
		    UT_ASSERT_NOT_REACHED();
		    break;
		  }
		return;
	}

	case _PS_DataItem:
	{
#ifdef ENABLE_RESOURCE_MANAGER
		XAP_ResourceManager & RM = getDoc()->resourceManager ();
		XAP_Resource * resource = RM.current ();
		if (resource == 0) break;
		if (!resource->bInternal) break;
		XAP_InternalResource * ri = dynamic_cast<XAP_InternalResource *>(resource);

		if (m_currentDataItemEncoded) // base64-encoded data
			{
				ri->buffer (s, len, true);
			}
		else // old file-format keeping MathML & SVG in CDATA section :-(
			{
				/* since SVG import was only ever a DEBUG option, and is currently disabled (why?),
				 * since MathML was never supported except in principle, and since this CDATA stuff
				 * (unencoded) is pretty unsafe anyway, I'm going to postpone import support
				 * indefinitely...                                              - fjf Aug. 19th '02
				 */
			}
#else /* ENABLE_RESOURCE_MANAGER */

#define MyIsWhite(c)			(((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r'))

		if (m_currentDataItemEncoded)
		{

			// DataItem data consists of Base64 encoded data with
			// white space added for readability.  strip out any
			// white space and put the rest in the ByteBuf.

			UT_ASSERT((sizeof(XML_Char) == sizeof(UT_Byte)));

			const UT_Byte * ss = (UT_Byte *)s;
			const UT_Byte * ssEnd = ss + len;
			while (ss < ssEnd)
			{
				while ((ss < ssEnd) && MyIsWhite(*ss))
					ss++;
				UT_uint32 k=0;
				while ((ss+k < ssEnd) && ( ! MyIsWhite(ss[k])))
					k++;
				if (k > 0)
					m_currentDataItem.ins(m_currentDataItem.getLength(),ss,k);

				ss += k;
			}

			return;
		}
		else
		{
			m_currentDataItem.append((UT_Byte*)s, len);
		}
#undef MyIsWhite
#endif /* ENABLE_RESOURCE_MANAGER */
	}
	}
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 IE_Imp_XML::_getInlineDepth(void) const
{
	return m_stackFmtStartIndex.getDepth();
}

bool IE_Imp_XML::_pushInlineFmt(const XML_Char ** atts)
{
	UT_uint32 start = m_vecInlineFmt.getItemCount()+1;
	UT_uint32 k;

	for (k=0; (atts[k]); k++)
	{
		XML_Char * p;
		if (!UT_XML_cloneString(p,atts[k]))
			return false;
		if (m_vecInlineFmt.addItem(p)!=0)
			return false;
	}
	if (!m_stackFmtStartIndex.push((void*)start))
		return false;
	return true;
}

void IE_Imp_XML::_popInlineFmt(void)
{
	UT_uint32 start;
	if (!m_stackFmtStartIndex.pop((void **)&start))
		return;
	UT_uint32 k;
	UT_uint32 end = m_vecInlineFmt.getItemCount();
	for (k=end; k>=start; k--)
	{
		const XML_Char * p = (const XML_Char *)m_vecInlineFmt.getNthItem(k-1);
		m_vecInlineFmt.deleteNthItem(k-1);
		if (p)
			free((void *)p);
	}
}

const XML_Char * IE_Imp_XML::_getXMLPropValue(const XML_Char *name,
					      const XML_Char ** atts)
{
  // find the 'name="value"' pair and return the "value".
  // ignore everything else

  // quick out
  if(!name || !atts)
    return NULL;

  for (const XML_Char ** a = atts; (*a); a++)
    if(a[0] && (UT_XML_strcmp(a[0],name) == 0))
      return a[1];

  return NULL;
}







