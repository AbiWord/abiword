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
#include "ie_exp_HTML.h"
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

IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_HTML::~IE_Exp_HTML()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_HTML::RecognizeSuffix(const char * szSuffix)
{
	return ((UT_stricmp(szSuffix,".html") == 0) || (UT_stricmp(szSuffix,".htm") == 0));
}

UT_Error IE_Exp_HTML::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_HTML::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "HTML (.html)";
	*pszSuffixList = "*.html";
	*ft = IEFT_HTML;
	return UT_TRUE;
}

UT_Bool IE_Exp_HTML::SupportsFileType(IEFileType ft)
{
	return (IEFT_HTML == ft);
}

/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6

class s_HTML_Listener : public PL_Listener
{
public:
	s_HTML_Listener(PD_Document * pDocument,
						IE_Exp_HTML * pie);
	virtual ~s_HTML_Listener();

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
	IE_Exp_HTML *		m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*

};

void s_HTML_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_pie->write("</div>\n");
	m_bInSection = UT_FALSE;
	return;
}

void s_HTML_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
	{
		return;
	}

       	if(m_iBlockType == BT_NORMAL)
       	m_pie->write("</p>\n");

	else if(m_iBlockType == BT_HEADING1)
		m_pie->write("</h1>\n");

        else if(m_iBlockType == BT_HEADING2)
		m_pie->write("</h2>\n");

        else if(m_iBlockType == BT_HEADING3)
		m_pie->write("</h3>\n");

        else if(m_iBlockType == BT_BLOCKTEXT)
		m_pie->write("</blockquote>\n");

	else if(m_iBlockType == BT_PLAINTEXT)
		m_pie->write("</pre>\n");

        // Add "catchall" for now

	else
	  m_pie->write("</p>\n");

	m_bInBlock = UT_FALSE;
	return;
}

void s_HTML_Listener::_openParagraph(PT_AttrPropIndex api)
{
	if (!m_bInSection)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	UT_Bool wasWritten = UT_FALSE;
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

                if (
			(pAP->getAttribute((XML_Char*)"style", szValue))
			)
		{

                        if(0 == UT_stricmp(szValue, "Heading 1")) 
			{

				// <p style="Heading 1"> ...

				m_iBlockType = BT_HEADING1;
				m_pie->write("<h1");
				wasWritten = UT_TRUE;
			}
			else if(0 == UT_stricmp(szValue, "Heading 2")) 
			{

				// <p style="Heading 2"> ...

				m_iBlockType = BT_HEADING2;
				m_pie->write("<h2");
				wasWritten = UT_TRUE;

			}
			else if(0 == UT_stricmp(szValue, "Heading 3")) 
			{
	
				// <p style="Heading 3"> ...

				m_iBlockType = BT_HEADING3;
				m_pie->write("<h3");
				wasWritten = UT_TRUE;

			}
			else if(0 == UT_stricmp(szValue, "Block Text"))
			{
				// <p style="Block Text"> ...

				m_iBlockType = BT_BLOCKTEXT;
				m_pie->write("<blockquote");
				wasWritten = UT_TRUE;

			}
			else if(0 == UT_stricmp(szValue, "Plain Text"))
			{
				// <p style="Plain Text"> ...

				m_iBlockType = BT_PLAINTEXT;
				m_pie->write("<pre");
				wasWritten = UT_TRUE;
			}
			else 
			{

				// <p style="<anything else!>"> ...

			        m_iBlockType = BT_NORMAL;
			      	m_pie->write("<p class=\"norm\"");
				wasWritten = UT_TRUE;
			}	
		}
		else 
		{

			// <p> with no style attribute ...

		  m_iBlockType = BT_NORMAL;
		  m_pie->write("<p class=\"norm\"");
		  wasWritten = UT_TRUE;
		}

		/* Assumption: never get property set with block text, plain text. Probably true. */

		if (
			m_iBlockType != BT_PLAINTEXT && m_iBlockType != BT_BLOCKTEXT && (pAP->getProperty((XML_Char*)"text-align", szValue))
			)
		{
			m_pie->write(" align=\"");
			m_pie->write((char*)szValue);
			m_pie->write("\"");
		}
	}
	else 
	{

		// <p> with no style attribute, and no properties either

	  m_iBlockType = BT_NORMAL;
	  m_pie->write("<p class=\"norm\"");
	  wasWritten = UT_TRUE;
	}
	if (wasWritten)
	  m_pie->write(">");

	m_bInBlock = UT_TRUE;
}

void s_HTML_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
	m_pie->write("<div>\n");
}

void s_HTML_Listener::_convertColor(char* szDest, const char* pszColor)
{
	/*
	  TODO we might want to be a little more careful about this.
	  The proper HTML color is #rrggbb, which is basically the same
	  as what we use this.  HTML browsers are likely to be more
	  forgiving than we are, so this is probably not a big
	  problem.
	*/
	strcpy(szDest, pszColor);
}

void s_HTML_Listener::_convertFontSize(char* szDest, const char* pszFontSize)
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

void s_HTML_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	UT_Bool span = UT_FALSE;
	UT_Bool textD = UT_FALSE;
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
		        if (!span)
			  {
			    m_pie->write("<span style=\"font-weight: bold");	
			    span = UT_TRUE;
			  }
			else
			  {
			    m_pie->write("; font-weight: bold");
			  }
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
		  if (!span)
			  {
			    m_pie->write("<span style=\"font-style: italic\"");	
			    span = UT_TRUE;
			  }
			else
			  {
			    m_pie->write("; font-style: italic");
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
				if (0 == UT_stricmp(q, "underline"))
				{
				  if (!span)
				    {
					m_pie->write("<span style=\"text-decoration: underline");	
					span = UT_TRUE;
					textD = UT_TRUE;
				    }
				  else if (!textD)
				    {
				        m_pie->write("; text-decoration: underline");
					textD = UT_TRUE;
				    }
				  else
				    {
				      m_pie->write(" underline");
				    }
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
				if (0 == UT_stricmp(q, "line-through"))
				{
				  if (!span)
				    {
					m_pie->write("<span style=\"text-decoration: line-through");	
					span = UT_TRUE;
					textD = UT_TRUE;
				    }
				  else if (!textD)
				    {
				        m_pie->write("; text-decoration: line-through");
					textD = UT_TRUE;
				    }
				  else
				    {
				        m_pie->write(" line-through");
				    }
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
				if (0 == UT_stricmp(q, "overline"))
				{
				  if (!span)
				    {
					m_pie->write("<span style=\"text-decoration: overline");	
					span = UT_TRUE;
					textD = UT_TRUE;
				    }
				  else if (!textD)
				    {
				        m_pie->write("; text-decoration: overline");
					textD = UT_TRUE;
				    }
				  else
				    {
				        m_pie->write(" overline");
				    }
				}

				q = strtok(NULL, " ");
			}

			free(p);
		}

		if (pAP->getProperty((XML_Char*)"text-position", szValue))
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
			(pAP->getProperty((XML_Char*)"color", szValue))
		    || (pAP->getProperty((XML_Char*)"font-size", szValue))
		    || (pAP->getProperty((XML_Char*)"font-family", szValue))
			)
		{
			const XML_Char* pszColor = NULL;
			const XML_Char* pszFontSize = NULL;
			const XML_Char* pszFontFamily = NULL;

			pAP->getProperty((XML_Char*)"color", pszColor);
		    pAP->getProperty((XML_Char*)"font-size", pszFontSize);
		    pAP->getProperty((XML_Char*)"font-family", pszFontFamily);

			if (pszColor)
			{
			  if (!span)
				    {
					m_pie->write("<span style=\"color: ");	
					char szColor[16];
					_convertColor(szColor,(char*)pszColor);
					m_pie->write(szColor);
					m_pie->write(";");
					span = UT_TRUE;
				    }
				  else 
				    {
					m_pie->write(" color: ");	
					char szColor[16];
					_convertColor(szColor,(char*)pszColor);
					m_pie->write(szColor);
					m_pie->write(";");
				    }
			}
			
			if (pszFontFamily)
			{
				  if (!span)
				    {
					m_pie->write("<span style=\"font-family: ");	
					m_pie->write((char*)pszFontFamily);
					m_pie->write(";");
					span = UT_TRUE;
				    }
				  else 
				    {
					m_pie->write(" font-family: ");	
					m_pie->write((char*)pszFontFamily);
					m_pie->write(";");
				    }

			}
			
			char szSize[4];

			if (pszFontSize)
			{
				  if (!span)
				    {
					m_pie->write("<span style=\"font-size: ");	
					sprintf(szSize, "%f", UT_convertToPoints(pszFontSize));
					m_pie->write(szSize);
					m_pie->write("pt;");
					span = UT_TRUE;
				    }
				  else 
				    {
					m_pie->write(" font-size: ");	
					sprintf(szSize, "%f", UT_convertToPoints(pszFontSize));
					m_pie->write(szSize);
					m_pie->write("pt;");
				    }

			}

		}
		
		if (span)
		  m_pie->write("\">");

		m_bInSpan = UT_TRUE;
		m_pAP_Span = pAP;
	}
}

void s_HTML_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{

		UT_Bool closeSpan = UT_FALSE;
		const XML_Char * szValue;
		
		if (
			(pAP->getProperty((XML_Char*)"color", szValue))
		    || (pAP->getProperty((XML_Char*)"font-size", szValue))
		    || (pAP->getProperty((XML_Char*)"font-family", szValue))
			)
		{
		  closeSpan = UT_TRUE;
		}

		if (pAP->getProperty((XML_Char*)"text-position", szValue))
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
			(pAP->getProperty((XML_Char*)"text-decoration", szValue))
			&& UT_stricmp(szValue, "none")
			)
		{
		  closeSpan = UT_TRUE;
		}

		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
		  closeSpan = UT_TRUE;
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_stricmp(szValue, "bold")
			)
		{
		  closeSpan = UT_TRUE;
		}

		if (closeSpan)
		  {
		    m_pie->write("</span>");
		  }

		m_pAP_Span = NULL;
	}

	m_bInSpan = UT_FALSE;
	return;
}

void s_HTML_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
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
			*pBuf++ = '<';				// these get mapped to <br />
			*pBuf++ = 'b';
			*pBuf++ = 'r';
			*pBuf++ = ' ';
			*pBuf++ = '/';
			*pBuf++ = '>';
			pData++;
			break;
			
		default:
			if (*pData > 0x007f)
			{
				UT_UCSChar c = XAP_EncodingManager::instance->try_UToNative(*pData);
				if (c!=0 && c<256)
				{
					*pBuf++ = (unsigned char)c;
				}
				else
				{
					char localBuf[20];
					char * plocal = localBuf;
					sprintf(localBuf,"&#%d;",*pData);
					while (*plocal)
						*pBuf++ = (UT_Byte)*plocal++;
				};
				pData++;
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

s_HTML_Listener::s_HTML_Listener(PD_Document * pDocument,
										 IE_Exp_HTML * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = UT_FALSE;
	m_bInBlock = UT_FALSE;
	m_bInSpan = UT_FALSE;
	
	m_pie->write("<!-- ================================================================================  -->\n");
	m_pie->write("<!-- This HTML file was created by AbiWord.                                            -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor.                                    -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com                -->\n");
	m_pie->write("<!-- ================================================================================  -->\n");
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
	
	m_pie->write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml/DTD/xhtml1-strict.dtd\">\n");
	m_pie->write("<html>\n");
	m_pie->write("<head>\n");
	m_pie->write("<meta http-equiv=\"content-type\" content=\"text/html;charset=");
	m_pie->write(XAP_EncodingManager::instance->getNativeEncodingName());
	m_pie->write("\"/>\n");
	m_pie->write("<title>AbiWord Document</title>\n");
	m_pie->write("<style type=\"text/css\">\n");
	m_pie->write("<!-- \n P.norm { margin-top: 0pt; margin-bottom: 0pt } \n -->\n");
	m_pie->write("</style>\n");
	m_pie->write("</head>\n");
	m_pie->write("<body>\n");

}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();
	
	m_pie->write("</body>\n");
	m_pie->write("</html>\n");
}

UT_Bool s_HTML_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_HTML_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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

UT_Bool s_HTML_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_HTML_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_HTML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_HTML::_writeDocument(void)
{
	m_pListener = new s_HTML_Listener(m_pDocument,this);
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

void s_HTML_Listener::_handleDataItems(void)
{
	/*
	  We *could* handle these by creating separate files with GIF/JPG
	  images in them, and inlining IMG tags into the HTML.  TODO
	*/
	
#if 0
	UT_Bool bWroteOpenDataSection = UT_FALSE;

	const char * szName;
	const UT_ByteBuf * pByteBuf;

	UT_ByteBuf bb64(1024);

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,NULL)); k++)
	{
		if (!bWroteOpenDataSection)
		{
			m_pie->write("<data>\n");
			bWroteOpenDataSection = UT_TRUE;
		}

		if (UT_Base64Encode(&bb64, pByteBuf))
		{
			m_pie->write("<d name=\"");
			m_pie->write(szName);
			m_pie->write("\">\n");

			// TODO for now just spat the whole thing, later we'll want to
			// TODO line wrap it for readability -- just like mime.

			m_pie->write((const char *)bb64.getPointer(0),bb64.getLength());
			
			m_pie->write("\n</d>\n");
		}
	}

	if (bWroteOpenDataSection)
		m_pie->write("</data>\n");

	return;
#endif	
}

