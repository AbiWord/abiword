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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_bytebuf.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

IE_Imp_AbiWord_1::~IE_Imp_AbiWord_1()
{
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
  : IE_Imp_XML(pDocument, UT_TRUE)
{
	m_bDocHasLists = UT_FALSE;
}

/* Quick hack for GZipAbiWord */
UT_Error IE_Imp_AbiWord_1::importFile(const char * szFilename)
{
  return IE_Imp_XML::importFile(szFilename);
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_AbiWord_1::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	char *magic ;
	p = szBuf ;
	while( iLinesToRead-- )
	{
		magic = "<abiword " ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_FALSE);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_TRUE);
		magic = "<!-- This file is an AbiWord document." ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_FALSE);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_TRUE);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++ ; p++ ;
			if( iBytesScanned+2 >= iNumbytes ) return(UT_FALSE);
		}
		/*  Seek past the next newline:  */
		if ( *p == '\n' || *p == '\r' )
		{
			iBytesScanned++ ; p++ ;
			if ( *p == '\n' || *p == '\r' )
			{
				iBytesScanned++ ; p++ ;
			}
		}
	}
	return(UT_FALSE);
}

UT_Bool IE_Imp_AbiWord_1::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".abw") == 0);
}

UT_Error IE_Imp_AbiWord_1::StaticConstructor(PD_Document * pDocument,
											 IE_Imp ** ppie)
{
	IE_Imp_AbiWord_1 * p = new IE_Imp_AbiWord_1(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_AbiWord_1::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "AbiWord (.abw)";
	*pszSuffixList = "*.abw";
	*ft = IEFT_AbiWord_1;
	return UT_TRUE;
}

UT_Bool IE_Imp_AbiWord_1::SupportsFileType(IEFileType ft)
{
	return (IEFT_AbiWord_1 == ft);
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0
#define TT_DOCUMENT		1		// a document <awml>
#define TT_SECTION		2		// a section <section>
#define TT_BLOCK		3		// a paragraph <p>
#define TT_INLINE		4		// inline span of text <c>
#define TT_IMAGE		5		// an image object <i>
#define TT_FIELD		6		// a computed field object <f>
#define TT_BREAK		7		// a forced line-break <br>
#define TT_DATASECTION	8		// a data section <data>
#define TT_DATAITEM		9		// a data item <d> within a data section
#define TT_COLBREAK		10		// a forced column-break <cbr>
#define TT_PAGEBREAK	11		// a forced page-break <pbr>
#define TT_STYLESECTION	12		// a style section <styles>
#define TT_STYLE		13		// a style <s> within a style section
#define TT_LISTSECTION		14	// a list section <lists>
#define TT_LIST			15	// a list <l> within a list section

/*
  TODO remove tag synonyms.  We're currently accepted
  synonyms for tags, as follows:

  abiword	awml
  field		f
  image		i

  The renaming of these tags occurred 26 Mar 1999, shortly
  after tarball 0.5.2.  Eventually, this backwards compatibility
  code should be removed.
*/

static struct xmlToIdMapping s_Tokens[] =
{
	{	"abiword",		TT_DOCUMENT		},
	{	"awml",			TT_DOCUMENT		},
	{	"br",			TT_BREAK		},
	{	"c",			TT_INLINE		},
	{	"cbr",			TT_COLBREAK		},
	{	"d",			TT_DATAITEM		},
	{	"data",			TT_DATASECTION		},
	{	"f",			TT_FIELD		},
	{	"field",		TT_FIELD		},
	{	"i",			TT_IMAGE		},
	{	"image",		TT_IMAGE		},
	{	"l",			TT_LIST			},
	{	"lists",		TT_LISTSECTION		},
	{	"p",			TT_BLOCK		},
	{	"pbr",			TT_PAGEBREAK		},
	{	"s",			TT_STYLE		},
	{	"section",		TT_SECTION		},
	{	"styles",		TT_STYLESECTION		}
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

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

#define X_VerifyInsideBlockOrField()  do { if(!(X_TestParseState(_PS_Block) || X_TestParseState(_PS_Field))) \
                                                                          {  m_error = UT_IE_BOGUSDOCUMENT;  \
                                                                             return; } } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_AbiWord_1::_startElement(const XML_Char *name, const XML_Char **atts)
{
	xxx_UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,atts));
		return;

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		UT_DEBUGMSG(("SEVIOR: Appending strux \n"));
		for(UT_sint32 i=0; atts[i] != NULL; i++)
		  { 
		    UT_DEBUGMSG(("Element %d is %s \n",i,atts[i]));
		  }
		return;
		
	case TT_INLINE:
        // ignored for fields
                if (m_parseState == _PS_Field) return;
		X_VerifyParseState(_PS_Block);
		X_CheckError(_pushInlineFmt(atts));
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

		// Images and Fields are not containers.  Therefore we don't
		// push the ParseState (_PS_...).
		// TODO should Images or Fields inherit the (possibly nested)
		// TODO inline span formatting.
		
	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendObject(PTO_Image,atts));
		return;

	case TT_FIELD:
		X_VerifyParseState(_PS_Block);
                m_parseState = _PS_Field;
		X_CheckError(m_pDocument->appendObject(PTO_Field,atts));
		UT_DEBUGMSG(("SEVIOR: Appending field \n"));
		for(UT_sint32 i=0; atts[i] != NULL; i++)
		  { 
		    UT_DEBUGMSG(("Element %d is %s \n",i,atts[i]));
		  }
		return;

		// Forced Line Breaks are not containers.  Therefore we don't
		// push the ParseState (_PS_...).  Breaks are marked with a
		// tag, but are translated into character data (LF).  This may
		// seem a little odd (perhaps an &lf; entity would be better).
		// Anyway, this distinction from ordinary LF's in the document
		// (which get mapped into SPACE) keeps the file sanely editable.

	case TT_BREAK:
	        if(X_TestParseState(_PS_Field))
		  return; // just return

		X_VerifyParseState(_PS_Block);

		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <br/>.  that is, decide
		// TODO if <br/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_COLBREAK:
#if 0	        
	        if(X_TestParseState(_PS_Field))
		        return; // just return
#endif
		X_VerifyParseState(_PS_Block);

		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <cbr/>.  that is, decide
		// TODO if <cbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_VTAB;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_PAGEBREAK:
#if 0
		if(X_TestParseState(_PS_Field)
		         return; //just return
#endif

		X_VerifyParseState(_PS_Block);
		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <pbr/>.  that is, decide
		// TODO if <pbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_FF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_DataSec;
		// We don't need to notify the piece table of the data section,
		// it will get the hint when we begin sending data items.
		return;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_DataItem;
		m_currentDataItem.truncate(0);
		X_CheckError(UT_XML_cloneString(m_currentDataItemName,_getDataItemName(atts)));
		X_CheckError(UT_XML_cloneString(m_currentDataItemMimeType,_getDataItemMimeType(atts)));
		m_currentDataItemEncoded = _getDataItemEncoded(atts);
		return;
		
	case TT_STYLESECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_StyleSec;
		// We don't need to notify the piece table of the style section,
		// it will get the hint when we begin sending styles.
		return;

	case TT_STYLE:
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Style;
		X_CheckError(m_pDocument->appendStyle(atts));
		return;
		
	case TT_LISTSECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_ListSec;
		// As per styles, we don't need to notify the piece table.
		return;
		
	case TT_LIST:
		X_VerifyParseState(_PS_ListSec);
		m_parseState = _PS_List;
		// Urgh! Complex. I think how done.
		X_CheckError(m_pDocument->appendList(atts));
		m_bDocHasLists = UT_TRUE;
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
#if 0
		m_error = UT_IE_BOGUSDOCUMENT;
#endif		
		return;
	}
}

void IE_Imp_AbiWord_1::_endElement(const XML_Char *name)
{
  	xxx_UT_DEBUGMSG(("endElement %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 trim;
	UT_uint32 len;
	const UT_Byte * buffer;

   	UT_uint32 tokenIndex = mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;
		return;

	case TT_BLOCK:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_INLINE:
		UT_ASSERT(m_lenCharDataSeen==0);
                if (m_parseState == _PS_Field) // just return 
			  return;
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_FIELD:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Field);
                m_parseState = _PS_Block;
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyInsideBlockOrField();
		return;

	case TT_COLBREAK:					// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
#if 1
		X_VerifyParseState(_PS_Block);
#else
		X_VerifyInsideBlockOrField();
#endif
		return;

	case TT_PAGEBREAK:					// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
#if 1
		X_VerifyParseState(_PS_Block);
#else
		X_VerifyInsideBlockOrField();
#endif
		return;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_Doc;
		return;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataItem);
		m_parseState = _PS_DataSec;
#define MyIsWhite(c)			(((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r'))
		trim = 0;
		len = m_currentDataItem.getLength();
		buffer = m_currentDataItem.getPointer(0);
		while (trim < len && MyIsWhite(buffer[trim])) trim++;
		if (trim) m_currentDataItem.del(0, trim);
		trim = m_currentDataItem.getLength();
		buffer = m_currentDataItem.getPointer(0);
		while (trim >= 0 && MyIsWhite(buffer[trim])) trim--;
		m_currentDataItem.truncate(trim+1);
#undef MyIsWhite
 		X_CheckError(m_pDocument->createDataItem((char*)m_currentDataItemName,m_currentDataItemEncoded,&m_currentDataItem,m_currentDataItemMimeType,NULL));
		FREEP(m_currentDataItemName);
		// the data item will free the token we passed (mime-type)
		m_currentDataItemMimeType = NULL;
 		return;
		
	case TT_STYLESECTION:
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Doc;
		return;

	case TT_STYLE:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Style);
		m_parseState = _PS_StyleSec;
		return;

	case TT_LISTSECTION:
		X_VerifyParseState(_PS_ListSec);
		if (m_bDocHasLists)
			X_CheckError(m_pDocument->fixListHierarchy());
		m_parseState = _PS_Doc;
		return;

	case TT_LIST:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_List);
		m_parseState = _PS_ListSec;
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
#if 0
		m_error = UT_IE_BOGUSDOCUMENT;
#endif		
		return;
	}
}

/*****************************************************************/
/*****************************************************************/

const XML_Char * IE_Imp_AbiWord_1::_getDataItemName(const XML_Char ** atts)
{
	// find the 'name="value"' pair and return the "value".
	// ignore everything else (which there shouldn't be)

	for (const XML_Char ** a = atts; (*a); a++)
		if (UT_XML_strcmp(a[0],"name") == 0)
			return a[1];
	return NULL;
}

const XML_Char * IE_Imp_AbiWord_1::_getDataItemMimeType(const XML_Char ** atts)
{
	// find the 'name="value"' pair and return the "value".
	// ignore everything else (which there shouldn't be)

	for (const XML_Char ** a = atts; (*a); a++)
		if (UT_XML_strcmp(a[0],"mime-type") == 0)
			return a[1];
	// if the mime-type was not specified, for backwards 
 	// compatibility we assume that it is a png image
 	return "image/png";
}

UT_Bool IE_Imp_AbiWord_1::_getDataItemEncoded(const XML_Char ** atts)
{
	for (const XML_Char ** a = atts; (*a); a++)
		if (UT_XML_strcmp(a[0],"base64") == 0) {
		   	if (UT_XML_strcmp(a[1], "no") == 0) return UT_FALSE;
		}
   	return UT_TRUE;
   	
}
