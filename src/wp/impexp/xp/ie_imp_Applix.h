/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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


#ifndef IE_IMP_APPLIX_H
#define IE_IMP_APPLIX_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_mbtowc.h"
#include "ut_growbuf.h"

class PD_Document;
class UT_ByteBuf;

// The importer/reader for Applix Word files

class IE_Imp_Applix_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_Applix_Sniffer() {}
	virtual ~IE_Imp_Applix_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_Applix : public IE_Imp
{
public:
	IE_Imp_Applix(PD_Document * pDocument);
	~IE_Imp_Applix();

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
						unsigned char * pData, 
						UT_uint32 lenData,
						const char * szEncoding = 0);
	
protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);

 private:
	bool m_bLastWasP;
	bool m_bInT;
	UT_GrowBuf m_textBuf;
	UT_Mbtowc		m_mbtowc;
	
	// the applix tags that i know about && maybe handle
	typedef enum {
	    APPLIX_T,
	    GLOBALS_T,
	    START_STYLES_T,
	    END_STYLES_T,
		STYLE_T,
	    COLOR_T,
	    START_FLOW_T,			  
	    END_FLOW_T,
	    WP400_T,
	    TEXT_T,
		PAGE_BREAK_T,
	    PARA_T,
	    START_VARS_T,
	    END_VARS_T,
	    VARIABLE_T,
	    END_DOCUMENT_T,
		OBJECT_T,
		PICTURE_T,
		SECTION_T,
		MARKER_T,
		START_FIELD_T,
		END_FIELD_T,
		FIELD_VALUE_T,
	    NOT_A_TAG, 
	    tag_Unknown
	} Applix_tag_t;

	// container
	typedef enum {
		axCtnNone,
		axCtnText,
		axCtnField,
		axCtnGlossary,
	} Applix_content_t;
	
	// context in the file
	// because T tag have different meanings.
	typedef enum {
		axCtxNone,
		axCtxDef,
		axCtxFlow,
		axCtxHdrFtr,
		axCtxFootnote,
		axCtxVar
	} Applix_context_t;

	Applix_context_t        m_axContext;

	void                    _dispatchTag (Applix_tag_t tag, const char *buf, size_t len);
	// tokenizer helpers
	typedef struct {
	    char * name;
	    Applix_tag_t tag;
	} Applix_mapping_t;
	static Applix_mapping_t axwords[];
	static Applix_tag_t     s_name_2_tag (const char *name, size_t n);
	static Applix_tag_t     s_getTagName(const char *str, size_t len);
	static short            s_8bitsToUCS (const char *str, size_t len, UT_UCSChar * c);
	static short            s_16bitsToUCS (const char *str, size_t len, UT_UCSChar * c);
	static short            s_decodeToUCS (const char *str, size_t len, UT_UCSChar * c);
	bool                    _applixGetLine (UT_ByteBuf* pBuf, FILE *fp);
	void                    _applixDecodeText (const char * buf, size_t len);
	void                    _applixNewPara (const char * buf, size_t len);
	void                    _applixPageBreak (const char * buf, size_t len);
};

#endif /* IE_IMP_APPLIX_H */









