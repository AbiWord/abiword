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

#include <string.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_exp_Text.h"
#include "fd_Field.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "ut_iconv.h"
#include "ut_wctomb.h"
#include "xap_EncodingManager.h"
#include "ap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"

#ifdef WIN32
#include "ut_Win32OS.h"
#endif

/*****************************************************************/
/*****************************************************************/

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument, bool bEncoded)
	: IE_Exp(pDocument),
	  m_pListener(NULL),
	  m_bIsEncoded(bEncoded)
{
	UT_ASSERT(pDocument);

	m_error = 0;

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	_setEncoding(szEncodingName);
}

/*****************************************************************/
/*****************************************************************/

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_Text_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".txt") || !UT_stricmp(szSuffix, ".text"));
}

UT_Error IE_Exp_Text_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_Text * p = new IE_Exp_Text(pDocument,false);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_Text_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Text (.txt, .text)";
	*pszSuffixList = "*.txt; *.text";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_EncodedText_Sniffer::recognizeSuffix(const char * szSuffix)
{
	// TODO Change to ".txt" and ".text" when bug # is fixed.
	return (!UT_stricmp(szSuffix,".etxt") || !UT_stricmp(szSuffix, ".etext"));
}

UT_Error IE_Exp_EncodedText_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_Text * p = new IE_Exp_Text(pDocument,true);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_EncodedText_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	// TODO Change to ".txt" and ".text" when bug # is fixed.
	*pszDesc = "Encoded Text (.etxt, .etext)";
	*pszSuffixList = "*.etxt; *.etext";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

PL_Listener * IE_Exp_Text::_constructListener(void)
{
	return new Text_Listener(m_pDocument,this,(m_pDocRange!=NULL),m_szEncoding,m_bIs16Bit,m_bUseBOM,m_bBigEndian);
}

// TODO This function is also used for Copy and Paste.
// TODO We should really always Copy Unicode to the clipboard.
// TODO On NT, the OS will implicitly convert the clipboard from Unicode for old apps.
// TODO On 95/98/ME we should probably convert it ourselves during "Copy".
// TODO NT also automatically puts locale info on the clipboard based on the input locale,
// TODO but it would be better to use the document locale.
// TODO On 95/98/NT we need to put locale info on the clipboard manually anyway.
// TODO Unicode clipboard and localized clipboard support for non-Windows OSes.

UT_Error IE_Exp_Text::_writeDocument(void)
{
	// TODO If we're going to the clipboard and the OS supports unicode, set encoding.
	// TODO Only supports Windows so far.
	// TODO Should use a finer-grain technique than IsWinNT() since Win98 supports unicode clipboard.
	if (m_pDocRange)
	{
#ifdef WIN32
		if (UT_IsWinNT())
			_setEncoding(XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName());
#endif
	}

	m_pListener = _constructListener();
	if (!m_pListener)
		return UT_IE_NOMEMORY;

	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);

	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*!
  Open the file to export to
 \param szFilename File to open
 */
bool IE_Exp_Text::_openFile(const char * szFilename)
{
	// Don't call base method if user cancels encoding dialog
	if (!m_bIsEncoded || _doEncodingDialog(m_szEncoding))
		return IE_Exp::_openFile(szFilename);
	else
		return false;
}

/*!
  Request file encoding from user
 \param szEncoding Encoding to export file into

 This function should be identical to the one in ie_Imp_Text
 */
bool IE_Exp_Text::_doEncodingDialog(const char *szEncoding)
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_ENCODING;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(m_pDocument->getApp()->getDialogFactory());

	XAP_Dialog_Encoding * pDialog
		= static_cast<XAP_Dialog_Encoding *>(pDialogFactory->requestDialog(id));
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
		_setEncoding(static_cast<const char *>(szEnc));
		m_pDocument->setEncodingName(szEnc);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*!
  Set exporter's encoding and related members
 \param szEncoding Encoding to export file into

 Decides endian and BOM policy based on encoding.
 Set to 0 to handle raw bytes.
 This function should be identical to the one in IE_Imp_Text.
 */
void IE_Exp_Text::_setEncoding(const char *szEncoding)
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

/*****************************************************************/
/*****************************************************************/

/*!
  Generate correct BOM

 Makes a Byte Order Mark correct for the encoding.
 */
void Text_Listener::_genBOM(void)
{
	// TODO iconv (at least libiconv) actually converts BOM to nothing at all ):
#if 0
	UT_UCSChar wcBOM[2] = {0,0};
	UT_UCSChar *pWC;
	char *pMB = static_cast<char *>(m_mbBOM);
	int mbLen;

	wcBOM[0] = UCS_BOM;

	for (pWC = wcBOM; *pWC; ++pwC)
	{
		if (_wctomb(pMB,mbLen,static_cast<wchar_t>(*pWC)))
			pMB += mbLen;
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	m_iBOMLen = pMB - m_mbBOM;
#else
	// Hard-coded BOM values
	if (m_bIs16Bit)
	{
		// This code should cover UCS-2 and UTF-16, both endians
		if (m_bBigEndian)
		{
			strcpy(m_mbBOM,"\xfe\xff");
			m_iBOMLen = 2;
		}
		else
		{
			strcpy(m_mbBOM,"\xff\xfe");
			m_iBOMLen = 2;
		}
	}
	else
	{
		// This code covers UTF-8 only
		strcpy(m_mbBOM,"\xef\xbb\xbf");
		m_iBOMLen = 3;
	}
	// TODO UTF-7, UCS-4, UTF-32
#endif
}

/*!
  Generate correct line break characters

 Makes a line break correct for the encoding and platform.
 */
void Text_Listener::_genLineBreak(void)
{
	UT_UCSChar wcLineBreak[3] = {0,0,0};
	UT_UCSChar *pWC;
	char *pMB = static_cast<char *>(m_mbLineBreak);
	int mbLen;

	// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
	wcLineBreak[0] = '\r';
	wcLineBreak[1] = '\n';
#else
	wcLineBreak[0] = '\n';
#endif

	for (pWC = wcLineBreak; *pWC; ++pWC)
	{
		if (_wctomb(pMB,mbLen,static_cast<wchar_t>(*pWC)))
			pMB += mbLen;
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	m_iLineBreakLen = pMB - m_mbLineBreak;
	UT_ASSERT(m_iLineBreakLen);
}

/*!
  Output text buffer to stream
 \param data Buffer to output
 \param length Size of buffer
 */
void Text_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_ByteBuf bBuf;
	const UT_UCSChar * pData;

	int mbLen;
	char pC[MY_MB_LEN_MAX];

	if (m_bFirstWrite)
	{
		if (m_szEncoding)
			m_wctomb.setOutCharset(m_szEncoding);

		_genLineBreak();

		if (m_bUseBOM)
		{
			_genBOM();
			m_pie->write(static_cast<const char *>(m_mbBOM),m_iBOMLen);
		}

		m_bFirstWrite = false;
	}

	for (pData=data; (pData<data+length); ++pData)
	{
		// We let any UCS_LF's (forced line breaks) go out as is.
		if (*pData==UCS_LF)
			bBuf.append(reinterpret_cast<UT_Byte *>(m_mbLineBreak),m_iLineBreakLen);
		else
		{
			if (!_wctomb(pC,mbLen,static_cast<wchar_t>(*pData)))
			{
				UT_ASSERT(!m_bIs16Bit);
				mbLen=1;
				pC[0]='?';
				m_wctomb.initialize();
			}
			UT_ASSERT(mbLen>=1);
			bBuf.append(reinterpret_cast<const UT_Byte *>(pC),mbLen);
		}
	}

	m_pie->write(reinterpret_cast<const char *>(bBuf.getPointer(0)),bBuf.getLength());
}

void Text_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	UT_ASSERT(!m_bFirstWrite);
	UT_ASSERT(m_iLineBreakLen);

	m_pie->write(static_cast<const char *>(m_mbLineBreak),m_iLineBreakLen);

	m_bInBlock = false;
	return;
}

Text_Listener::Text_Listener(PD_Document * pDocument,
							 IE_Exp_Text * pie,
							 bool bToClipboard,
							 const char *szEncoding,
							 bool bIs16Bit,
							 bool bUseBOM,
							 bool bBigEndian)
	: m_pDocument(pDocument),
	  m_pie(pie),
	  // when we are going to the clipboard, we should implicitly
	  // assume that we are starting in the middle of a block.
	  // when going to a file we should not.
	  m_bInBlock(bToClipboard),
	  m_bToClipboard(bToClipboard),
	  m_bFirstWrite(true),
	  m_szEncoding(szEncoding),
	  m_bIs16Bit(bIs16Bit),
	  m_bBigEndian(bBigEndian),
	  m_bUseBOM(bToClipboard ? false : bUseBOM)
{
}

bool Text_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 1
			// TODO decide how to indicate objects in text output.

			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			//PT_AttrPropIndex api = pcr->getIndexAP();
			fd_Field* field;
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				return true;

			case PTO_Field:
				// Lossy, but pretty much unavoidable
				field = pcro->getField();
				UT_ASSERT(field);
//
// Sevior: This makes me really unconfortable. I this will oly work for piecetable
// fields
//
				if(field->getValue() != NULL)
					m_pie->write(field->getValue());

				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
#else
			return true;
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool Text_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeBlock();
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			_closeBlock();
			return true;
		}

	case PTX_Block:
		{
			_closeBlock();
			m_bInBlock = true;
			return true;
		}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool Text_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool Text_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool Text_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}
