/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000 Frodo Looijaard <frodol@dds.nl>
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

/* This importer was written by Frodo Looijaard <frodol@dds.nl> */

#ifndef IE_IMP_PSION_H
#define IE_IMP_PSION_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_bytebuf.h"

#include <psiconv/data.h>

class PD_Document;

// The importer/reader for Psion Word Files.

class IE_Imp_Psion : public IE_Imp
{
public:
	IE_Imp_Psion(PD_Document * pDocument);
	~IE_Imp_Psion();

	virtual UT_Error	importFile(const char * szFilename);
	virtual void        pasteFromBuffer(PD_DocumentRange * pDocRange,
	                                    unsigned char * pData, 
	                                    UT_uint32 lenData,
										const char * szEncoding = 0);

protected:
		bool			getCharacterAttributes(psiconv_character_layout layout, UT_ByteBuf *props);
		bool			getParagraphAttributes(psiconv_paragraph_layout layout, UT_ByteBuf *props);
		bool			applyCharacterAttributes(psiconv_character_layout layout);
		bool			applyParagraphAttributes(psiconv_paragraph_layout layout,const XML_Char *stylename);
		bool			applyPageAttributes(psiconv_page_layout_section layout);
		bool			prepareCharacters(char *input, int length, 
						                  UT_GrowBuf *gbBlock);
		UT_Error		readParagraphs(psiconv_text_and_layout psiontext, psiconv_word_styles_section style_sec);
		bool 		applyStyles(psiconv_word_styles_section style_sec);

	virtual	UT_Error	parseFile(psiconv_file psionfile) = 0;
	const XML_Char *listid;
};


class IE_Imp_Psion_Word_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_Psion_Word_Sniffer() {}
	virtual ~IE_Imp_Psion_Word_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_Psion_TextEd_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_Psion_TextEd_Sniffer() {}
	virtual ~IE_Imp_Psion_TextEd_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_Psion_Word : public IE_Imp_Psion
{
public:
	IE_Imp_Psion_Word(PD_Document * pDocument);
	~IE_Imp_Psion_Word();


protected:
	virtual	UT_Error	parseFile(psiconv_file psionfile);
};

class IE_Imp_Psion_TextEd : public IE_Imp_Psion
{
public:
	IE_Imp_Psion_TextEd(PD_Document * pDocument);
	~IE_Imp_Psion_TextEd();

protected:
	virtual	UT_Error	parseFile(psiconv_file psionfile);
};



#endif /* IE_IMP_PSION_H */
