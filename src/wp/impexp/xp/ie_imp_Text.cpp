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

/*****************************************************************/
/*****************************************************************/

/*!
  Check buffer for identifiable encoded characters
 \param szBuf Buffer to check
 \param iNumbytes Size of buffer
 */
bool IE_Imp_Text_Sniffer::recognizeContents(const char * szBuf, 
											UT_uint32 iNumbytes)
{
	// TODO It may or may not be worthwhile trying to recognize CJK encodings.
	
	bool bSuccess = false;

	bSuccess = _recognizeUTF8(szBuf, iNumbytes);

	if (bSuccess == false)
	{
		if (_recognizeUCS2(szBuf, iNumbytes, false) != UE_NotUCS)
		{
			bSuccess = true;
		}
	}
	
	return bSuccess;
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

/*
  Import data from a plain text file.  We allow either
  LF or CR or CRLF line termination.  Each line
  terminator is taken to be a paragraph break.
*/

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_Text::importFile(const char * szFilename)
{
	// We must open in binary mode for UCS-2 compatibility.
	FILE *fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error;

	// First we need to determine the encoding.
	X_CleanupIfError(error,_recognizeEncoding(fp));
	X_CleanupIfError(error,_writeHeader(fp));
	X_CleanupIfError(error,_parseFile(fp));

	error = UT_OK;

Cleanup:
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

/*!
  Destruct text importer
 */
IE_Imp_Text::~IE_Imp_Text()
{
}

IE_Imp_Text::IE_Imp_Text(PD_Document * pDocument, bool bEncoded)
	: IE_Imp(pDocument)
{
	m_bIsEncoded = bEncoded;
	// TODO Use persistent document encoding when it exists
	_setEncoding(XAP_EncodingManager::get_instance()->getNativeEncodingName());
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

	if (IE_Imp_Text_Sniffer::_recognizeUTF8(szBuf, iNumbytes))
	{
		_setEncoding("UTF-8");
	}
	else
	{
		IE_Imp_Text_Sniffer::UCS2_Endian eUcs2 = IE_Imp_Text_Sniffer::UE_NotUCS;

		eUcs2 = IE_Imp_Text_Sniffer::_recognizeUCS2(szBuf, iNumbytes, true);
		
		// TODO Old libiconv uses UCS-2-BE, new uses UCS-2BE
		if (eUcs2 == IE_Imp_Text_Sniffer::UE_BigEnd)
			_setEncoding("UCS-2-BE");
		else if (eUcs2 == IE_Imp_Text_Sniffer::UE_LittleEnd)
			_setEncoding("UCS-2-LE");
	}

	return UT_OK;
}

UT_Error IE_Imp_Text::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

UT_Error IE_Imp_Text::_parseFile(FILE * fp)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bEmptyFile = true;
	unsigned char b;
	UT_UCSChar c;
	wchar_t wc;

	// Call encoding dialog
	if (!m_bIsEncoded || _doEncodingDialog(m_szEncoding))
	{
		UT_ASSERT(m_szEncoding);
		m_Mbtowc.setInCharset(m_szEncoding);

		while (fread(&b, 1, sizeof(b), fp) > 0)
		{
			if(!m_Mbtowc.mbtowc(wc,b))
				continue;
			c = (UT_UCSChar)wc;

			// TODO We should switch fonts when we encounter
			// TODO characters from different scripts
			switch (c)
			{
			case (UT_UCSChar)'\r':
			case (UT_UCSChar)'\n':
			case 0x2028:			// Unicode line separator
			case 0x2029:			// Unicode paragraph separator
				
				if ((c == (UT_UCSChar)'\n') && bEatLF)
				{
					bEatLF = false;
					break;
				}

				if (c == (UT_UCSChar)'\r')
				{
					bEatLF = true;
				}
				
				// we interpret either CRLF, CR, or LF as a paragraph break.
				// we also accept U+2028 (line separator) and U+2029 (para separator)
				// especially since these are recommended by Mac OS X.
				
				// start a paragraph and emit any text that we
				// have accumulated.
				X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
				bEmptyFile = false;
				if (gbBlock.getLength() > 0)
				{
					X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
					gbBlock.truncate(0);
				}
				break;

			default:
				bEatLF = false;
				X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&c,1));
				break;
			}
		} 

		if (gbBlock.getLength() > 0 || bEmptyFile)
		{
			// if we have text left over (without final CR/LF),
			// or if we read an empty file,
			// create a paragraph and emit the text now.
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
			if (gbBlock.getLength() > 0)
				X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
		}
		return UT_OK;
	}

	// TODO If the encoding dialog was cancelled we still get an empty new document
	// TODO with an error dialog ):

	return UT_ERROR;
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
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*!
  Set exporter's encoding and related members
 \param szEncoding Encoding to export file into

 Decides endian and BOM policy based on encoding
 */
void IE_Imp_Text::_setEncoding(const char *szEncoding)
{
	UT_ASSERT(szEncoding);

	m_szEncoding = szEncoding;

	// TODO some iconvs use a different string!
	if (!strncmp(m_szEncoding,"UCS-2",5))
	{
		m_bIs16Bit = true;
		if (!strcmp(m_szEncoding + strlen(m_szEncoding) - 2, "BE"))
			m_bBigEndian = true;
		else if (!strcmp(m_szEncoding + strlen(m_szEncoding) - 2, "LE"))
			m_bBigEndian = false;
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

		// TODO Should BOM use be a user pref?
		// TODO Does Mac OSX prefer BOMs?
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
								  unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	// Attempt to guess whether we're pasting 8 bit or unicode text
	IE_Imp_Text_Sniffer::UCS2_Endian eUcs2 = IE_Imp_Text_Sniffer::_recognizeUCS2((const char *)pData, lenData, true);
	
	// TODO Old libiconv uses UCS-2-BE, new uses UCS-2BE
	if (eUcs2 == IE_Imp_Text_Sniffer::UE_BigEnd)
		_setEncoding("UCS-2-BE");
	else if (eUcs2 == IE_Imp_Text_Sniffer::UE_LittleEnd)
		_setEncoding("UCS-2-LE");
	else
		_setEncoding(XAP_EncodingManager::get_instance()->getNativeEncodingName());

	m_Mbtowc.setInCharset(m_szEncoding);

	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bSuppressLeadingParagraph = true;
	bool bInColumn1 = true;
	unsigned char * pc;

	PT_DocPosition dpos = pDocRange->m_pos1;
	
	for (pc=pData; (pc<pData+lenData); pc++)
	{
		unsigned char b = *pc;
		UT_UCSChar c;
		wchar_t wc;
		if(!m_Mbtowc.mbtowc(wc,b))
		    continue;
		c = (UT_UCSChar)wc;
		
		// TODO We should switch fonts when we encounter
		// TODO characters from different scripts
		switch (c)
		{
		case (UT_UCSChar)'\r':
		case (UT_UCSChar)'\n':
		case 0x2028:			// Unicode line separator
		case 0x2029:			// Unicode paragraph separator
			if ((c == (UT_UCSChar)'\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (c == (UT_UCSChar)'\r')
			{
				bEatLF = true;
			}
			
			// we interpret either CRLF, CR, or LF as a paragraph break.
			// we also accept U+2028 (line separator) and U+2029 (para separator)
			// especially since these are recommended by Mac OS X.
			
			if (gbBlock.getLength() > 0)
			{
				// flush out what we have
				m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
				dpos += gbBlock.getLength();
				gbBlock.truncate(0);
			}
			bInColumn1 = true;
			break;

		default:
			bEatLF = false;
			if (bInColumn1 && !bSuppressLeadingParagraph)
			{
				m_pDocument->insertStrux(dpos,PTX_Block);
				dpos++;
			}
			
			gbBlock.ins(gbBlock.getLength(),&c,1);

			bInColumn1 = false;
			bSuppressLeadingParagraph = false;
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF),
		m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
		dpos += gbBlock.getLength();
	}

	return;
}


