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

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "ut_map.h"
#include "ut_pair.h"
#include "pt_Types.h"
#include "ie_exp_XSL-FO.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pp_Property.h"
#include "ap_Prefs.h"
#include "pd_Style.h"
#include "fd_Field.h"
#include "xap_EncodingManager.h"
#include "fl_AutoNum.h"
#include "fp_PageSize.h"

#include "ut_string_class.h"

//////////////////////////////////////////////////
// Some helper functions to prevent the use of static buffers.
// may we should add these hacks to ut_string_class
//////////////////////////////////////////////////

UT_String&
operator<< (UT_String& st, int i)
{
  return (st += UT_String_sprintf ("%d", i));
}
 
UT_UCS2String&
operator<< (UT_UCS2String& st, int i)
{
	UT_UCSChar tmp2[16];
	UT_UCS_strcpy_char(tmp2, UT_String_sprintf("%d", i).c_str());
	return (st += tmp2);
}

UT_String&
operator<< (UT_String& st, const UT_String& st2)
{
	st += st2;
	return st;
}

UT_UCS2String&
operator<< (UT_UCS2String& st, const UT_UCS2String& st2)
{
	st += st2;
	return st;
}

/*****************************************************************/
/*****************************************************************/

// A little class to help export lists

class ListHelper
{
public:
	explicit ListHelper(UT_uint32 id = 0)
		: m_pan(0),
		  m_iNextNb(0),
		  m_iInc(0)
	{
		setIdList(id);
	}

	void setIdList(UT_uint32 id)
	{
		UT_Map::Iterator it(myLists.find(reinterpret_cast<UT_Map::key_t> (id)));

		if (it.is_valid())
		{
			m_pan = static_cast<const fl_AutoNum*> (static_cast<const UT_Pair*> (it.value())->second());
			UT_ASSERT(m_pan);
			m_iNextNb = m_pan->getStartValue32();

			if (m_pan->getType() >= BULLETED_LIST)
				m_iInc = 0;
			else
				m_iInc = 1;
		}
	}

	UT_String getNextLabel()
	{
		UT_ASSERT(m_pan);
		UT_uint32 next_nb = m_iNextNb;
		m_iNextNb += m_iInc;
		// TODO: I'm laking pre and post text
		return int2label(next_nb);
	}
	
    static void addList(fl_AutoNum* pAutoNum) 
	{
		myLists.insert(reinterpret_cast<void*> (pAutoNum->getID()), pAutoNum);
	}

private:
	UT_String int2label(UT_uint32 i)
	{
		UT_String retval;
		retval << i;
		return retval;
	}

	const fl_AutoNum* m_pan;
	UT_uint32 m_iNextNb;
	UT_uint32 m_iInc;
    static UT_Map myLists;
};

UT_Map  ListHelper::myLists;

/************************************************************/
/************************************************************/

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
	void				_handleDataItems();
	void				_handleLists();
	void				_handleField(PT_AttrPropIndex api);
	void                _outputData(const UT_UCSChar * data, UT_uint32 length);

	void				_convertFontSize(UT_String& szDest, const char* szFontSize);
	void                _convertColor(UT_String& szDest, const char* pszColor);

	void				_closeSection();
	void				_closeBlock();
	void				_closeSpan();
	void				_openBlock(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);

private:
	PD_Document *		m_pDocument;
	IE_Exp_XSL_FO *	    m_pie;
	ListHelper			m_List;

	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bFirstWrite;
	int                 m_iImgCnt;
};

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("XSL-FO")

// we use a reference-counted sniffer
static IE_Exp_XSL_FO_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
		m_sniffer = new IE_Exp_XSL_FO_Sniffer ();
	else
		m_sniffer->ref();

	mi->name = "XSL-FO Exporter";
	mi->desc = "Export XSL-FO Documents";
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
		m_sniffer = 0;

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

IE_Exp_XSL_FO::IE_Exp_XSL_FO(PD_Document * pDocument)
	: IE_Exp(pDocument),
	  m_pListener(0),
	  m_error(UT_OK)
{
}

IE_Exp_XSL_FO::~IE_Exp_XSL_FO()
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_XSL_FO_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix, ".fo"));
}

UT_Error IE_Exp_XSL_FO_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_XSL_FO * p = new IE_Exp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_XSL_FO_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_XSL_FO::_writeDocument()
{
	m_pListener = new s_XSL_FO_Listener(getDoc(),this);
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

s_XSL_FO_Listener::s_XSL_FO_Listener(PD_Document * pDocument,
									 IE_Exp_XSL_FO * pie)
	: m_pDocument(pDocument),
	  m_pie (pie),
	  m_List(),
	  m_bInSection(false),
	  m_bInBlock(false),
	  m_bInSpan(false),
	  m_bFirstWrite(true),
	  m_iImgCnt(0)
{
	// Be nice to XML apps.  See the notes in _outputData() for more 
	// details on the charset used in our documents.  By not declaring 
	// any encoding, XML assumes we're using UTF-8.  Note that US-ASCII 
	// is a strict subset of UTF-8. 

	if (!XAP_EncodingManager::get_instance()->cjk_locale() &&
	    (XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) != 0xa1)) {
	    // use utf8 for CJK locales and latin1 locales and unicode locales
	    m_pie->write("<?xml version=\"1.0\" encoding=\"");
	    m_pie->write(XAP_EncodingManager::get_instance()->getNativeEncodingName());
	    m_pie->write("\"?>\n");
	} else {
	    m_pie->write("<?xml version=\"1.0\"?>\n");
	}

	m_pie->write("<fo:root xmlns:fo=\"http://www.w3.org/1999/XSL/Format\">\n\n");

	m_pie->write("<!-- This document was created by AbiWord -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor. -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com -->\n\n");

	_handleLists();
}

s_XSL_FO_Listener::~s_XSL_FO_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
	_handleDataItems();

	m_pie->write ("</fo:root>\n");
}

void s_XSL_FO_Listener::_handleLists()
{
	fl_AutoNum* pAutoNum;

	for (UT_uint32 k = 0; m_pDocument->enumLists(k, &pAutoNum); ++k)
	{
		if (pAutoNum->isEmpty() == true)
			continue;

		ListHelper::addList(pAutoNum);
	}
}

void s_XSL_FO_Listener::_handleField(PT_AttrPropIndex api)
{
	const PP_AttrProp* pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char* szValue;
		if (pAP->getAttribute("type", szValue))
		{
			if (szValue[0] == 'l' && strcmp((const char*) szValue, "list_label") == 0)
			{
				m_pie->write("<fo:list-item-label end-indent=\"label-end()\">\n"
							 "  <fo:block>\n");
				m_pie->write(m_List.getNextLabel().c_str());
				m_pie->write("  </fo:block>\n"
							 "</fo:list-item-label>\n");
			}
		}
	}
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

			switch (pcro->getObjectType())
			{
			case PTO_Image:
			{
			        m_pie->write(UT_String_sprintf("<fo:external-graphic src=\"%s-%d.png\"/>\n", m_pie->getFileName(), m_iImgCnt++));
				return true;
			}

			case PTO_Field:
			{
				_handleField(pcr->getIndexAP());
				return true;
			}

				// todo: support these
			case PTO_Hyperlink:
			case PTO_Bookmark:
			  return true;

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
		_closeSpan();
		_closeBlock();
		
//		<fo:static-content flow-name="xsl-region-before">
//		<fo:block>
//		</fo:block>
//		</fo:static-content> 

		
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

static UT_String
purgeSpaces(const char* st)
{
	UT_String retval;

	while (*st != '\0')
	{
		if (*st != ' ')
			retval += *st++;
		else
			++st;
	}

	return retval;
}

void s_XSL_FO_Listener::_handlePageSize(PT_AttrPropIndex api)
{
  //
  // Code to write out the PageSize Definitions to disk
  // 
	char *old_locale;

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
		
		UT_Dimension docUnit = m_pDocument->m_docPageSize.getDims(); 
		char buf[20];

		m_pie->write( UT_String_sprintf(" page-width=\"%f%s\"", m_pDocument->m_docPageSize.Width(docUnit), UT_dimensionName(docUnit)) );

		m_pie->write(UT_String_sprintf(" page-height=\"%f%s\"", m_pDocument->m_docPageSize.Height(docUnit), UT_dimensionName(docUnit)) );
	}
	// page-width, page-height

	m_pie->write(" master-reference=\"first\"");

	m_pie->write(">\n");
	m_pie->write("\t<fo:region-body/>\n");
	m_pie->write("</fo:simple-page-master>\n\n");
	m_pie->write("</fo:layout-master-set>\n\n");

	setlocale (LC_NUMERIC, old_locale);

	m_bFirstWrite = false;
	return;
}

void s_XSL_FO_Listener::_handleDataItems()
{
	const char * szName;
   	const char * szMimeType;
	const UT_ByteBuf * pByteBuf;

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,(void**)&szMimeType)); k++)
	{	  	  
	  FILE *fp;
	  UT_String fname;
	  
	  if (!UT_strcmp(szMimeType, "image/svg-xml"))
	    UT_String_sprintf(fname, "%s-%d.svg", m_pie->getFileName(), k);
	  if (!UT_strcmp(szMimeType, "text/mathml"))
	    UT_String_sprintf(fname, "%s-%d.mathml", m_pie->getFileName(), k);
	  else // PNG Image
	    UT_String_sprintf(fname, "%s-%d.png", m_pie->getFileName(), k);
	  
	  fp = fopen (fname.c_str(), "wb+");
	  
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
	m_pie->write("<fo:flow flow-name=\"xsl-region-body\">\n");
}

#define PROPERTY(x) \
	if (pAP->getProperty(x, szValue)) \
		content_st << " "x"=\"" << (const char*) szValue << "\"";

void s_XSL_FO_Listener::_openBlock(PT_AttrPropIndex api)
{
	if (!m_bInSection)
		return;

	UT_String start_st;
	UT_String content_st;
	const PP_AttrProp* pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);

	m_bInBlock = true;

	// query and output properties
	// todo - validate these and make sure they all make sense
	const XML_Char* szValue;

	if (bHaveProp && pAP)
	{
		if (pAP->getProperty("bgcolor", szValue))
		{
			content_st += " background-color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				content_st += '#';

			content_st << (const char*) szValue << "\"";
		}

		if (pAP->getProperty("color", szValue))
		{
			content_st += " color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				content_st += '#';

			content_st << (const char*) szValue << "\"";
		}

		if (pAP->getProperty("lang", szValue))
			content_st << " language=\"" << (const char*) szValue << "\"";

		if (pAP->getProperty("font-size", szValue))
			content_st << " font-size=\"" << purgeSpaces((const char *)szValue).c_str() << "\"";

		PROPERTY("font-family");
		PROPERTY("font-weight");
		PROPERTY("font-style");
		PROPERTY("font-stretch");
		PROPERTY("keep-together");
		PROPERTY("keep-with-next");
		PROPERTY("line-height");
		PROPERTY("margin-bottom");
		PROPERTY("margin-top");
		PROPERTY("margin-left");
		PROPERTY("margin-right");
		PROPERTY("text-align");
		PROPERTY("widows");
	}

	if (pAP && pAP->getAttribute("listid", szValue))
	{
		start_st = "<fo:list-block";
		UT_uint32 id = (UT_uint32) atoi((const char*)szValue);
		m_List.setIdList(id);
	}

	if ( true /*!content_st.empty()*/ )
	{
		if (start_st.empty())
			start_st = "<fo:block";

		start_st += content_st;
		start_st += '>';
		m_pie->write(start_st.c_str());
	}
}

void s_XSL_FO_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
		return;

	const PP_AttrProp* pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
	UT_String start_st("<fo:inline");
	UT_String content_st;
	
	m_bInSpan = true;

	// query and output properties
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;

		if (pAP->getProperty("bgcolor", szValue))
		{
			content_st += " background-color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				content_st += "#";

			content_st += (const char *)szValue;
			content_st += "\"";
		}

		if (pAP->getProperty("color", szValue))
		{
			content_st += " color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				content_st += "#";

			content_st += (const char *)szValue;
			content_st += "\"";
		}

		if (pAP->getProperty("lang", szValue))
		{
			content_st += " language=\"";
			content_st += (const char *)szValue;
			content_st += "\"";
		}
		
		if (pAP->getProperty("font-size", szValue))
		{
			content_st += " font-size=\"";
			content_st += purgeSpaces((const char *)szValue).c_str();
			content_st += "\"";
		}		

		PROPERTY("font-family");
		PROPERTY("font-weight");
		PROPERTY("font-style");
		PROPERTY("font-stretch");
		PROPERTY("keep-together");
		PROPERTY("keep-with-next");
		PROPERTY("text-decoration");
	}

	if (!content_st.empty())
	{
		start_st += content_st;
		start_st += '>';
		m_pie->write(start_st.c_str());
	}
}

#undef PROPERTY

void s_XSL_FO_Listener::_closeBlock()
{
	if (!m_bInBlock)
		return;

	m_bInBlock = false;
	m_pie->write("\n</fo:block>\n");
}

void s_XSL_FO_Listener::_closeSection()
{
	if (!m_bInSection)
		return;
	
	m_bInSection = false;
	m_pie->write("</fo:flow>\n");
	m_pie->write("</fo:page-sequence>\n");
}

void s_XSL_FO_Listener::_closeSpan()
{
	if (!m_bInSpan)
		return;

	m_bInSpan = false;
	m_pie->write("</fo:inline>");
}

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_convertColor(UT_String& szDest, const char* pszColor)
{
	/*
	 * TODO we might want to be a little more careful about this.
	 * The proper XSL-FO color is #rrggbb, which is basically the same
	 * as what we use this.
	 */
	szDest = pszColor;
}

void s_XSL_FO_Listener::_convertFontSize(UT_String& szDest, const char* pszFontSize)
{
	szDest = pszFontSize;
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
					  sBuf += UT_String_sprintf("&#x%x;",*pData++);
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












