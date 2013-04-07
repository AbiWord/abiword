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
#include "ie_exp_EPUB_EPUB3Writer.h"

IE_Exp_EPUB_EPUB3Writer::IE_Exp_EPUB_EPUB3Writer(IE_Exp_HTML_OutputWriter* 
	pOutputWriter):
IE_Exp_HTML_DocumentWriter(pOutputWriter)
{
    m_pTagWriter->enableXmlMode(true);
}

void IE_Exp_EPUB_EPUB3Writer::openDocument()
{
    m_pTagWriter->openTag("html", false, false);
    m_pTagWriter->addAttribute("xmlns", "http://www.w3.org/1999/xhtml");
    m_pTagWriter->addAttribute("profile", EPUB3_CONTENT_PROFILE);
}


void IE_Exp_EPUB_EPUB3Writer::openAnnotation()
{
	m_pTagWriter->openTag("a", true);
	m_pTagWriter->addAttribute("href", 
		UT_UTF8String_sprintf("#annotation-%d",
			m_iAnnotationCount + 1).utf8_str());
	
	m_pTagWriter->addAttribute("epub:type", "annoref");
}

void IE_Exp_EPUB_EPUB3Writer::closeAnnotation()
{
	m_pTagWriter->closeTag();
}


void IE_Exp_EPUB_EPUB3Writer::insertDTD()
{
    m_pOutputWriter->write("<?xml version=\"1.0\"?>\n");	
}

void IE_Exp_EPUB_EPUB3Writer::insertTOC(const gchar * /*title*/,
					const std::vector<UT_UTF8String> & /*items*/,
					const std::vector<UT_UTF8String> & /*itemUriList*/)
{

}

void IE_Exp_EPUB_EPUB3Writer::insertEndnotes(
	const std::vector<UT_UTF8String> &endnotes)
{
	if (endnotes.size() == 0) return;
	
	m_pTagWriter->openTag("aside");
	m_pTagWriter->addAttribute("epub:type", "rearnotes");
		
    for (size_t i = 0; i < endnotes.size(); i++)
    {
        m_pTagWriter->openTag("section");
        // m_pTagWriter->addAttribute("class", "endnote_anchor");
        m_pTagWriter->addAttribute("id", UT_UTF8String_sprintf("endnote-%d", 
            m_iEndnoteAnchorCount + 1).utf8_str());
		m_pTagWriter->addAttribute("epub:type", "rearnote");
        m_pTagWriter->writeData(endnotes.at(i).utf8_str());
        m_pTagWriter->closeTag();
        m_iEndnoteAnchorCount++;
    }
	
	m_pTagWriter->closeTag();
}

void IE_Exp_EPUB_EPUB3Writer::insertFootnotes(
	const std::vector<UT_UTF8String> &footnotes)
{
	if (footnotes.size() == 0) return;
	
	m_pTagWriter->openTag("aside");
	m_pTagWriter->addAttribute("epub:type", "footnotes");
	for (size_t i = 0; i < footnotes.size(); i++) {
		m_pTagWriter->openTag("section");
		// m_pTagWriter->addAttribute("class", "footnote_anchor");
		m_pTagWriter->addAttribute("id",
								   UT_UTF8String_sprintf("footnote-%d", i + 1).utf8_str());
		m_pTagWriter->addAttribute("epub:type", "footnote");
		m_pTagWriter->writeData(footnotes.at(i).utf8_str());
		m_pTagWriter->closeTag();
	}
	m_pTagWriter->closeTag();	
}

void IE_Exp_EPUB_EPUB3Writer::insertAnnotations(
	const std::vector<UT_UTF8String> &titles,
	const std::vector<UT_UTF8String> &authors,
	const std::vector<UT_UTF8String> &annotations)
{
	m_pTagWriter->openTag("section");
    m_pTagWriter->addAttribute("epub:type", "annotations");
    
    for(size_t i = 0; i < annotations.size(); i++)
    {
        UT_UTF8String title = titles.at(i);
        UT_UTF8String author = authors.at(i);
        UT_UTF8String annotation = annotations.at(i);
        
        m_pTagWriter->openTag("section");
        // m_pTagWriter->addAttribute("class", "annotation");
		m_pTagWriter->addAttribute("epub:type", "annotation");
        m_pTagWriter->addAttribute("id", UT_UTF8String_sprintf("annotation-%d", 
            i + 1).utf8_str());
        if (title.length())
        {
            m_pTagWriter->openTag("h4");
            m_pTagWriter->writeData(title.utf8_str());
            m_pTagWriter->closeTag();
        }
        
        /*if (author.length())
        {
            m_pTagWriter->openTag("span");
            m_pTagWriter->addAttribute("class", "annotation-author");
            m_pTagWriter->writeData(author.utf8_str());
            m_pTagWriter->closeTag();
            m_pTagWriter->openTag("br", false, true);
            m_pTagWriter->closeTag();
        }*/
        
        if (annotation.length())
        {
            m_pTagWriter->openTag("blockquote");
            // m_pTagWriter->addAttribute("class", "annotation-content");
            m_pTagWriter->writeData(annotation.utf8_str());
            m_pTagWriter->closeTag();
        }
        
        m_pTagWriter->closeTag();        
    }
    
    m_pTagWriter->closeTag();
}

IE_Exp_HTML_DocumentWriter *IE_Exp_EPUB_EPUB3WriterFactory::
constructDocumentWriter(IE_Exp_HTML_OutputWriter* pOutputWriter)
{
	return new IE_Exp_EPUB_EPUB3Writer(pOutputWriter);
}
