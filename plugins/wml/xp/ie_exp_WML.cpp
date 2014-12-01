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


#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_locale.h"
#include "ut_units.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "ie_exp_WML.h"
#include "ie_TOC.h"
#include "pd_Document.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "fd_Field.h"
#include "xap_App.h"
#include "pd_Style.h"
#include "ap_Strings.h"
#include "ut_path.h"
#include "xap_EncodingManager.h"

#include "ut_string_class.h"

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

IE_Exp_WML_Sniffer::IE_Exp_WML_Sniffer (const char * _name) :
  IE_ExpSniffer(_name)
{
  // 
}

bool IE_Exp_WML_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".wml"));
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
	m_pListener = new s_WML_Listener(getDoc(),this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	DELETEP(m_pListener);

	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

s_WML_Listener::s_WML_Listener(PD_Document * pDocument,
			       IE_Exp_WML * pie)
	: m_pDocument(pDocument),
	m_pie(pie),
	m_bInSection(false),
	m_bInBlock(false),
	m_bInSpan(false),
	m_bInAnchor(false),
	m_bInHyperlink(false),
	m_bInCell(false),
	m_bInRow(false),
	m_bInTable(false),
	m_bPendingClose(false),
	m_bWasSpace(false),
	m_iCards(1),
	m_iTableDepth(0),
	mTableHelper(pDocument),
	m_toc(0),
	m_heading_count(0)
{
	m_pie->write("<!DOCTYPE wml PUBLIC \"-//PHONE.COM//DTD WML 1.1//EN\"\n");
	m_pie->write("\t\"http://www.openwave.com/dtd/wml11.dtd\" >\n");

	/* keep ads to a minimum. size is at a premium */
	m_pie->write("<!-- This WML file was created by AbiWord -->\n");
	m_pie->write("<!-- See http://www.abisource.com/ -->\n\n");
	m_pie->write("<wml>\n");
	_handleMetaData();

	m_toc = new IE_TOCHelper(m_pDocument);
}

s_WML_Listener::~s_WML_Listener()
{
	_closeSection();

	if(m_bPendingClose)
		m_pie->write("</card>\n");

	m_pie->write("</wml>\n");
	_handleDataItems();

	UT_VECTOR_FREEALL(char*, m_utvDataIDs);
	DELETEP(m_toc);
}

/*****************************************************************/
/*****************************************************************/

static char *_stripSuffix(const char* from, char delimiter)
{
    char * fremove_s = static_cast<char *>(g_try_malloc(strlen(from)+1));
    strcpy(fremove_s, from);

    char * p = fremove_s + strlen(fremove_s);
    while ((p >= fremove_s) && (*p != delimiter))
        p--;

    if (p >= fremove_s)
	*p = '\0';

    return fremove_s;
}

static char *_stripSuffix(const UT_UTF8String &from, char delimiter)
{
  return _stripSuffix(from.utf8_str(), delimiter);
}

void s_WML_Listener::_closeAnchor()
{
	if(!m_bInAnchor)
	{
		return;
	}

	m_pie->write("</anchor>");
	m_bInAnchor = false;
}

void s_WML_Listener::_closeHyperlink()
{
	if(!m_bInHyperlink)
	{
		return;
	}

	m_pie->write("</a>");
	m_bInHyperlink = false;
}

void s_WML_Listener::_closeTable()
{
	if(!m_bInTable)
	{
		return;
	}

	_closeCell();
	_closeRow();
	m_pie->write("</table>\n</p>\n");
	m_bInTable = false;
}

void s_WML_Listener::_closeRow()
{
	if(!m_bInRow || !m_bInTable)
	{
		return;
	}

	m_pie->write("</tr>\n");
	m_bInRow = false;
}

void s_WML_Listener::_closeCell()
{
	if(!m_bInCell || !m_bInTable)
	{
		return;
	}

	m_pie->write("</td>\n");
	m_bInCell = false;

	if ( mTableHelper.getNumCols () == mTableHelper.getRight () )
	{
		// logical end of a row
		_closeRow();
	} 
}

void s_WML_Listener::_openTable(PT_AttrPropIndex api)
{
	if(!m_bInSection)
		_openSection(api);

	if(m_bInTable)
	{
		return; //no nested tables
	}

	UT_sint32 nCols = mTableHelper.getNumCols();
  
	UT_UTF8String tableSpec = UT_UTF8String_sprintf("<p>\n<table columns=\"%d\">\n",nCols);
	m_pie->write(tableSpec.utf8_str(), tableSpec.size());
	m_bInTable = true;
}

void s_WML_Listener::_openCell(void)
{
	if(!m_bInTable)
	{
		return;
	}

	//note: wml 1.1 doesn't support colspan or rowspan

	if(mTableHelper.isNewRow())
	{
		// beginning of a new row
		_closeCell();
		_closeRow();
		_openRow();
	}

	m_pie->write("<td>");
	m_bInCell = true;
}

void s_WML_Listener::_openRow(void)
{
	if(!m_bInRow)
	{
		m_pie->write("<tr>\n");
		m_bInRow = true;
	}
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_closeSection(void)
{
	_closeSpan();
	_closeBlock();

	if(m_bInSection)
		m_bPendingClose = true;
}

void s_WML_Listener::_openSection(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar * szValue = NULL;

	if(pAP && bHaveProp && (pAP->getAttribute("strux-image-dataid", szValue)))
	{
		_openSpan(api);
		_handleImage(api, true);
		_closeSpan();
		return; //don't open a new section
	}

	if(m_bPendingClose)
	{
		m_iCards++;
		m_pie->write(UT_UTF8String_sprintf("<do type=\"accept\" label=\"Next\"><go href=\"#card%d\"/></do>\n", m_iCards).utf8_str());
		m_pie->write("</card>\n");
		m_bInSection = false;
		m_bPendingClose = false;
	}

	if(!m_bInSection)
	{
		m_pie->write(UT_UTF8String_sprintf("<card id=\"card%d\" ordered=\"true\">\n", m_iCards).utf8_str());
		m_bInSection = true;
	}
}

void s_WML_Listener::_closeBlock(void)
{
	_closeAnchor();
	_closeHyperlink();

	if(!m_bInBlock)
	{
		return;
	}
	if(!m_bInTable)
	{
		m_pie->write("</p>\n");
	}

	m_bInBlock = false;
	return;
}

bool s_WML_Listener::_styleDescendsFrom(const char * style_name, const char * base_name)
{
	PD_Style * style = 0;
	m_pDocument->getStyle (style_name, &style);
	UT_sint32 iLoop = 0;

	while (style && (iLoop < 10))
	{
		if (g_ascii_strcasecmp (base_name, style->getName ()) == 0)
			return true;

		style = style->getBasedOn ();
		iLoop++;
	}

	return false;
}

void s_WML_Listener::_openParagraph(PT_AttrPropIndex api)
{
	xxx_UT_DEBUGMSG(("WML Export: OpenParagraph called!\n"));

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if(!m_bInSection)
		_openSection(api);

	if (m_bInTable) //<p> tags aren't allowed inside of table-related tags
	{
		m_bInBlock = true;
		return;
	}
	
	if (bHaveProp && pAP)
	{
		const gchar * szValue;

		m_pie->write("<p");
		if (pAP->getProperty("text-align", szValue))
		{
		  if (!strcmp(szValue, "center")) 
			m_pie->write(" align=\"center\"");
		  else if (!strcmp(szValue, "right"))
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

	const gchar * szStyle;
	bool have_style  = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, szStyle);

	if (have_style && szStyle)
	{
		if (m_toc)
		{
			if (_styleDescendsFrom(szStyle, "Heading 1") || _styleDescendsFrom(szStyle, "Heading 2") ||
			    _styleDescendsFrom(szStyle, "Heading 3") || _styleDescendsFrom(szStyle, "Heading 4"))
			{
				UT_UTF8String tocAnchor(UT_UTF8String_sprintf("<anchor id=\"AbiTOC%d\"></anchor>", m_heading_count));
				m_pie->write(tocAnchor.utf8_str());
				m_heading_count++;
			}
		}
	}

	m_bInBlock = true;
}

void s_WML_Listener::_emitTOC (PT_AttrPropIndex api) {
	int level1_depth = 0;
	int level2_depth = 0;
	int level3_depth = 0;
	int level4_depth = 0;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);
	bool bEmitHeading = true;
	const gchar * szValue = 0;
	std::string tocHeadingUTF8;

	_closeSpan();
	_closeBlock();

	if(bHaveProp && pAP && pAP->getProperty("toc-has-heading", szValue) && szValue) //check to see if the TOC heading is hidden
	{
		if(atoi(szValue) == 0)
			bEmitHeading = false;
	}

	if(bEmitHeading)
	{
		if(bHaveProp && pAP && pAP->getProperty("toc-heading", szValue) && szValue) // user-defined TOC heading
		{
			tocHeadingUTF8 = szValue;
		}
		else
		{
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
			if(pSS)
				pSS->getValueUTF8(AP_STRING_ID_TOC_TocHeading, tocHeadingUTF8);
		}

		//TODO: query the style properties to determine whether to center the TOC heading
		m_pie->write("<p>");
		m_pie->write(UT_escapeXML(tocHeadingUTF8));
		m_pie->write("</p>\n");
	}

	for (int i = 0; i < m_toc->getNumTOCEntries(); i++) {
		int tocLevel = 0;		
		
		UT_UCS4String tocText(m_toc->getNthTOCEntry(i, &tocLevel).utf8_str());

		m_pie->write("<p>");
			
		UT_UCS4String tocLevelText;
		if(tocLevel == 1) {
			level1_depth++;
			level2_depth = level3_depth = level4_depth = 0;
				
			tocLevelText = UT_UTF8String_sprintf("[%d] ", level1_depth).ucs4_str();
		} else if(tocLevel == 2) {
			level2_depth++;
			level3_depth = level4_depth = 0;
			tocLevelText = UT_UTF8String_sprintf("[%d.%d] ", level1_depth, level2_depth).ucs4_str();
		} else if(tocLevel == 3) {
			level3_depth++;
			level4_depth = 0;
			tocLevelText = UT_UTF8String_sprintf("[%d.%d.%d] ", level1_depth, level2_depth, level3_depth).ucs4_str();
		} else if(tocLevel == 4) {
			level4_depth++;
			tocLevelText = UT_UTF8String_sprintf("[%d.%d.%d.%d] ", level1_depth, level2_depth, level3_depth, level4_depth).ucs4_str();
		}
			
		UT_UTF8String tocLink(UT_UTF8String_sprintf("<a href=\"#AbiTOC%d\">", i));
		m_pie->write(tocLink.utf8_str(), tocLink.byteLength());
		_outputDataUnchecked (tocLevelText.ucs4_str(), tocLevelText.length());
		_outputDataUnchecked (tocText.ucs4_str(), tocText.length());
		m_pie->write("</a>", 4);
		m_pie->write("</p>");
	}
}

/*****************************************************************/
/*****************************************************************/

bool s_WML_Listener::populate(fl_ContainerLayout* /*sfh*/,
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
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();

			switch (pcro->getObjectType())
			{
			case PTO_Image:
			{
				_handleImage(api);
				return true;
			}

			case PTO_Field:
			{
				_handleField(pcro, api);
				return true;
			}

			case PTO_Hyperlink:
			{
				_handleHyperlink(api);
				return true;
			}

			case PTO_Bookmark:
			{
				_handleBookmark(api);
				return true;
			}

			case PTO_Embed:
			{
				_handleEmbedded(api);
				return true;
			}

			case PTO_Math:
			{
				_handleMath(api);
				return true;
			}

			case PTO_Annotation:
			{
				return true;
			}

			default:
				UT_ASSERT_HARMLESS(UT_TODO);
				return true;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT_HARMLESS(UT_TODO);
		return true;
	}
}

bool s_WML_Listener::populateStrux(pf_Frag_Strux* sdh,
				      const PX_ChangeRecord * pcr,
				      fl_ContainerLayout* * psfh)
{
	UT_ASSERT_HARMLESS(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	case PTX_SectionFrame:
	case PTX_SectionHdrFtr:
	{
		if(!m_bInTable)
		{
			_closeSection();
			_openSection(pcr->getIndexAP());
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

	case PTX_SectionTable:
	{
		m_iTableDepth++;
		if(m_iTableDepth == 1)
		{
		    _closeSpan();
		    _closeBlock();
		    mTableHelper.OpenTable(sdh,pcr->getIndexAP());
		    _openTable(pcr->getIndexAP());
		}
	    return true;
	}

	case PTX_EndTable:
	{
		m_iTableDepth--;
		if(m_iTableDepth == 0)
		{
		    _closeBlock();
		    _closeTable();
	    	mTableHelper.CloseTable();
		}
	    return true;
	}

	case PTX_SectionCell:
	{
		if(m_iTableDepth == 1)
		{
		    mTableHelper.OpenCell(pcr->getIndexAP()) ;
		    _closeSpan();
	    	_closeBlock();
		    _openCell();
		}
	    return true;
	}

	case PTX_EndCell:
	{
		if(m_iTableDepth == 1)
		{
		    _closeBlock();
		    _closeCell();
	    	mTableHelper.CloseCell();
		}
	    return true;
	}

	case PTX_EndFrame:
	{
		if(!m_bInTable)
		{
			_closeSection();
		}
		return true;
	}

	case PTX_EndEndnote:
	case PTX_EndFootnote:
	case PTX_EndAnnotation:
	case PTX_SectionEndnote:
	case PTX_SectionFootnote:
	case PTX_SectionAnnotation:
	{
		return true;
	}

	case PTX_SectionTOC:
	{
		_emitTOC(pcr->getIndexAP());
		return true;
	}

	case PTX_EndTOC:
	  {
	    return true;
	  }

	case PTX_EndMarginnote:
	case PTX_SectionMarginnote:
	default:
		UT_ASSERT_HARMLESS(UT_TODO);
		return true;
	}
}

bool s_WML_Listener::change(fl_ContainerLayout* /*sfh*/,
			       const PX_ChangeRecord * /*pcr*/)
{
  UT_ASSERT_NOT_REACHED();	    // this function is not used.
  return false;
}

bool s_WML_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
				     const PX_ChangeRecord * /*pcr*/,
				     pf_Frag_Strux* /*sdh*/,
				     PL_ListenerId /* lid */,
				     void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
								 PL_ListenerId /* lid */,
																 fl_ContainerLayout* /* sfhNew */))
{
  UT_ASSERT_NOT_REACHED();	    // this function is not used.
  return false;
}

bool s_WML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();
	return false;
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleMetaData()
{
	std::string metaProp;
	m_pie->write("<head>\n");

	#define WRITEMETA(name)\
		metaProp = UT_escapeXML(metaProp);\
		if(metaProp.length())\
		  m_pie->write(UT_std_string_sprintf("<meta name=\"%s\" content=\"%s\"/>\n", name, metaProp.c_str()).c_str()); \

	if (m_pDocument->getMetaDataProp (PD_META_KEY_TITLE, metaProp) && metaProp.size())
	{
		WRITEMETA("title");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR, metaProp) && metaProp.size())
	{
		WRITEMETA("author");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_SUBJECT, metaProp) && metaProp.size())
	{
		WRITEMETA("subject");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_DESCRIPTION, metaProp) && metaProp.size())
	{
		WRITEMETA("description");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_PUBLISHER, metaProp) && metaProp.size())
	{
		WRITEMETA("publisher");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_CONTRIBUTOR, metaProp) && metaProp.size())
	{
		WRITEMETA("contributor");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_DATE, metaProp) && metaProp.size())
	{
		WRITEMETA("date");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_SOURCE, metaProp) && metaProp.size())
	{
		WRITEMETA("source");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_RELATION, metaProp) && metaProp.size())
	{
		WRITEMETA("relation");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_COVERAGE, metaProp) && metaProp.size())
	{
		WRITEMETA("coverage");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_RIGHTS, metaProp) && metaProp.size())
	{
		WRITEMETA("rights");
	}
	if (m_pDocument->getMetaDataProp (PD_META_KEY_KEYWORDS, metaProp) && metaProp.size())
	{
		WRITEMETA("keywords");
	}

	#undef WRITEMETA

	m_pie->write("</head>\n");
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleEmbedded(PT_AttrPropIndex api)
{
	//this should never happen, but it doesn't hurt to make sure
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const gchar* szValue = 0;
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("dataid", szValue))

	UT_UTF8String buf = "snapshot-png-";
	buf += szValue;

	char* dataid = g_strdup(buf.utf8_str());
	m_utvDataIDs.push_back(dataid);
	buf += ".png";

	m_pie->write("<img alt=\"AbiWord Chart\" src=\""); //an alt attribute is required in WML
	m_pie->write(UT_go_basename(m_pie->getFileName()).utf8_str());
	m_pie->write("_data/");
	m_pie->write(buf.utf8_str());
	m_pie->write("\"");

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("height", szValue))
	{
		buf.clear();
		UT_UTF8String_sprintf(buf, "%f", UT_convertToDimension(szValue, DIM_PX));
		m_pie->write(" height=\"");
		m_pie->write(buf.utf8_str());
		m_pie->write("\"");
	}
	if(pAP->getProperty("width", szValue))
	{
		buf.clear();
		UT_UTF8String_sprintf(buf, "%f", UT_convertToDimension(szValue, DIM_PX));
		m_pie->write(" width=\"");
		m_pie->write(buf.utf8_str());
		m_pie->write("\"");
	}
	if(pAP->getProperty("lang", szValue))
	{
		m_pie->write(" xml:lang=\"");
		m_pie->write(szValue);
		m_pie->write("\"");
	}

	m_pie->write("/>");
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api)
{
	//this can happen with certain .doc files
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const PP_AttrProp * pAP = NULL;
	const gchar* szValue = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("type", szValue));

	UT_UTF8String buf;
	fd_Field * field = pcro->getField();

	m_pie->populateFields ();

	if(strcmp(szValue, "list_label") != 0)
	{
		buf = field->getValue();  //retrieve the field text
		buf.escapeXML();

		if(buf.length())
		{
			m_pie->write(buf.utf8_str());
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleBookmark(PT_AttrPropIndex api)
{
	//this can happen with certain .doc files
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const PP_AttrProp * pAP = NULL;
	const gchar* szValue = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	UT_UTF8String buf;

	if(bHaveProp && pAP && pAP->getAttribute("type", szValue))
	{
		_closeAnchor(); //nested anchors are not allowed

		if(strcmp(szValue, "start") == 0 && (m_bInHyperlink == 0) && pAP->getAttribute("name", szValue))
		{
			buf = szValue;
			buf.escapeXML();

			if(buf.length())
			{
				m_pie->write("<anchor id=\"");
				m_pie->write(buf.utf8_str());
				m_pie->write("\">");
				m_bInAnchor = true;
			}
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleHyperlink(PT_AttrPropIndex api)
{
	//this can happen with certain .doc files
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const PP_AttrProp * pAP = NULL;
	const gchar* szValue = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	UT_UTF8String buf;

	if(bHaveProp && pAP && pAP->getAttribute("xlink:href", szValue))
	{
		buf = szValue;
		buf.escapeURL();
		_closeAnchor();
		_closeHyperlink();

		if(buf.length())
		{
			m_pie->write("<a href=\"");
			m_pie->write(buf.utf8_str());
			m_pie->write("\">");
			m_bInHyperlink = true;
		}
	}
	else
	{
		_closeHyperlink();
	}
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleMath(PT_AttrPropIndex api)
{
	//this should never happen, but it doesn't hurt to make sure
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const gchar* szValue = 0;
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("dataid", szValue));

	UT_UTF8String buf = "snapshot-png-";
	buf += szValue;

	char *dataid = g_strdup(buf.utf8_str());
	m_utvDataIDs.push_back(dataid);
	buf += ".png";

	m_pie->write("<img alt=\"AbiWord Equation\" src=\""); //an alt attribute is required in WML
	m_pie->write(UT_go_basename(m_pie->getFileName()).utf8_str());
	m_pie->write("_data/");
	m_pie->write(buf.utf8_str());
	m_pie->write("\"");

	if(pAP->getProperty("lang", szValue))
	{
		m_pie->write(" xml:lang=\"");
		m_pie->write(szValue);
		m_pie->write("\"");
	}

	m_pie->write("/>");
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleImage(PT_AttrPropIndex api, bool bPos)
{
	//this should never happen, but it doesn't hurt to make sure
	if((m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	const PP_AttrProp * pAP = NULL;
	const gchar* szValue = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if(!(bHaveProp && pAP))
		return;

	bool bRes = false;
	if(bPos)
		bRes = pAP->getAttribute("strux-image-dataid", szValue);
	else
		bRes = pAP->getAttribute("dataid", szValue);

	if(!(bRes && szValue && *szValue))
		return;

	char * dataid = g_strdup(static_cast<const char*>(szValue));
	UT_return_if_fail(dataid);

	m_utvDataIDs.push_back(dataid);
	char * temp = _stripSuffix(UT_go_basename(szValue), '_');
	char * fstripped = _stripSuffix(temp, '.');

	UT_UTF8String buf = fstripped;
	FREEP(temp);
	FREEP(fstripped);

	std::string sExt;
	if(!m_pDocument->getDataItemFileExtension(dataid, sExt))
		sExt = ".png";
	buf += sExt;

	m_pie->write("<img alt=\""); //an alt attribute is required in WML

	if(pAP->getAttribute("alt", szValue))  // check for existing alt text
	{
		UT_UTF8String alt = szValue;
		alt.escapeXML();
		m_pie->write(alt.utf8_str());
	}
	else  // fall back to the file name
	{
		m_pie->write("AbiWord Image ");
		m_pie->write(buf.utf8_str());
	}

	m_pie->write("\" src=\"");
	m_pie->write(UT_go_basename(m_pie->getFileName()).utf8_str());
	m_pie->write("_data/");
	m_pie->write(buf.utf8_str());
	m_pie->write("\"");

	const gchar * szWidth = 0;
	const gchar * szHeight = 0;
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("width", szWidth) && szWidth && *szWidth)
	{
		UT_UTF8String_sprintf(buf, "%f", UT_convertToDimension(szWidth, DIM_PX));
		m_pie->write (" width=\"");
		m_pie->write (buf.utf8_str());
		m_pie->write ("\"");
	}
	if(pAP->getProperty("height", szHeight) && szHeight && *szHeight)
	{
		UT_UTF8String_sprintf(buf, "%f", UT_convertToDimension(szHeight, DIM_PX));
		m_pie->write (" height=\"");
		m_pie->write (buf.utf8_str());
		m_pie->write ("\"");
	}
	if(pAP->getProperty("lang", szValue) && szValue && *szValue)
	{
		m_pie->write(" xml:lang=\"");
		m_pie->write(szValue);
		m_pie->write("\"");
	}
				
	m_pie->write("/>");
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_handleDataItems(void)
{
 	const char * szName;
    std::string mimeType;
	const UT_ByteBuf * pByteBuf;

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,
                                                    &mimeType)); k++)
	{
		UT_sint32 loc = -1;
		for (UT_sint32 i = 0; i < m_utvDataIDs.getItemCount(); i++)
		{
			if(strcmp(static_cast<const char*>(m_utvDataIDs[i]), szName) == 0)
			{
				loc = i;
				break;
			}
		}

		if(loc > -1)
		{
			UT_UTF8String fname;
			UT_UTF8String_sprintf(fname, "%s_data", m_pie->getFileName());

			/* UT_sint32 result = */ UT_go_directory_create(fname.utf8_str(), NULL);
			if (0 /* result < 0 */)
			{
				UT_DEBUGMSG(("WML Export: Failed to create directory\n"));
				/* we might want to return an error here, 
				 * but I don't think so. */
			}

			if (mimeType == "image/svg+xml")
			{
				UT_UTF8String_sprintf(fname, "%s/%s_%d.svg", fname.utf8_str(), szName, loc);
			}
			else if (mimeType == "application/mathml+xml")
			{
				UT_UTF8String_sprintf(fname, "%s/%s_%d.mathml", fname.utf8_str(), szName, loc);
			}
			else if (mimeType == "image/png")// PNG Image
			{
				char * temp = _stripSuffix(UT_go_basename(szName), '_');
				char * fstripped = _stripSuffix(temp, '.');
				FREEP(temp);
				UT_UTF8String_sprintf(fname, "%s/%s.png", fname.utf8_str(), fstripped);
				FREEP(fstripped);
			}
			else
			{
				UT_DEBUGMSG(("WML export: Unhandled/ignored mime type: %s\n", mimeType.c_str()));
			}

			GsfOutput *fp = UT_go_file_create (fname.utf8_str(), NULL);
			
			if(!fp)
			  continue;
			
			gsf_output_write(fp, pByteBuf->getLength(), (const guint8 *)pByteBuf->getPointer(0));
			gsf_output_close(fp);
			g_object_unref(G_OBJECT(fp));
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void s_WML_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock || m_bInHyperlink || m_bInAnchor || (m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (bHaveProp && pAP)
	{
		const gchar * szValue = NULL;

		if ((pAP->getProperty("font-weight", szValue))
			&& !strcmp(szValue, "bold"))
		{
			m_pie->write("<b>");
		}
		
		if ((pAP->getProperty("font-style", szValue))
			&& !strcmp(szValue, "italic"))
		{
			m_pie->write("<i>");
		}
		
		if ((pAP->getProperty("text-decoration", szValue)))
		{
			if(strstr(szValue,"underline"))
				m_pie->write("<u>");
		}

		// In my WML world...
		// superscript = big
		// subscript = small
		if (pAP->getProperty("text-position", szValue))
		{
			if (!strcmp("superscript", szValue))
			{
				m_pie->write("<big>");
			}
			else if (!strcmp("subscript", szValue))
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
	if (!m_bInBlock || (m_bInTable && (!m_bInRow || !m_bInCell)))
	{
		return;
	}

	_outputDataUnchecked(data, length);
}

void s_WML_Listener::_outputDataUnchecked(const UT_UCSChar * data, UT_uint32 length)	
{
	// TODO deal with unicode.
	// TODO for now, just squish it into ascii.
	
	UT_UTF8String sBuf;
	const UT_UCSChar * pData;
	m_bWasSpace = false;

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

		case UCS_VTAB:					// column break, export as a linebreak for now
		case UCS_FF:					// page break, export as a linebreak for now
		case UCS_LF:					// LF -- representing a Forced-Line-Break
			sBuf += "<br/>";
			pData++;
			break;

		case ' ':
		case UCS_TAB:
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
		{
			if(*pData < 0x20)  //invalid xml chars
			{
				pData++;
			}
			else
			{
				sBuf.appendUCS4(pData, 1);
				pData++;
			}
			m_bWasSpace = false;
		}
		}
	}

	m_pie->write(sBuf.utf8_str(),sBuf.byteLength());
}

void s_WML_Listener::_closeSpan(void)
{
	if (!m_bInSpan || m_bInHyperlink || m_bInAnchor)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{
		const gchar * szValue = NULL;
		
		if (pAP->getProperty("text-position", szValue))
		{
			if (!strcmp("superscript", szValue))
			{
				m_pie->write("</big>");
			}
			else if (!strcmp("subscript", szValue))
			{
				m_pie->write("</small>");
			}
		}

		if ((pAP->getProperty("text-decoration", szValue)))
		{
			if(strstr(szValue,"underline"))
				m_pie->write("</u>");				
		}

		if ((pAP->getProperty("font-style", szValue))
			&& !strcmp(szValue, "italic"))
		{
		  m_pie->write("</i>");
		}
		
		if ((pAP->getProperty("font-weight", szValue))
			&& !strcmp(szValue, "bold"))
		{
		  m_pie->write("</b>");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}
