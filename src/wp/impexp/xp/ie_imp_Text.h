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
#include "ut_mbtowc.h"
#include "pd_Document.h"

// Stream class can be File or Clipboard

class ImportStream
{
public:
	ImportStream();
	virtual ~ImportStream() {}
	bool init(const char *szEncoding);
	bool getChar(UT_UCSChar &b);
	UT_UCSChar peekChar() { return m_ucsLookAhead; }
protected:
	virtual bool _getByte(unsigned char &b) = 0;
	virtual bool getRawChar(UT_UCSChar &b);
	UT_Mbtowc m_Mbtowc;
	UT_UCSChar m_ucsLookAhead;
	bool m_bEOF;
	bool m_bRaw;
};

// File stream class

class ImportStreamFile : public ImportStream
{
public:
	ImportStreamFile(FILE *pFile);
	~ImportStreamFile() {}
	bool getChar();
protected:
	bool _getByte(unsigned char &b);
private:
	FILE *m_pFile;
};

// Clipboard stream class

class ImportStreamClipboard : public ImportStream
{
public:
	ImportStreamClipboard(unsigned char *pClipboard, UT_uint32 iLength);
	~ImportStreamClipboard() {};
	bool getChar();
protected:
	bool _getByte(unsigned char &b);
private:
	unsigned char *m_p;
	unsigned char *m_pEnd;
};

// Helper class so we can parse files and clipboard with same code

class Inserter
{
public:
	Inserter(PD_Document * pDocument);
	Inserter(PD_Document * pDocument, PT_DocPosition dPos);
	bool insertBlock();
	bool insertSpan(UT_GrowBuf &b);
private:
	PD_Document * m_pDocument;
	bool m_bClipboard;
	PT_DocPosition m_dPos;
};

// The importer/reader for Plain Text Files.

class IE_Imp_Text_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_Text;

public:
	IE_Imp_Text_Sniffer() {}
	virtual ~IE_Imp_Text_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
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

// The importer/reader for Plain Text Files with selectable encoding.

class IE_Imp_EncodedText_Sniffer : public IE_Imp_Text_Sniffer
{
	friend class IE_Imp;
	friend class IE_Imp_Text;

public:
	IE_Imp_EncodedText_Sniffer() {}
	virtual ~IE_Imp_EncodedText_Sniffer() {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

protected:
};

class IE_Imp_Text : public IE_Imp
{
public:
	IE_Imp_Text(PD_Document * pDocument, bool bEncoded=false);
	~IE_Imp_Text() {}

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData, const char * szEncoding = 0);

protected:
	UT_Error			_recognizeEncoding(FILE * fp);
	UT_Error			_recognizeEncoding(const char *szBuf, UT_uint32 iNumbytes);
	virtual UT_Error	_constructStream(ImportStream *& pStream, FILE * fp);
	UT_Error			_writeHeader(FILE * fp);
	UT_Error			_parseStream(ImportStream * pStream, class Inserter & ins);
	bool				_doEncodingDialog(const char *szEncoding);
	void				_setEncoding(const char *szEncoding);

	const char *	m_szEncoding;
	bool			m_bIsEncoded;
	bool			m_bIs16Bit;
	bool			m_bUseBOM;
	bool			m_bBigEndian;
};

#endif /* IE_IMP_TEXT_H */
