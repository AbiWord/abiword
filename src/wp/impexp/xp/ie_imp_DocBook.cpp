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
#include "ie_imp_DocBook.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*
 * DocBook is a SGML derivate with lots of friggin' tags
 * We hardly support any of them now, only the ones we export
 */

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("DocBook")

// we use a reference-counted sniffer
static IE_Imp_DocBook_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_DocBook_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "DocBook Importer";
	mi->desc = "Import DocBook Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
  return 1;
}

#endif

/*****************************************************************/
/*****************************************************************/

UT_Confidence_t IE_Imp_DocBook_Sniffer::recognizeContents(const char * szBuf, 
											   UT_uint32 iNumbytes)
{
  // no doubt, this could be better
  // but this should suffice for all I care

  if(strstr(szBuf, "<!DOCTYPE book") == NULL && 
	 strstr(szBuf, "<!doctype book") == NULL)
    return UT_CONFIDENCE_ZILCH;

  // found one of the top 2 but not this... hrm...
  if(strstr(szBuf, "<book") == NULL)
    return UT_CONFIDENCE_SOSO;

  return UT_CONFIDENCE_PERFECT;
}

UT_Confidence_t IE_Imp_DocBook_Sniffer::recognizeSuffix(const char * szSuffix)
{
	if (UT_stricmp(szSuffix,".dbk") == 0)
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_DocBook_Sniffer::constructImporter(PD_Document * pDocument,
												   IE_Imp ** ppie)
{
	IE_Imp_DocBook * p = new IE_Imp_DocBook(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_DocBook_Sniffer::getDlgLabels(const char ** pszDesc,
											 const char ** pszSuffixList,
											 IEFileType * ft)
{
	*pszDesc = "DocBook (.dbk)";
	*pszSuffixList = "*.dbk";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_DocBook::~IE_Imp_DocBook()
{
}

IE_Imp_DocBook::IE_Imp_DocBook(PD_Document * pDocument)
	: IE_Imp_XML(pDocument, false)
{
	m_iTitle [0] = 0;
	m_iTitle [1] = 0;
	m_iTitle [2] = 0;
	m_iTitle [3] = 0;
	m_iTitle [4] = 0;
	m_iTitle [5] = 0;
	m_iTitle [6] = 0;
	m_iCurListID = 10000;
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0               // anything else
#define TT_DOCUMENT	        1		// a document <book>
#define TT_SECTION              2               // card or section
#define TT_BLOCK		3		// a paragraph <para>
#define TT_PHRASE               4               // formatted text
#define TT_EMPHASIS             5               // emphasized (italic) text
#define TT_SUPERSCRIPT          6               // superscript
#define TT_SUBSCRIPT            7               // subscript
#define TT_BLOCKQUOTE           8               // block quote
#define TT_BRIDGEHEAD           9               // heading  <bridgehead ...>
#define TT_CHAPTER              10              // legacy abiword documents
#define TT_TITLE                11              // title
#define	TT_PAGEBREAK			12
#define	TT_PLAINTEXT			13
struct _TokenTable
{
	const char *	m_name;
	int             m_type;
};

static struct xmlToIdMapping s_Tokens[] =
{
	{	"beginpage",		TT_PAGEBREAK		},
	{       "blockquote",           TT_BLOCKQUOTE           },
	{	"book",			TT_DOCUMENT		},
	{       "bridgehead",           TT_BRIDGEHEAD           },
	{       "chapter",              TT_CHAPTER              },
	{       "emphasis",             TT_EMPHASIS             },	
	{	"para",			TT_BLOCK		},
	{       "phrase",               TT_PHRASE               },
	{	"programlisting",		TT_PLAINTEXT	},
	{       "section",              TT_SECTION              },
	{       "subscript",            TT_SUBSCRIPT            },
	{       "superscript",          TT_SUPERSCRIPT          },
	{       "title",                TT_TITLE                }
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  { UT_DEBUGMSG(("DOM: X_VerifyParseState failed: %s\n", #ps)); \
									    m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  { UT_DEBUGMSG(("DOM: X_CheckDocument failed: %s\n", #b)); \
									    m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  { UT_DEBUGMSG(("DOM: X_CheckError failed: %s\n", #v)); \
									    m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

#define	CHAPTER_HEADING		1
#define	SECTION1_HEADING	2
#define	SECTION2_HEADING	3
#define	SECTION3_HEADING	4
#define	SECTION4_HEADING	5
#define	SECTION5_HEADING	6
#define	SECTION6_HEADING	7

/*****************************************************************/
/*****************************************************************/

void IE_Imp_DocBook::startElement(const XML_Char *name,
				   const XML_Char **atts)
{
	UT_DEBUGMSG(("DocBook import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	m_bMustAddTitle = false;

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		/* starts the document */
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Sec;
	    X_CheckError(getDoc()->appendStrux(PTX_Section,(const XML_Char **)NULL));
		m_iInSection = 0;	/* not in a section, nor a chapter */
		return;

	case TT_CHAPTER:
	  {
		X_VerifyParseState(_PS_Sec);
		m_iInSection = 1;
		/* we'll have to add a number */
		m_bMustNumber = true;

	    return;
	  }

	case TT_SECTION:
	  {
		X_VerifyParseState(_PS_Sec);
		m_iInSection ++; /* 1 more section */
	    {
			/* we check is the next title should be numbered or not */
 	    	const XML_Char *p_val = _getXMLPropValue((const XML_Char *)"role", atts);
	    	m_bMustNumber = (p_val != NULL && (strcmp(p_val, "numbered") == 0));
	    }	
		return;
	  }

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(getDoc()->appendStrux(PTX_Block, NULL));
		return;
		
	case TT_BRIDGEHEAD:
	        X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		{
		  const XML_Char **p_atts;
		  XML_Char *buf[3];
		  buf[2] = NULL;

		  const XML_Char *p_val;
		  p_val = _getXMLPropValue((const XML_Char *)"renderas", atts);
		  XML_Char style_att[15] = "Heading a";
		  style_att[8] = p_val[4]; 

		  X_CheckError(getDoc()->appendStrux(PTX_Block, NULL));
		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *) style_att);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(getDoc()->appendFmt(p_atts));
		  return;
		}

	case TT_BLOCKQUOTE:
	        X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		{
		  const XML_Char **p_atts;
		  XML_Char *buf[3];
		  buf[2] = NULL;

		  XML_Char style_att[15] = "Block Text";

		  X_CheckError(getDoc()->appendStrux(PTX_Block, NULL));
		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *) style_att);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(getDoc()->appendFmt(p_atts));
		  return;
		}
	case TT_PLAINTEXT:
	        X_VerifyParseState(_PS_Block);
		{
		  const XML_Char **p_atts;
		  XML_Char *buf[3];
		  buf[2] = NULL;

		  XML_Char style_att[15] = "Plain Text";

		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *) style_att);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(getDoc()->appendFmt(p_atts));
		  m_bWhiteSignificant = true;
		  return;
		}

	case TT_PHRASE:
	case TT_EMPHASIS:
	case TT_SUPERSCRIPT:
	case TT_SUBSCRIPT:
	    X_VerifyParseState(_PS_Block);
	    {
		const XML_Char **p_atts;
		XML_Char *buf[3];
		UT_XML_cloneString(buf[0], "props");
		buf[2] = NULL;
		
		switch(tokenIndex) {
		case TT_EMPHASIS: 
		    UT_XML_cloneString(buf[1], "font-style:italic"); 
		    break;
		case TT_SUPERSCRIPT: 
		    UT_XML_cloneString(buf[1], "text-position:superscript"); 
		    break;
		case TT_SUBSCRIPT: 
		    UT_XML_cloneString(buf[1], "text-position:subscript"); 
		    break;
		case TT_PHRASE:
		{
		    const XML_Char *p_val = _getXMLPropValue((const XML_Char *)"role", atts);
		    if(p_val != NULL && !strcmp(p_val, "strong"))
			UT_XML_cloneString(buf[1],  "font-weight:bold");
		    else
			buf[0] = NULL;
		    break;
		}
		
		default:
		    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		    break;
		}
		
		p_atts = (const XML_Char **)buf;
		X_CheckError(_pushInlineFmt(p_atts));
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
	    }
	    return;
	    
	case TT_TITLE:
	  X_VerifyParseState(_PS_Sec);
	  /* adds a title here */
	  m_bTitleAdded = false;
	  m_bMustAddTitle = true;
	  return;
	 
	case TT_PAGEBREAK:
	  X_VerifyParseState(_PS_Block);
	  {
		UT_UCSChar ucs = UCS_FF;
	  	getDoc()->appendSpan(&ucs,1);
	  }
	  return;
	  
	case TT_OTHER:
	default:
	    UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
	    return;
	}

}

void IE_Imp_DocBook::endElement(const XML_Char *name)
{
  
    UT_DEBUGMSG(("DocBook import: endElement: %s\n", name));

    // xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
   	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Init;
		return;

	case TT_CHAPTER:
		X_VerifyParseState(_PS_Sec);
		m_iInSection = 0;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Sec);
		m_iInSection --;
		return;

	case TT_BRIDGEHEAD:
	case TT_BLOCKQUOTE:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		return;

	case TT_PLAINTEXT:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		m_bWhiteSignificant = false;
		break;		

	case TT_BLOCK:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_PHRASE:
	case TT_EMPHASIS:
	case TT_SUPERSCRIPT:
	case TT_SUBSCRIPT:
	    UT_ASSERT(m_lenCharDataSeen==0);
	    X_VerifyParseState(_PS_Block);
	    X_CheckDocument(_getInlineDepth()>0);
	    _popInlineFmt();
	    X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
	    return;
	    
	case TT_TITLE:
		if (m_bTitleAdded)
		{
	        UT_ASSERT(m_lenCharDataSeen==0);
			X_VerifyParseState(_PS_Block);
			m_parseState = _PS_Sec;
			X_CheckDocument(_getInlineDepth()==0);
			_popInlineFmt();
			X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		}
		m_bTitleAdded = false;
		m_bMustAddTitle = false;
	    return;

	case TT_OTHER:
	default:
	    UT_DEBUGMSG(("Unknown or intentionally unhandled end tag [%s]\n",name));
	    return;
	}
}


/*
 * we redefine this function, so that it intercepts title
 * if a title has to been added, we first check that there is something to add
 * if so, we add the heading
 * if not, we skip the heading.
 * then, it calls its parent's charData.
 */
void IE_Imp_DocBook::charData(const XML_Char *s, int len)
{
	if (m_bMustAddTitle && len > 0)
	{
		/* arf, a title is to be added... damn, ok we take care of it */
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		{
		  const XML_Char **p_atts;
		  /* list of attributes */
		  XML_Char *buf[11];
		  buf[10] = NULL;
		  buf[2] = NULL;

		  if (m_bMustNumber)
		  {
			  /*
			   * we must add a numbered heading; that means that we must put 
			   * it into a list
			   */
			  /* deletes previous lists of same level and above */
			  for (int i = m_iInSection; i < 7; i++)
			  {
				  if (i == SECTION1_HEADING - 1)
					  // section headings are not cut by chapter headings :(
					  continue;
				  m_iTitle [i] = NULL;
			  }
			  
			  UT_XML_cloneString(buf[8], PT_PROPS_ATTRIBUTE_NAME);

			  if (! m_iTitle [m_iInSection - 1])
			  {
				  /* damn! no list... we create one ;) */
				  createList ();
				  UT_XML_cloneString(buf[9], "start-value:1; list-style:Numbered List");
			  }
			  else
				  UT_XML_cloneString(buf[9], "list-style:Numbered List");

			  /* ok now it's created, we should add the id and the parent id */
			  UT_XML_cloneString (buf[2], PT_LEVEL_ATTRIBUTE_NAME);
			  UT_XML_cloneString (
					  buf[3],
					  (XML_Char *) ((UT_String_sprintf ("%d", m_iTitle [m_iInSection - 1] -> getLevel ())).c_str ())
					 );
			  UT_XML_cloneString (buf[4], PT_LISTID_ATTRIBUTE_NAME);
			  UT_XML_cloneString (
					  buf[5],
					  (XML_Char *) ((UT_String_sprintf ("%d", m_iTitle [m_iInSection - 1] -> getID ())).c_str ())
					 );
			  UT_XML_cloneString (buf[6], PT_PARENTID_ATTRIBUTE_NAME);
			  UT_XML_cloneString (
					  buf[7],
					  (XML_Char *) ((UT_String_sprintf ("%d", m_iTitle [m_iInSection - 1] -> getParentID ())).c_str ())
					 );
		  }

		  switch (m_iInSection)
		  {
			  case CHAPTER_HEADING:
				  /* we must add a chapter heading */
		  			UT_XML_cloneString(buf[1], "Chapter Heading");
				  break;
			  case SECTION1_HEADING:
				  /* we must add a section heading */
		  			UT_XML_cloneString(buf[1], "Section Heading");
				  break;
			  case SECTION2_HEADING:
				  /* we must add a heading 1 */
				  if (m_bMustNumber)
				  {
		  			UT_XML_cloneString(buf[1], "Numbered Heading 1");
				  }
				  else
				  {
		  			UT_XML_cloneString(buf[1], "Heading 1");
				  }
				  break;
			  case SECTION3_HEADING:
				  /* we must add a heading 2 */
				  if (m_bMustNumber)
				  {
		  			UT_XML_cloneString(buf[1], "Numbered Heading 2");
				  }
				  else
				  {
		  			UT_XML_cloneString(buf[1], "Heading 2");
				  }
				  break;
			  case SECTION4_HEADING:
			  case SECTION5_HEADING:
			  case SECTION6_HEADING:
				  /* we must add a heading 3 */
				  if (m_bMustNumber)
				  {
		  			UT_XML_cloneString(buf[1], "Numbered Heading 3");
				  }
				  else
				  {
		  			UT_XML_cloneString(buf[1], "Heading 3");
				  }
				  break;

			  default:
				  UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		  }

		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(getDoc()->appendStrux(PTX_Block, p_atts));
		  if (m_bMustNumber)
		  {
			  /* adds field */
			  XML_Char * buf2 [3];
			  const XML_Char **satts;
			  UT_XML_cloneString (buf2[0], PT_TYPE_ATTRIBUTE_NAME);
			  UT_XML_cloneString (buf2[1], "list_label");
			  satts = (const XML_Char **) buf2;
			  buf2 [2] = NULL;

			  X_CheckError (getDoc() -> appendObject (PTO_Field, satts));
			  X_CheckError (getDoc() -> appendFmt (satts));
			  UT_UCSChar ucs = '\t';
			  getDoc()->appendSpan(&ucs,1);
			  _popInlineFmt();
		  }
		  X_CheckError (getDoc() -> appendFmt ((const XML_Char **) NULL));
		}
		  m_bTitleAdded = true;
	}
	IE_Imp_XML :: charData (s, len);
}
/*****************************************************************************/


/*****************************************************************************/
/*
 * this function creates a new list in the document, and insert it in the
 * list table
 */
void IE_Imp_DocBook :: createList (void)
{
	int pid = 0;
	UT_String delim;


	for (int i = m_iInSection - 1; i < 7; i++)
	{
		if (i == SECTION1_HEADING - 1)
			// section headings are not cut by chapter headings :(
			continue;
		m_iTitle [i] = NULL;
	}
	
	for (i = m_iInSection - 2; i >= 1; i++)
	{
		/* retrieves parent id, if available */
		if (m_iTitle [i])
		{
			pid = m_iTitle [i] -> getID ();
			break;
		}
	}


	
	/* creates the new list */
	fl_AutoNum *an = new fl_AutoNum (
			m_iCurListID,
			pid,
			NUMBERED_LIST,
			1,
			"%L.",
			"",
			getDoc ()
		);
	getDoc () -> addList (an);

	/* register it in the vector */
	m_iTitle [m_iInSection - 1] = an;

	/* increment the id counter, so that it is unique */
	m_iCurListID ++;
}
/*****************************************************************************/
