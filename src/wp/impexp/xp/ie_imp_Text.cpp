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


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_iconv.h"
#include "ie_imp_Text.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_EncodingManager.h"


#include "ap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"

/*!
  Construct ImportStream
 */
ImportStream::ImportStream() :
	m_ucsLookAhead(0),
	m_bEOF(false),
	m_bRaw(false)
{
}

/*!
  Initialize ImportStream
 \param szEncoding Text encoding to convert from

 Sets encoding and prefetches lookahead character.
 Set to 0 to handle raw bytes.
 */
bool ImportStream::init(const char *szEncoding)
{
	if (szEncoding)
		m_Mbtowc.setInCharset(szEncoding);
	else
		m_bRaw = true;

	UT_UCSChar dummy;
	return getChar(dummy);
}

/*!
  Get UCS-2 character from stream
 \param ucs Reference to the character

 Returns single character for CRLF combination
 */
bool ImportStream::getChar(UT_UCSChar &ucs)
{
	if (!getRawChar(ucs))
		return false;
	if (ucs == UCS_CR && peekChar() == UCS_LF)
		getRawChar(ucs);
	return true;
}

/*!
  Get UCS-2 character from stream
 \param ucs Reference to the character

 Get the next UCS character, converting from file's encoding
 */
bool ImportStream::getRawChar(UT_UCSChar &ucs)
{
	wchar_t wc = 0;
	unsigned char b;

	if (m_bEOF)
		return false;

	do
	{
		if (!_getByte(b))
		{
			m_bEOF = true;
			break;
		}
		else if (m_bRaw)
		{
			wc = b;
			break;
		}

	} while (!m_Mbtowc.mbtowc(wc,b));

	// Watch for evil Unicode values!
	// Surrogates
	UT_ASSERT(!(wc >= 0xD800 && wc <= 0xDFFF));
	// Private Use Area
	UT_ASSERT(!((wc >= 0xDB80 && wc <= 0xDBFF)||(wc >= 0xE000 && wc <= 0xF8FF)));
	// AbiWord control characters
	UT_ASSERT(wc != UCS_FIELDSTART && wc != UCS_FIELDEND);
	// Illegal characters
	UT_ASSERT(wc != 0xFFFE && wc != 0xFFFF);

	ucs = m_ucsLookAhead;
	m_ucsLookAhead = wc;

	return true;
}

/*!
  Construct ImportStreamFile from FILE pointer
 \param pFile File to read from
 */
ImportStreamFile::ImportStreamFile(FILE *pFile) :
	m_pFile(pFile)
{
}

/*!
  Get next byte from file
 \param b Reference to the byte
 */
bool ImportStreamFile::_getByte(unsigned char &b)
{
	UT_ASSERT(m_pFile);

	return fread(&b, 1, sizeof(b), m_pFile) > 0;
}

/*!
  Construct ImportStreamClipboard from memory buffer
 \param pClipboard Buffer to read from
 \param iLength Length of buffer
 */
ImportStreamClipboard::ImportStreamClipboard(unsigned char *pClipboard, UT_uint32 iLength) :
	m_p(pClipboard),
	m_pEnd(pClipboard + iLength)
{
}

/*!
  Get next byte from clipboard
 \param b Reference to the byte
 */
bool ImportStreamClipboard::_getByte(unsigned char &b)
{
	if (m_p >= m_pEnd)
		return false;
	b = *m_p++;
	return true;
}

/*!
  Construct Inserter helper class
 \param pDocument Document to insert data into
 */
Inserter::Inserter(PD_Document * pDocument) :
	m_pDocument(pDocument),
	m_bClipboard(false)
{
}

/*!
  Construct Inserter helper class
 \param pDocument Document to insert data into
 \param dPos Position in document to begin inserting at
 */
Inserter::Inserter(PD_Document * pDocument, PT_DocPosition dPos) :
	m_pDocument(pDocument),
	m_bClipboard(true),
	m_dPos(dPos)
{
}

/*!
  Insert a Block into the document

 Uses appropriate function for clipboard or file
 */
bool Inserter::insertBlock()
{
	bool bRes;
	
	if (m_bClipboard)
	{
		bRes = m_pDocument->insertStrux(m_dPos, PTX_Block);
		m_dPos++;
	}
	else
		bRes = m_pDocument->appendStrux(PTX_Block, NULL);

	return bRes;
}

/*!
  Insert a span of text into the document
 \param b Buffer containing UCS text to insert

 Uses appropriate function for clipboard or file
 */
bool Inserter::insertSpan(UT_GrowBuf &b)
{
	bool bRes;
	
	if (m_bClipboard)
	{
		bRes = m_pDocument->insertSpan(m_dPos, b.getPointer(0), b.getLength());
		m_dPos += b.getLength();
	}
	else
		bRes = m_pDocument->appendSpan(b.getPointer(0), b.getLength());

	b.truncate(0);

	return bRes;
}

/*****************************************************************/
/*****************************************************************/

/*!
  Check if buffer contains data meant for this importer.

 We don't attmpt to recognize since other filetypes (HTML) can
 use the same encodings a text file can.
 We also don't want to steal recognition when user wants to use
 the Encoded Text importer.
 */
bool IE_Imp_Text_Sniffer::recognizeContents(const char * /* szBuf */,
											UT_uint32 /* iNumbytes */)
{
	return false;
}

/*!
  Check buffer for UTF-8 encoded characters
 \param szBuf Buffer to check
 \param iNumbytes Size of buffer
 */
bool IE_Imp_Text_Sniffer::_recognizeUTF8(const char * szBuf,
										 UT_uint32 iNumbytes)
{
	bool bSuccess = false;
	const unsigned char *p = reinterpret_cast<const unsigned char *>(szBuf);

	while (p < reinterpret_cast<const unsigned char *>(szBuf + iNumbytes))
	{
		UT_sint32 iLen;
		
		if ((*p & 0x80) == 0)				// ASCII
		{
			++p;
			continue;
		}
		else if ((*p & 0xc0) == 0x80)			// not UTF-8
		{
			return false;
		}
		else if (*p == 0xfe || *p == 0xff)
		{
			// BOM shouldn't occur in UTF-8 - file may be UCS-2
			return false;
		}
		else if ((*p & 0xfe) == 0xfc)			// lead byte in 6-byte sequence
			iLen = 6;
		else if ((*p & 0xfc) == 0xf8)			// lead byte in 5-byte sequence
			iLen = 5;
		else if ((*p & 0xf8) == 0xf0)			// lead byte in 4-byte sequence
			iLen = 4;
		else if ((*p & 0xf0) == 0xe0)			// lead byte in 3-byte sequence
			iLen = 3;
		else if ((*p & 0xe0) == 0xc0)			// lead byte in 2-byte sequence
			iLen = 2;
		else	
		{
			// the above code covers all cases - if we reach here the logic is wrong
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	
		while (--iLen)
		{
			++p;
			if (p >= reinterpret_cast<const unsigned char *>(szBuf + iNumbytes))
			{
				//UT_DEBUGMSG(("  out of data!\n"));
				break;
			}
			if ((*p & 0xc0) != 0x80)
				return false;
		}
		// all bytes in sequence were ok
		bSuccess = true;
		++p;
	}
	
	return bSuccess;
}

/*!
  Check buffer for UCS-2 encoded characters
 \param szBuf Buffer to check
 \param iNumbytes Size of buffer
 \param bDeep Set to true for extra, non-authoritative tests
 */
IE_Imp_Text_Sniffer::UCS2_Endian IE_Imp_Text_Sniffer::_recognizeUCS2(const char * szBuf,
																	 UT_uint32 iNumbytes,
																	 bool bDeep)
{
	UCS2_Endian eResult = UE_NotUCS;
	
	if (iNumbytes >= 2)
	{
		const unsigned char *p = reinterpret_cast<const unsigned char *>(szBuf);

		// Big endian ?
		if (p[0] == 0xfe && p[1] == 0xff)
			eResult = UE_BigEnd;

		// Little endian
		else if (p[0] == 0xff && p[1] == 0xfe)
			eResult = UE_LittleEnd;

		if (eResult == UE_NotUCS && bDeep)
		{
			// If we know this is a text file, know it isn't UTF-8, and it doesn't
			// begin with a BOM, let's try a couple of heuristics too see if it
			// might be a UCS-2 file without a BOM.
			// Since CR and LF are very common and their endian-swapped counterparts
			// are reserved in Unicode, they should only exist in big endian or
			// little endian but not both.
			// If there are no CRs or LFs we fall back on counting how many characters
			// fall within the ASCII range for both endians.  The one with the higher
			// count wins.
			// Text files which contain NUL characters will be wrongly identified as
			// UCS-2 using this technique.

			UT_sint32 iLineEndBE = 0;
			UT_sint32 iLineEndLE = 0;
			UT_sint32 iAsciiBE = 0;
			UT_sint32 iAsciiLE = 0;

			// Count all CR, LF, and ASCII range characters.
			for (p = reinterpret_cast<const unsigned char *>(szBuf);
				 p < reinterpret_cast<const unsigned char *>(szBuf + iNumbytes - 1);
				 p += 2)
			{
				// A 16-bit null character probably won't exist in a UCS-2 file
				if (p[0] == 0 && p[1] == 0)
					break;
				if (p[0] == 0)
				{
					++iAsciiBE;
					if (p[1] == 0x0A || p[1] == 0x0D)
						++iLineEndBE;
				}
				if (p[1] == 0)
				{
					++iAsciiLE;
					if (p[0] == 0x0A || p[0] == 0x0D)
						++iLineEndLE;
				}
			}

			// Take an educated guess.
			if (iLineEndBE && !iLineEndLE)
				eResult = UE_BigEnd;
			else if (iLineEndLE && !iLineEndBE)
				eResult = UE_LittleEnd;
			else if (!iLineEndBE && !iLineEndLE)
			{
				if (iAsciiBE > iAsciiLE)
					eResult = UE_BigEnd;
				else if (iAsciiLE > iAsciiBE)
					eResult = UE_LittleEnd;
			}
		}
	}

	return eResult;
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Imp_Text_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp (szSuffix, ".txt") || !UT_stricmp(szSuffix, ".text"));
}

UT_Error IE_Imp_Text_Sniffer::constructImporter(PD_Document * pDocument,
												IE_Imp ** ppie)
{
	IE_Imp_Text * p = new IE_Imp_Text(pDocument,false);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_Text_Sniffer::getDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "Text (.txt, .text)";
	*pszSuffixList = "*.txt; *.text";
	*ft = getFileType();
	return true;
}

/*!
  Check if buffer contains data meant for this importer.

 We don't attempt to recognize.  User must specifically choose Encoded Text.
 */
bool IE_Imp_EncodedText_Sniffer::recognizeContents(const char * /* szBuf */,
												   UT_uint32 /* iNumbytes */)
{
	return false;
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Imp_EncodedText_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp (szSuffix, ".etxt") || !UT_stricmp(szSuffix, ".etext"));
}

UT_Error IE_Imp_EncodedText_Sniffer::constructImporter(PD_Document * pDocument,
												IE_Imp ** ppie)
{
	IE_Imp_Text * p = new IE_Imp_Text(pDocument,true);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_EncodedText_Sniffer::getDlgLabels(const char ** pszDesc,
											  const char ** pszSuffixList,
											  IEFileType * ft)
{
	*pszDesc = "Encoded Text (.etxt, .etext)";
	*pszSuffixList = "*.etxt; *.etext";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

/*
  Import data from a plain text file
 \param szFilename Name of file to import
  
 Each line terminator is taken to be a paragraph break
*/
UT_Error IE_Imp_Text::importFile(const char * szFilename)
{
	// We must open in binary mode for UCS-2 compatibility.
	FILE *fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	ImportStream *pStream = 0;
	UT_Error error;

	// First we try to determine the encoding.
	if (_recognizeEncoding(fp) == UT_OK)
		m_pDocument->setEncodingName(m_szEncoding);

	// Call encoding dialog
	if (!m_bIsEncoded || _doEncodingDialog(m_szEncoding))
	{
		X_CleanupIfError(error,_constructStream(pStream,fp));
		Inserter ins(m_pDocument);
		X_CleanupIfError(error,_writeHeader(fp));
		X_CleanupIfError(error,_parseStream(pStream,ins));
		error = UT_OK;
	}
	else
		error = UT_ERROR;

Cleanup:
	delete pStream;
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

/*
  Construct text importer
 \param pDocument Document to import text into
 \param bEncoded True if we should show encoding dialog
  
 Uses current document's encoding if it is set
*/
IE_Imp_Text::IE_Imp_Text(PD_Document * pDocument, bool bEncoded)
	: IE_Imp(pDocument)
{
	UT_ASSERT(pDocument);

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	m_bIsEncoded = bEncoded;

	_setEncoding(szEncodingName);
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

/*!
  Detect encoding of text file
 \param fp File

 Supports UTF-8 and UCS-2 big and little endian
 CJK encodings could be added
 */
UT_Error IE_Imp_Text::_recognizeEncoding(FILE * fp)
{
	char szBuf[4096];  // 4096 ought to be enough
	UT_sint32 iNumbytes;

	iNumbytes = fread(szBuf, 1, sizeof(szBuf), fp);
	fseek(fp, 0, SEEK_SET);

	return _recognizeEncoding(szBuf, iNumbytes);
}

/*!
  Detect encoding of text buffer
 \param pData Buffer
 \param lenData Length of buffer

 Supports UTF-8 and UCS-2 big and little endian
 CJK encodings could be added
 */
UT_Error IE_Imp_Text::_recognizeEncoding(const char *szBuf, UT_uint32 iNumbytes)
{
	if (IE_Imp_Text_Sniffer::_recognizeUTF8(szBuf, iNumbytes))
		_setEncoding("UTF-8");
	else
	{
		IE_Imp_Text_Sniffer::UCS2_Endian eUcs2 = IE_Imp_Text_Sniffer::UE_NotUCS;

		eUcs2 = IE_Imp_Text_Sniffer::_recognizeUCS2(szBuf, iNumbytes, true);
		
		if (eUcs2 == IE_Imp_Text_Sniffer::UE_BigEnd)
			_setEncoding(XAP_EncodingManager::get_instance()->getUCS2BEName());
		else if (eUcs2 == IE_Imp_Text_Sniffer::UE_LittleEnd)
			_setEncoding(XAP_EncodingManager::get_instance()->getUCS2LEName());
	}

	return UT_OK;
}

/*!
  Create a stream of the appropriate type
 \param pStream Pointer to created stream
 \param fp File to construct stream from

 Override this virtual function to derive from the text importer
 */
UT_Error IE_Imp_Text::_constructStream(ImportStream *& pStream, FILE * fp)
{
	return (pStream = new ImportStreamFile(fp)) ? UT_OK : UT_IE_NOMEMORY;
}

/*!
  Write header to document

 Writes the minimum needed Section and Block before we begin import
 */
UT_Error IE_Imp_Text::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));

	return UT_OK;
}

/*!
  Parse stream contents into the document
 \param stream Stream to import from
 \param ins Inserter helper class

 This code is used for both files and the clipboard
 */
UT_Error IE_Imp_Text::_parseStream(ImportStream * pStream, Inserter & ins)
{
	UT_ASSERT(pStream);

	bool bFirstChar = true;
	UT_GrowBuf gbBlock(1024);
	UT_UCSChar c;

	pStream->init(m_szEncoding);

	while (pStream->getChar(c))
	{
		// TODO We should switch fonts when we encounter
		// TODO characters from different scripts
		switch (c)
		{
		case UCS_CR:
		case UCS_LF:
		case UCS_LINESEP:
		case UCS_PARASEP:
			// we interpret either CRLF, CR, or LF as a paragraph break.
			// we also accept U+2028 (line separator) and U+2029 (para separator)
			// especially since these are recommended by Mac OS X.

			// flush out what we have
			if (gbBlock.getLength() > 0)
				X_ReturnNoMemIfError(ins.insertSpan(gbBlock));
			X_ReturnNoMemIfError(ins.insertBlock());
			break;

		case UCS_BOM:
			// This is Byte Order Mark at the start of file, Zero Width Non Joiner elsewhere
			if (bFirstChar)
				break;

		default:
			X_ReturnNoMemIfError(gbBlock.append(&c,1));
			break;
		}
		bFirstChar = false;
	}

	if (gbBlock.getLength() > 0)
		X_ReturnNoMemIfError(ins.insertSpan(gbBlock));

	return UT_OK;
}

/*!
  Request file encoding from user

 This function should be identical to the one in ie_Exp_Text
 */
bool IE_Imp_Text::_doEncodingDialog(const char *szEncoding)
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_ENCODING;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(m_pDocument->getApp()->getDialogFactory());

	XAP_Dialog_Encoding * pDialog
		= (XAP_Dialog_Encoding *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setEncoding(szEncoding);

	// run the dialog
	XAP_Frame * pFrame = m_pDocument->getApp()->getLastFocussedFrame();
	UT_ASSERT(pFrame);

	pDialog->runModal(pFrame);

	// extract what they did
	
	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Encoding::a_OK);

	if (bOK)
	{
		const XML_Char * s;
		static XML_Char szEnc[16];

		s = pDialog->getEncoding();
		UT_ASSERT (s);

		strcpy(szEnc,s);
		_setEncoding((const char *)szEnc);
		m_pDocument->setEncodingName(szEnc);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*!
  Set importer's encoding and related members
 \param szEncoding Encoding to export file into

 Decides endian and BOM policy based on encoding.
 Set to 0 to handle raw bytes.
 This function should be identical to the one in IE_Exp_Text.
 */
void IE_Imp_Text::_setEncoding(const char *szEncoding)
{
	m_szEncoding = szEncoding;

	// TODO Should BOM use be a user pref?
	// TODO Does Mac OSX prefer BOMs?
	if (szEncoding && !strcmp(szEncoding,XAP_EncodingManager::get_instance()->getUCS2LEName()))
	{
		m_bIs16Bit = true;
		m_bBigEndian = false;
#ifdef WIN32
		m_bUseBOM = true;
#else
		m_bUseBOM = false;
#endif
	}
	else if (szEncoding && !strcmp(szEncoding,XAP_EncodingManager::get_instance()->getUCS2BEName()))
	{
		m_bIs16Bit = true;
		m_bBigEndian = true;
#ifdef WIN32
		m_bUseBOM = true;
#else
		m_bUseBOM = false;
#endif
	}
	else
	{
		m_bIs16Bit = false;
		// These are currently meaningless when not in a Unicode encoding
		m_bBigEndian = false;
		m_bUseBOM = false;
	}
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

/*****************************************************************/
/*****************************************************************/

// TODO This function needs an encoding parameter since the clipboard can be
// TODO in any encoding and the OSes can track it.  Currently we check for
// TODO UCS-2 which handles Windows's unicode clipboard.  8-bit data is
// TODO always interpreted using the system default encoding which can be wrong.

void IE_Imp_Text::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData,
								  const char *szEncoding)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	// Attempt to guess whether we're pasting 8 bit or unicode text
	if (szEncoding)
		_setEncoding(szEncoding);
	else
		_recognizeEncoding((const char *)pData, lenData);

	ImportStreamClipboard stream(pData, lenData);
	Inserter ins(m_pDocument, pDocRange->m_pos1);

	_parseStream(&stream, ins);
}

