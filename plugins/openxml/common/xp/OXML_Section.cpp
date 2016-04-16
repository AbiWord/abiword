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
#include <OXML_Section.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Element.h>
#include <OXML_Document.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>
#include <pt_Types.h>

// External includes
#include <string>

OXML_Section::OXML_Section() : 
	OXML_ObjectWithAttrProp(), 
	m_id(""), 
	m_breakType(NEXTPAGE_BREAK),
	m_lastParagraph(NULL),
	m_target(0),
	m_handledHdrFtr(false)
{
	m_headerIds[0] = NULL; m_headerIds[1] = NULL; m_headerIds[2] = NULL;
	m_footerIds[0] = NULL; m_footerIds[1] = NULL; m_footerIds[2] = NULL;
	m_children.clear();
}

OXML_Section::OXML_Section(const std::string & id) : 
	OXML_ObjectWithAttrProp(), 
	m_id(id), 
	m_breakType(NEXTPAGE_BREAK),
	m_lastParagraph(NULL),
	m_target(0),
	m_handledHdrFtr(false)
{
	m_headerIds[0] = NULL; m_headerIds[1] = NULL; m_headerIds[2] = NULL;
	m_footerIds[0] = NULL; m_footerIds[1] = NULL; m_footerIds[2] = NULL;
	m_children.clear();
}

OXML_Section::~OXML_Section()
{
	g_free(m_headerIds[0]); g_free(m_headerIds[1]); g_free(m_headerIds[2]);
	g_free(m_footerIds[0]); g_free(m_footerIds[1]); g_free(m_footerIds[2]);
	clearChildren();
}

bool OXML_Section::operator ==(const std::string & id)
{
	return this->m_id.compare(id) == 0;
}

OXML_SharedElement OXML_Section::getElement(const std::string & id)
{
	OXML_ElementVector::iterator it;
	it = std::find(m_children.begin(), m_children.end(), id);
	return ( it != m_children.end() ) ? (*it) : OXML_SharedElement() ;
}

UT_Error OXML_Section::appendElement(OXML_SharedElement obj)
{
	UT_return_val_if_fail(obj.get() != NULL, UT_ERROR);

	try {
		m_children.push_back(obj);
	} catch(...) {
		UT_DEBUGMSG(("Bad alloc!\n"));
		return UT_OUTOFMEM;
	}

	obj->setTarget(m_target);

	return UT_OK;
}

UT_Error OXML_Section::clearChildren()
{
	m_children.clear();
	return m_children.size() == 0 ? UT_OK : UT_ERROR;
}

UT_Error OXML_Section::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;
	OXML_Document* doc = OXML_Document::getInstance();

	applyDocumentProperties();

	OXML_ElementVector::size_type i;

	if(this != doc->getLastSection().get())
	{
		for (i = 0; i < m_children.size(); i++)
		{
			if(m_children[i].get() && ((m_children[i].get())->getTag() == P_TAG))
			{
				static_cast<OXML_Element_Paragraph*>(m_children[i].get())->setSection(this);
				m_lastParagraph = static_cast<OXML_Element_Paragraph*>(m_children[i].get());
			}
		}
	}

	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}	
	return UT_OK;
}

void OXML_Section::applyDocumentProperties()
{
	OXML_Document* pDoc = OXML_Document::getInstance();

	const gchar* num = NULL;
	const gchar* sep = "off";
	const gchar* marginTop = NULL;
	const gchar* marginLeft = NULL;
	const gchar* marginRight = NULL;
	const gchar* marginBottom = NULL;

	if(getProperty("columns", num) != UT_OK)
		num = NULL;

	if((getProperty("column-line", sep) != UT_OK) || (strcmp(sep, "on") != 0))
		sep = "off";

	if(getProperty("page-margin-top", marginTop) != UT_OK)
		marginTop = NULL;

	if(getProperty("page-margin-left", marginLeft) != UT_OK)
		marginLeft = NULL;

	if(getProperty("page-margin-right", marginRight) != UT_OK)
		marginRight = NULL;

	if(getProperty("page-margin-bottom", marginBottom) != UT_OK)
		marginBottom = NULL;

	if(num && sep)
		pDoc->setColumns(num, sep);

	if(marginTop && marginLeft && marginRight && marginBottom)
		pDoc->setPageMargins(marginTop, marginLeft, marginRight, marginBottom);
}

UT_Error OXML_Section::serializeProperties(IE_Exp_OpenXML* exporter, OXML_Element_Paragraph* pParagraph)
{
	//TODO: Add all the property serializations here
	UT_Error err = UT_OK;

	if(pParagraph != m_lastParagraph)
	{
		return UT_OK;
	}

	OXML_Document* doc = OXML_Document::getInstance();
	bool defaultHdr = doc->isAllDefault(true);
	bool defaultFtr = doc->isAllDefault(false);

	const gchar* num = NULL;
	const gchar* sep = "off";
	const gchar* marginTop = NULL;
	const gchar* marginLeft = NULL;
	const gchar* marginRight = NULL;
	const gchar* marginBottom = NULL;
	const gchar* footerId = NULL;
	const gchar* headerId = NULL;

	if(getProperty("columns", num) != UT_OK)
		num = NULL;

	if((getProperty("column-line", sep) != UT_OK) || (strcmp(sep, "on") != 0))
		sep = "off";

	if(getProperty("page-margin-top", marginTop) != UT_OK)
		marginTop = NULL;

	if(getProperty("page-margin-left", marginLeft) != UT_OK)
		marginLeft = NULL;

	if(getProperty("page-margin-right", marginRight) != UT_OK)
		marginRight = NULL;

	if(getProperty("page-margin-bottom", marginBottom) != UT_OK)
		marginBottom = NULL;

	if(getAttribute("header", headerId) != UT_OK)
		headerId = NULL;

	if(getAttribute("footer", footerId) != UT_OK)
		footerId = NULL;

	err = exporter->startSectionProperties();
	if(err != UT_OK)
		return err;

	if(num && sep)
	{
		err = exporter->setColumns(m_target, num, sep);
		if(err != UT_OK)
			return err;
	}

	err = exporter->setContinuousSection(m_target);
	if(err != UT_OK)
		return err;

	if(defaultHdr && headerId && doc)
	{
		OXML_SharedSection header_section = doc->getHdrFtrById(true, headerId);
		if(header_section != NULL)
		{
			header_section->setHandledHdrFtr(true);
			err = header_section->serializeHeader(exporter);
			if (err != UT_OK)
				return err;
		}
	}

	if(defaultFtr && footerId && doc)
	{
		OXML_SharedSection footer_section = doc->getHdrFtrById(false, footerId);
		if(footer_section != NULL)
		{
			footer_section->setHandledHdrFtr(true);
			err = footer_section->serializeFooter(exporter);
			if (err != UT_OK)
				return err;
		}
	}

	if(marginTop && marginLeft && marginRight && marginBottom)
	{	
		err = exporter->setPageMargins(m_target, marginTop, marginLeft, marginRight, marginBottom);
		if(err != UT_OK)
			return err;
	}

	return exporter->finishSectionProperties();
}

bool OXML_Section::hasFirstPageHdrFtr() const
{
	UT_Error ret = UT_OK;

	const gchar* headerType;

	ret = getAttribute("type", headerType);
	if(ret != UT_OK)
		return false;

	return strstr(headerType, "first");
}

bool OXML_Section::hasEvenPageHdrFtr() const
{
	UT_Error ret = UT_OK;

	const gchar* headerType;

	ret = getAttribute("type", headerType);
	if(ret != UT_OK)
		return false;

	return strstr(headerType, "even");
}

/**
 * Serialize the section as a header
 */
UT_Error OXML_Section::serializeHeader(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	const gchar* headerId;
	const gchar* headerType;

	ret = getAttribute("id", headerId);
	if(ret != UT_OK)
		return UT_OK;

	ret = getAttribute("type", headerType);
	if(ret != UT_OK)
		return UT_OK;

	const gchar* type = "default";
	//OOXML includes default, first and even.  
	if(strstr(headerType, "first"))
	{
		type = "first";
	}
	else if(strstr(headerType, "even"))
	{
		type = "even";
	}
	else if(strstr(headerType, "last"))
	{
		//last not implemented in OOXML
		return UT_OK;
	}
			
	std::string header("hId");
	header += headerId;

	ret = exporter->setHeaderReference(header.c_str(), type);
	if(ret != UT_OK)
		return ret;

	ret = exporter->setHeaderRelation(header.c_str(), headerId);
	if(ret != UT_OK)
		return ret;	

	ret = exporter->startHeaderStream(headerId);
	if(ret != UT_OK)
		return ret;	

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return exporter->finishHeaderStream();
}

/**
 * Serialize the section as a footer
 */
UT_Error OXML_Section::serializeFooter(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	const gchar* footerId;
	const gchar* footerType;

	ret = getAttribute("id", footerId);
	if(ret != UT_OK)
		return UT_OK;
		
	std::string footer("fId");
	footer += footerId;

	ret = getAttribute("type", footerType);
	if(ret != UT_OK)
		return UT_OK;

	const gchar* type = "default";
	//OOXML includes default, first and even.  
	if(strstr(footerType, "first"))
	{
		type = "first";
	}
	else if(strstr(footerType, "even"))
	{
		type = "even";
	}
	else if(strstr(footerType, "last"))
	{
		//last not implemented in OOXML
		return UT_OK;
	}


	ret = exporter->setFooterReference(footer.c_str(), type);
	if(ret != UT_OK)
		return ret;

	ret = exporter->setFooterRelation(footer.c_str(), footerId);
	if(ret != UT_OK)
		return ret;	

	ret = exporter->startFooterStream(footerId);
	if(ret != UT_OK)
		return ret;	

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		m_children[i]->setTarget(TARGET_FOOTER);
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return exporter->finishFooterStream();
}

/**
 * Serialize the section as a footnote
 */
UT_Error OXML_Section::serializeFootnote(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	const gchar* footnoteId;

	ret = getAttribute("footnote-id", footnoteId);
	if(ret != UT_OK)
		return UT_OK;

	ret = exporter->startFootnote(footnoteId);
	if(ret != UT_OK)
		return ret;	

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return exporter->finishFootnote();
}

/**
 * Serialize the section as a endnote
 */
UT_Error OXML_Section::serializeEndnote(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	const gchar* endnoteId;

	ret = getAttribute("endnote-id", endnoteId);
	if(ret != UT_OK)
		return UT_OK;

	ret = exporter->startEndnote(endnoteId);
	if(ret != UT_OK)
		return ret;	

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return exporter->finishEndnote();
}

UT_Error OXML_Section::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	if (pDocument == NULL)
		return UT_ERROR;

	ret = _setReferenceIds();
	UT_return_val_if_fail(ret == UT_OK, ret);

	//Appending section
	PP_PropertyVector attr = this->getAttributesWithProps();
	ret = pDocument->appendStrux(PTX_Section, attr) ? UT_OK : UT_ERROR;
	UT_return_val_if_fail(ret == UT_OK, ret);

	//Appending page break to the section if necessary
	if (m_breakType == NEXTPAGE_BREAK || m_breakType == ODDPAGE_BREAK || m_breakType == EVENPAGE_BREAK) {
		UT_UCSChar ucs = UCS_FF;
		ret = pDocument->appendSpan(&ucs, 1) ? UT_OK : UT_ERROR;
		UT_return_val_if_fail(ret == UT_OK, ret);
	}

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->addToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
	}

	return UT_OK;
}

UT_Error OXML_Section::addToPTAsFootnote(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	const PP_PropertyVector attr = { "footnote-id", m_id };

	ret = pDocument->appendStrux(PTX_SectionFootnote, attr) ? UT_OK : UT_ERROR;
	UT_return_val_if_fail(ret == UT_OK, ret);

	const PP_PropertyVector field_fmt = {
		"type", "footnote_anchor",
		"footnote-id", m_id
	};

	if(!pDocument->appendObject(PTO_Field, field_fmt))
		return UT_ERROR;

	OXML_ElementVector::size_type i;
	i = 0;

	if(m_children[0].get() && ((m_children[0].get())->getTag() == P_TAG))
	{
		//skip the first paragraph and directly add its children
		ret = m_children[0]->addChildrenToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
		i = 1;
	}

	for (; i < m_children.size(); i++)
	{
		ret = m_children[i]->addToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
	}

	return pDocument->appendStrux(PTX_EndFootnote, PP_NOPROPS) ? UT_OK : UT_ERROR;
}

UT_Error OXML_Section::addToPTAsEndnote(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	const PP_PropertyVector attr = { "endnote-id", m_id };

	ret = pDocument->appendStrux(PTX_SectionEndnote, attr) ? UT_OK : UT_ERROR;
	UT_return_val_if_fail(ret == UT_OK, ret);

	const PP_PropertyVector field_fmt = {
		"type", "endnote_anchor",
		"endnote-id", m_id
	};

	if(!pDocument->appendObject(PTO_Field, field_fmt))
		return UT_ERROR;

	OXML_ElementVector::size_type i;
	i = 0;

	if(m_children[0].get() && ((m_children[0].get())->getTag() == P_TAG))
	{
		//skip the first paragraph and directly add its children
		ret = m_children[0]->addChildrenToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
		i = 1;
	}

	for (; i < m_children.size(); i++)
	{
		ret = m_children[i]->addToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
	}

	return pDocument->appendStrux(PTX_EndEndnote, PP_NOPROPS) ? UT_OK : UT_ERROR;
}

UT_Error OXML_Section::addToPTAsHdrFtr(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	const PP_PropertyVector attr = this->getAttributes();
	ret = pDocument->appendStrux(PTX_SectionHdrFtr, attr) ? UT_OK : UT_ERROR;
	UT_return_val_if_fail(ret == UT_OK, ret);

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->addToPT(pDocument);
		UT_return_val_if_fail(ret == UT_OK, ret);
	}
	return ret;
}

UT_Error OXML_Section::_setReferenceIds()
{
	OXML_Document * doc = OXML_Document::getInstance(); UT_ASSERT(NULL != doc);
	OXML_SharedSection corresp_sect;
	const char * ooxml_id(NULL), * abw_id(NULL);

	//Headers...
	for (UT_uint32 i = 0; i <= 2; i++) {
		ooxml_id = m_headerIds[i]; abw_id = NULL;
		if (NULL != ooxml_id) {
			corresp_sect = doc->getHeader(ooxml_id);
			UT_return_val_if_fail( NULL != corresp_sect.get(), UT_ERROR );
			corresp_sect->getAttribute("id", abw_id);
			UT_return_val_if_fail( NULL != abw_id, UT_ERROR );
			if (i == DEFAULT_HDRFTR) {
				this->setAttribute("header", abw_id );
			} else if (i == FIRSTPAGE_HDRFTR) {
				this->setAttribute("header-first", abw_id );
			} else if (i == EVENPAGE_HDRFTR) {
				this->setAttribute("header-even", abw_id );
			}	
		}
	}

	//Footers...
	for (UT_uint32 i = 0; i <= 2; i++) {
		ooxml_id = m_footerIds[i]; abw_id = NULL;
		if (NULL != ooxml_id) {
			corresp_sect = doc->getFooter(ooxml_id);
			UT_return_val_if_fail( NULL != corresp_sect.get(), UT_ERROR );
			corresp_sect->getAttribute("id", abw_id);
			UT_return_val_if_fail( NULL != abw_id, UT_ERROR );
			if (i == DEFAULT_HDRFTR) {
				this->setAttribute("footer", abw_id );
			} else if (i == FIRSTPAGE_HDRFTR) {
				this->setAttribute("footer-first", abw_id );
			} else if (i == EVENPAGE_HDRFTR) {
				this->setAttribute("footer-even", abw_id );
			}	
		}
	}
	return UT_OK;
}

void OXML_Section::setTarget(int target)
{
	m_target = target;
}

UT_Error OXML_Section::setPageMargins(const std::string & top, const std::string & left, const std::string & right, const std::string & bottom)
{
	UT_Error ret = UT_OK;

	if(top.compare(""))
	{
		ret = setProperty("page-margin-top", top);
		if(ret != UT_OK)
			return ret;
	}

	if(left.compare(""))
	{
		ret = setProperty("page-margin-left", left);
		if(ret != UT_OK)
			return ret;
	}

	if(right.compare(""))
	{
		ret = setProperty("page-margin-right", right);
		if(ret != UT_OK)
			return ret;
	}

	if(bottom.compare(""))
	{
		ret = setProperty("page-margin-bottom", bottom);	
		if(ret != UT_OK)
			return ret;
	}

	return ret;
}
