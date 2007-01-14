/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#ifndef IE_IMP_GZIPABIWORD_H
#define IE_IMP_GZIPABIWORD_H

#include <stdio.h>
#include <zlib.h>
#include "ie_imp_AbiWord_1.h"

// The importer/reader for GZipped AbiWord file format version 1.

class ABI_EXPORT IE_Imp_GZipAbiWord_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_GZipAbiWord_Sniffer();
	virtual ~IE_Imp_GZipAbiWord_Sniffer() {}

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

class ABI_EXPORT IE_Imp_GZipAbiWord : public IE_Imp_AbiWord_1, public UT_XML::Reader
{
public:
	IE_Imp_GZipAbiWord(PD_Document * pDocument);
	~IE_Imp_GZipAbiWord();

	/* Implementation of UT_XML::Reader
	 */
	virtual bool      openFile (const char * szFilename);
	virtual UT_uint32 readBytes (char * buffer, UT_uint32 length);
	virtual void      closeFile (void);

private:
	gzFile m_gzfp;
};

#endif /* IE_IMP_GZIPABIWORD_H */
