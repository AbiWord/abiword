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

/* See bug 1764
 * This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef IE_IMP_WP_H
#define IE_IMP_WP_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_growbuf.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"

// The importer/reader for WordPerfect 6 documents.

#define WP_WORDPERFECT6789_EXPECTED_MAJOR_VERSION 2
#define WP_WORDPERFECT_DOCUMENT_FILE_TYPE 10
#define WP_FONT_TABLE_SIZE_GUESS 10

#define WP_HEADER_PRODUCT_TYPE_OFFSET 8
#define WP_HEADER_FILE_TYPE_OFFSET 9
#define WP_HEADER_MAJOR_VERSION_OFFSET 10
#define WP_HEADER_MINOR_VERSION_OFFSET 11
#define WP_HEADER_ENCRYPTION_POSITION 12
#define WP_HEADER_DOCUMENT_POINTER_POSITION 4
#define WP_HEADER_DOCUMENT_SIZE_POSITION 20
#define WP_HEADER_INDEX_HEADER_POSITION 14

#define WP_INDEX_HEADER_NUM_INDICES_POSITION 2
#define WP_INDEX_HEADER_INDICES_POSITION 14
#define WP_INDEX_HEADER_FONT_TYPEFACE_DESCRIPTOR_POOL 32
#define WP_INDEX_HEADER_DESIRED_FONT_DESCRIPTOR_POOL 85
#define WP_INDEX_HEADER_GRAPHICS_BOX_STYLE 65
#define WP_INDEX_HEADER_ELEMENT_CHILD_PACKET_BIT 1

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
#define WP_TOP_CROSSREFERENCE_GROUP 215 // (0xd5)
#define WP_TOP_HEADER_FOOTER_GROUP 214 // (0xd6)
#define WP_TOP_FOOTENDNOTE_GROUP 215 // (0xd7)
#define WP_TOP_SET_NUMBER_GROUP 216  // (0xd8)
#define WP_TOP_NUMBERING_METHOD_GROUP 217 // (0xd9)
#define WP_TOP_DISPLAY_NUMBER_REFERENCE_GROUP 218 // (0xda)
#define WP_TOP_INCREMENT_NUMBER_GROUP 219 // (0xdb)
#define WP_TOP_DECREMENT_NUMBER_GROUP 220 // (0xdc)
#define WP_TOP_STYLE_GROUP 221 // (0xdd)
#define WP_TOP_MERGE_GROUP 222 // (0xde)
#define WP_TOP_BOX_GROUP 223 // (0xdf)
#define WP_TOP_TAB_GROUP 224 // (0xe0)
#define WP_TOP_PLATFORM_GROUP 225 // (0xe1)
#define WP_TOP_FORMATTER_GROUP 226 // (0xe2)
#define WP_TOP_EXTENDED_CHARACTER 240// (0xf0)
#define WP_TOP_UNDO_GROUP 241 // (0xf1)
#define WP_TOP_ATTRIBUTE_ON 242 // (0xf2)
#define WP_TOP_ATTRIBUTE_OFF 243 // (0xf3)

#define WP_UNDO_GROUP_SIZE 5
#define WP_ATTRIBUTE_ON_GROUP_SIZE 3
#define WP_ATTRIBUTE_OFF_GROUP_SIZE 3

#define WP_CHARACTER_GROUP_FONT_FACE_CHANGE 26 // 0x1a
#define WP_CHARACTER_GROUP_FONT_SIZE_CHANGE 27 // 0x1b

#define WP_PARAGRAPH_GROUP_JUSTIFICATION 5 // 0x05
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT 0
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL 1
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_CENTER 2
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RIGHT 3
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL_ALL_LINES 4
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RESERVED 5

#define WP_BOX_GROUP_NUM_RESERVED_BYTES 14
#define WP_BOX_GROUP_OVERRIDE_FLAGS_BOX_CONTENT 8192

// Character properties
class ABI_EXPORT WordPerfectTextAttributes
{
 public:

   WordPerfectTextAttributes();
   ~WordPerfectTextAttributes() {}

   // text attributes
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
   // font stuff
   UT_uint16 m_fontSize;
};

struct ABI_EXPORT WordPerfectFontDescriptor
{
   int m_packetID;

   UT_uint16 m_characterWidth;
   UT_uint16 m_ascenderHeight;
   UT_uint16 m_xHeight;
   UT_uint16 m_descenderHeight;
   UT_uint16 m_italicsAdjust;
   UT_Byte m_primaryFamilyId; // family id's are supposed to be one unified element, but I split them up to ease parsing
   UT_Byte m_primaryFamilyMemberId;
   
   UT_Byte m_scriptingSystem;
   UT_Byte m_primaryCharacterSet;
   UT_Byte m_width;
   UT_Byte m_weight; 
   UT_Byte m_attributes;
   UT_Byte m_generalCharacteristics;
   UT_Byte m_classification;
   UT_Byte m_fill; // fill byte
   UT_Byte m_fontType;
   UT_Byte m_fontSourceFileType;
   char *m_fontName; // is this platform indep?
};

// Paragraph Properties (set using group WP_TOP_PARAGrAPH_GROUP)
struct ABI_EXPORT WordPerfectParagraphProperties
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

struct WordPerfectHeaderPacket
{
   WordPerfectHeaderPacket(unsigned int PID, unsigned char type, long packetPosition, bool hasChildren);
   unsigned int m_PID;
   unsigned char m_type;
   long m_packetPosition;
   bool m_hasChildren;
};

class ABI_EXPORT IE_Imp_WordPerfect_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_WordPerfect;

public:
	IE_Imp_WordPerfect_Sniffer() {}
	virtual ~IE_Imp_WordPerfect_Sniffer() {}

	virtual UT_Confidence_t recognizeContents (const char * szBuf,
					    UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

class ABI_EXPORT IE_Imp_WordPerfect : public IE_Imp
{
public:
	IE_Imp_WordPerfect(PD_Document * pDocument);
	~IE_Imp_WordPerfect(); 

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
						unsigned char * pData, UT_uint32 lenData, const char * szEncoding = 0);
 protected:
   UT_Error _parseHeader();
   UT_Error _parseIndexHeader();
   UT_Error _parseFontDescriptorPacket(int packetID, UT_uint32 dataPacketSize, UT_uint32 dataPointer);
   UT_Error _handleBoxGroupTemplate(int boxGroupTemplatePID);
   UT_Error _handleBoxGroupContent(int boxContentPID);
   UT_Error _handleGraphicsData(int graphicPID);
   UT_Error _parseDocument();
   UT_Error _insertSpace();
   UT_Error _insertHyphen();
   UT_Error _handleHardEndOfLine();
   UT_Error _handleEndOfLineGroup();
   UT_Error _handlePageGroup();
   UT_Error _handleColumnGroup();
   UT_Error _handleParagraphGroup();
   UT_Error _handleParagraphGroupJustification();
   UT_Error _handleSetNumberGroup();
   UT_Error _handleNumberingMethodGroup();
   UT_Error _handleDisplayNumberReferenceGroup();
   UT_Error _handleIncrementNumberGroup();
   UT_Error _handleDecrementNumberGroup();
   UT_Error _handleStyleGroup();
   UT_Error _handleMergeGroup();
   UT_Error _handleBoxGroup();
   UT_Error _handleTabGroup();
   UT_Error _handlePlatformGroup();
   UT_Error _handleFormatterGroup();
   UT_Error _handleCharacterGroup();
   UT_Error _handleCrossReferenceGroup();
   UT_Error _handleHeaderFooterGroup();
   UT_Error _handleFootEndNoteGroup();
   UT_Error _handleFontFaceChange();
   UT_Error _handleFontSizeChange();
   UT_Error _handleExtendedCharacter();
   UT_Error _handleUndo();
   UT_Error _handleAttributeOn();
   UT_Error _handleAttributeOff();
   UT_Error _handleVariableGroupHeader(long &startPosition, unsigned char &subGroup, UT_uint16 &size, unsigned char &flags);
   UT_Error _skipGroup(long startPosition, UT_uint16 groupSize);
   UT_Error _appendCurrentTextProperties();
   UT_Error _appendCurrentParagraphProperties();
   UT_Error _flushText();
   UT_Error _flushParagraph();
   
   bool _appendSection( const XML_Char ** props = NULL ) ;

 private:
   FILE *m_importFile;
   UT_uint32 m_documentEnd;
   UT_uint32 m_documentPointer;
   UT_uint16 m_indexPointer;
   bool m_undoOn;
   bool m_bParagraphChanged;
   bool m_bParagraphExists;
   bool m_bInSection;
   UT_Mbtowc m_Mbtowc;
   UT_GrowBuf m_textBuf;
   UT_Vector m_fontDescriptorList;
   UT_Vector m_headerPacketList;
   UT_Vector m_wordPerfectDispatchBytes;
   WordPerfectTextAttributes m_textAttributes;
   WordPerfectParagraphProperties m_paragraphProperties;
};

struct ABI_EXPORT WordPerfectByteTag
{
   WordPerfectByteTag( unsigned char byte, UT_Error (IE_Imp_WordPerfect::*func)() );   
   unsigned char m_byte;
   UT_Error (IE_Imp_WordPerfect::*m_func) () ;
}; 

#endif /* IE_IMP_WP_H */
