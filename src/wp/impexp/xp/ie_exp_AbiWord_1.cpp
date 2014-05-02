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

#ifdef ENABLE_RESOURCE_MANAGER
#include "xap_ResourceManager.h"
#endif

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
	: IE_Exp(pDocument), m_bIsTemplate(isTemplate), m_bIsCompressed(isCompressed), m_pListener(0), m_output(0)
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

UT_uint32 IE_Exp_AbiWord_1::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	if(!pBytes || !length)
	  return 0;

	if (m_output)
		{
			gsf_output_write(m_output, length, pBytes);
			return length;
		}
	else
		{
			return IE_Exp::_writeBytes(pBytes, length);
		}
}

void IE_Exp_AbiWord_1::_setupFile()
{
	// allow people to override this on the command line or otherwise
	const std::string & prop = (getProperty ("compress"));
	if (!prop.empty())
		m_bIsCompressed = UT_parseBool(prop.c_str (), m_bIsCompressed);
	bool forceCompression = false;
	if (!m_bIsCompressed)
	{
		// check if file name suggests compression
		const char *name = getFileName();
		int length = name? strlen(name): 0;
		if ((length > 5 && !strcmp(name + length - 5, ".zabw")) ||
		     (length > 7 && !strcmp(name + length - 7, ".abw.gz")))
		{
		     forceCompression = m_bIsCompressed = true;
		}
	}
	
	if (m_bIsCompressed)
	{
		GsfOutput * gzip = gsf_output_gzip_new(getFp (), NULL);
		m_output = gzip;
	}
	else
	{
		m_output = 0;
	}
	if (forceCompression)
		m_bIsCompressed = false;
}

static void close_gsf_handle(GsfOutput * output)
{
	if (output)
		{
			gsf_output_close (output);
			g_object_unref (G_OBJECT (output));
		}
}

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_RESOURCE_MANAGER
class ABI_EXPORT s_AbiWord_1_Listener : public PL_Listener, XAP_ResourceManager::Writer
#else
class ABI_EXPORT s_AbiWord_1_Listener : public PL_Listener
#endif
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
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 bool bNewLineAfter, PT_AttrPropIndex api,
								 UT_uint32 iXID,
								 bool bIgnoreProperties = false);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_outputXMLChar(const gchar * data, UT_uint32 length);
	void                _outputXMLChar(const std::string & s)
	{
		_outputXMLChar(s.c_str(), s.size());
	}
	void				_outputXMLAttribute(const gchar * key, const gchar * value, UT_uint32 length);
	void				_outputXMLAttribute(const gchar * key, const std::string& value );
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

	m_pie->write("</section>\n");
	m_bInSection = false;
	return;
}


void s_AbiWord_1_Listener::_closeTable(void)
{
	if (m_iInTable == 0)
		return;

	m_pie->write("</table>\n");
	m_iInTable--;
	return;
}


void s_AbiWord_1_Listener::_closeCell(void)
{
	if (m_iInCell == 0)
		return;

	m_pie->write("</cell>\n");
	m_iInCell--;
	return;
}

void s_AbiWord_1_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_pie->write("</p>\n");
	m_bInBlock = false;
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
	if (m_bOpenChar) m_pie->write("</c>");
	m_bOpenChar = false;
}

void s_AbiWord_1_Listener::_closeField(void)
{
	if (!m_pCurrentField)
		return;
    _closeSpan();
	m_pie->write("</field>");
    m_pCurrentField = NULL;
	return;
}

void s_AbiWord_1_Listener::_closeHyperlink(void)
{
	if (!m_bInHyperlink)
		return;
    _closeSpan();
	m_pie->write("</a>");
    m_bInHyperlink = false;
	return;
}


void s_AbiWord_1_Listener::_closeAnnotation(void)
{
	if (!m_bInAnnotation)
		return;
	UT_DEBUGMSG(("Doing close annotation object method \n"));
    _closeSpan();
	m_pie->write("</ann>");
    m_bInAnnotation = false;
	return;
}

void s_AbiWord_1_Listener::_closeRDFAnchor(void)
{
	UT_DEBUGMSG(("Doing close rdf anchor object method \n"));
    _closeSpan();
	m_pie->write("</textmeta>");
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

	_openTag("c","",false,apiSpan,0);
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;
	return;
}

void s_AbiWord_1_Listener::_openTag(const char * szPrefix, const char * szSuffix,
									bool bNewLineAfter, PT_AttrPropIndex api,
									UT_uint32 iXID,
									bool bIgnoreProperties)
{
#ifdef ENABLE_RESOURCE_MANAGER
	UT_ASSERT_HARMLESS (!m_bOpenChar);

	UT_UTF8String tag("<");
	UT_UTF8String url;
	UT_UTF8String esc;
	
	tag += szPrefix;

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	if (bHaveProp && pAP)
	{
		const gchar * szName = 0;
		const gchar * szValue = 0;

		UT_uint32 k = 0;
		while (pAP->getNthAttribute (k++, szName, szValue))
		{
#if 0
			//
			// Strip out Author attributes for now.
			//
			if( !m_pDocument->isExportAuthorAtts() && strcmp(szName,PT_AUTHOR_NAME) == 0)
				continue;
#endif
			tag += " ";
			tag += szName;
			tag += "=\"";

			if ((strcmp (szName, "href") == 0) || (strcmp (szName, "xlink:href") == 0))
			{
				if(*szValue == '/')
				{
					XAP_ResourceManager & RM = m_pDocument->resourceManager ();
					XAP_ExternalResource * re = dynamic_cast<XAP_ExternalResource *>(RM.resource (szValue, false));
					if (re)
					{
						url = re->URL ();
						url.escapeURL();
						tag += url;
					}
				}
				else
				{
					url = szValue;
					url.escapeURL();
					tag += url;
				}
			}
			else
			{
				esc = szValue;
				esc.escapeXML();
				tag += esc;
			}
			

			tag += "\"";
		}

		if(iXID != 0)
		{
			// insert xid attribute
			tag += " ";
			tag += PT_XID_ATTRIBUTE_NAME;
			tag += "=\"";
			UT_UTF8String s;
			UT_UTF8String_sprintf(s, "%d\"", iXID);
			tag += s;
		}
		
		if (!bIgnoreProperties)
		{
			k = 0;
			while (pAP->getNthProperty (k++, szName, szValue))
			{
				if (k == 1)
				{
					tag += " ";
					tag += PT_PROPS_ATTRIBUTE_NAME;
					tag += "=\"";
				}
				else tag += "; ";

				tag += szName;
				tag += ":";

				esc = szValue;
				esc.escapeXML();
				tag += esc;
			}
			if (k > 1) tag += "\"";
		}
	}
	if (szSuffix)
		if (*szSuffix == '/')
			tag += "/";
	tag += ">";
	if (bNewLineAfter) tag += "\n";
	m_pie->write (tag.utf8_str (), tag.byteLength());

	if (strcmp (szPrefix, "c") == 0) m_bOpenChar = true;

#else /* ENABLE_RESOURCE_MANAGER */
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	xxx_UT_DEBUGMSG(("_openTag: api %d, bHaveProp %d\n",api, bHaveProp));

	UT_return_if_fail(szPrefix && *szPrefix);

	m_pie->write("<");

	if(strcmp(szPrefix,"c")== 0)
		m_bOpenChar = true;
	m_pie->write(szPrefix);
	if (bHaveProp && pAP)
	{
		UT_UTF8String url;
		const gchar * szName;
		const gchar * szValue;
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

			m_pie->write(" ");
			m_pie->write(static_cast<const char*>(szName));
			m_pie->write("=\"");

			if ((strcmp (szName, "href") == 0) || (strcmp (szName, "xlink:href") == 0))
			{
				url = szValue;
				url.escapeURL();
				_outputXMLChar(url.utf8_str(), url.byteLength());
			}
			else
			{
				_outputXMLChar(szValue, strlen(szValue));
			}
			
			m_pie->write("\"");
		}
		if(iXID != 0)
		{
			// insert xid attribute
			m_pie->write(" ");
			m_pie->write(PT_XID_ATTRIBUTE_NAME);
			m_pie->write("=\"");
			UT_String s;
			UT_String_sprintf(s, "%d\"", iXID);
			m_pie->write(s.c_str());
		}
		if (!bIgnoreProperties && pAP->getNthProperty(0,szName,szValue))
		{
			m_pie->write(" ");
			m_pie->write(static_cast<const char*>(PT_PROPS_ATTRIBUTE_NAME));
			m_pie->write("=\"");
			m_pie->write(static_cast<const char*>(szName));
			m_pie->write(":");
			_outputXMLChar(szValue, strlen(szValue));
			UT_uint32 j = 1;
			while (pAP->getNthProperty(j++,szName,szValue))
			{
				// TMN: Patched this since I got an assert. What's the fix?
				// is it to write out a quoted empty string, or not to write
				// the property at all? For now I fixed it by the latter.
				if (*szValue)
				{
					m_pie->write("; ");
					m_pie->write(static_cast<const char*>(szName));
					m_pie->write(":");
					_outputXMLChar(szValue, strlen(szValue));
				}
			}
			m_pie->write("\"");
		}
	}
	if(strcmp(szPrefix,"math") == 0)
	{
		UT_UTF8String tag;
		const char * szPropVal = NULL;
		pAP->getAttribute("dataid",szPropVal);
		if(szPropVal != NULL)
		{
			tag = ">";
			if (bNewLineAfter) tag += "\n";
			m_pie->write (tag.utf8_str (), tag.byteLength());
			tag.clear();
			tag = "<image dataid=";
			tag += "\"";
			tag += "snapshot-svg-";
			tag += szPropVal;
			tag += "\"";
			tag += " ";
			tag += PT_PROPS_ATTRIBUTE_NAME;
			tag += "=\"";
			bool bFound = pAP->getProperty("height", szPropVal);
			UT_UTF8String sVal;
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				UT_UTF8String_sprintf(sVal,"%fin",dInch);
				tag += "height:";
				tag += sVal;
				tag += "; ";
			}
			bFound = pAP->getProperty("width", szPropVal);
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				UT_UTF8String_sprintf(sVal,"%fin",dInch);
				tag += "width:";
				tag += sVal;
			}
			tag += "\"";
			tag += "/";
			tag += ">";
			tag += "</math";
			tag += ">";
		}
		else
		{
			if (szSuffix)
				if (*szSuffix == '/')
					tag += "/";
			tag += ">";
			if (bNewLineAfter) tag += "\n";
		}
		m_pie->write (tag.utf8_str (), tag.byteLength());
	}
	else if(strcmp(szPrefix,"embed") == 0)
	{
		UT_UTF8String tag;
		const char * szPropVal = NULL;
		pAP->getAttribute("dataid",szPropVal);
		if(szPropVal != NULL)
		{
			bool has_svg;
			tag = ">";
			if (bNewLineAfter) tag += "\n";
			std::string sID = std::string("snapshot-svg-") + szPropVal;
			has_svg = m_pDocument->getDataItemDataByName(sID.c_str (), NULL, NULL, NULL);
			m_pie->write (tag.utf8_str (), tag.byteLength());
			tag.clear();
			tag = "<image dataid=";
			tag += "\"";
			tag += (has_svg)? "snapshot-svg-": "snapshot-png-";
			tag += szPropVal;
			tag += "\"";
			tag += " ";
			tag += PT_PROPS_ATTRIBUTE_NAME;
			tag += "=\"";
			bool bFound = pAP->getProperty("height", szPropVal);
			UT_UTF8String sVal;
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				UT_UTF8String_sprintf(sVal,"%fin",dInch);
				tag += "height:";
				tag += sVal;
				tag += "; ";
			}
			bFound = pAP->getProperty("width", szPropVal);
			if(bFound)
			{
				double dInch = static_cast<double>(atoi(szPropVal))/UT_LAYOUT_RESOLUTION;
				UT_UTF8String_sprintf(sVal,"%fin",dInch);
				tag += "width:";
				tag += sVal;
			}
			tag += "\"";
			tag += "/";
			tag += ">";
			tag += "</embed";
			tag += ">";
		}
		else
		{
			if (szSuffix)
				if (*szSuffix == '/')
					tag += "/";
			tag += ">";
			if (bNewLineAfter) tag += "\n";
		}
		m_pie->write (tag.utf8_str (), tag.byteLength());
	}
	else
	{
		if (szSuffix)
			if (*szSuffix == '/')
				m_pie->write ("/");
		m_pie->write (">");
		if (bNewLineAfter)
			m_pie->write ("\n");
	}
	//	m_bInTag = true;
#endif /* ENABLE_RESOURCE_MANAGER */
}

// This method is very much like _outputData but uses gchars instead of UT_UCS4_Char's.
void s_AbiWord_1_Listener::_outputXMLChar(const gchar * data, UT_uint32 length)
{
	UT_UTF8String sBuf (data, length);
	sBuf.escapeXML();
	
	m_pie->write(sBuf.utf8_str(),sBuf.byteLength());
}

// This is very much like _outputXMLChar but adds the >>> key="value" <<< padding an quoting for you 
void s_AbiWord_1_Listener::_outputXMLAttribute(const gchar * key, const gchar * value, UT_uint32 length)
{
    m_pie->write(" ");
    m_pie->write(key);
    m_pie->write("=\"");
    _outputXMLChar ( value, length );
    m_pie->write("\" ");    
}

void s_AbiWord_1_Listener::_outputXMLAttribute(const gchar * key, const std::string& value )
{
    _outputXMLAttribute( key, value.c_str(), value.length() );
}



void s_AbiWord_1_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_UTF8String sBuf;
	const UT_UCSChar * pData;

	UT_return_if_fail(sizeof(UT_Byte) == sizeof(char));
	sBuf.reserve(length);

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

		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			sBuf += "<cbr/>";
			pData++;
			break;

		case UCS_TAB:
			sBuf += "\t";
			pData++;
			break;

		case UCS_FF:					// FF -- representing a Forced-Page-Break
			sBuf += "<pbr/>";
			pData++;
			break;

		default:
			if (*pData < 0x20)         // Silently eat these characters.
				pData++;
			else
				{
					sBuf.appendUCS4 (pData, 1);
					pData++;
				}
		}
	}

	m_pie->write(sBuf.utf8_str(),sBuf.byteLength());
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
	m_bInAnnotation = false;

	m_pie->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	m_pie->write ("<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML 1.0 Strict//EN\" \"http://www.abisource.com/awml.dtd\">\n");

	/***********************************************************************************

        The fixed attributes of the <abiword> token that used to reside
        here are now found in PD_Document::setAttrProp()

	************************************************************************************/

	// we want to update the XID counter and the template status
	UT_String s;
	UT_String_sprintf(s, "%d", pDocument->getTopXID());
	
	const gchar *attr[5];
	attr[0] = "template";
	attr[1] = m_bIsTemplate ? "true" : "false";
	attr[2] = "xid-max";
	attr[3] = s.c_str();
	attr[4] = NULL;
	
	pDocument->setAttributes(attr);

	_openTag("abiword", NULL, true, pDocument->getAttrPropIndex(),false,0);

	// NOTE we output the following preamble in XML comments.
	// NOTE this information is for human viewing only.

	m_pie->write("<!-- ======================================================================== -->\n");
	m_pie->write("<!-- This file is an AbiWord document.                                        -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor.                           -->\n");
	m_pie->write("<!-- More information about AbiWord is available at http://www.abisource.com/ -->\n");
	m_pie->write("<!-- You should not edit this file by hand.                                   -->\n");
	m_pie->write("<!-- ======================================================================== -->\n\n");

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

	m_pie->write("</abiword>\n");
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
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

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
#ifndef ENABLE_RESOURCE_MANAGER
				const gchar* image_name = getObjectKey(api, static_cast<const gchar*>("dataid"));
				if (image_name)
					m_pUsedImages.insert(image_name);
#endif
				_openTag("image","/",false,api,pcr->getXID());

				return true;
				}
			case PTO_Field:
                {
                    _closeSpan();
                    _closeField();
                    _openTag("field","",false,api,pcr->getXID());
                    m_pCurrentField = pcro->getField();
                    UT_ASSERT_HARMLESS(m_pCurrentField);
                    return true;
                }
 			case PTO_Math:
                {
                    _closeSpan();
                    _closeField();
                    _openTag("math","/",false,api,pcr->getXID());
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
                    _openTag("embed","/",false,api,pcr->getXID());
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
   					_openTag("bookmark", "/",false, api,pcr->getXID(),true);
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
   						_openTag("a", "",false, api,pcr->getXID(),true);
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
   						_openTag("ann", "",false, api,pcr->getXID(),true);
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
   						_openTag("textmeta", "",false, api,pcr->getXID(),true);
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
		if(m_bOpenChar)
			_closeTag();
		_openTag("c","",false,pcr->getIndexAP(),0);
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
#ifndef ENABLE_RESOURCE_MANAGER
	PT_AttrPropIndex api = pcr->getIndexAP();
	const gchar* image_name = getObjectKey(api, static_cast<const gchar*>(PT_STRUX_IMAGE_DATAID));
	if (image_name)
		m_pUsedImages.insert(image_name);
#endif

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
			_openTag("section","",true,pcr->getIndexAP(),pcr->getXID());
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
			_openTag("table","",true,pcr->getIndexAP(),pcr->getXID());
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
			_openTag("cell","",true,pcr->getIndexAP(),pcr->getXID());
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
			_openTag("foot","",true,pcr->getIndexAP(),pcr->getXID());
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
			_openTag("annotate","",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionEndnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			m_bInBlock = false;
			_openTag("endnote","",true,pcr->getIndexAP(),pcr->getXID());
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
			_openTag("toc","",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionMarginnote:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("margin","",true,pcr->getIndexAP(),pcr->getXID());
			return true;
		}
	case PTX_SectionFrame:
		{
			_closeSpan();
            _closeField();
            _closeHyperlink();
			_closeAnnotation();
			_closeBlock();
			_openTag("frame","",true,pcr->getIndexAP(),pcr->getXID());
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
			m_pie->write("</foot>");
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
			m_pie->write("</annotate>");
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
			m_pie->write("</endnote>");
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
			m_pie->write("</toc>");
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
			m_pie->write("</frame>");
			return true;
		}
	case PTX_Block:
		{
			_closeSpan();
            _closeField();
			_closeHyperlink();
			_closeBlock();
			_openTag("p","",false,pcr->getIndexAP(),pcr->getXID());
			m_bInBlock = true;
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

/* base64-encoded object data
 */
UT_Error s_AbiWord_1_Listener::write_base64 (void * /*context*/, const char * base64, UT_uint32 length, bool /*final*/)
{
	m_pie->write (base64, length);
	m_pie->write ("\n");

	return UT_OK;
}

/* start tag & attributes
 */
UT_Error s_AbiWord_1_Listener::write_xml (void * /*context*/, const char * name, const char * const * atts)
{
	UT_UTF8String tag(" <");

	tag += name;

	const char * const * attr = atts;
	while (*attr)
		{
			tag += " ";
			tag += *attr++;
			tag += "=\"";
			tag += *attr++;
			tag += "\"";
		}
	tag += ">\n";

	m_pie->write (tag.utf8_str (), tag.byteLength());

	return UT_OK;
}

/* end tag
 */
UT_Error s_AbiWord_1_Listener::write_xml (void * /*context*/, const char * name)
{
	UT_UTF8String tag(" </");

	tag += name;
	tag += ">\n";

	m_pie->write (tag.utf8_str (), tag.byteLength());

	return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_AbiWord_1::_writeDocument(void)
{
	_setupFile();

	m_pListener = new s_AbiWord_1_Listener(getDoc(),this, m_bIsTemplate);
	if (!m_pListener)
	{
		close_gsf_handle(m_output);
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
	close_gsf_handle(m_output);

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
			m_pie->write("<styles>\n");
			bWroteOpenStyleSection = true;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s","/",true,api,0);
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
			m_pie->write("<styles>\n");
			bWroteOpenStyleSection = true;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s","/",true,api,0);
	}

	delete pStyles;
	
	if (bWroteOpenStyleSection)
		m_pie->write("</styles>\n");

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
			m_pie->write("<lists>\n");
			bWroteOpenListSection = true;
		}
		m_pie->write("<l");
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
				m_pie->write(" ");
				m_pie->write(s.utf8_str());
				m_pie->write("=\"");
				m_pie->write(vAttrs[i+1].utf8_str());
				m_pie->write("\"");
			}
		}
		m_pie->write("/>\n");
		// No, no, you can't g_free attr and expect good things to happen.
		FREEP(attr0);
	}


#undef LCheck

	if (bWroteOpenListSection)
		m_pie->write("</lists>\n");

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

  m_pie->write("<metadata>\n");

  std::map<std::string, std::string>::const_iterator iter = ref.begin();
  for ( ; iter != ref.end(); ++iter ) {
	  if( !iter->second.empty() ) {
	      m_pie->write( "<m key=\"" ) ;
	      _outputXMLChar ( iter->first ) ;
	      m_pie->write ( "\">" ) ;
	      _outputXMLChar ( iter->second ) ;
	      m_pie->write ( "</m>\n" ) ;
	  }
  }

  m_pie->write("</metadata>\n");
}

void s_AbiWord_1_Listener::_handleRDF(void)
{
  m_pie->write("<rdf>\n");

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

          
          m_pie->write("<t ");
          _outputXMLAttribute( "s", subject.toString() );
          _outputXMLAttribute( "p", predicate.toString() );
          {
              std::stringstream ss;
              ss << object.getObjectType();
              _outputXMLAttribute( "objecttype", ss.str() );
          }
          _outputXMLAttribute( "xsdtype",    object.getXSDType() );
          m_pie->write(" >");
          UT_UTF8String esc = object.toString().c_str();
          _outputXMLChar ( esc.utf8_str(), esc.byteLength() ) ;
          m_pie->write("</t>\n");
      }
  }
  
  m_pie->write("</rdf>\n");
}

void s_AbiWord_1_Listener::_handlePageSize(void)
{
  //
  // Code to write out the PageSize Definitions to disk
  //
	m_pie->write("<pagesize pagetype=\"");
	m_pie->write(m_pDocument->m_docPageSize.getPredefinedName());
	m_pie->write("\"");

	m_pie->write(" orientation=\"");
	if(m_pDocument->m_docPageSize.isPortrait() == true)
	        m_pie->write("portrait\"");
	else
	        m_pie->write("landscape\"");
	UT_Dimension docUnit = m_pDocument->m_docPageSize.getDims();

	UT_LocaleTransactor t(LC_NUMERIC, "C");
	m_pie->write( UT_String_sprintf(" width=\"%f\"", m_pDocument->m_docPageSize.Width(docUnit)).c_str() );
	m_pie->write( UT_String_sprintf(" height=\"%f\"",m_pDocument->m_docPageSize.Height(docUnit)).c_str() );
	m_pie->write(" units=\"");
	m_pie->write(UT_dimensionName(docUnit));
	m_pie->write("\"");
	m_pie->write( UT_String_sprintf(" page-scale=\"%f\"/>\n",m_pDocument->m_docPageSize.getScale()).c_str() );
}

void s_AbiWord_1_Listener::_handleDataItems(void)
{
#ifdef ENABLE_RESOURCE_MANAGER
	m_pDocument->resourceManager().write_xml (0, *this);
#else
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
			m_pie->write("<data>\n");
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
		   bbEncoded.append(buf, off);
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
	   		m_pie->write("<d name=\"");
			// We assume that UT_gchar is equivalent to char.
			// That's not really a good assumption, but, hey.
			// TODO: make szName, szMimeType be UT_gchars.
			_outputXMLChar(szName, strlen(szName));
		   	if (!mimeType.empty())
			{
			   m_pie->write("\" mime-type=\"");
			   _outputXMLChar(mimeType.c_str(), mimeType.size());
			}
		   	if (encoded)
		    {
			   	m_pie->write("\" base64=\"yes\">\n");
				// break up the Base64 blob as a series lines
				// like MIME does.

				UT_uint32 jLimit = bbEncoded.getLength();
				UT_uint32 jSize;
				UT_uint32 j;
				for (j=0; j<jLimit; j+=72)
				{
					jSize = UT_MIN(72,(jLimit-j));
					m_pie->write(reinterpret_cast<const char *>(bbEncoded.getPointer(j)),jSize);
					m_pie->write("\n");
				}
			}
		   	else
		    {
		     	m_pie->write("\" base64=\"no\">\n");
			   	m_pie->write(reinterpret_cast<const char*>(bbEncoded.getPointer(0)), bbEncoded.getLength());
			}
			m_pie->write("</d>\n");
		}
	}

	if (bWroteOpenDataSection)
		m_pie->write("</data>\n");
#endif
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
			s = UT_std_string_sprintf("<revisions show=\"%d\" mark=\"%d\" show-level=\"%d\" auto=\"%d\">\n",
							  m_pDocument->isShowRevisions(),
							  m_pDocument->isMarkRevisions(),
							  m_pDocument->getShowRevisionId(),
							  m_pDocument->isAutoRevisioning());
			
			m_pie->write(s.c_str());
			bWroteOpenRevisionsSection = true;
		}

		s = UT_std_string_sprintf("<r id=\"%u\" time-started=\"%ld\" version=\"%u\">",
						  pRev->getId(),
						  pRev->getStartTime(),
						  pRev->getVersion());
		
		m_pie->write(s.c_str());

		if(pRev->getDescription())
		{
			_outputData(pRev->getDescription(), UT_UCS4_strlen(pRev->getDescription()));
		}
		

		m_pie->write("</r>\n");
	}

	if (bWroteOpenRevisionsSection)
		m_pie->write("</revisions>\n");

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
			UT_UTF8String_sprintf(s, "<history version=\"%d\" edit-time=\"%d\" last-saved=\"%d\" uid=\"%s\">\n",
							  m_pDocument->getDocVersion(),
							  static_cast<UT_sint32>(m_pDocument->getEditTime()),
							  static_cast<UT_sint32>(m_pDocument->getLastSavedTime()),
							  m_pDocument->getDocUUIDString());
			
			m_pie->write(s.utf8_str());
			bWroteOpenSection = true;
		}

		UT_UTF8String_sprintf(s, "<version id=\"%d\" started=\"%d\" uid=\"%s\" auto=\"%d\" top-xid=\"%d\"/>\n",
						  iVersion, static_cast<UT_sint32>(tStarted), hUid.utf8_str(),bAuto, iXID);
		
		m_pie->write(s.utf8_str());
	}

	if (bWroteOpenSection)
		m_pie->write("</history>\n");

	return;
}

void s_AbiWord_1_Listener::_handleAuthors(void)
{
	UT_sint32 nAuthors = m_pDocument-> getNumAuthors();
	if(nAuthors <= 0)
		return;
	m_pie->write("<authors>\n");
	UT_sint32 i = 0;
	UT_String sVal;
	for(i=0;i<nAuthors;i++)
	{
		pp_Author * pAuthor = m_pDocument->getNthAuthor(i);
		m_pie->write("<author id=\"");
		UT_String_sprintf(sVal,"%d",pAuthor->getAuthorInt());
		m_pie->write(sVal.c_str());
		m_pie->write("\" ");
		PP_AttrProp * pAP = pAuthor->getAttrProp();
		if(pAP->getPropertyCount()>0)
		{
			m_pie->write(static_cast<const char*>(PT_PROPS_ATTRIBUTE_NAME));
			m_pie->write("=\"");
			const gchar * szName = NULL;
			const gchar * szValue = NULL;
			UT_uint32 j = 0;
			while (pAP->getNthProperty(j++,szName,szValue))
			{
				if (szName && *szName && szValue && *szValue)
				{
					if(j>1)
						m_pie->write("; ");
					m_pie->write(static_cast<const char*>(szName));
					m_pie->write(":");
					_outputXMLChar(szValue, strlen(szValue));
				}
			}
		m_pie->write("\"");
		}
		m_pie->write("/>\n");
	}
	m_pie->write("</authors>\n");
}

