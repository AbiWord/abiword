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


#ifndef IE_IMP_APPLIX_H
#define IE_IMP_APPLIX_H

#include <stdio.h>
#include "ie_imp.h"
#include"ut_mbtowc.h"
class PD_Document;

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
										unsigned char * pData, UT_uint32 lenData);
	
protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);

 private:
	bool m_bLastWasP;
	bool m_bInT;

};

#endif /* IE_IMP_APPLIX_H */
