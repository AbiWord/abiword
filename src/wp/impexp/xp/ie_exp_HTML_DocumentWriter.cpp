#include "ie_exp_HTML_DocumentWriter.h"

IE_Exp_HTML_DocumentWriter::IE_Exp_HTML_DocumentWriter(
    IE_Exp_HTML_OutputWriter* pOutputWriter) :
m_pOutputWriter(pOutputWriter),
m_pTagWriter(new IE_Exp_HTML_TagWriter(m_pOutputWriter)),
m_iEndnoteCount(0),
m_iEndnoteAnchorCount(0),
m_iFootnoteCount(0),
m_iAnnotationCount(0),
m_bInsertPhp(false)
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
	const UT_UTF8String& style)
{
    m_pTagWriter->openTag("p");
    _handleStyleAndId(szStyleName, NULL, style.utf8_str());
}

void IE_Exp_HTML_DocumentWriter::closeBlock()
{
    m_pTagWriter->closeTag();
}

void IE_Exp_HTML_DocumentWriter::openHeading(size_t level, 
    const gchar* szStyleName, const gchar *szId)
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

void IE_Exp_HTML_DocumentWriter::openList(bool ordered, const gchar *szStyleName)
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

void IE_Exp_HTML_DocumentWriter::closeField()
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
void IE_Exp_HTML_DocumentWriter::_handleStyleAndId(const gchar* szStyleName, 
    const gchar* szId, const gchar* szStyle)
{
    if (szStyleName != NULL)
    {
        m_pTagWriter->addAttribute("class", szStyleName);
    }

    if (szId != NULL)
    {
        m_pTagWriter->addAttribute("id", szId);
    }
	
	if (szStyle != NULL)
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

void IE_Exp_HTML_DocumentWriter::insertTOC(const gchar* title, 
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

void IE_Exp_HTML_DocumentWriter::insertMeta(const UT_UTF8String& name, 
                                            const UT_UTF8String& content)
{
    m_pTagWriter->openTag("meta", false, true);
    m_pTagWriter->addAttribute("name",name.utf8_str());
    m_pTagWriter->addAttribute("content",content.utf8_str());
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

void IE_Exp_HTML_DocumentWriter::insertTitle(const UT_UTF8String& title)
{
	m_pTagWriter->openTag("title", false, false);
	m_pTagWriter->writeData(title.utf8_str());
	m_pTagWriter->closeTag();
}
/*
 * 
 */

IE_Exp_HTML_XHTMLWriter::IE_Exp_HTML_XHTMLWriter(
	IE_Exp_HTML_OutputWriter* pOutputWriter) :
		IE_Exp_HTML_DocumentWriter(pOutputWriter)
{
	m_pTagWriter->enableXmlMode();
}

void IE_Exp_HTML_XHTMLWriter::insertDTD()
{
	m_pOutputWriter->write(XHTML_DTD);
}

void IE_Exp_HTML_XHTMLWriter::openDocument()
{
	m_pTagWriter->openTag("html");
	m_pTagWriter->addAttribute("xmlns", XHTML_NS);
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