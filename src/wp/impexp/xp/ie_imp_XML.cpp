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
#ifdef HAVE_LIBXML2
#include <glib.h>
#endif

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_XML.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_bytebuf.h"
#include "ut_hash.h"

#include "ap_Prefs.h"

#include "xap_EncodingManager.h"
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
	static UT_HashTable tokens(30);

	xmlToIdMapping * id = NULL;

	UT_HashTable::HashValType pEntry = tokens.pick ((UT_HashTable::HashKeyType)name);

	if (pEntry)
	{
		return (int)pEntry;
	}
	
	id = (xmlToIdMapping *)bsearch (name, idlist, len, 
									sizeof (xmlToIdMapping), s_str_compare);
	if (id)
    {
		tokens.insert ((UT_HashTable::HashKeyType)name, (UT_HashTable::HashValType)id->m_type);
		return id->m_type;
    }
	return -1;
}

/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

#ifdef HAVE_LIBXML2
#define XML_Char xmlChar // HACK
#endif
static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->incOperationCount();
	pDocReader->_startElement((const char*)name, (const char**)atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->incOperationCount();
	pDocReader->_endElement((const char*)name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->incOperationCount();
	pDocReader->_charData((const char*)s, len);
}
#ifdef HAVE_LIBXML2
#undef XML_Char
#endif

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_XML::_openFile(const char * szFilename) 
{
    m_fp = fopen(szFilename, "r");
    return (m_fp != NULL);
}

UT_uint32 IE_Imp_XML::_readBytes(char * buf, UT_uint32 length) 
{
    return fread(buf, 1, length, m_fp);
}

void IE_Imp_XML::_closeFile(void) 
{
    if (m_fp) {
	fclose(m_fp);
    }
}

UT_Error IE_Imp_XML::importFile(const char * szFilename)
{
#ifdef HAVE_LIBXML2
	m_error = _sax (szFilename);
#else
	XML_Parser parser = NULL;
	int done = 0;
	char buf[4096];

	if (!_openFile(szFilename))
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_error = UT_IE_FILENOTFOUND;
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, (XML_StartElementHandler)startElement, (XML_EndElementHandler)endElement);
	XML_SetCharacterDataHandler(parser, (XML_CharacterDataHandler)charData);
	XML_SetUnknownEncodingHandler(parser,(XML_UnknownEncodingHandler)XAP_EncodingManager::XAP_XML_UnknownEncodingHandler,NULL);

	while (!done)
	{
		size_t len = _readBytes(buf, sizeof(buf));
		done = (len < sizeof(buf));

#if 1
        // TODO - remove this then not needed anymore. In ver 0.7.7 and erlier, AbiWord export inserted 
        // chars below 0x20. Most of these are invalid XML and can't be imported.
        // See bug #762.
        for( UT_uint32 n1 = 0; n1 < len; n1++ )
	        if( buf[n1] >= 0x00 && buf[n1] < 0x20 && buf[n1] != 0x09 && buf[n1] != 0x0a && buf[n1] != 0x0d )
		        buf[n1] = 0x0d;
#endif

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						 XML_ErrorString(XML_GetErrorCode(parser)),
						 XML_GetCurrentLineNumber(parser)));
			m_error = UT_IE_BOGUSDOCUMENT;
			goto Cleanup;
		}

		if (m_error)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
	} 
	
	m_error = UT_OK;

Cleanup:
	if (parser)
		XML_ParserFree(parser);
	_closeFile();
#endif /* HAVE_LIBXML2 */
	if(m_error ==  UT_IE_BOGUSDOCUMENT)
	  {
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	  }
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
	: IE_Imp(pDocument), m_parseState(_PS_Init), m_bLoadIgnoredWords(false),
	  m_error(UT_OK), m_lenCharDataSeen(0), m_lenCharDataExpected(0), 
	  m_iOperationCount(0), m_bSeenCR(false), 
	  m_bWhiteSignificant(whiteSignificant), m_bWasSpace(false),
	  m_currentDataItemName(NULL), m_currentDataItemMimeType(NULL)
{
	XAP_App *pApp = m_pDocument->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs *pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	
	pPrefs->getPrefsValueBool((XML_Char *)AP_PREF_KEY_SpellCheckIgnoredWordsLoad, 
							  &m_bLoadIgnoredWords);
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XML::_charData(const XML_Char *s, int len)
{
	// TODO XML_Char is defined in the xml parser
	// TODO as a 'char' not as a 'unsigned char'.
	// TODO does this cause any problems ??
	// TODO Needs to handle preformatted text
	
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
	{
		UT_ASSERT(sizeof(XML_Char) == sizeof(UT_Byte));
		UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));
		
		// parse UTF-8 text and convert to Unicode.
		// also take care of some white-space issues:
		//    [] convert CRLF to SP.
		//    [] convert CR to SP.
		//    [] convert LF to SP.
		// ignored words processing doesn't care about the 
		// white-space stuff, but it does no harm
		
		UT_Byte * ss = (UT_Byte *)s;
		UT_UCSChar _buf[1024], *buf = _buf;
		UT_Byte currentChar;
		int bufLen = 0;
		
		for (int k=0; k<len; k++)
		{
			if (bufLen == NrElements(_buf))		// pump it out in chunks
			{
				switch (m_parseState)
				{
				case _PS_Block:
					X_CheckError(m_pDocument->appendSpan(buf,bufLen));
					break;
				case _PS_IgnoredWordsItem:
					if (m_bLoadIgnoredWords)
					{ 
						X_CheckError(m_pDocument->appendIgnore(buf,bufLen));
					}
					break;
				default:
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				bufLen = 0;
			}
			
			currentChar = ss[k];
			
			if ((ss[k] < 0x80) && (m_lenCharDataSeen > 0))
			{
				// is it us-ascii and we are in a UTF-8
				// multi-byte sequence.  puke.
				X_CheckError(0);
			}
			
			// TODO Needs to handle preformatted text
			if (currentChar == UCS_CR)
			{
				if (!m_bWhiteSignificant)
					buf[bufLen++] = UCS_SPACE;		// substitute a SPACE
				else
					buf[bufLen++] = UCS_LF;
				m_bSeenCR = true;
				continue;
			}
			
			// only honor one space
			// if !m_bWhiteSignificant (XHTML, WML)
			// else just blissfully ignore everything
			// (ABW)
			if (!m_bWhiteSignificant)
			{
				// TODO Needs to handle preformatted text
				if(UT_UCS_isspace(currentChar))
				{
					if(!m_bWasSpace)
					{
						buf[bufLen++] = UCS_SPACE;
						m_bWasSpace = true;
					}
					continue;
				}
				else
				{
					m_bWasSpace = false;
				}
			}
			
			// TODO Needs to handle preformatted text
			if (currentChar == UCS_LF)	// LF
			{
				if (!m_bSeenCR)		// if not immediately after a CR,
					if (!m_bWhiteSignificant)
						buf[bufLen++] = UCS_SPACE;	// substitute a SPACE.  otherwise, eat.
					else
						buf[bufLen++] = UCS_LF;
				m_bSeenCR = false;
				continue;
			}
			
			m_bSeenCR = false;
			
			if (currentChar < 0x80)			// plain us-ascii part of latin-1
			{
				buf[bufLen++] = ss[k];		// copy as is.
			}
			else if ((currentChar & 0xf0) == 0xf0)	// lead byte in 4-byte surrogate pair
			{
				// surrogate pairs are defined in section 3.7 of the
				// unicode standard version 2.0 as an extension
				// mechanism for rare characters in future extensions
				// of the unicode standard.
				UT_ASSERT(m_lenCharDataSeen == 0);
				UT_ASSERT(UT_NOT_IMPLEMENTED);
			}
			else if ((currentChar & 0xe0) == 0xe0)  // lead byte in 3-byte sequence
			{
				UT_ASSERT(m_lenCharDataSeen == 0);
				m_lenCharDataExpected = 3;
				m_charDataSeen[m_lenCharDataSeen++] = currentChar;
			}
			else if ((currentChar & 0xc0) == 0xc0)	// lead byte in 2-byte sequence
			{
				UT_ASSERT(m_lenCharDataSeen == 0);
				m_lenCharDataExpected = 2;
				m_charDataSeen[m_lenCharDataSeen++] = currentChar;
			}
			else if ((currentChar & 0x80) == 0x80)		// trailing byte in multi-byte sequence
			{
				UT_ASSERT(m_lenCharDataSeen > 0);
				m_charDataSeen[m_lenCharDataSeen++] = currentChar;
				if (m_lenCharDataSeen == m_lenCharDataExpected)
				{
					buf[bufLen++] = UT_decodeUTF8char(m_charDataSeen,m_lenCharDataSeen);
					m_lenCharDataSeen = 0;
				}
			}
		}
		
		// flush out the buffer
		
		if (bufLen > 0)
		{
			switch (m_parseState)
			{
			case _PS_Block:
				X_CheckError(m_pDocument->appendSpan(buf,bufLen));
				break;
			case _PS_IgnoredWordsItem:
				if (m_bLoadIgnoredWords) 
				{
					X_CheckError(m_pDocument->appendIgnore(buf,bufLen));
				}
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		}
		return;
	}
	
	case _PS_DataItem:
	{
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

void IE_Imp_XML::pasteFromBuffer(PD_DocumentRange * pDocRange,
				       unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

#ifdef HAVE_LIBXML2
#include <libxml/parserInternals.h>

static xmlEntityPtr _getEntity(void *user_data, const CHAR *name) {
      return xmlGetPredefinedEntity(name);
}

UT_Error IE_Imp_XML::_sax(const char *path)
{
	UT_Error ret = UT_OK;
	xmlSAXHandler hdl;
	hdl.internalSubset = NULL;
	hdl.isStandalone = NULL;
	hdl.hasInternalSubset = NULL;
	hdl.hasExternalSubset = NULL;
	hdl.resolveEntity = NULL;
	hdl.getEntity = _getEntity;
	hdl.entityDecl = NULL;
	hdl.notationDecl = NULL;
	hdl.attributeDecl = NULL;
	hdl.elementDecl = NULL;
	hdl.unparsedEntityDecl = NULL;
	hdl.setDocumentLocator = NULL;
	hdl.startDocument = NULL;
	hdl.endDocument = NULL;
	hdl.startElement = startElement;
	hdl.endElement = endElement;
	hdl.reference = NULL;
	hdl.characters = charData;
	hdl.ignorableWhitespace = NULL;
	hdl.processingInstruction = NULL;
	hdl.comment = NULL;
	hdl.warning = NULL;
	hdl.error = NULL;
	hdl.fatalError = NULL;

	xmlParserCtxtPtr ctxt;

	ctxt = xmlCreateFileParserCtxt(path);
	if (ctxt == NULL)
	{
		UT_DEBUGMSG(("Could not open and parse file %s\n",
					 path));
		// by this point we haven't allocated anything so we can just return right here
		return UT_IE_FILENOTFOUND;
	}
	ctxt->sax = &hdl;
	ctxt->userData = (void *) this;

	xmlParseDocument(ctxt);


	if (!ctxt->wellFormed)
		ret = UT_IE_IMPORTERROR;
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
	return ret;
}
#endif /* HAVE_LIBXML2 */
