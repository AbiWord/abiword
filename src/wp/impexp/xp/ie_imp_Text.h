/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef IE_IMP_TEXT_H
#define IE_IMP_TEXT_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_mbtowc.h"
#include "pd_Document.h"

class pf_Frag_Strux;

// Stream class can be File or Clipboard

class ABI_EXPORT ImportStream
{
 public:
	ImportStream();
	virtual ~ImportStream();
	bool init(const char *szEncoding);
	bool getChar(UT_UCSChar &b);
	UT_UCSChar peekChar() { return m_ucsLookAhead; }
 protected:
	virtual bool _getByte(unsigned char &b) = 0;
	virtual bool getRawChar(UT_UCSChar &b);

	bool _get_eof () const { return m_bEOF; }
	void _set_eof (bool b) { m_bEOF = b; }
	UT_UCSChar _lookAhead () const { return m_ucsLookAhead; }
	void _lookAhead ( UT_UCSChar c ) { m_ucsLookAhead = c; }

 private:
	UT_UCS4_mbtowc m_Mbtowc;
	UT_UCSChar m_ucsLookAhead;
	bool m_bEOF;
	bool m_bRaw;
};

// File stream class

class ABI_EXPORT ImportStreamFile : public ImportStream
{
public:
	ImportStreamFile(GsfInput *pFile);
	~ImportStreamFile();
	bool getChar();
protected:
	bool _getByte(unsigned char &b);
private:
	GsfInput *m_pFile;
};

// Clipboard stream class

class ABI_EXPORT ImportStreamClipboard : public ImportStream
{
public:
	ImportStreamClipboard(const unsigned char *pClipboard, UT_uint32 iLength);
	~ImportStreamClipboard();
	//	bool getChar();
protected:
	bool _getByte(unsigned char &b);
private:
	const unsigned char *m_p;
	const unsigned char *m_pEnd;
};

// The importer/reader for Plain Text Files.

class ABI_EXPORT IE_Imp_Text_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_Text;

public:
	IE_Imp_Text_Sniffer();
	virtual ~IE_Imp_Text_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	const char * recognizeContentsType (const char * szBuf,
									UT_uint32 iNumbytes);
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

class ABI_EXPORT IE_Imp_EncodedText_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_Text;

public:
	IE_Imp_EncodedText_Sniffer();
	virtual ~IE_Imp_EncodedText_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }

	virtual UT_Confidence_t recognizeContents (const char * szBuf,
					    UT_uint32 iNumbytes);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

protected:
};

class ABI_EXPORT IE_Imp_Text : public IE_Imp
{
public:
	IE_Imp_Text(PD_Document * pDocument, bool bEncoded=false);
	IE_Imp_Text(PD_Document * pDocument, const char * encoding);
	virtual ~IE_Imp_Text();

	virtual bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData, UT_uint32 lenData, const char * szEncoding = 0);

protected:
	virtual UT_Error	_loadFile(GsfInput * fp);
	UT_Error			_recognizeEncoding(GsfInput * fp);
	UT_Error			_recognizeEncoding(const char *szBuf, UT_uint32 iNumbytes);
	virtual UT_Error	_constructStream(ImportStream *& pStream, GsfInput * fp);
	UT_Error			_writeHeader(GsfInput * fp);
	UT_Error			_parseStream(ImportStream * pStream);
	bool				_doEncodingDialog(const char *szEncoding);
	void				_setEncoding(const char *szEncoding);

	bool _insertBlock ();
	bool _insertSpan (UT_GrowBuf &b);

 private:
	const char *	m_szEncoding;
	bool m_bExplicitlySetEncoding;
	bool			m_bIsEncoded;
	bool			m_bIs16Bit;
	bool			m_bUseBOM;
	bool			m_bBigEndian;
	bool            m_bBlockDirectionPending;
	bool            m_bFirstBlockData;
	pf_Frag_Strux * m_pBlock;
};

#endif /* IE_IMP_TEXT_H */
