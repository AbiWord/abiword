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
#include "pd_Style.h"
#include "ut_bytebuf.h"
#include "xap_EncodingManager.h"
#include "ut_misc.h"

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

UT_Confidence_t IE_Imp_AbiWord_1_Sniffer::recognizeContents (const char * szBuf, 
												  UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	char *magic ;
	p = szBuf ;
	while( iLinesToRead-- )
	{
		magic = "<abiword " ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
	       magic = "<awml " ;
	       if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
		magic = "<!-- This file is an AbiWord document." ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++ ; p++ ;
			if( iBytesScanned+2 >= iNumbytes ) return(UT_CONFIDENCE_ZILCH);
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
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_AbiWord_1_Sniffer::recognizeSuffix (const char * szSuffix)
{
  if (!UT_stricmp(szSuffix, ".abw") || !UT_stricmp(szSuffix, ".awt"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}

bool IE_Imp_AbiWord_1_Sniffer::getDlgLabels (const char ** szDesc,
											 const char ** szSuffixList,
											 IEFileType * ft)
{
	*szDesc = "AbiWord Documents (.abw, .awt)";
	*szSuffixList = "*.abw; *.awt";
	*ft = getFileType();
	return true;
}

UT_Error IE_Imp_AbiWord_1_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_AbiWord_1 * p = new IE_Imp_AbiWord_1(pDocument);
	*ppie = p;
	return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_AbiWord_1::~IE_Imp_AbiWord_1()
{
  if ( !m_bWroteSection )
    X_CheckError(getDoc()->appendStrux(PTX_Section,NULL));
  if ( !m_bWroteParagraph )
    X_CheckError(getDoc()->appendStrux(PTX_Block,NULL));
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
  : IE_Imp_XML(pDocument, true), m_bWroteSection (false),
    m_bWroteParagraph(false), m_bDocHasLists(false), m_bDocHasPageSize(false)
{
}

/* Quick hack for GZipAbiWord */
UT_Error IE_Imp_AbiWord_1::importFile(const char * szFilename)
{
	UT_Error bret = IE_Imp_XML::importFile(szFilename);
	if(!bret && !m_bDocHasPageSize)
		getDoc()->setDefaultPageSize();
	return bret;
}

/*****************************************************************/
/*****************************************************************/


#define TT_OTHER		0
#define TT_DOCUMENT		1		// a document <abiword>
#define TT_SECTION		2		// a section <section>
#define TT_BLOCK		3		// a paragraph <p>
#define TT_INLINE		4		// inline span of text <c>
#define TT_IMAGE		5		// an image object <image>
#define TT_FIELD		6		// a computed field object <field>
#define TT_BREAK		7		// a forced line-break <br>
#define TT_DATASECTION	8		// a data section <data>
#define TT_DATAITEM		9		// a data item <d> within a data section
#define TT_COLBREAK		10		// a forced column-break <cbr>
#define TT_PAGEBREAK	11		// a forced page-break <pbr>
#define TT_STYLESECTION	12		// a style section <styles>
#define TT_STYLE		13		// a style <s> within a style section
#define TT_LISTSECTION		14	// a list section <lists>
#define TT_LIST			15	// a list <l> within a list section
#define TT_PAGESIZE             16      // The PageSize <pagesize> 
#define TT_IGNOREDWORDS 17		// an ignored words section <ignoredwords>
#define TT_IGNOREDWORD  18      // a word <iw> within an ignored words section
#define TT_BOOKMARK     19		// <bookmark>
#define TT_HYPERLINK	20		// <a href=>
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

///
/// NOTE TO ALL HACKERS!! This must be in alphabetical order on Pain of Death
///
static struct xmlToIdMapping s_Tokens[] =
{
	{	"a",			TT_HYPERLINK	},
	{	"abiword",		TT_DOCUMENT		},
	{	"awml",			TT_DOCUMENT		},
	{	"bookmark",		TT_BOOKMARK		},
	{	"br",			TT_BREAK		},
	{	"c",			TT_INLINE		},
	{	"cbr",			TT_COLBREAK		},
	{	"d",			TT_DATAITEM		},
	{	"data",			TT_DATASECTION	},
	{	"f",			TT_FIELD		},
	{	"field",		TT_FIELD		},
	{	"i",			TT_IMAGE		},
	{	"ignoredwords",	TT_IGNOREDWORDS	},
	{	"image",		TT_IMAGE		},
	{	"iw",			TT_IGNOREDWORD	},
	{	"l",			TT_LIST			},
	{	"lists",		TT_LISTSECTION	},
	{	"p",			TT_BLOCK		},
	{   "pagesize",     TT_PAGESIZE     },
	{	"pbr",			TT_PAGEBREAK	},
	{	"s",			TT_STYLE		},
	{	"section",		TT_SECTION		},
	{	"styles",		TT_STYLESECTION	},
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

void IE_Imp_AbiWord_1::startElement(const XML_Char *name, const XML_Char **atts)
{
	xxx_UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);
	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;

		{
		  const XML_Char * lockedStyles = 0;
		  lockedStyles = (XML_Char*)_getXMLPropValue("styles", atts);;
		  if ((lockedStyles != NULL) && (UT_strcmp(lockedStyles, "locked") == 0))
		    getDoc()->lockStyles(true);
		  else
		    getDoc()->lockStyles(false);
		}
		return;

	case TT_SECTION:
	{
		X_VerifyParseState(_PS_Doc);
		const XML_Char * pszId = (XML_Char*)_getXMLPropValue("id", atts);
		bool bOK = true;
		if(pszId)
		{
		    bOK = getDoc()->verifySectionID(pszId);
		}
		if(bOK)
		{
		    m_parseState = _PS_Sec;
		    m_bWroteSection = true;
		    X_CheckError(getDoc()->appendStrux(PTX_Section,atts));
		    return;
		}
		else
		{
//
// OK this is a header/footer with an id without a matching section. Fix this
// now.
//
			const XML_Char * pszType = (XML_Char*)_getXMLPropValue("type", atts);
			if(pszType)
			{
				PL_StruxDocHandle sdh = getDoc()->getLastSectionSDH();
				getDoc()->changeSectionAttsNoUpdate(sdh,pszType,pszId);
				m_parseState = _PS_Sec;
				m_bWroteSection = true;
				X_CheckError(getDoc()->appendStrux(PTX_Section,atts));
				return;
			}
			m_error = UT_IE_SKIPINVALID;
		    X_EatIfAlreadyError();
		    return;
		}
	}
	case TT_BLOCK:
	{
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		m_bWroteParagraph = true;
		X_CheckError(getDoc()->appendStrux(PTX_Block,atts));
		m_iInlineStart = getOperationCount();
		return;
	}

	case TT_INLINE:
		// ignored for fields
		if (m_parseState == _PS_Field) return;
		X_VerifyParseState(_PS_Block);
		X_CheckError(_pushInlineFmt(atts));
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		m_iInlineStart++;
		return;

		// Images and Fields are not containers.  Therefore we don't
		// push the ParseState (_PS_...).
		// TODO should Images or Fields inherit the (possibly nested)
		// TODO inline span formatting.
		
	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		X_CheckError(getDoc()->appendObject(PTO_Image,atts));
		return;
	case TT_BOOKMARK:
		X_VerifyParseState(_PS_Block);
		X_CheckError(getDoc()->appendObject(PTO_Bookmark,atts));
		return;
	case TT_HYPERLINK:
		X_VerifyParseState(_PS_Block);
		X_CheckError(getDoc()->appendObject(PTO_Hyperlink,atts));
		return;
	
	case TT_FIELD:
	{
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Field;
		X_CheckError(getDoc()->appendObject(PTO_Field,atts));

#ifdef DEBUG
		UT_DEBUGMSG(("SEVIOR: Appending field \n"));
		{	// M$ compilers don't regard for-loop variable scoping! :-<
			for(UT_sint32 i=0; atts[i] != NULL; i++)
			{
				UT_DEBUGMSG(("Element %d is %s \n",i,atts[i]));
			}
		}
#endif
		return;
	}
	
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
			X_CheckError(getDoc()->appendSpan(&ucs,1));
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
			X_CheckError(getDoc()->appendSpan(&ucs,1));
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
			X_CheckError(getDoc()->appendSpan(&ucs,1));
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
	{
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Style;
//
// Have to see if the style already exists. If it does, replace it with this.
//
		const XML_Char * szName = UT_getAttribute(PT_NAME_ATTRIBUTE_NAME,atts);
		PD_Style * pStyle = NULL;
		if(getDoc()->getStyle(szName, &pStyle))
		{
			X_CheckError(pStyle->addAttributes(atts));
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			X_CheckError(getDoc()->appendStyle(atts));
		}
		return;
	}
	case TT_LISTSECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_ListSec;
		// As per styles, we don't need to notify the piece table.
		return;
		
	case TT_LIST:
		X_VerifyParseState(_PS_ListSec);
		m_parseState = _PS_List;
		// Urgh! Complex. I think how done.
		X_CheckError(getDoc()->appendList(atts));
		m_bDocHasLists = true;
		return;

	case TT_PAGESIZE:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_PageSize;
		X_CheckError(getDoc()->setPageSizeFromFile(atts));
		m_bDocHasPageSize = true;
		return;

	case TT_IGNOREDWORDS:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_IgnoredWordsSec;
		return;

	case TT_IGNOREDWORD:
		X_VerifyParseState(_PS_IgnoredWordsSec);
		m_parseState = _PS_IgnoredWordsItem;
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

void IE_Imp_AbiWord_1::endElement(const XML_Char *name)
{
  	xxx_UT_DEBUGMSG(("endElement %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 trim;
	UT_uint32 len;
	const UT_Byte * buffer;

   	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

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
		// Insert a FmtMark if nothing was inserted in the block.
		if (m_iInlineStart == getOperationCount()+1)
		{
			X_CheckError(getDoc()->appendFmtMark());
		}
		_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BOOKMARK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_HYPERLINK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		// we append another Hyperlink Object, but with no attributes		
		X_CheckError(getDoc()->appendObject(PTO_Hyperlink,NULL));
		return;
		
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
		while (trim > 0 && MyIsWhite(buffer[trim])) trim--;
		m_currentDataItem.truncate(trim+1);
#undef MyIsWhite
 		X_CheckError(getDoc()->createDataItem((char*)m_currentDataItemName,m_currentDataItemEncoded,&m_currentDataItem,m_currentDataItemMimeType,NULL));
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
			X_CheckError(getDoc()->fixListHierarchy());
		m_parseState = _PS_Doc;
		return;

	case TT_LIST:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_List);
		m_parseState = _PS_ListSec;
		return;

	case TT_PAGESIZE:
		X_VerifyParseState(_PS_PageSize);
		m_parseState = _PS_Doc;
		return;
		
	case TT_IGNOREDWORDS:
		X_VerifyParseState(_PS_IgnoredWordsSec);
		m_parseState = _PS_Doc;
		return;

	case TT_IGNOREDWORD:
		X_VerifyParseState(_PS_IgnoredWordsItem);
		m_parseState = _PS_IgnoredWordsSec;
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
	return _getXMLPropValue ("name", atts);
}

const XML_Char * IE_Imp_AbiWord_1::_getDataItemMimeType(const XML_Char ** atts)
{
	const XML_Char *val = _getXMLPropValue ("mime-type", atts);

	// if the mime-type was not specified, for backwards 
 	// compatibility we assume that it is a png image
	return (val ? val : "image/png");
}

bool IE_Imp_AbiWord_1::_getDataItemEncoded(const XML_Char ** atts)
{ 
  	const XML_Char *val = _getXMLPropValue ("base64", atts);

	if ((!val) || (UT_XML_strcmp (val, "no") != 0))
	  return true;

	return false;
}
