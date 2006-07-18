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
#include "ut_misc.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_string_class.h"

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
UT_ASSERT_HARMLESS(0); \
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

extern "C" { // for MRC compiler (Mac)
	static int s_str_compare (const void * a, const void * b)
	{
		const char * name = static_cast<const char *>(a);
		const xmlToIdMapping * id = static_cast<const xmlToIdMapping *>(b);

		return UT_strcmp (name, id->m_name);
	}
}



int IE_Imp_XML::_mapNameToToken (const char * name,
								 struct xmlToIdMapping * idlist, int len)
{
	xmlToIdMapping * id = NULL;

	UT_sint32 iEntry = m_tokens[name];

	if (iEntry >= 0) return static_cast<int>(iEntry);

	id = static_cast<xmlToIdMapping *>(bsearch (name, idlist, len,
									   sizeof (xmlToIdMapping), s_str_compare));
	if (id)
    {
		m_tokens.ins (name, static_cast<UT_sint32>(id->m_type));
		return id->m_type;
    }
	return -1;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_XML::importFile(const char * szFilename)
{
	m_szFileName = szFilename;

	UT_XML default_xml;
	UT_XML * parser = &default_xml;
	if (m_pParser) parser = m_pParser;

	parser->setListener (this);
	if (m_pReader) parser->setReader (m_pReader);

	UT_Error err = parser->parse (szFilename);

	if ((err != UT_OK) && (err != UT_IE_SKIPINVALID))
		m_error = UT_IE_BOGUSDOCUMENT;

	if (m_error != UT_OK)
	{
		UT_DEBUGMSG(("Problem reading document\n"));
		if(m_error != UT_IE_SKIPINVALID)
			m_szFileName = 0;
	}

	return m_error;
}

UT_Error IE_Imp_XML::importFile(const UT_ByteBuf * data)
{
	m_szFileName = 0;

	UT_XML default_xml;
	UT_XML * parser = &default_xml;
	if (m_pParser) parser = m_pParser;

	parser->setListener (this);
	if (m_pReader) parser->setReader (m_pReader);

	UT_Error err = parser->parse (data);

	if ((err != UT_OK) && (err != UT_IE_SKIPINVALID))
		m_error = UT_IE_BOGUSDOCUMENT;

	if (m_error != UT_OK)
	{
		UT_DEBUGMSG(("Problem reading document\n"));
		if(m_error != UT_IE_SKIPINVALID)
			m_szFileName = 0;
	}

	return m_error;
}

bool IE_Imp_XML::pasteFromBuffer(PD_DocumentRange * pDocRange, const unsigned char * pData, 
								 UT_uint32 lenData, const char * /*szEncoding*/)
{
	UT_DEBUGMSG(("IE_Imp_XML::pasteFromBuffer\n"));
	UT_return_val_if_fail(pDocRange && pDocRange->m_pDoc,false);
	setClipboard(pDocRange->m_pos1);

	UT_XML default_xml;
	UT_XML * parser = &default_xml;
	if (m_pParser) parser = m_pParser;

	parser->setListener (this);
	if (m_pReader) parser->setReader (m_pReader);

	UT_Error err = parser->parse ((const char*)pData, lenData);

	if ((err != UT_OK) && (err != UT_IE_SKIPINVALID))
		m_error = UT_IE_BOGUSDOCUMENT;

	if (m_error != UT_OK)
	{
		UT_DEBUGMSG(("Problem reading document\n"));
		return false;
	}
	return true;
}


/*****************************************************************/
/*****************************************************************/

IE_Imp_XML::~IE_Imp_XML()
{
	FREEP(m_currentDataItemName);
	FREEP(m_currentDataItemMimeType);
}

IE_Imp_XML::IE_Imp_XML(PD_Document * pDocument, bool whiteSignificant)
	: IE_Imp(pDocument), m_pReader(NULL), m_pParser(NULL), m_error(UT_OK),
          m_parseState(_PS_Init),
	  m_lenCharDataSeen(0), m_lenCharDataExpected(0),
	  m_iOperationCount(0), m_bSeenCR(false),
	  m_bWhiteSignificant(whiteSignificant), m_bWasSpace(false),
	  m_currentDataItemName(NULL), m_currentDataItemMimeType(NULL), m_tokens(-1,30)
{
	XAP_App *pApp = getDoc()->getApp();
	UT_return_if_fail(pApp);
	XAP_Prefs *pPrefs = pApp->getPrefs();
	UT_return_if_fail(pPrefs);

	_data_NewBlock ();
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XML::startElement (const XML_Char * /*name*/, const XML_Char ** /*atts*/)
{
	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	m_error = UT_IE_UNSUPTYPE;
	UT_DEBUGMSG(("you must override virtual method IE_Imp_XML::startElement\n"));
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

void IE_Imp_XML::endElement (const XML_Char * /*name*/)
{
	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	m_error = UT_IE_UNSUPTYPE;
	UT_DEBUGMSG(("you must override virtual method IE_Imp_XML::endElement\n"));
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

void IE_Imp_XML::_data_NewBlock ()
{
	m_iCharCount = 0;
	m_bStripLeading = true; // only makes a difference if !m_bWhiteSignificant
}

void IE_Imp_XML::charData(const XML_Char *s, int len)
{
	// TODO XML_Char is defined in the xml parser
	// TODO as a 'char' not as a 'unsigned char'.
	// TODO does this cause any problems ??

	xxx_UT_DEBUGMSG(("In IE_Imp_XML::charDat, len = %d parsestate %d \n",len,m_parseState));
	if(!s || !len)
		return;

	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed

	switch (m_parseState)
		{
		case _PS_Block:
		case _PS_IgnoredWordsItem:
		case _PS_Meta:
		case _PS_Revision:
			{
				UT_UCS4String buf(s,static_cast<size_t>(len),!m_bWhiteSignificant);
				xxx_UT_DEBUGMSG(("In IE_Imp_XML::charData buf.size %d \n",buf.size()));
				// flush out the buffer
				if (buf.size() == 0)
					return;
				
				switch (m_parseState)
					{
					case _PS_Block:
						if (!m_bWhiteSignificant && m_bStripLeading && (buf[0] == UCS_SPACE))
						{
							if (buf.size () > 1)
							{
								X_CheckError(appendSpan (buf.ucs4_str()+1, buf.size()-1));
								m_iCharCount += buf.size () - 1;
							}
						}
						else
						{
							X_CheckError(appendSpan (buf.ucs4_str(), buf.size()));
							m_iCharCount += buf.size ();
						}
						m_bStripLeading = (buf[buf.size()-1] == UCS_SPACE);
						return;
					case _PS_IgnoredWordsItem:
						return;
					case _PS_Meta:
						{
							UT_UTF8String data(s,len);
							getDoc()->setMetaDataProp(m_currentMetaDataName, data);
							return;
						}
					case _PS_Revision:

						// 0 is not a valid revision Id
						if(m_currentRevisionId)
						{
							UT_DEBUGMSG(("Doing revision \n"));
							X_CheckError(getDoc()->addRevision(m_currentRevisionId,
															   buf.ucs4_str(),
															   buf.size(),
															   m_currentRevisionTime,
															   m_currentRevisionVersion));

							// we need to reset the revision Id in order
							// to be able to handle the case when there is
							// no character data present in our
							// endofelement handler
							m_currentRevisionId = 0;
						}
						
						return;			
						
					default:
						UT_ASSERT_NOT_REACHED();
						return;
					}		
			}
			
		case _PS_Field:
			{
				// discard contents of the field - force recalculation
				// this gives us a higher chance of correcting fields
				// with the wrong values
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
						ri->buffer (s, len, true);
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
						
						UT_return_if_fail ((sizeof(XML_Char) == sizeof(UT_Byte)));
						
						UT_uint32 actualLen = m_currentDataItem.getLength();
						m_currentDataItem.ins(actualLen, len); // allocate all the possibly needed memory at once
						const UT_Byte * ss = reinterpret_cast<const UT_Byte *>(s);
						const UT_Byte * ssEnd = ss + len;
						while (ss < ssEnd)
							{
								while ((ss < ssEnd) && MyIsWhite(*ss))
									ss++;
								UT_uint32 k=0;
								while ((ss+k < ssEnd) && ( ! MyIsWhite(ss[k])))
									k++;
								if (k > 0)
								{
									m_currentDataItem.overwrite(actualLen, const_cast<UT_Byte *>(ss), k);
									actualLen += k;
								}
								
								ss += k;
							}
						
							m_currentDataItem.truncate(actualLen); // chop off the mem we don't need after all
						return;
					}
				else
						m_currentDataItem.append(reinterpret_cast<const UT_Byte*>(s), len);
#undef MyIsWhite
#endif /* ENABLE_RESOURCE_MANAGER */
			}
			
		default:
			UT_DEBUGMSG(("ie_imp_XML::charData Default just return \n"));
			return;
		}
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 IE_Imp_XML::_getInlineDepth(void) const
{
	return m_nstackFmtStartIndex.getDepth();
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
	if (!m_nstackFmtStartIndex.push(start))
		return false;
	return true;
}

void IE_Imp_XML::_popInlineFmt(void)
{
	UT_sint32 start;
	if (!m_nstackFmtStartIndex.pop(&start))
		return;
	UT_uint32 k;
	UT_uint32 end = m_vecInlineFmt.getItemCount();
	for (k=end; k>=start; k--)
	{
		const XML_Char * p = static_cast<XML_Char *>(m_vecInlineFmt.getNthItem(k-1));
		m_vecInlineFmt.deleteNthItem(k-1);
		if (p)
			free(const_cast<void *>(static_cast<const void *>(p)));
	}
}

const XML_Char * IE_Imp_XML::_getXMLPropValue(const XML_Char *name,
											  const XML_Char ** atts)
{
	return UT_getAttribute(name, atts);
}
