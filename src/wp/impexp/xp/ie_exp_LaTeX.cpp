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
#include "ie_exp_LaTeX.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"

/*****************************************************************/
/*****************************************************************/

#define DEFAULT_SIZE "12pt"

enum JustificationTypes {
	JUSTIFIED,
	CENTER,
	RIGHT,
	LEFT
};

IE_Exp_LaTeX::IE_Exp_LaTeX(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_LaTeX::~IE_Exp_LaTeX()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_LaTeX::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".latex") == 0);
}

UT_Error IE_Exp_LaTeX::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_LaTeX * p = new IE_Exp_LaTeX(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_LaTeX::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "LaTeX (.latex)";
	*pszSuffixList = "*.latex";
	*ft = IEFT_LaTeX;
	return UT_TRUE;
}

UT_Bool IE_Exp_LaTeX::SupportsFileType(IEFileType ft)
{
	return (IEFT_LaTeX == ft);
}

/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6

class s_LaTeX_Listener : public PL_Listener
{
public:
	s_LaTeX_Listener(PD_Document * pDocument,
						IE_Exp_LaTeX * pie);
	virtual ~s_LaTeX_Listener();

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
	IE_Exp_LaTeX *		m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;
	UT_Bool             m_bMultiCols;
	JustificationTypes  m_eJustification;
	UT_Bool				m_bLineHeight;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*

};

void s_LaTeX_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}

	if (m_bMultiCols)
	{
		m_pie->write("\\end{multicols}\n");
		m_bMultiCols = UT_FALSE;
	}

	m_bInSection = UT_FALSE;
	return;
}

void s_LaTeX_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	switch (m_iBlockType)
	{
	case BT_NORMAL:
		switch (m_eJustification)
		{
		case JUSTIFIED:
			break;
		case CENTER:
			m_pie->write("\n\\end{center}");
			break;
		case RIGHT:
			m_pie->write("\n\\end{flushright}");
			break;
		case LEFT:
			m_pie->write("\n\\end{flushleft}");
			break;
		}

		if (m_bLineHeight)
		  m_pie->write("\n\\end{spacing}");

		m_pie->write("\n\n");
		break;
	case BT_HEADING1:
	case BT_HEADING2:
	case BT_HEADING3:
		m_pie->write("}\n");
		break;
	case BT_BLOCKTEXT:
		m_pie->write("\n\\end{quote}\n"); // It's not correct, but I'll leave it by now...
		break;
	case BT_PLAINTEXT:
		m_pie->write("}\n");
		break;
	default:
		m_pie->write("%% oh, oh\n");
	}

	m_bInBlock = UT_FALSE;
	return;
}

void s_LaTeX_Listener::_openParagraph(PT_AttrPropIndex api)
{
	m_eJustification = JUSTIFIED;
	m_bLineHeight = UT_FALSE;

	if (!m_bInSection)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getAttribute((XML_Char*)"style", szValue))
		{
			
			if(0 == UT_stricmp(szValue, "Heading 1")) 
			{
				m_iBlockType = BT_HEADING1;
				m_pie->write("\\section{");
			}
			else if(0 == UT_stricmp(szValue, "Heading 2")) 
			{
				m_iBlockType = BT_HEADING2;
				m_pie->write("\\subsection{");
			}
			else if(0 == UT_stricmp(szValue, "Heading 3")) 
			{
				m_iBlockType = BT_HEADING3;
				m_pie->write("\\subsubsection{");
			}
			else if(0 == UT_stricmp(szValue, "Block Text"))
			{
				m_iBlockType = BT_BLOCKTEXT;
				m_pie->write("\\begin{quote}\n");
			}
			else if(0 == UT_stricmp(szValue, "Plain Text"))
			{
				m_iBlockType = BT_PLAINTEXT;
				m_pie->write("\\texttt{");
			}
			else 
			{
				m_iBlockType = BT_NORMAL;
			}	
		}
		else 
		{
			m_iBlockType = BT_NORMAL;
		}
		
		/* Assumption: never get property set with h1-h3, block text, plain text. Probably true. */
		// TODO: Split this function in several ones
		if (m_iBlockType == BT_NORMAL)
		{
			if (pAP->getProperty((XML_Char*)"text-align", szValue))
			{
				if (0 == UT_stricmp(szValue, "center"))
				{
					m_pie->write("\\begin{center}\n");
					m_eJustification = CENTER;
				}
				if (0 == UT_stricmp(szValue, "right"))
				{
					m_pie->write("\\begin{flushright}\n");
					m_eJustification = RIGHT;
				}
				if (0 == UT_stricmp(szValue, "left"))
				{
					m_pie->write("\\begin{flushleft}\n");
					m_eJustification = LEFT;
				}
			}

			if (pAP->getProperty((XML_Char*)"line-height", szValue))
			{
				if (0 != UT_stricmp(szValue, "1.0"))
				{
					m_pie->write("\\begin{spacing}{");
					m_bLineHeight = UT_TRUE;
				}
				if (0 == UT_stricmp(szValue, "1.5"))
					m_pie->write("1.24}\n");
				else if (0 == UT_stricmp(szValue, "2.0"))
					m_pie->write("1.66}\n");
				else // glup.  TODO: calculate the spacing :)
				    m_pie->write("1.0} % Sorry.  I know that you don't expected the 1.0... feel free to fix it! :)\n");
			}
		}
	}
	else 
	{
		m_iBlockType = BT_NORMAL;
	}
	
	m_bInBlock = UT_TRUE;
}

void s_LaTeX_Listener::_openSection(PT_AttrPropIndex api)
{
	const PP_AttrProp* pAP = NULL;
	static UT_Bool firstSection = UT_TRUE;

	m_bMultiCols = UT_FALSE;

	if (m_pDocument->getAttrProp(api, &pAP) && pAP)
	{
		const XML_Char* pszNbCols = NULL;
		const XML_Char* pszPageMarginLeft = NULL;
		const XML_Char* pszPageMarginRight = NULL;

		pAP->getProperty((XML_Char*)"columns", pszNbCols);
		pAP->getProperty((XML_Char*)"page-margin-right", pszPageMarginLeft);
		pAP->getProperty((XML_Char*)"page-margin-left", pszPageMarginRight);

		if (pszNbCols != NULL && ((0 == UT_stricmp(pszNbCols, "2"))
								  || (0 == UT_stricmp(pszNbCols, "3"))))
		{
			m_pie->write("\\begin{multicols}{");
			m_pie->write((char*)pszNbCols);
			m_pie->write("}\n");
			m_bMultiCols = UT_TRUE;
		}
		if (pszPageMarginLeft != NULL)
		{
			m_pie->write("\\setlength{\\oddsidemargin}{");
			m_pie->write((char*)pszPageMarginLeft);
			m_pie->write("-1in");
			m_pie->write("}\n");
		}
		if (pszPageMarginRight != NULL)
		{
			m_pie->write("\\setlength{\\textwidth}{\\paperwidth - ");
			m_pie->write((char*)pszPageMarginRight);
			m_pie->write("-");
			m_pie->write((char*)pszPageMarginLeft);
			m_pie->write("}\n");
		}
	}

	if (firstSection)
	{
		m_pie->write ("\n\n\\begin{document}\n");
		firstSection = UT_FALSE;
	}
}

void s_LaTeX_Listener::_convertColor(char* szDest, const char* pszColor)
{
	/*
	  TODO I've no clue about the colors in LaTeX
	*/
	strcpy(szDest, pszColor);
}

void s_LaTeX_Listener::_convertFontSize(char* szDest, const char* pszFontSize)
{
	double fSizeInPoints = UT_convertToPoints(pszFontSize);

	if (fSizeInPoints <= 6)
	{
		strcpy(szDest, "tiny");
	}
	else if (fSizeInPoints <= 8)
	{
		strcpy(szDest, "scriptsize");
	}
	else if (fSizeInPoints <= 10)
	{
		strcpy(szDest, "footnotesize");
	}
	else if (fSizeInPoints <= 11)
	{
		strcpy(szDest, "small");
	}
	else if (fSizeInPoints <= 12)
	{
		strcpy(szDest, "normalsize");
	}
	else if (fSizeInPoints <= 14)
	{
		strcpy(szDest, "large");
	}
	else if (fSizeInPoints <= 17)
	{
		strcpy(szDest, "Large");
	}
	else if (fSizeInPoints <= 20)
	{
		strcpy(szDest, "LARGE");
	}
	else if (fSizeInPoints <= 25)
	{
		strcpy(szDest, "huge");
	}
	else
	{
		strcpy(szDest, "Huge");
	}
}

void s_LaTeX_Listener::_openSpan(PT_AttrPropIndex api)
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
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
			m_pie->write("\\textbf{");
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
			m_pie->write("\\textit{");
		}
		
		if (pAP->getProperty((XML_Char*)"text-decoration", szValue))
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
			  if (0 == UT_stricmp(q, "underline")) // TODO: \def\undertext#1{$\underline{\vphantom{y}\smash{\hbox{#1}}}$}
				{
					m_pie->write("\\underline{");
				}

				if (0 == UT_stricmp(q, "overline"))
				{
					m_pie->write("$\\overline{\\textrm{");
				}

				if (0 == UT_stricmp(q, "line-through"))
				{
					m_pie->write("");	// TODO
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (pAP->getProperty((XML_Char*)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("$^{\\rm{}"); // TODO: Finish it
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("$_{\\rm{}"); // TODO: Finish it
			}
		}
		
		if (pAP->getProperty((XML_Char*)"color", szValue))
			; // TODO

		if (pAP->getProperty((XML_Char*)"font-size", szValue))
		{
			if (strcmp (szValue, DEFAULT_SIZE) != 0)
			{
				m_pie->write("{\\");
				char szSize[16];
				_convertFontSize(szSize, (char*)szValue);
				m_pie->write(szSize);
				m_pie->write("{}");
			}
		}
		
		if (pAP->getProperty((XML_Char*)"font-family", szValue))
			; // TODO

		m_bInSpan = UT_TRUE;
		m_pAP_Span = pAP;
	}
}

void s_LaTeX_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{
		const XML_Char * szValue;
		
		if (// (pAP->getProperty((XML_Char*)"color", szValue)) ||    // TODO
		    (pAP->getProperty((XML_Char*)"font-size", szValue))
//		    || (pAP->getProperty((XML_Char*)"font-family", szValue))  // TODO
			)
		{
			if (strcmp (szValue, DEFAULT_SIZE) != 0)
				m_pie->write("}");
		}

		if (pAP->getProperty((XML_Char*)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("}$"); // TODO
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("}$"); // TODO
			}
		}

		if (
			(pAP->getProperty((XML_Char*)"text-decoration", szValue))
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
					m_pie->write("");	// TODO
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty((XML_Char*)"text-decoration", szValue))
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
					m_pie->write("}");
				}

				if (0 == UT_stricmp(q, "overline"))
				{
					m_pie->write("}}$");
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
			m_pie->write("}");
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
			m_pie->write("}");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = UT_FALSE;
	return;
}

void s_LaTeX_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
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

	for (pBuf = buf, pData = data; (pData < data + length); /**/)
	{
		if (pBuf >= (buf + MY_BUFFER_SIZE - MY_HIGHWATER_MARK))
		{
			m_pie->write (buf, (pBuf - buf));
			pBuf = buf;
		}

		switch (*pData)
		{
		case '\\':
			strncpy (pBuf, "\\ensuremath{\\backslash}}", MY_BUFFER_SIZE);
			pBuf += strlen ("\\ensuremath{\\backslash}}");
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

		case '~':
			*pBuf++ = '\\';
			*pBuf++ = '~';
			*pBuf++ = '{';
			*pBuf++ = '}';
			pData++;
			break;

		case '^':
			*pBuf++ = '\\';
			*pBuf++ = '^';
			*pBuf++ = '{';
			*pBuf++ = '}';
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			*pBuf++ = '\\';
			*pBuf++ = '\\';
			pData++;
			break;

		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break -- TODO
			pData++;
			break;
			
		case UCS_FF:					// FF -- representing a Forced-Page-Break
			*pBuf++ = '\\';
			*pBuf++ = 'n';
			*pBuf++ = 'e';
			*pBuf++ = 'w';
			*pBuf++ = 'p';
			*pBuf++ = 'a';
			*pBuf++ = 'g';
			*pBuf++ = 'e';
			*pBuf++ = '\n';
			pData++;
			break;
			
			
		default:
			if (*pData < 256)
				*pBuf++ = (UT_Byte) *pData++;
			else
			{
				pData += 2; // TODO
			}

			break;
		}
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

s_LaTeX_Listener::s_LaTeX_Listener(PD_Document * pDocument,
										 IE_Exp_LaTeX * pie)
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
	m_pie->write("\\usepackage{calc}\n");
	m_pie->write("\\usepackage{setspace}\n");
	m_pie->write("\\usepackage{multicol}\t% TODO: I don't need this package if the document is a single column one.\n");
	m_pie->write("\n");
	//	m_pie->write("\\begin{document}\n");  // I've to leave this step to the openSection, and that implies
	//	m_pie->write("\n");                   // future problems when we will support several sections in the same doc...
}

s_LaTeX_Listener::~s_LaTeX_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();
	
	m_pie->write("\\end{document}\n");
}

UT_Bool s_LaTeX_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_LaTeX_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
			pAP->getAttribute((XML_Char*)"type", pszSectionType);
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

UT_Bool s_LaTeX_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_LaTeX_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_LaTeX_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_LaTeX::_writeDocument(void)
{
	m_pListener = new s_LaTeX_Listener(m_pDocument,this);
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

void s_LaTeX_Listener::_handleDataItems(void)
{
}

