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

class IE_Imp_GZipAbiWord : public IE_Imp_AbiWord_1
{
public:
	IE_Imp_GZipAbiWord(PD_Document * pDocument);
	~IE_Imp_GZipAbiWord();

	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData);

	static bool		RecognizeContents(const char * szBuf, UT_uint32 iNumbytes);
	static bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
										  IE_Imp ** ppie);
	static bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
	static bool 		SupportsFileType(IEFileType ft);

protected:
    virtual bool			_openFile(const char * szFilename);
    virtual UT_uint32			_readBytes(char * buf, UT_uint32 length);
    virtual void			_closeFile(void);

    gzFile m_gzfp;
};

#endif /* IE_IMP_GZIPABIWORD_H */
