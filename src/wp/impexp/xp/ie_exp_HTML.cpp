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

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
	m_lid = 0;
}

IE_Exp_HTML::~IE_Exp_HTML()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_HTML::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".html") == 0);
}

IEStatus IE_Exp_HTML::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	*ppie = p;
	return IES_OK;
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

IEStatus IE_Exp_HTML::writeFile(const char * szFilename)
{
	UT_ASSERT(m_pDocument);
	UT_ASSERT(szFilename && *szFilename);

	if (!_openFile(szFilename))
		return IES_CouldNotOpenForWriting;

	IEStatus status = _writeDocument();
	if (status == IES_OK)
		_closeFile();
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return status;
}

void IE_Exp_HTML::write(const char * sz)
{
	if (m_error)
		return;
	m_error |= ! _writeBytes((UT_Byte *)sz);
	return;
}

void IE_Exp_HTML::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;
	if (_writeBytes((UT_Byte *)sz,length) != length)
		m_error = UT_TRUE;
	
	return;
}

/*****************************************************************/
/*****************************************************************/

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

	m_pie->write("</p>\n");
	m_bInBlock = UT_FALSE;
	return;
}

void s_HTML_Listener::_openParagraph(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	m_pie->write("<P");
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (
			(pAP->getProperty("text-align", szValue))
			)
		{
			m_pie->write(" ALIGN=\"");
			m_pie->write(szValue);
			m_pie->write("\"");
		}
	}

	m_pie->write(">");
}

void s_HTML_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
	m_pie->write("<DIV>\n");
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
			m_pie->write("<i>");
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;

			XML_Char* p;
			if (!UT_cloneString(p, pszDecor))
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
			if (!UT_cloneString(p, pszDecor))
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

void s_HTML_Listener::_closeSpan(void)
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

		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString(p, pszDecor))
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
			if (!UT_cloneString(p, pszDecor))
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

void s_HTML_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
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
			
		default:
			if (*pData > 0x007f)
			{
				// convert non us-ascii into numeric entities.
				// we could convert them into UTF-8 multi-byte
				// sequences, but i prefer these.
				char localBuf[20];
				char * plocal = localBuf;
				sprintf(localBuf,"&#x%x;",*pData++);
				while (*plocal)
					*pBuf++ = (UT_Byte)*plocal++;
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
	m_pie->write("<!-- This HTML file was created by AbiWord.                                             -->\n");
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
	
	m_pie->write("<html>\n");
}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();
	
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
			_openSection(pcr->getIndexAP());
			m_bInSection = UT_TRUE;
			return UT_TRUE;
		}

	case PTX_Block:
		{
			_closeSpan();
			_closeBlock();
			_openParagraph(pcr->getIndexAP());
			m_bInBlock = UT_TRUE;
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

IEStatus IE_Exp_HTML::_writeDocument(void)
{
	m_pListener = new s_HTML_Listener(m_pDocument,this);
	if (!m_pListener)
		return IES_NoMemory;
	if (!m_pDocument->addListener(static_cast<PL_Listener *>(m_pListener),&m_lid))
		return IES_Error;
	m_pDocument->removeListener(m_lid);
	delete m_pListener;

	m_lid = 0;
	m_pListener = NULL;
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
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
