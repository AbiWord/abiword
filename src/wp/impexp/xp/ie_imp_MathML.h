/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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


#ifndef IE_IMP_MATHML_H
#define IE_IMP_MATHML_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"
class   UT_ByteBuf;
class   ImportStream;

// The importer/reader for MathML Files.

class ABI_EXPORT IE_Imp_MathML_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_MathML;

public:
	IE_Imp_MathML_Sniffer();
	virtual ~IE_Imp_MathML_Sniffer() {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	const char * recognizeContentsType (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

protected:
	enum UCS2_Endian { UE_BigEnd = -1, UE_NotUCS = 0, UE_LittleEnd };

	static bool _recognizeUTF8 (const char * szBuf,
								UT_uint32 iNumbytes);
	static UCS2_Endian _recognizeUCS2 (const char * szBuf,
									   UT_uint32 iNumbytes,
									   bool bDeep);
};

// The importer/reader for MathML Files.

class ABI_EXPORT IE_Imp_MathML : public IE_Imp
{
public:
	IE_Imp_MathML(PD_Document * pDocument);
	virtual	~IE_Imp_MathML();

	virtual UT_Error	importFile(const char * szFilename);
	virtual bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData, UT_uint32 lenData, const char * szEncoding = 0);
	UT_ByteBuf *        getByteBuf(void) const {return m_pByteBuf;}

protected:
	UT_Error			_parseStream(ImportStream * pStream);

 private:
	UT_ByteBuf *        m_pByteBuf;
};

#endif /* IE_IMP_MATHML_H */
