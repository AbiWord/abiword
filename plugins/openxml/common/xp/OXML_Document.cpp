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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
#include <ut_types.h>
#include <pd_Document.h>
#include <ut_exception.h>

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

OXML_SharedStyle OXML_Document::getStyleById(const std::string & id)
{
	OXML_StyleMap::iterator it;
	it = m_styles_by_id.find(id);
	return it != m_styles_by_id.end() ? it->second : OXML_SharedStyle() ;
}

OXML_SharedStyle OXML_Document::getStyleByName(const std::string & name)
{
	OXML_StyleMap::iterator it;
	it = m_styles_by_name.find(name);
	return it != m_styles_by_name.end() ? it->second : OXML_SharedStyle() ;
}

UT_Error OXML_Document::addStyle(const std::string & id, const std::string & name, 
								 const gchar ** attributes)
{
	OXML_SharedStyle obj;
	UT_TRY {
		obj.reset( new OXML_Style(id, name) );
	} UT_CATCH (UT_CATCH_ANY) {
		UT_DEBUGMSG(("Object creation failed!\n"));
		return UT_OUTOFMEM;
	} UT_END_CATCH
	obj->setAttributes(attributes);
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

OXML_SharedList OXML_Document::getListById(UT_uint32 id)
{
	OXML_ListMap::iterator it;
	it = m_lists_by_id.find(id);
	return it != m_lists_by_id.end() ? it->second : OXML_SharedList() ;
}

UT_Error OXML_Document::addImage(const OXML_SharedImage & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	std::string str("");
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

OXML_SharedSection OXML_Document::getHeader(const std::string & id)
{
	OXML_SectionMap::iterator it;
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

OXML_SharedSection OXML_Document::getFooter(const std::string & id)
{
	OXML_SectionMap::iterator it;
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

OXML_SharedSection OXML_Document::getLastSection()
{
	UT_return_val_if_fail(!m_sections.empty(), OXML_SharedSection() );
	return m_sections.back();
}

OXML_SharedSection OXML_Document::getSection(const std::string & id)
{
	OXML_SectionVector::iterator it;
	it = std::find(m_sections.begin(), m_sections.end(), id);
	return ( it != m_sections.end() ) ? (*it) : OXML_SharedSection() ;
}

UT_Error OXML_Document::appendSection(const OXML_SharedSection & obj)
{
	UT_return_val_if_fail(obj, UT_ERROR);

	UT_TRY {
		m_sections.push_back(obj);
	} UT_CATCH (UT_CATCH_ANY) {
		UT_DEBUGMSG(("Bad alloc!\n"));
		return UT_OUTOFMEM;
	} UT_END_CATCH
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
		UT_TRY {
			m_theme.reset(new OXML_Theme());
		} UT_CATCH (UT_CATCH_ANY) {
			UT_DEBUGMSG(("Bad alloc!\n"));
			return OXML_SharedTheme();
		} UT_END_CATCH
	}
	return m_theme;
}

OXML_SharedFontManager OXML_Document::getFontManager()
{
	if (m_fontManager.get() == NULL) {
		UT_TRY {
			m_fontManager.reset(new OXML_FontManager());
		} UT_CATCH (UT_CATCH_ANY) {
			UT_DEBUGMSG(("Bad alloc!\n"));
			return OXML_SharedFontManager();
		} UT_END_CATCH
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

	ret = exporter->writeDefaultStyle();
	if(ret != UT_OK)
		return ret;

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

	//serialize headers
	OXML_SectionMap::iterator it5;
	for (it5 = m_headers.begin(); it5 != m_headers.end(); it5++) {

		if(it5->second->hasFirstPageHdrFtr())
			firstPageHdrFtr = true;	
		if(it5->second->hasEvenPageHdrFtr())
			evenPageHdrFtr = true;	

		ret = it5->second->serializeHeader(exporter);
		if (ret != UT_OK)
			return ret;
	}

	//serialize footers
	OXML_SectionMap::iterator it6;
	for (it6 = m_footers.begin(); it6 != m_footers.end(); it6++) {

		if(it6->second->hasFirstPageHdrFtr())
			firstPageHdrFtr = true;	
		if(it6->second->hasEvenPageHdrFtr())
			evenPageHdrFtr = true;	

		ret = it6->second->serializeFooter(exporter);
		if (ret != UT_OK)
			return ret;
	}

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

	return ret;
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

std::string OXML_Document::getMappedNumberingId(std::string numId)
{
	std::map<std::string, std::string>::iterator iter = m_numberingMap.find(numId);
	if(iter == m_numberingMap.end())
		return "";
	return iter->second; 
}

bool OXML_Document::setMappedNumberingId(std::string numId, std::string abstractNumId)
{
	m_numberingMap.insert(std::make_pair(numId, abstractNumId));
	return m_numberingMap.find(numId) != m_numberingMap.end();
}

