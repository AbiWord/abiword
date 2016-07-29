/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#define XHTML_TABLES_SUPPORTED 1

/* Define if meta information is supported
 */
#define XHTML_META_SUPPORTED

/* Define if meta information is supported
 */
#define XHTML_RUBY_SUPPORTED

class PD_Document;
class FG_Graphic;
class IE_Imp_TableHelperStack;

// The importer/reader for XHTML 1.0

class ABI_EXPORT IE_Imp_XHTML_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_XHTML_Sniffer();
	virtual ~IE_Imp_XHTML_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
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

	void					startElement (const gchar * name, const gchar ** atts);
	void					endElement (const gchar * name);

	virtual void			charData (const gchar * buffer, int length);

	virtual bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData,
										UT_uint32 lenData,
										const char * szEncoding = 0);

	virtual bool  appendStrux(PTStruxType pts, const PP_PropertyVector & attributes);
	virtual bool  appendFmt(const PP_PropertyVector & vecAttributes);
	virtual bool  appendSpan(const UT_UCSChar * p, UT_uint32 length);
	virtual bool  appendObject(PTObjectType pto, const PP_PropertyVector & attributes);


protected:
	virtual UT_Error        _loadFile (GsfInput * input);
	virtual FG_Graphic *	importImage (const gchar * szSrc);

private:
	FG_Graphic *			importDataURLImage (const gchar * szData);

	bool					pushInline (const char * props);
	bool					newBlock (const char * style, const char * css, const char * align);
	bool					requireBlock ();
	bool					requireSection ();
	bool					childOfSection ();

	IE_Imp_TableHelperStack *	m_TableHelperStack;

	enum listType {L_NONE = 0, L_OL = 1, L_UL = 2 } m_listType;
	UT_uint16	m_iListID;
	UT_uint16	m_iNewListID;
	UT_uint16	m_iNewImage;

	UT_Stack	m_utsParents;
	std::string m_szBookMarkName;

	bool        m_addedPTXSection;

	UT_uint16	m_iPreCount;

	UT_Vector	m_divClasses;
	UT_GenericVector<UT_UTF8String *>	m_divStyles;
	bool        bInTable(void);
	bool        m_bFirstBlock;
	bool		m_bInMath;
	UT_ByteBuf* m_pMathBB;
	std::string m_Title;
};

#endif /* IE_IMP_XHTML_H */
