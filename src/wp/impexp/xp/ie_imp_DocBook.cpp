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

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Imp_DocBook_Sniffer * m_sniffer = 0;
static UT_sint32 m_refs = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_refs && !m_sniffer)
	{
		m_sniffer = new IE_Imp_DocBook_Sniffer ();
		m_refs++;
	}
	else if (m_refs && m_sniffer)
	{
		m_refs++;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	mi->name = "DocBook Importer";
	mi->desc = "Import DocBook Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_refs && m_sniffer);

	m_refs--;
	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_refs)
	{
		delete m_sniffer;
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return SUPPORTS_ABI_VERSION(major, minor, release);
}

#endif

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_DocBook_Sniffer::recognizeContents(const char * szBuf, 
											   UT_uint32 iNumbytes)
{
  // no doubt, this could be better
  // but this should suffice for all I care

  if(strstr(szBuf, "<!DOCTYPE book") == NULL && 
	 strstr(szBuf, "<!doctype book") == NULL)
    return false;

  if(strstr(szBuf, "<book") == NULL)
    return false;

  return true;
}

bool IE_Imp_DocBook_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".dbk") == 0);
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

struct _TokenTable
{
	const char *	m_name;
	int             m_type;
};

static struct xmlToIdMapping s_Tokens[] =
{
	{       "blockquote",           TT_BLOCKQUOTE           },
	{	"book",			TT_DOCUMENT		},
	{       "bridgehead",           TT_BRIDGEHEAD           },
	{       "chapter",              TT_CHAPTER              },
	{       "emphasis",             TT_EMPHASIS             },	
	{	"para",			TT_BLOCK		},
	{       "phrase",               TT_PHRASE               },
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
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_DocBook::_startElement(const XML_Char *name,
				   const XML_Char **atts)
{
	UT_DEBUGMSG(("DocBook import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_CHAPTER:
	case TT_SECTION:
	  {
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		
		// I'm torn as to where to put this:
		// TT_DOCUMENT or here. Oh well :-)
		X_CheckError(m_pDocument->appendStrux(PTX_Section,(const XML_Char **)NULL));
		return;
	  }

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block, NULL));
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

		  X_CheckError(m_pDocument->appendStrux(PTX_Block, NULL));
		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *) style_att);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(m_pDocument->appendFmt(p_atts));
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

		  X_CheckError(m_pDocument->appendStrux(PTX_Block, NULL));
		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *) style_att);
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(m_pDocument->appendFmt(p_atts));
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
		
		switch(s_Tokens[tokenIndex].m_type) {
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
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
	    }
	    return;
	    
	case TT_TITLE:
	    X_VerifyParseState(_PS_Sec);
	    return;

	case TT_OTHER:
	default:
	    UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
	    return;
	}

}

void IE_Imp_DocBook::_endElement(const XML_Char *name)
{
  
        UT_DEBUGMSG(("DocBook import: endElement: %s\n", name));

        // xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
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

	case TT_BRIDGEHEAD:
	case TT_BLOCKQUOTE:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

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
	    X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
	    return;
	    
	case TT_TITLE:
	    return;

	case TT_OTHER:
	default:
	    UT_DEBUGMSG(("Unknown or intentionally unhandled end tag [%s]\n",name));
	    return;
	}
}

