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

#ifndef _OXML_SECTION_H_
#define _OXML_SECTION_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>
#include <OXML_Element.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>

// External includes
#include <string>
#include <vector>
#include <map>
#include <memory>

class OXML_Section;
class OXML_Element_Paragraph;
class IE_Exp_OpenXML;

typedef std::shared_ptr<OXML_Section> OXML_SharedSection;

/* \class OXML_Section
 * \brief This class represents a single section in the OpenXML data model.
 * It holds references to all its content, as well as references
 * to all its corresponding headers and footers.
*/
class OXML_Section : public OXML_ObjectWithAttrProp
{
public:
	OXML_Section();
	OXML_Section(const std::string & id);
	virtual ~OXML_Section();

	const std::string & getId() const
		{ return m_id; }
	OXML_SectionBreakType getBreakType() const
		{ return m_breakType; }
	void setBreakType(OXML_SectionBreakType br)
		{ m_breakType = br; }

	const char * getHeaderId(OXML_HeaderFooterType type) const
		{ return m_headerIds[type]; }
	const char * getFooterId(OXML_HeaderFooterType type) const
		{ return m_footerIds[type]; }
	void setHeaderId(const char * id, OXML_HeaderFooterType type)
		{ m_headerIds[type] = g_strdup(id); }
	void setFooterId(const char * id, OXML_HeaderFooterType type)
		{ m_footerIds[type] = g_strdup(id); }

	bool operator ==(const std::string & id);
	friend bool operator ==(const OXML_SharedSection& lhs, const std::string & id);

	OXML_SharedElement getElement(const std::string & id);
	UT_Error appendElement(OXML_SharedElement obj);
	inline void setChildren(OXML_ElementVector c) { m_children = c; }
	UT_Error clearChildren();

	//! Writes the OpenXML section and all its content to a file on disk.
	/*! This method is used during the export process.
		\param exporter the actual exporter which handles writing the files.
	*/
	UT_Error serialize(IE_Exp_OpenXML* exporter);
	UT_Error serializeProperties(IE_Exp_OpenXML* exporter, OXML_Element_Paragraph* pParagraph);
	void applyDocumentProperties();

	UT_Error serializeHeader(IE_Exp_OpenXML* exporter);
	UT_Error serializeFooter(IE_Exp_OpenXML* exporter);
	UT_Error serializeFootnote(IE_Exp_OpenXML* exporter);
	UT_Error serializeEndnote(IE_Exp_OpenXML* exporter);

	//! Appends this section and all its content to the Abiword Piecetable.
	/*! This method is used during the import process.
		\param pDocument A valid reference to the PD_Document object.
	*/
	UT_Error addToPT(PD_Document * pDocument);
	UT_Error addToPTAsHdrFtr(PD_Document * pDocument);
	UT_Error addToPTAsFootnote(PD_Document * pDocument);
	UT_Error addToPTAsEndnote(PD_Document * pDocument);

	void setTarget(int target);
	bool hasFirstPageHdrFtr() const;
	bool hasEvenPageHdrFtr() const;
	void setHandledHdrFtr(bool val)
		{ m_handledHdrFtr = val; }
	bool getHandledHdrFtr() const
		{ return m_handledHdrFtr; }

	UT_Error setPageMargins(const std::string & top, const std::string & left, const std::string & right, const std::string & bottom);

private:
	std::string m_id;
	OXML_SectionBreakType m_breakType;
	OXML_ElementVector m_children;
	OXML_Element_Paragraph* m_lastParagraph;
	char * m_headerIds[3];
	char * m_footerIds[3];
	int m_target;
	bool m_handledHdrFtr;

	UT_Error _setReferenceIds();
};


typedef std::vector< OXML_SharedSection > OXML_SectionVector;
typedef std::map<std::string, OXML_SharedSection > OXML_SectionMap;

inline bool operator ==(const OXML_SharedSection& lhs, const std::string & id)
{
	return (*lhs) == id;
}

#endif //_OXML_SECTION_H_

