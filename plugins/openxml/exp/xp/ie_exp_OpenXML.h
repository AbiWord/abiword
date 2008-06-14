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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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

//target streams
#define TARGET_STYLES 1
#define TARGET_DOCUMENT 2
#define TARGET_DOCUMENT_RELATION 3
#define TARGET_RELATION 4
#define TARGET_CONTENT 5

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
	UT_Error startParagraph();
	UT_Error finishParagraph();
	UT_Error startText();
	UT_Error writeText(const char* text);
	UT_Error finishText();
	UT_Error startRun();
	UT_Error finishRun();
	UT_Error startRunProperties(int target);
	UT_Error finishRunProperties(int target);
	UT_Error startParagraphProperties(int target);
	UT_Error finishParagraphProperties(int target);
	UT_Error startStyle(std::string name, std::string basedon, std::string followedby);
	UT_Error finishStyle();
	UT_Error writeDefaultStyle();
	UT_Error setBold(int target);
	UT_Error setItalic(int target);
	UT_Error setUnderline(int target);
	UT_Error setOverline(int target);
	UT_Error setLineThrough(int target);
	UT_Error setSuperscript(int target);
	UT_Error setSubscript(int target);
	UT_Error setTextColor(int target, const gchar* color);
	UT_Error setTextBackgroundColor(int target, const gchar* color);
	UT_Error setTextAlignment(int target, const gchar* alignment);
	UT_Error setTextIndentation(int target, const gchar* indentation);
	UT_Error setParagraphStyle(int target, const gchar* style);
	UT_Error setParagraphLeftMargin(int target, const gchar* margin);
	UT_Error setParagraphRightMargin(int target, const gchar* margin);
	UT_Error setParagraphTopMargin(int target, const gchar* margin);
	UT_Error setParagraphBottomMargin(int target, const gchar* margin);
	UT_Error setLineHeight(int target, const gchar* height);
	UT_Error setFontSize(int target, const gchar* size);

protected:
    virtual UT_Error _writeDocument(void);
    
private:
	GsfOutfile* root; //.docx file zip root
	GsfOutfile* relsDir; // _rels
	GsfOutfile* wordDir; // word 
	GsfOutfile* wordRelsDir; // word/_rels
	GsfOutput* contentTypesStream; // [Content_Types].xml
	GsfOutput* relStream; // _rels/.rels
	GsfOutput* wordRelStream; // word/_rels/document.xml.rels
	GsfOutput* documentStream; // word/document.xml
	GsfOutput* stylesStream; // word/styles.xml

	UT_Error startStyles();
	UT_Error startContentTypes();
	UT_Error startRelations();
	UT_Error startWordRelations();
	UT_Error startMainPart();
	UT_Error finishStyles();
	UT_Error finishContentTypes();
	UT_Error finishRelations();
	UT_Error finishWordRelations();
	UT_Error finishMainPart();
	UT_Error writeXmlHeader(GsfOutput* file);

	const gchar* convertToTwips(const gchar* str);
	const gchar* convertToPositiveTwips(const gchar* str);
	bool isNegativeQuantity(const gchar* quantity);
	const gchar* convertToLines(const gchar* str);
	const gchar* computeFontSize(const gchar* str);

	GsfOutput* getTargetStream(int target);
	UT_Error writeTargetStream(int target, const char* str);

	void _cleanup();
};

#endif //_IE_EXP_OPENXML_H_

