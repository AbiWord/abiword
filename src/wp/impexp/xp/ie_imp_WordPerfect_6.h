/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 William Lachance (wlach@interlog.com)
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


#ifndef IE_IMP_WP6_H
#define IE_IMP_WP6_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_growbuf.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"

// The importer/reader for WordPerfect 6 documents.

#define WP_TOP_SOFT_SPACE 128
#define WP_TOP_HARD_HYPHEN 132 // (0x84)
#define WP_TOP_DORMANT_HARD_RETURN 135 // (0x87)
#define WP_TOP_HARD_EOL 204
#define WP_TOP_SOFT_EOL 207
#define WP_TOP_EOL_GROUP 208 // (0xd0)
#define WP_TOP_PAGE_GROUP 209 // (0xd1)
#define WP_TOP_COLUMN_GROUP 210 // (0xd2)
#define WP_TOP_PARAGRAPH_GROUP 211 // (0xd3)
#define WP_TOP_CHARACTER_GROUP 212 // (0xd4)
#define WP_TOP_FOOTENDNOTE_GROUP 215 // (0xd7)
#define WP_TOP_STYLE_GROUP 221 // (0xdd)
#define WP_TOP_TAB_GROUP 224 // (0xe0)
#define WP_TOP_EXTENDED_CHARACTER 240// (0xf0)
#define WP_TOP_UNDO 241 // (0xf1)
#define WP_TOP_ATTRIBUTE_ON 242 // (0xf2)
#define WP_TOP_ATTRIBUTE_OFF 243 // (0xf3)

#define WP_PARAGRAPH_GROUP_JUSTIFICATION 5 // 0x05
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT 0
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL 1
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_CENTER 2
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RIGHT 3
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL_ALL_LINES 4
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RESERVED 5

// Character properties
struct WordPerfectTextAttributes
{
   WordPerfectTextAttributes();
   bool	m_extraLarge;
   bool	m_veryLarge;
   bool	m_large;
   bool m_smallPrint;
   bool	m_finePrint;
   bool m_superScript;
   bool m_subScript;
   bool m_outline;
   bool m_italics;
   bool m_shadow;
   bool m_redLine;
   bool m_bold;
   bool m_strikeOut;
   bool m_underLine;
   bool m_smallCaps;
   bool m_Blink;
   bool m_reverseVideo;     
};

// Paragraph Properties (set using group WP_TOP_PARAGrAPH_GROUP)
struct WordPerfectParagraphProperties
{

   WordPerfectParagraphProperties();
   //float m_lineHeight; // originally in "signed WPU" 0.0=default
   unsigned int m_lineSpacing; // d301
   // (TODO: leftHotZone) d302
   // TODO: rightHotZone) d303
   // (TODO: tab set definitions 0xd304)
   enum ParagraphJustification { left, full, center, right, fullAllLines, reserved };
   ParagraphJustification m_justificationMode;
   // (TODO hyphenationOn 0xd306
   // (TODO:leadingAdjustment) originally in "signed WPU" d307
   // (TODO: generated text?! 0xd308-0xd309)
   // (TODO: spacing after paragraph 0xd30a)
   // (TODO: indent first line of paragraph) 0xd30b
   // (TODO: left margin adjustment) 0xd30c
   // (TODO: right margin adjustment) 0xd30d
   // (TODO: outline define) 0xd30e
   // (TODO: paragraph border) 0xd30f
   // (TODO: define math columns) 0xd310
   // (TODO: math on/off) 0xd311
   // (TODO: line numbering def'n) 0xd312
   // (TODO: force odd/even/new) 0xd313
   // (TODO: endnotes print here) 0xd314
   // (TODO: endnotes print here end) 0xd315
   // (TODO: define marked text) 0xd316
   // (TODO: define drop cap) 0xd317
   // (TODO: paragraph text direction) 0xd318
   // (TODO: asian mapping) 0xd319
   // (TODO: paragraph character count) 0xd31a
};

class IE_Imp_WordPerfect_6_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_WordPerfect_6;

public:
	IE_Imp_WordPerfect_6_Sniffer() {}
	virtual ~IE_Imp_WordPerfect_6_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

class IE_Imp_WordPerfect_6 : public IE_Imp
{
public:
	IE_Imp_WordPerfect_6(PD_Document * pDocument);
	~IE_Imp_WordPerfect_6() {}

	virtual UT_Error	importFile(const char * szFilename);

 protected:
   UT_Error _parseHeader();
   UT_Error _parseDocument();
   UT_Error _handleHardEndOfLine();
   UT_Error _handleEndOfLineGroup();
   UT_Error _handlePageGroup();
   UT_Error _handleColumnGroup();
   UT_Error _handleParagraphGroup();
   UT_Error _handleParagraphGroupJustification();
   UT_Error _handleStyleGroup();
   UT_Error _handleTabGroup();
   UT_Error _handleCharacterGroup();
   UT_Error _handleFootEndNoteGroup();
   UT_Error _handleExtendedCharacter();
   UT_Error _handleAttribute(bool attributeOn);
   UT_Error _skipGroup(int groupByte);
   UT_Error _appendCurrentTextProperties();
   UT_Error _appendCurrentParagraphProperties();
   UT_Error _flushText();
   UT_Error _flushParagraph();
   
 private:
   FILE *m_importFile;
   UT_uint32 m_documentEnd;
   UT_uint32 m_documentPointer;

   bool m_firstParagraph; 
   UT_Mbtowc m_Mbtowc;
   UT_GrowBuf m_textBuf;
   WordPerfectTextAttributes m_textAttributes;
   WordPerfectParagraphProperties m_paragraphProperties;
};

#endif /* IE_IMP_WP6_H */
