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


#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_units.h"
#include "pt_Types.h"
#include "ie_exp_DocBook.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "ut_path.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("DocBook")

// we use a reference-counted sniffer
static IE_Exp_DocBook_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_DocBook_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "DocBook Exporter";
	mi->desc = "Export DocBook Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
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

	IE_Exp::unregisterExporter (m_sniffer);
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

bool IE_Exp_DocBook_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".dbk") == 0);
}

UT_Error IE_Exp_DocBook_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_DocBook * p = new IE_Exp_DocBook(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_DocBook_Sniffer::getDlgLabels(const char ** pszDesc,
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

IE_Exp_DocBook::IE_Exp_DocBook(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	s_align = 0;
	m_pListener = NULL;
}

IE_Exp_DocBook::~IE_Exp_DocBook()
{
}

int IE_Exp_DocBook :: indent (void)
{
	return ++s_align;
}

int IE_Exp_DocBook :: unindent (void)
{
	if (--s_align < 0)
	{
		s_align = 0;
		UT_DEBUGMSG (("trying to unindent at min indent.\n"));
	}
	return s_align;
}

void IE_Exp_DocBook :: iwrite (const char *txt)
{
	if (s_align)
	{
		char *indent = new char [s_align + 1];
		memset (indent, '\t', s_align);
		indent [s_align] = '\0';

		IE_Exp :: write (indent);
	
		delete (indent);
	}

	IE_Exp :: write (txt);

	return;
}

void IE_Exp_DocBook :: writeln (const char *txt)
{
	iwrite (txt);
	IE_Exp :: write ("\n");

	return;
}
/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT			5
#define BT_PLAINTEXT			6

bool s_DocBook_Listener::_initFile(void)
{
	// write out the doctype descriptor
	m_pie->writeln("<!DOCTYPE book PUBLIC \"-//OASIS//DTD DocBook XML V4.1.2//EN\"");
	m_pie->writeln("\t\"http://www.oasis-open.org/docbook/xml/4.0/docbookx.dtd\">\n");
	m_pie->writeln("<book>");

	m_pie->writeln("<!-- ================================================================================ -->");
	m_pie->writeln("<!-- This DocBook file was created by AbiWord.										-->");
	m_pie->writeln("<!-- AbiWord is a free, Open Source word processor.								   -->");
	m_pie->writeln("<!-- You may obtain more information about AbiWord at www.abisource.com			   -->");
	m_pie->writeln("<!-- ================================================================================ -->");
	m_pie->write("\n\n");
	m_pie -> indent ();

	return true;
}

void s_DocBook_Listener::_closeFile(void)
{
//	_closeSpan();
//	_closeParagraph();

	_closeChapter ();
	_handleDataItems();

	m_pie -> unindent ();
	m_pie -> writeln("</book>");
}


/*****************************************************************************/
void s_DocBook_Listener :: _closeSection(int sub)
{
	if (sub >= 5)
		return;
	if (! m_bInSection[sub])
		return;

	_closeSection (sub+1);
	_closeSectionTitle (sub);
	_closeParagraph ();

	m_pie -> unindent ();
	m_pie -> writeln ("</section>");
	m_bInSection[sub] = false;
}

void s_DocBook_Listener :: _closeSectionTitle (int sub)
{
	if (sub >= 5)
		/* too many levels of recursion */
		return;

	if (! m_bInSectionTitle[sub])
	{
		/* not in a title */
		/* if no title has been put, then put one */
		if (m_bCanSectionTitle [sub])
			m_pie -> writeln ("<title></title>");
		m_bCanSectionTitle [sub] = false;
		return;
	}

	m_pie -> write ("</title>\n");
	m_bInSectionTitle[sub] = false;
	m_bCanSectionTitle[sub] = false;
}


void s_DocBook_Listener :: _closeParagraph(void)
{
	if (m_bInParagraph)
	{
		if(m_iBlockType == BT_NORMAL)
			m_pie->write("</para>\n");

			else if(m_iBlockType == BT_BLOCKTEXT)
			m_pie->write("</blockquote>\n"); 

//		else if(m_iBlockType >= BT_HEADING1 && m_iBlockType <= BT_HEADING3)
//			m_pie->write("</bridgehead>\n");

			/* this is now handled better */
		else if(m_iBlockType == BT_PLAINTEXT) 
			m_pie->write("</programlisting></para>\n");

			// Add "catchall" for now

		else
			m_pie->write("   oh, oh\n");

		m_bInParagraph = false;
		return;
	}
	else if (m_bInSectionTitle[0])
		_closeSectionTitle (0);
	else if (m_bInSectionTitle[1])
		_closeSectionTitle (1);
	else if (m_bInSectionTitle[2])
		_closeSectionTitle (2);
	else if (m_bInSectionTitle[3])
		_closeSectionTitle (3);
	else if (m_bInSectionTitle[4])
		_closeSectionTitle (4);
	else if (m_bInChapterTitle)
		_closeChapterTitle ();
	// this insures we open/close a section title everytime we need it
	_closeSectionTitle (0);
}

void s_DocBook_Listener :: _closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{
		const XML_Char * szValue;
		
		if (pAP->getProperty((const XML_Char *)"text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("</superscript>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("</subscript>");
			}
		}

		if (
			(pAP->getProperty((const XML_Char *)"font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			m_pie->write("</emphasis>");
		}
		
		if (
			(pAP->getProperty((const XML_Char *)"font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			m_pie->write("</phrase>");
		}
		
		if (pAP->getAttribute((const XML_Char *)"style", szValue))
		{
			if (!UT_strcmp (szValue, "Plain Text"))
			{
				m_pie -> write ("</programlisting></para>\n");
			}
		}


		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}

void s_DocBook_Listener :: _closeList (void)
{
}

void s_DocBook_Listener :: _closeChapter (void)
{
	if (! m_bInChapter)
		return;
	_closeChapterTitle ();
	_closeSection (0);

	m_pie -> unindent ();
	m_pie -> writeln ("</chapter>");
	m_bInChapter = false;
}

void s_DocBook_Listener :: _closeChapterTitle (void)
{
	if (! m_bInChapterTitle)
	{
		/* not in a title */
		/* if no title has been put, then put one */
		if (m_bCanChapterTitle)
			m_pie -> writeln ("<title></title>");
		m_bCanChapterTitle = false;
		return;
	}

	m_pie -> write ("</title>\n");
	m_bInChapterTitle = false;
	m_bCanChapterTitle = false;
}

void s_DocBook_Listener :: _openChapter (PT_AttrPropIndex api)
{
	_closeChapter ();

	m_pie -> writeln ("<chapter>");
	m_pie -> indent ();
	m_bInChapter = true;
	m_bCanChapterTitle = true;
}

void s_DocBook_Listener :: _openChapterTitle (PT_AttrPropIndex api)
{
	if (! (m_bInChapter && m_bCanChapterTitle))
		_openChapter (api);

	m_pie -> iwrite ("<title>");
	m_bInChapterTitle = true;
}

void s_DocBook_Listener :: _openList (PT_AttrPropIndex api)
{
}

void s_DocBook_Listener :: _openParagraph(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);


	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getAttribute((const XML_Char *)PT_STYLE_ATTRIBUTE_NAME, szValue))
		{	
			if(0 == UT_strcmp(szValue, "Heading 1")
				|| 0 == UT_strcmp(szValue, "Numbered Heading 1")) 
			{
				// <p style="Heading 1"> ...
				_closeParagraph ();
				_openSectionTitle(api, 1, (*szValue == 'N'));
				return;
			}

			else if(0 == UT_strcmp(szValue, "Heading 2")
				|| 0 == UT_strcmp(szValue, "Numbered Heading 2")) 
			{

				// <p style="Heading 2"> ...
				_closeParagraph ();
				_openSectionTitle(api, 2, (*szValue == 'N'));
				return;
			}

			else if(0 == UT_strcmp(szValue, "Heading 3")
				|| 0 == UT_strcmp(szValue, "Numbered Heading 3")) 
			{
	
				// <p style="Heading 3"> ...
				_closeParagraph ();
				_openSectionTitle (api, 3, (*szValue == 'N'));
				return;
			}

		    else if(0 == UT_strcmp(szValue, "Block Text"))
			{
				// <p style="Block Text"> ...
				if (! m_bInSection[0])
					_openSection (api,0, false);
				_closeParagraph ();
				m_iBlockType = BT_BLOCKTEXT;
				m_pie->iwrite("<blockquote");
			}

			else if (0 == UT_strcmp (szValue, "Chapter Heading"))
			{
				_closeParagraph ();
				_openChapterTitle (api);
				return;
			}
			
			else if (0 == UT_strcmp (szValue, "Section Heading"))
			{
				_closeParagraph ();
				_openSectionTitle (api, 0, true);
				return;
			}
			else if (0 == UT_strcmp (szValue, "Plain Text"))
			{
				if (! m_bInSection[0])
					_openSection (api,0, false);
				/* merge all plaintexts into 1 if possible */
				if (!m_bInParagraph || !(m_iBlockType == BT_PLAINTEXT))
				{
					_closeParagraph ();
					m_iBlockType = BT_PLAINTEXT;
					m_pie -> write ("<para><programlisting>");
					
					m_bInParagraph = true;
					m_bCanSectionTitle[0] = false;
					m_bCanSectionTitle[1] = false;
					m_bCanSectionTitle[2] = false;
					m_bCanSectionTitle[3] = false;
					m_bCanSectionTitle[4] = false;
					return;
				}
				else
				{
					m_pie -> write ("\n");
					return;
				}
			}
			else
			{
				/* anything unhandled */
				if (! m_bInSection[0])
					_openSection (api,0, false);
				_closeParagraph ();
				m_iBlockType = BT_NORMAL;
				m_pie->iwrite("<para");
			}	
		}
		else 
		{
			// <p> with no style attribute ...
			if (! m_bInSection[0])
				_openSection (api,0, false);
			_closeParagraph ();
			m_iBlockType = BT_NORMAL;
			m_pie->iwrite("<para");
		}

		/* Assumption: never get property set with h1-h3, block text, plain text. Probably true. */

		if (
			m_iBlockType == BT_NORMAL && (pAP->getProperty((const XML_Char *)"text-align", szValue))
			)
		{
// 			m_pie->write(" ALIGN=\"");
// 			m_pie->write(szValue);
// 			m_pie->write("\"");
		}
	}
	else 
	{
		// <p> with no style attribute, and no properties either
		if (! m_bInSection[0])
			_openSection (api,0, false);
		_closeParagraph ();
		m_iBlockType = BT_NORMAL;
	    m_pie->iwrite("<para");
	}
	m_pie -> write (">");
	m_bInParagraph = true;
	m_bCanSectionTitle[0] = false;
	m_bCanSectionTitle[1] = false;
	m_bCanSectionTitle[2] = false;
	m_bCanSectionTitle[3] = false;
	m_bCanSectionTitle[4] = false;
}

void s_DocBook_Listener :: _openSection (PT_AttrPropIndex api, int sub, bool isNum)
{
	if (sub >= 5)
		/* too many levels of recurion */
		return;
	_closeSection (sub);
	if (sub - 1 >= 0)
	{
		if (m_bInSection [sub-1])
		{
			_closeSectionTitle (sub-1);
		}
		else
		{
			_openSection (api, sub-1, false);
			_closeSectionTitle (sub-1);
		}
	}
	else
	{
		if (m_bInChapter)
		{
			_closeChapterTitle ();
		}
		else
		{
			_openChapter (api);
			_closeChapterTitle ();
		}
	}
	
	m_pie -> iwrite ("<section role=\"");
	if (isNum)
		m_pie -> write ("numbered");
	else
		m_pie -> write ("unnumbered");
	m_pie -> write ("\">\n");
	m_pie -> indent ();
	m_bInSection[sub] = true;
	/* possible to insert a title right now */
	m_bCanSectionTitle[sub] = true;

	return;
}

void s_DocBook_Listener :: _openSectionTitle (PT_AttrPropIndex api, int sub, bool isNum)
{
	if (! (m_bInSection[sub] && m_bCanSectionTitle[sub]))
		_openSection (api, sub, isNum);
	m_pie -> iwrite ("<title>");
	m_bInSectionTitle[sub] = true;

	return;
}

void s_DocBook_Listener :: _openSpan(PT_AttrPropIndex api)
{
	if (!
			(m_bInParagraph
			 | m_bInChapterTitle
			 | m_bInSectionTitle[0]
			 | m_bInSectionTitle[1]
			 | m_bInSectionTitle[2]
			 | m_bInSectionTitle[3]
			 | m_bInSectionTitle[4]
			)
		)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP && m_bInParagraph)
	{
		const XML_Char * szValue;

		if ((pAP->getProperty((const XML_Char *)"font-weight", szValue))
			&& !UT_strcmp(szValue, "bold"))
		{
			m_pie->write("<phrase role=\"strong\">");
		}
		
		if ((pAP->getProperty((const XML_Char *)"font-style", szValue))
			&& !UT_strcmp(szValue, "italic"))
		{
			m_pie->write("<emphasis>");
		}
		

		if (pAP->getProperty((const XML_Char *)"text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("<superscript>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("<subscript>");
			}
		}
		
		if (pAP->getAttribute((const XML_Char *)"style", szValue))
		{
			if (!UT_strcmp (szValue, "Plain Text"))
			{
				/* merge all plaintexts into 1 if possible */
				if (!m_bInSpan || !(m_iBlockType == BT_PLAINTEXT))
				{
					m_iBlockType = BT_PLAINTEXT;
					m_pie -> write ("<programlisting>");
					
					m_bInParagraph = true;
					m_bCanSectionTitle[0] = false;
					m_bCanSectionTitle[1] = false;
					m_bCanSectionTitle[2] = false;
					m_bCanSectionTitle[3] = false;
					m_bCanSectionTitle[4] = false;
					return;
				}
				else
				{
					m_pie -> write ("\n");
					return;
				}
			}
		}
		
		m_bInSpan = true;
		m_pAP_Span = pAP;
	}
}
/*****************************************************************************/





/*****************************************************************************/
void s_DocBook_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
		if (!
			(m_bInParagraph
			 | m_bInChapterTitle
			 | m_bInSectionTitle[0]
			 | m_bInSectionTitle[1]
			 | m_bInSectionTitle[2]
			 | m_bInSectionTitle[3]
			 | m_bInSectionTitle[4]
			)
		)
	{
		return;
	}
	m_bWasSpace = true;
	
	UT_String sBuf;
	const UT_UCSChar * pData;

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		switch (*pData)
		{
		case 0x0c:
			// page break
			if (m_bInParagraph)
				// we don't put tags in <title>...
				// TODO: what do we do with page breaks then?
				sBuf += "<beginpage/>";
			pData++;
			break;
		case '<':
			sBuf += "&lt;";
			pData++;
			break;
			
		case '>':
			sBuf += "&gt;";
			pData++;
			break;
			
		case '&':
			sBuf += "&amp;";
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			if (m_iBlockType != BT_PLAINTEXT)
				sBuf += "<br/>";
			else
				sBuf += "\n";
			pData++;
			break;

		case ' ':
		  // try to honor multiple spaces
		  // except in PLAINTEXT
		  if (m_iBlockType != BT_PLAINTEXT)
		  {
		  	if(m_bWasSpace)
				{
					// not defined in dbk, nor in abw...
//					sBuf += "&nbsp;";
					pData++;
				}
			  else
				{
				  // just tack on a single space to the textrun
				  m_bWasSpace = true;
				  sBuf += " ";
				  pData++;
				}
			  break;
		  }
		  else
		  {
			  // plain text: allowing multiple spaces and so
			  sBuf += *pData;
			  pData++;
			  break;
		  }
		case '\t':
		  // try to honor multiple spaces
		  // except in PLAINTEXT
		  if (m_iBlockType != BT_PLAINTEXT)
		  {
		  	if(m_bWasSpace)
				{
					// not defined in dbk, nor in abw...
//					sBuf += "&nbsp;";
					pData++;
				}
			  else
				{
				  // just tack on a single space to the textrun
				  m_bWasSpace = true;
				  sBuf += "\t";
				  pData++;
				}
			  break;
		  }
		  else
		  {
			  // plain text: allowing multiple spaces and so
			  sBuf += *pData;
			  pData++;
			  break;
		  }

		default:

		  // reset this variable
		  m_bWasSpace = false;

			if (*pData > 0x007f)
			{
				if(XAP_EncodingManager::get_instance()->isUnicodeLocale() || 
				   (XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) == 0xa1))

				{
					XML_Char * pszUTF8 = UT_encodeUTF8char(*pData++);
					while (*pszUTF8)
					{
						sBuf += (char)*pszUTF8;
						pszUTF8++;
					}
				}
				else
				{
					/*
					Try to convert to native encoding and if
					character fits into byte, output raw byte. This 
					is somewhat essential for single-byte non-latin
					languages like russian or polish - since
					tools like grep and sed can be used then for
					these files without any problem.
					Networks and mail transfers are 8bit clean
					these days.  - VH
					*/
					UT_UCSChar c = XAP_EncodingManager::get_instance()->try_UToNative(*pData);
					if (c==0 || c>255)
					{
						sBuf += UT_String_sprintf("&#x%x;",*pData++);
					}
					else
					{
						sBuf += (char)c;
						pData++;
					}
				}
			}
			else
			{
				sBuf += (char)*pData++;
			}
			break;
		}
	}

	m_pie->write(sBuf.c_str(),sBuf.size());
}

s_DocBook_Listener::s_DocBook_Listener(PD_Document * pDocument,
										 IE_Exp_DocBook * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection[0] = false;
	m_bInSection[1] = false;
	m_bInSection[2] = false;
	m_bInSection[3] = false;
	m_bInSection[4] = false;
	m_bInSectionTitle[0] = false;
	m_bInSectionTitle[1] = false;
	m_bInSectionTitle[2] = false;
	m_bInSectionTitle[3] = false;
	m_bInSectionTitle[4] = false;
	m_bInParagraph = false;
	m_bInSpan = false;
	m_bInChapter = false;
	m_bInChapterTitle = false;
	m_bCanChapterTitle = false;
	m_bCanSectionTitle[0] = false;
	m_bCanSectionTitle[1] = false;
	m_bCanSectionTitle[2] = false;
	m_bCanSectionTitle[3] = false;
	m_bCanSectionTitle[4] = false;
	m_iListDepth = 0;
	m_iPreviousListDepth = 0;
}

s_DocBook_Listener::~s_DocBook_Listener()
{

}

bool s_DocBook_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								   const PX_ChangeRecord * pcr)
{
	switch (pcr -> getType ())
	{
		case PX_ChangeRecord :: PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			if (api)
			{
				_openSpan(api);
			}
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			if (api)
				_closeSpan();
			return true;
		}
	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			const XML_Char* szValue;
			PT_AttrPropIndex api = pcr->getIndexAP();
			UT_String buf;

			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// Lifted from HTML exporter.

				if(bHaveProp && pAP && pAP->getAttribute("dataid", szValue))
				{
					char* dataid = strdup((char*) szValue);

					m_utvDataIDs.push_back(dataid);

					char * temp = _stripSuffix(UT_basename(szValue), '_');
					char * fstripped = _stripSuffix(temp, '.');
					FREEP(temp);
					UT_String_sprintf(buf, "%s.png", fstripped);
					FREEP(fstripped);
					
					/* va à la ligne */
					m_pie -> write ("\n");
					m_pie -> writeln ("<figure>");
					m_pie -> indent ();
					m_pie -> iwrite ("<title>");
					m_pie -> write (buf);
					m_pie -> write ("</title>\n");
					m_pie -> writeln ("<mediaobject>");
					m_pie -> indent ();
					m_pie -> writeln ("<imageobject>");
					m_pie -> indent ();
                    m_pie -> iwrite("<imagedata fileref=\"");
					m_pie -> write (UT_basename(m_pie->getFileName()));
					m_pie -> write ("_data/");
					m_pie -> write(buf);
					m_pie -> write("\" format=\"PNG\"></imagedata>\n");
					m_pie -> unindent ();
					m_pie -> writeln ("</imageobject>");
					m_pie -> unindent ();
                    m_pie -> writeln ("</mediaobject>");
					m_pie -> unindent ();
					m_pie -> writeln ("</figure>");
					/* remet l'indentation pour le /para */
					m_pie -> iwrite ("");
					
				}
				return true;

			case PTO_Field:
				// we do nothing with computed fields.
				return true;

			case PTO_Hyperlink:
				/* mostly taken from HTML */
				if(bHaveProp && pAP && pAP->getAttribute("xlink:href", szValue))
				{
					m_pie->write("<ulink url=\"");
					m_pie->write(szValue);
					m_pie->write("\">");
				}
				else
				{
					m_pie->write("</ulink>");
				}
				return true;
			
			case PTO_Bookmark:
				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
			return false;
		}


		default:
			/* TODO: changer ca en false sans que ca plante */
			return true;
	}
}

bool s_DocBook_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
			/* new section */
			_openChapter (pcr -> getIndexAP ());
			return true;
		}

		case PTX_SectionHdrFtr:
		{
			/* unhandled */
			return false;
		}

		case PTX_Block:
		{
			_openParagraph (pcr -> getIndexAP ());
			return true;
		}

		default:
			UT_ASSERT(0);
			return false;
	}
}

bool s_DocBook_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_DocBook_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_DocBook_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_DocBook::_writeDocument(void)
{
	m_pListener = new s_DocBook_Listener(getDoc(),this);

	if (!m_pListener)
		return UT_IE_NOMEMORY;
	m_pListener -> _initFile ();
	if (!getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	m_pListener -> _closeFile ();
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

/*!
   removes the suffix from a string by searching backwards for the specified 
   character delimiter. If the delimiter is not found, a copy of the original 
   string is returned
   
   eg. _stripSuffix("/home/user/file.png, '.') returns "/home/user/file" 
	   _stripSuffix("/home/user/foo_bar, '_') returns /home/user/foo 
	   _stripSuffix("/home/user/file.png, '_') returns /home/user/file.png"
   TODO: put this in UT_String somehow, it came from ie_exp_HTML.
*/
char *s_DocBook_Listener::_stripSuffix(const char* from, char delimiter)
{
	char * fremove_s = (char *)malloc(strlen(from)+1);
	strcpy(fremove_s, from);   

	char * p = fremove_s + strlen(fremove_s);
	while ((p >= fremove_s) && (*p != delimiter))
		p--;
	
	if (p >= fremove_s)
	*p = '\0';
	
	return fremove_s;
}

void s_DocBook_Listener::_handleDataItems(void)
{
	// Lifted from HTML listener
 	const char * szName;
	const char * szMimeType;
	const UT_ByteBuf * pByteBuf;
	
	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,(void**)&szMimeType)); k++)
	{
		UT_sint32 loc = -1;
		for (UT_uint32 i = 0; i < m_utvDataIDs.getItemCount(); i++)
		{
			if(UT_strcmp((char*) m_utvDataIDs[i], szName) == 0)
			{
				loc = i;
				break;
			}
		}
		
		if(loc > -1)
		{
			FILE *fp;
			UT_String fname; // EVIL EVIL bad hardcoded buffer size
			
			UT_String_sprintf(fname, "%s_data", m_pie->getFileName());
			/* int result = */
			m_pDocument->getApp()->makeDirectory(fname.c_str(), 0750);
			
			if (!UT_strcmp(szMimeType, "image/svg-xml"))
				UT_String_sprintf(fname, "%s/%s_%d.svg", fname.c_str(), szName, loc);
			if (!UT_strcmp(szMimeType, "text/mathml"))
				UT_String_sprintf(fname, "%s/%s_%d.mathml", fname.c_str(), szName, loc);
			else // PNG Image
			{  
				char * temp = _stripSuffix(UT_basename(szName), '_');
				char * fstripped = _stripSuffix(temp, '.');
				FREEP(temp);
				UT_String_sprintf(fname, "%s/%s.png", fname.c_str(), fstripped);
				FREEP(fstripped);
			}
			
			if (!UT_isRegularFile(fname.c_str()))
			{
				fp = fopen (fname.c_str(), "wb+");
			
				if(!fp)
					continue;
			
				int cnt = 0, len = pByteBuf->getLength();
			
				while (cnt < len)
				{
					cnt += fwrite (pByteBuf->getPointer(cnt), 
								 sizeof(UT_Byte), len-cnt, fp);
				}
			
				fclose(fp);
			}
		}
	}
	
	return;
}
