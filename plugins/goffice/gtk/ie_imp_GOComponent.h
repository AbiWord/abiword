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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef IE_IMP_GOComponent_H
#define IE_IMP_GOComponent_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"
#include <glib.h>
class  UT_ByteBuf;
class  ImportStream;

extern GSList *mime_types;

// The importer/reader for Embedable Components.

class IE_Imp_Component_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_Component;

public:
	IE_Imp_Component_Sniffer();
	virtual ~IE_Imp_Component_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence() override;
	virtual const IE_MimeConfidence * getMimeConfidence() override;
	virtual UT_Confidence_t recognizeContents(const char * szBuf,
									UT_uint32 iNumbytes) override;
	const char * recognizeContentsType (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual bool getDlgLabels(const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft) override;
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie) override;

protected:
	enum UCS2_Endian { UE_BigEnd = -1, UE_NotUCS = 0, UE_LittleEnd };

	static bool _recognizeUTF8 (const char * szBuf,
								UT_uint32 iNumbytes);
	static UCS2_Endian _recognizeUCS2 (const char * szBuf,
									   UT_uint32 iNumbytes,
									   bool bDeep);
};

// The importer/reader for GNOME-Office charts.

class IE_Imp_Component : public IE_Imp
{
public:
	IE_Imp_Component(PD_Document * pDocument, char *mime_type = nullptr);
	virtual	~IE_Imp_Component();

	virtual bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData, UT_uint32 lenData, const char * szEncoding = nullptr) override;
	const UT_ByteBufPtr &     getByteBuf(void) const {return m_pByteBuf;}

protected:
	virtual UT_Error	_loadFile(GsfInput * input) override;
	UT_Error			_parseStream(ImportStream * pStream);

 private:
	UT_ByteBufPtr       m_pByteBuf;
	std::string			m_MimeType;
};

#endif /* IE_IMP_Component_H */
