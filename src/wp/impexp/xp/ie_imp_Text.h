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


#ifndef IE_IMP_TEXT_H
#define IE_IMP_TEXT_H

#include <stdio.h>
#include "ie_imp.h"
class PD_Document;

// The importer/reader for Plain Text Files.

class IE_Imp_Text : public IE_Imp
{
public:
	IE_Imp_Text(PD_Document * pDocument);
	~IE_Imp_Text();

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData);

	static UT_Bool		RecognizeContents(const char * szBuf, int iNumbytes);
	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
										  IE_Imp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
	static UT_Bool 		SupportsFileType(IEFileType ft);
	
protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);
};

#endif /* IE_IMP_TEXT_H */
