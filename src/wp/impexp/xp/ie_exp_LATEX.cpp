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
#include "ie_exp_LATEX.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_LATEX::IE_Exp_LATEX(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_LATEX::~IE_Exp_LATEX()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_LATEX::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".latex") == 0);
}

IEStatus IE_Exp_LATEX::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_LATEX * p = new IE_Exp_LATEX(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Exp_LATEX::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "LaTeX (.latex)";
	*pszSuffixList = "*.latex";
	*ft = IEFT_LATEX;
	return UT_TRUE;
}

UT_Bool IE_Exp_LATEX::SupportsFileType(IEFileType ft)
{
	return (IEFT_LATEX == ft);
}

/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6

class s_LATEX_Listener : public PL_Listener
{
public:
	s_LATEX_Listener(PD_Document * pDocument,
						IE_Exp_LATEX * pie);
	virtual ~s_LATEX_Listener();

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
	IE_Exp_LATEX *		m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*

};

void s_LATEX_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_pie->write("%% End of section\n");
	m_bInSection = UT_FALSE;
	return;
}

void s_LATEX_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	if(m_iBlockType == BT_NORMAL)
		m_pie->write("\n\n");

        else if(m_iBlockType == BT_HEADING1)
		m_pie->write("}\n");

        else if(m_iBlockType == BT_HEADING2)
		m_pie->write("}\n");

        else if(m_iBlockType == BT_HEADING3)
		m_pie->write("}\n");

        else if(m_iBlockType == BT_BLOCKTEXT)
		  m_pie->write("\n\\end{quote}\n"); // It's not correct, but I'll leave it by now...

	else if(m_iBlockType == BT_PLAINTEXT) // it's \texttt{ ??
		m_pie->write("\n");

        // Add "catchall" for now

	else
		m_pie->write("%% oh, oh\n");

	m_bInBlock = UT_FALSE;
	return;
}

void s_LATEX_Listener::_openParagraph(PT_AttrPropIndex api)
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

		if (pAP->getAttribute("style", szValue))
		{
			
			if(0 == UT_stricmp(szValue, "Heading 1")) 
			{

				// <p style="Heading 1"> ...

				m_iBlockType = BT_HEADING1;
				m_pie->write("\\section{");
			}
			else if(0 == UT_stricmp(szValue, "Heading 2")) 
			{

				// <p style="Heading 2"> ...

				m_iBlockType = BT_HEADING2;
				m_pie->write("\\subsection{");
			}
			else if(0 == UT_stricmp(szValue, "Heading 3")) 
			{
	
				// <p style="Heading 3"> ...

				m_iBlockType = BT_HEADING3;
				m_pie->write("\\subsubsection{");
			}
			else if(0 == UT_stricmp(szValue, "Block Text"))
			{
				// <p style="Block Text"> ...

				m_iBlockType = BT_BLOCKTEXT;
				m_pie->write("\\begin{quote}\n");
			}
			else if(0 == UT_stricmp(szValue, "Plain Text"))
			{
				// <p style="Plain Text"> ...

				m_iBlockType = BT_PLAINTEXT;
				m_pie->write("%% Plain text");
			}
			else 
			{

				// <p style="<anything else!>"> ...

				m_iBlockType = BT_NORMAL;
				m_pie->write("\n");
			}	
		}
		else 
		{

			// <p> with no style attribute ...

			m_iBlockType = BT_NORMAL;
			m_pie->write("\n");
		}

		/* Assumption: never get property set with h1-h3, block text, plain text. Probably true. */

		if (
			m_iBlockType == BT_NORMAL && (pAP->getProperty("text-align", szValue))
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
		//		m_pie->write("<p");
	}

	//	m_pie->write(">");

	m_bInBlock = UT_TRUE;
}

void s_LATEX_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
	m_pie->write("%% New AbiWord Section\n");
}

void s_LATEX_Listener::_convertColor(char* szDest, const char* pszColor)
{
	/*
	  TODO we might want to be a little more careful about this.
	  The proper LATEX color is #rrggbb, which is basically the same
	  as what we use this.  HTML browsers are likely to be more
	  forgiving than we are, so this is probably not a big
	  problem.
	*/
	strcpy(szDest, pszColor);
}

void s_LATEX_Listener::_convertFontSize(char* szDest, const char* pszFontSize)
{
	double fSizeInPoints = UT_convertToPoints(pszFontSize);

	/*
	  TODO we can probably come up with a mapping of font sizes that
	  is more accurate than the code below.  I just guessed.
	*/
	
	if (fSizeInPoints <= 7)
	{
		strcpy(szDest, "1");
	}
	else if (fSizeInPoints <= 10)
	{
		strcpy(szDest, "2");
	}
	else if (fSizeInPoints <= 12)
	{
		strcpy(szDest, "3");
	}
	else if (fSizeInPoints <= 16)
	{
		strcpy(szDest, "4");
	}
	else if (fSizeInPoints <= 24)
	{
		strcpy(szDest, "5");
	}
	else if (fSizeInPoints <= 36)
	{
		strcpy(szDest, "6");
	}
	else
	{
		strcpy(szDest, "7");
	}
}

/*
  Note that I've gone to lots of trouble to make sure
  that the HTML formatting tags are properly nested.
  The properties/tags are checked in the exact opposite
  order in closeSpan as they are in openSpan.  I guess
  what I SHOULD have done is write them out haphazardly,
  since most web browsers have to be able to handle that
  kind of &^%#&^ anyway.  But if I did that, someone
  might get the impression that I'm still holding a grudge
  or something.  :-)	--EWS
*/

void s_LATEX_Listener::_openSpan(PT_AttrPropIndex api)
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

		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
			m_pie->write("<b>");
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
			m_pie->write("{em");
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;

			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");

			while (q)
			{
				if (0 == UT_stricmp(q, "underline"))
				{
					m_pie->write("<u>");
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");

			while (q)
			{
				if (0 == UT_stricmp(q, "line-through"))
				{
					m_pie->write("<s>");	// is it <s> or <strike> ? TODO
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("<sup>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("<sub>");
			}
		}
		
		if (
			(pAP->getProperty("color", szValue))
		    || (pAP->getProperty("font-size", szValue))
		    || (pAP->getProperty("font-family", szValue))
			)
		{
			const XML_Char* pszColor = NULL;
			const XML_Char* pszFontSize = NULL;
			const XML_Char* pszFontFamily = NULL;

			pAP->getProperty("color", pszColor);
		    pAP->getProperty("font-size", pszFontSize);
		    pAP->getProperty("font-family", pszFontFamily);

			m_pie->write("<font");
			if (pszColor)
			{
				m_pie->write(" COLOR=\"");
				char szColor[16];
				_convertColor(szColor, pszColor);
				m_pie->write(szColor);
				m_pie->write("\"");
			}
			
			if (pszFontFamily)
			{
				m_pie->write(" FACE=\"");
				m_pie->write(pszFontFamily);
				m_pie->write("\"");
			}
			
			if (pszFontSize)
			{
				m_pie->write(" SIZE=\"");
				char szSize[16];
				_convertFontSize(szSize, pszFontSize);
				m_pie->write(szSize);
				m_pie->write("\"");
			}

			m_pie->write(">");
		}
		
		m_bInSpan = UT_TRUE;
		m_pAP_Span = pAP;
	}
}

void s_LATEX_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{
		const XML_Char * szValue;
		
		if (
			(pAP->getProperty("color", szValue))
		    || (pAP->getProperty("font-size", szValue))
		    || (pAP->getProperty("font-family", szValue))
			)
		{
			m_pie->write("</font>");
		}

		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("</sup>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("</sub>");
			}
		}

		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");

			while (q)
			{
				if (0 == UT_stricmp(q, "line-through"))
				{
					m_pie->write("</s>");	// is it <s> or <strike> ? TODO
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");

			while (q)
			{
				if (0 == UT_stricmp(q, "underline"))
				{
					m_pie->write("</u>");
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
			m_pie->write("</i>");
		}
		
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
			m_pie->write("</b>");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = UT_FALSE;
	return;
}

void s_LATEX_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
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

		UT_ASSERT(*pData < 256);
		switch (*pData)
		{
		case '\\':
			*pBuf++ = '$';
			*pBuf++ = '\\';
			*pBuf++ = 'b';
			*pBuf++ = 'a';
			*pBuf++ = 'c';
			*pBuf++ = 'k';
			*pBuf++ = 's';
			*pBuf++ = 'l';
			*pBuf++ = 'a';
			*pBuf++ = 's';
			*pBuf++ = 'h';
			*pBuf++ = '$';
			pData++;
			break;
			
		case '$':
			*pBuf++ = '\\';
			*pBuf++ = '$';
			pData++;
			break;

		case '%':
			*pBuf++ = '\\';
			*pBuf++ = '%';
			pData++;
			break;
			
		case '&':
			*pBuf++ = '\\';
			*pBuf++ = '&';
			pData++;
			break;

		case '#':
			*pBuf++ = '\\';
			*pBuf++ = '#';
			pData++;
			break;

		case '_':
			*pBuf++ = '\\';
			*pBuf++ = '_';
			pData++;
			break;

		case '{':
			*pBuf++ = '\\';
			*pBuf++ = '{';
			pData++;
			break;

		case '}':
			*pBuf++ = '\\';
			*pBuf++ = '}';
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			*pBuf++ = '\\';
			*pBuf++ = '\\';
			pData++;
			break;
			
		default:
			*pBuf++ = (UT_Byte)*pData++;
			break;
		}
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

s_LATEX_Listener::s_LATEX_Listener(PD_Document * pDocument,
										 IE_Exp_LATEX * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = UT_FALSE;
	m_bInBlock = UT_FALSE;
	m_bInSpan = UT_FALSE;
	
	m_pie->write("%% ================================================================================\n");
	m_pie->write("%% This LaTeX file was created by AbiWord.                                         \n");
	m_pie->write("%% AbiWord is a free, Open Source word processor.                                  \n");
	m_pie->write("%% You may obtain more information about AbiWord at www.abisource.com              \n");
	m_pie->write("%% ================================================================================\n");
	m_pie->write("\n");

	if (XAP_App::s_szBuild_ID && XAP_App::s_szBuild_ID[0])
	{
		m_pie->write("%%         Build_ID          = ");
		m_pie->write(XAP_App::s_szBuild_ID);
		m_pie->write("\n");
	}
	if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
	{
		m_pie->write("%%         Build_Version     = ");
		m_pie->write(XAP_App::s_szBuild_Version);
		m_pie->write("\n");
	}
	if (XAP_App::s_szBuild_Options && XAP_App::s_szBuild_Options[0])
	{
		m_pie->write("%%         Build_Options     = ");
		m_pie->write(XAP_App::s_szBuild_Options);
		m_pie->write("\n");
	}
	if (XAP_App::s_szBuild_Target && XAP_App::s_szBuild_Target[0])
	{
		m_pie->write("%%         Build_Target      = ");
		m_pie->write(XAP_App::s_szBuild_Target);
		m_pie->write("\n");
	}
	if (XAP_App::s_szBuild_CompileTime && XAP_App::s_szBuild_CompileTime[0])
	{
		m_pie->write("%%         Build_CompileTime = ");
		m_pie->write(XAP_App::s_szBuild_CompileTime);
		m_pie->write("\n");
	}
	if (XAP_App::s_szBuild_CompileDate && XAP_App::s_szBuild_CompileDate[0])
	{
		m_pie->write("%%         Build_CompileDate = ");
		m_pie->write(XAP_App::s_szBuild_CompileDate);
		m_pie->write("\n");
	}
	
	m_pie->write("\n");
	
	m_pie->write("\\documentclass[12pt]{article}\n");
	m_pie->write("\\usepackage[T1]{fontenc}\n");
	m_pie->write("\n");
	m_pie->write("\\begin{document}\n");
	m_pie->write("\n");
}

s_LATEX_Listener::~s_LATEX_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();
	
	m_pie->write("\\end{document}\n");
}

UT_Bool s_LATEX_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_LATEX_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
			pAP->getAttribute("type", pszSectionType);
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

UT_Bool s_LATEX_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_LATEX_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_LATEX_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}


/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_LATEX::_writeDocument(void)
{
	m_pListener = new s_LATEX_Listener(m_pDocument,this);
	if (!m_pListener)
		return IES_NoMemory;
	if (!m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return IES_Error;
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_LATEX_Listener::_handleDataItems(void)
{
}

