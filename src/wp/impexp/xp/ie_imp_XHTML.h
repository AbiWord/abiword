/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
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

#ifndef IE_IMP_XHTML_1_H
#define IE_IMP_XHTML_1_H

#include "ie_imp_XML.h"
#include "ut_stack.h"

/* NOTE: I'm trying to keep the code similar across versions,
 *       and therefore features are enabled/disabled here:
 */

/* Define if the base unicode char is UCS-4
 */
#define XHTML_UCS4

/* Define if the sniffers need to pass export name to parent
 */
#define XHTML_NAMED_CONSTRUCTORS

/* Define if the tables are supported
 */
#define XHTML_TABLES_SUPPORTED

/* Define if meta information is supported
 */
#define XHTML_META_SUPPORTED

/* Define if meta information is supported
 */
#define XHTML_RUBY_SUPPORTED

class PD_Document;
class FG_Graphic;

// The importer/reader for XHTML 1.0

class ABI_EXPORT IE_Imp_XHTML_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_XHTML_Sniffer();
	virtual ~IE_Imp_XHTML_Sniffer() {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class ABI_EXPORT IE_Imp_XHTML : public IE_Imp_XML
{
public:
	IE_Imp_XHTML (PD_Document * pDocument);

	virtual ~IE_Imp_XHTML ();

	virtual UT_Error		importFile (const char * szFilename);

	void					startElement (const XML_Char * name, const XML_Char ** atts);
	void					endElement (const XML_Char * name);

	virtual void			charData (const XML_Char * buffer, int length);

	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData, 
										UT_uint32 lenData, 
										const char * szEncoding = 0);

protected:
	virtual FG_Graphic *	importImage (const XML_Char * szSrc);

private:
	FG_Graphic *			importDataURLImage (const XML_Char * szData);

	bool					pushInline (const char * props);
	bool					newBlock (const char * style, const char * css, const char * align);
	bool					requireBlock ();
	bool					requireSection ();
	bool					childOfSection ();

	enum listType {L_NONE = 0, L_OL = 1, L_UL = 2 } m_listType;
	UT_uint16	m_iListID;
	bool        m_bFirstDiv;
	bool		m_bUseTidy;
	UT_uint16	m_iNewListID;
	UT_uint16	m_iNewImage;

	UT_Stack	m_utsParents;
	XML_Char *  m_szBookMarkName;

	bool        m_addedPTXSection;

	UT_uint16	m_iPreCount;

	UT_String	m_dirname;

	UT_Vector	m_divClasses;
	UT_Vector	m_divStyles;
};

#endif /* IE_IMP_XHTML_H */
