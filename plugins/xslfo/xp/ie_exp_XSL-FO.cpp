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

#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_locale.h"
#include "pt_Types.h"
#include "ie_impexp_XSL-FO.h"
#include "ie_exp_XSL-FO.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pp_Property.h"
#include "pd_Style.h"
#include "fd_Field.h"
#include "xap_App.h"

#include "fp_PageSize.h"


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

IE_Exp_XSL_FO_Sniffer::IE_Exp_XSL_FO_Sniffer (const char * _name) :
  IE_ExpSniffer(_name)
{
  // 
}

bool IE_Exp_XSL_FO_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix, ".fo"));
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
	DELETEP(m_pListener);
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}  

/*****************************************************************/
/*****************************************************************/

static char *_stripSuffix(const char* from, char delimiter)
{
	// taken from DocBook exporter
	char * fremove_s = (char *)g_try_malloc(strlen(from)+1);
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

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_tagClose(UT_uint32 tagID, const UT_UTF8String & content, bool newline)
{
	UT_uint32 i = 0;

	m_pie->write("</");
	m_pie->write("fo:");  //this might be unnecessary one day, so keep this here for easier removal (see bug 4355, comment 6)
	m_pie->write(content.utf8_str());
	m_pie->write(">");

	if(newline)
		m_pie->write("\n");

	m_utnsTagStack.pop((UT_sint32*)&i);
	m_iLastClosed = i;
	xxx_UT_DEBUGMSG(("XSL-FO export: Popping %d off of stack\n",i));

	if(i != tagID)
	{
		UT_DEBUGMSG(("XSL-FO export: possible mismatched tag. Requested: %d, Popped: %d\n",tagID,i));
	}
}

void s_XSL_FO_Listener::_tagOpen(UT_uint32 tagID, const UT_UTF8String & content, bool newline)
{
	m_pie->write("<");
	m_pie->write("fo:");  //this might be unnecessary one day, so keep this here for easier removal (see bug 4355, comment 6)
	m_pie->write(content.utf8_str());
	m_pie->write(">");

	if(newline)
		m_pie->write("\n");

	m_utnsTagStack.push(tagID);
	xxx_UT_DEBUGMSG(("XSL-FO export: Pushing %d onto stack\n",tagID));
}

void s_XSL_FO_Listener::_tagOpenClose(const UT_UTF8String & content, bool suppress, bool newline)
{
	m_pie->write("<");
	m_pie->write("fo:"); //this might be unnecessary one day, so keep this here for easier removal (see bug 4355, comment 6)
	m_pie->write(content.utf8_str());

	if(suppress)
		m_pie->write("/>");
	else
	{
		m_pie->write("></");
		m_pie->write("fo:"); //this might be unnecessary one day, so keep this here for easier removal (see bug 4355, comment 6)
		m_pie->write(content.utf8_str());
		m_pie->write(">");
	}

	if(newline)
		m_pie->write("\n");
}

UT_uint32 s_XSL_FO_Listener::_tagTop(void)
{
	UT_sint32 i = 0;

	if (m_utnsTagStack.viewTop (i))
		return (UT_uint32)i;
	return 0;
}

void s_XSL_FO_Listener::_closeTable(void)
{
	_closeCell();
	_closeRow();

	if(_tagTop() == TT_TABLEBODY)
		_tagClose(TT_TABLEBODY, "table-body");

	if(_tagTop() == TT_TABLE)
		_tagClose(TT_TABLE, "table");
}

void s_XSL_FO_Listener::_closeRow(void)
{
	if(_tagTop() == TT_TABLEROW)
		_tagClose(TT_TABLEROW, "table-row");
}

void s_XSL_FO_Listener::_closeCell(void)
{
	if(_tagTop() == TT_TABLECELL)
	{
		if(m_iLastClosed != TT_BLOCK)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			_tagOpenClose("block", false, true);
		}
		_tagClose(TT_TABLECELL, "table-cell");
	}
}

void s_XSL_FO_Listener::_openTable(PT_AttrPropIndex /*api*/)
{
	if (!m_bInSection)
	{
		return;
	}

	UT_UTF8String buf = "table";

	buf += _getTableThicknesses();
	buf += _getTableColors();

	_tagOpen(TT_TABLE, buf);
	_handleTableColumns();
	_tagOpen(TT_TABLEBODY, "table-body");
}

void s_XSL_FO_Listener::_openRow(void)
{
	if (mTableHelper.isNewRow())
	{
		_closeCell();
		_closeRow();

		mTableHelper.incCurRow();
		UT_sint32 row = mTableHelper.getCurRow();
		UT_sint32 curHeight = 0;

		UT_UTF8String tr = "table-row", buf;
		const char* szHeight = NULL;
		szHeight = mTableHelper.getTableProp("table-row-heights");

		UT_LocaleTransactor t(LC_NUMERIC, "C");

		while(szHeight && (*szHeight != '\0'))
		{
			if(*szHeight == '/')
			{
				szHeight++;

				if(curHeight == row)
				{
					break;
				}
				else
				{
					curHeight++;
					buf.clear();
				}
			}
			else
			{
				buf += *szHeight++;
			}
		}

		if(buf.length())
		{
			tr += " height=\"";
			tr += buf;
			tr += "\"";
		}

		_tagOpen(TT_TABLEROW, tr);
	}
}

void s_XSL_FO_Listener::_openCell(PT_AttrPropIndex /*api*/)
{
	if (!m_bInSection)
	{
		return;
	}

	_popListToDepth(0);
	_closeCell();
	_openRow();

	UT_sint32 rowspan = 1, colspan = 1;

	rowspan = mTableHelper.getBot() - mTableHelper.getTop();
	colspan = mTableHelper.getRight() - mTableHelper.getLeft();

	UT_UTF8String td ("table-cell");

	if (rowspan > 1)
		td += UT_UTF8String_sprintf(" number-rows-spanned=\"%d\"", rowspan);

	if (colspan > 1)
		td += UT_UTF8String_sprintf(" number-columns-spanned=\"%d\"", colspan);

	td += _getCellThicknesses();
	td += _getCellColors();

	_tagOpen(TT_TABLECELL, td);
}

UT_UTF8String s_XSL_FO_Listener::_getCellColors(void)
{
	UT_UTF8String tableSpec, color;
	const char *prop = NULL;

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	prop = mTableHelper.getCellProp("background-color");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("background-color");

	color = prop ? prop : "white";
	tableSpec += " background-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getCellProp("left-color");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("left-color");

	color = prop ? prop : "black";
	tableSpec += " border-left-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getCellProp("right-color");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("right-color");

	color = prop ? prop : "black";
	tableSpec += " border-right-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getCellProp("top-color");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("top-color");

	color = prop ? prop : "black";
	tableSpec += " border-top-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getCellProp("bot-color");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("bot-color");

	color = prop ? prop : "black";
	tableSpec += " border-bottom-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	return tableSpec;
}

UT_UTF8String s_XSL_FO_Listener::_getCellThicknesses(void)
{
	UT_UTF8String tableSpec;
	double val = 0;
	const char *prop = NULL;

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	tableSpec = " border=\"solid\"";
	prop = mTableHelper.getCellProp("left-thickness");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("left-thickness");

	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-left-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getCellProp("right-thickness");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("right-thickness");

	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-right-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getCellProp("top-thickness");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("top-thickness");

	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-top-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getCellProp("bot-thickness");

	if(!prop) //try to inherit table props
		prop = mTableHelper.getTableProp("bot-thickness");

	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-bottom-width=\"%1.2fpt\"", val);

	return tableSpec;
}

UT_UTF8String s_XSL_FO_Listener::_getTableColors(void)
{
	UT_UTF8String tableSpec, color;
	const char *prop = NULL;

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	prop = mTableHelper.getTableProp("background-color");
	color = prop ? prop : "white";
	tableSpec += " background-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getTableProp("left-color");
	color = prop ? prop : "black";
	tableSpec += " border-left-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getTableProp("right-color");
	color = prop ? prop : "black";
	tableSpec += " border-right-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getTableProp("top-color");
	color = prop ? prop : "black";
	tableSpec += " border-top-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	prop = mTableHelper.getTableProp("bot-color");
	color = prop ? prop : "black";
	tableSpec += " border-bottom-color=\"";

	if(prop)
		tableSpec += "#";

	tableSpec += color;
	tableSpec += "\"";

	return tableSpec;
}

void s_XSL_FO_Listener::_handleTableColumns(void)
{
	UT_sint32 nCols = mTableHelper.getNumCols();
	const char *prop = NULL;
	prop = mTableHelper.getTableProp("table-column-props");

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	//handle column widths
	for (int i = 0; i < nCols; i++)
	{
		UT_UTF8String col = "table-column", buf;

		while(prop && (*prop != '\0'))
		{
			if(*prop != '/')
			{
				buf += *prop++;
			}
			else
			{
				prop++;
				break;
			}
		}

		if(buf.length())
		{
			col += " column-width=\"";
			col += buf;
			col += "\"";
		}

		_tagOpenClose(col);
		buf.clear();
	}
}

UT_UTF8String s_XSL_FO_Listener::_getTableThicknesses(void)
{
	UT_UTF8String tableSpec;
	double val = 0;
	const char *prop = NULL;

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	tableSpec = " border=\"solid\"";
	prop = mTableHelper.getTableProp("left-thickness");
	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-left-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getTableProp("right-thickness");
	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-right-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getTableProp("top-thickness");
	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-top-width=\"%1.2fpt\"", val);

	prop = mTableHelper.getTableProp("bot-thickness");
	val = prop ? atof(prop) : 1;
	tableSpec += UT_UTF8String_sprintf(" border-bottom-width=\"%1.2fpt\"", val);

	return tableSpec;
}

/*****************************************************************/
/*****************************************************************/

s_XSL_FO_Listener::s_XSL_FO_Listener(PD_Document * pDocument,
									 IE_Exp_XSL_FO * pie)
	: m_pDocument(pDocument),
	  m_pie(pie),
	  m_bFirstWrite(true),
	  m_bInLink(false),
	  m_bInNote(false),
	  m_bInSection(false),
	  m_bInSpan(false),
	  m_bWroteListField(false),
	  m_iBlockDepth(0),
	  m_iLastClosed(0),
	  m_iListBlockDepth(0),
	  m_iListID(0),
	  mTableHelper(pDocument)
{
	m_pie->write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	_tagOpen(TT_DOCUMENT, "root xmlns:fo=\"http://www.w3.org/1999/XSL/Format\"");
	m_pie->write("\n<!-- This document was created by AbiWord -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor. -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at http://www.abisource.com -->\n\n");

	_handleLists();
}

s_XSL_FO_Listener::~s_XSL_FO_Listener()
{
	_closeSection();
	_handleDataItems();
	UT_VECTOR_FREEALL(char *, m_utvDataIDs);
	UT_VECTOR_SPARSEPURGEALL(ListHelper *, m_Lists);

	_tagClose(TT_DOCUMENT, "root");
}

void s_XSL_FO_Listener::_handleLists(void)
{
	fl_AutoNum* pAutoNum;

	for (UT_uint32 k = 0; m_pDocument->enumLists(k, &pAutoNum); ++k)
	{
		if (pAutoNum->isEmpty() == true)
			continue;

		m_Lists.addItem(new ListHelper);
		ListHelper * lh = m_Lists[m_Lists.getItemCount() - 1];
		(*lh).addList(pAutoNum);
	}
}

void s_XSL_FO_Listener::_handleBookmark(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	if(bHaveProp && pAP && pAP->getAttribute("type", szValue) && szValue)
	{
		if(!strcmp(szValue, "start") && pAP->getAttribute("name", szValue) && szValue)
		{
			UT_UTF8String buf, anchor = szValue;
			anchor.escapeXML();

			if(anchor.length())
			{
				buf = "inline id=\"";
				buf += anchor;
				buf += "\"";
				_tagOpenClose(buf, true, false);
			}
		}
	}
}

void s_XSL_FO_Listener::_handleEmbedded(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("dataid", szValue) && szValue);

	UT_UTF8String buf, img, url;

	buf = "snapshot-png-";
	buf += szValue;
	buf.escapeXML();
	const gchar* dataid = g_strdup(buf.utf8_str());
	m_utvDataIDs.push_back(dataid);

	url = UT_go_basename(m_pie->getFileName());
	url.escapeXML();

	img = "external-graphic src=\"url('";
	img += url;
	img += "_data/";
	img += buf;
	img += ".png')\"";

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("width", szValue) && szValue)
	{
		img += " content-width=\"";
		img += szValue;
		img += "\"";
	}
	if(pAP->getProperty("height", szValue) && szValue)
	{
		img += " content-height=\"";
		img += szValue;
		img += "\"";
	}
	//note: language isn't a valid attribute

	_tagOpenClose(img, true, false);
}

void s_XSL_FO_Listener::_handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api)
{
	if(!m_iBlockDepth && !m_iListBlockDepth)
		return;

	const PP_AttrProp* pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
	
	if (bHaveProp && pAP)
	{
		const gchar* szValue = NULL;
		if (pAP->getAttribute("type", szValue) && szValue)
		{
			fd_Field * field = pcro->getField();

			m_pie->populateFields ();

			if((_tagTop() == TT_LISTBLOCK) && !strcmp(static_cast<const char*>(szValue), "list_label"))
			{
				m_pie->write("\n");

				_tagOpen(TT_LISTITEM, "list-item");
				_tagOpen(TT_LISTITEMLABEL, "list-item-label end-indent=\"label-end()\"", false);
				_tagOpen(TT_BLOCK, "block", false);

				UT_UTF8String label = "";
				for(UT_sint32 i = 0; i < m_Lists.getItemCount(); i++)
				{
					ListHelper * lh = m_Lists[i];
					if(lh && ((*lh).retrieveID() == m_iListID))
					{
						label = (*lh).getNextLabel();
						break;
					}
				}

				if(label.length())
					m_pie->write(label.utf8_str()); //write out the list label text

				_tagClose(TT_BLOCK, "block", false);
				_tagClose(TT_LISTITEMLABEL, "list-item-label");
				_tagOpen(TT_LISTITEMBODY, "list-item-body start-indent=\"body-start()\"", false);
				_tagOpen(TT_BLOCK, "block", false);

				m_iBlockDepth++;
				m_bWroteListField = true;
			}
			else if(!strcmp(szValue, "footnote_ref"))
			{
				UT_UTF8String buf = field->getValue();
				buf.escapeXML();

				_tagOpen(TT_FOOTNOTE, "footnote", false);
				_tagOpen(TT_INLINE, "inline", false);

				if(buf.length())
					m_pie->write(buf.utf8_str());

				_tagClose(TT_INLINE, "inline", false);
			}
			else
			{
				UT_UTF8String buf = field->getValue();
				buf.escapeXML();

				if(buf.length())
					m_pie->write(buf.utf8_str());
			}
		}
	}
}

void s_XSL_FO_Listener::_handleFrame(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar * szValue = NULL;

	if(pAP && bHaveProp && (pAP->getAttribute("strux-image-dataid", szValue)) && szValue)
	{
		_handlePositionedImage(api);
		return;
	}

	//TODO
}

void s_XSL_FO_Listener::_handleHyperlink(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	if(bHaveProp && pAP && pAP->getAttribute("xlink:href", szValue) && szValue)
	{
		_closeLink();

		UT_UTF8String buf, url;
		buf = "basic-link text-decoration=\"underline\" color=\"blue\"";

		if( szValue[0] == '#' )
		{
			url = szValue + 1;
			url.escapeXML();

			buf += " internal-destination=\"";
			buf += url;
			buf += "\"";
		}
		else
		{
			url = szValue;
			url.escapeURL();

			buf += " external-destination=\"url('";
			buf += url;
			buf += "')\"";
		}

		_tagOpen(TT_BASICLINK, buf, false);
		m_bInLink = true;
	}
	else
	{
		_closeLink();
	}
}


void s_XSL_FO_Listener::_handleImage(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("dataid", szValue) && szValue);

	const gchar * dataid = g_strdup(szValue);
	m_utvDataIDs.push_back(dataid);

	UT_UTF8String buf, img, url;

	url = UT_go_basename(m_pie->getFileName());
	url.escapeXML();
	buf = szValue;
	buf.escapeXML();

	img = "external-graphic src=\"url('";
	img += url;
	img += "_data/";
	img += buf;

    std::string ext;
    if(m_pDocument->getDataItemFileExtension(dataid, ext, true)) {
        img += ext;
    }
    else {
        img += ".png";
    }

	img += "')\"";
	buf.clear();

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("width", szValue) && szValue)
	{
		img += " content-width=\"";
		img += szValue;
		img += "\"";
	}
	if(pAP->getProperty("height", szValue) && szValue)
	{
		img += " content-height=\"";
		img += szValue;
		img += "\"";
	}
	//note: language isn't a valid attribute

	_tagOpenClose(img, true, false);
}

void s_XSL_FO_Listener::_handlePositionedImage(PT_AttrPropIndex api)
{
	//TODO: save positioning?

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("strux-image-dataid", szValue) && szValue);

	const gchar * dataid = g_strdup(szValue);
	m_utvDataIDs.push_back(dataid);

	UT_UTF8String buf, img, url;

	url = UT_go_basename(m_pie->getFileName());
	url.escapeXML();
	buf = szValue;
	buf.escapeXML();

	img = "external-graphic src=\"url('";
	img += url;
	img += "_data/";
	img += buf;

    std::string ext;
    if(m_pDocument->getDataItemFileExtension(dataid, ext, true)) {
        img += ext;
    }
    else {
        img += ".png";
    }

	img += "')\"";
	buf.clear();

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("width", szValue) && szValue)
	{
		img += " content-width=\"";
		img += szValue;
		img += "\"";
	}
	if(pAP->getProperty("height", szValue) && szValue)
	{
		img += " content-height=\"";
		img += szValue;
		img += "\"";
	}
	//note: language isn't a valid attribute

	_tagOpenClose(img, true, false);
}

void s_XSL_FO_Listener::_handleMath(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	const gchar* szValue = NULL;

	UT_return_if_fail(bHaveProp && pAP && pAP->getAttribute("dataid", szValue) && szValue);

	//TODO: investigate mathml exporting

	UT_UTF8String buf, img, url;

	buf = "snapshot-png-";
	buf += szValue;
	buf.escapeXML();
	const gchar * dataid = g_strdup(buf.utf8_str());
	m_utvDataIDs.push_back(dataid);

	url = UT_go_basename(m_pie->getFileName());
	url.escapeXML();

	img = "external-graphic src=\"url('";
	img += url;
	img += "_data/";
	img += buf;
	img += ".png')\"";
	buf.clear();

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if(pAP->getProperty("width", szValue) && szValue)
	{
		double dInch = static_cast<double>(atoi(szValue))/UT_LAYOUT_RESOLUTION;
		UT_UTF8String_sprintf(buf,"%fin",dInch);

		img += " content-width=\"";
		img += buf;
		img += "\"";
		buf.clear();
	}
	if(pAP->getProperty("height", szValue) && szValue)
	{
		double dInch = static_cast<double>(atoi(szValue))/UT_LAYOUT_RESOLUTION;
		UT_UTF8String_sprintf(buf,"%fin",dInch);

		img += " content-height=\"";
		img += buf;
		img += "\"";
	}
	//note: language isn't a valid attribute

	_tagOpenClose(img, true, false);
}

bool s_XSL_FO_Listener::populate(fl_ContainerLayout* /*sfh*/,
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

				case PTO_Math:
				{
					_handleMath(api);
					return true;
				}

				case PTO_Embed:
				{
					_handleEmbedded(api);
					return true;
				}

				default:
				{
					UT_ASSERT(UT_TODO);
					return true;
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

bool s_XSL_FO_Listener::populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	PT_AttrPropIndex api = pcr->getIndexAP();

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		_closeSection();
		
		const PP_AttrProp* pAP = NULL;
		if (m_pDocument->getAttrProp(api, &pAP) && pAP)
		{
			const gchar* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == strcmp(pszSectionType, "doc"))
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
		_closeBlock();
		_popListToDepth(0);		

//		<fo:static-content flow-name="xsl-region-before">
//		<fo:block>
//		</fo:block>
//		</fo:static-content> 

		
		// TODO???
		return true;
	}
	
	case PTX_Block:
	{
		_closeBlock();
		_openBlock(api);

		return true;
	}

	case PTX_SectionTable:
	{
		mTableHelper.openTable(sdh,api);
		_closeBlock();
		_openTable(api);
		return true;
	}

	case PTX_SectionCell:
	{
		mTableHelper.openCell(api);
		_closeBlock();
		_popListToDepth(0);
		_openCell(api);
		return true;
	}

	case PTX_EndTable:
	{
		_closeBlock();
		_popListToDepth(0);
		_closeTable();
		mTableHelper.closeTable();
		return true;
	}

	case PTX_EndCell:
	{
		_closeBlock();
		_closeCell();
		mTableHelper.closeCell();
		return true;
	}

	case PTX_SectionFrame:
	{
		_popListToDepth(0);
		_handleFrame(api);
		return true;
	}

	case PTX_EndFrame:
	{
		UT_ASSERT(UT_TODO);
		return true;
	}

	case PTX_SectionFootnote:
	{
		if(_tagTop() != TT_FOOTNOTE) //can happen with certain .doc files
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			_tagOpen(TT_FOOTNOTE, "footnote", false);
			_tagOpenClose("inline", false, false);
		}
		_tagOpen(TT_FOOTNOTEBODY, "footnote-body", false);
		m_bInNote = true;
		return true;
	}

	case PTX_EndFootnote:
	{
		if(m_bInNote)
		{
			_closeBlock();
			if(_tagTop() == TT_FOOTNOTEBODY)
			{
				_tagClose(TT_FOOTNOTEBODY, "footnote-body", false);
				_tagClose(TT_FOOTNOTE, "footnote", false);
			}
			else
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}

			m_bInNote = false;
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}

		return true;
	}

	case PTX_SectionEndnote:
	{
		m_bInNote = true;
		return true;
	}

	case PTX_EndEndnote:
	{
		if(m_bInNote)
		{
			m_bInNote = false;
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}

		return true;
	}

	case PTX_SectionTOC:
	case PTX_EndTOC:
	{
		UT_ASSERT(UT_TODO);
		return true;
	}

	case PTX_EndMarginnote:
	case PTX_SectionMarginnote:
	default:
		UT_ASSERT(UT_TODO);
		return true;
	}
}

bool s_XSL_FO_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

bool s_XSL_FO_Listener::change(fl_ContainerLayout* /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_XSL_FO_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * /*pcr*/,
									pf_Frag_Strux* /*sdh*/,
									PL_ListenerId /* lid */,
									void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																PL_ListenerId /* lid */,
																fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

/*****************************************************************/
/*****************************************************************/

static UT_UTF8String purgeSpaces(const char* st)
{
	UT_UTF8String retval;

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

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_UTF8String master = "simple-page-master";

	_tagOpen(TT_LAYOUT_MASTER_SET, "layout-master-set");

	// query and output properties
	// todo - validate these and make sure they all make sense
	if (bHaveProp && pAP)
	{
		const gchar * szValue = NULL;

		szValue = PP_evalProperty("page-margin-top",
								  NULL, NULL, pAP, m_pDocument, true);
		if(szValue)
		{
			master += " margin-top=\"";
			master += szValue;
			master += "\"";
		}

		szValue = PP_evalProperty("page-margin-bottom",
								  NULL, NULL, pAP, m_pDocument, true);
		if(szValue)
		{
			master += " margin-bottom=\"";
			master += szValue;
			master += "\"";
		}

		szValue = PP_evalProperty("page-margin-left",
								  NULL, NULL, pAP, m_pDocument, true);
		if(szValue)
		{
			master += " margin-left=\"";
			master += szValue;
			master += "\"";
		}

		szValue = PP_evalProperty("page-margin-right",
								  NULL, NULL, pAP, m_pDocument, true);
		if(szValue)
		{
			master += " margin-right=\"";
			master += szValue;
			master += "\"";
		}
		
		UT_Dimension docUnit = m_pDocument->m_docPageSize.getDims(); 
		UT_UTF8String buf;

		// page-width, page-height
		UT_UTF8String_sprintf(buf, " page-width=\"%f%s\"", m_pDocument->m_docPageSize.Width(docUnit), UT_dimensionName(docUnit));
		master += buf;
		buf.clear();

		UT_UTF8String_sprintf(buf, " page-height=\"%f%s\"", m_pDocument->m_docPageSize.Height(docUnit), UT_dimensionName(docUnit));
		master += buf;
	}

	master += " master-name=\"first\"";

	_tagOpen(TT_SIMPLE_PAGE_MASTER, master);
	m_pie->write("\t");
	_tagOpenClose("region-body");
	_tagClose(TT_SIMPLE_PAGE_MASTER, "simple-page-master");
	_tagClose(TT_LAYOUT_MASTER_SET, "layout-master-set");
	m_pie->write("\n");

	m_bFirstWrite = false;
}

void s_XSL_FO_Listener::_handleDataItems(void)
{
	const char * szName = NULL;
    std::string mimeType;
	UT_ConstByteBufPtr pByteBuf;

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k, NULL, &szName, pByteBuf, &mimeType)); k++)
	{	  	  
		UT_sint32 loc = -1;
		for (UT_sint32 i = 0; i < m_utvDataIDs.getItemCount(); i++)
		{
			if(strcmp(reinterpret_cast<const char*>(m_utvDataIDs[i]), szName) == 0)
			{
				loc = i;
				break;
			}
		}

		if(loc > -1)
		{
			UT_UTF8String fname;

			UT_UTF8String_sprintf(fname, "%s_data", m_pie->getFileName());
			UT_go_directory_create(fname.utf8_str(), NULL);

			if (mimeType == "image/svg+xml")
				UT_UTF8String_sprintf(fname, "%s/%d.svg", fname.utf8_str(), loc);
			else if (mimeType == "application/mathml+xml")
				UT_UTF8String_sprintf(fname, "%s/%d.mathml", fname.utf8_str(), loc);
			else // raster Image
			{  
                const char * extension = "png";
                if(mimeType == "image/jpeg") {
                    extension = "jpg";
                }
				char * temp = _stripSuffix(UT_go_basename(szName), '_');
				char * fstripped = _stripSuffix(temp, '.');
				UT_UTF8String_sprintf(fname, "%s/%s.%s", fname.utf8_str(), fstripped, extension);

				FREEP(temp);
				FREEP(fstripped);
			}
	  
			GsfOutput *fp = UT_go_file_create (fname.utf8_str(), NULL);
	  
			if(!fp)
				continue;
	  
			gsf_output_write(fp, pByteBuf->getLength(), (const guint8 *)pByteBuf->getPointer(0));
			gsf_output_close(fp);
			g_object_unref(fp);
		}
	}

	return;
}

void s_XSL_FO_Listener::_openSection(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_handlePageSize(api);
	}

	_tagOpen(TT_PAGE_SEQUENCE, "page-sequence master-reference=\"first\"");
	_tagOpen(TT_SECTION, "flow flow-name=\"xsl-region-body\"");
	m_bInSection = true;
}

#define PROPERTY(x) \
	if (pAP->getProperty(x, szValue) && szValue && *szValue) \
	{ \
		UT_UTF8String esc = szValue; \
		esc.escapeXML(); \
		buf += " " x"=\""; \
		buf += esc.utf8_str(); \
		buf += "\""; \
	}

void s_XSL_FO_Listener::_openBlock(PT_AttrPropIndex api)
{
	if (!m_bInSection)
		return;

	_closeLink();

	UT_UTF8String buf;
	const PP_AttrProp* pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP), bList = false;
	const gchar* szValue = NULL;

	if(bHaveProp && pAP && pAP->getAttribute("level", szValue) && szValue)
	{
		_popListToDepth(atoi(szValue));
	}

	if(bHaveProp && pAP && pAP->getAttribute("listid", szValue) && szValue)
	{
		m_iListID = static_cast<UT_uint32>(atoi(static_cast<const char*>(szValue)));

		if(_tagTop() == TT_LISTBLOCK)
			_openListItem();

		buf = "list-block";
		m_iListBlockDepth++;
		bList = true;
	}
	else
	{
		if(_tagTop() == TT_LISTBLOCK)
			_openListItem();

		buf = "block";
		m_iBlockDepth++;
	}


	// query and output properties
	// todo - validate these and make sure they all make sense
	if (bHaveProp && pAP)
	{
		if (pAP->getProperty("bgcolor", szValue) && szValue)
		{
			buf += " background-color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				buf += '#';

			buf += szValue;
			buf += "\"";
		}

		if (pAP->getProperty("color", szValue) && szValue)
		{
			buf += " color=\"";

			if (*szValue >= '0' && *szValue <= '9')
				buf += '#';

			buf += szValue;
			buf += "\"";
		}

		if (pAP->getProperty("lang", szValue) && szValue)
		{
			buf += " language=\"";
			buf += szValue;
			buf += "\"";
		}

		if (pAP->getProperty("font-size", szValue) && szValue)
		{
			buf += " font-size=\"";
			buf += purgeSpaces(static_cast<const char *>(szValue)).utf8_str();
			buf += "\"";
		}

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

	_tagOpen(bList ? TT_LISTBLOCK : TT_BLOCK, buf, false);
}

void s_XSL_FO_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_iBlockDepth && !m_iListBlockDepth)
		return;

	_closeSpan();

	if(_tagTop() == TT_LISTBLOCK)
	{
		_openListItem();
	}

	const PP_AttrProp* pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
	UT_UTF8String buf = "inline";

	// query and output properties
	if (bHaveProp && pAP)
	{
		const gchar * szValue = NULL;

		if (pAP->getProperty("bgcolor", szValue) && szValue)
		{
			buf += " background-color=\"";

			if ((*szValue >= '0') && (*szValue <= '9'))
				buf += "#";

			buf += static_cast<const char *>(szValue);
			buf += "\"";
		}

		if (pAP->getProperty("color", szValue) && szValue)
		{
			buf += " color=\"";

			if ((*szValue >= '0') && (*szValue <= '9'))
				buf += "#";

			buf += static_cast<const char *>(szValue);
			buf += "\"";
		}

		if (pAP->getProperty("lang", szValue) && szValue)
		{
			buf += " language=\"";
			buf += static_cast<const char *>(szValue);
			buf += "\"";
		}
		
		if (pAP->getProperty("font-size", szValue) && szValue)
		{
			buf += " font-size=\"";
			buf += purgeSpaces(static_cast<const char *>(szValue)).utf8_str();
			buf += "\"";
		}		

		PROPERTY("font-family");
		PROPERTY("font-weight");
		PROPERTY("font-style");
		PROPERTY("font-stretch");
		PROPERTY("keep-together");
		PROPERTY("keep-with-next");
		PROPERTY("text-decoration");
		PROPERTY("text-transform");
	}

	_tagOpen(TT_INLINE, buf, false);
	m_bInSpan = true;
}

#undef PROPERTY

void s_XSL_FO_Listener::_openListItem(void)
{
	if(_tagTop() != TT_LISTBLOCK)
		return;

	m_pie->write("\n");
	_tagOpen(TT_LISTITEM, "list-item");
	_tagOpen(TT_LISTITEMLABEL, "list-item-label end-indent=\"label-end()\"", false);
	_tagOpenClose("block", false, false);
	_tagClose(TT_LISTITEMLABEL, "list-item-label");
	_tagOpen(TT_LISTITEMBODY, "list-item-body start-indent=\"body-start()\"", false);
	_tagOpen(TT_BLOCK, "block", false);

	m_iListBlockDepth++;
}

void s_XSL_FO_Listener::_closeBlock(void)
{
	_closeSpan();
	_closeLink();

	if(!m_iBlockDepth && !m_iListBlockDepth)
		return;

	if(m_iBlockDepth)
	{
		if(_tagTop() == TT_BLOCK)
		{
			_tagClose(TT_BLOCK, "block");
			m_iBlockDepth--;
		}
	}
	else if(m_iListBlockDepth)
	{
		if(!m_bWroteListField && (_tagTop() == TT_LISTBLOCK)) // in a list block without a list label - add some corrective markup
		{
			_openListItem();
		}

		_popListToDepth(m_iListBlockDepth - 1);
	}
}

void s_XSL_FO_Listener::_closeSection(void)
{
	if (!m_bInSection)
		return;

	_closeBlock();
	_popListToDepth(0);

	if(m_bInNote)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		if(_tagTop() == TT_FOOTNOTEBODY)
		{
			_tagClose(TT_FOOTNOTEBODY, "footnote-body", false);
			_tagClose(TT_FOOTNOTE, "footnote", false);
		}
	}

	_closeTable();

	_tagClose(TT_SECTION, "flow");
	_tagClose(TT_PAGE_SEQUENCE, "page-sequence");
	m_bInSection = false;
}

void s_XSL_FO_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	if(_tagTop() == TT_INLINE)
	{
		_tagClose(TT_INLINE, "inline", false);
		m_bInSpan = false;
	}
}

void s_XSL_FO_Listener::_closeLink(void)
{
	if(!m_bInLink)
		return;

	if(_tagTop() == TT_BASICLINK)
		_tagClose(TT_BASICLINK, "basic-link", false);
}

void s_XSL_FO_Listener::_popListToDepth(UT_sint32 depth)
{
	if (m_iListBlockDepth <= depth)
		return;

	while(m_iListBlockDepth > depth)
	{
		if(_tagTop() == TT_BLOCK)
		{
			_tagClose(TT_BLOCK, "block");
			m_iBlockDepth--;
		}
		else if(_tagTop() == TT_LISTBLOCK)
		{
			_openListItem();
		}

		if(_tagTop() == TT_LISTITEMBODY)
		{
			_tagClose(TT_LISTITEMBODY, "list-item-body");
			_tagClose(TT_LISTITEM, "list-item");
			_tagClose(TT_LISTBLOCK, "list-block");
			m_iListBlockDepth--;
			m_bWroteListField = false;
		}
		else
		{
			break; //don't loop forever
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_UTF8String sBuf;
	const UT_UCSChar * pData;

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	sBuf.reserve(length);
	for (pData=data; (pData<data+length); /**/)
	{
		switch (*pData)
		{
			case '<':
			{
				sBuf += "&lt;";
				pData++;
				break;
			}

			case '>':
			{
				sBuf += "&gt;";
				pData++;
				break;
			}

			case '&':
			{
				sBuf += "&amp;";
				pData++;
				break;
			}

			case UCS_LF:					// LF -- representing a Forced-Line-Break
			{
				UT_ASSERT(UT_TODO);
				pData++;
				break;
			}

			case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			{
				UT_ASSERT(UT_TODO);
				pData++;
				break;
			}

			case UCS_FF:					// FF -- representing a Forced-Page-Break
			{
				UT_ASSERT(UT_TODO);
				pData++;
				break;
			}

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
				break;
			}
		}
	}

	m_pie->write(sBuf.utf8_str(), sBuf.byteLength());
}
