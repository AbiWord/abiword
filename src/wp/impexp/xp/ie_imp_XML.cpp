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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_GNOME_XML2
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

/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

#ifndef HAVE_GNOME_XML2
static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->_startElement(name, atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->_endElement(name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	IE_Imp_XML* pDocReader = (IE_Imp_XML*) userData;
	pDocReader->_charData(s, len);
}
#endif /* HAVE_GNOME_XML2 */

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_XML::_openFile(const char * szFilename) 
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
#ifdef HAVE_GNOME_XML2
	xmlDocPtr dok = xmlParseFile(szFilename);
	if (dok == NULL)
	  {
	    UT_DEBUGMSG(("Could not open and parse file %s\n",
			 szFilename));
	    m_error = UT_IE_FILENOTFOUND;
	  }
	else
	  {
	    xmlNodePtr node = xmlDocGetRootElement(dok);
	    _scannode(dok,node,0);
	    xmlFreeDoc(dok);
	    m_error = UT_OK;
	  }
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
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

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
#endif /* HAVE_GNOME_XML2 */
	return m_error;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_XML::~IE_Imp_XML()
{
	FREEP(m_currentDataItemName);
	FREEP(m_currentDataItemMimeType);
}

IE_Imp_XML::IE_Imp_XML(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_error = UT_OK;
	m_parseState = _PS_Init;
	m_lenCharDataSeen = 0;
	m_lenCharDataExpected = 0;
	m_bSeenCR = UT_FALSE;
	m_bWhiteSignificant = UT_TRUE;
	m_bWasSpace = UT_FALSE;

	m_currentDataItemName = NULL;
	m_currentDataItemMimeType = NULL;
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XML::_charData(const XML_Char *s, int len)
{
	// TODO XML_Char is defined in the xml parser
	// TODO as a 'char' not as a 'unsigned char'.
	// TODO does this cause any problems ??
	
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed

	switch (m_parseState)
	{
	default:
		{
			xxx_UT_DEBUGMSG(("charData DISCARDED [length %d]\n",len));
			return;
		}
		
	case _PS_Block:
		{
			UT_ASSERT(sizeof(XML_Char) == sizeof(UT_Byte));
			UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));

			// parse UTF-8 text and convert to Unicode.
			// also take care of some white-space issues:
			//    [] convert CRLF to SP.
			//    [] convert CR to SP.
			//    [] convert LF to SP.

			UT_Byte * ss = (UT_Byte *)s;
			UT_Byte currentChar;
			UT_UCSChar buf[1024];
			int bufLen = 0;

			for (int k=0; k<len; k++)
			{
				if (bufLen == NrElements(buf))		// pump it out in chunks
				{
					X_CheckError(m_pDocument->appendSpan(buf,bufLen));
					bufLen = 0;
				}

				currentChar = ss[k];

				if ((ss[k] < 0x80) && (m_lenCharDataSeen > 0))
				{
					// is it us-ascii and we are in a UTF-8
					// multi-byte sequence.  puke.
					X_CheckError(0);
				}

				if (currentChar == UCS_CR)
				{
					buf[bufLen++] = UCS_SPACE;		// substitute a SPACE
					m_bSeenCR = UT_TRUE;
					continue;
				}

				// only honor one space
				// if !m_bWhiteSignificant (XHTML, WML)
				// else just blissfully ignore everything
				// (ABW)
				if (!m_bWhiteSignificant)
				{
				  if(UT_UCS_isspace(currentChar))
				    {
				      if(!m_bWasSpace)
					{
					  buf[bufLen++] = UCS_SPACE;
					  m_bWasSpace = UT_TRUE;
					}
				      continue;
				    }
				  else
				    {
				      m_bWasSpace = UT_FALSE;
				    }
				}

				if (currentChar == UCS_LF)				// LF
				{
					if (!m_bSeenCR)					// if not immediately after a CR,
						buf[bufLen++] = UCS_SPACE;	// substitute a SPACE.  otherwise, eat.
					m_bSeenCR = UT_FALSE;
					continue;
				}
				
				m_bSeenCR = UT_FALSE;

				if (currentChar < 0x80)					// plain us-ascii part of latin-1
				{
					buf[bufLen++] = ss[k];			// copy as is.
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
				else if ((currentChar & 0xe0) == 0xe0)		// lead byte in 3-byte sequence
				{
					UT_ASSERT(m_lenCharDataSeen == 0);
					m_lenCharDataExpected = 3;
					m_charDataSeen[m_lenCharDataSeen++] = currentChar;
				}
				else if ((currentChar & 0xc0) == 0xc0)		// lead byte in 2-byte sequence
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

			// flush out the last piece of a buffer

			if (bufLen > 0)
				X_CheckError(m_pDocument->appendSpan(buf,bufLen));
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

UT_Bool IE_Imp_XML::_pushInlineFmt(const XML_Char ** atts)
{
	UT_uint32 start = m_vecInlineFmt.getItemCount()+1;
	UT_uint32 k;

	for (k=0; (atts[k]); k++)
	{
		XML_Char * p;
		if (!UT_XML_cloneString(p,atts[k]))
			return UT_FALSE;
		if (m_vecInlineFmt.addItem(p)!=0)
			return UT_FALSE;
	}
	if (!m_stackFmtStartIndex.push((void*)start))
		return UT_FALSE;
	return UT_TRUE;
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
    if(a[0] && (UT_XML_stricmp(a[0],name) == 0))
      return a[1];
  
  return NULL;
}

void IE_Imp_XML::pasteFromBuffer(PD_DocumentRange * pDocRange,
				       unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

#ifdef HAVE_GNOME_XML2
void IE_Imp_XML::_scannode(xmlDocPtr dok, xmlNodePtr cur, int c)
{
  while (cur != NULL)
    {
      if (strcmp("text", (char*) cur->name) == 0)
	{
	  xmlChar* s = cur->content; // xmlNodeListGetString(dok, cur, 1);
	  _charData(s, strlen((char*) s));
	}
      else
	{
	  xmlChar *prop = NULL;
	  const xmlChar* props[3] = { NULL, NULL, NULL };
	  if (cur->properties)
	    {
	      props[0] = cur->properties->name;
	      props[1] = cur->properties->children->content;
	    }
	  _startElement(cur->name, props);
	  if (prop) g_free(prop);
	}
      _scannode(dok, cur->children, c + 1);
      if (strcmp("text", (char*) cur->name) != 0)
	_endElement(cur->name);
      cur = cur->next;
    }
}
#endif /* HAVE_GNOME_XML2 */
