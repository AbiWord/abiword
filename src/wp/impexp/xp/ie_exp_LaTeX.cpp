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
#include <locale.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "pt_Types.h"
#include "ie_exp_LaTeX.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_EncodingManager.h"
#include "fd_Field.h"

#include "ut_string_class.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("LaTeX")

// we use a reference-counted sniffer
static IE_Exp_LaTeX_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_LaTeX_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "LaTeX Exporter";
	mi->desc = "Export LaTeX Documents";
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

bool IE_Exp_LaTeX_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".tex") || !UT_stricmp(szSuffix, ".latex"));
}

UT_Error IE_Exp_LaTeX_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_LaTeX * p = new IE_Exp_LaTeX(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_LaTeX_Sniffer::getDlgLabels(const char ** pszDesc,
										const char ** pszSuffixList,
										IEFileType * ft)
{
	*pszDesc = "LaTeX (.latex)";
	*pszSuffixList = "*.tex; *.latex";
	*ft = getFileType();
	return true;
}

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
typedef UT_UCSChar U16;
static int wvConvertUnicodeToLaTeX(U16 char16,char*& out);
static bool _convertLettersToSymbols(char c, char *& subst);

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

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openParagraph(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_handleDataItems(void);
	void				_convertFontSize(UT_String& szDest, const char* pszFontSize);
	void				_convertColor(UT_String& szDest, const char* pszColor);
	
	PD_Document *		m_pDocument;
	IE_Exp_LaTeX *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;
	bool                m_bMultiCols;
	UT_uint16           m_iInSymbol;
	JustificationTypes  m_eJustification;
	bool				m_bLineHeight;
	bool				m_bFirstSection;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*
	UT_Wctomb		m_wctomb;
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
		m_bMultiCols = false;
	}

	m_bInSection = false;
	return;
}

void s_LaTeX_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	switch (m_iBlockType)
	{
	case BT_NORMAL:
		if (m_bLineHeight)
		  m_pie->write("\n\\end{spacing}");

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

	m_bInBlock = false;
	return;
}

void s_LaTeX_Listener::_openParagraph(PT_AttrPropIndex api)
{
	m_eJustification = JUSTIFIED;
	m_bLineHeight = false;

	if (!m_bInSection)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
		{
			
			if(0 == UT_strcmp(szValue, "Heading 1")) 
			{
				m_iBlockType = BT_HEADING1;
				m_pie->write("\\section{");
			}
			else if(0 == UT_strcmp(szValue, "Heading 2")) 
			{
				m_iBlockType = BT_HEADING2;
				m_pie->write("\\subsection{");
			}
			else if(0 == UT_strcmp(szValue, "Heading 3")) 
			{
				m_iBlockType = BT_HEADING3;
				m_pie->write("\\subsubsection{");
			}
			else if(0 == UT_strcmp(szValue, "Block Text"))
			{
				m_iBlockType = BT_BLOCKTEXT;
				m_pie->write("\\begin{quote}\n");
			}
			else if(0 == UT_strcmp(szValue, "Plain Text"))
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
			if (pAP->getProperty("text-align", szValue))
			{
				if (0 == UT_strcmp(szValue, "center"))
				{
					m_pie->write("\\begin{center}\n");
					m_eJustification = CENTER;
				}
				if (0 == UT_strcmp(szValue, "right"))
				{
					m_pie->write("\\begin{flushright}\n");
					m_eJustification = RIGHT;
				}
				if (0 == UT_strcmp(szValue, "left"))
				{
					m_pie->write("\\begin{flushleft}\n");
					m_eJustification = LEFT;
				}
			}

			if (pAP->getProperty("line-height", szValue))
			{
				double height = atof(szValue);

				if (height < 0.9 || height > 1.1)
				{
					m_pie->write("\\begin{spacing}{");
					printf("m_bLineHeight = true\n");
					m_bLineHeight = true;
				}
				if (height > 1.4 && height < 1.6)
					m_pie->write("1.24}\n");
				else if (height > 1.9 && height < 2.1)
					m_pie->write("1.66}\n");
				else if (m_bLineHeight) // glup.  TODO: calculate the spacing :)
				    m_pie->write("1.0} % Sorry.  I know that you don't expect the 1.0... feel free to fix it! :)\n");
			}
		}
	}
	else 
	{
		m_iBlockType = BT_NORMAL;
	}
	
	m_bInBlock = true;
}

void s_LaTeX_Listener::_openSection(PT_AttrPropIndex api)
{
	const PP_AttrProp* pAP = NULL;
	bool bMustEmitMulticol = false;
	const XML_Char* pszNbCols = NULL;

	m_bMultiCols = false;

	if (m_pDocument->getAttrProp(api, &pAP) && pAP)
	{
		const XML_Char* pszPageMarginLeft = NULL;
		const XML_Char* pszPageMarginRight = NULL;

		pAP->getProperty("columns", pszNbCols);
		pAP->getProperty("page-margin-right", pszPageMarginLeft);
		pAP->getProperty("page-margin-left", pszPageMarginRight);

		if (pszNbCols != NULL && ((0 == UT_strcmp(pszNbCols, "2"))
								  || (0 == UT_strcmp(pszNbCols, "3"))))
		{
			bMustEmitMulticol = true;
			m_bMultiCols = true;
		}
		if (pszPageMarginLeft != NULL)
		{
			m_pie->write("\\setlength{\\oddsidemargin}{");
			m_pie->write(static_cast<const char *> (pszPageMarginLeft));
			m_pie->write("-1in");
			m_pie->write("}\n");
		}
		if (pszPageMarginRight != NULL)
		{
			m_pie->write("\\setlength{\\textwidth}{\\paperwidth - ");
			m_pie->write(static_cast<const char *> (pszPageMarginRight));
			m_pie->write("-");
			m_pie->write(static_cast<const char *> (pszPageMarginLeft));
			m_pie->write("}\n");
		}
	}

	if (m_bFirstSection)
	{
		m_pie->write ("\n\n\\begin{document}\n");
		m_bFirstSection = false;
	}

	if (bMustEmitMulticol)
	{
		m_pie->write("\\begin{multicols}{");
		m_pie->write(static_cast<const char *> (pszNbCols));
		m_pie->write("}\n");
	}
}

void s_LaTeX_Listener::_convertColor(UT_String& szDest, const char* pszColor)
{
	char colors[3][3];
	for (int i=0;i<3;++i)
	{
		strncpy (colors[i],&pszColor[2*i],2);
		colors[i][2]=0;
	}
	setlocale (LC_NUMERIC, "C");
	UT_String_sprintf (szDest, "%.3f,%.3f,%.3f",
			   strtol (&colors[0][0],NULL,16)/255.,
			   strtol (&colors[1][0],NULL,16)/255.,
			   strtol (&colors[2][0],NULL,16)/255.);
	setlocale (LC_NUMERIC, "");
}

void s_LaTeX_Listener::_convertFontSize(UT_String& szDest, const char* pszFontSize)
{
	double fSizeInPoints = UT_convertToPoints(pszFontSize);

	if (fSizeInPoints <= 6)
	{
		szDest = "tiny";
	}
	else if (fSizeInPoints <= 8)
	{
		szDest = "scriptsize";
	}
	else if (fSizeInPoints <= 10)
	{
		szDest = "footnotesize";
	}
	else if (fSizeInPoints <= 11)
	{
		szDest = "small";
	}
	else if (fSizeInPoints <= 12)
	{
		szDest = "normalsize";
	}
	else if (fSizeInPoints <= 14)
	{
		szDest = "large";
	}
	else if (fSizeInPoints <= 17)
	{
		szDest = "Large";
	}
	else if (fSizeInPoints <= 20)
	{
		szDest = "LARGE";
	}
	else if (fSizeInPoints <= 25)
	{
		szDest = "huge";
	}
	else
	{
		szDest = "Huge";
	}
}

void s_LaTeX_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			m_pie->write("\\textbf{");
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			m_pie->write("\\textit{");
		}
		
		if (pAP->getProperty("text-decoration", szValue))
		{
			const XML_Char* pszDecor = szValue;

			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");

			// See the ulem.sty documentation (available at www.ctan.org)
			// if you wish to include other kinds of underlines, such as
			// double underlines or wavy underlines
			while (q)
			{
			  if (0 == UT_strcmp(q, "underline")) // TODO: \def\undertext#1{$\underline{\vphantom{y}\smash{\hbox{#1}}}$}
				{
					m_pie->write("\\uline{");
				}

				if (0 == UT_strcmp(q, "overline"))
				{
					m_pie->write("$\\overline{\\textrm{");
				}

				if (0 == UT_strcmp(q, "line-through"))
				{
					m_pie->write("\\sout{");
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("$^{\\mathrm{");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("$_{\\mathrm{");
			}
		}
		
		const XML_Char* pszColor = NULL;
		pAP->getProperty("color", pszColor);
		if (pszColor)
		{
		  UT_String szColor;
			_convertColor(szColor,(char*)pszColor);
			m_pie->write("\\textcolor[rgb]{");
			m_pie->write(szColor);
			m_pie->write("}{");
		}
		
		const XML_Char* pszBgColor = NULL;
		pAP->getProperty("bgcolor", pszBgColor);
		if (pszBgColor)
		{
		  UT_String szColor;
			_convertColor(szColor,(char*)pszBgColor);
			m_pie->write("\\colorbox[rgb]{");
			m_pie->write(szColor);
			m_pie->write("}{");
		}

		if (pAP->getProperty("font-size", szValue))
		{
			if (UT_strcmp (DEFAULT_SIZE, szValue) != 0)
			{
				m_pie->write("{\\");
				UT_String szSize;
				_convertFontSize(szSize, (char*)szValue);
				m_pie->write(szSize);
				m_pie->write("{}");
			}
		}
		
		if (pAP->getProperty("font-family", szValue))
		{
			if (!UT_strcmp ("Symbol", szValue) || 
				!UT_strcmp("Standard Symbols", szValue))
				m_iInSymbol++;
			UT_DEBUGMSG (("Latex export: TODO: 'font-family' property\n"));
		}

		m_bInSpan = true;
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
		
		if (pAP->getProperty("color", szValue))
		{
			m_pie->write("}");
		}

		if (pAP->getProperty("bgcolor", szValue))
		{
			m_pie->write("}");
		}

		if ((pAP->getProperty("font-size", szValue))
//		    || (pAP->getProperty("font-family", szValue))  // TODO
			)
		{
			if (strcmp (szValue, DEFAULT_SIZE) != 0)
				m_pie->write("}");
		}

		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("}}$");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("}}$");
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
				if (0 == UT_strcmp(q, "line-through"))
				{
					m_pie->write("}");
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
				if (0 == UT_strcmp(q, "underline"))
				{
					m_pie->write("}");
				}

				if (0 == UT_strcmp(q, "overline"))
				{
					m_pie->write("}}$");
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			m_pie->write("}");
		}
		
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			m_pie->write("}");
		}

		if (pAP->getProperty("font-family", szValue))
		{
			if (!UT_strcmp ("Symbol", szValue) ||
				!UT_strcmp("Standard Symbols", szValue))
				m_iInSymbol--;
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}


void s_LaTeX_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (!m_bInBlock)
	{
		return;
	}

	UT_String sBuf;
	const UT_UCSChar * pData;

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData = data; (pData < data + length); /**/)
	{
		char* subst = "";

		if (m_iInSymbol > 0)
		{
			if (_convertLettersToSymbols(*pData, subst))
			{
				while (*subst)
					sBuf += *subst++;
				pData++;
				continue;
			}
		}

		switch (*pData)
		{
		case '\\':
			sBuf += "\\ensuremath{\\backslash}";
			pData++;
			break;
			
		case '$':
			sBuf += '\\'; 
			sBuf += '$';
			pData++;
			break;

		case '%':
			sBuf += '\\'; 
			sBuf += '%';
			pData++;
			break;
			
		case '&':
			sBuf += '\\'; 
			sBuf += '&';
			pData++;
			break;

		case '#':
			sBuf += '\\'; 
			sBuf += '#';
			pData++;
			break;

		case '_':
			sBuf += '\\'; 
			sBuf += '_';
			pData++;
			break;

		case '{':
			sBuf += '\\'; 
			sBuf += '{';
			pData++;
			break;

		case '}':
			sBuf += '\\';
			sBuf += '}';
			pData++;
			break;

		case '~':
			sBuf += '\\';
			sBuf += '~';
			sBuf += '{';
			sBuf += '}';
			pData++;
			break;

		case '^':
			sBuf += '\\';
			sBuf += '^';
			sBuf += '{';
			sBuf += '}';
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			sBuf += '\\';
			sBuf += '\\';
			pData++;
			break;

		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break -- TODO
			pData++;
			break;
			
		case UCS_FF:					// FF -- representing a Forced-Page-Break
			sBuf += '\\';
			sBuf += 'n';
			sBuf += 'e';
			sBuf += 'w';
			sBuf += 'p';
			sBuf += 'a';
			sBuf += 'g';
			sBuf += 'e';
			sBuf += '\n';
			pData++;
			break;
			
			
		default:
			int translated =  wvConvertUnicodeToLaTeX(*pData,subst);
			if (translated) 
			{
				while (*subst)
					sBuf += *subst++;
				pData++;
			}
			else 
			{
				char buf[30];
				int len;
				if (m_wctomb.wctomb(buf,len,(wchar_t)*pData++)) {
				    for(int i=0;i<len;++i)
						sBuf += buf[i];
				};
			}
			break;
		}
	}

	m_pie->write(sBuf.c_str(),sBuf.size());
}

#define SUB(a,who) case a: subst = "\\(\\"who"\\)"; return true;
#define SUBd(a,who) case a: subst = who; return true;
static bool _convertLettersToSymbols(char c, char *& subst)
{
	switch (c)
	{
		// only-if-amssymb
// 		SUB('\\', "therefore");
		// todo: $ and ^ don't actually work right.

		SUB('\"', "forall");    SUB('$', "exists");
		SUB('\'', "ni");        SUB('@', "cong");
		SUB('^', "perp");       SUB('`', "overline{\\ }");
		SUB('a', "alpha");      SUBd('A', "A");
		SUB('b', "beta"); 	    SUBd('B', "B");
		SUB('c', "chi");  	    SUBd('C', "X");
		SUB('d', "delta");	    SUB('D', "Delta");
		SUB('e', "varepsilon"); SUBd('E', "E");
		SUB('f', "phi");  	    SUB('F', "Phi");
		SUB('g', "gamma");	    SUB('G', "Gamma");
		SUB('h', "eta");	    SUBd('H', "H");
		SUB('i', "iota"); 	    SUBd('I', "I"); 
		SUB('j', "varphi");     SUB('J', "vartheta");
		SUB('k', "kappa"); 	    SUBd('K', "K");
		SUB('l', "lambda");	    SUB('L', "Lambda");
		SUB('m', "mu");    	    SUBd('M', "M");
		SUB('n', "nu");    	    SUBd('N', "N");
		SUBd('o', "o");    	    SUBd('O', "O");
		SUB('p', "pi");    	    SUB('P', "Pi");
		SUB('q', "theta"); 	    SUB('Q', "Theta");
		SUB('r', "rho");   	    SUBd('R', "P");
		SUB('s', "sigma"); 	    SUB('S', "Sigma");
		SUB('t', "tau");   	    SUBd('T', "T");
		SUB('u', "upsilon");    SUBd('U', "Y");
 		SUB('v', "varpi");		SUB('V', "varsigma");
		SUB('w', "omega");      SUB('W', "Omega");
		SUB('x', "xi");         SUB('X', "Xi");
		SUB('y', "psi");        SUB('Y', "Psi");
		SUB('z', "zeta");       SUBd('Z', "Z");
// TODO all those fun upper-ascii letters
	default: return false;
	}
}

s_LaTeX_Listener::s_LaTeX_Listener(PD_Document * pDocument,
										 IE_Exp_LaTeX * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	m_bFirstSection = true;
	m_iInSymbol = 0;

	m_pie->write("%% ================================================================================\n");
	m_pie->write("%% This LaTeX file was created by AbiWord.                                         \n");
	m_pie->write("%% AbiWord is a free, Open Source word processor.                                  \n");
	m_pie->write("%% You may obtain more information about AbiWord at www.abisource.com              \n");
	m_pie->write("%% ================================================================================\n");
	m_pie->write("\n");

	m_pie->write("\\documentclass[12pt]{article}\n");
	m_pie->write("\\usepackage[T1]{fontenc}\n");
	m_pie->write("\\usepackage{calc}\n");
	m_pie->write("\\usepackage{hyperref}");
	m_pie->write("\\usepackage{setspace}\n");
	m_pie->write("\\usepackage{multicol}\n");
	m_pie->write("\\usepackage[normalem]{ulem}\n");
	m_pie->write("\\usepackage{color}\n");
	{
	    const char* misc = XAP_EncodingManager::get_instance()->getTexPrologue();
	    if (misc)
		m_pie->write(misc);
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

bool s_LaTeX_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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
			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			PT_AttrPropIndex api = pcr->getIndexAP();
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

			const XML_Char* szValue = NULL;

			fd_Field* field = NULL;

			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// TODO we *could* insert the images and create separate GIF files.
				return true;

			case PTO_Field:

			  field = pcro->getField();
			  m_pie->write(field->getValue());

				// we do nothing with computed fields.
				return true;

			case PTO_Hyperlink:
			  _closeSpan () ;
				if(bHaveProp && pAP && pAP->getAttribute("xlink:href", szValue))
				{
					m_pie->write("\\href{");
					m_pie->write(szValue);
					m_pie->write("}{");
				}
				else
				{
					m_pie->write("}");
				}
				return true;

			case PTO_Bookmark:
			  return true;

			default:
				UT_ASSERT(0);
				return true;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_LaTeX_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				_openSection(pcr->getIndexAP());
				m_bInSection = true;
			}
			else
			{
				m_bInSection = false;
			}
		}
		else
		{
			m_bInSection = false;
		}
		
		return true;
	}

	case PTX_SectionHdrFtr:
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
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				_openSection(pcr->getIndexAP());
				m_bInSection = true;
			}
			else
			{
				m_bInSection = false;
			}
		}
		else
		{
			m_bInSection = false;
		}
		
		return true;
	}

	case PTX_Block:
	{
		_closeSpan();
		_closeBlock();
		_openParagraph(pcr->getIndexAP());
		return true;
	}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_LaTeX_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_LaTeX_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

bool s_LaTeX_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_LaTeX::_writeDocument(void)
{
	m_pListener = new s_LaTeX_Listener(getDoc(),this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)))
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
		
		case 12:
		case 13:
		case 14:
		case 7:
			return(1);
		case 45:
			printf("-");
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
			printf("\\'{\\AA}");  /* � with acute */
			return(1);
		case 0x01FB:
			printf("\\'{\\aa}");  /* � with acute */
			return(1);
		case 0x01FC:
			printf("\\'{\\AE}");  /* � with acute */
			return(1);
		case 0x01FD:
			printf("\\'{\\ae}");  /* � with acute */
			return(1);
		case 0x01FE:
			printf("\\'{\\O}");  /* � with acute */
			return(1);
		case 0x01FF:
			printf("\\'{\\o}");  /* � with acute */
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
