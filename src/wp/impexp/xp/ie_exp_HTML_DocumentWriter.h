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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef IE_EXP_HTML_MAINLISTENER_H
#define IE_EXP_HTML_MAINLISTENER_H

// HTML exporter includes
#include "ie_exp_HTML.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_StyleTree.h"
#include "ie_exp_HTML_Listener.h"
#include "xap_Dlg_HTMLOptions.h"

class IE_Exp_HTML_DocumentWriter : public IE_Exp_HTML_ListenerImpl
{
public:
    IE_Exp_HTML_DocumentWriter(IE_Exp_HTML_OutputWriter* pOutputWriter);
    
    ~IE_Exp_HTML_DocumentWriter();

    void openSpan(const gchar *szStyleNames);
    void closeSpan();
    
    void openBlock(const gchar* szStyleName);
    void closeBlock();
    
    void openHeading(size_t level, const gchar* szStyleName, const gchar *szId);
    void closeHeading();
    
    void openSection(const gchar* szStyleName);
    void closeSection();
    
    void openHyperlink(const gchar *szUri, const gchar *szStyleName, 
        const gchar *szId);
    void closeHyperlink();
    
    void openDocument();
    void closeDocument();
    
    void openHead();
    void closeHead();
    
    void openBody();
    void closeBody();
        
    void openTable(const PP_AttrProp* properties);
    void closeTable();
    
    void openRow();
    void closeRow();
    
    void openCell(const PP_AttrProp* properties);
    void closeCell();
    
    void openBookmark(const gchar* szBookmarkName);
    void closeBookmark();
    
    void openList(bool ordered, const gchar *szStyleName);
    void closeList();
    
    void openListItem();
    void closeListItem();
    
    void openField(const UT_UTF8String &fieldType, const UT_UTF8String &value);
    void closeField();
    
    void openAnnotation();
    void closeAnnotation();
    
    void insertDTD();
    void insertMeta(const UT_UTF8String &name, const UT_UTF8String &content);
    void insertText(const UT_UTF8String &text);
    void insertImage(const UT_UTF8String &url, const UT_UTF8String &width, 
        const UT_UTF8String &height, const UT_UTF8String &top, 
        const UT_UTF8String &left,
        const UT_UTF8String &title,
        const UT_UTF8String &alt);
    void insertTOC(const gchar *title, const std::vector<UT_UTF8String> &items,
        const std::vector<UT_UTF8String> &itemUriList);
    void insertEndnotes(const std::vector<UT_UTF8String> &endnotes);
    void insertFootnotes(const std::vector<UT_UTF8String> &footnotes);
    void insertAnnotations(const std::vector<UT_UTF8String> &titles,
       const std::vector<UT_UTF8String> &authors,
       const std::vector<UT_UTF8String> &annotations);
    void insertStyle(const UT_UTF8String &style);
private:
    
    void inline _handleStyleAndId(const gchar *szStyleName, const gchar *szId);
    
    IE_Exp_HTML_OutputWriter *m_pOutputWriter;
    IE_Exp_HTML_TagWriter *m_pTagWriter;
    UT_uint32 m_iEndnoteCount;
    UT_uint32 m_iEndnoteAnchorCount;
    UT_uint32 m_iFootnoteCount;
    UT_uint32 m_iAnnotationCount;
};
#endif