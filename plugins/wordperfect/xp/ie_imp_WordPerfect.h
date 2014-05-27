/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2002-2004 Marc Maurer (uwog@uwog.net)
 * Copyright (C) 2001-2003 William Lachance (william.lachance@sympatico.ca)
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

/* See bug 1764
 * This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef IE_IMP_WP_H
#define IE_IMP_WP_H

#include <stdio.h>
#include <librevenge/librevenge.h>
#include "ie_imp.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_growbuf.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"
#include "fl_AutoNum.h"
#include "fl_TableLayout.h"
#include "fp_types.h"

using namespace std;

#define WP6_NUM_LIST_LEVELS 8  // see WP6FileStructure.h

// ABI_ListDefinition: tracks information on the list
class ABI_ListDefinition
{
public:
    ABI_ListDefinition(int iOutlineHash);
    void setListID(const int iLevel, const UT_uint32 iID) { m_iListIDs[iLevel-1] = iID; }
    UT_uint32 getListID(const int iLevel) const { return m_iListIDs[iLevel-1]; }
    FL_ListType getListType(const int iLevel) const { return m_listTypes[iLevel-1]; }
    void setListType(const int iLevel, const char type);
    void incrementLevelNumber(const int iLevel) { m_iListNumbers[iLevel - 1]++; }
    void setLevelNumber(const int iLevel, const int iNumber) { m_iListNumbers[iLevel - 1] = iNumber; }
    void setListLeftOffset(const int iLevel, const float listLeftOffset) { m_listLeftOffset[iLevel - 1] = listLeftOffset; }
    void setListMinLabelWidth(const int iLevel, const float listMinLabelWidth) { m_listMinLabelWidth[iLevel - 1] = listMinLabelWidth; }
    int getLevelNumber(const int iLevel) const { return m_iListNumbers[iLevel - 1]; }
    float getListLeftOffset(const int iLevel) const { return m_listLeftOffset[iLevel - 1]; }
    float getListMinLabelWidth(const int iLevel) const { return m_listMinLabelWidth[iLevel - 1]; }
    int getOutlineHash() const { return m_iOutlineHash; }

private:
    //int m_iWPOutlineHash; // we don't use this information in AbiWord, only for id purposes during filtering
    UT_uint32 m_iListIDs[WP6_NUM_LIST_LEVELS];
    int m_iListNumbers[WP6_NUM_LIST_LEVELS];
    FL_ListType m_listTypes[WP6_NUM_LIST_LEVELS];
    float m_listLeftOffset[WP6_NUM_LIST_LEVELS];
    float m_listMinLabelWidth[WP6_NUM_LIST_LEVELS];
    int m_iOutlineHash;
};

class IE_Imp_WordPerfect_Sniffer : public IE_ImpSniffer
{
    friend class IE_Imp;
    friend class IE_Imp_WordPerfect;

public:
    IE_Imp_WordPerfect_Sniffer();
    virtual ~IE_Imp_WordPerfect_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
    virtual UT_Confidence_t recognizeContents (GsfInput * input);
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
    virtual bool getDlgLabels (const char ** szDesc,
			       const char ** szSuffixList,
			       IEFileType * ft);
    virtual UT_Error constructImporter (PD_Document * pDocument,
					IE_Imp ** ppie);
};

class IE_Imp_WordPerfect : public IE_Imp, public librevenge::RVNGTextInterface
{
public:
    IE_Imp_WordPerfect(PD_Document * pDocument);
    virtual ~IE_Imp_WordPerfect();

    virtual void pasteFromBuffer(PD_DocumentRange * pDocRange,
				 UT_uint8 * pData, UT_uint32 lenData, const char * szEncoding = 0);

	virtual void setDocumentMetaData(const librevenge::RVNGPropertyList &propList);

	virtual void startDocument(const librevenge::RVNGPropertyList &propList);
	virtual void endDocument();

	virtual void defineEmbeddedFont(const librevenge::RVNGPropertyList &propList);

	virtual void definePageStyle(const librevenge::RVNGPropertyList &);
	virtual void openPageSpan(const librevenge::RVNGPropertyList &propList);
	virtual void closePageSpan();
	virtual void openHeader(const librevenge::RVNGPropertyList &propList);
	virtual void closeHeader();
	virtual void openFooter(const librevenge::RVNGPropertyList &propList);
	virtual void closeFooter();

	virtual void defineSectionStyle(const librevenge::RVNGPropertyList &);
	virtual void openSection(const librevenge::RVNGPropertyList &propList);
	virtual void closeSection();

	virtual void defineParagraphStyle(const librevenge::RVNGPropertyList &);
	virtual void openParagraph(const librevenge::RVNGPropertyList &propList);
	virtual void closeParagraph();

	virtual void defineCharacterStyle(const librevenge::RVNGPropertyList &);
	virtual void openSpan(const librevenge::RVNGPropertyList &propList);
	virtual void closeSpan();

	virtual void openLink(const librevenge::RVNGPropertyList &propList);
	virtual void closeLink();

	virtual void insertTab();
	virtual void insertText(const librevenge::RVNGString &text);
	virtual void insertSpace();
	virtual void insertLineBreak();
	virtual void insertField(const librevenge::RVNGPropertyList &propList);

	virtual void openOrderedListLevel(const librevenge::RVNGPropertyList &propList);
	virtual void openUnorderedListLevel(const librevenge::RVNGPropertyList &propList);
	virtual void closeOrderedListLevel();
	virtual void closeUnorderedListLevel();
	virtual void openListElement(const librevenge::RVNGPropertyList &propList);
	virtual void closeListElement();

	virtual void openFootnote(const librevenge::RVNGPropertyList &propList);
	virtual void closeFootnote();
	virtual void openEndnote(const librevenge::RVNGPropertyList &propList);
	virtual void closeEndnote();
	virtual void openComment(const librevenge::RVNGPropertyList &propList);
	virtual void closeComment();
	virtual void openTextBox(const librevenge::RVNGPropertyList &propList);
	virtual void closeTextBox();

	virtual void openTable(const librevenge::RVNGPropertyList &propList);
	virtual void openTableRow(const librevenge::RVNGPropertyList &propList);
	virtual void closeTableRow();
	virtual void openTableCell(const librevenge::RVNGPropertyList &propList);
	virtual void closeTableCell();
	virtual void insertCoveredTableCell(const librevenge::RVNGPropertyList &propList);
	virtual void closeTable();

	virtual void openFrame(const librevenge::RVNGPropertyList &propList);
	virtual void closeFrame();

	virtual void openGroup(const librevenge::RVNGPropertyList &propList);
	virtual void closeGroup();

	virtual void defineGraphicStyle(const librevenge::RVNGPropertyList &propList);
	virtual void drawRectangle(const librevenge::RVNGPropertyList &propList);
	virtual void drawEllipse(const librevenge::RVNGPropertyList &propList);
	virtual void drawPolygon(const librevenge::RVNGPropertyList &propList);
	virtual void drawPolyline(const librevenge::RVNGPropertyList &propList);
	virtual void drawPath(const librevenge::RVNGPropertyList &propList);
	virtual void drawConnector(const librevenge::RVNGPropertyList &propList);

	virtual void insertBinaryObject(const librevenge::RVNGPropertyList &propList);
	virtual void insertEquation(const librevenge::RVNGPropertyList &propList);

protected:
	virtual UT_Error _loadFile(GsfInput * input);
    UT_Error							_appendSection(int numColumns, const float, const float);
//    UT_Error							_appendSpan(const guint32 textAttributeBits, const char *fontName, const float fontSize, UT_uint32 listTag = 0);
    UT_Error                            _appendListSpan(UT_uint32 listTag);
//    UT_Error							_appendParagraph(const guint8 paragraphJustification, const guint32 textAttributeBits,
//										 const gchar *fontName, const float fontSize, const float lineSpacing);
    UT_Error							_updateDocumentOrderedListDefinition(ABI_ListDefinition *pListDefinition,
												     int iLevel, const char listType,
												     const UT_UTF8String &sTextBeforeNumber,
												     const UT_UTF8String &sTextAfterNumber,
												     int iStartingNumber);
    UT_Error							_updateDocumentUnorderedListDefinition(ABI_ListDefinition *pListDefinition,
												       int level);
private:
    // section props
    float								m_leftPageMargin;
    float								m_rightPageMargin;
    float								m_leftSectionMargin;
    float								m_rightSectionMargin;
    int									m_sectionColumnsCount;
	UT_sint8							m_headerId; // -1 means no header
	UT_sint8							m_footerId; // -1 means no footer
	UT_uint32							m_nextFreeId;

	// paragraph props
    float								m_topMargin;
    float								m_bottomMargin;
    float								m_leftMarginOffset;
    float								m_rightMarginOffset;
    float								m_textIndent;

    // state handling that libwpd can't account for
    //UT_StringPtrMap						m_listStylesHash;
    ABI_ListDefinition *				m_pCurrentListDefinition;
    bool								m_bParagraphChanged;
    bool								m_bParagraphInSection;
    bool								m_bInSection;
    bool								m_bSectionChanged;
    bool								m_bRequireBlock;

    int							        m_iCurrentListLevel;
    bool								m_bInCell;

	// HACK HACK HACK
	int									m_bHdrFtrOpenCount;
};

#ifdef HAVE_LIBWPS

class IE_Imp_MSWorks_Sniffer : public IE_ImpSniffer
{
    friend class IE_Imp;
    friend class IE_Imp_MSWorks;

public:
    IE_Imp_MSWorks_Sniffer();
    virtual ~IE_Imp_MSWorks_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
    virtual UT_Confidence_t recognizeContents (GsfInput * input);
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
    virtual bool getDlgLabels (const char ** szDesc,
			       const char ** szSuffixList,
			       IEFileType * ft);
    virtual UT_Error constructImporter (PD_Document * pDocument,
					IE_Imp ** ppie);
};

#endif

#endif /* IE_IMP_WP_H */
