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

#include <locale.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "ie_exp_XSL-FO.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pp_Property.h"
#include "xap_App.h"
#include "ap_Prefs.h"
#include "pd_Style.h"
#include "fd_Field.h"
#include "xap_EncodingManager.h"
#include "fl_AutoNum.h"
#include "fp_PageSize.h"

#include "ut_string_class.h"

/*****************************************************************/
/*****************************************************************/

class s_XSL_FO_Listener : public PL_Listener
{
public:
	s_XSL_FO_Listener(PD_Document * pDocument,
					  IE_Exp_XSL_FO * pie);
	virtual ~s_XSL_FO_Listener();

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

	void                _handlePageSize(PT_AttrPropIndex api);
	void				_handleDataItems(void);
	void                _outputData(const UT_UCSChar * data, UT_uint32 length);

	void				_convertFontSize(char* szDest, const char* szFontSize);
	void                _convertColor(char* szDest, const char* pszColor);

	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openBlock(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);


	PD_Document *		m_pDocument;
	IE_Exp_XSL_FO *	    m_pie;

	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bFirstWrite;

	int                 m_iImgCnt;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_XSL_FO::IE_Exp_XSL_FO(PD_Document * pDocument)
	: IE_Exp(pDocument), m_pListener(0)
{
	m_error = UT_OK;
}

IE_Exp_XSL_FO::~IE_Exp_XSL_FO()
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_XSL_FO::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".fo") == 0);
}

UT_Error IE_Exp_XSL_FO::StaticConstructor(PD_Document * pDocument,
										  IE_Exp ** ppie)
{
	IE_Exp_XSL_FO * p = new IE_Exp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Exp_XSL_FO::GetDlgLabels(const char ** pszDesc,
									const char ** pszSuffixList,
									IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = IEFT_XSL_FO;
	return true;
}

bool IE_Exp_XSL_FO::SupportsFileType(IEFileType ft)
{
	return (IEFT_XSL_FO == ft);
}

UT_Error IE_Exp_XSL_FO::_writeDocument(void)
{
	m_pListener = new s_XSL_FO_Listener(m_pDocument,this);
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

s_XSL_FO_Listener::s_XSL_FO_Listener(PD_Document * pDocument,
									 IE_Exp_XSL_FO * pie)
	: m_pDocument (pDocument), m_pie (pie), 
	m_bInSection(false), m_bInBlock(false), 
	m_bInSpan(false), m_bFirstWrite(true),
	m_iImgCnt(0)
{
	// Be nice to XML apps.  See the notes in _outputData() for more 
	// details on the charset used in our documents.  By not declaring 
	// any encoding, XML assumes we're using UTF-8.  Note that US-ASCII 
	// is a strict subset of UTF-8. 

	if (!XAP_EncodingManager::instance->cjk_locale() &&
	    (XAP_EncodingManager::instance->try_nativeToU(0xa1) != 0xa1)) {
	    // use utf8 for CJK locales and latin1 locales and unicode locales
	    m_pie->write("<?xml version=\"1.0\" encoding=\"");
	    m_pie->write(XAP_EncodingManager::instance->getNativeEncodingName());
	    m_pie->write("\"?>\n");
	} else {
	    m_pie->write("<?xml version=\"1.0\"?>\n");
	}

	m_pie->write("<fo:root xmlns:fo=\"http://www.w3.org/1999/XSL/Format\">\n\n");

	m_pie->write("<!-- This document was created by AbiWord -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor. -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com -->\n\n");
}

s_XSL_FO_Listener::~s_XSL_FO_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();

	m_pie->write ("</fo:root>\n");
}

bool s_XSL_FO_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								 const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = 
				static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			if (api)
			{
				_openSpan(api);
			}
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			if (api)
			{
				_closeSpan();
			}
			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();

			switch (pcro->getObjectType())
			{
			case PTO_Image:
			{
				char buf[16];
				sprintf(buf, "%d.png", m_iImgCnt++);
				m_pie->write("<fo:external-graphic src=\"");
				m_pie->write(m_pie->getFileName());
				m_pie->write(buf);
				m_pie->write("\"/>\n");
				return true;
			}

			case PTO_Field:
			{
				return true;
			}

			default:
			{
				UT_ASSERT(0);
				return false;
			}

			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_XSL_FO_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
		// TODO???
		return true;
	}
	
	case PTX_Block:
	{
		_closeSpan();
		_closeBlock();
		_openBlock(pcr->getIndexAP());
		return true;
	}
	
	default:
	{
		UT_ASSERT(0);
		return false;
	}

	}
}

bool s_XSL_FO_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

bool s_XSL_FO_Listener::change(PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_XSL_FO_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

/*****************************************************************/
/*****************************************************************/

static const char *
docUnitToString(fp_PageSize::Unit docUnit)
{
	if(docUnit == fp_PageSize::cm)
		return "cm";
	else if(docUnit == fp_PageSize::mm)
		return "mm";
	else if(docUnit == fp_PageSize::inch)
		return "in";
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return "";
	}
}

void s_XSL_FO_Listener::_handlePageSize(PT_AttrPropIndex api)
{
  //
  // Code to write out the PageSize Definitions to disk
  // 
	char *old_locale;

	char buf[20];

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	old_locale = setlocale (LC_NUMERIC, "C");

	m_pie->write("<fo:layout-master-set>\n");
	m_pie->write("<fo:simple-page-master");

	// query and output properties
	// todo - validate these and make sure they all make sense
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		szValue = PP_evalProperty("page-margin-top",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write(" margin-top=\"");
		m_pie->write(szValue);
		m_pie->write("\"");

		szValue = PP_evalProperty("page-margin-bottom",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write(" margin-bottom=\"");
		m_pie->write(szValue);
		m_pie->write("\"");

		szValue = PP_evalProperty("page-margin-left",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write(" margin-left=\"");
		m_pie->write(szValue);
		m_pie->write("\"");

		szValue = PP_evalProperty("page-margin-right",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write(" margin-right=\"");
		m_pie->write(szValue);
		m_pie->write("\"");
		
		fp_PageSize::Unit  docUnit = m_pDocument->m_docPageSize.getUnit(); 
		char buf[20];

		m_pie->write( " page-width=\"");
		sprintf((char *) buf,"%f",m_pDocument->m_docPageSize.Width(docUnit));
		m_pie->write((char *)buf);
		m_pie->write(docUnitToString(docUnit));
		m_pie->write("\"");

		m_pie->write(" page-height=\"");
		sprintf((char *) buf,"%f",m_pDocument->m_docPageSize.Height(docUnit));
		m_pie->write((char *)buf);
		m_pie->write(docUnitToString(docUnit));
		m_pie->write("\"");
		
	}
	// page-width, page-height

	m_pie->write(" master-name=\"first\"");

	m_pie->write("/>\n");
	m_pie->write("\t<fo:region-body/>\n");
	m_pie->write("</fo:simple-page-master>\n\n");

	setlocale (LC_NUMERIC, old_locale);

	m_bFirstWrite = false;
	return;
}

void s_XSL_FO_Listener::_handleDataItems(void)
{
	const char * szName;
   	const char * szMimeType;
	const UT_ByteBuf * pByteBuf;

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,(void**)&szMimeType)); k++)
	{	  	  
	  FILE *fp;
	  char fname [1024]; // EVIL EVIL bad hardcoded buffer size
	  
	  if (!UT_strcmp(szMimeType, "image/svg-xml"))
	      sprintf(fname, "%s-%d.svg", m_pie->getFileName(), k);
	  if (!UT_strcmp(szMimeType, "text/mathml"))
	    sprintf(fname, "%s-%d.mathml", m_pie->getFileName(), k);
	  else // PNG Image
	    sprintf(fname, "%s-%d.png", m_pie->getFileName(), k);
	  
	  fp = fopen (fname, "wb+");
	  
	  if(!fp)
	    continue;
	  
	  int cnt = 0, len = pByteBuf->getLength();
	  
	  while (cnt < len)
	    {
	      xxx_UT_DEBUGMSG(("DOM: len: %d cnt: %d\n", len, cnt));
	      cnt += fwrite (pByteBuf->getPointer(cnt), sizeof(UT_Byte), len-cnt, fp);
	    }
	  
	  fclose(fp);
	}
	
	return;
}

void s_XSL_FO_Listener::_openSection(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_handlePageSize(api);
	}

	m_bInSection = true;

	m_pie->write("<fo:page-sequence master-name=\"first\">\n");
	m_pie->write("<fo:flow>\n");
}

#define BLOCK() do {if(block) m_pie->write(" "); else m_pie->write(" "); block = true;} while (0)

void s_XSL_FO_Listener::_openBlock(PT_AttrPropIndex api)
{
	if (!m_bInSection)
	{
		return;
	}

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	m_bInBlock = true;
	m_pie->write("<fo:block");

	// keep track of whether we have we written anything
	bool block = false;

	// query and output properties
	// todo - validate these and make sure they all make sense
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getProperty("bgcolor", szValue))
		{
			BLOCK();
			m_pie->write("background-color=\"#");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("color", szValue))
		{
			BLOCK();
			m_pie->write("color=\"#");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("lang", szValue))
		{
			BLOCK();
			m_pie->write("language=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}
		
		if (pAP->getProperty("font-size", szValue))
		{
			BLOCK();
			m_pie->write("font-size=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}		

		if (pAP->getProperty("font-family", szValue))
		{
			BLOCK();
			m_pie->write("font-family=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-weight", szValue))
		{
			BLOCK();
			m_pie->write("font-weight=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-style", szValue))
		{
			BLOCK();
			m_pie->write("font-style=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-stretch", szValue))
		{
			BLOCK();
			m_pie->write("font-stretch=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("keep-together", szValue))
		{
			BLOCK();
			m_pie->write("keep-together=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("keep-with-next", szValue))
		{
			BLOCK();
			m_pie->write("keep-with-next=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("line-height", szValue))
		{
			BLOCK();
			m_pie->write("line-height=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("margin-bottom", szValue))
		{
			BLOCK();
			m_pie->write("margin-bottom=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("margin-top", szValue))
		{
			BLOCK();
			m_pie->write("margin-top=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("margin-left", szValue))
		{
			BLOCK();
			m_pie->write("margin-left=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("margin-right", szValue))
		{
			BLOCK();
			m_pie->write("margin-right=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("windows", szValue))
		{
			BLOCK();
			m_pie->write("windows=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}
	}

	m_pie->write(">\n");
}

#undef BLOCK

#define SPAN() do {if(span) m_pie->write(" "); else m_pie->write(" "); span = true;} while (0)

void s_XSL_FO_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
	{
		return;
	}

	m_bInSpan = true;

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	// keep track of whether we've written out anything
	bool span = false;

	m_pie->write("<fo:character");

	// query and output properties
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getProperty("bgcolor", szValue))
		{
			SPAN();
			m_pie->write("background-color=\"#");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("color", szValue))
		{
			SPAN();
			m_pie->write("color=\"#");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("lang", szValue))
		{
			SPAN();
			m_pie->write("language=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}
		
		if (pAP->getProperty("font-size", szValue))
		{
			SPAN();
			m_pie->write("font-size=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}		

		if (pAP->getProperty("font-family", szValue))
		{
			SPAN();
			m_pie->write("font-family=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-weight", szValue))
		{
			SPAN();
			m_pie->write("font-weight=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-style", szValue))
		{
			SPAN();
			m_pie->write("font-style=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("font-stretch", szValue))
		{
			SPAN();
			m_pie->write("font-stretch=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("keep-together", szValue))
		{
			SPAN();
			m_pie->write("keep-together=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("keep-with-next", szValue))
		{
			SPAN();
			m_pie->write("keep-with-next=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("lang", szValue))
		{
			SPAN();
			m_pie->write("language=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

		if (pAP->getProperty("text-decoration", szValue))
		{
			SPAN();
			m_pie->write("text-decoration=\"");
			m_pie->write((const char *)szValue);
			m_pie->write("\"");
		}

	}

	m_pie->write(">");
}

#undef SPAN

void s_XSL_FO_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	m_bInBlock = false;
	m_pie->write("\n</fo:block>\n");
}

void s_XSL_FO_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_bInSection = false;

	m_pie->write("</fo:flow>\n");
	m_pie->write("</fo:page-sequence>\n");
}

void s_XSL_FO_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
	{
		return;
	}

	m_bInSpan = false;

	m_pie->write("</fo:character>");
}

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_convertColor(char* szDest, const char* pszColor)
{
	/*
	 * TODO we might want to be a little more careful about this.
	 * The proper XSL-FO color is #rrggbb, which is basically the same
	 * as what we use this.
	 */
	strcpy(szDest, pszColor);
}

void s_XSL_FO_Listener::_convertFontSize(char* szDest, const char* pszFontSize)
{
	strcpy (szDest, pszFontSize);
}

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String sBuf;
	const UT_UCSChar * pData;

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		switch (*pData)
		{
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
			// TODO
			UT_ASSERT(UT_TODO);
			pData++;
			break;
			
		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			// TODO
			UT_ASSERT(UT_TODO);
			pData++;
			break;
			
		case UCS_FF:					// FF -- representing a Forced-Page-Break
			// TODO:
			UT_ASSERT(UT_TODO);
			pData++;
			break;
			
		default:

			if (*pData > 0x007f)
			{
				if(XAP_EncodingManager::instance->isUnicodeLocale() || 
				   (XAP_EncodingManager::instance->try_nativeToU(0xa1) == 0xa1))

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
					UT_UCSChar c = XAP_EncodingManager::instance->try_UToNative(*pData);
					if (c==0 || c>255)
					{
						char localBuf[20];
						char * plocal = localBuf;
						sprintf(localBuf,"&#x%x;",*pData++);
						sBuf += plocal;
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

	m_pie->write(sBuf.c_str(), sBuf.size());
}
