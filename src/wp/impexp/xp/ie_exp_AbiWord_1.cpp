/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <time.h>
#include <set>

#include "ut_locale.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"
#include "ut_uuid.h"

#include "xap_App.h"

#include "pt_Types.h"

#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "pd_Style.h"

#include "pp_AttrProp.h"
#include "pp_Author.h"

#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#include "fd_Field.h"

#include "fl_AutoNum.h"

#include "fp_PageSize.h"

#include "ie_impexp_AbiWord_1.h"
#include "ie_exp_AbiWord_1.h"

#include "ap_Prefs.h"

#include <gsf/gsf-output-gzip.h>

#include <sstream>

// the fileformat that used to be defined here is now defined at the
// top of pd_Document.cpp

/*****************************************************************/
/*****************************************************************/

IE_Exp_AbiWord_1_Sniffer::IE_Exp_AbiWord_1_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_AWML11)
{
	// 
}

UT_Confidence_t IE_Exp_AbiWord_1_Sniffer::supportsMIME (const char * szMIME)
{
	if (strcmp (szMIME, IE_MIMETYPE_AbiWord) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

bool IE_Exp_AbiWord_1_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".abw") || !g_ascii_strcasecmp(szSuffix,".zabw") || !g_ascii_strcasecmp(szSuffix, ".abw.gz"));
}

UT_Error IE_Exp_AbiWord_1_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	*ppie = new IE_Exp_AbiWord_1(pDocument);
	return UT_OK;
}

bool IE_Exp_AbiWord_1_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "AbiWord (.abw, .zabw, abw.gz)";
	*pszSuffixList = "*.abw; *.zabw; *.abw.gz";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_AbiWord_1::IE_Exp_AbiWord_1(PD_Document * pDocument, bool isTemplate, bool isCompressed)
	: IE_Exp_XML(pDocument), m_bIsTemplate(isTemplate), m_bIsCompressed(isCompressed), m_pListener(0)
{
	m_error = 0;

	UT_ASSERT_HARMLESS( pDocument );

	// depending on how this document came to be, it might contain fragments without xid;
	// this is legitimate and desirable while the doc is loade, but once we are saving to
	// disk, we want to assing xids to any frags that should have them and do not
	if(pDocument)
		pDocument->fixMissingXIDs();
}

IE_Exp_AbiWord_1::~IE_Exp_AbiWord_1()
{
}

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT s_AbiWord_1_Listener : public PL_Listener
{
public:
	s_AbiWord_1_Listener(PD_Document * pDocument,
						IE_Exp_AbiWord_1 * pie, bool isTemplate);
	virtual ~s_AbiWord_1_Listener();

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord * pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

	/* implementation of XAP_[Internal]Resource[Manager]::Writer
	 */
	UT_Error write_base64 (void * context, const char * base64, UT_uint32 length, bool final);

	UT_Error write_xml (void * context, const char * name, const char * const * atts);
	UT_Error write_xml (void * context, const char * name);

protected:
	void                _closeTable(void);
	void                _closeCell(void);
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_closeField(void);
	void				_closeHyperlink(void);
	void				_closeAnnotation(void);
    void                _closeRDFAnchor(void);
	void				_closeTag(void);
	void				_openSpan(PT_AttrPropIndex apiSpan);
	void				_openTag(const char * szPrefix, bool bHasContent,
								 PT_AttrPropIndex api, UT_uint32 iXID,
								 bool bIgnoreProperties = false);
	//void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	//void				_outputXMLChar(const gchar * data, UT_uint32 length);
	/*void                _outputXMLChar(const std::string & s)
	{
		_outputXMLChar(s.c_str(), s.size());
	}*/
	//void				_outputXMLAttribute(const gchar * key, const gchar * value, UT_uint32 length);
	//void				_outputXMLAttribute(const gchar * key, const std::string& value );
	void				_handleStyles(void);
	void				_handleLists(void);
	void				_handlePageSize(void);
	void				_handleDataItems(void);
    void                _handleMetaData(void);
    void                _handleRDF(void);
	void                _handleRevisions(void);
	void                _handleHistory(void);
	void                _handleAuthors(void);

	PD_Document *		m_pDocument;
	IE_Exp_AbiWord_1 *	m_pie;
	bool                m_bIsTemplate;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	//	bool				m_bInTag;
	bool				m_bInHyperlink;
	UT_sint32           m_iInTable;
	UT_sint32           m_iInCell;
	UT_sint32           m_iBlockLevel;
	PT_AttrPropIndex	m_apiLastSpan;
    fd_Field *          m_pCurrentField;
	bool                m_bOpenChar;
	UT_GenericVector<UT_UTF8String *> m_vecSnapNames;
	bool				m_bInAnnotation;


private:
	// despite being a std::string, it will store an UTF-8 buffer
	typedef std::set<std::string> string_set;
	string_set m_pUsedImages;
	const gchar*		getObjectKey(const PT_AttrPropIndex& api, const gchar* key);
};

void s_AbiWord_1_Listener::_closeSection(void)
{
	if (!m_bInSection)
		return;

	m_pie->endElement();
	m_bInSection = false;
	return;
}


void s_AbiWord_1_Listener::_closeTable(void)
{
	if (m_iInTable == 0)
		return;

	m_pie->endElement();
	m_iInTable--;
	return;
}


void s_AbiWord_1_Listener::_closeCell(void)
{
	if (m_iInCell == 0)
		return;

	m_pie->endElement();
	m_iInCell--;
	return;
}

void s_AbiWord_1_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_pie->endElement();
	m_bInBlock = false;
	if (--m_iBlockLevel == 0) {
		m_pie->setPrettyPrint(true);
		m_pie->addString(NULL, "\n");
	}
	return;
}

void s_AbiWord_1_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	_closeTag();
	m_bInSpan = false;
	return;
}

void s_AbiWord_1_Listener::_closeTag(void)
{
	if (m_bOpenChar)
		m_pie->endElement();
	m_bOpenChar = false;
}

void s_AbiWord_1_Listener::_closeField(void)
{
	if (!m_pCurrentField)
		return;
    _closeSpan();
	m_pie->endElement();
    m_pCurrentField = NULL;
	return;
}

void s_AbiWord_1_Listener::_closeHyperlink(void)
{
	if (!m_bInHyperlink)
		return;
    _closeSpan();
	m_pie->endElement();
    m_bInHyperlink = false;
	return;
}


void s_AbiWord_1_Listener::_closeAnnotation(void)
{
	if (!m_bInAnnotation)
		return;
	UT_DEBUGMSG(("Doing close annotation object method \n"));
    _closeSpan();
	m_pie->endElement();
    m_bInAnnotation = false;
	return;
}

void s_AbiWord_1_Listener::_closeRDFAnchor(void)
{
	UT_DEBUGMSG(("Doing close rdf anchor object method \n"));
    _closeSpan();
	m_pie->endElement();
	return;
}

void s_AbiWord_1_Listener::_openSpan(PT_AttrPropIndex apiSpan)
{
	if (m_bInSpan)
	{
		if (m_apiLastSpan == apiSpan)
			return;
		_closeSpan();
	}

	if (!apiSpan)				// don't write tag for empty A/P
		return;

	_openTag("c",true,apiSpan,0);
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;
	return;
}

void s_AbiWord_1_Listener::_openTag(const char * szPrefix, bool bHasContent,
									PT_AttrPropIndex api, UT_uint32 iXID,
									bool bIgnoreProperties)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar * szValue;
	xxx_UT_DEBUGMSG(("_openTag: api %d, bHaveProp %d\n", api, bHaveProp));

	UT_return_if_fail(szPrefix && *szPrefix);

	if(strcmp(szPrefix,"c")== 0) {
		// We must not export with no attribute to save with, see #13708
		if (pAP &&
		    (pAP->getPropertyCount() > 0 || (pAP->getAttributeCount() > 0 &&
		                                    (m_pDocument->isExportAuthorAtts() ||
		                                     pAP->getAttributeCount() > 1 ||
		                                     !pAP->getAttribute(PT_AUTHOR_NAME, szValue)))))
		{
			m_bOpenChar = true;
			m_pie->startElement(szPrefix);
		}
	} else
		m_pie->startElement(szPrefix);
	if (bHaveProp && pAP)
	{
		UT_UTF8String url;
		const gchar * szName;
		UT_uint32 k = 0;

		while (pAP->getNthAttribute(k++,szName,szValue))
		{
			// TODO we force double-quotes on all values.
			// TODO consider scanning the value to see if it has one
			// TODO in it and escaping it or using single-quotes.
			// Let's also escape ampersands and other goodies.
			//
			// Strip out Author attributes for now.
			//
			if( !m_pDocument->isExportAuthorAtts() && strcmp(szName,PT_AUTHOR_NAME) == 0)
				continue;

			if ((strcmp (szName, "href") == 0) || (strcmp (szName, "xlink:href") == 0))
			{
				url = szValue;
				url.escapeURL();
				m_pie->addString(szName, url.utf8_str());
			}
			else
			{
				m_pie->addString(szName, szValue);
			}
			
		}
		if(iXID != 0)
		{
			// insert xid attribute
			m_pie->addInt("xid", iXID);
		}
		if (!bIgnoreProperties && pAP->getNthProperty(0,szName,szValue))
		{
			std::ostringstream buf;
			buf << szName << ":" << szValue;
			UT_uint32 j = 1;
			while (pAP->getNthProperty(j++,szName,szValue))
			{
				// TMN: Patched this since I got an assert. What's the fix?
				// is it to write out a quoted empty string, or not to write
				// the property at all? For now I fixed it by the latter.
				if (*szValue)
				{
					buf << "; " << szName << ":" << szValue;
				}
			}
			m_pie->addString(PT_PROPS_ATTRIBUTE_NAME, buf.str());
		}
	}
	if(strcmp(szPrefix,"math") == 0)
	{
		const char * szPropVal = NULL;
		pAP->getAttribute("dataid",szPropVal);
		if(szPropVal != NULL)
		{
			m_pie->startElement("image");
			std::string tag = "snapshot-svg-";
			tag += szPropVal;
			m_pie->addString("dataid", tag);
			bool bFound = pAP->getProperty("height", szPropVal);
			std::ostringstream buf;
			bool need_sep = false;
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				buf << "height:" << dInch << "in";
				need_sep = true;
			}
			bFound = pAP->getProperty("width", szPropVal);
			if(bFound)
			{
				if (need_sep)
					buf << "; ";
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				buf << "width:" << dInch << "in";
			}
			m_pie->addString(PT_PROPS_ATTRIBUTE_NAME, buf.str());
			m_pie->endElement();
		}
	}
	else if(strcmp(szPrefix,"embed") == 0)
	{
		const char * szPropVal = NULL;
		pAP->getAttribute("dataid",szPropVal);
		if(szPropVal != NULL)
		{
			m_pie->startElement("image");
			std::string tag = std::string("snapshot-svg-") + szPropVal;
			if (!m_pDocument->getDataItemDataByName(tag.c_str (), NULL, NULL, NULL))
				tag = std::string("snapshot-png-") + szPropVal;
			m_pie->addString("dataid", tag);
			bool bFound = pAP->getProperty("height", szPropVal);
			std::ostringstream buf;
			bool need_sep = false;
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				buf << "height:" << dInch << "in";
				need_sep = true;
			}
			bFound = pAP->getProperty("width", szPropVal);
			if(bFound)
			{
				if (need_sep)
					buf << "; ";
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				buf << "width:" << dInch << "in";
			}
			m_pie->addString(PT_PROPS_ATTRIBUTE_NAME, buf.str());
			m_pie->endElement();
		}
	}
	if (!bHasContent)
		m_pie->endElement();
}

s_AbiWord_1_Listener::s_AbiWord_1_Listener(PD_Document * pDocument,
										   IE_Exp_AbiWord_1 * pie,
										   bool isTemplate)
	: m_pUsedImages()
{
	m_bIsTemplate = isTemplate;
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	//	m_bInTag = false;
	m_bInHyperlink = false;
	m_bOpenChar = false;
	m_apiLastSpan = 0;
	m_pCurrentField = 0;
	m_iInTable = 0;
	m_iInCell = 0;
	m_iBlockLevel = 0;
	m_bInAnnotation = false;

	/***********************************************************************************

        The fixed attributes of the <abiword> token that used to reside
        here are now found in PD_Document::setAttrProp()

	************************************************************************************/
	m_pie->setDocType ("<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML 1.0 Strict//EN\" \"http://www.abisource.com/awml.dtd\">\n");

	// we want to update the XID counter and the template status
	const PP_PropertyVector attr = {
		"template", m_bIsTemplate ? "true" : "false",
		"xid-max", UT_std_string_sprintf("%d", pDocument->getTopXID())
	};
	pDocument->setAttributes(attr);

	_openTag("abiword", true, pDocument->getAttrPropIndex(),false,0);

	// NOTE we output the following preamble in XML comments.
	// NOTE this information is for human viewing only.

	m_pie->addString(NULL, "\n");
	m_pie->addComment("========================================================================");
	m_pie->addComment("This file is an AbiWord document.                                       ");
	m_pie->addComment("AbiWord is a free, Open Source word processor.                          ");
	m_pie->addComment("More information about AbiWord is available at http://www.abisource.com/");
	m_pie->addComment("You should not edit this file by hand.                                  ");
	m_pie->addComment("========================================================================");
	m_pie->addString(NULL, "\n");
		
	// end of preamble.
	// now we begin the actual document.

	_handleMetaData();
	_handleRDF();
	_handleHistory();
	_handleRevisions();
	_handleStyles();
	_handleLists();
	_handlePageSize();
	if(m_pDocument->isExportAuthorAtts())
		_handleAuthors();
}

s_AbiWord_1_Listener::~s_AbiWord_1_Listener()
{
	_closeSpan();
	_closeField();
	_closeHyperlink();
	_closeAnnotation();
	_closeBlock();
	_closeSection();
	_handleDataItems();

	m_pie->endElement();
	UT_VECTOR_PURGEALL(UT_UTF8String * ,m_vecSnapNames);
}


const gchar*
s_AbiWord_1_Listener::getObjectKey(const PT_AttrPropIndex& api, const gchar* key)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	if (bHaveProp && pAP)
	{
		const gchar* value;
		if (pAP->getAttribute(key, value))
			return value;
	}

	return 0;
}


bool s_AbiWord_1_Listener::populate(fl_ContainerLayout* /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
            if (pcrs->getField()!=m_pCurrentField)
            {
                _closeField();
            }
			PT_AttrPropIndex api = pcr->getIndexAP();
			_openSpan(api);

			PT_BufIndex bi = pcrs->getBufIndex();
			m_pie->addString(NULL, m_pDocument->getPointer(bi), pcrs->getLength());

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
				_closeSpan();
                _closeField();
				const gchar* image_name = getObjectKey(api, static_cast<const gchar*>("dataid"));
				if (image_name)
					m_pUsedImages.insert(image_name);
				_openTag("image",false,api,pcr->getXID());

				return true;
				}
			case PTO_Field:
                {
                    _closeSpan();
                    _closeField();
                    _openTag("field",true,api,pcr->getXID());
                    m_pCurrentField = pcro->getField();
                    UT_ASSERT_HARMLESS(m_pCurrentField);
                    return true;
                }
 			case PTO_Math:
                {
                    _closeSpan();
                    _closeField();
                    _openTag("math",false,api,pcr->getXID());
					const gchar* image_name = getObjectKey(api, static_cast<const gchar*>("dataid"));
					if (image_name)
					{
						UT_DEBUGMSG(("resource name #%s# recorded \n",image_name));
						m_pUsedImages.insert(image_name);
						UT_UTF8String * sSVGname = new UT_UTF8String("snapshot-svg-");
						m_vecSnapNames.addItem(sSVGname);
						*sSVGname += image_name;
						UT_DEBUGMSG(("resource name #%s# recorded \n",sSVGname->utf8_str()));
						m_pUsedImages.insert(sSVGname->utf8_str());
					}
					const gchar* latex_name = getObjectKey(api, static_cast<const gchar*>("latexid"));
					if(latex_name)
					{
						UT_DEBUGMSG(("resource name #%s# recorded \n",latex_name));
						m_pUsedImages.insert(latex_name);
					}
                    return true;
                }
 			case PTO_Embed:
                {
                    _closeSpan();
                    _closeField();
                    _openTag("embed",false,api,pcr->getXID());
					const gchar* image_name = getObjectKey(api, static_cast<const gchar*>("dataid"));
					if (image_name)
					{
						UT_DEBUGMSG(("resource name #%s# recorded \n",image_name));
						m_pUsedImages.insert(image_name);
						UT_UTF8String * sPNGname = new UT_UTF8String("snapshot-svg-");
						m_vecSnapNames.addItem(sPNGname);
						*sPNGname += image_name;
						if (!m_pDocument->getDataItemDataByName(sPNGname->utf8_str(), NULL, NULL, NULL))
							*sPNGname = UT_UTF8String("snapshot-png-") + image_name;
						UT_DEBUGMSG(("resource name #%s# recorded \n",sPNGname->utf8_str()));
						m_pUsedImages.insert(sPNGname->utf8_str());
					}
                    return true;
                }
  			case PTO_Bookmark:
   				{
   					_closeSpan();
   					_closeField();
   					_openTag("bookmark", false, api,pcr->getXID(),true);
   					return true;
   				}

   			case PTO_Hyperlink:
   				{
   					_closeSpan();
   					_closeField();
					_closeHyperlink();
					const PP_AttrProp * pAP = NULL;
					m_pDocument->getAttrProp(api,&pAP);
					const gchar * pName;
					const gchar * pValue;
					bool bFound = false;
					UT_uint32 k = 0;

					while(pAP->getNthAttribute(k++, pName, pValue))
					{
						bFound = (0 == g_ascii_strncasecmp(pName,"xlink:href",10));
						if(bFound)
							break;
					}
					
					if(bFound)
					{
						//this is the start of the hyperlink
   						_openTag("a", true, api,pcr->getXID(),true);
   						m_bInHyperlink = true;
   					}

   					return true;
   				}

   			case PTO_Annotation:
   				{
   					_closeSpan();
   					_closeField();
					_closeAnnotation();
					const PP_AttrProp * pAP = NULL;
					m_pDocument->getAttrProp(api,&pAP);
					const gchar * pName;
					const gchar * pValue;
					bool bFound = false;
					UT_uint32 k = 0;

					while(!bFound && pAP->getNthAttribute(k++, pName, pValue))
					{
						bFound = (0 == g_ascii_strncasecmp(pName,"Annotation",10));
					}

					if(bFound)
					{
						//this is the start of the Annotation
   						_openTag("ann", true, api,pcr->getXID(),true);
						UT_DEBUGMSG(("Doing open annotation object \n"));
   						m_bInAnnotation = true;
   					}
					
   					return true;
   				}

   			case PTO_RDFAnchor:
   				{
   					_closeSpan();
   					_closeField();
					const PP_AttrProp * pAP = NULL;
					m_pDocument->getAttrProp(api,&pAP);
                    RDFAnchor a( pAP );
                    if( !a.isEnd() )
                    {
   						_openTag("textmeta", true, api,pcr->getXID(),true);
                    }
                    else
                    {
   						_closeRDFAnchor();
                    }
   					return true;
   				}
                

			default:
				UT_ASSERT_NOT_REACHED();
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
#if 1
		// This code was commented by tf for reasons we can't remember
		// Reinstate it to fix bug - 11629 blank lines lose their
		// properties on export.
		// Might be related to things like #13708
		if(m_bOpenChar)
			_closeTag();
		_openTag("c",true,pcr->getIndexAP(),0);
		_closeTag();
#endif
		return true;

	default:
	  UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_AbiWord_1_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   fl_ContainerLayout* * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.
	PT_AttrPropIndex api = pcr->getIndexAP();
	const gchar* image_name = getObjectKey(api, static_cast<const gchar*>(PT_STRUX_IMAGE_DATAID));
	if (image_name)
		m_pUsedImages.insert(image_name);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	case PTX_SectionHdrFtr:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();	
			_closeAnnotation();
			_closeBlock();
			_closeSection();
			_openTag("section",true,pcr->getIndexAP(),pcr->getXID());
			m_bInSection = true;
			return true;
		}
	case PTX_SectionTable:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("table",true,pcr->getIndexAP(),pcr->getXID());
			m_iInTable++;
			return true;
		}
	case PTX_SectionCell:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("cell",true,pcr->getIndexAP(),pcr->getXID());
			m_iInCell++;
			return true;
		}
	case PTX_SectionFootnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			m_bInBlock = false;
			_openTag("foot",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionAnnotation:
		{
			// spans and field have been closed by the annotation
			//			_closeSpan();
            // _closeField();
			// We may have to close hyperlinks but I hope not.
			//            _closeHyperlink();
			UT_DEBUGMSG(("Found start annotation strux \n"));
			m_bInBlock = false;
			_openTag("annotate",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionEndnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			m_bInBlock = false;
			_openTag("endnote",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionTOC:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			m_bInBlock = false;
			_openTag("toc",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionMarginnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("margin",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionFrame:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("frame",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_EndTable:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_closeTable();
			return true;
		}
	case PTX_EndCell:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_closeCell();
			return true;
		}
	case PTX_EndFootnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			m_pie->endElement();
			m_bInBlock = true;
			return true;
		}
	case PTX_EndAnnotation:
		{
			UT_DEBUGMSG(("End of annotation strux \n"));
			_closeSpan();
            _closeField();
			// Lets not close out hyperlinks to start with
			//            _closeHyperlink();
			_closeBlock();
			m_pie->endElement();
			m_bInBlock = true;
			return true;
		}
	case PTX_EndEndnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			m_pie->endElement();
			m_bInBlock = true;
			return true;
		}
	case PTX_EndTOC:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			m_pie->endElement();
			return true;
		}
	case PTX_EndMarginnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			return true;
		}
	case PTX_EndFrame:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			m_pie->endElement();
			return true;
		}
	case PTX_Block:
		{
			_closeSpan();
            _closeField();
			_closeHyperlink();
			_closeBlock();
			_openTag("p",true,pcr->getIndexAP(),pcr->getXID());
			m_pie->setPrettyPrint(false);
			m_bInBlock = true;
			m_iBlockLevel++;
			return true;
		}

	default:
	  UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_AbiWord_1_Listener::change(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
  UT_ASSERT_NOT_REACHED();
	return false;
}

bool s_AbiWord_1_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  pf_Frag_Strux* /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool s_AbiWord_1_Listener::signal(UT_uint32 /* iSignal */)
{
  UT_ASSERT_NOT_REACHED();
	return false;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_AbiWord_1::_writeDocument(void)
{
	// allow people to override this on the command line or otherwise
	const std::string & prop = (getProperty ("compress"));
	if (!prop.empty())
		m_bIsCompressed = UT_parseBool(prop.c_str (), m_bIsCompressed);
	setupFile(m_bIsCompressed);

	m_pListener = new s_AbiWord_1_Listener(getDoc(),this, m_bIsTemplate);
	if (!m_pListener)
	{
		closeHandle();
		return UT_IE_NOMEMORY;
	}

	bool bStatusTellListener = true;
	if (getDocRange())
	{
		bStatusTellListener = getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),getDocRange());
	}
	else
	{
		bStatusTellListener = getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener));
	}
	
	delete m_pListener;
	m_pListener = NULL;
	closeHandle();

	if (!bStatusTellListener)
	{
		return UT_ERROR;
	}
	else if (m_error)
	{
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

void s_AbiWord_1_Listener::_handleStyles(void)
{
	bool bWroteOpenStyleSection = false;

	const PD_Style * pStyle=NULL;
	UT_GenericVector<PD_Style *> vecStyles;
	m_pDocument->getAllUsedStyles(&vecStyles);
	UT_sint32 k = 0;
	for (k=0; k < vecStyles.getItemCount(); k++)
	{
		pStyle = vecStyles.getNthItem(k);
		if (!bWroteOpenStyleSection)
		{
			m_pie->startElement("styles");
			bWroteOpenStyleSection = true;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s",false,api,0);
	}

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	m_pDocument->enumStyles(pStyles);
	UT_ASSERT_HARMLESS( pStyles );
	UT_sint32 iStyleCount = m_pDocument->getStyleCount();
	
	for (k=0; (k < iStyleCount) && pStyles; k++)
	{
		pStyle = pStyles->getNthItem(k);
		UT_continue_if_fail( pStyle );
		
		if (!pStyle->isUserDefined() || (vecStyles.findItem(const_cast<PD_Style*>(pStyle))) >= 0)
			continue;

		if (!bWroteOpenStyleSection)
		{
			m_pie->startElement("styles");
			bWroteOpenStyleSection = true;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s",false,api,0);
	}

	delete pStyles;
	
	if (bWroteOpenStyleSection)
		m_pie->endElement();

	return;
}

void s_AbiWord_1_Listener::_handleLists(void)
{
	bool bWroteOpenListSection = false;

 	//const char * szID;
	//const char * szPid;
	//const char * szProps;

#define LCheck(str) (0 == strcmp(s.utf8_str(), str))
	UT_UTF8String esc;
	
	fl_AutoNum * pAutoNum;
	for (UT_uint32 k = 0; (m_pDocument->enumLists(k, &pAutoNum )); k++)
	{
		const char ** attr0 = NULL;

		if (pAutoNum->isEmpty() == true)
			continue;

		std::vector<UT_UTF8String> vAttrs;
		pAutoNum->getAttributes (vAttrs, true);
		
		if (!bWroteOpenListSection)
		{
			m_pie->startElement("lists");
			bWroteOpenListSection = true;
		}
		m_pie->startElement("l");
		for (UT_sint32 i = 0; i < ((UT_sint32)vAttrs.size()) - 1;
			 i += 2)
		{
			const UT_UTF8String & s = vAttrs[i];
			
			if (LCheck("id")           ||
				LCheck("parentid")     ||
				LCheck("type")         ||
				LCheck("start-value")  ||
				LCheck("list-delim")   ||
				LCheck("list-decimal"))
			{
				m_pie->addString(s.utf8_str(), vAttrs[i+1].utf8_str());
			}
		}
		m_pie->endElement();
		// No, no, you can't g_free attr and expect good things to happen.
		FREEP(attr0);
	}


#undef LCheck

	if (bWroteOpenListSection)
		m_pie->endElement();

	return;
}

void s_AbiWord_1_Listener::_handleMetaData(void)
{
	if (m_pie->isCopying ())
		return;

  // set all of the important meta-data props

  m_pDocument->setMetaDataProp ( PD_META_KEY_GENERATOR, "AbiWord" ) ;
  m_pDocument->setMetaDataProp ( PD_META_KEY_FORMAT,    IE_MIMETYPE_AbiWord ) ;

#if 0
  // get the saved time, remove trailing newline
  time_t now = time ( NULL ) ;
  UT_String now_str ( ctime(&now) ) ;
  now_str = now_str.substr ( 0, now_str.size() -1 ) ;
  m_pDocument->setMetaDataProp ( PD_META_KEY_DATE_LAST_CHANGED, UT_UTF8String(now_str.c_str()) ) ;
#endif

  // TODO: set dc.date and abiword.date_created if document is new (i.e. first save)

  const std::map<std::string, std::string> & ref = m_pDocument->getMetaData() ;

  // don't print out a thing
  if ( ref.empty() ) {
    return ;
  }

  m_pie->startElement("metadata");

  std::map<std::string, std::string>::const_iterator iter = ref.begin();
  for ( ; iter != ref.end(); ++iter ) {
	  if( !iter->second.empty() ) {
	      m_pie->startElement("m");
		  m_pie->addString("key", iter->first);
	      m_pie->addString(NULL, iter->second);
	      m_pie->endElement() ;
	  }
  }

  m_pie->endElement();
}

void s_AbiWord_1_Listener::_handleRDF(void)
{
  m_pie->startElement("rdf");

  //
  // Walk every subject in the RDF model
  //
  PD_DocumentRDFHandle rdf = m_pDocument->getDocumentRDF();
  PD_URIList subjects = rdf->getAllSubjects();
  PD_URIList::iterator subjend = subjects.end();
  for( PD_URIList::iterator subjiter = subjects.begin();
       subjiter != subjend; ++subjiter )
  {
      PD_URI subject = *subjiter;
      POCol polist = rdf->getArcsOut( subject );
      POCol::iterator poend = polist.end();
      for( POCol::iterator poiter = polist.begin();
           poiter != poend; ++poiter )
      {
          //
          // For each subject, predicate, object
          // create an XML element.
          //
          PD_URI    predicate = poiter->first;
          PD_Object object = poiter->second;

          m_pie->startElement("t");
          m_pie->addString("s", subject.toString());
          m_pie->addString("p", predicate.toString());
          {
              std::stringstream ss;
              ss << object.getObjectType();
              m_pie->addString("objecttype", ss.str());
          }
          m_pie->addString("xsdtype", object.getXSDType());
          UT_UTF8String esc = object.toString().c_str();
          m_pie->addString(NULL, esc.utf8_str());
         m_pie->endElement();
       }
  }
  
  m_pie->endElement();
}

void s_AbiWord_1_Listener::_handlePageSize(void)
{
  //
  // Code to write out the PageSize Definitions to disk
  //
	m_pie->startElement("pagesize");
	m_pie->addString("pagetype", m_pDocument->m_docPageSize.getPredefinedName());
	m_pie->addString("orientation",
	                 (m_pDocument->m_docPageSize.isPortrait() == true)?
	             	   "portrait": "landscape");
	UT_Dimension docUnit = m_pDocument->m_docPageSize.getDims();
	m_pie->addFloat("width", m_pDocument->m_docPageSize.Width(docUnit));
	m_pie->addFloat("height", m_pDocument->m_docPageSize.Height(docUnit));
	m_pie->addString("units", UT_dimensionName(docUnit));
	m_pie->addFloat("page-scale", m_pDocument->m_docPageSize.getScale());
	m_pie->endElement();
}

void s_AbiWord_1_Listener::_handleDataItems(void)
{
	bool bWroteOpenDataSection = false;

	const char * szName;
    std::string mimeType;
	const UT_ByteBuf * pByteBuf;

	UT_ByteBuf bbEncoded(1024);
	string_set::iterator end(m_pUsedImages.end());
	UT_DEBUGMSG(("Used images are... \n"));
	for (UT_uint32 k=0;
		 
		 (m_pDocument->enumDataItems(k, NULL, &szName, &pByteBuf, &mimeType));
		 k++)
	{
		string_set::iterator it(m_pUsedImages.find(szName));
		if (it == end)
		{
			// This data item is no longer used. Don't output it to a file.
			UT_DEBUGMSG(("item #%s# not found in set:\n", szName));
			continue;
		}
		else
		{
			UT_DEBUGMSG(("item #%s# found:\n", szName));
			m_pUsedImages.erase(it);
		}

		if (!bWroteOpenDataSection)
		{
			m_pie->startElement("data");
			bWroteOpenDataSection = true;
		}

	   	bool status = false;
	   	bool encoded = true;

		if (!mimeType.empty() 
            && ((mimeType ==  "image/svg+xml") 
                || (mimeType == "application/mathml+xml")))
	    {
		   bbEncoded.truncate(0);
		   bbEncoded.append(reinterpret_cast<const UT_Byte*>("<![CDATA["), 9);
		   UT_uint32 off = 0;
		   UT_uint32 len = pByteBuf->getLength();
		   const UT_Byte * buf = pByteBuf->getPointer(0);
		   while (off < len) {
		      if (buf[off] == ']' && buf[off+1] == ']' && buf[off+2] == '>') {
			 bbEncoded.append(buf, off-1);
			 bbEncoded.append(reinterpret_cast<const UT_Byte*>("]]&gt;"), 6);
			 off += 3;
			 len -= off;
			 buf = pByteBuf->getPointer(off);
			 off = 0;
			 continue;
		      }
		      off++;
		   }
		   bbEncoded.append(buf, len);
		   bbEncoded.append(reinterpret_cast<const UT_Byte*>("]]>\n"), 4);
		   status = true;
		   encoded = false;
		}
	   	else
		{
		   status = UT_Base64Encode(&bbEncoded, pByteBuf);
		   encoded = true;
		}

	   	if (status)
	    {
	   		m_pie->startElement("d");
			m_pie->setPrettyPrint(false);
			m_pie->addString("name", szName);
		   	if (!mimeType.empty())
			{
			   m_pie->addString("mime-type", mimeType.c_str());
			}
		   	if (encoded)
		    {
				std::ostringstream buf;
			   	m_pie->addString("base64", "yes");
				   // break up the Base64 blob as a series lines
				// like MIME does.

				UT_uint32 jLimit = bbEncoded.getLength();
				UT_uint32 jSize;
				UT_uint32 j;
				for (j=0; j<jLimit; j+=72)
				{
					jSize = UT_MIN(72,(jLimit-j));
					buf.write(reinterpret_cast<const char *>(bbEncoded.getPointer(j)),jSize);
					buf << std::endl;
				}
			}
		   	else
		    {
			   	m_pie->addString("base64", "no");
			}
			m_pie->addStringUnchecked(NULL, reinterpret_cast<const char*>(bbEncoded.getPointer(0)));
			m_pie->endElement();
			m_pie->setPrettyPrint(true);

		}
	}

	if (bWroteOpenDataSection)
		m_pie->endElement();
}

void s_AbiWord_1_Listener::_handleRevisions(void)
{
	bool bWroteOpenRevisionsSection = false;

	const AD_Revision * pRev=NULL;

	const UT_GenericVector<AD_Revision*> & vRevisions = m_pDocument->getRevisions();

	UT_sint32 k = 0;
	std::string s;
	for (k=0; k < vRevisions.getItemCount(); k++)
	{
		pRev = vRevisions.getNthItem(k);
		UT_continue_if_fail(pRev);
		
		if (!bWroteOpenRevisionsSection)
		{
			m_pie->startElement("revisions");
			m_pie->addBool("show", m_pDocument->isShowRevisions());
			m_pie->addBool("mark", m_pDocument->isMarkRevisions());
			m_pie->addInt("show-level", m_pDocument->getShowRevisionId());
			m_pie->addBool("auto", m_pDocument->isAutoRevisioning());
			bWroteOpenRevisionsSection = true;
		}

		m_pie->startElement("r");
		m_pie->addUint("id",pRev->getId());
		m_pie->addLint("time-started", pRev->getStartTime());
		m_pie->addUint("version", pRev->getVersion());

		if(pRev->getDescription())
		{
			m_pie->addString(NULL, pRev->getDescription(), UT_UCS4_strlen(pRev->getDescription()));
		}
		

		m_pie->endElement();
	}

	if (bWroteOpenRevisionsSection)
		m_pie->endElement();

	return;
}

void s_AbiWord_1_Listener::_handleHistory(void)
{
	bool bWroteOpenSection = false;

	UT_uint32 k = 0;
	const UT_uint32 iCount = m_pDocument->getHistoryCount();
	
	for (k=0; k < iCount; k++)
	{
		UT_uint32 iVersion  =  m_pDocument->getHistoryNthId(k);
		const UT_UUID& UID  =  m_pDocument->getHistoryNthUID(k);
		time_t tStarted     =  m_pDocument->getHistoryNthTimeStarted(k);
		bool bAuto          =  m_pDocument->getHistoryNthAutoRevisioned(k);
		UT_uint32 iXID      =  m_pDocument->getHistoryNthTopXID(k);
		
		UT_UTF8String s, hUid;
		UID.toString(hUid);
		
		
		if (!bWroteOpenSection)
		{
			m_pie->startElement("history");
			m_pie->addInt("version", m_pDocument->getDocVersion());
			m_pie->addInt("edit-time", static_cast<UT_sint32>(m_pDocument->getEditTime()));
			m_pie->addInt("last-saved", static_cast<UT_sint32>(m_pDocument->getLastSavedTime()));
			m_pie->addString("uid", m_pDocument->getDocUUIDString());
			bWroteOpenSection = true;
		}

		m_pie->startElement("version");
		m_pie->addInt("id", iVersion);
		m_pie->addInt("started", static_cast<UT_sint32>(tStarted));
		m_pie->addString("uid", hUid.utf8_str());
		m_pie->addBool("auto", bAuto);
		m_pie->addInt("top-xid", iXID);
		m_pie->endElement();
	}

	if (bWroteOpenSection)
		m_pie->endElement();

	return;
}

void s_AbiWord_1_Listener::_handleAuthors(void)
{
	UT_sint32 nAuthors = m_pDocument-> getNumAuthors();
	if(nAuthors <= 0)
		return;
	m_pie->startElement("authors");
	UT_sint32 i = 0;
	UT_String sVal;
	for(i=0;i<nAuthors;i++)
	{
		pp_Author * pAuthor = m_pDocument->getNthAuthor(i);
		m_pie->startElement("author");
		m_pie->addInt("id", pAuthor->getAuthorInt());
		PP_AttrProp * pAP = pAuthor->getAttrProp();
		if(pAP->getPropertyCount()>0)
		{
			std::ostringstream buf;
			const gchar * szName = NULL;
			const gchar * szValue = NULL;
			UT_uint32 j = 0;
			while (pAP->getNthProperty(j++,szName,szValue))
			{
				if (szName && *szName && szValue && *szValue)
				{
					if(j>1)
						buf << "; ";
					buf << szName << ":" << szValue;
				}
			}
			m_pie->addString(PT_PROPS_ATTRIBUTE_NAME,buf.str());
		}
		m_pie->endElement();
	}
	m_pie->endElement();
}

