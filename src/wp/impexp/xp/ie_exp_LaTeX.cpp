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
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

#define DEFAULT_SIZE "12pt"
#define EPSILON 0.1

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
typedef UT_UCSChar U16;
static int wvConvertUnicodeToLaTeX(U16 char16,char*& out);

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
				double height = atof(szValue);

				if (height < 0.9 || height > 1.1)
				{
					m_pie->write("\\begin{spacing}{");
					printf("m_bLineHeight = UT_TRUE\n");
					m_bLineHeight = UT_TRUE;
				}
				if (height > 1.4 && height < 1.6)
					m_pie->write("1.24}\n");
				else if (height > 1.9 && height < 2.1)
					m_pie->write("1.66}\n");
				else if (m_bLineHeight) // glup.  TODO: calculate the spacing :)
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
			char* subst = "";
			int translated =  wvConvertUnicodeToLaTeX(*pData,subst);
			if (translated) 
			{
				while (*subst)
					*pBuf++ = *subst++;
				pData++;
			}
			else 
			{
				UT_UCSChar c = XAP_EncodingManager::instance->UToNative(*pData++);
				if (c<256)
					*pBuf++ = (UT_Byte)c;
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
	{
	    XAP_EncodingManager* em = XAP_EncodingManager::instance;
	    const char* enc =  em->getNativeTexEncodingName();
	    if (enc) {
		m_pie->write("\\usepackage[");
		m_pie->write(enc);
		m_pie->write("]{inputenc}\n");
	    }
	    const char* babelarg = em->getNativeBabelArgument();
	    if (babelarg) {
		m_pie->write("\\usepackage[");
		m_pie->write(babelarg);
		m_pie->write("]{babel}\n");
	    };
	}
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


/*
  This is a copy from wv. Returns 1 if was translated, 0 if wasn't.
  It can convert to empty string.
*/
#undef printf
#define printf(x) out = (x); 

static int wvConvertUnicodeToLaTeX(U16 char16,char*& out)
	{
	out = ""; //this is needed

	/* 
	german and scandinavian characters, MV 1.7.2000 
	See man iso_8859_1

 	This requires the inputencoding latin1 package,
 	see latin1.def. Chars in range 160...255 are just
	put through as these are legal iso-8859-1 symbols.
	(see above)
	
	Best way to do it until LaTeX is Unicode enabled 
	(Omega project).
	-- MV 4.7.2000
 
	We use a separate if-statement here ... the 'case range'
	construct is gcc specific :-(  -- MV 13/07/2000
	*/

	if ( (char16 >= 0xa0) && (char16 <= 0xff) ) 
	{
	switch(char16)
		{
		/* Fix up these as math characters: */
		case 0xb1:
			printf("$\\pm$");
			return(1);
		case 0xb2:
			printf("$\\mathtwosuperior$");
			return(1);
		case 0xb3:
			printf("$\\maththreesuperior$");
			return(1);
		case 0xb5:
			printf("$\\mu$");
			return(1);
		case 0xb9:
			printf("$\\mathonesuperior$");
			return(1);
		}
		return(0);
	}
	switch(char16)
		{
		case 37:
			printf("\\%%");
			return(1);
		case 11:
			printf("\\\\\n");
			return(1);
		case 30:
		case 31:
		case 45:
		
		case 12:
		case 13:
		case 14:
		case 7:
			return(1);
		case 34:
			printf("\"");
			return(1);
		case 35:
			printf("\\#"); /* MV 14.8.2000 */
			return(1);
		case 36:
			printf("\\$"); /* MV 14.8.2000 */
			return(1);
		case 38:
			printf("\\&"); /* MV 1.7.2000 */
			return(1);
		case 60:
			printf("<");
			return(1);
		case 62:
			printf(">");
			return(1);

		case 0xF8E7:	
		/* without this, things should work in theory, but not for me */
			printf("_");
			return(1);

	/* Added some new Unicode characters. It's probably difficult
           to write these characters in AbiWord, though ... :(
           -- 2000-08-11 huftis@bigfoot.com */

		case 0x0100:
			printf("\\=A"); /* A with macron */
			return(1);
		case 0x0101:
			printf("\\=a");  /* a with macron */
			return(1);
		case 0x0102:
			printf("\\u{A}");  /* A with breve */
			return(1);
		case 0x0103:
			printf("\\u{a}");  /* a with breve */
			return(1);

		case 0x0106:
			printf("\\'C");  /* C with acute */
			return(1);
		case 0x0107:
			printf("\\'c");  /* c with acute */
			return(1);
		case 0x0108:
			printf("\\^C");  /* C with circumflex */
			return(1);
		case 0x0109:
			printf("\\^c");  /* c with circumflex */
			return(1);
		case 0x010A:
			printf("\\.C");  /* C with dot above */
			return(1);
		case 0x010B:
			printf("\\.c");  /* c with dot above */
			return(1);
		case 0x010C:
			printf("\\v{C}");  /* C with caron */
			return(1);
		case 0x010D:
			printf("\\v{c}");  /* c with caron */
			return(1);
		case 0x010E:
			printf("\\v{D}");  /* D with caron */
			return(1);
		case 0x010F:
			printf("\\v{d}");  /* d with caron */
			return(1);
		case 0x0110:
			printf("\\DJ{}");  /* D with stroke */
			return(1);
		case 0x0111:
			printf("\\dj{}");  /* d with stroke */
			return(1);
		case 0x0112:
			printf("\\=E");  /* E with macron */
			return(1);
		case 0x0113:
			printf("\\=e");  /* e with macron */
			return(1);
		case 0x0114:
			printf("\\u{E}");  /* E with breve */
			return(1);
		case 0x0115:
			printf("\\u{e}");  /* e with breve */
			return(1);
		case 0x0116:
			printf("\\.E");  /* E with dot above */
			return(1);
		case 0x0117:
			printf("\\.e");  /* e with dot above */
			return(1);

		case 0x011A:
			printf("\\v{E}");  /* E with caron */
			return(1);
		case 0x011B:
			printf("\\v{e}");  /* e with caron */
			return(1);
		case 0x011C:
			printf("\\^G");  /* G with circumflex */
			return(1);
		case 0x011D:
			printf("\\^g");  /* g with circumflex */
			return(1);
		case 0x011E:
			printf("\\u{G}");  /* G with breve */
			return(1);
		case 0x011F:
			printf("\\u{g}");  /* g with breve */
			return(1);
		case 0x0120:
			printf("\\.G");  /* G with dot above */
			return(1);
		case 0x0121:
			printf("\\u{g}");  /* g with dot above */
			return(1);
		case 0x0122:
			printf("^H");  /* H with circumflex */
			return(1);
		case 0x0123:
			printf("^h");  /* h with circumflex */
			return(1);

		case 0x0128:
			printf("\\~I");  /* I with tilde */
			return(1);
		case 0x0129:
			printf("\\~{\\i}");  /* i with tilde (dotless) */
			return(1);
		case 0x012A:
			printf("\\=I");  /* I with macron */
			return(1);
		case 0x012B:
			printf("\\={\\i}");  /* i with macron (dotless) */
			return(1);
		case 0x012C:
			printf("\\u{I}");  /* I with breve */
			return(1);
		case 0x012D:
			printf("\\u{\\i}");  /* i with breve */
			return(1);

		case 0x0130:
			printf("\\.I");  /* I with dot above */
			return(1);
		case 0x0131:
			printf("\\i{}");  /* dotless i */
			return(1);
		case 0x0132:
			printf("IJ");  /* IJ ligature */
			return(1);
		case 0x0133:
			printf("ij");  /* ij ligature  */
			return(1);
		case 0x0134:
			printf("\\^J");  /* J with circumflex (dotless) */
			return(1);
		case 0x0135:
			printf("\\^{\\j}");  /* j with circumflex (dotless) */
			return(1);
		case 0x0136:
			printf("\\c{K}");  /* K with cedilla */
			return(1);
		case 0x0137:
			printf("\\c{k}");  /* k with cedilla */
			return(1);

		case 0x0138:
			printf("k");  /* NOTE: Not the correct character (kra), but similar */
			return(1);

		case 0x0139:
			printf("\\'L");  /* L with acute */
			return(1);
		case 0x013A:
			printf("\\'l");  /* l with acute  */
			return(1);
		case 0x013B:
			printf("\\c{L}");  /* L with cedilla */
			return(1);
		case 0x013C:
			printf("\\c{l}");  /* l with cedilla */
			return(1);
		case 0x013D:
			printf("\\v{L}");  /* L with caron */
			return(1);
		case 0x013E:
			printf("\\v{l}");  /* l with caron */
			return(1);

		case 0x0141:
			printf("\\L{}");  /* L with stroke */
			return(1);
		case 0x0142:
			printf("\\l{}");  /* l with stroke  */
			return(1);
		case 0x0143:
			printf("\\'N");  /* N with acute */
			return(1);
		case 0x0144:
			printf("\\'n");  /* n with acute */
			return(1);
		case 0x0145:
			printf("\\c{N}");  /* N with cedilla */
			return(1);
		case 0x0146:
			printf("\\c{n}");  /* n with cedilla */
			return(1);
		case 0x0147:
			printf("\\v{N}");  /* N with caron */
			return(1);
		case 0x0148:
			printf("\\v{n}");  /* n with caron */
			return(1);
		case 0x0149:
			printf("'n");  /* n preceed with apostroph  */
			return(1);
		case 0x014A:
			printf("\\NG{}");  /* ENG character */
			return(1);
		case 0x014B:
			printf("\\ng{}");  /* eng character */
			return(1);
		case 0x014C:
			printf("\\=O");  /* O with macron */
			return(1);
		case 0x014D:
			printf("\\=o");  /* o with macron */
			return(1);
		case 0x014E:
			printf("\\u{O}");  /* O with breve */
			return(1);
		case 0x014F:
			printf("\\u{o}");  /* o with breve */
			return(1);
		case 0x0150:
			printf("\\H{O}");  /* O with double acute */
			return(1);
		case 0x0151:
			printf("\\H{o}");  /* o with double acute */
			return(1);
		case 0x0152:
			printf("\\OE{}");  /* OE ligature */
			return(1);
		case 0x0153:
			printf("\\oe{}");  /* oe ligature */
			return(1);
		case 0x0154:
			printf("\\'R");  /* R with acute */
			return(1);
		case 0x0155:
			printf("\\'r");  /* r with acute */
			return(1);
		case 0x0156:
			printf("\\c{R}");  /* R with cedilla */
			return(1);
		case 0x0157:
			printf("\\c{r}");  /* r with cedilla */
			return(1);
		case 0x0158:
			printf("\\v{R}");  /* R with caron */
			return(1);
		case 0x0159:
			printf("\\v{r}");  /* r with caron */
			return(1);
		case 0x015A:
			printf("\\'S");  /* S with acute */
			return(1);
		case 0x015B:
			printf("\\'s");  /* s with acute */
			return(1);
		case 0x015C:
			printf("\\^S");  /* S with circumflex */
			return(1);
		case 0x015D:
			printf("\\^s");  /* c with circumflex */
			return(1);
		case 0x015E:
			printf("\\c{S}");  /* S with cedilla */
			return(1);
		case 0x015F:
			printf("\\c{s}");  /* s with cedilla */
			return(1);
		case 0x0160:
			printf("\\v{S}");  /* S with caron */
			return(1);
		case 0x0161:
			printf("\\v{s}");  /* s with caron */
			return(1);
		case 0x0162:
			printf("\\c{T}");  /* T with cedilla */
			return(1);
		case 0x0163:
			printf("\\c{t}");  /* t with cedilla */
			return(1);
		case 0x0164:
			printf("\\v{T}");  /* T with caron */
			return(1);
		case 0x0165:
			printf("\\v{t}");  /* t with caron */
			return(1);

		case 0x0168:
			printf("\\~U");  /* U with tilde */
			return(1);
		case 0x0169:
			printf("\\~u");  /* u with tilde */
			return(1);
		case 0x016A:
			printf("\\=U");  /* U with macron */
			return(1);

		/* Greek (thanks Petr Vanicek!): */
		case 0x0391:
			printf("$\\Alpha$");
			return(1);
		case 0x0392:
			printf("$\\Beta$");
			return(1);
		case 0x0393:
			printf("$\\Gamma$");
			return(1);
		case 0x0394:
			printf("$\\Delta$");
			return(1);
		case 0x0395:
			printf("$\\Epsilon$");
			return(1);
		case 0x0396:
			printf("$\\Zeta$");
			return(1);
		case 0x0397:
			printf("$\\Eta$");
			return(1);
		case 0x0398:
			printf("$\\Theta$");
			return(1);
		case 0x0399:
			printf("$\\Iota$");
			return(1);
		case 0x039a:
			printf("$\\Kappa$");
			return(1);
		case 0x039b:
			printf("$\\Lambda$");
			return(1);
		case 0x039c:
			printf("$\\Mu$");
			return(1);
		case 0x039d:
			printf("$\\Nu$");
			return(1);
		case 0x039e:
			printf("$\\Xi$");
			return(1);
		case 0x039f:
			printf("$\\Omicron$");
			return(1);
		case 0x03a0:
			printf("$\\Pi$");
			return(1);
		case 0x03a1:
			printf("$\\Rho$");
			return(1);

		case 0x03a3:
			printf("$\\Sigma$");
			return(1);
		case 0x03a4:
			printf("$\\Tau$");
			return(1);
		case 0x03a5:
			printf("$\\Upsilon$");
			return(1);
		case 0x03a6:
			printf("$\\Phi$");
			return(1);
		case 0x03a7:
			printf("$\\Chi$");
			return(1);
		case 0x03a8:
			printf("$\\Psi$");
			return(1);
		case 0x03a9:
			printf("$\\Omega$");
			return(1);

		/* ...and lower case: */

		case 0x03b1:
			printf("$\\alpha$");
			return(1);
		case 0x03b2:
			printf("$\\beta$");
			return(1);
		case 0x03b3:
			printf("$\\gamma$");
			return(1);
		case 0x03b4:
			printf("$\\delta$");
			return(1);
		case 0x03b5:
			printf("$\\epsilon$");
			return(1);
		case 0x03b6:
			printf("$\\zeta$");
			return(1);
		case 0x03b7:
			printf("$\\eta$");
			return(1);
		case 0x03b8:
			printf("$\\theta$");
			return(1);
		case 0x03b9:
			printf("$\\iota$");
			return(1);
		case 0x03ba:
			printf("$\\kappa$");
			return(1);
		case 0x03bb:
			printf("$\\lambda$");
			return(1);
		case 0x03bc:
			printf("$\\mu$");
			return(1);
		case 0x03bd:
			printf("$\\nu$");
			return(1);
		case 0x03be:
			printf("$\\xi$");
			return(1);
		case 0x03bf:
			printf("$\\omicron$");
			return(1);
		case 0x03c0:
			printf("$\\pi$");
			return(1);
		case 0x03c1:
			printf("$\\rho$");
			return(1);

		case 0x03c3:
			printf("$\\sigma$");
			return(1);
		case 0x03c4:
			printf("$\\tau$");
			return(1);
		case 0x03c5:
			printf("$\\upsilon$");
			return(1);
		case 0x03c6:
			printf("$\\phi$");
			return(1);
		case 0x03c7:
			printf("$\\chi$");
			return(1);
		case 0x03c8:
			printf("$\\psi$");
			return(1);
		case 0x03c9:
			printf("$\\omega$");
			return(1);

	/* More math, typical inline: */
		case 0x2111:
			printf("$\\Im$");
			return(1);
		case 0x2118:
			printf("$\\wp$");   /* Weierstrass p */
			return(1);
		case 0x211c:
			printf("$\\Re$");
			return(1);
		case 0x2135:
			printf("$\\aleph$");
			return(1);

		case 0x2190:
			printf("$\\leftarrow$");
			return(1);
		case 0x2191:
			printf("$\\uparrow$");
			return(1);
		case 0x2192:
			printf("$\\rightarrow$");
			return(1);
		case 0x2193:
			printf("$\\downarrow$");
			return(1);
		case 0x21d0:
			printf("$\\Leftarrow$");
			return(1);
		case 0x21d1:
			printf("$\\Uparrow$");
			return(1);
		case 0x21d2:
			printf("$\\Rightarrow$");
			return(1);
		case 0x21d3:
			printf("$\\Downarrow$");
			return(1);
		case 0x21d4:
			printf("$\\Leftrightarrow$");
			return(1);

		case 0x2200:
			printf("$\\forall$");
			return(1);
		case 0x2202:
			printf("$\\partial$");
			return(1);
		case 0x2203:
			printf("$\\exists$");
			return(1);
		case 0x2205:
			printf("$\\emptyset$");
			return(1);
		case 0x2207:
			printf("$\\nabla$");
			return(1);
		case 0x2208:
			printf("$\\in$");   /* element of */
			return(1);
		case 0x2209:
			printf("$\\notin$");   /* not an element of */
			return(1);
		case 0x220b:
			printf("$\\ni$");   /* contains as member */
			return(1);
		case 0x221a:
			printf("$\\surd$"); 	/* sq root */
			return(1);
		case 0x2212:
			printf("$-$");		/* minus */
			return(1);
		case 0x221d:
			printf("$\\propto$");
			return(1);
		case 0x221e:
			printf("$\\infty$");
			return(1);
		case 0x2220:
			printf("$\\angle$");
			return(1);
		case 0x2227:
			printf("$\\land$"); /* logical and */
			return(1);
		case 0x2228:
			printf("$\\lor$");   /* logical or */
			return(1);
		case 0x2229:
			printf("$\\cap$"); /* intersection */
			return(1);
		case 0x222a:
			printf("$\\cup$"); /* union */
			return(1);
		case 0x223c:
			printf("$\\sim$"); /* similar to  */
			return(1);
		case 0x2248:
			printf("$\\approx$");
			return(1);
		case 0x2261:
			printf("$\\equiv$");
			return(1);
		case 0x2260:
			printf("$\\neq$");
			return(1);
		case 0x2264:
			printf("$\\leq$");
			return(1);
		case 0x2265:
			printf("$\\geq$");
			return(1);
		case 0x2282:
			printf("$\\subset$");
			return(1);
		case 0x2283:
			printf("$\\supset$");
			return(1);
		case 0x2284:
			printf("$\\notsubset$");
			return(1);
		case 0x2286:
			printf("$\\subseteq$");
			return(1);
		case 0x2287:
			printf("$\\supseteq$");
			return(1);
		case 0x2295:
			printf("$\\oplus$");   /* circled plus */
			return(1);
		case 0x2297:
			printf("$\\otimes$");
			return(1);
		case 0x22a5:
			printf("$\\perp$");	/* perpendicular */
			return(1);




		case 0x2660:
			printf("$\\spadesuit$");
			return(1);
		case 0x2663:
			printf("$\\clubsuit$");
			return(1);
		case 0x2665:
			printf("$\\heartsuit$");
			return(1);
		case 0x2666:
			printf("$\\diamondsuit$");
			return(1);


		case 0x01C7:
			printf("LJ");  /* the LJ letter */
			return(1);
		case 0x01C8:
			printf("Lj");  /* the Lj letter */
			return(1);
		case 0x01C9:
			printf("lj");  /* the lj letter */
			return(1);
		case 0x01CA:
			printf("NJ");  /* the NJ letter */
			return(1);
		case 0x01CB:
			printf("Nj");  /* the Nj letter */
			return(1);
		case 0x01CC:
			printf("nj");  /* the nj letter */
			return(1);
		case 0x01CD:
			printf("\\v{A}");  /* A with caron */
			return(1);
		case 0x01CE:
			printf("\\v{a}");  /* a with caron */
			return(1);
		case 0x01CF:
			printf("\\v{I}");  /* I with caron */
			return(1);
		case 0x01D0:
			printf("\\v{\\i}");  /* i with caron (dotless) */
			return(1);
		case 0x01D1:
			printf("\\v{O}");  /* O with caron */
			return(1);
		case 0x01D2:
			printf("\\v{o}");  /* o with caron */
			return(1);
		case 0x01D3:
			printf("\\v{U}");  /* U with caron */
			return(1);
		case 0x01D4:
			printf("\\v{u}");  /* u with caron */
			return(1);

		case 0x01E6:
			printf("\\v{G}");  /* G with caron */
			return(1);
		case 0x01E7:
			printf("\\v{g}");  /* g with caron */
			return(1);
		case 0x01E8:
			printf("\\v{K}");  /* K with caron */
			return(1);
		case 0x01E9:
			printf("\\v{k}");  /* k with caron */
			return(1);


		case 0x01F0:
			printf("\\v{\\j}");  /* j with caron (dotless) */
			return(1);
		case 0x01F1:
			printf("DZ");  /* the DZ letter */
			return(1);
		case 0x01F2:
			printf("Dz");  /* the Dz letter */
			return(1);
		case 0x01F3:
			printf("dz");  /* the dz letter */
			return(1);
		case 0x01F4:
			printf("\\'G");  /* G with acute */
			return(1);
		case 0x01F5:
			printf("\\'g");  /* g with acute */
			return(1);

		case 0x01FA:
			printf("\\'{\\AA}");  /* Å with acute */
			return(1);
		case 0x01FB:
			printf("\\'{\\aa}");  /* å with acute */
			return(1);
		case 0x01FC:
			printf("\\'{\\AE}");  /* Æ with acute */
			return(1);
		case 0x01FD:
			printf("\\'{\\ae}");  /* æ with acute */
			return(1);
		case 0x01FE:
			printf("\\'{\\O}");  /* Ø with acute */
			return(1);
		case 0x01FF:
			printf("\\'{\\o}");  /* ø with acute */
			return(1);

		case 0x2010:
			printf("-"); /* hyphen */
			return(1);
		case 0x2011:
			printf("-"); /* non-breaking hyphen (is there a way to get this in LaTeX?) */
			return(1);
		case 0x2012:
			printf("--"); /* figure dash (similar to en-dash) */
			return(1);
		case 0x2013:
			/* 
			soft-hyphen? Or en-dash? I find that making 
			this a soft-hyphen works very well, but makes
			the occasional "hard" word-connection hyphen 
			(like the "-" in roller-coaster) disappear.
			(Are these actually en-dashes? Dunno.)
			How does MS Word distinguish between the 0x2013's
			that signify soft hyphens and those that signify
			word-connection hyphens? wvware should be able
			to as well. -- MV 8.7.2000
	
			U+2013 is the en-dash character and not a soft
			hyphen. Soft hyphen is U+00AD. Changing to
			"--". -- 2000-08-11 huftis@bigfoot.com
			*/
			printf("--"); 
			return(1);

		case 0x016B:
			printf("\\=u");  /* u with macron */
			return(1);
		case 0x016C:
			printf("\\u{U}");  /* U with breve */
			return(1);
		case 0x016D:
			printf("\\u{u}");  /* u with breve */
			return(1);
		case 0x016E:
			printf("\\r{U}");  /* U with ring above */
			return(1);
		case 0x016F:
			printf("\\r{U}");  /* u with ring above */
			return(1);
		case 0x0170:
			printf("\\H{U}");  /* U with double acute */
			return(1);
		case 0x0171:
			printf("\\H{u}");  /* u with double acute */
			return(1);

		case 0x0174:
			printf("\\^W");  /* W with circumflex */
			return(1);
		case 0x0175:
			printf("\\^w");  /* w with circumflex */
			return(1);
		case 0x0176:
			printf("\\^Y");  /* Y with circumflex */
			return(1);
		case 0x0177:
			printf("\\^y");  /* y with circumflex */
			return(1);
		case 0x0178:
			printf("\\\"Y");  /* Y with diaeresis */
			return(1);
		case 0x0179:
			printf("\\'Z");  /* Z with acute */
			return(1);
		case 0x017A:
			printf("\\'z");  /* z with acute */
			return(1);
		case 0x017B:
			printf("\\.Z");  /* Z with dot above */
			return(1);
		case 0x017C:
			printf("\\.z");  /* z with dot above */
			return(1);
		case 0x017D:
			printf("\\v{Z}");  /* Z with caron */
			return(1);
		case 0x017E:
			printf("\\v{z}");  /* z with caron */
			return(1);

	/* Windows specials (MV 4.7.2000). More could be added. 
	See http://www.hut.fi/u/jkorpela/www/windows-chars.html
	*/

		case 0x2014:
			printf("---"); /* em-dash */
			return(1);
		case 0x2018:
			printf("`");  /* left single quote, Win */
			return(1);
		case 0x2019:
			printf("'");  /* Right single quote, Win */
			return(1);
		case 0x201A:
			printf("\\quotesinglbase{}");  /* single low 99 quotation mark */
			return(1);
		case 0x201C:
			printf("``");  /* inverted double quotation mark */
			return(1);
		case 0x201D:
			printf("''");  /* double q.m. */
			return(1);
		case 0x201E:
			printf("\\quotedblbase{}");  /* double low 99 quotation mark */
			return(1);
		case 0x2020:
			printf("\\dag{}");  /* dagger */
			return(1);
		case 0x2021:
			printf("\\ddag{}");  /* double dagger */
			return(1);
		case 0x2022:
			printf("$\\bullet$");  /* bullet */
			return(1);
		case 0x2023:
			printf("$\\bullet$");  /* NOTE: Not a real triangular bullet */
			return(1);

		case 0x2024:
			printf(".");  /* One dot leader (for use in TOCs) */
			return(1);
		case 0x2025:
			printf("..");  /* Two dot leader (for use in TOCs) */
			return(1);
		case 0x2026:
			printf("\\ldots"); /* ellipsis */
			return(1);

		case 0x2039:
			printf("\\guilsinglleft{}");  /* single left angle quotation mark */
			return(1);
		case 0x203A:
			printf("\\guilsinglright{}"); /* single right angle quotation mark */
			return(1);

		case 0x203C:
			printf("!!"); /* double exclamation mark */
			return(1);

		case 0x2215:
			printf("$/$");  /* Division slash */
			return(1);

		case 0x2030:
			printf("o/oo");
			return(1);

		case 0x20ac:
			printf("\\euro");
                        /* No known implementation ;-)

                        Shouldn't we use the package 'eurofont'?
                        -- 2000-08-15 huftis@bigfoot.com 
                        */
			return(1);

		case 0x2160:
			printf("I"); /* Roman numeral I */
			return(1);
		case 0x2161:
			printf("II"); /* Roman numeral II */
			return(1);
		case 0x2162:
			printf("III"); /* Roman numeral III */
			return(1);
		case 0x2163:
			printf("IV"); /* Roman numeral IV */
			return(1);
		case 0x2164:
			printf("V"); /* Roman numeral V */
			return(1);
		case 0x2165:
			printf("VI"); /* Roman numeral VI */
			return(1);
		case 0x2166:
			printf("VII"); /* Roman numeral VII */
			return(1);
		case 0x2167:
			printf("VIII"); /* Roman numeral VIII */
			return(1);
		case 0x2168:
			printf("IX"); /* Roman numeral IX */
			return(1);
		case 0x2169:
			printf("X"); /* Roman numeral X */
			return(1);
		case 0x216A:
			printf("XI"); /* Roman numeral XI */
			return(1);
		case 0x216B:
			printf("XII"); /* Roman numeral XII */
			return(1);
		case 0x216C:
			printf("L"); /* Roman numeral L */
			return(1);
		case 0x216D:
			printf("C"); /* Roman numeral C */
			return(1);
		case 0x216E:
			printf("D"); /* Roman numeral D */
			return(1);
		case 0x216F:
			printf("M"); /* Roman numeral M */
			return(1);
		case 0x2170:
			printf("i"); /* Roman numeral i */
			return(1);
		case 0x2171:
			printf("ii"); /* Roman numeral ii */
			return(1);
		case 0x2172:
			printf("iii"); /* Roman numeral iii */
			return(1);
		case 0x2173:
			printf("iv"); /* Roman numeral iv */
			return(1);
		case 0x2174:
			printf("v"); /* Roman numeral v */
			return(1);
		case 0x2175:
			printf("vi"); /* Roman numeral vi */
			return(1);
		case 0x2176:
			printf("vii"); /* Roman numeral vii */
			return(1);
		case 0x2177:
			printf("viii"); /* Roman numeral viii */
			return(1);
		case 0x2178:
			printf("ix"); /* Roman numeral ix */
			return(1);
		case 0x2179:
			printf("x"); /* Roman numeral x */
			return(1);
		case 0x217A:
			printf("xi"); /* Roman numeral xi */
			return(1);
		case 0x217B:
			printf("xiii"); /* Roman numeral xii */
			return(1);
		case 0x217C:
			printf("l"); /* Roman numeral l */
			return(1);
		case 0x217D:
			printf("c"); /* Roman numeral c */
			return(1);
		case 0x217E:
			printf("d"); /* Roman numeral d */
			return(1);
		case 0x217F:
			printf("m"); /* Roman numeral m */
			return(1);

		}
	/* Debugging aid: */
	return(0);
	}
#undef printf