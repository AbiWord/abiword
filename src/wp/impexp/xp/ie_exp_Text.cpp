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

#include "ut_string_class.h"

#include "ap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"

#define MY_MB_LEN_MAX 6

//////////////////////////////////////////////////////////////////
// a private listener class to help us translate the document
// into a text stream.  code is at the bottom of this file.
//////////////////////////////////////////////////////////////////

class s_Text_Listener : public PL_Listener
{
public:
	s_Text_Listener(PD_Document * pDocument,
					IE_Exp_Text * pie,
					bool bToClipboard,
					const char *szEncoding,
					bool bIs16Bit,
					bool bUseBOM,
					bool bBigEndian);
	virtual ~s_Text_Listener() {};

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	void				_closeBlock(void);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_output8BitData(const UT_UCSChar * , UT_uint32 length);
	void				_output16BitData(const UT_UCSChar * , UT_uint32 length);
	
	PD_Document *		m_pDocument;
	IE_Exp_Text *		m_pie;
	bool				m_bInBlock;
	bool				m_bToClipboard;
	bool				m_bFirstWrite;
	UT_Wctomb 			m_wctomb;
	const char *		m_szEncoding;
	bool				m_bIs16Bit;
	bool				m_bBigEndian;
	bool				m_bUseBOM;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument, bool bEncoded)
	: IE_Exp(pDocument)
{
	UT_ASSERT(pDocument);

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	m_error = 0;
	m_pListener = NULL;
	m_bIsEncoded = bEncoded;

	// TODO Use persistent document encoding when it exists
	_setEncoding(szEncodingName);
}

/*!
  Destruct text exporter
 */
IE_Exp_Text::~IE_Exp_Text()
{
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

// TODO This function is also used for Copy and Paste.
// TODO We should really always Copy Unicode to the clipboard and have the
// TODO clipboard handle conversion.  Currently we use the same encoding
// TODO as when we're Saving a file.
// TODO We'll need a way of knowing which Unicode format each OS needs for its clipboard.

UT_Error IE_Exp_Text::_writeDocument(void)
{
	m_pListener = new s_Text_Listener(m_pDocument,this,(m_pDocRange!=NULL),m_szEncoding,m_bIs16Bit,m_bUseBOM,m_bBigEndian);
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

 This function should be identical to the one in ie_Imp_Text
 */
bool IE_Exp_Text::_doEncodingDialog(const char *szEncoding)
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

/*****************************************************************/
/*****************************************************************/

void s_Text_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	UT_UCSChar wcLineBreak[3] = {0,0,0};
	char mbLineBreak[MY_MB_LEN_MAX*2];
	UT_UCSChar *pWC = wcLineBreak;
	char *pMB = mbLineBreak;
	int mbLen;

	// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
	wcLineBreak[0] = '\r';
	wcLineBreak[1] = '\n';
#else
	wcLineBreak[0] = '\n';
#endif
	
	while (*pWC)
	{
		if (!m_wctomb.wctomb(pMB,mbLen,(wchar_t)*pWC))
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		++pWC;
		pMB += mbLen;
	}

	m_pie->write(mbLineBreak,pMB-mbLineBreak);
	m_bInBlock = false;
	return;
}

/*!
  Output text buffer to file
 \param data Buffer to output
 \param length Size of buffer

 Currently calls different functions for 8 bit and 16 bit encodings due to use
 of dynamically sized string classes which only support one character size.
 If we support UCS-4/UTF-32 in the future we'll need a third function.
 */
void s_Text_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (m_bFirstWrite)
	{
		UT_ASSERT(m_szEncoding);
		m_wctomb.setOutCharset(m_szEncoding);

		// TODO BOMs need separate code for UTF-7, UCS-4, etc
		if (m_bUseBOM)
		{
			if (m_bIs16Bit)
			{
				if (m_bBigEndian)
					m_pie->write("\xfe\xff",2);
				else
					m_pie->write("\xff\xfe",2);
			}
			else
			{
				// TODO There may be reason for using a BOM in UTF-8 text.
				// TODO I've seen MS software do it.
				m_pie->write("\xef\xbb\xbf",3);
			}
		}
		m_bFirstWrite = false;
	}

	// TODO some iconvs use a different string!
	if (m_bIs16Bit)
		_output16BitData(data, length);
	else
		_output8BitData(data, length);
}

/*!
  Output 8-bit text buffer to file
 \param data Buffer to output
 \param length Size of buffer

 Single byte and multi byte encodings are supported.
 Wide character encodings are not supported.
 The buffer must not contain NULL bytes.
 */
void s_Text_Listener::_output8BitData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String sBuf;
	const UT_UCSChar * pData;
	
	int mbLen;
	char pC[MY_MB_LEN_MAX];

	// TODO Do we still need this?
	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		if(!m_wctomb.wctomb(pC,mbLen,(wchar_t)*pData))
		{
			mbLen=1;
			pC[0]='?';
			m_wctomb.initialize();
		}
		if (mbLen>1)		
		{
			pC[mbLen]='\0';
			sBuf += pC;
		}
		else
		{
			// We let any UCS_LF's (forced line breaks) go out as is.
			// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
			if (pC[0]==UCS_LF)
				sBuf += '\r';
#endif
			sBuf += (char)pC[0];
		}
		pData++;
	}

	m_pie->write(sBuf.c_str(),sBuf.size());
}

/*!
  Output 16-bit text buffer to file
 \param data Buffer to output
 \param length Size of buffer

 Supports the UCS-2 encodings.  UCS-2 streams include NULL bytes.
 */
void s_Text_Listener::_output16BitData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_UCS2String sBuf;
	const UT_UCSChar * pData;
	
	int mbLen;
	UT_UCSChar c;

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		if(!m_wctomb.wctomb(reinterpret_cast<char *>(&c),mbLen,(wchar_t)*pData))
		{
			mbLen=2;
			// TODO U+FFFD "REPLACEMENT CHARACTER" is the
			// TODO correct unicode equivalent of '?' isn't it?
			// TODO Or is it U+25A0 "BLACK SQUARE"?
			c=0xFFFD;
			m_wctomb.initialize();
		}
		UT_ASSERT(mbLen==2);
		// We let any UCS_LF's (forced line breaks) go out as is.
		// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
		if (c==UCS_LF)
			sBuf += '\r';
#endif
		sBuf += c;

		pData++;
	}

	m_pie->write(reinterpret_cast<const char *>(sBuf.ucs_str()),sBuf.size()*sizeof(UT_UCSChar));
}

/*!
  Set exporter's encoding and related members
 \param szEncoding Encoding to export file into

 Decides endian and BOM policy based on encoding
 */
void IE_Exp_Text::_setEncoding(const char *szEncoding)
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

s_Text_Listener::s_Text_Listener(PD_Document * pDocument,
								 IE_Exp_Text * pie,
								 bool bToClipboard,
								 const char *szEncoding,
								 bool bIs16Bit,
								 bool bUseBOM,
								 bool bBigEndian)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bToClipboard = bToClipboard;
	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a block.
	// when going to a file we should not.
	m_bInBlock = m_bToClipboard;
	m_bFirstWrite = true;
	m_szEncoding = szEncoding;
	m_bIs16Bit = bIs16Bit;
	m_bUseBOM = bToClipboard ? false : bUseBOM;
	m_bBigEndian = bBigEndian;
}

bool s_Text_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 1
			// TODO decide how to indicate objects in text output.
			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
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

bool s_Text_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
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

bool s_Text_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_Text_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

bool s_Text_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}
