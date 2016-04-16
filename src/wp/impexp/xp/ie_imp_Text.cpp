/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net> 
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

#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_iconv.h"
#include "ie_impexp_Text.h"
#include "ie_imp_Text.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"


#include "ap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"
#include "ap_Prefs.h"

#include "pt_PieceTable.h"
#include "pf_Frag_Strux.h"

/*!
  Construct ImportStream
 */
ImportStream::ImportStream() :
	m_Mbtowc(XAP_EncodingManager::get_instance()->getNative8BitEncodingName()),
	m_ucsLookAhead(0),
	m_bEOF(false),
	m_bRaw(false)
{
}

ImportStream::~ImportStream()
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
	UT_UCS4Char wc = 0;
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
	UT_ASSERT_HARMLESS(!(wc >= 0xD800 && wc <= 0xDFFF));
	// Private Use Area
	// No, Private User Area is not evil!  Commenting it out for now.
	/* UT_ASSERT_HARMLESS(!((wc >= 0xDB80 && wc <= 0xDBFF)||(wc >= 0xE000 && wc <= 0xF8FF))); */
	// AbiWord control characters
	UT_ASSERT_HARMLESS(wc < UCS_ABICONTROL_START || wc > UCS_ABICONTROL_END);
	// Illegal characters
	UT_ASSERT_HARMLESS(wc != 0xFFFE && wc != 0xFFFF);

	ucs = m_ucsLookAhead;
	m_ucsLookAhead = wc;

	return true;
}

/*!
  Construct ImportStreamFile from FILE pointer
 \param pFile File to read from
 */
ImportStreamFile::ImportStreamFile(GsfInput *pFile) :
	m_pFile(pFile)
{
}

ImportStreamFile::~ImportStreamFile()
{
}

/*!
  Get next byte from file
 \param b Reference to the byte
 */
bool ImportStreamFile::_getByte(unsigned char &b)
{
	UT_return_val_if_fail(m_pFile, false);

	return (gsf_input_read(m_pFile, 1, &b) != NULL);
}

/*!
  Construct ImportStreamClipboard from memory buffer
 \param pClipboard Buffer to read from
 \param iLength Length of buffer
 */
ImportStreamClipboard::ImportStreamClipboard(const unsigned char *pClipboard, UT_uint32 iLength) :
	m_p(pClipboard),
	m_pEnd(pClipboard + iLength)
{
}

ImportStreamClipboard::~ImportStreamClipboard()
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
  Insert a Block into the document

 Uses appropriate function for clipboard or file
 */
bool IE_Imp_Text::_insertBlock()
{
	bool ret = false;
	m_bBlockDirectionPending = true;
	m_bFirstBlockData = true;
	
	if (isClipboard ()) // intentional - don't append style
						// information
	{		
		ret = appendStrux(PTX_Block, PP_NOPROPS);
	}
	else
	{
	    // text gets applied in the Normal style
	    const PP_PropertyVector propsArray = {
	        "style", "Normal"
	    };

	    ret = appendStrux(PTX_Block, propsArray);
	}
	if(!isPasting())
	{
		pf_Frag * pf = getDoc()->getPieceTable()->getFragments().getLast();
		UT_return_val_if_fail( pf->getType() == pf_Frag::PFT_Strux, false);
		m_pBlock = (pf_Frag_Strux *) pf;
		UT_return_val_if_fail( m_pBlock->getStruxType() == PTX_Block, false);
	}
	else
	{
		pf_Frag_Strux* sdh = NULL;
		if(getDoc()->getStruxOfTypeFromPosition(getDocPos(), PTX_Block,&sdh))
		{
			m_pBlock = sdh;
		}
		else
		{
			m_pBlock = NULL;
		}
	}
	return ret;
}

/*!
  Insert a span of text into the document
 \param b Buffer containing UCS text to insert

 Uses appropriate function for clipboard or file
 */
bool IE_Imp_Text::_insertSpan(UT_GrowBuf &b)
{
	UT_uint32 iLength = b.getLength();
	const UT_UCS4Char * pData = (const UT_UCS4Char *)b.getPointer(0);

	// handle block direction if needed ...
	if(pData && m_bBlockDirectionPending)
	{
		const UT_UCS4Char * p = pData;

		// we look for the first strong character
		for(UT_uint32 i = 0; i < iLength; i++, p++)
		{
			UT_BidiCharType type = UT_bidiGetCharType(*p);

			if(UT_BIDI_IS_STRONG(type))
			{
				m_bBlockDirectionPending = false;

				// set 'dom-dir' property of the block ...
				const gchar * propsArray[3];
				propsArray[0] = "props";
				propsArray[1] = NULL;
				propsArray[2] = NULL;

				UT_String props("dom-dir:");
				
				if(UT_BIDI_IS_RTL(type))
					props += "rtl;text-align:right";
				else
					props += "ltr;text-align:left";

				propsArray[1] = props.c_str();
				
				// we need to modify the existing formatting ...
				if(m_pBlock == NULL)
				{
					pf_Frag_Strux* sdh = NULL;
					if(getDoc()->getStruxOfTypeFromPosition(getDocPos(), PTX_Block,&sdh))
					{
						m_pBlock = sdh;
					}
				}
				appendStruxFmt(m_pBlock, static_cast<const gchar **>(&propsArray[0]));
			
				// if this is the first data in the block and the first
				// character is LRM or RLM followed by a strong character,
				// then we will remove it
				if(m_bFirstBlockData && i==0 && iLength > 1 && (*p == UCS_LRM || *p == UCS_RLM))
				{
					UT_BidiCharType next_type = UT_bidiGetCharType(*(p+1));
					if(UT_BIDI_IS_STRONG(next_type))
					{
						pData++;
						iLength--;
					}
				}
				
				break;
			}
		}
	}
	
	bool bRes = appendSpan (pData, iLength);
	b.truncate(0);
	m_bFirstBlockData = false;
	return bRes;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_Text_Sniffer::IE_Imp_Text_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_TEXT, true)
{
	// 
}

IE_Imp_Text_Sniffer::~IE_Imp_Text_Sniffer ()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_Text_Sniffer__SuffixConfidence[] = {
	{ "txt", 	UT_CONFIDENCE_PERFECT 	},
	{ "text", 	UT_CONFIDENCE_PERFECT 	},
	{ "doc", 	UT_CONFIDENCE_POOR 		},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_Text_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_Text_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_Text_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	IE_MIMETYPE_Text, 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_CLASS, 	"text", 			UT_CONFIDENCE_SOSO 	}, 
	{ IE_MIME_MATCH_BOGUS, 	"", 				UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_Text_Sniffer::getMimeConfidence ()
{
	return IE_Imp_Text_Sniffer__MimeConfidence;
}

/*!
  Check if buffer contains data meant for this importer.

 We don't attmpt to recognize since other filetypes (HTML) can
 use the same encodings a text file can.
 We also don't want to steal recognition when user wants to use
 the Encoded Text importer.
 */
UT_Confidence_t IE_Imp_Text_Sniffer::recognizeContents(const char * szBuf,
													   UT_uint32 iNumbytes)
{
  if (_recognizeUTF8 (szBuf, iNumbytes))
	  return UT_CONFIDENCE_PERFECT - 1; // it's UTF-8, but might not be plaintext
  else if (UE_NotUCS != _recognizeUCS2(szBuf, iNumbytes, false))
	  return UT_CONFIDENCE_PERFECT - 1; // it's UTF-8, but might not be plaintext
  return UT_CONFIDENCE_POOR; // anything can be a text document - we'll unfairly weight this down
}


/*!
  This sniffer is useful to convert text from the clipbaord.
 */
const char * IE_Imp_Text_Sniffer::recognizeContentsType(const char * szBuf,
													   UT_uint32 iNumbytes)
{
  if (_recognizeUTF8 (szBuf, iNumbytes))
  {
	  return "UTF-8";
  }
  else if (UE_BigEnd == _recognizeUCS2(szBuf, iNumbytes, false))
  {
	  return XAP_EncodingManager::get_instance()->getUCS2BEName();
  }
  else if (UE_LittleEnd == _recognizeUCS2(szBuf, iNumbytes, false))
  {
	  return XAP_EncodingManager::get_instance()->getUCS2LEName();
  }
  return "none";
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

		if (*p == 0) return false;			// ??

		if ((*p & 0x80) == 0)				// ASCII
		{
			++p;
			continue;
		}
		if ((*p & 0xc0) == 0x80)			// not UTF-8
		{
			return false;
		}
		if (*p == 0xfe || *p == 0xff)
		{
			// BOM shouldn't occur in UTF-8 - file may be UCS-2
			return false;
		}

		if ((*p & 0xfe) == 0xfc)				// lead byte in 6-byte sequence
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
		  UT_ASSERT_NOT_REACHED();
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

IE_Imp_EncodedText_Sniffer::IE_Imp_EncodedText_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_TEXTENC)
{
	// 
}

IE_Imp_EncodedText_Sniffer::~IE_Imp_EncodedText_Sniffer ()
{
}

// supported suffixes
// We don't attempt to recognize.  User must specifically choose Encoded Text.
static IE_SuffixConfidence IE_Imp_EncodedText_Sniffer__SuffixConfidence[] = {
	{ "txt", 	UT_CONFIDENCE_POOR 		},
	{ "text", 	UT_CONFIDENCE_POOR 		},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_EncodedText_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_EncodedText_Sniffer__SuffixConfidence;
}

/*!
  Check if buffer contains data meant for this importer.

 We don't attempt to recognize.  User must specifically choose Encoded Text.
 */
UT_Confidence_t IE_Imp_EncodedText_Sniffer::recognizeContents(const char * /* szBuf */,
															  UT_uint32 /* iNumbytes */)
{
	return UT_CONFIDENCE_ZILCH;
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
	*pszDesc = "Encoded Text (.txt, .text)";
	*pszSuffixList = "*.txt; *.text";
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
UT_Error IE_Imp_Text::_loadFile(GsfInput * fp)
{
	ImportStream *pStream = 0;
	UT_Error error;

	// First we try to determine the encoding.
	if (_recognizeEncoding(fp) == UT_OK)
		getDoc()->setEncodingName(m_szEncoding);

	// Call encoding dialog
	if (!m_bIsEncoded || m_bExplicitlySetEncoding || _doEncodingDialog(m_szEncoding))
	{
		X_CleanupIfError(error,_constructStream(pStream,fp));
		X_CleanupIfError(error,_writeHeader(fp));
		X_CleanupIfError(error,_parseStream(pStream));
		error = UT_OK;
	}
	else
		error = UT_ERROR;

Cleanup:
	delete pStream;
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
  : IE_Imp(pDocument),
    m_szEncoding(0),
    m_bExplicitlySetEncoding(false),
    m_bIsEncoded(false),
    m_bIs16Bit(false),
    m_bUseBOM(false),
	m_bBigEndian(false),
	m_bBlockDirectionPending(true),
	m_bFirstBlockData(true),
	m_pBlock(NULL)
{
	// Get encoding dialog prefs setting
	bool bAlwaysPrompt;
	XAP_App::getApp()->getPrefsValueBool(AP_PREF_KEY_AlwaysPromptEncoding, &bAlwaysPrompt);

	m_bIsEncoded = bAlwaysPrompt | bEncoded;

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	_setEncoding(szEncodingName);
}

IE_Imp_Text::IE_Imp_Text(PD_Document * pDocument, const char * encoding)
  : IE_Imp(pDocument),
    m_szEncoding(0),
    m_bExplicitlySetEncoding(false),
    m_bIsEncoded(false),
    m_bIs16Bit(false),
    m_bUseBOM(false),
	m_bBigEndian(false),
	m_bBlockDirectionPending(true),
	m_bFirstBlockData(true),
	m_pBlock(0)
{
  m_bIsEncoded = ((encoding != NULL) && (strlen(encoding) > 0));
  
  if ( m_bIsEncoded )
    {
      m_bExplicitlySetEncoding = true ;
      _setEncoding(encoding);
    }
}

IE_Imp_Text::~IE_Imp_Text ()
{
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
UT_Error IE_Imp_Text::_recognizeEncoding(GsfInput * fp)
{
	char szBuf[4096];  // 4096 ought to be enough
	UT_sint32 iNumbytes;

	iNumbytes = UT_MIN(4096, gsf_input_remaining(fp));
	gsf_input_read(fp, iNumbytes, (guint8 *)szBuf);
	gsf_input_seek(fp, 0, G_SEEK_SET);

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

		eUcs2 = IE_Imp_Text_Sniffer::_recognizeUCS2(szBuf, iNumbytes, false);

		if (eUcs2 == IE_Imp_Text_Sniffer::UE_BigEnd)
			_setEncoding(XAP_EncodingManager::get_instance()->getUCS2BEName());
		else if (eUcs2 == IE_Imp_Text_Sniffer::UE_LittleEnd)
			_setEncoding(XAP_EncodingManager::get_instance()->getUCS2LEName());
		else
			_setEncoding(
#ifdef TOOLKIT_WIN
				XAP_EncodingManager::get_instance()->getNative8BitEncodingName()
#else
				"ISO-8859-1"
#endif
			);
	}

	return UT_OK;
}

/*!
  Create a stream of the appropriate type
 \param pStream Pointer to created stream
 \param fp File to construct stream from

 Override this virtual function to derive from the text importer
 */
UT_Error IE_Imp_Text::_constructStream(ImportStream *& pStream, GsfInput * fp)
{
	return (pStream = new ImportStreamFile(fp)) ? UT_OK : UT_IE_NOMEMORY;
}

/*!
  Write header to document

 Writes the minimum needed Section and Block before we begin import
 */
UT_Error IE_Imp_Text::_writeHeader(GsfInput * /* fp */)
{
	// text gets applied in the Normal style
	const PP_PropertyVector propsArray = {
		"style", "Normal"
	};

	X_ReturnNoMemIfError(appendStrux(PTX_Section, PP_NOPROPS));
	X_ReturnNoMemIfError(appendStrux(PTX_Block, propsArray));

	pf_Frag * pf = getDoc()->getPieceTable()->getFragments().getLast();
	UT_return_val_if_fail( pf->getType() == pf_Frag::PFT_Strux, UT_ERROR);
	m_pBlock = (pf_Frag_Strux *) pf;
	UT_return_val_if_fail( m_pBlock->getStruxType() == PTX_Block, UT_ERROR );

	return UT_OK;
}

/*!
  Parse stream contents into the document
 \param stream Stream to import from

 This code is used for both files and the clipboard
 */
UT_Error IE_Imp_Text::_parseStream(ImportStream * pStream)
{
	UT_return_val_if_fail(pStream, UT_ERROR);

	bool bFirstChar = true;
	UT_GrowBuf gbBlock(1024);
	UT_UCSChar c;

	if (!m_bExplicitlySetEncoding) {
		std::string prop;

		prop = getProperty ("encoding");
		if (!prop.empty()) {
			_setEncoding (prop.c_str());
		}
	}

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
				X_ReturnNoMemIfError(_insertSpan(gbBlock));
			X_ReturnNoMemIfError(_insertBlock());
			break;

		case UCS_BOM:
			// This is Byte Order Mark at the start of file, Zero Width Non Joiner elsewhere
			if (bFirstChar)
				break;

		// if we encounter any of the following characters we will
		// substitute a '?' as they correspond to control characters,
		// though some text files use them for their character representations
		// We do this instead of of immediately returning an error
		// (and assuming they have no business in a text file) so we can
		// still show usable text to a user who has one of these files.
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x0004:
		case 0x0005:
		case 0x0006:
		case 0x0007:
		case 0x0008:
		case 0x000e:
		case 0x000f:
		case 0x0010:
		case 0x0011:
		case 0x0012:
		case 0x0013:
		case 0x0014:
		case 0x0015:
		case 0x0016:
		case 0x0017:
		case 0x0018:
		case 0x0019:
		case 0x001a:
		case 0x001b:
		case 0x001c:
		case 0x001d:
		case 0x001e:
		case 0x001f:
			// UT_ASSERT(!(c <= 0x001f));
			c = '?';
			/* return UT_ERROR; // fall through with modified character */
			
		default:
			X_ReturnNoMemIfError(gbBlock.append(reinterpret_cast<UT_GrowBufElement*>(&c),1));
			break;
		}
		bFirstChar = false;
	}

	if (gbBlock.getLength() > 0)
		X_ReturnNoMemIfError(_insertSpan(gbBlock));

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
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	XAP_Dialog_Encoding * pDialog
		= static_cast<XAP_Dialog_Encoding *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);

	pDialog->setEncoding(szEncoding);

	// run the dialog
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Encoding::a_OK);

	if (bOK)
	{
		const gchar * s;
		static gchar szEnc[16];

		s = pDialog->getEncoding();
		UT_return_val_if_fail (s, false);

		strcpy(szEnc,s);
		_setEncoding(static_cast<const char *>(szEnc));
		getDoc()->setEncodingName(szEnc);
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
	const char *szUCS2LEName = XAP_EncodingManager::get_instance()->getUCS2LEName();
	const char *szUCS2BEName = XAP_EncodingManager::get_instance()->getUCS2BEName();

	if (szEncoding && szUCS2LEName && !strcmp(szEncoding,szUCS2LEName))
	{
		m_bIs16Bit = true;
		m_bBigEndian = false;
#ifdef _WIN32
		m_bUseBOM = true;
#else
		m_bUseBOM = false;
#endif
	}
	else if (szEncoding && szUCS2BEName && !strcmp(szEncoding,szUCS2BEName))
	{
		m_bIs16Bit = true;
		m_bBigEndian = true;
#ifdef _WIN32
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

bool IE_Imp_Text::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  const unsigned char * pData, UT_uint32 lenData,
								  const char *szEncoding)
{
	UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
	UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);

	// Attempt to guess whether we're pasting 8 bit or unicode text
	if (szEncoding)
		_setEncoding(szEncoding);
	else
		_recognizeEncoding(reinterpret_cast<const char *>(pData), lenData);

	ImportStreamClipboard stream(pData, lenData);
	setClipboard (pDocRange->m_pos1);
	_parseStream(&stream);
	return true;
}

