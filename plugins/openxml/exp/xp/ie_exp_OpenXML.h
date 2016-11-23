/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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


#ifndef _IE_EXP_OPENXML_H_
#define _IE_EXP_OPENXML_H_

// AbiWord includes
#include <ie_exp.h>
#include <ut_debugmsg.h>
#include <ut_types.h>
#include <ut_misc.h>
#include <ut_assert.h>
#include <ut_string_class.h>
#include <pp_Property.h>

#include <OXML_Document.h>
#include <ie_exp_OpenXML_Listener.h>

// External includes
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-output-memory.h>
#include <string>
#include <map>

//target streams
#define TARGET_DOCUMENT 0
#define TARGET_STYLES 1
#define TARGET_DOCUMENT_RELATION 2
#define TARGET_RELATION 3
#define TARGET_CONTENT 4
#define TARGET_NUMBERING 5
#define TARGET_HEADER 6
#define TARGET_FOOTER 7
#define TARGET_SETTINGS 8
#define TARGET_FOOTNOTE 9
#define TARGET_ENDNOTE 10

class OXML_Document;

/**
 * Class used to export OpenXML files
 */
class IE_Exp_OpenXML : public IE_Exp
{
public:
	IE_Exp_OpenXML (PD_Document * pDocument);
	virtual ~IE_Exp_OpenXML ();
	UT_Error startDocument();
	UT_Error finishDocument();
	UT_Error startSection();
	UT_Error finishSection();
	UT_Error startSectionProperties();
	UT_Error finishSectionProperties();
	UT_Error startParagraph(int target);
	UT_Error finishParagraph(int target);
	UT_Error startText(int target);
	UT_Error writeText(int target, const UT_UCS4Char* text, bool list);
	UT_Error finishText(int target);
	UT_Error startMath();
	UT_Error writeMath(const char* omml);
	UT_Error finishMath();
	UT_Error startRun(int target);
	UT_Error finishRun(int target);
	UT_Error startRunProperties(int target);
	UT_Error finishRunProperties(int target);
	UT_Error startParagraphProperties(int target);
	UT_Error finishParagraphProperties(int target);
	UT_Error startCellProperties(int target);
	UT_Error finishCellProperties(int target);
	UT_Error startStyle(const std::string& name, const std::string& basedon, const std::string& followedby, const std::string& type);
	UT_Error finishStyle();
	UT_Error startDocumentDefaultProperties();
	UT_Error finishDocumentDefaultProperties();
	UT_Error startRunDefaultProperties();
	UT_Error finishRunDefaultProperties();
	UT_Error startParagraphDefaultProperties();
	UT_Error finishParagraphDefaultProperties();
	UT_Error startTable();
	UT_Error finishTable();
	UT_Error startTableProperties(int target);
	UT_Error finishTableProperties(int target);
	UT_Error startTableBorderProperties(int target);
	UT_Error finishTableBorderProperties(int target);
	UT_Error startCellBorderProperties(int target);
	UT_Error finishCellBorderProperties(int target);
	UT_Error startListProperties(int target);
	UT_Error finishListProperties(int target);
	UT_Error startAbstractNumbering(int target, UT_uint32 id);
	UT_Error finishAbstractNumbering(int target);
	UT_Error startTextBox(int target, const gchar* id);
	UT_Error finishTextBox(int target);
	UT_Error startTextBoxProperties(int target);
	UT_Error finishTextBoxProperties(int target);
	UT_Error startTextBoxContent(int target);
	UT_Error finishTextBoxContent(int target);
	UT_Error startNumbering(int target, UT_uint32 id);
	UT_Error finishNumbering(int target);
	UT_Error startNumberingLevel(int target, UT_uint32 level);
	UT_Error finishNumberingLevel(int target);
	UT_Error startRow();
	UT_Error finishRow();
	UT_Error startRowProperties(int target);
	UT_Error finishRowProperties(int target);
	UT_Error startCell();
	UT_Error finishCell();
	UT_Error startTableGrid(int target);
	UT_Error finishTableGrid(int target);
	UT_Error startExternalHyperlink(const gchar* id);
	UT_Error startInternalHyperlink(const gchar* anchor);
	UT_Error setHeaderRelation(const char* relId, const char* headerId);
	UT_Error setFooterRelation(const char* relId, const char* footerId);
	UT_Error finishHyperlink();
	UT_Error startBookmark(const gchar* id, const gchar* name);
	UT_Error finishBookmark(const gchar* id);
	UT_Error startFootnote(const gchar* id);
	UT_Error finishFootnote();
	UT_Error startEndnote(const gchar* id);
	UT_Error finishEndnote();
	UT_Error setBold(int target);
	UT_Error setItalic(int target);
	UT_Error setUnderline(int target);
	UT_Error setOverline();
	UT_Error setLineThrough(int target);
	UT_Error setSuperscript(int target);
	UT_Error setSubscript(int target);
	UT_Error setTextColor(int target, const gchar* color);
	UT_Error setBackgroundColor(int target, const gchar* color);
	UT_Error setTextAlignment(int target, const gchar* alignment);
	UT_Error setTextIndentation(int target, const gchar* indentation);
	UT_Error setParagraphStyle(int target, const gchar* style);
	UT_Error setParagraphLeftMargin(int target, const gchar* margin);
	UT_Error setParagraphRightMargin(int target, const gchar* margin);
	UT_Error setParagraphTopMargin(int target, const gchar* margin);
	UT_Error setParagraphBottomMargin(int target, const gchar* margin);
	UT_Error setLineHeight(int target, const gchar* height);
	UT_Error setTabstops(int target, const gchar* tabstops);
	UT_Error setFontSize(int target, const gchar* size);
	UT_Error setFontFamily(int target, const gchar* family);
	UT_Error setTextDirection(int target, const gchar* direction);
	UT_Error setWidows(int target, const gchar* widows);
	UT_Error setGridSpan(int target, UT_sint32 hspan);
	UT_Error setVerticalMerge(int target, const char* vmerge);
	UT_Error setTableBorder(int target, const char* border, const char* type, const char* color, const char* size);
	UT_Error setGridCol(int target, const char* column);
	UT_Error setColumnWidth(int target, const char* width);
	UT_Error setRowHeight(int target, const char* height);
	UT_Error setListLevel(int target, const char* level);
	UT_Error setListFormat(int target, const char* format);
	UT_Error setListStartValue(int target, UT_uint32 startValue);
	UT_Error setListLevelText(int target, const char* text);
	UT_Error setListType(int target, const char* type);
	UT_Error setAbstractNumberingId(int target, UT_uint32 id);
	UT_Error setNumberingFormat(int target, const char* format);
	UT_Error setMultilevelType(int target, const char* type);
	UT_Error setHyperlinkRelation(int target, const char* id, const char* addr, const char* mode);
	UT_Error setImage(const char* id, const char* relId, const char* filename, const char* width, const char* height);
	UT_Error setPositionedImage(const char* id, const char* relId, const char* filename, const char* width, const char* height, const char* xpos, const char* ypos, const char* wrapMode);
	UT_Error setImageRelation(const char* filename, const char* id);
	UT_Error writeImage(const char* filename, const UT_ConstByteBufPtr & data);
	UT_Error setSimpleField(int target, const char* instr, const char* value);
	UT_Error setHeaderReference(const char* id, const char* type);
	UT_Error setFooterReference(const char* id, const char* type);
	UT_Error setFootnoteReference(const char* id);
	UT_Error setFootnoteRef();
	UT_Error setEndnoteReference(const char* id);
	UT_Error setEndnoteRef();
	UT_Error startHeaderStream(const char* id);
	UT_Error finishHeaderStream();
	UT_Error startFooterStream(const char* id);
	UT_Error finishFooterStream();
	UT_Error setTitlePage();
	UT_Error setEvenAndOddHeaders();
	UT_Error setColumns(int target, const gchar* num, const gchar* sep);
	UT_Error setContinuousSection(int target);
	UT_Error setPageBreak(int target);
	UT_Error setPageSize(int target, const char* width, const char* height, const char* orientation);
	UT_Error setPageMargins(int target, const char* top, const char* left, const char* right, const char* bottom);
	UT_Error setLanguage(int target, const gchar* lang);
	UT_Error setNoProof(int target);
	UT_Error setTextBoxWidth(int target, const gchar* width);
	UT_Error setTextBoxHeight(int target, const gchar* height);
	PD_Document* getDoc() {return m_pDoc;};

protected:
    virtual UT_Error _writeDocument(void);

private:
	PD_Document* m_pDoc;
	GsfOutfile* root; //.docx file zip root
	GsfOutfile* relsDir; // _rels
	GsfOutfile* wordDir; // word
	GsfOutfile* wordRelsDir; // word/_rels
	GsfOutfile* wordMediaDir; // word/media
	GsfOutput* contentTypesStream; // [Content_Types].xml
	GsfOutput* relStream; // _rels/.rels
	GsfOutput* wordRelStream; // word/_rels/document.xml.rels
	GsfOutput* documentStream; // word/document.xml
	GsfOutput* settingsStream; // word/settings.xml
	GsfOutput* stylesStream; // word/styles.xml
	GsfOutput* numberingStream; // word/numbering.xml
	GsfOutput* headerStream; //word/headerXX.xml
	GsfOutput* footerStream; //word/footerXX.xml
	GsfOutput* footnoteStream; //word/footnotes.xml
	GsfOutput* endnoteStream; //word/endnotes.xml
	std::map<std::string, GsfOutput*> mediaStreams; // all image filename, stream pairs
	std::map<std::string, GsfOutput*> headerStreams; //all header id, stream pairs
	std::map<std::string, GsfOutput*> footerStreams; //all footer id, stream pairs

	bool isOverline;

	UT_Error startNumbering();
	UT_Error startStyles();
	UT_Error startContentTypes();
	UT_Error startRelations();
	UT_Error startWordRelations();
	UT_Error startWordMedia();
	UT_Error startMainPart();
	UT_Error startHeaders();
	UT_Error startFooters();
	UT_Error startSettings();
	UT_Error startFootnotes();
	UT_Error startEndnotes();
	UT_Error finishNumbering();
	UT_Error finishStyles();
	UT_Error finishContentTypes();
	UT_Error finishRelations();
	UT_Error finishWordRelations();
	UT_Error finishWordMedia();
	UT_Error finishMainPart();
	UT_Error finishHeaders();
	UT_Error finishFooters();
	UT_Error finishSettings();
	UT_Error finishFootnotes();
	UT_Error finishEndnotes();
	UT_Error writeXmlHeader(GsfOutput* file);

	const gchar* convertToPoints(const gchar* str);
	const gchar* convertToTwips(const gchar* str);
	const gchar* convertToPositiveTwips(const gchar* str);
	const gchar* convertToPositiveEmus(const gchar* str);
	bool isNegativeQuantity(const gchar* quantity);
	const gchar* convertToLines(const gchar* str);
	const gchar* computeFontSize(const gchar* str);
	const gchar* computeBorderWidth(const gchar* str);

	GsfOutput* getTargetStream(int target);
	UT_Error writeTargetStream(int target, const char* str);

	bool isListBullet(const char* str);

	void _cleanup();
};

#endif //_IE_EXP_OPENXML_H_

