/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
#include "ut_wctomb.h"
#include "pt_Types.h"
#include "ie_exp_HRText.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "ut_debugmsg.h"
#include "ut_string_class.h"

// our delimiters
#define BOLD_DELIM           "*"
#define ITALIC_DELIM         "/"
#define UNDERLINE_DELIM      "_"
#define UNDERLINE_DELIM_CHAR '_'
#define LIST_DELIM           "* "
#define BLOCK_DELIM          "| "
#define PLAIN_DELIM          "| "
#define SUPERSCRIPT_DELIM    "^"
#define SUBSCRIPT_DELIM      "_"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Exp_HRText_Sniffer * m_sniffer = 0;
static UT_sint32 m_refs = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_refs && !m_sniffer)
	{
		m_sniffer = new IE_Exp_HRText_Sniffer ();
		m_refs++;
	}
	else if (m_refs && m_sniffer)
	{
		m_refs++;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	mi->name = "HRText Exporter";
	mi->desc = "Export HRText Documents";
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

	UT_ASSERT (m_refs && m_sniffer);

	m_refs--;
	IE_Exp::unregisterExporter (m_sniffer);
	if (!m_refs)
	{
		delete m_sniffer;
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

bool IE_Exp_HRText_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".abw") == 0);
}

UT_Error IE_Exp_HRText_Sniffer::constructExporter(PD_Document * pDocument,
												  IE_Exp ** ppie)
{
	IE_Exp_HRText * p = new IE_Exp_HRText(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_HRText_Sniffer::getDlgLabels(const char ** pszDesc,
										 const char ** pszSuffixList,
										 IEFileType * ft)
{
	*pszDesc = "Human Readable Text (.txt)";
	*pszSuffixList = "*.txt";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_HRText::IE_Exp_HRText(PD_Document * pDocument)
	: IE_Exp(pDocument), m_pListener(0)
{
}

IE_Exp_HRText::~IE_Exp_HRText()
{
}

/*****************************************************************/
/*****************************************************************/

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6
#define BT_NUMBEREDLIST	7
#define BT_BULLETLIST	8

class s_HRText_Listener : public PL_Listener
{
public:
	s_HRText_Listener(PD_Document * pDocument,
						IE_Exp_HRText * pie);
	virtual ~s_HRText_Listener();

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
	void				_closeTag(void);
	void				_closeSpan(void);
	void				_openTag(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_handleDataItems(void);
	void				_convertFontSize(char* szDest, const char* pszFontSize);
	void				_convertColor(char* szDest, const char* pszColor);
	
	PD_Document *		m_pDocument;
	IE_Exp_HRText *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bNextIsSpace;
	bool				m_bInList;
	const PP_AttrProp*	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	char			m_iDecoration;
	UT_uint16		m_iBlockType;	// BT_*
	UT_uint16		m_iListDepth;	// 0 corresponds to not in a list
	UT_Wctomb		m_wctomb;
};

void s_HRText_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_bInSection = false;
	return;
}

void s_HRText_Listener::_closeTag(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	m_pie->write("\n\n");

	m_bInBlock = false;
	return;
}

void s_HRText_Listener::_openTag(PT_AttrPropIndex api)
{
	if (!m_bInSection)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;
		//const XML_Char * szLevel;
		const XML_Char * szListID;

		if (
		   (pAP->getAttribute((XML_Char*)"style", szValue))
		   )
		{
			if(pAP->getAttribute((XML_Char*)"listid", szListID) &&
			   0 != UT_strcmp(szListID, "0"))
			{	// we're in a list
				m_pie->write(LIST_DELIM);
			}
			else 
			{
				if(0 == UT_strcmp(szValue, "Block Text"))
				{
					// <p style="Block Text"> ...

					m_iBlockType = BT_BLOCKTEXT;
					m_pie->write(BLOCK_DELIM);
				}
				else if(0 == UT_strcmp(szValue, "Plain Text"))
				{
					// <p style="Plain Text"> ...

					m_iBlockType = BT_PLAINTEXT;
					m_pie->write(PLAIN_DELIM);
				}
			}
		}
	}
	m_bInBlock = true;
}

void s_HRText_Listener::_openSection(PT_AttrPropIndex /* api*/)
{
	m_pie->write("\n");
}

void s_HRText_Listener::_openSpan(PT_AttrPropIndex api)
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
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			m_pie->write(BOLD_DELIM);
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			m_pie->write(ITALIC_DELIM);
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
		    	XML_Char*       q = strtok(p, " ");

		    	while (q)
		    	{
				  if (0 == UT_strcmp(q, "underline"))
				  {
					  m_iDecoration = UNDERLINE_DELIM_CHAR;
					  m_pie->write(UNDERLINE_DELIM);
				  }

				  q = strtok(NULL, " ");
		    }

		    free(p);
		}

		if (pAP->getProperty((XML_Char*)"text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write(SUPERSCRIPT_DELIM);
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write(SUBSCRIPT_DELIM);
			}
		}
		
		m_bInSpan = true;
		m_pAP_Span = pAP;
	}
}

void s_HRText_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{

		const XML_Char * szValue;
		
		if (
			(pAP->getProperty((XML_Char*)"text-decoration", szValue))
			&& UT_strcmp(szValue, "none")
			)
		{
			if (m_iDecoration)
		  		m_pie->write(&m_iDecoration, 1);
		}

		if (
			(pAP->getProperty((XML_Char*)"font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
		  m_pie->write(ITALIC_DELIM);
		}
		
		if (
			(pAP->getProperty((XML_Char*)"font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
		  m_pie->write(BOLD_DELIM);
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}

void s_HRText_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String sBuf;
	const UT_UCSChar * pData;
	
	int mbLen;
	char pC[MB_LEN_MAX];
	
	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		if(!m_wctomb.wctomb(pC,mbLen,(wchar_t)*pData))
		{
		    mbLen=1;
		    pC[0]='?';
		    m_wctomb.initialize();
		}
		pData++;		
		if (mbLen>1)		
		{
			sBuf += pC;
		}
		else
		{
			sBuf += (char)pC[0];
		}
	}

	m_pie->write(sBuf.c_str(),sBuf.size());
}

s_HRText_Listener::s_HRText_Listener(PD_Document * pDocument,
										 IE_Exp_HRText * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	m_bNextIsSpace = false;
	m_bInList = false;
	m_iListDepth = 0;
	m_iDecoration = 0;
	
}

s_HRText_Listener::~s_HRText_Listener()
{
	_closeSpan();
	_closeTag();
	_closeSection();
	_handleDataItems();
}

bool s_HRText_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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
			return true;
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_HRText_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
		_closeTag();
		_closeSection();

		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDocument->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute((XML_Char*)"type", pszSectionType);
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
		_closeTag();
		_openTag(pcr->getIndexAP());
		return true;
	}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_HRText_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_HRText_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

bool s_HRText_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_HRText::_writeDocument(void)
{
	m_pListener = new s_HRText_Listener(m_pDocument,this);
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

void s_HRText_Listener::_handleDataItems(void)
{
	/* Not much we can do with these in text. */
}

