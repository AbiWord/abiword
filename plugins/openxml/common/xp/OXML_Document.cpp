/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

// Class definition include
#include <OXML_Document.h>

// Internal includes
#include <OXML_Section.h>
#include <OXML_Style.h>
#include <OXML_Types.h>
#include <OXML_Theme.h>
#include <OXML_FontManager.h>

// AbiWord includes
#include <fp_PageSize.h>
#include <ut_types.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_Document* OXML_Document::s_docInst = NULL;

OXML_Document* OXML_Document::getNewInstance()
{
	OXML_Document::destroyInstance();
	return OXML_Document::getInstance();
}

OXML_Document* OXML_Document::getInstance()
{
	if (s_docInst == NULL)
		s_docInst = new OXML_Document();
	return s_docInst;
}

void OXML_Document::destroyInstance()
{
	DELETEP(s_docInst);
}

OXML_SharedSection OXML_Document::getCurrentSection()
{
	UT_return_val_if_fail(s_docInst != NULL, OXML_SharedSection() );
	return s_docInst->getLastSection();
}

OXML_Document::OXML_Document() : 
	OXML_ObjectWithAttrProp(), 
	m_theme(), 
	m_fontManager()
{
	clearStyles();
	clearHeaders();
	clearFooters();
	clearSections();
	clearFootnotes();
	clearEndnotes();
}

OXML_Document::~OXML_Document()
{
	clearStyles();
	clearHeaders();
	clearFooters();
	clearSections();
	clearFootnotes();
	clearEndnotes();
}

OXML_SharedStyle OXML_Document::getStyleById(const std::string & id) const
{
	OXML_StyleMap::const_iterator it;
	it = m_styles_by_id.find(id);
	return it != m_styles_by_id.end() ? it->second : OXML_SharedStyle() ;
}

OXML_SharedStyle OXML_Document::getStyleByName(const std::string & name) const
{
	OXML_StyleMap::const_iterator it;
	it = m_styles_by_name.find(name);
	return it != m_styles_by_name.end() ? it->second : OXML_SharedStyle() ;
}

UT_Error OXML_Document::addStyle(const std::string & id, const std::string & name, 
								 const gchar ** attributes)
{
	OXML_SharedStyle obj;
	try {
		obj.reset( new OXML_Style(id, name) );
	} 
    catch(...) {
		UT_DEBUGMSG(("Object creation failed!\n"));
		return UT_OUTOFMEM;
	}
	obj->setAttributes(PP_std_copyProps(attributes));
	return addStyle(obj);
}

UT_Error OXML_Document::addStyle(const OXML_SharedStyle & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_styles_by_id[obj->getId()] = obj;
	m_styles_by_name[obj->getName()] = obj;
	return UT_OK;
}

UT_Error OXML_Document::addList(const OXML_SharedList & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_lists_by_id[obj->getId()] = obj;
	return UT_OK;
}

OXML_SharedList OXML_Document::getListById(UT_uint32 id) const
{
	OXML_ListMap::const_iterator it;
	it = m_lists_by_id.find(id);
	return it != m_lists_by_id.end() ? it->second : OXML_SharedList() ;
}

OXML_SharedImage OXML_Document::getImageById(const std::string & id) const
{
	OXML_ImageMap::const_iterator it;
	it = m_images_by_id.find(id);
	return it != m_images_by_id.end() ? it->second : OXML_SharedImage() ;
}

UT_Error OXML_Document::addImage(const OXML_SharedImage & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	std::string str;
	str += obj->getId();
	m_images_by_id[str] = obj;
	return UT_OK;
}

UT_Error OXML_Document::clearStyles()
{
	m_styles_by_id.clear();
	m_styles_by_name.clear();
	return m_styles_by_id.size() == 0 && m_styles_by_name.size() == 0 ? UT_OK : UT_ERROR;
}

OXML_SharedSection OXML_Document::getHeader(const std::string & id) const
{
	OXML_SectionMap::const_iterator it;
	it = m_headers.find(id);
	return it != m_headers.end() ? it->second : OXML_SharedSection() ;
}

UT_Error OXML_Document::addHeader(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_headers[obj->getId()] = obj;
	return UT_OK;
}

UT_Error OXML_Document::clearHeaders()
{
	m_headers.clear();
	return m_headers.size() == 0 ? UT_OK : UT_ERROR;
}

bool OXML_Document::isAllDefault(const bool & header) const
{
	const gchar* type = NULL;
	OXML_SectionMap::const_iterator it;
	if(!header)
	{
		for (it = m_footers.begin(); it != m_footers.end(); it++)
		{
			if(it->second->getAttribute("type", type) == UT_OK)
			{
				if(strcmp(type, "default") != 0)
				{
					return false;
				}
			}
		}
	}
	else
	{
		for (it = m_headers.begin(); it != m_headers.end(); it++)
		{
			if(it->second->getAttribute("type", type) == UT_OK)
			{
				if(strcmp(type, "default") != 0)
				{
					return false;
				}
			}
		}
	}
	return true;
}
OXML_SharedSection OXML_Document::getHdrFtrById(const bool & header, const std::string & id) const
{
	const gchar* hdrFtrId = NULL;
	OXML_SectionMap::const_iterator it;
	if(!header)
	{
		for (it = m_footers.begin(); it != m_footers.end(); it++)
		{
			if(it->second->getAttribute("id", hdrFtrId) == UT_OK)
			{
				if(!strcmp(hdrFtrId, id.c_str()))
				{
					return it->second;
				}
			}
		}
	}
	else
	{
		for (it = m_headers.begin(); it != m_headers.end(); it++)
		{
			if(it->second->getAttribute("id", hdrFtrId) == UT_OK)
			{
				if(!strcmp(hdrFtrId, id.c_str()))
				{
					return it->second;
				}
			}
		}
	}
	return OXML_SharedSection();
}

OXML_SharedSection OXML_Document::getFootnote(const std::string & id) const
{
	OXML_SectionMap::const_iterator it;
	it = m_footnotes.find(id);
	return it != m_footnotes.end() ? it->second : OXML_SharedSection() ;
}

UT_Error OXML_Document::addFootnote(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_footnotes[obj->getId()] = obj;
	return UT_OK;
}

UT_Error OXML_Document::clearFootnotes()
{
	m_footnotes.clear();
	return m_footnotes.size() == 0 ? UT_OK : UT_ERROR;
}

OXML_SharedSection OXML_Document::getEndnote(const std::string & id) const
{
	OXML_SectionMap::const_iterator it;
	it = m_endnotes.find(id);
	return it != m_endnotes.end() ? it->second : OXML_SharedSection() ;
}

UT_Error OXML_Document::addEndnote(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_endnotes[obj->getId()] = obj;
	return UT_OK;
}

UT_Error OXML_Document::clearEndnotes()
{
	m_endnotes.clear();
	return m_endnotes.size() == 0 ? UT_OK : UT_ERROR;
}

OXML_SharedSection OXML_Document::getFooter(const std::string & id) const
{
	OXML_SectionMap::const_iterator it;
	it = m_footers.find(id);
	return it != m_footers.end() ? it->second : OXML_SharedSection() ;
}

UT_Error OXML_Document::addFooter(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	m_footers[obj->getId()] = obj;
	return UT_OK;
}

UT_Error OXML_Document::clearFooters()
{
	m_footers.clear();
	return m_footers.size() == 0 ? UT_OK : UT_ERROR;
}

OXML_SharedSection OXML_Document::getLastSection() const
{
	UT_return_val_if_fail(!m_sections.empty(), OXML_SharedSection() );
	return m_sections.back();
}

OXML_SharedSection OXML_Document::getSection(const std::string & id) const
{
	OXML_SectionVector::const_iterator it;
	it = std::find(m_sections.begin(), m_sections.end(), id);
	return ( it != m_sections.end() ) ? (*it) : OXML_SharedSection() ;
}

UT_Error OXML_Document::appendSection(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	try {
		m_sections.push_back(obj);
	} 
    catch(...) {
		UT_DEBUGMSG(("Bad alloc!\n"));
		return UT_OUTOFMEM;
	}
	return UT_OK;
}

UT_Error OXML_Document::clearSections()
{
	m_sections.clear();
	return m_sections.size() == 0 ? UT_OK : UT_ERROR;
}

OXML_SharedTheme OXML_Document::getTheme()
{
	if (m_theme.get() == NULL) {
		try {
			m_theme.reset(new OXML_Theme());
		} 
        catch(...) {
			UT_DEBUGMSG(("Bad alloc!\n"));
			return OXML_SharedTheme();
		}
	}
	return m_theme;
}

OXML_SharedFontManager OXML_Document::getFontManager()
{
	if (m_fontManager.get() == NULL) {
		try {
			m_fontManager.reset(new OXML_FontManager());
		} 
        catch(...) {
			UT_DEBUGMSG(("Bad alloc!\n"));
			return OXML_SharedFontManager();
		}
	}
	return m_fontManager;
}

UT_Error OXML_Document::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;
	
	ret = exporter->startDocument();
	if(ret != UT_OK)
		return ret;

	OXML_StyleMap::iterator it1;
	for (it1 = m_styles_by_id.begin(); it1 != m_styles_by_id.end(); it1++) {
		ret = it1->second->serialize(exporter);
		if (ret != UT_OK)
			return ret;
	}

	//serialize abstract numbering definitions
	OXML_ListMap::iterator it2;
	for (it2 = m_lists_by_id.begin(); it2 != m_lists_by_id.end(); it2++) {
		ret = it2->second->serialize(exporter);
		if (ret != UT_OK)
			return ret;
	}

	//serialize actual numbering definitions
	OXML_ListMap::iterator it3;
	for (it3 = m_lists_by_id.begin(); it3 != m_lists_by_id.end(); it3++) {
		ret = it3->second->serializeNumbering(exporter);
		if (ret != UT_OK)
			return ret;
	}

	OXML_ImageMap::iterator it4;
	for (it4 = m_images_by_id.begin(); it4 != m_images_by_id.end(); it4++) {
		ret = it4->second->serialize(exporter);
		if (ret != UT_OK)
			return ret;
	}

	OXML_SectionMap::iterator it5;
	for (it5 = m_headers.begin(); it5 != m_headers.end(); it5++)
	{
		it5->second->setHandledHdrFtr(false); // Headers are not handled before serialization of sections
	}

	OXML_SectionMap::iterator it6;
	for (it6 = m_footers.begin(); it6 != m_footers.end(); it6++)
	{
		it6->second->setHandledHdrFtr(false); // Footers are not handled before serialization of sections
	}

	OXML_SectionVector::size_type i;
	for (i = 0; i < m_sections.size(); i++)
	{
		ret = m_sections[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	ret = exporter->startSectionProperties();
	if(ret != UT_OK)
		return ret;

	bool firstPageHdrFtr = false;
	bool evenPageHdrFtr = false;
	bool handled = false;

	//serialize headers
	for (it5 = m_headers.begin(); it5 != m_headers.end(); it5++) {

		if(it5->second->hasFirstPageHdrFtr())
			firstPageHdrFtr = true;	
		if(it5->second->hasEvenPageHdrFtr())
			evenPageHdrFtr = true;	

		handled = it5->second->getHandledHdrFtr();

		if(!handled)
		{
			it5->second->setHandledHdrFtr(true);
			ret = it5->second->serializeHeader(exporter);
			if (ret != UT_OK)
				return ret;
		}
	}

	//serialize footers
	for (it6 = m_footers.begin(); it6 != m_footers.end(); it6++) {

		if(it6->second->hasFirstPageHdrFtr())
			firstPageHdrFtr = true;	
		if(it6->second->hasEvenPageHdrFtr())
			evenPageHdrFtr = true;	

		handled = it6->second->getHandledHdrFtr();

		if(!handled)
		{
			it6->second->setHandledHdrFtr(true);
			ret = it6->second->serializeFooter(exporter);
			if (ret != UT_OK)
				return ret;
		}
	}

	ret = exporter->setContinuousSection(TARGET_DOCUMENT);
	if(ret != UT_OK)
		return ret;

	if(firstPageHdrFtr)
	{
		ret = exporter->setTitlePage();
		if(ret != UT_OK)
			return ret;
	}

	if(evenPageHdrFtr)
	{
		ret = exporter->setEvenAndOddHeaders();
		if(ret != UT_OK)
			return ret;
	}

	//set page size and orientation here
	if(!m_pageWidth.empty() && !m_pageHeight.empty())
	{
		ret = exporter->setPageSize(TARGET_DOCUMENT, m_pageWidth.c_str(), m_pageHeight.c_str(), m_pageOrientation.c_str());
		if(ret != UT_OK)
			return ret;		
	}

	//set page margins
	if(!m_pageMarginTop.empty() && !m_pageMarginLeft.empty() && !m_pageMarginRight.empty() && !m_pageMarginBottom.empty())
	{
		ret = exporter->setPageMargins(TARGET_DOCUMENT, m_pageMarginTop.c_str(), m_pageMarginLeft.c_str(),
										m_pageMarginRight.c_str(), m_pageMarginBottom.c_str());
		if(ret != UT_OK)
			return ret;		
	}

	//set page columns
	if(!m_colNum.empty() && !m_colSep.empty())
	{
		ret = exporter->setColumns(TARGET_DOCUMENT, m_colNum.c_str(), m_colSep.c_str());
		if(ret != UT_OK)
			return ret;
	}

	ret = exporter->finishSectionProperties();
	if(ret != UT_OK)
		return ret;

	//serialize footnotes
	OXML_SectionMap::iterator it7;
	for (it7 = m_footnotes.begin(); it7 != m_footnotes.end(); it7++) {
		ret = it7->second->serializeFootnote(exporter);
		if (ret != UT_OK)
			return ret;
	}

	//serialize endnotes
	OXML_SectionMap::iterator it8;
	for (it8 = m_endnotes.begin(); it8 != m_endnotes.end(); it8++) {
		ret = it8->second->serializeEndnote(exporter);
		if (ret != UT_OK)
			return ret;
	}
	
	return exporter->finishDocument();
}

UT_Error OXML_Document::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	//Adding styles to PT
	OXML_StyleMap::iterator it1;
	for (it1 = m_styles_by_id.begin(); it1 != m_styles_by_id.end(); it1++) {
		ret = it1->second->addToPT(pDocument);
		if (ret != UT_OK) return ret;
	}

	_assignHdrFtrIds(); //Must be done before appending sections to the PT

	//Adding sections to PT
	OXML_SectionVector::iterator it2;
	for (it2 = m_sections.begin(); it2 != m_sections.end(); it2++) {
		//set page margins here 
		ret = (*it2)->setPageMargins(m_pageMarginTop, m_pageMarginLeft, m_pageMarginRight, m_pageMarginBottom);
		if (ret != UT_OK) return ret;		
		ret = (*it2)->addToPT(pDocument);
		if (ret != UT_OK) return ret;
	}

	//Adding header and footer sections to PT
	OXML_SectionMap::iterator it3;
	for (it3 = m_headers.begin(); it3 != m_headers.end(); it3++) {
		ret = it3->second->addToPTAsHdrFtr(pDocument);
		if (ret != UT_OK) return ret;
	}
	for (it3 = m_footers.begin(); it3 != m_footers.end(); it3++) {
		ret = it3->second->addToPTAsHdrFtr(pDocument);
		if (ret != UT_OK) return ret;
	}

	//Adding lists to PT
	OXML_ListMap::iterator it4;
	for (it4 = m_lists_by_id.begin(); it4 != m_lists_by_id.end(); it4++) {
		ret = it4->second->addToPT(pDocument);
		if (ret != UT_OK) return ret;
	}

	//Adding images to PT
	OXML_ImageMap::iterator it5;
	for (it5 = m_images_by_id.begin(); it5 != m_images_by_id.end(); it5++) {
		ret = it5->second->addToPT(pDocument);
		if (ret != UT_OK) return ret;
	}

	return applyPageProps(pDocument);
}

UT_Error OXML_Document::applyPageProps(PD_Document* pDocument)
{
	PP_PropertyVector pageAtts = {
        "units",    "in",
        "page-scale", "1.0"
    };

	if(m_pageOrientation.empty())
		m_pageOrientation = "portrait";

	if(!m_pageWidth.empty())
	{
		pageAtts.push_back("width");
		pageAtts.push_back(m_pageWidth);
	}
	if(!m_pageHeight.empty())
	{
		pageAtts.push_back("height");
		pageAtts.push_back(m_pageHeight);
	}
	if(!m_pageOrientation.empty())
	{
		pageAtts.push_back("orientation");
		pageAtts.push_back(m_pageOrientation);
	}

	fp_PageSize ps(UT_convertDimensionless(m_pageWidth.c_str()), UT_convertDimensionless(m_pageHeight.c_str()), DIM_IN);
	pageAtts.push_back("pagetype");
	pageAtts.push_back(ps.getPredefinedName());

	return pDocument->setPageSizeFromFile(pageAtts) ? UT_OK : UT_ERROR;
}

void OXML_Document::_assignHdrFtrIds()
{
	OXML_SectionMap::iterator it;
	UT_uint32 index = 0;
	for (it = m_headers.begin(); it != m_headers.end(); it++) {
		it->second->setAttribute("id", UT_convertToDimensionlessString(index, ".0") );
		index++;
	}
	for (it = m_footers.begin(); it != m_footers.end(); it++) {
		it->second->setAttribute("id", UT_convertToDimensionlessString(index, ".0") );
		index++;
	}
}

std::string OXML_Document::getMappedNumberingId(const std::string & numId) const
{
	std::map<std::string, std::string>::const_iterator iter = m_numberingMap.find(numId);
	if(iter == m_numberingMap.end())
		return "";
	return iter->second; 
}

bool OXML_Document::setMappedNumberingId(const std::string & numId, const std::string & abstractNumId)
{
	m_numberingMap.insert(std::make_pair(numId, abstractNumId));
	return m_numberingMap.find(numId) != m_numberingMap.end();
}

std::string OXML_Document::getBookmarkName(const std::string & bookmarkId) const
{
	std::map<std::string, std::string>::const_iterator iter = m_bookmarkMap.find(bookmarkId);
	if(iter == m_bookmarkMap.end())
	{
		UT_DEBUGMSG(("FRT:Can't find bookmark name with id:%s\n", bookmarkId.c_str()));
		return "";
	}
	return iter->second; 
}

std::string OXML_Document::getBookmarkId(const std::string & bookmarkName) const
{
	std::map<std::string, std::string>::const_iterator iter;
	for (iter = m_bookmarkMap.begin(); iter != m_bookmarkMap.end(); iter++) {
		if(!(iter->second).compare(bookmarkName))
		{
			return iter->first;
		}
	}
	return ""; 
}

bool OXML_Document::setBookmarkName(const std::string & bookmarkId, const std::string & bookmarkName)
{
	m_bookmarkMap.insert(std::make_pair(bookmarkId, bookmarkName));
	return m_bookmarkMap.find(bookmarkId) != m_bookmarkMap.end();
}

void OXML_Document::setPageWidth(const std::string & width)
{
	m_pageWidth = width;
}

void OXML_Document::setPageHeight(const std::string & height)
{
	m_pageHeight = height;
}

void OXML_Document::setPageOrientation(const std::string & orientation)
{
	m_pageOrientation = orientation;
}

void OXML_Document::setPageMargins(const std::string & top, const std::string & left, const std::string & right, const std::string & bottom)
{
	m_pageMarginTop = top;
	m_pageMarginLeft = left;
	m_pageMarginRight = right;
	m_pageMarginBottom = bottom;
}

void OXML_Document::setColumns(const std::string & colNum, const std::string & colSep)
{
	m_colNum = colNum;
	m_colSep = colSep;
}
