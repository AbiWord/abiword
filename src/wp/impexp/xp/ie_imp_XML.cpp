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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "pd_DocumentRDF.h"

#include "ap_Prefs.h"

#include "ie_imp_XML.h"
#include "ie_types.h"

#define DEBUG_RDF_IO  1

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

		return strcmp (name, id->m_name);
	}
}



int IE_Imp_XML::_mapNameToToken (const char * name,
								 struct xmlToIdMapping * idlist, int len)
{
	xmlToIdMapping * id = NULL;

	token_map_t::iterator i = m_tokens.find(name);

	if (i != m_tokens.end()) {
		return static_cast<int>((*i).second);
	}

	id = static_cast<xmlToIdMapping *>(bsearch (name, idlist, len,
									   sizeof (xmlToIdMapping), s_str_compare));
	if (id) {
		m_tokens.insert(token_map_t::value_type(name, static_cast<UT_sint32>(id->m_type)));
		return id->m_type;
    }
	return -1;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_XML::_loadFile(GsfInput * input)
{
	m_szFileName = gsf_input_name (input);

	UT_XML default_xml;
	UT_XML * parser = &default_xml;
	if (m_pParser) parser = m_pParser;

	parser->setListener (this);
	if (m_pReader) parser->setReader (m_pReader);

	// hack!!!
	size_t num_bytes = gsf_input_size(input);
	char * bytes = (char *)gsf_input_read(input, num_bytes, NULL);

	UT_Error err = parser->parse (bytes, num_bytes);
	
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

UT_Error IE_Imp_XML::importFile(const char * data, UT_uint32 length)
{
	m_szFileName = 0;

	UT_XML default_xml;
	UT_XML * parser = &default_xml;
	if (m_pParser) parser = m_pParser;

	parser->setListener (this);
	if (m_pReader) parser->setReader (m_pReader);

	UT_Error err = parser->parse (data, length);

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
	return importFile((const char *)data->getPointer(0), data->getLength());
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
}

IE_Imp_XML::IE_Imp_XML(PD_Document * pDocument, bool whiteSignificant)
	: IE_Imp(pDocument), m_pReader(NULL), m_pParser(NULL), m_error(UT_OK),
          m_parseState(_PS_Init),
	  m_lenCharDataSeen(0), m_lenCharDataExpected(0),
	  m_iOperationCount(0), m_bSeenCR(false),
	  m_bWhiteSignificant(whiteSignificant), m_bWasSpace(false),
	  m_currentDataItem(new UT_ByteBuf),
	  m_currentRevisionId(0), m_currentRevisionTime(0), m_currentRevisionVersion(0), m_tokens()
{
	_data_NewBlock ();
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XML::startElement (const gchar * /*name*/, const gchar ** /*atts*/)
{
	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	m_error = UT_IE_UNSUPTYPE;
	UT_DEBUGMSG(("you must override virtual method IE_Imp_XML::startElement\n"));
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

void IE_Imp_XML::endElement (const gchar * /*name*/)
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

void IE_Imp_XML::charData(const gchar *s, int len)
{
	// TODO gchar is defined in the xml parser
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
		case _PS_RDFTriple:
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
							std::string data(s,len);
							getDoc()->setMetaDataProp(m_currentMetaDataName, data);
							return;
						}
					case _PS_RDFTriple:
                    {
                        std::string data(s,len);

                        if(!m_rdfMutation)
                        {
                            UT_DEBUGMSG(("ie_imp_XML should be parsing triple, but we are not in right parse state!\n"));
                            return;
                        }

#ifdef DEBUG_RDF_IO                        
                        UT_DEBUGMSG(("xml::import() adding s:%s p:%s o:%s otv:%d ots:%s\n",
                                     m_rdfSubject.c_str(),
                                     m_rdfPredicate.c_str(),
                                     data.c_str(),
                                     m_rdfObjectType,
                                     m_rdfXSDType.c_str()
                            ));
                        {
                            static int addCount = 0;
                            addCount++;
                            UT_DEBUGMSG(("xml::import() addCount:%d\n", addCount ));
                        }
#endif
                        
                        m_rdfMutation->add(
                            m_rdfSubject,
                            m_rdfPredicate,
                            PD_Object( data,
                                       m_rdfObjectType,
                                       m_rdfXSDType ));

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
						
						UT_return_if_fail ((sizeof(gchar) == sizeof(UT_Byte)));
						
						UT_uint32 actualLen = m_currentDataItem->getLength();
						m_currentDataItem->ins(actualLen, len); // allocate all the possibly needed memory at once
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
									m_currentDataItem->overwrite(actualLen, ss, k);
									actualLen += k;
								}
								
								ss += k;
							}
						
							m_currentDataItem->truncate(actualLen); // chop off the mem we don't need after all
						return;
					}
				else
						m_currentDataItem->append(reinterpret_cast<const UT_Byte*>(s), len);
#undef MyIsWhite
#endif /* ENABLE_RESOURCE_MANAGER */
			}
			
		default:
			return;
		}
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 IE_Imp_XML::_getInlineDepth(void) const
{
	return m_nstackFmtStartIndex.getDepth();
}

bool IE_Imp_XML::_pushInlineFmt(const PP_PropertyVector & atts)
{
	UT_uint32 start = m_vecInlineFmt.size() + 1;

	m_vecInlineFmt.insert(m_vecInlineFmt.end(),
						  atts.begin(), atts.end());
	if (!m_nstackFmtStartIndex.push(start))
		return false;
	return true;
}

void IE_Imp_XML::_popInlineFmt(void)
{
	UT_sint32 start;
	if (!m_nstackFmtStartIndex.pop(&start))
		return;

	m_vecInlineFmt.erase(m_vecInlineFmt.begin() + (start - 1),
						 m_vecInlineFmt.end());
}

const gchar * IE_Imp_XML::_getXMLPropValue(const gchar *name,
											  const gchar ** atts)
{
	return UT_getAttribute(name, atts);
}
