/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include "ut_string_class.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ut_hash.h"

#include "xap_EncodingManager.h"

#include "pd_Document.h"
#include "pd_Style.h"

#include "ie_imp_AbiWord_1.h"
#include "ie_types.h"

/*****************************************************************/
/*****************************************************************/

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))  \
									  {  m_error = UT_IE_BOGUSDOCUMENT;	UT_ASSERT_HARMLESS(0); \
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

IE_Imp_AbiWord_1_Sniffer::IE_Imp_AbiWord_1_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_AWML11)
{
	// 
}

UT_Confidence_t IE_Imp_AbiWord_1_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (szMIME, "application/abiword-compressed") == 0)
		{
			/* slightly odd case, since if we're using libxml2 then the compression
			 * doesn't really matter; GZipAbiWord should return a higher confidence
			 * here...
			 */
			return UT_CONFIDENCE_POOR;
		}
	if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_AbiWord) == 0)
		{
			/* MIME types don't normally distinguish compression, so let's be cautious
			 */
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

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
		magic = "<abiword" ;
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
	if(!getLoadStylesOnly()	)
	{
		if ( !m_bWroteSection )
			X_CheckError(appendStrux(PTX_Section,NULL));
		if ( !m_bWroteParagraph )
			X_CheckError(appendStrux(PTX_Block,NULL));
	}
	
  if (m_refMap)
  {
	  m_refMap->purgeData();
	  delete m_refMap;
	  m_refMap = 0;
  }
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
  : IE_Imp_XML(pDocument, true), m_bWroteSection (false),
    m_bWroteParagraph(false), m_bDocHasLists(false), m_bDocHasPageSize(false),
	m_iInlineStart(0), m_refMap(new UT_GenericStringMap<UT_UTF8String*>),
	m_bAutoRevisioning(false)
{
}

/* Quick hack for GZipAbiWord */
UT_Error IE_Imp_AbiWord_1::importFile(const char * szFilename)
{
	return IE_Imp_XML::importFile(szFilename);
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
#define TT_METADATA     21 // <metadata>
#define TT_META         22 // <m>
#define TT_TABLE        23 // <table>
#define TT_CELL         24 // <cell>
#define TT_FOOTNOTE     25 // <foot>
#define TT_MARGINNOTE   26 // <margin>
#define TT_FRAME        27 // <frame>
#define TT_REVISIONSECTION 28 //<revisions>
#define TT_REVISION        29 //<r>
#define TT_RESOURCE        30 // <resource>
#define TT_ENDNOTE         31 //<endnote>
#define TT_HISTORYSECTION  32 //<history>
#define TT_VERSION         33 //<version>
#define TT_TOC             34 //<toc> (Table of Contents
#define TT_MATH            35 //<math> Math Run


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
	{	"cell",		    TT_CELL		    },
	{	"d",			TT_DATAITEM		},
	{	"data",			TT_DATASECTION	},
	{   "endnote",      TT_ENDNOTE      },
	{	"f",			TT_FIELD		},
	{	"field",		TT_FIELD		},
	{	"foot",		    TT_FOOTNOTE	    },
	{	"frame",		TT_FRAME	    },
	{   "history",      TT_HISTORYSECTION},
	{	"i",			TT_IMAGE		},
	{	"ignoredwords",	TT_IGNOREDWORDS	},
	{	"image",		TT_IMAGE		},
	{	"iw",			TT_IGNOREDWORD	},
	{	"l",			TT_LIST			},
	{	"lists",		TT_LISTSECTION	},
	{       "m",        TT_META         },
	{	"margin",		TT_MARGINNOTE	},
	{	"math",		    TT_MATH     	},
	{       "metadata", TT_METADATA     },
	{	"p",			TT_BLOCK		},
	{   "pagesize",     TT_PAGESIZE     },
	{	"pbr",			TT_PAGEBREAK	},
    {   "r",            TT_REVISION     },
	{   "resource",     TT_RESOURCE     },
	{   "revisions",    TT_REVISIONSECTION},
	{	"s",			TT_STYLE		},
	{	"section",		TT_SECTION		},
	{	"styles",		TT_STYLESECTION	},
	{	"table",		TT_TABLE		},
	{	"toc",		    TT_TOC  		},
	{   "version",      TT_VERSION      }
	
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

void IE_Imp_AbiWord_1::startElement(const XML_Char *name, const XML_Char **atts)
{
	xxx_UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed

	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	// if we are loading styles only, we do not care about anything
	// but the two style tags and the doc tag ...
	if(getLoadStylesOnly() &&
	   tokenIndex != TT_STYLESECTION && tokenIndex != TT_STYLE && tokenIndex != TT_DOCUMENT)
		return;

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;

		if(!isClipboard() && (!getLoadStylesOnly() || getLoadDocProps()))
		{
		  X_CheckError(getDoc()->setAttrProp(atts));
		}
		return;

	case TT_SECTION:
	{
		X_VerifyParseState(_PS_Doc);
		const XML_Char * pszId = static_cast<const XML_Char*>(_getXMLPropValue("id", atts));
		bool bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			getDoc()->setMinUID(UT_UniqueId::HeaderFtr, id+1);
			bOK = getDoc()->verifySectionID(pszId);
		}
		if(bOK)
		{
		    m_parseState = _PS_Sec;
		    m_bWroteSection = true;
		    X_CheckError(appendStrux(PTX_Section,atts));
		    return;
		}
		else
		{
//
// OK this is a header/footer with an id without a matching section. Fix this
// now.
//
			const XML_Char * pszType = static_cast<const XML_Char*>(_getXMLPropValue("type", atts));
			if(pszType)
			{
				PL_StruxDocHandle sdh = getDoc()->getLastSectionSDH();
				getDoc()->changeStruxAttsNoUpdate(sdh,pszType,pszId);
				m_parseState = _PS_Sec;
				m_bWroteSection = true;
				X_CheckError(appendStrux(PTX_Section,atts));
				return;
			}
			m_error = UT_IE_SKIPINVALID;
		    X_EatIfAlreadyError();
		    return;
		}
	}
	case TT_FOOTNOTE:
	{
		// Footnotes are contained inside a Block
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;

		// Don't check for id on the footnote. It should match an id on
        // a footnote reference field.

		// Need to set the min Unique id now.

		const XML_Char * pszId = static_cast<const XML_Char*>(_getXMLPropValue("footnote-id", atts));
		bool bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::Footnote, id+1);
		}

		X_CheckError(appendStrux(PTX_SectionFootnote,atts));
		xxx_UT_DEBUGMSG(("FInished Append footnote strux \n"));
		return;
	}
	case TT_ENDNOTE:
	{
		// Endnotes are contained inside a Block
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;

		// Need to set the min Unique id now.

		const XML_Char * pszId = static_cast<const XML_Char*>(_getXMLPropValue("endnote-id", atts));
		bool bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::Endnote, id+1);
		}

		// Don't check for id of the endnote strux. It should match the
		// id of the endnote reference.

		X_CheckError(appendStrux(PTX_SectionEndnote,atts));
		xxx_UT_DEBUGMSG(("Finished Append Endnote strux \n"));
		return;
	}
	case TT_TOC:
	{
		// Table of Contents are inside a DocSection
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionTOC,atts));
		UT_DEBUGMSG(("Finished Append TOC strux \n"));
		return;
	}
	case TT_BLOCK:
	{
		// when pasting from clipboard, the doc might not open with section/paragraph
		if(!isClipboard() || m_bWroteSection)
			X_VerifyParseState(_PS_Sec);
		else
		{
			m_bWroteSection = true;
		}
		
		m_parseState = _PS_Block;
		m_bWroteParagraph = true;
		const XML_Char * pszId = _getXMLPropValue("list", atts);
		bool bOK;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::List,id+1);
			if(!bOK)
			{
				UT_DEBUGMSG(("List id %d [%s] already in use\n",id,pszId));
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			}
		}
		X_CheckError(appendStrux(PTX_Block,atts));
		m_iInlineStart = getOperationCount();
		return;
	}
	case TT_INLINE:
		// ignored for fields
		if (m_parseState == _PS_Field) return;

		// when pasting from clipboard, the doc might not open with section/paragraph
		if(!isClipboard() || m_bWroteParagraph)
			X_VerifyParseState(_PS_Block);
		else
		{
			m_parseState = _PS_Block;
			m_bWroteParagraph = true;
		}
		
		
		X_CheckError(_pushInlineFmt(atts));

		if(!isClipboard())
			X_CheckError(appendFmt(&m_vecInlineFmt));

		m_iInlineStart++;
		return;

		// Images and Fields are not containers.  Therefore we don't
		// push the ParseState (_PS_...).
		// TODO should Images or Fields inherit the (possibly nested)
		// TODO inline span formatting.

	case TT_IMAGE:
	{
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Image,atts));
		return;
	}

	case TT_MATH:
	{
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Math,atts));
		return;
	}
	case TT_BOOKMARK:
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Bookmark,atts));
		return;
		
	case TT_HYPERLINK:
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Hyperlink,atts));
		return;

	case TT_FIELD:
	{
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Field;
		X_CheckError(appendObject(PTO_Field,atts));

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
			X_CheckError(appendSpan(&ucs,1));
		}
		return;

	case TT_COLBREAK:
		X_VerifyParseState(_PS_Block);

		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <cbr/>.  that is, decide
		// TODO if <cbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_VTAB;
			X_CheckError(appendSpan(&ucs,1));
		}
		return;

	case TT_PAGEBREAK:
		X_VerifyParseState(_PS_Block);
		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <pbr/>.  that is, decide
		// TODO if <pbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_FF;
			X_CheckError(appendSpan(&ucs,1));
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

	case TT_RESOURCE:
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

	case TT_REVISIONSECTION:
	{
				
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_RevisionSec;

		// parse the attributes ...
		const XML_Char * szS = UT_getAttribute("show",atts);
		UT_uint32 i;
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setShowRevisions(i != 0);			
		}

		szS = UT_getAttribute("mark",atts);
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setMarkRevisions(i != 0);
		}
		
		szS = UT_getAttribute("show-level",atts);
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setShowRevisionId(i);
		}

		szS = UT_getAttribute("auto",atts);
		if(szS)
		{
			i = atoi(szS);
			// we cannot call setAutoRevisioning() from here because
			// it creates a new revision, so we can only call it after
			// the revisions have been all read it -- we will call it
			// in the </revisions> processing
			m_bAutoRevisioning = (i == 1);

			// autorevisioned documents should not have revision
			// marking turned on
			if(m_bAutoRevisioning)
			{
				getDoc()->setShowRevisionId(PD_MAX_REVISION);
				getDoc()->setShowRevisions(false);
			}
			
			
		}
		return;
	}

	case TT_REVISION:
	{
		X_VerifyParseState(_PS_RevisionSec);
		m_parseState = _PS_Revision;

		const XML_Char * szS = UT_getAttribute(PT_ID_ATTRIBUTE_NAME,atts);
		if(szS)
		{
			m_currentRevisionId = atoi(szS);
			m_currentRevisionTime = 0;

			szS = UT_getAttribute("time-started",atts);
			if(szS)
				m_currentRevisionTime = (time_t)atoi(szS);

			szS = UT_getAttribute("version",atts);
			if(szS)
				m_currentRevisionVersion = atoi(szS);
		}

		return;
	}

	case TT_HISTORYSECTION:
	{
				
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_HistorySec;

		// parse the attributes ...
		const XML_Char * szS = UT_getAttribute("version",atts);
		UT_uint32 i;
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setDocVersion(i);			
		}

		szS = UT_getAttribute("edit-time",atts);
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setEditTime(i);
		}
		
		szS = UT_getAttribute("last-saved",atts);
		if(szS)
		{
			i = atoi(szS);
			getDoc()->setLastSavedTime((time_t)i);
		}
		szS = UT_getAttribute("uid",atts);
		if(szS)
		{
			getDoc()->setDocUUID(szS);
		}
		
		return;
	}
	
	case TT_VERSION:
	{
		X_VerifyParseState(_PS_HistorySec);
		m_parseState = _PS_Version;

		const XML_Char * szS = UT_getAttribute(PT_ID_ATTRIBUTE_NAME,atts);
		if(szS)
		{
			UT_uint32 iId = atoi(szS);

			time_t tStarted = 0;
			szS = UT_getAttribute("started",atts);
			if(szS)
				tStarted = (time_t) atoi(szS);

			bool bAuto = false;
			szS = UT_getAttribute("auto",atts);
			if(szS)
				bAuto = (0 != atoi(szS));
			else
				bAuto = false;
			
			szS = UT_getAttribute("uid",atts);

			AD_VersionData v(iId, szS, tStarted, bAuto);
			getDoc()->addRecordToHistory(v);
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

	case TT_METADATA:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_MetaData;
		return;

	case TT_META:
		X_VerifyParseState(_PS_MetaData);
		m_parseState = _PS_Meta;
		m_currentMetaDataName = _getXMLPropValue("key", atts);
		return;

	case TT_TABLE:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionTable,atts));
		return;

	case TT_CELL:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionCell,atts));
		return;

	case TT_FRAME:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionFrame,atts));
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
		   UT_ASSERT_NOT_REACHED();
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

	// if we are loading styles only, we do not care about anything
	// but the two style tags and the doc tag ...
	if(getLoadStylesOnly() &&
	   tokenIndex != TT_STYLESECTION && tokenIndex != TT_STYLE && tokenIndex != TT_DOCUMENT)
		return;
	
	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		// when pasting from clipboard, there does not have to be any sections/paragraphs
		if(!isClipboard())
			X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;
		return;

	case TT_FOOTNOTE:
		X_VerifyParseState(_PS_Sec);
		X_CheckError(appendStrux(PTX_EndFootnote,NULL));
		xxx_UT_DEBUGMSG(("FInished Append End footnote strux \n"));
		m_parseState = _PS_Block;
		return;

	case TT_ENDNOTE:
		X_VerifyParseState(_PS_Sec);
		X_CheckError(appendStrux(PTX_EndEndnote,NULL));
		xxx_UT_DEBUGMSG(("Finished Append End Endnote strux \n"));
		m_parseState = _PS_Block;
		return;


	case TT_TOC:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndTOC,NULL));
		UT_DEBUGMSG(("Finished Append End TOC strux \n"));
		return;

	case TT_TABLE:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndTable,NULL));
		return;

	case TT_CELL:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndCell,NULL));
		return;

	case TT_FRAME:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndFrame,NULL));
		return;

	case TT_BLOCK:
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;

	case TT_INLINE:
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
                if (m_parseState == _PS_Field) // just return
			  return;
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		// Insert a FmtMark if nothing was inserted in the block.
		if (!isClipboard() && (m_iInlineStart == getOperationCount()+1))
		{
			X_CheckError(getDoc()->appendFmtMark());
		}
		_popInlineFmt();

		if(!isClipboard())
			X_CheckError(appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_MATH:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BOOKMARK:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_HYPERLINK:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		// we append another Hyperlink Object, but with no attributes
		X_CheckError(appendObject(PTO_Hyperlink,NULL));
		return;

		return;

	case TT_FIELD:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Field);
                m_parseState = _PS_Block;
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyInsideBlockOrField();
		return;

	case TT_COLBREAK:					// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_PAGEBREAK:					// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
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
 		X_CheckError(getDoc()->createDataItem(static_cast<const char*>(m_currentDataItemName),m_currentDataItemEncoded,&m_currentDataItem,m_currentDataItemMimeType,NULL));
		FREEP(m_currentDataItemName);
		// the data item will free the token we passed (mime-type)
		m_currentDataItemMimeType = NULL;
 		return;

	case TT_RESOURCE:
 		return;

	case TT_STYLESECTION:
		X_VerifyParseState(_PS_StyleSec);
		if(getLoadStylesOnly())
		{
			stopParser();
			m_parseState = _PS_Doc;
		}
		else
		{
			m_parseState = _PS_Doc;
		}
		
		return;

	case TT_STYLE:
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Style);
		m_parseState = _PS_StyleSec;
		return;

	case TT_HISTORYSECTION:
		X_VerifyParseState(_PS_HistorySec);
		m_parseState = _PS_Doc;
		return;
		
	case TT_VERSION:
		X_VerifyParseState(_PS_Version);
		m_parseState = _PS_HistorySec;
		return;

	case TT_REVISIONSECTION:
		X_VerifyParseState(_PS_RevisionSec);
		m_parseState = _PS_Doc;
		getDoc()->setAutoRevisioning(m_bAutoRevisioning);
		m_bAutoRevisioning = false;
		return;

	case TT_REVISION:
		X_VerifyParseState(_PS_Revision);

		if(m_currentRevisionId != 0)
		{
			// the revision had no comment associated, so it was not
			// added to the doc by the xml paraser
			X_CheckError(getDoc()->addRevision(m_currentRevisionId, NULL, 0,
											   m_currentRevisionTime,
											   m_currentRevisionVersion));
			m_currentRevisionId = 0;
		}
		
		m_parseState = _PS_RevisionSec;
		return;

	case TT_LISTSECTION:
		X_VerifyParseState(_PS_ListSec);
		if (m_bDocHasLists)
			X_CheckError(getDoc()->fixListHierarchy());
		m_parseState = _PS_Doc;
		return;

	case TT_LIST:
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
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

	case TT_METADATA:
		X_VerifyParseState(_PS_MetaData);
		m_parseState = _PS_Doc;
		return;

	case TT_META:
		X_VerifyParseState(_PS_Meta);
		m_parseState = _PS_MetaData;
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
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

bool IE_Imp_AbiWord_1::_handleImage (const XML_Char ** atts)
{
	return false;
}


bool IE_Imp_AbiWord_1::_handleResource (const XML_Char ** atts, bool isResource)
{
	return false;
}
