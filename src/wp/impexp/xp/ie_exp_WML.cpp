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

#include "xap_EncodingManager.h"

#include "ut_string_class.h"

// first declare the listener

/*****************************************************************/
/*****************************************************************/

class s_WML_Listener : public PL_Listener
{
public:
	s_WML_Listener(PD_Document * pDocument,
		       IE_Exp_WML * pie);
	virtual ~s_WML_Listener();

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
	
	PD_Document *		m_pDocument;
	IE_Exp_WML *		m_pie;
	bool			m_bInBlock;
	bool			m_bInSpan;
        bool                 m_bWasSpace;

	const PP_AttrProp*	m_pAP_Span;
};

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Exp_WML_Sniffer * m_sniffer = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_WML_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "WML Exporter";
	mi->desc = "Export WML Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
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

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return SUPPORTS_ABI_VERSION(major, minor, release);
}

#endif

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

bool IE_Exp_WML_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".wml"));
}

UT_Error IE_Exp_WML_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_WML * p = new IE_Exp_WML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_WML_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "WML (.wml)";
	*pszSuffixList = "*.wml";
	*ft = getFileType();
	return true;
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
	m_bInBlock = false;
	m_bInSpan = false;
	m_bWasSpace = false;

	m_pie->write("<!DOCTYPE wml PUBLIC \"-//PHONE.COM//DTD WML 1.1//EN\"\n");
	m_pie->write("\t\"http://www.phone.com/dtd/wml11.dtd\" >\n");

	/* keep ads to a minimum. size is at a premium */
	m_pie->write("<!-- This WML file was created by AbiWord -->\n");
	m_pie->write("<!-- See http://www.abisource.com/ -->\n\n");

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
	m_bInBlock = false;
	return;
}

void s_WML_Listener::_openParagraph(PT_AttrPropIndex api)
{
        UT_DEBUGMSG(("OpenParagraph called!\n"));

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		m_pie->write("<p");
		if (pAP->getProperty("text-align", szValue))
		{
		  if (!UT_strcmp(szValue, "center")) 
			m_pie->write(" align=\"center\"");
		  else if (!UT_strcmp(szValue, "right"))
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

	m_bInBlock = true;
}

/*****************************************************************/
/*****************************************************************/

bool s_WML_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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
#if 0			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// TODO we *could* insert the images and create separate WBMP files.
				return true;

			case PTO_Field:
				// we do nothing with computed fields.
				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
#else
			return true;
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_WML_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
				      const PX_ChangeRecord * pcr,
				      PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		return true;

	case PTX_SectionHdrFtr:
		return true;

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

bool s_WML_Listener::change(PL_StruxFmtHandle /*sfh*/,
			       const PX_ChangeRecord * /*pcr*/)
{
  UT_ASSERT(0);	    // this function is not used.
  return false;
}

bool s_WML_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
				     const PX_ChangeRecord * /*pcr*/,
				     PL_StruxDocHandle /*sdh*/,
				     PL_ListenerId /* lid */,
				     void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
								 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
  UT_ASSERT(0);	    // this function is not used.
  return false;
}

bool s_WML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
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
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
		  m_pie->write("<b>");
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
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
				      m_pie->write("<u>");
				}

				q = strtok(NULL, " ");
			}
			free(p);
		}

		// In my WML world...
		// superscript = big
		// subscript = small
		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("<big>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("<small>");
			}
		}
		
		m_bInSpan = true;
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
	
	UT_String sBuf;
	const UT_UCSChar * pData;

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
			sBuf += "<br/>";
			pData++;
			break;

		case ' ':
		case '\t':
		  // try to honor multiple spaces
		  // tabs get treated as a single space
		  //
		  if(m_bWasSpace)
		    {
				sBuf += "&nbsp;";
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

	m_pie->write(sBuf.c_str(),sBuf.size());
}

void s_WML_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{

		const XML_Char * szValue;
		
		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("</big>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("</small>");
			}
		}

		if ((pAP->getProperty("text-decoration", szValue)))
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
				      m_pie->write("</u>");
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
		  m_pie->write("</i>");
		}
		
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
		  m_pie->write("</b>");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}
