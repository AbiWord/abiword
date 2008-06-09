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

#include <OXML_Document.h>
#include <ie_exp_OpenXML_Listener.h>

// External includes
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-output-memory.h>

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
	UT_Error startRunProperties();
	UT_Error finishRunProperties();
	UT_Error startParagraphProperties();
	UT_Error finishParagraphProperties();
	UT_Error setBold();
	UT_Error setItalic();
	UT_Error setUnderline();
	UT_Error setOverline();
	UT_Error setLineThrough();
	UT_Error setSuperscript();
	UT_Error setSubscript();
	UT_Error setTextColor(const gchar* color);
	UT_Error setTextBackgroundColor(const gchar* color);
	UT_Error setTextAlignment(const gchar* alignment);

protected:
    virtual UT_Error _writeDocument(void);
    
private:
	GsfOutfile* root; //.docx file zip root
	GsfOutfile* relsDir; // _rels
	GsfOutfile* wordDir; // word 
	GsfOutput* contentTypesStream; // [Content_Types].xml
	GsfOutput* relStream; // _rels/.rels
	GsfOutput* documentStream; // word/document.xml

	UT_Error startContentTypes();
	UT_Error startRelations();
	UT_Error startMainPart();
	UT_Error finishContentTypes();
	UT_Error finishRelations();
	UT_Error finishMainPart();
	UT_Error writeXmlHeader(GsfOutput* file);

	void _cleanup();
};

#endif //_IE_EXP_OPENXML_H_

