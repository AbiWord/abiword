/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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
#ifndef IE_EXP_HTML_MAINLISTENER_H
#define IE_EXP_HTML_MAINLISTENER_H

// HTML exporter includes
#include "ie_exp_HTML.h"
#include "ie_exp_HTML_Listener.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_StyleTree.h"
#include "xap_Dlg_HTMLOptions.h"

#include "pp_AttrProp.h"

#define XHTML_DTD "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \
\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
#define XHTML_NS "http://www.w3.org/1999/xhtml"

#define XHTML_AWML_DTD "<!DOCTYPE html PUBLIC \"-//ABISOURCE//DTD XHTML plus \
AWML 2.2//EN\" \"http://www.abisource.com/2004/xhtml-awml/xhtml-awml.mod\">"
#define AWML_NS "http://www.abisource.com/2004/xhtml-awml/"

#define HTML4_DTD "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \
\"http://www.w3.org/TR/html4/strict.dtd\">\n"



#define XML_DECLARATION "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"

class ABI_EXPORT IE_Exp_HTML_DocumentWriter : public IE_Exp_HTML_ListenerImpl
{
public:
    IE_Exp_HTML_DocumentWriter(IE_Exp_HTML_OutputWriter* pOutputWriter);

    virtual ~IE_Exp_HTML_DocumentWriter();

    void openSpan(const gchar *szStyleNames, const UT_UTF8String& style);
    void closeSpan();

    void openBlock(const gchar* szStyleName, const UT_UTF8String& style,
        const PP_AttrProp *pAP);
    void closeBlock();

    void openHeading(size_t level, const gchar* szStyleName,
        const gchar *szId, const PP_AttrProp *pAP);
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

    void openTable(const UT_UTF8String &style,
        const UT_UTF8String &cellPadding, const UT_UTF8String &border);
    void closeTable();

    void openRow();
    void closeRow();

    void openCell(const UT_UTF8String &style,
        const UT_UTF8String &rowspan, const UT_UTF8String &colspan);
    void closeCell();

    void openBookmark(const gchar* szBookmarkName);
    void closeBookmark();

    void openList(bool ordered, const gchar *szStyleName,
        const PP_AttrProp *pAP);
    void closeList();

    void openListItem();
    void closeListItem();

    void openField(const UT_UTF8String &fieldType, const UT_UTF8String &value);
    void closeField(const UT_UTF8String& fieldType);

    void openAnnotation();
    void closeAnnotation();

    void openTextbox(const UT_UTF8String &style);
    void closeTextbox();

    void insertDTD();
    void insertMeta(const std::string& name, const std::string& content,
            const std::string& httpEquiv);
    void insertText(const UT_UTF8String &text);
    void insertImage(const UT_UTF8String &url, const UT_UTF8String &width,
        const UT_UTF8String &align, const UT_UTF8String &style,
        const UT_UTF8String &alt);
    void insertTOC(const gchar *title, const std::vector<UT_UTF8String> &items,
        const std::vector<UT_UTF8String> &itemUriList);
    void insertEndnotes(const std::vector<UT_UTF8String> &endnotes);
    void insertFootnotes(const std::vector<UT_UTF8String> &footnotes);
    void insertAnnotations(const std::vector<UT_UTF8String> &titles,
       const std::vector<UT_UTF8String> &authors,
       const std::vector<UT_UTF8String> &annotations);
    void insertStyle(const UT_UTF8String &style);
    virtual void insertJavaScript(const gchar * /*src*/,
								  const gchar* /*script*/) {}
    void insertTitle(const std::string& title);
    void insertLink(const UT_UTF8String &rel,
            const UT_UTF8String &type, const UT_UTF8String &uri);
    void insertMath(const UT_UTF8String &mathml,
    const UT_UTF8String &width, const UT_UTF8String &height);
    inline void enablePHP (bool bEnable = true) { m_bInsertPhp = bEnable; }
    inline void enableSVGScript (bool bEnable = true) { m_bInsertSvgScript = bEnable; }
protected:
    IE_Exp_HTML_DocumentWriter(){}
    void inline _handleStyleAndId(const gchar *szStyleName, const gchar *szId,
            const gchar *szStyle);

    IE_Exp_HTML_OutputWriter *m_pOutputWriter;
    IE_Exp_HTML_TagWriter *m_pTagWriter;
    UT_uint32 m_iEndnoteCount;
    UT_uint32 m_iEndnoteAnchorCount;
    UT_uint32 m_iFootnoteCount;
    UT_uint32 m_iAnnotationCount;

    bool m_bInsertPhp;
    bool m_bInsertSvgScript;
};

/*
 * Writer class for XHTML document creation
 */
class ABI_EXPORT IE_Exp_HTML_XHTMLWriter : public IE_Exp_HTML_DocumentWriter
{
public:
    IE_Exp_HTML_XHTMLWriter(IE_Exp_HTML_OutputWriter* pOutputWriter);
    void insertDTD();
    void openDocument();
    void openHead();
    void openList(bool ordered, const gchar *szStyleName, const PP_AttrProp *pAP);
    void openHeading(size_t level, const gchar* szStyleName, const gchar *szId,
        const PP_AttrProp *pAP);
    void openBlock(const gchar* szStyleName, const UT_UTF8String& style,
        const PP_AttrProp *pAP);
    inline void enableXmlDeclaration(bool bEnable) { m_bEnableXmlDeclaration = bEnable; }
    inline void enableAwmlNamespace(bool bEnable) { m_bUseAwml = bEnable; }
private:
    void _handleAwmlStyle(const PP_AttrProp *pAP);
    bool m_bEnableXmlDeclaration;
    bool m_bUseAwml;

};

class ABI_EXPORT IE_Exp_HTML_HTML4Writer : public IE_Exp_HTML_DocumentWriter
{
public:
    IE_Exp_HTML_HTML4Writer(IE_Exp_HTML_OutputWriter* pOutputWriter);
    void insertDTD();
    void openHead();
};

/*
 * This factory class gives ability to customize HTML exporter using
 * different content generators
 */
class ABI_EXPORT IE_Exp_HTML_WriterFactory
{
public:
    virtual ~IE_Exp_HTML_WriterFactory() {}
    virtual IE_Exp_HTML_DocumentWriter *constructDocumentWriter(
        IE_Exp_HTML_OutputWriter *pOutputWriter) = 0;

};


class ABI_EXPORT IE_Exp_HTML_DefaultWriterFactory : public IE_Exp_HTML_WriterFactory
{
public:
    virtual ~IE_Exp_HTML_DefaultWriterFactory() {}
    IE_Exp_HTML_DefaultWriterFactory(PD_Document *pDocument,
            XAP_Exp_HTMLOptions &exp_opt);
    IE_Exp_HTML_DocumentWriter *constructDocumentWriter(
    IE_Exp_HTML_OutputWriter *pOutputWriter);
private:
    XAP_Exp_HTMLOptions &m_exp_opt;
    PD_Document *m_pDocument;
};
#endif
