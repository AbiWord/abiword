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
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "ie_exp_WML.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"

// first declare the listener

/*****************************************************************/
/*****************************************************************/

class s_WML_Listener : public PL_Listener
{
public:
	s_WML_Listener(PD_Document * pDocument,
		       IE_Exp_WML * pie);
	virtual ~s_WML_Listener();

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
	
	PD_Document *		m_pDocument;
	IE_Exp_WML *		m_pie;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	const PP_AttrProp*	m_pAP_Span;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_WML::IE_Exp_WML(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_WML::~IE_Exp_WML()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_WML::RecognizeSuffix(const char * szSuffix)
{
        return (!(UT_stricmp(szSuffix,".wml")));
}

UT_Error IE_Exp_WML::StaticConstructor(PD_Document * pDocument,
				       IE_Exp ** ppie)
{
	IE_Exp_WML * p = new IE_Exp_WML(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_WML::GetDlgLabels(const char ** pszDesc,
				 const char ** pszSuffixList,
				 IEFileType * ft)
{
	*pszDesc = "WML (.wml)";
	*pszSuffixList = "*.wml";
	*ft = IEFT_WML;
	return UT_TRUE;
}

UT_Bool IE_Exp_WML::SupportsFileType(IEFileType ft)
{
	return (IEFT_WML == ft);
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_WML::_writeDocument(void)
{
	m_pListener = new s_WML_Listener(m_pDocument,this);
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

s_WML_Listener::s_WML_Listener(PD_Document * pDocument,
			       IE_Exp_WML * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInBlock = UT_FALSE;
	m_bInSpan = UT_FALSE;
	
	m_pie->write("<?xml version=\"1.0\"?>\n");

	/* keep this to a minimum. size is at a premium */
	m_pie->write("<!-- This WML file was created by AbiWord -->\n");
	m_pie->write("<!-- See http://www.abisource.com -->\n\n");

	m_pie->write("<wml>\n");
	m_pie->write("<card>\n");
}

s_WML_Listener::~s_WML_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();

	m_pie->write("</card>\n");
	m_pie->write("</wml>\n");
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_closeSection(void)
{
  // wml is simple: 1 section per document
  // we could get fancy with cards later
  return;
}

void s_WML_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
  // wml is simple: 1 section per document
  // we could get fancy with cards later
  return;
}

void s_WML_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	m_pie->write("</p>\n");
	m_bInBlock = UT_FALSE;
	return;
}

void s_WML_Listener::_openParagraph(PT_AttrPropIndex api)
{
        UT_DEBUGMSG(("OpenParagraph called!\n"));

	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		m_pie->write("<p");
		if (pAP->getProperty((XML_Char*)"text-align", szValue))
		{
		  if (!UT_stricmp(szValue, "center")) 
			m_pie->write(" align=\"center\"");
		  else if (!UT_stricmp(szValue, "right"))
		        m_pie->write(" align=\"right\"");
		  else //left or block-justify
		        m_pie->write(" align=\"left\"");
		}
		m_pie->write(">");
		
	}
	else 
	{
	  // <p> with no style attribute, and no properties either
	  m_pie->write("<p>");
	}

	m_bInBlock = UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool s_WML_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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
				// TODO we *could* insert the images and create separate WBMP files.
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

UT_Bool s_WML_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
				      const PX_ChangeRecord * pcr,
				      PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		return UT_TRUE;

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

UT_Bool s_WML_Listener::change(PL_StruxFmtHandle /*sfh*/,
			       const PX_ChangeRecord * /*pcr*/)
{
  UT_ASSERT(0);	    // this function is not used.
  return UT_FALSE;
}

UT_Bool s_WML_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
				     const PX_ChangeRecord * /*pcr*/,
				     PL_StruxDocHandle /*sdh*/,
				     PL_ListenerId /* lid */,
				     void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
								 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
  UT_ASSERT(0);	    // this function is not used.
  return UT_FALSE;
}

UT_Bool s_WML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

void s_WML_Listener::_handleDataItems(void)
{
	/*
	  We *could* handle these by creating separate files with WBMP
	  images in them, and inlining IMG tags into the WML. That is,
	  if Abiword ever supports WBMP graphics, which I don't see
	  happening. I mean, first phones will have to properly support
	  them, right :)
	*/
  return;
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_openSpan(PT_AttrPropIndex api)
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
		  m_pie->write("<b>");
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
		  m_pie->write("<i>");
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
				      m_pie->write("<u>");
				}

				q = strtok(NULL, " ");
			}
			free(p);
		}

		// In my WML world...
		// superscript = big
		// subscript = small
		if (pAP->getProperty((XML_Char*)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("<big>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("<small>");
			}
		}
		
		m_bInSpan = UT_TRUE;
		m_pAP_Span = pAP;
	}
}

void s_WML_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
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
				// convert non us-ascii into numeric entities.
				// we could convert them into UTF-8 multi-byte
				// sequences, but i prefer these.
				char localBuf[20];
				char * plocal = localBuf;
				sprintf(localBuf,"&#%d;",*pData++);
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

void s_WML_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{

		const XML_Char * szValue;
		
		if (pAP->getProperty((XML_Char*)"text-position", szValue))
		{
			if (!UT_stricmp("superscript", szValue))
			{
				m_pie->write("</big>");
			}
			else if (!UT_stricmp("subscript", szValue))
			{
				m_pie->write("</small>");
			}
		}

		if ((pAP->getProperty((XML_Char*)"text-decoration", szValue)))
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
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_stricmp(szValue, "italic")
			)
		{
		  m_pie->write("</i>");
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
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
