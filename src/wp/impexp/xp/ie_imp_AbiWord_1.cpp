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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#ifdef ENABLE_RESOURCE_MANAGER
#include "xap_ResourceManager.h"
#endif

#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "pd_Style.h"

#include "ie_impexp_AbiWord_1.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_types.h"
#include "pp_Author.h"
#include "pp_AttrProp.h"

#include <sstream>

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

// supported suffixes
static IE_SuffixConfidence IE_Imp_AbiWord_1_Sniffer__SuffixConfidence[] = {
	{ "abw", 	UT_CONFIDENCE_PERFECT 	},
	{ "awt", 	UT_CONFIDENCE_PERFECT 	},
	{ "zabw", 	UT_CONFIDENCE_PERFECT 	},
	{ "abw.gz", UT_CONFIDENCE_PERFECT 	},
	{ "bzabw", 		UT_CONFIDENCE_PERFECT 	},
	{ "abw.bz2", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_AbiWord_1_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_AbiWord_1_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_AbiWord_1_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	IE_MIMETYPE_AbiWord, 					UT_CONFIDENCE_GOOD 	},
	/* aliases */
	{ IE_MIME_MATCH_FULL, 	"application/abiword",					UT_CONFIDENCE_GOOD	},
	{ IE_MIME_MATCH_FULL, 	"application/abiword-template",			UT_CONFIDENCE_GOOD	},
	{ IE_MIME_MATCH_FULL, 	"application/x-vnd.AbiSource.AbiWord",	UT_CONFIDENCE_GOOD	},
	{ IE_MIME_MATCH_FULL, 	"text/abiword",							UT_CONFIDENCE_GOOD	},
	{ IE_MIME_MATCH_FULL, 	"text/x-abiword",						UT_CONFIDENCE_GOOD	},
	{ IE_MIME_MATCH_FULL,	"application/abiword-compressed", 		UT_CONFIDENCE_POOR 	}, 
	{ IE_MIME_MATCH_BOGUS, 	"", 									UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_AbiWord_1_Sniffer::getMimeConfidence ()
{
	return IE_Imp_AbiWord_1_Sniffer__MimeConfidence;
}

UT_Confidence_t IE_Imp_AbiWord_1_Sniffer::recognizeContents (const char * szBuf,
												  UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	const char *magic ;
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

bool IE_Imp_AbiWord_1_Sniffer::getDlgLabels (const char ** szDesc,
											 const char ** szSuffixList,
											 IEFileType * ft)
{
	*szDesc = "AbiWord Documents (.abw, .awt, .zabw)";
	*szSuffixList = "*.abw; *.awt; *.zabw; *.abw.gz; *.bzabw; *.abw.bz2";
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
			X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
		if ( !m_bWroteParagraph )
			X_CheckError(appendStrux(PTX_Block, PP_NOPROPS));
	}
	
  if (m_refMap)
  {
	  m_refMap->purgeData();
	  delete m_refMap;
	  m_refMap = 0;
  }
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
  : IE_Imp_XML(pDocument, true), 
	m_bWroteSection (false),
    m_bWroteParagraph(false), 
	m_bDocHasLists(false), 
	m_bDocHasPageSize(false),
	m_iInlineStart(0), 
	m_refMap(new UT_GenericStringMap<UT_UTF8String*>),
	m_bAutoRevisioning(false),
	m_bInMath(false),
	m_bInEmbed(false),
	m_iImageId(0)
{
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
#define TT_EMBED           36 //<embed> Generic Embeded Run
#define TT_AUTHORSECTION   37 //<authors>
#define TT_AUTHOR          38 //<author>
#define TT_ANN             39 //<ann> Annotate region
#define TT_ANNOTATE        40 //<annotate> Annotation content
#define TT_RDFBLOCK        41 //<rdf> complete block
#define TT_RDFTRIPLE       42 //<t> but only within an <rdf> block
#define TT_TEXTMETA        43 //<textmeta> 


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
/// It appears that prefix strings sort before longer ones too (a < abiword)
///
static struct xmlToIdMapping s_Tokens[] =
{
	{	"a",			TT_HYPERLINK	},
	{	"abiword",		TT_DOCUMENT		},
	{	"ann",		    TT_ANN  		},
	{	"annotate",		TT_ANNOTATE		},
	{   "author",       TT_AUTHOR      },
	{   "authors",      TT_AUTHORSECTION},
	{	"awml",			TT_DOCUMENT		},
	{	"bookmark",		TT_BOOKMARK		},
	{	"br",			TT_BREAK		},
	{	"c",			TT_INLINE		},
	{	"cbr",			TT_COLBREAK		},
	{	"cell",		    TT_CELL		    },
	{	"d",			TT_DATAITEM		},
	{	"data",			TT_DATASECTION	},
	{   "embed",      TT_EMBED      },
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
    {   "rdf",          TT_RDFBLOCK     },
	{   "resource",     TT_RESOURCE     },
	{   "revisions",    TT_REVISIONSECTION},
	{	"s",			TT_STYLE		},
	{	"section",		TT_SECTION		},
	{	"styles",		TT_STYLESECTION	},
	{	"t",		    TT_RDFTRIPLE	},
	{	"table",		TT_TABLE		},
	{	"textmeta",		TT_TEXTMETA		},
	{	"toc",		    TT_TOC  		},
	{   "version",      TT_VERSION      }
	
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

void IE_Imp_AbiWord_1::startElement(const gchar *name,
									const gchar **attributes)
{
	const gchar ** atts =
		(const gchar **)UT_cloneAndDecodeAttributes (attributes);
	
	xxx_UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed

	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	// if we are loading styles only, we do not care about anything
	// but the two style tags and the doc tag ...
	if(getLoadStylesOnly() &&
	   tokenIndex != TT_STYLESECTION && tokenIndex != TT_STYLE && tokenIndex != TT_DOCUMENT)
		goto cleanup;

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;

		if(!isClipboard() && (!getLoadStylesOnly() || getLoadDocProps()))
		{
		  X_CheckError(getDoc()->setAttrProp(atts));
		}
		goto cleanup;

	case TT_SECTION:
	{
		X_VerifyParseState(_PS_Doc);
		const gchar * pszId = static_cast<const gchar*>(_getXMLPropValue("id", atts));
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
		    X_CheckError(appendStrux(PTX_Section, PP_std_copyProps(atts)));
		    goto cleanup;
		}
		else
		{
//
// OK this is a header/footer with an id without a matching section. Fix this
// now.
//
			const gchar * pszType = static_cast<const gchar*>(_getXMLPropValue("type", atts));
			if(pszType)
			{
				pf_Frag_Strux* sdh = getDoc()->getLastSectionMutableSDH();
				getDoc()->changeStruxAttsNoUpdate(sdh,pszType,pszId);
				m_parseState = _PS_Sec;
				m_bWroteSection = true;
				X_CheckError(appendStrux(PTX_Section, PP_std_copyProps(atts)));
				goto cleanup;
			}
			m_error = UT_IE_SKIPINVALID;
		    X_EatIfAlreadyError();
		    goto cleanup;
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

		const gchar * pszId = static_cast<const gchar*>(_getXMLPropValue("footnote-id", atts));
		UT_DebugOnly<bool> bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::Footnote, id+1);
			UT_ASSERT(bOK);
		}

		X_CheckError(appendStrux(PTX_SectionFootnote, PP_std_copyProps(atts)));
		xxx_UT_DEBUGMSG(("FInished Append footnote strux \n"));
		goto cleanup;
	}
	case TT_ANNOTATE:
	{
		// Annotations are contained inside a Block
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;

		// Don't check for id on the footnote. It should match an id on
        // a footnote reference field.

		// Do we Need to set the min Unique id now???
		const gchar * pszId = static_cast<const gchar*>(_getXMLPropValue("annotation-id", atts));
		UT_DebugOnly<bool> bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::Annotation, id+1);
			UT_ASSERT(bOK);
		}
		X_CheckError(appendStrux(PTX_SectionAnnotation, PP_std_copyProps(atts)));
		UT_DEBUGMSG(("FInished Append Annotation strux \n"));
		goto cleanup;
	}
	case TT_ENDNOTE:
	{
		// Endnotes are contained inside a Block
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;

		// Need to set the min Unique id now.

		const gchar * pszId = static_cast<const gchar*>(_getXMLPropValue("endnote-id", atts));
		UT_DebugOnly<bool> bOK = true;
		if(pszId)
		{
			UT_uint32 id = atoi(pszId);
			bOK = getDoc()->setMinUID(UT_UniqueId::Endnote, id+1);
			UT_ASSERT(bOK);
		}

		// Don't check for id of the endnote strux. It should match the
		// id of the endnote reference.

		X_CheckError(appendStrux(PTX_SectionEndnote, PP_std_copyProps(atts)));
		xxx_UT_DEBUGMSG(("Finished Append Endnote strux \n"));
		goto cleanup;
	}
	case TT_TOC:
	{
		// Table of Contents are inside a DocSection
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionTOC, PP_std_copyProps(atts)));
		UT_DEBUGMSG(("Finished Append TOC strux \n"));
		goto cleanup;
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
		const gchar * pszId = _getXMLPropValue("list", atts);
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
		X_CheckError(appendStrux(PTX_Block, PP_std_copyProps(atts)));
		m_iInlineStart = getOperationCount();
		goto cleanup;
	}
	case TT_INLINE:
		// ignored for fields
		if (m_parseState == _PS_Field) goto cleanup;

		// when pasting from clipboard, the doc might not open with section/paragraph
		if(!isClipboard() || m_bWroteParagraph)
			X_VerifyParseState(_PS_Block);
		else
		{
			m_parseState = _PS_Block;
			m_bWroteParagraph = true;
		}
		
		
		X_CheckError(_pushInlineFmt(PP_std_copyProps(atts)));

		if(!isClipboard())
			X_CheckError(appendFmt(m_vecInlineFmt));

		m_iInlineStart++;
		goto cleanup;

		// Images and Fields are not containers.  Therefore we don't
		// push the ParseState (_PS_...).
		// TODO should Images or Fields inherit the (possibly nested)
		// TODO inline span formatting.

	case TT_IMAGE:
	{
		if(m_bInMath)
		{
			UT_DEBUGMSG(("Ignore image in math-tag \n"));
			goto cleanup;
		}
		if(m_bInEmbed)
		{
			UT_DEBUGMSG(("Ignore image in embed-tag \n"));
			goto cleanup;
		}
		//		X_VerifyParseState(_PS_Block);
#ifdef ENABLE_RESOURCE_MANAGER
		X_CheckError(_handleImage (atts));
#else
		//const gchar * pszId = _getXMLPropValue("dataid", atts);

		//
		// Remove this assert because the image object MUST have already
		// defined the correct ID.
		//
		X_CheckError(appendObject(PTO_Image, PP_std_copyProps(atts)));
#endif
		goto cleanup;
	}
	case TT_MATH:
	{
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Math, PP_std_copyProps(atts)));
		m_iImageId++;
		getDoc()->setMinUID(UT_UniqueId::Image, m_iImageId);
		m_bInMath = true;
		goto cleanup;
	}
	case TT_EMBED:
	{
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Embed, PP_std_copyProps(atts)));
		
		m_iImageId++;
		getDoc()->setMinUID(UT_UniqueId::Image, m_iImageId);
		m_bInEmbed = true;
		goto cleanup;
	}
	case TT_BOOKMARK:
    {
		X_VerifyParseState(_PS_Block);


        const gchar* type = UT_getAttribute("type", atts);
        if( type && !strcmp(type,"end"))
        {
            std::string  xmlid = "";
            const gchar* nameAttr  = UT_getAttribute("name", atts);
            if( nameAttr )
            {
                xmlid = xmlidMapForBookmarks[nameAttr];
                xmlidMapForBookmarks.erase(nameAttr);
            }
            PP_PropertyVector pp = PP_std_copyProps(atts);
            pp.push_back(PT_XMLID);
            pp.push_back(xmlid);
            X_CheckError(appendObject(PTO_Bookmark, pp));
        }
        else
        {
            X_CheckError(appendObject(PTO_Bookmark, PP_std_copyProps(atts)));
            const gchar* nameAttr  = UT_getAttribute("name", atts);
            const gchar* xmlid = UT_getAttribute("xml:id", atts);
            if( name && xmlid )
            {
                xmlidMapForBookmarks[nameAttr] = ( xmlid ? xmlid : "" );
            }
        }
        
		goto cleanup;
    }
    
	case TT_TEXTMETA:
    {
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_RDFAnchor, PP_std_copyProps(atts)));
        const gchar* xmlid = UT_getAttribute("xml:id", atts);
        if( !xmlid )
            xmlid = "";
        xmlidStackForTextMeta.push_back(xmlid);
		goto cleanup;
    }
    
	case TT_HYPERLINK:
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Hyperlink, PP_std_copyProps(atts)));
		goto cleanup;

	case TT_ANN:
		X_VerifyParseState(_PS_Block);
		X_CheckError(appendObject(PTO_Annotation, PP_std_copyProps(atts)));
		goto cleanup;

	case TT_FIELD:
	{
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Field;
		X_CheckError(appendObject(PTO_Field, PP_std_copyProps(atts)));
		goto cleanup;
	}

		// Forced Line Breaks are not containers.  Therefore we don't
		// push the ParseState (_PS_...).  Breaks are marked with a
		// tag, but are translated into character data (LF).  This may
		// seem a little odd (perhaps an &lf; entity would be better).
		// Anyway, this distinction from ordinary LF's in the document
		// (which get mapped into SPACE) keeps the file sanely editable.
	
	case TT_BREAK:
		if(X_TestParseState(_PS_Field))
			goto cleanup; // just return

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
		goto cleanup;

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
		goto cleanup;

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
		goto cleanup;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_DataSec;
		// We don't need to notify the piece table of the data section,
		// it will get the hint when we begin sending data items.
		goto cleanup;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_DataItem;
#ifdef ENABLE_RESOURCE_MANAGER
		_handleResource (atts, false);
#else
		m_currentDataItem.truncate(0);
		X_CheckError((m_currentDataItemName = g_strdup(_getDataItemName(atts))));
		m_currentDataItemMimeType = _getDataItemMimeType(atts);
		m_currentDataItemEncoded = _getDataItemEncoded(atts);
#endif
		goto cleanup;

	case TT_RESOURCE:
#ifdef ENABLE_RESOURCE_MANAGER
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_DataItem;
		_handleResource (atts, true);
#endif
		goto cleanup;

	case TT_STYLESECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_StyleSec;
		// We don't need to notify the piece table of the style section,
		// it will get the hint when we begin sending styles.
		goto cleanup;

	case TT_STYLE:
	{
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Style;
//
// Have to see if the style already exists. If it does, replace it with this.
//
		const gchar * szName = UT_getAttribute(PT_NAME_ATTRIBUTE_NAME,atts);
		PD_Style * pStyle = NULL;
		if(getDoc()->getStyle(szName, &pStyle))
		{
			X_CheckError(pStyle->addAttributes(atts));
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			X_CheckError(getDoc()->appendStyle(PP_std_copyProps(atts)));
		}
		goto cleanup;
	}

	case TT_REVISIONSECTION:
	{
				
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_RevisionSec;

		// parse the attributes ...
		const gchar * szS = UT_getAttribute("show",atts);
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
		goto cleanup;
	}

	case TT_REVISION:
	{
		X_VerifyParseState(_PS_RevisionSec);
		m_parseState = _PS_Revision;

		const gchar * szS = UT_getAttribute(PT_ID_ATTRIBUTE_NAME,atts);
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

		goto cleanup;
	}

	case TT_AUTHORSECTION:
	{
				
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_AuthorSec;
		goto cleanup;
	}

	case TT_AUTHOR:
	{
		X_VerifyParseState(_PS_AuthorSec);
		m_parseState = _PS_Author;

		const gchar * szInt = UT_getAttribute("id",atts);
		UT_sint32 iAuthorInt = atoi(szInt);
		pp_Author * pA = getDoc()->addAuthor(iAuthorInt);
		PP_AttrProp * pPA = pA->getAttrProp();
		const gchar * szProps = UT_getAttribute(PT_PROPS_ATTRIBUTE_NAME,atts);
		if(szProps)
		{
			const gchar * szExtraAtts[3] = {PT_PROPS_ATTRIBUTE_NAME,
											szProps,
											NULL};
			pPA->setAttributes(szExtraAtts);
		}
		goto cleanup;
	}


	case TT_HISTORYSECTION:
	{
				
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_HistorySec;

		// parse the attributes ...
		const gchar * szS = UT_getAttribute("version",atts);
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
		
		goto cleanup;
	}
	
	case TT_VERSION:
	{
		X_VerifyParseState(_PS_HistorySec);
		m_parseState = _PS_Version;

		const gchar * szS = UT_getAttribute(PT_ID_ATTRIBUTE_NAME,atts);
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

			UT_uint32 iXID = 0;
			szS = UT_getAttribute("top-xid",atts);
			if(szS)
				iXID = atoi(szS);
			
			szS = UT_getAttribute("uid",atts);

			if(szS)
			{
				AD_VersionData v(iId, szS, tStarted, bAuto, iXID);
				getDoc()->addRecordToHistory(v);
			}
		}

		goto cleanup;
	}

		case TT_LISTSECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_ListSec;
		// As per styles, we don't need to notify the piece table.
		goto cleanup;

	case TT_LIST:
		X_VerifyParseState(_PS_ListSec);
		m_parseState = _PS_List;
		// Urgh! Complex. I think how done.
		X_CheckError(getDoc()->appendList(atts));
		m_bDocHasLists = true;
		goto cleanup;

	case TT_PAGESIZE:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_PageSize;
		X_CheckError(getDoc()->setPageSizeFromFile(atts));
		m_bDocHasPageSize = true;
		goto cleanup;

	case TT_IGNOREDWORDS:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_IgnoredWordsSec;
		goto cleanup;

	case TT_IGNOREDWORD:
		X_VerifyParseState(_PS_IgnoredWordsSec);
		m_parseState = _PS_IgnoredWordsItem;
		goto cleanup;

	case TT_METADATA:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_MetaData;
		goto cleanup;

	case TT_META:
		X_VerifyParseState(_PS_MetaData);
		m_parseState = _PS_Meta;
		m_currentMetaDataName = _getXMLPropValue("key", atts);
		goto cleanup;

	case TT_RDFBLOCK:
    {
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_RDFData;
        PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
        m_rdfMutation = rdf->createMutation();
		goto cleanup;
    }
    
	case TT_RDFTRIPLE:
    {
		X_VerifyParseState(_PS_RDFData);
		m_parseState   = _PS_RDFTriple;
		m_rdfSubject   = _getXMLPropValue("s", atts);
		m_rdfPredicate = _getXMLPropValue("p", atts);
        m_rdfXSDType   = _getXMLPropValue("xsdtype", atts);
        {
            std::stringstream ss;
            ss << _getXMLPropValue("objecttype", atts);
            m_rdfObjectType = PD_Object::OBJECT_TYPE_URI;
            ss >> m_rdfObjectType;
        }
        // content of element is object.
		goto cleanup;
    }
    
	case TT_TABLE:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionTable, PP_std_copyProps(atts)));
		goto cleanup;

	case TT_CELL:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionCell, PP_std_copyProps(atts)));
		goto cleanup;

	case TT_FRAME:
		m_parseState = _PS_Sec;
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_SectionFrame, PP_std_copyProps(atts)));
		goto cleanup;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
		   UT_ASSERT_NOT_REACHED();
		goto cleanup;
	}

  cleanup:
	gchar ** p = (gchar **) atts;
	if (p)
	{
		while (*p)
		{
			FREEP(*p);
			++p;
		}

		g_free ((void*)atts);
	}
}

void IE_Imp_AbiWord_1::endElement(const gchar *name)
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
		X_CheckError(appendStrux(PTX_EndFootnote, PP_NOPROPS));
		xxx_UT_DEBUGMSG(("FInished Append End footnote strux \n"));
		m_parseState = _PS_Block;
		return;

	case TT_ANNOTATE:
		X_VerifyParseState(_PS_Sec);
		X_CheckError(appendStrux(PTX_EndAnnotation, PP_NOPROPS));
		UT_DEBUGMSG(("FInished Append End annotation strux \n"));
		m_parseState = _PS_Block;
		return;

	case TT_ENDNOTE:
		X_VerifyParseState(_PS_Sec);
		X_CheckError(appendStrux(PTX_EndEndnote, PP_NOPROPS));
		xxx_UT_DEBUGMSG(("Finished Append End Endnote strux \n"));
		m_parseState = _PS_Block;
		return;


	case TT_TOC:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndTOC, PP_NOPROPS));
		UT_DEBUGMSG(("Finished Append End TOC strux \n"));
		return;

	case TT_TABLE:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndTable, PP_NOPROPS));
		return;

	case TT_CELL:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndCell, PP_NOPROPS));
		return;

	case TT_FRAME:
		X_VerifyParseState(_PS_Sec);
		m_bWroteSection = true;
		X_CheckError(appendStrux(PTX_EndFrame, PP_NOPROPS));
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
			X_CheckError(appendFmt(m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		//		X_VerifyParseState(_PS_Block);
		m_iImageId++;
		getDoc()->setMinUID(UT_UniqueId::Image, m_iImageId);
		return;

	case TT_MATH:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		m_bInMath = false;
		UT_DEBUGMSG(("In math set false \n"));
		X_VerifyParseState(_PS_Block);
		return;
	case TT_EMBED:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		m_bInEmbed = false;
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BOOKMARK:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_TEXTMETA:						// not a container, so we don't pop stack
    {
        std::string xmlid = xmlidStackForTextMeta.back();
        xmlidStackForTextMeta.pop_back();

		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);

        PP_PropertyVector ppAtts = {
            PT_XMLID, xmlid,
            // sanity check
            "this-is-an-rdf-anchor", "yes",
            PT_RDF_END, "yes"
        };
		X_CheckError(appendObject(PTO_RDFAnchor,ppAtts));
		return;
    }
    
	case TT_HYPERLINK:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		// we append another Hyperlink Object, but with no attributes
		X_CheckError(appendObject(PTO_Hyperlink, PP_NOPROPS));
		return;

	case TT_ANN:						// not a container, so we don't pop stack
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		UT_DEBUGMSG(("End of annotation region \n"));
		// we append another Hyperlink Object, but with no attributes
		X_CheckError(appendObject(PTO_Annotation, PP_NOPROPS));
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
#ifndef ENABLE_RESOURCEMANAGER
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
		m_currentDataItemMimeType.clear();
#endif
 		return;

	case TT_RESOURCE:
#ifdef ENABLE_RESOURCEMANAGER
		X_VerifyParseState(_PS_DataItem);
		m_parseState = _PS_Doc;
#endif
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
											   m_currentRevisionVersion, true));
			m_currentRevisionId = 0;
		}
		
		m_parseState = _PS_RevisionSec;
		return;


	case TT_AUTHORSECTION:
		X_VerifyParseState(_PS_AuthorSec);
		m_parseState = _PS_Doc;
		return;

	case TT_AUTHOR:
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Author);
		m_parseState = _PS_AuthorSec;
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

	case TT_RDFBLOCK:
		X_VerifyParseState(_PS_RDFData);
		m_parseState = _PS_Doc;
        if(m_rdfMutation)
        {
            m_rdfMutation->commit();
        }
		return;

	case TT_RDFTRIPLE:
		X_VerifyParseState(_PS_RDFTriple);
		m_parseState = _PS_RDFData;
		return;
        

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		return;
	}
}

/*****************************************************************/
/*****************************************************************/

const gchar * IE_Imp_AbiWord_1::_getDataItemName(const gchar ** atts)
{
	return _getXMLPropValue ("name", atts);
}

const gchar * IE_Imp_AbiWord_1::_getDataItemMimeType(const gchar ** atts)
{
	const gchar *val = _getXMLPropValue ("mime-type", atts);

	// if the mime-type was not specified, for backwards
 	// compatibility we assume that it is a png image
	return (val ? val : "image/png");
}

bool IE_Imp_AbiWord_1::_getDataItemEncoded(const gchar ** atts)
{
  	const gchar *val = _getXMLPropValue ("base64", atts);

	if ((!val) || (strcmp (val, "no") != 0))
	  return true;

	return false;
}

bool IE_Imp_AbiWord_1::_handleImage(const gchar ** atts)
{
#ifdef ENABLE_RESOURCE_MANAGER
	static const char * psz_href = "href"; // could make this xlink:href, but is #ID valid in XLINK?

	XAP_ResourceManager & RM = getDoc()->resourceManager ();

	/* old: <image dataid="ID" props="height:HH; width:WW" />
	 * new: <image href="#ID" props="height:HH; width:WW" />
	 * 
	 * we need to re-map resource IDs so that we can allocate new IDs
	 * sensibly later on
	 */
	const char * old_id = 0;

	/* going to assume the document is one or the other, not a mixture...
	 */
	bool is_data = false;
	bool is_href = false;

	UT_uint32 natts = 0;
	const char ** attr = atts;
	while (*attr)
		{
			if ((strcmp (*attr, "href") == 0) || (strcmp (*attr, "xlink:href") == 0))
				{
					attr++;
					old_id = *attr;
					is_href = true;
				}
			else if (strcmp (*attr, "dataid") == 0)
				{
					attr++;
					old_id = *attr;
					is_data = true;
				}
			else attr++;
			attr++;
			natts += 2;
		}
	if (is_href && is_data) return false; // huh?

	if ( old_id == 0) return false; // huh?
	if (*old_id == 0) return false; // huh?

	UT_UTF8String re_id;

	const UT_UTF8String * new_id = 0;

	if (is_href && (*old_id != '#'))
		{
			/* this is a hyperlink; we don't map these
			 */
			re_id = RM.new_id (false); // external resource id, "/re_abc123"

			new_id = &re_id;
		}
	else if ((new_id = m_refMap->pick (old_id)) == 0)
		{
			/* first occurence of this href/dataid; add to map
			 */
			UT_UTF8String * ri_id = new UT_UTF8String(RM.new_id());
			if (ri_id)
				{
					m_refMap->insert (old_id, ri_id);
					if ((new_id = m_refMap->pick (old_id)) == 0)
						{
							delete ri_id;
						}
				}
		}
	if (new_id == 0) return false; // hmm

	/* it is necessary to reference a resource before you can set URL or data
	 */
	if (!RM.ref (new_id->utf8_str ())) return false; // reference the object

	/* for external resources (i.e., hyperlinks) we set the URL now; data comes *much* later...
	 */
	if (is_href && (*old_id != '#'))
		{
			XAP_ExternalResource * re = dynamic_cast<XAP_ExternalResource *>(RM.resource (re_id.utf8_str (), false));
			if (re == 0) return false; // huh?

			re->URL (UT_UTF8String(old_id));
		}

	/* copy attribute list; replace dataid/href value with new ID
	 */
	const char ** new_atts = static_cast<const char **>(g_try_malloc ((natts + 2) * sizeof (char *)));
	if (new_atts == 0) return false; // hmm

	const char ** new_attr = new_atts;
	attr = atts;
	while (*attr)
		{
			if ((strcmp (*attr, "href") == 0) || (strcmp (*attr, "xlink:href") == 0) || (strcmp (*attr, "dataid") == 0))
				{
					*new_attr++ = psz_href; // href="#ID"
					*new_attr++ = new_id->utf8_str ();
				}
			else
				{
					*new_attr++ = *attr++;
					*new_attr++ = *attr++;
				}
		}
	*new_attr++ = 0;
	*new_attr++ = 0;

	bool success = appendObject (PTO_Image, new_atts);
	m_iImageId++;
	getDoc()->setMinUID(UT_UniqueId::Image, m_iImageId);

	g_free (new_atts);

	return success;
#else
	UT_UNUSED(atts);
	return false;
#endif
}

bool IE_Imp_AbiWord_1::_handleResource (const gchar ** atts, bool isResource)
{
#ifdef ENABLE_RESOURCE_MANAGER
	if (atts == 0) return false;

	XAP_ResourceManager & RM = getDoc()->resourceManager ();

	if (isResource)
		{
			// <resource id="ID" type="" desc=""> ... </resource>

			const gchar * r_id = 0;
			const gchar * r_mt = 0;
			const gchar * r_ds = 0;

			const gchar ** attr = atts;
			while (*attr)
				{
					if (strcmp (*attr, "id") == 0)
						{
							attr++;
							r_id = *attr++;
						}
					else if (strcmp (*attr, "type") == 0)
						{
							attr++;
							r_mt = *attr++;
						}
					else if (strcmp (*attr, "desc") == 0)
						{
							attr++;
							r_ds = *attr++;
						}
					else
						{
							attr++;
							attr++;
						}
				}
			if (r_id == 0) return false;

			XAP_InternalResource * ri = dynamic_cast<XAP_InternalResource *>(RM.resource (r_id, true));
			if (ri == 0) return false;

			if (r_mt) ri->type (r_mt);
			if (r_ds) ri->Description = r_ds;

			m_currentDataItemEncoded = true;

			return true;
		}
	else
		{
			// <d name="ID" mime-type="image/png" base64="yes"> ... </d>
			// <d name="ID" mime-type="image/svg+xml | application/mathxml+xml" base64="no"> <![CDATA[ ... ]]> </d>

			const gchar * r_id = 0;
			const gchar * r_64 = 0;

			enum { mt_unknown, mt_png, mt_svg, mt_mathml,mt_embed } mt = mt_unknown;
			const gchar * pszEmbed = NULL;
			const gchar ** attr = atts;
			while (*attr)
				{
					if (strcmp (*attr, "name") == 0)
						{
							attr++;
							r_id = *attr++;
						}
					else if (strcmp (*attr, "mime-type") == 0)
						{
							attr++;

							if (strcmp (*attr, "image/png") == 0)
								mt = mt_png;
							else if (strcmp (*attr, "image/svg+xml") == 0 || strcmp (*attr, "image/svg"))
								mt = mt_svg;
							else if (strcmp (*attr, "application/mathml+xml") == 0)
								mt = mt_mathml;
							else
								{
									pszEmbed = *attr;
									mt = mt_embed;
								}

							attr++;
						}
					else if (strcmp (*attr, "base64") == 0)
						{
							attr++;
							r_64 = *attr++;
						}
					else
						{
							attr++;
							attr++;
						}
				}
			if (r_id == 0) return false;
			if (r_64 == 0) return false;

			/* map dataid to new resource ID
			 */
			const UT_UTF8String * new_id = m_refMap->pick (r_id);
			if (new_id == 0) return false;

			XAP_InternalResource * ri = dynamic_cast<XAP_InternalResource *>(RM.resource (new_id->utf8_str (), true));
			if (ri == 0) return false;

			bool add_resource = false;
			switch (mt)
				{
				case mt_png:
					if (strcmp (r_64, "yes") == 0)
						{
							ri->type ("image/png");
							m_currentDataItemEncoded = true;
							add_resource = true;
						}
					break;
				case mt_svg:
					if (strcmp (r_64, "no") == 0) // hmm, CDATA fun
						{
							ri->type ("image/svg+xml"); // image/svg & image/svg-xml are possible but not recommended
							m_currentDataItemEncoded = false;
							add_resource = true;
						}
					break;
				case mt_mathml:
					if (strcmp (r_64, "no") == 0) // hmm, CDATA fun
						{
							ri->type ("application/mathml+xml"); // preferred by MathML 2.0
							m_currentDataItemEncoded = false;
							add_resource = true;
						}
					break;
				case mt_embed:
					if (strcmp (r_64, "no") == 0) // hmm, CDATA fun
						{
							ri->type (pszEmbed); 
							m_currentDataItemEncoded = false;
							add_resource = true;
						}
					break;
				default:
					break;
				}
			if (!add_resource) RM.clear_current (); // not going to add the data :-(

			return add_resource;
		}
#else
	UT_UNUSED(atts);
	UT_UNUSED(isResource);
	return false;
#endif
}
