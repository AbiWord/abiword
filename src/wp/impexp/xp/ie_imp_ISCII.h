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

#ifndef IE_IMP_ISCII_H
#define IE_IMP_ISCII_H

#include "ie_imp_Text.h"

#define ISCII_INV	0xD9	// Invisible character - acts like Unicode ZWJ
#define ISCII_NUKTA	0xE9	// A Nukta following a character can modify its meaning
#define ISCII_ATTR	0xEF	// Attribute - meaning?
#define ISCII_EXT	0xF0	// EXT preceeding a character can modify its meaning

class PD_Document;

// ISCII file stream class

class ImportISCIIStreamFile : public ImportStreamFile
{
public:
	ImportISCIIStreamFile(FILE *pFile);
	~ImportISCIIStreamFile() {}
protected:
	virtual bool getRawChar(UT_UCSChar &b);
private:
	unsigned char m_cLookAhead;
	bool m_bNeedByte;
};

// The importer/reader for ISCII files.

class IE_Imp_ISCII_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_ISCII_Sniffer() {}
	virtual ~IE_Imp_ISCII_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_ISCII : public IE_Imp_Text
{
public:
	IE_Imp_ISCII(PD_Document * pDocument);
	virtual ~IE_Imp_ISCII() {};

protected:
	virtual UT_Error	_constructStream(ImportStream *& pStream, FILE * fp);
};

#endif /* IE_IMP_ISCII_H */
