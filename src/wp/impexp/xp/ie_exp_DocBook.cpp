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

/*****************************************************************/
/*****************************************************************/

IE_Exp_DocBook::IE_Exp_DocBook(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_DocBook::~IE_Exp_DocBook()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_DocBook::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".dbk") == 0);
}

UT_Error IE_Exp_DocBook::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_DocBook * p = new IE_Exp_DocBook(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_DocBook::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "DocBook (.dbk)";
	*pszSuffixList = "*.dbk";
	*ft = IEFT_DocBook;
	return UT_TRUE;
}

UT_Bool IE_Exp_DocBook::SupportsFileType(IEFileType ft)
{
	return (IEFT_DocBook == ft);
}

/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	        5
#define BT_PLAINTEXT	        6

class s_DocBook_Listener : public PL_Listener
{
public:
	s_DocBook_Listener(PD_Document * pDocument,
						IE_Exp_DocBook * pie);
	virtual ~s_DocBook_Listener();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual UT_Bool		signal(UT_uint32 iSignal);

protected:
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openParagraph(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_handleDataItems(void);
	void				_convertFontSize(char* szDest, const char* pszFontSize);
	void				_convertColor(char* szDest, const char* pszColor);
	
	PD_Document *		m_pDocument;
	IE_Exp_DocBook *		m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*
        UT_Bool                 m_bWasSpace;

};

void s_DocBook_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_pie->write("</chapter>\n");
	m_bInSection = UT_FALSE;
	return;
}

void s_DocBook_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	if(m_iBlockType == BT_NORMAL)
		m_pie->write("</para>\n");

        else if(m_iBlockType == BT_BLOCKTEXT)
		  m_pie->write("</blockquote>\n"); 

	else if(m_iBlockType == BT_PLAINTEXT) 
		m_pie->write("</para>\n");

        // Add "catchall" for now

	else
		m_pie->write("   oh, oh\n");

	m_bInBlock = UT_FALSE;
	return;
}

void s_DocBook_Listener::_openParagraph(PT_AttrPropIndex api)
{
	if (!m_bInSection)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getAttribute((const XML_Char *)"style", szValue))
		{
			
		       	if(0 == UT_stricmp(szValue, "Block Text"))
			{
				// <p style="Block Text"> ...

				m_iBlockType = BT_BLOCKTEXT;
				m_pie->write("<blockquote\n");
			}

			else 
			{

				// <p style="<anything else!>"> ...

				m_iBlockType = BT_NORMAL;
				m_pie->write("<para");
			}	
		}
		else 
		{

			// <p> with no style attribute ...

			m_iBlockType = BT_NORMAL;
			m_pie->write("<para");
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

		m_iBlockType = BT_NORMAL;
	       	m_pie->write("<para");
	}

		m_pie->write(">");

	m_bInBlock = UT_TRUE;
}

void s_DocBook_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
	m_pie->write("<chapter>\n<title></title>\n");
}



void s_DocBook_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if ((pAP->getProperty((const XML_Char *)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold"))
		{
			m_pie->write("<phrase role=\"strong\">");
		}
		
		if ((pAP->getProperty((const XML_Char *)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic"))
		{
			m_pie->write("<emphasis>");
		}
		

		if (pAP->getProperty((const XML_Char *)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("<superscript>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("<subscript>");
			}
		}
		
		
		m_bInSpan = UT_TRUE;
		m_pAP_Span = pAP;
	}
}

void s_DocBook_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{
		const XML_Char * szValue;
		
		if (pAP->getProperty((const XML_Char *)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("</superscript>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("</subscript>");
			}
		}

		if (
			(pAP->getProperty((const XML_Char *)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
			m_pie->write("</emphasis>");
		}
		
		if (
			(pAP->getProperty((const XML_Char *)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
			m_pie->write("</phrase>");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = UT_FALSE;
	return;
}

void s_DocBook_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (!m_bInBlock)
	{
		return;
	}
	
	// TODO deal with unicode.
	// TODO for now, just squish it into ascii.
	
#define MY_BUFFER_SIZE		1024
#define MY_HIGHWATER_MARK	20
	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
		{
			m_pie->write(buf,(pBuf-buf));
			pBuf = buf;
		}

		switch (*pData)
		{
		case '<':
			*pBuf++ = '&';
			*pBuf++ = 'l';
			*pBuf++ = 't';
			*pBuf++ = ';';
			pData++;
			break;
			
		case '>':
			*pBuf++ = '&';
			*pBuf++ = 'g';
			*pBuf++ = 't';
			*pBuf++ = ';';
			pData++;
			break;
			
		case '&':
			*pBuf++ = '&';
			*pBuf++ = 'a';
			*pBuf++ = 'm';
			*pBuf++ = 'p';
			*pBuf++ = ';';
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			*pBuf++ = '<';				// these get mapped to <br/>
			*pBuf++ = 'b';
			*pBuf++ = 'r';
			*pBuf++ = '/';
			*pBuf++ = '>';
			pData++;
			break;

		case ' ':
		case '\t':
		  // try to honor multiple spaces
		  // tabs get treated as a single space
		  //
		  if(m_bWasSpace)
		    {
		      *pBuf++ = '&';
		      *pBuf++ = 'n';
		      *pBuf++ = 'b';
		      *pBuf++ = 's';
		      *pBuf++ = 'p';
		      *pBuf++ = ';';
		      pData++;
		    }
		  else
		    {
		      // just tack on a single space to the textrun
		      m_bWasSpace = UT_TRUE;
		      *pBuf++ = ' ';
		      pData++;
		    }
		  break;

		default:

		  // reset this variable
		  m_bWasSpace = UT_FALSE;

		  // thanks for the international patch, vlad :-)
		  if (*pData > 0x007f)
		    {
#if 1
#	if 0
				// convert non us-ascii into numeric entities.
				// this has the advantage that our file format is
				// 7bit clean and safe for email and other network
				// transfers....
				char localBuf[20];
				char * plocal = localBuf;
				sprintf(localBuf,"&#x%x;",*pData++);
				while (*plocal)
					*pBuf++ = (UT_Byte)*plocal++;
#	else
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
				UT_UCSChar c = XAP_EncodingManager::instance->try_UToNative(*pData);
				if (c==0 || c>255)
				{
					char localBuf[20];
					char * plocal = localBuf;
					sprintf(localBuf,"&#x%x;",*pData++);
					while (*plocal)
						*pBuf++ = (UT_Byte)*plocal++;
				}
				else
				{
					*pBuf++ = (UT_Byte)c;
					pData++;
				}
#	endif
#else
				// convert to UTF8
				// TODO if we choose this, do we have to put the ISO header in
				// TODO like we did for the strings files.... i hesitate to
				// TODO make such a change to our file format.
				XML_Char * pszUTF8 = UT_encodeUTF8char(*pData);
				while (*pszUTF8)
				{
					*pBuf++ = (UT_Byte)*pszUTF8;
					pszUTF8++;
				}
#endif
			}
			else
			{
				*pBuf++ = (UT_Byte)*pData++;
			}
			break;
		}
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

s_DocBook_Listener::s_DocBook_Listener(PD_Document * pDocument,
										 IE_Exp_DocBook * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = UT_FALSE;
	m_bInBlock = UT_FALSE;
	m_bInSpan = UT_FALSE;
	

	// write out the doctype descriptor
	m_pie->write("<!DOCTYPE book PUBLIC \"-//OASIS//DTD DocBook XML V4.1.2//EN\"\n");
        m_pie->write("\t\"http://www.oasis-open.org/docbook/xml/4.0/docbookx.dtd\">\n\n");

	m_pie->write("<!-- ================================================================================ -->\n");
	m_pie->write("<!-- This DocBook file was created by AbiWord.                                        -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor.                                   -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com               -->\n");
	m_pie->write("<!-- ================================================================================ -->\n");
	m_pie->write("\n");

	if (XAP_App::s_szBuild_ID && XAP_App::s_szBuild_ID[0])
	{
		m_pie->write("<!--         Build_ID          = ");
		m_pie->write(XAP_App::s_szBuild_ID);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
	{
		m_pie->write("<!--         Build_Version     = ");
		m_pie->write(XAP_App::s_szBuild_Version);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Options && XAP_App::s_szBuild_Options[0])
	{
		m_pie->write("<!--         Build_Options     = ");
		m_pie->write(XAP_App::s_szBuild_Options);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Target && XAP_App::s_szBuild_Target[0])
	{
		m_pie->write("<!--         Build_Target      = ");
		m_pie->write(XAP_App::s_szBuild_Target);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_CompileTime && XAP_App::s_szBuild_CompileTime[0])
	{
		m_pie->write("<!--         Build_CompileTime = ");
		m_pie->write(XAP_App::s_szBuild_CompileTime);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_CompileDate && XAP_App::s_szBuild_CompileDate[0])
	{
		m_pie->write("<!--         Build_CompileDate = ");
		m_pie->write(XAP_App::s_szBuild_CompileDate);
		m_pie->write(" -->\n");
	}
	
	m_pie->write("\n");
	
	m_pie->write("<book>");
	m_pie->write("\n");
}

s_DocBook_Listener::~s_DocBook_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();
	
	m_pie->write("</book>\n");
}

UT_Bool s_DocBook_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								   const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
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
			return UT_TRUE;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// TODO we *could* insert the images and create separate GIF files.
				return UT_TRUE;

			case PTO_Field:
				// we do nothing with computed fields.
				return UT_TRUE;

			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
#else
			return UT_TRUE;
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return UT_TRUE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_DocBook_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
		_closeSpan();
		_closeBlock();
		_closeSection();

		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDocument->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute((const XML_Char *)"type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_stricmp(pszSectionType, "doc"))
				)
			{
				_openSection(pcr->getIndexAP());
				m_bInSection = UT_TRUE;
			}
			else
			{
				m_bInSection = UT_FALSE;
			}
		}
		else
		{
			m_bInSection = UT_FALSE;
		}
		
		return UT_TRUE;
	}

	case PTX_Block:
	{
		_closeSpan();
		_closeBlock();
		_openParagraph(pcr->getIndexAP());
		return UT_TRUE;
	}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_DocBook_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_DocBook_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_DocBook_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_DocBook::_writeDocument(void)
{
	m_pListener = new s_DocBook_Listener(m_pDocument,this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_DocBook_Listener::_handleDataItems(void)
{
}








