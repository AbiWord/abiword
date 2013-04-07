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

#ifndef _OXML_DOCUMENT_H_
#define _OXML_DOCUMENT_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>
#include "OXML_Section.h"
#include "OXML_Style.h"
#include "OXML_Theme.h"
#include "OXML_FontManager.h"

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>
#include <ie_exp_OpenXML.h>

// External includes
#include <string>

/* \class OXML_Document
 * This class represents the data model representation of the OpenXML document.
 * Since there is only one document to be imported / exported at a time, this is
 * enforced by the use of the Singleton pattern.
 */
class OXML_Document : public OXML_ObjectWithAttrProp
{
public:
	//! Clears any previous document instance and provides a new blank one.
	static OXML_Document* getNewInstance();
	//! Provides a reference to the current OpenXML document.
	static OXML_Document* getInstance();
	//! Frees the current document and all its content from memory.
	static void destroyInstance();
	//! Provides a pointer to the last section that was appended (or empty SharedSection if none found).
	static OXML_SharedSection getCurrentSection();

	//! Returns a reference to the FIRST style with corresponding ID OR empty SharedStyle if none found.
	OXML_SharedStyle getStyleById(const std::string & id) const;
	//! Returns a reference to the FIRST style with corresponding name OR empty SharedStyle if none found.
	OXML_SharedStyle getStyleByName(const std::string & name) const;
	UT_Error addStyle(const std::string & id, const std::string & name, const gchar ** attributes);
	UT_Error addStyle(const OXML_SharedStyle & obj);
	UT_Error clearStyles();

	UT_Error addList(const OXML_SharedList& obj);
	UT_Error addImage(const OXML_SharedImage& obj);
	OXML_SharedList getListById(UT_uint32 id) const;
	OXML_SharedImage getImageById(const std::string & id) const;

	//! Returns a reference to the FIRST footnote with corresponding ID OR empty SharedSection if none found.
	OXML_SharedSection getFootnote(const std::string & id) const;
	UT_Error addFootnote(const OXML_SharedSection & obj);
	UT_Error clearFootnotes();

	OXML_SharedSection getEndnote(const std::string & id) const;
	UT_Error addEndnote(const OXML_SharedSection & obj);
	UT_Error clearEndnotes();

	//! Returns a reference to the FIRST header with corresponding ID OR empty SharedSection if none found.
	OXML_SharedSection getHeader(const std::string & id) const;
	UT_Error addHeader(const OXML_SharedSection & obj);
	UT_Error clearHeaders();

	bool isAllDefault(const bool & header) const;
	OXML_SharedSection getHdrFtrById(const bool & header, const std::string & id) const;

	//! Returns a reference to the FIRST footer with corresponding ID OR NULL if none found.
	OXML_SharedSection getFooter(const std::string & id) const;
	UT_Error addFooter(const OXML_SharedSection & obj);
	UT_Error clearFooters();

	//! Retrieves the last appended section of the document OR empty SharedSection if no sections have been appended.
	OXML_SharedSection getLastSection() const;
	//! Returns a reference to the FIRST section with corresponding ID OR empty SharedSection if none found.
	OXML_SharedSection getSection(const std::string & id) const;
	//! Appends a new section at the end of the list.
	UT_Error appendSection(const OXML_SharedSection & obj);
	UT_Error clearSections();

	OXML_SharedTheme getTheme();
	OXML_SharedFontManager getFontManager();

	//! Writes the OpenXML document and all its content to a file on disk.
	/*! This method is used during the export process.
		\param exporter the actual exporter which handles writing the files.
	*/
	UT_Error serialize(IE_Exp_OpenXML* exporter);
	//! Builds the Abiword Piecetable representation of the OpenXML document and all its content.
	/*! This method is used during the import process.
		\param pDocument A valid reference to the PD_Document object.
	*/
	UT_Error addToPT(PD_Document * pDocument);

	std::string getMappedNumberingId(const std::string & numId) const;
	bool setMappedNumberingId(const std::string & numId, const std::string & abstractNumId);

	std::string getBookmarkName(const std::string & bookmarkId) const;
	std::string getBookmarkId(const std::string & bookmarkName) const;
	bool setBookmarkName(const std::string & bookmarkId, const std::string & bookmarkName);

	void setPageWidth(const std::string & width);
	void setPageHeight(const std::string & height);
	void setPageOrientation(const std::string & orientation);
	void setPageMargins(const std::string & top, const std::string & left, const std::string & right, const std::string & bottom);
	void setColumns(const std::string & colNum, const std::string & colSep);

private:
	static OXML_Document* s_docInst;
	OXML_Document();
	virtual ~OXML_Document();

	OXML_SectionVector m_sections;

	OXML_SectionMap m_headers;
	OXML_SectionMap m_footers;
	OXML_SectionMap m_footnotes;
	OXML_SectionMap m_endnotes;

	OXML_StyleMap m_styles_by_id;
	OXML_StyleMap m_styles_by_name;

	OXML_SharedTheme m_theme;
	OXML_SharedFontManager m_fontManager;

	OXML_ListMap m_lists_by_id;
	OXML_ImageMap m_images_by_id;

	std::map<std::string, std::string> m_numberingMap;
	std::map<std::string, std::string> m_bookmarkMap;

	std::string m_pageWidth;
	std::string m_pageHeight;
	std::string m_pageOrientation;

	std::string m_pageMarginTop;
	std::string m_pageMarginLeft;
	std::string m_pageMarginRight;
	std::string m_pageMarginBottom;

	std::string m_colNum;
	std::string m_colSep;

	void _assignHdrFtrIds();
	UT_Error applyPageProps(PD_Document* pDocument);
};

#endif //_OXML_DOCUMENT_H_

