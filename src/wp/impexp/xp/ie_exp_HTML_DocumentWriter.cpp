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
#include "ie_exp_HTML_DocumentWriter.h"

IE_Exp_HTML_DocumentWriter::IE_Exp_HTML_DocumentWriter(
    IE_Exp_HTML_OutputWriter* pOutputWriter) :
m_pOutputWriter(pOutputWriter),
m_pTagWriter(new IE_Exp_HTML_TagWriter(m_pOutputWriter)),
m_iEndnoteCount(0),
m_iEndnoteAnchorCount(0),
m_iFootnoteCount(0),
m_iAnnotationCount(0),
m_bInsertPhp(false),
m_bInsertSvgScript(false)
{

}

IE_Exp_HTML_DocumentWriter::~IE_Exp_HTML_DocumentWriter()
{
    DELETEP(m_pTagWriter);
}

void IE_Exp_HTML_DocumentWriter::openSpan(const gchar *szStyleName,
	const UT_UTF8String& style)
{
    m_pTagWriter->openTag("span", true);
    _handleStyleAndId(szStyleName, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeSpan()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openBlock(const gchar* szStyleName,
										   const UT_UTF8String& style, const PP_AttrProp * /*pAP*/)
{
    m_pTagWriter->openTag("p");
    _handleStyleAndId(szStyleName, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeBlock()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openHeading(size_t level, 
											 const gchar* szStyleName, const gchar *szId,
											 const PP_AttrProp * /*pAP*/)
{
    switch (level)
    {
    case 1:
        m_pTagWriter->openTag("h1");
        break;
    case 2:
        m_pTagWriter->openTag("h2");
        break;
    case 3:
        m_pTagWriter->openTag("h3");
        break;
    case 4:
        m_pTagWriter->openTag("h4");
        break;
    default:
        m_pTagWriter->openTag("h1");
        break;
    }
    
    
    _handleStyleAndId(szStyleName, szId, NULL);
}

void IE_Exp_HTML_DocumentWriter::closeHeading()
{   
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openSection(const gchar* szStyleName)
{
    m_pTagWriter->openTag("div");
    _handleStyleAndId(szStyleName, NULL, NULL);
}

void IE_Exp_HTML_DocumentWriter::closeSection()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openHyperlink(const gchar *szUri, 
    const gchar *szStyleName, const gchar *szId)
{
    m_pTagWriter->openTag("a", true);
    _handleStyleAndId(szStyleName, szId, NULL);

    if (szUri != NULL)
    {
        m_pTagWriter->addAttribute("href", szUri);
    }
}

void IE_Exp_HTML_DocumentWriter::closeHyperlink()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openTable(const UT_UTF8String &style,
    const UT_UTF8String &cellPadding, const UT_UTF8String &border)
{
    m_pTagWriter->openTag("table");
	m_pTagWriter->addAttribute("border", border.utf8_str());
	m_pTagWriter->addAttribute("cellpadding", cellPadding.utf8_str());
	_handleStyleAndId(NULL, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeTable()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openRow()
{
    m_pTagWriter->openTag("tr");
}

void IE_Exp_HTML_DocumentWriter::closeRow()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openCell(const UT_UTF8String &style,
    const UT_UTF8String &rowspan, const UT_UTF8String &colspan)
{
	m_pTagWriter->openTag("td");
	m_pTagWriter->addAttribute("rowspan", rowspan.utf8_str());
	m_pTagWriter->addAttribute("colspan", colspan.utf8_str());
	_handleStyleAndId(NULL, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeCell()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openBookmark(const gchar* szBookmarkName)
{
    m_pTagWriter->openTag("a");
    m_pTagWriter->addAttribute("name", szBookmarkName);
}

void IE_Exp_HTML_DocumentWriter::closeBookmark()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openList(bool ordered, const gchar *szStyleName,
										  const PP_AttrProp * /*pAP*/)
{
    if (ordered)
    {
        m_pTagWriter->openTag("ol");
    }
    else
    {
        m_pTagWriter->openTag("ul");
    }
    _handleStyleAndId(szStyleName, NULL, NULL);
}

void IE_Exp_HTML_DocumentWriter::closeList()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openListItem()
{
    m_pTagWriter->openTag("li");
}

void IE_Exp_HTML_DocumentWriter::closeListItem()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openField(const UT_UTF8String& fieldType, 
    const UT_UTF8String& value)
{
    if (fieldType == "endnote_ref")
    {
        m_pTagWriter->openTag("a", true);
        m_pTagWriter->addAttribute("href",UT_UTF8String_sprintf("#endnote-%d", 
            m_iEndnoteCount + 1).utf8_str());
        m_pTagWriter->writeData(
            UT_UTF8String_sprintf("%d", m_iEndnoteCount + 1).utf8_str());
        m_iEndnoteCount++;
    }  else
    if (fieldType == "footnote_ref")
    {
        m_pTagWriter->openTag("a", true);
        m_pTagWriter->addAttribute("href",UT_UTF8String_sprintf("#footnote-%d", 
            m_iEndnoteCount + 1).utf8_str());
        m_pTagWriter->writeData(UT_UTF8String_sprintf("%d", 
            m_iFootnoteCount + 1).utf8_str());
        m_iFootnoteCount++;
    }
    else
    {
        m_pTagWriter->openTag("span", true);
        m_pTagWriter->writeData(value.utf8_str());
    }
}

void IE_Exp_HTML_DocumentWriter::closeField(const UT_UTF8String&)
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openAnnotation()
{
    m_pTagWriter->openTag("a", true);
    m_pTagWriter->addAttribute("href",UT_UTF8String_sprintf("#annotation-%d", 
            m_iAnnotationCount + 1).utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeAnnotation()
{
    m_pTagWriter->closeTag();
}


void IE_Exp_HTML_DocumentWriter::openDocument()
{
    m_pTagWriter->openTag("html");   
}

void IE_Exp_HTML_DocumentWriter::closeDocument()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openHead()
{
    m_pTagWriter->openTag("head");
}

void IE_Exp_HTML_DocumentWriter::closeHead()
{
	if (m_bInsertPhp)
	{
		UT_UTF8String phpFragment = "<?php";
		phpFragment += "  include($_SERVER['DOCUMENT_ROOT'].'/x-header.php');" MYEOL " ";
		phpFragment += "?>";
		m_pTagWriter->writeData(phpFragment.utf8_str());
	}	
	
    if (m_bInsertSvgScript)
    {
        m_pTagWriter->openTag("script", false, false);
        m_pTagWriter->addAttribute("type", "text/javascript");
        m_pTagWriter->openComment();
        m_pTagWriter->writeData(sMathSVGScript.utf8_str());
        m_pTagWriter->closeComment();
        m_pTagWriter->closeTag();
    }
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openBody()
{
    m_pTagWriter->openTag("body", true);
	if (m_bInsertPhp) {
		UT_UTF8String phpFragment = "<?php";
		phpFragment += MYEOL "  include($_SERVER['DOCUMENT_ROOT'].'/x-page-begin.php');" MYEOL " ";
		phpFragment += "?>";
		m_pTagWriter->writeData(phpFragment.utf8_str());
	}	
	
}

void IE_Exp_HTML_DocumentWriter::closeBody()
{
	if (m_bInsertPhp) {
		UT_UTF8String phpFragment = "<?php";
		phpFragment += MYEOL "  include($_SERVER['DOCUMENT_ROOT'].'/x-page-end.php');" MYEOL " ";
		phpFragment += "?>";
		m_pTagWriter->writeData(phpFragment.utf8_str());
	}	
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openTextbox(const UT_UTF8String& style)
{
    m_pTagWriter->openTag("div", false, false);
    _handleStyleAndId(NULL, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeTextbox()
{
    m_pTagWriter->closeTag();
}
void IE_Exp_HTML_DocumentWriter::_handleStyleAndId(const gchar* szStyleName, 
    const gchar* szId, const gchar* szStyle)
{
    if ((szStyleName != NULL)  && (szStyle != NULL) && (strlen(szStyle) > 0))
    {
        m_pTagWriter->addAttribute("class", szStyleName);
    }

    if ((szId != NULL) && (strlen(szId) > 0))
    {
        m_pTagWriter->addAttribute("id", szId);
    }
	
	if ((szStyle != NULL) && (strlen(szStyle) > 0))
	{
		m_pTagWriter->addAttribute("style", szStyle);
	}
	
}

void IE_Exp_HTML_DocumentWriter::insertText(const UT_UTF8String &text)
{
    m_pTagWriter->writeData(text.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::insertImage(const UT_UTF8String& url, 
	const UT_UTF8String& align, const UT_UTF8String& style,
    const UT_UTF8String &title, const UT_UTF8String &alt)
{
    m_pTagWriter->openTag("img", true, true);
	_handleStyleAndId(NULL, NULL, style.utf8_str());
    m_pTagWriter->addAttribute("src", url.utf8_str());
    m_pTagWriter->addAttribute("title", title.utf8_str());
    m_pTagWriter->addAttribute("alt", alt.utf8_str());
	m_pTagWriter->addAttribute("align", align.utf8_str());
    m_pTagWriter->closeTag();

}

void IE_Exp_HTML_DocumentWriter::insertTOC(const gchar* /*title*/,
    const std::vector<UT_UTF8String>& items, 
    const std::vector<UT_UTF8String>& itemUriList)
{
    m_pTagWriter->openTag("ul");
    m_pTagWriter->addAttribute("class", "table-of-contents");
    
    for (size_t i = 0; i < items.size(); i++)
    {
        m_pTagWriter->openTag("li");
        m_pTagWriter->openTag("a");
        m_pTagWriter->addAttribute("class", "toc-item");
        m_pTagWriter->addAttribute("href", itemUriList.at(i).utf8_str());
        m_pTagWriter->writeData(items.at(i).utf8_str());
        m_pTagWriter->closeTag();
        m_pTagWriter->closeTag();
    }
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertFootnotes(
    const std::vector<UT_UTF8String>& footnotes)
{
	if (footnotes.size() == 0) return;
    m_pTagWriter->openTag("ol");
    for (size_t i = 0; i < footnotes.size(); i++)
    {
        m_pTagWriter->openTag("li");
        m_pTagWriter->addAttribute("class", "footnote_anchor");
        m_pTagWriter->addAttribute("id", 
            UT_UTF8String_sprintf("footnote-%d", i + 1).utf8_str());
        m_pTagWriter->writeData(footnotes.at(i).utf8_str());
        m_pTagWriter->closeTag();
    }
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertEndnotes(
const std::vector<UT_UTF8String>& endnotes)
{
	if (endnotes.size() == 0) return;
    m_pTagWriter->openTag("ol");
    for (size_t i = 0; i < endnotes.size(); i++)
    {
        m_pTagWriter->openTag("li");
        m_pTagWriter->addAttribute("class", "endnote_anchor");
        m_pTagWriter->addAttribute("id", UT_UTF8String_sprintf("endnote-%d", 
            m_iEndnoteAnchorCount + 1).utf8_str());
        m_pTagWriter->writeData(endnotes.at(i).utf8_str());
        m_pTagWriter->closeTag();
        m_iEndnoteAnchorCount++;
    }
    
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertDTD()
{
}

void IE_Exp_HTML_DocumentWriter::insertMeta(const std::string& name, 
                                            const std::string& content,
                                            const std::string& httpEquiv)
{
    m_pTagWriter->openTag("meta", false, true);
    if (name.size() > 0)
        m_pTagWriter->addAttribute("name", name);
    if (httpEquiv.size() > 0)
        m_pTagWriter->addAttribute("http-equiv", httpEquiv);
    m_pTagWriter->addAttribute("content", content);
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertAnnotations(
    const std::vector<UT_UTF8String>& titles, 
    const std::vector<UT_UTF8String>& authors, 
    const std::vector<UT_UTF8String>& annotations)
{
    m_pTagWriter->openTag("div");
    m_pTagWriter->addAttribute("class", "annotation-section");
    
    for(size_t i = 0; i < annotations.size(); i++)
    {
        UT_UTF8String title = titles.at(i);
        UT_UTF8String author = authors.at(i);
        UT_UTF8String annotation = annotations.at(i);
        
        m_pTagWriter->openTag("p");
        m_pTagWriter->addAttribute("class", "annotation");
        m_pTagWriter->addAttribute("id", UT_UTF8String_sprintf("annotation-%d", 
            i + 1).utf8_str());
        if (title.length())
        {
            m_pTagWriter->openTag("span");
            m_pTagWriter->addAttribute("class", "annotation-title");
            m_pTagWriter->writeData(title.utf8_str());
            m_pTagWriter->closeTag();
            m_pTagWriter->openTag("br", false, true);
            m_pTagWriter->closeTag();
        }
        
        if (author.length())
        {
            m_pTagWriter->openTag("span");
            m_pTagWriter->addAttribute("class", "annotation-author");
            m_pTagWriter->writeData(author.utf8_str());
            m_pTagWriter->closeTag();
            m_pTagWriter->openTag("br", false, true);
            m_pTagWriter->closeTag();
        }
        
        if (annotation.length())
        {
            m_pTagWriter->openTag("blockquote");
            m_pTagWriter->addAttribute("class", "annotation-content");
            m_pTagWriter->writeData(annotation.utf8_str());
            m_pTagWriter->closeTag();
        }
        
        m_pTagWriter->closeTag();        
    }
    
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertStyle(const UT_UTF8String &style)
{
    m_pTagWriter->openTag("style", false, false);
    m_pTagWriter->addAttribute("type", "text/css");
    m_pTagWriter->openComment();
    m_pTagWriter->writeData(style.utf8_str());
    m_pTagWriter->closeComment();
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertTitle(const std::string& title)
{
	m_pTagWriter->openTag("title", false, false);
	m_pTagWriter->writeData(title);
	m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertLink(const UT_UTF8String &rel,
            const UT_UTF8String &type, const UT_UTF8String &uri)
{
    m_pTagWriter->openTag("link", false, true);
    m_pTagWriter->addAttribute("rel", rel.utf8_str());
    m_pTagWriter->addAttribute("type", type.utf8_str());
    m_pTagWriter->addAttribute("href", uri.utf8_str());
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::insertMath(const UT_UTF8String& mathml, 
											const UT_UTF8String& /*width*/, const UT_UTF8String& /*height*/)
{
    m_pTagWriter->writeData(mathml.utf8_str());    
}
/*
 * 
 */

IE_Exp_HTML_XHTMLWriter::IE_Exp_HTML_XHTMLWriter(
	IE_Exp_HTML_OutputWriter* pOutputWriter) :
		IE_Exp_HTML_DocumentWriter(pOutputWriter),
    m_bEnableXmlDeclaration(false),
    m_bUseAwml(false)
{
	m_pTagWriter->enableXmlMode();
}

void IE_Exp_HTML_XHTMLWriter::insertDTD()
{
    if (m_bEnableXmlDeclaration)
    {
        m_pOutputWriter->write(XML_DECLARATION);
    }
    
    if (m_bUseAwml)
    {
        m_pOutputWriter->write(XHTML_AWML_DTD);
    } else
    {
        m_pOutputWriter->write(XHTML_DTD);
    }
}

void IE_Exp_HTML_XHTMLWriter::openDocument()
{
	m_pTagWriter->openTag("html");
	m_pTagWriter->addAttribute("xmlns", XHTML_NS);
    
    if (m_bUseAwml)
    {
       m_pTagWriter->addAttribute("xmlns:awml", AWML_NS); 
    }
}

void IE_Exp_HTML_XHTMLWriter::openList(bool ordered, 
    const gchar *szStyleName, const PP_AttrProp *pAP)
{
    IE_Exp_HTML_DocumentWriter::openList(ordered, szStyleName, pAP);
    _handleAwmlStyle(pAP);
}

void IE_Exp_HTML_XHTMLWriter::openHeading(size_t level, 
    const gchar* szStyleName, const gchar *szId,
    const PP_AttrProp *pAP)
{
    IE_Exp_HTML_DocumentWriter::openHeading(level, szStyleName, szId, pAP);
    _handleAwmlStyle(pAP);
}

void IE_Exp_HTML_XHTMLWriter::openBlock(const gchar* szStyleName, 
    const UT_UTF8String& style, const PP_AttrProp *pAP)
{
    IE_Exp_HTML_DocumentWriter::openBlock(szStyleName,      style, 
        pAP);
    _handleAwmlStyle(pAP);
}

void IE_Exp_HTML_XHTMLWriter::openHead()
{
    IE_Exp_HTML_DocumentWriter::openHead();
    insertMeta(std::string(),  "application/xhtml+xml; charset=UTF-8", 
               "Content-Type");
}


void IE_Exp_HTML_XHTMLWriter::_handleAwmlStyle(const PP_AttrProp *pAP)
{
    if (!m_bUseAwml || (pAP == NULL))
    {
        return;
    }

    const gchar *szStyleName = NULL;
    pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szStyleName);
    
    if (szStyleName != NULL)
    {
        m_pTagWriter->addAttribute("awml:style", szStyleName);
    }

}
    
/*
 * 
 */

IE_Exp_HTML_HTML4Writer::IE_Exp_HTML_HTML4Writer(
IE_Exp_HTML_OutputWriter* pOutputWriter):
		IE_Exp_HTML_DocumentWriter(pOutputWriter)
{
	m_pTagWriter->enableXmlMode(false);
}

void IE_Exp_HTML_HTML4Writer::insertDTD()
{
	m_pOutputWriter->write(HTML4_DTD);
}

void IE_Exp_HTML_HTML4Writer::openHead()
{
    IE_Exp_HTML_DocumentWriter::openHead();
    insertMeta(std::string(), "text/html; charset=UTF-8", "Content-Type");
}

IE_Exp_HTML_DefaultWriterFactory::IE_Exp_HTML_DefaultWriterFactory(
PD_Document *pDocument,
XAP_Exp_HTMLOptions& exp_opt):
		m_exp_opt(exp_opt),
    m_pDocument(pDocument)
{
	
}

IE_Exp_HTML_DocumentWriter *IE_Exp_HTML_DefaultWriterFactory::constructDocumentWriter(
    IE_Exp_HTML_OutputWriter *pOutputWriter)
{
	IE_Exp_HTML_DocumentWriter *pWriter = NULL;
	if (m_exp_opt.bIs4) {
		pWriter = new IE_Exp_HTML_HTML4Writer(pOutputWriter);
	}
	else {
		IE_Exp_HTML_XHTMLWriter *pXhtmlWriter = 
            new IE_Exp_HTML_XHTMLWriter(pOutputWriter);
        pXhtmlWriter->enableAwmlNamespace(m_exp_opt.bAllowAWML);
        pXhtmlWriter->enableXmlDeclaration(m_exp_opt.bDeclareXML);
        pWriter = pXhtmlWriter;
        
	}

	pWriter->enablePHP(m_exp_opt.bIsAbiWebDoc);
    pWriter->enableSVGScript((!m_exp_opt.bMathMLRenderPNG) 
        && m_pDocument->hasMath());
	return pWriter;
}
