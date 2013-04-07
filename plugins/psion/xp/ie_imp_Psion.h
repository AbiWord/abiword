/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000,2004 Frodo Looijaard <frodol@dds.nl>
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

/* This importer is written by Frodo Looijaard <frodol@dds.nl> */

#ifndef IE_IMP_PSION_H
#define IE_IMP_PSION_H

#include "ie_imp.h"
#include "ie_impexp_Psion.h"
#include "pd_Document.h"

#include <psiconv/data.h>


// The importer/reader for Psion Word and TextEd Files.


class IE_Imp_Psion_Sniffer : public IE_ImpSniffer
{
public:
	IE_Imp_Psion_Sniffer(const char * _name): IE_ImpSniffer(_name) {}
	virtual ~IE_Imp_Psion_Sniffer() {}
protected:
	UT_Confidence_t checkContents (const char *szBuf, UT_uint32 iNumbytes,
	                               psiconv_file_type_t filetype);
};

class IE_Imp_Psion_Word_Sniffer : public IE_Imp_Psion_Sniffer
{
public:
	IE_Imp_Psion_Word_Sniffer(const char * _name): IE_Imp_Psion_Sniffer(_name) {}
	virtual ~IE_Imp_Psion_Word_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									           UT_uint32 iNumbytes);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

class IE_Imp_Psion_TextEd_Sniffer :
	                                                public IE_Imp_Psion_Sniffer
{
public:
	IE_Imp_Psion_TextEd_Sniffer(const char * _name):
                                                  IE_Imp_Psion_Sniffer(_name) {}
	virtual ~IE_Imp_Psion_TextEd_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									           UT_uint32 iNumbytes);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

class IE_Imp_Psion : public IE_Imp
{
public:
	IE_Imp_Psion(PD_Document * pDocument): IE_Imp(pDocument),list(false) {}
	~IE_Imp_Psion() {}

protected:
	virtual UT_Error _loadFile(GsfInput * input);

	UT_Error getCharacterAttributes(const psiconv_character_layout layout,
                                UT_UTF8String &props);
	UT_Error getParagraphAttributes(const psiconv_paragraph_layout layout,
                                UT_UTF8String &props);
	UT_Error applyCharacterAttributes(const psiconv_character_layout layout);
	UT_Error applyParagraphAttributes(const psiconv_paragraph_layout layout,
                                  const gchar *stylename);
	UT_Error applyPageAttributes(const psiconv_page_layout_section layout,
                                 bool &with_header, bool &with_footer);
	UT_Error prepareCharacters(const psiconv_ucs2 *input, int length,
                           UT_UCS4String &text);
	UT_Error readParagraphs(const psiconv_text_and_layout psiontext,
                            const psiconv_word_styles_section style_sec);
	UT_Error applyStyles(const psiconv_word_styles_section style_sec);
	UT_Error processHeaderFooter(const psiconv_page_layout_section layout,
                                           bool with_header, bool with_footer);
    UT_Error insertImage(const psiconv_in_line_layout in_line);
	UT_Error insertObject(const psiconv_in_line_layout in_line);

	virtual	UT_Error parseFile(const psiconv_file psionfile) = 0;

 private:
	bool list;
};

class IE_Imp_Psion_Word : public IE_Imp_Psion
{
public:
	IE_Imp_Psion_Word(PD_Document * pDocument): IE_Imp_Psion(pDocument) {}
	~IE_Imp_Psion_Word() {}

protected:
	virtual	UT_Error parseFile(const psiconv_file psionfile);
};

class IE_Imp_Psion_TextEd : public IE_Imp_Psion
{
public:
	IE_Imp_Psion_TextEd(PD_Document * pDocument): IE_Imp_Psion(pDocument) {}
	~IE_Imp_Psion_TextEd() {}

protected:
	virtual	UT_Error parseFile(const psiconv_file psionfile);
};

#endif /* IE_IMP_PSION_H */
