/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_impexp_Text.h"
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
#include "xap_App.h"
#include "xap_EncodingManager.h"
#include "ap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"
#include "ap_Prefs.h"
#include "ut_string_class.h"
#ifdef TOOLKIT_WIN
  #include "ut_Win32OS.h"
#endif


/*****************************************************************/
/*****************************************************************/

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument, bool bEncoded)
	: IE_Exp(pDocument),
	  m_pListener(NULL),
	  m_bIsEncoded(false),
	  m_szEncoding(0),
	  m_bExplicitlySetEncoding(false),
	  m_bIs16Bit(false),
	  m_bUnicode(false),
	  m_bBigEndian(false),
	  m_bUseBOM(false)
{
	m_error = UT_OK;

	// Get encoding dialog prefs setting
	bool bAlwaysPrompt = false;
	XAP_App::getApp()->getPrefsValueBool(AP_PREF_KEY_AlwaysPromptEncoding, &bAlwaysPrompt);

	m_bIsEncoded = bAlwaysPrompt | bEncoded;

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	_setEncoding(szEncodingName);
}

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument, const char * encoding)
  : IE_Exp(pDocument),
    m_pListener(NULL),
    m_bIsEncoded(false),
    m_szEncoding(0),
    m_bExplicitlySetEncoding(false),
    m_bIs16Bit(false),
    m_bUnicode(false),
    m_bBigEndian(false),
	m_bUseBOM(false)
{
  m_error = UT_OK;
  
  m_bIsEncoded = ((encoding != NULL) && (strlen(encoding) > 0));
  
  if ( m_bIsEncoded )
    {
      m_bExplicitlySetEncoding = true;
     _setEncoding(encoding);
    }
}

IE_Exp_Text::~IE_Exp_Text ()
{
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_Text_Sniffer::IE_Exp_Text_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_TEXT, true)
{
	// 
}

IE_Exp_Text_Sniffer::~IE_Exp_Text_Sniffer ()
{
}

UT_Confidence_t IE_Exp_Text_Sniffer::supportsMIME (const char * szMIME)
{
	if (strcmp (szMIME, IE_MIMETYPE_Text) == 0)
		{
			return UT_CONFIDENCE_PERFECT;
		}
	if (strncmp (szMIME, "text/", 5) == 0)
		{
			return UT_CONFIDENCE_SOSO;
		}
	return UT_CONFIDENCE_ZILCH;
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_Text_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".txt") || !g_ascii_strcasecmp(szSuffix, ".text"));
}

UT_Error IE_Exp_Text_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	*ppie = new IE_Exp_Text(pDocument,"UTF-8");
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

IE_Exp_EncodedText_Sniffer::IE_Exp_EncodedText_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_TEXTENC, false)
{
	// 
}

IE_Exp_EncodedText_Sniffer::~IE_Exp_EncodedText_Sniffer ()
{
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_EncodedText_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".txt") || !g_ascii_strcasecmp(szSuffix, ".text"));
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
	*pszDesc = "Encoded Text (.txt, .text)";
	*pszSuffixList = "*.txt; *.text";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

PL_Listener * IE_Exp_Text::_constructListener(void)
{
	if (!m_bExplicitlySetEncoding) {
		const std::string & prop = getProperty ("encoding");
		if (!prop.empty()) {
			_setEncoding (prop.c_str());
		}
	}

	return new Text_Listener(getDoc(),this,(getDocRange()!=NULL),m_szEncoding,
							 m_bIs16Bit,m_bUnicode,m_bUseBOM,m_bBigEndian);
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
	// Don't call base method if user cancels encoding dialog
	if (!(!m_bIsEncoded || m_bExplicitlySetEncoding || _doEncodingDialog(m_szEncoding)))
		return UT_SAVE_CANCELLED;

	// TODO If we're going to the clipboard and the OS supports unicode, set encoding.
	// TODO Only supports Windows so far.
	// TODO Should use a finer-grain technique than IsWinNT() since Win98 supports unicode clipboard.
	if (getDocRange())
	{
#ifdef _WIN32
		if (UT_IsWinNT())
			_setEncoding(XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName());
#endif
	}

	m_pListener = _constructListener();
	if (!m_pListener)
		return UT_IE_NOMEMORY;

	if (getDocRange())
		getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),getDocRange());
	else
		getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);

	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
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
		static UT_String szEnc;

		s = pDialog->getEncoding();
		UT_return_val_if_fail (s, false);

		szEnc = s;
		_setEncoding(szEnc.c_str());
		getDoc()->setEncodingName(szEnc.c_str());
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
		m_bUnicode = true;
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
		m_bUnicode = true;
	}
	else if(szEncoding && !g_ascii_strncasecmp(szEncoding,"UTF-",4))
	{
		// TODO -- can encoding be utf-16 or utf-32?
		m_bIs16Bit = false;
		m_bBigEndian = false;
		m_bUseBOM = false;
		m_bUnicode = true;
	}
	else
	{
		m_bIs16Bit = false;
		// These are currently meaningless when not in a Unicode encoding
		m_bBigEndian = false;
		m_bUseBOM = false;
		m_bUnicode = false;
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
}

/*!
  Generate correct line break characters

 Makes a line break correct for the encoding and platform.
 */
void Text_Listener::_genLineBreak(void)
{
	char *pMB = static_cast<char *>(m_mbLineBreak);
	const UT_UCSChar *pWC = 0;
	int mbLen = 0;

	// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef _WIN32
	static const UT_UCSChar wcLineBreak[3] = {'\r', '\n', 0};
#else
	static const UT_UCSChar wcLineBreak[3] = {'\n', 0, 0};
#endif

	for (pWC = wcLineBreak; *pWC; ++pWC)
	{
		if (_wctomb(pMB,mbLen,*pWC))
		{
			pMB += mbLen;
		}
		else
		{
		  UT_ASSERT_NOT_REACHED();
		}
	}

	m_iLineBreakLen = pMB - m_mbLineBreak;

	UT_ASSERT_HARMLESS(m_iLineBreakLen && m_iLineBreakLen < 20);
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
			if (!_wctomb(pC,mbLen,*pData))
			{
				UT_ASSERT_HARMLESS(!m_bIs16Bit);
				mbLen=1;
				pC[0]='?';
				m_wctomb.initialize();
			}
			UT_ASSERT_HARMLESS(mbLen>=1);
			bBuf.append(reinterpret_cast<const UT_Byte *>(pC),mbLen);
		}
	}

	m_pie->write(reinterpret_cast<const char *>(bBuf.getPointer(0)),bBuf.getLength());
}

void Text_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	if (!m_bFirstWrite)
	{
		m_pie->write(static_cast<const char *>(m_mbLineBreak),m_iLineBreakLen);
		if (m_bBreakExtra)
			m_pie->write(static_cast<const char *>(m_mbLineBreak),m_iLineBreakLen);
	}

	m_bInBlock = false;
	m_eDirOverride = DO_UNSET;
	m_eDirMarkerPending = DO_UNSET;
	return;
}

void Text_Listener::_handleDirMarker(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	
	UT_UCS4Char * pMarker = NULL;
		
	if (bHaveProp && pAP)
	{
		UT_UCS4Char cRLO = UCS_RLO;
		UT_UCS4Char cLRO = UCS_LRO;
		UT_UCS4Char cPDF = UCS_PDF;

		const gchar *szValue = NULL;
		if(pAP->getProperty("dir-override", szValue))
		{
			if(m_eDirOverride == DO_UNSET)
			{
				if(!g_ascii_strcasecmp(szValue, "rtl"))
				{
					m_eDirOverride = DO_RTL;
					pMarker = &cRLO;
				}
				else if(!g_ascii_strcasecmp(szValue, "ltr"))
				{
					m_eDirOverride = DO_LTR;
					pMarker = &cLRO;
				}
			}
			else if(m_eDirOverride == DO_RTL)
			{
				if(!g_ascii_strcasecmp(szValue, "rtl"))
				{
					// no change
				}
				else if(!g_ascii_strcasecmp(szValue, "ltr"))
				{
					m_eDirOverride = DO_LTR;
					pMarker = &cLRO;
				}
			}
			else if(m_eDirOverride == DO_LTR)
			{
				if(!g_ascii_strcasecmp(szValue, "ltr"))
				{
					// no change
				}
				else if(!g_ascii_strcasecmp(szValue, "rtl"))
				{
					m_eDirOverride = DO_RTL;
					pMarker = &cRLO;
				}
			}
			else
			{
				UT_DEBUGMSG(("Text_Listener::_handleDirMarker: dir-override value '%s'\n",
							 szValue));
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			if(m_eDirOverride != DO_UNSET)
			{
				m_eDirOverride = DO_UNSET;
				pMarker = &cPDF;
			}
		}
	}
	else
	{
		UT_DEBUGMSG(("Text_Listener::_handleDirMarker: no props! (bHaveProp %d, pAP %p)\n",
					 bHaveProp, pAP));
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	}

	// first see if there is a pending marker (for block direction)
	// and handle it
	if(pMarker && m_eDirMarkerPending != DO_UNSET)
	{
		UT_UCS4Char cRLM = UCS_RLM;
		UT_UCS4Char cLRM = UCS_LRM;

		if(m_eDirMarkerPending == DO_RTL && *pMarker == UCS_RLO)
		{
			//the override corresponds to the marker, no marker needed
			m_eDirMarkerPending = DO_UNSET;
		}
		else if(m_eDirMarkerPending == DO_RTL && *pMarker == UCS_LRO)
		{
			//need to issue marker
			_outputData(&cRLM, 1);
			m_eDirMarkerPending = DO_UNSET;
		}
		else if(m_eDirMarkerPending == DO_LTR && *pMarker == UCS_LRO)
		{
			//the override corresponds to the marker, no marker needed
			m_eDirMarkerPending = DO_UNSET;
		}
		else if(m_eDirMarkerPending == DO_LTR && *pMarker == UCS_RLO)
		{
			//need to issue marker
			_outputData(&cLRM, 1);
			m_eDirMarkerPending = DO_UNSET;
		}
	}

	if(pMarker)
	{
		_outputData(pMarker, 1);
	}
}



Text_Listener::Text_Listener(PD_Document * pDocument,
							 IE_Exp_Text * pie,
							 bool bToClipboard,
							 const char *szEncoding,
							 bool bIs16Bit,
							 bool bUnicode,
							 bool bUseBOM,
							 bool bBigEndian)
	: m_pDocument(pDocument),
	  m_pie(pie),
	  m_wctomb(XAP_EncodingManager::get_instance()->getNative8BitEncodingName()),
	  // when we are going to the clipboard, we should implicitly
	  // assume that we are starting in the middle of a block.
	  // when going to a file we should not.
	  m_iBOMLen(0),
	  m_iLineBreakLen(0),
	  m_bInBlock(bToClipboard),
	  m_bFirstWrite(true),
	  m_szEncoding(szEncoding),
	  m_bIs16Bit(bIs16Bit),
	  m_bBigEndian(bBigEndian),
	  m_bUnicode(bUnicode),
	  m_bUseBOM(bToClipboard ? false : bUseBOM),
	  m_bBreakExtra(false),
	  m_eDirOverride(DO_UNSET),
	  m_eDirMarkerPending(DO_UNSET),
	  m_eSectionDir(DO_UNSET),
	  m_eDocDir(DO_UNSET)
{
	PT_AttrPropIndex api = m_pDocument->getAttrPropIndex();
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (bHaveProp && pAP)
	{
		const gchar *szValue = NULL;
		if(pAP->getProperty("dom-dir", szValue))
		{
			if(!g_ascii_strcasecmp("rtl",szValue))
			{
				m_eDocDir = DO_RTL;
			}
			else
			{
				m_eDocDir = DO_LTR;
			}
		}
		else
		{
			// something wrong, default to LTR
			m_eSectionDir = DO_LTR;
		}
				
	}
}

bool Text_Listener::populate(fl_ContainerLayout* /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_handleDirMarker(api);

			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCS4Char * pData = m_pDocument->getPointer(bi);

			// first see if there is a pending marker (for block direction)
			// and handle it
			if(pData && m_eDirMarkerPending != DO_UNSET)
			{
				UT_UCS4Char cRLM = UCS_RLM;
				UT_UCS4Char cLRM = UCS_LRM;

				UT_BidiCharType type = UT_bidiGetCharType(*pData);
				
				if(m_eDirMarkerPending == DO_RTL && type == UT_BIDI_RTL)
				{
					//the override corresponds to the marker, no marker needed
					m_eDirMarkerPending = DO_UNSET;
				}
				else if(m_eDirMarkerPending == DO_RTL && type == UT_BIDI_LTR)
				{
					//need to issue marker
					_outputData(&cRLM, 1);
					m_eDirMarkerPending = DO_UNSET;
				}
				else if(m_eDirMarkerPending == DO_LTR && type == UT_BIDI_LTR)
				{
					//the override corresponds to the marker, no marker needed
					m_eDirMarkerPending = DO_UNSET;
				}
				else if(m_eDirMarkerPending == DO_LTR && type == UT_BIDI_RTL)
				{
					//need to issue marker
					_outputData(&cLRM, 1);
					m_eDirMarkerPending = DO_UNSET;
				}
			}
			
			_outputData(pData,pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
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
				UT_return_val_if_fail(field, false);

				m_pie->populateFields ();
				if(field->getValue() != NULL) {
					UT_UCS4String ws(field->getValue());
					_outputData(ws.ucs4_str(),ws.length());
				}

				return true;
			
			case PTO_Bookmark:
				return true;
			
			case PTO_Hyperlink:
				return true;

			case PTO_Embed:
				return true;

			case PTO_Math:
				return true;

			case PTO_Annotation:
				return true;
                
			case PTO_RDFAnchor:
				return true;

			default:
				UT_ASSERT_HARMLESS(UT_TODO);
				return true;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT_HARMLESS(0);
		return false;
	}
}

bool Text_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   fl_ContainerLayout* * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_SectionEndnote:
	case PTX_SectionHdrFtr:
	case PTX_Section:
		{
			_closeBlock();
			PT_AttrPropIndex api = pcr->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

			if (bHaveProp && pAP)
			{
				const gchar *szValue = NULL;
				if(pAP->getProperty("dom-dir", szValue))
				{
					if(!g_ascii_strcasecmp("rtl",szValue))
					{
						m_eSectionDir = DO_RTL;
					}
					else
					{
						m_eSectionDir = DO_LTR;
					}
				}
				else
				{
					m_eSectionDir = DO_UNSET;
				}
				
			}
			return true;
		}

	case PTX_Block:
		{
			_closeBlock();
			m_bInBlock = true;

			const gchar * szValue = NULL;

			PT_AttrPropIndex api = pcr->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

			m_bBreakExtra = false;

			if (bHaveProp && pAP)
			{
				szValue = PP_evalProperty ("margin-top", 0, pAP, 0, m_pDocument, true);
				if(szValue)
				{
					double inches = UT_convertToInches(szValue);
					if (!m_bFirstWrite && (inches > 0.01))
						m_pie->write(static_cast<const char *>(m_mbLineBreak),m_iLineBreakLen);
				}
				szValue = PP_evalProperty ("margin-bottom", 0, pAP, 0, m_pDocument, true);
				if(szValue)
				{
					double inches = UT_convertToInches(szValue);
					if (inches > 0.01)
						m_bBreakExtra = true;
				}
			}

			// in 16-bit encodings we sometimes have to issue LRM or
			// RLM to indicate the dominant direction of the block
			// (the actual insertion will be done in the subsequent
			// call to _outputData(), since most often the marker is
			// unnecessary
			if(m_bUnicode)
			{
				if (bHaveProp && pAP)
				{
					szValue = NULL;
					if(pAP->getProperty("dom-dir", szValue))
					{
						if(!g_ascii_strcasecmp("rtl",szValue))
						{
							m_eDirMarkerPending = DO_RTL;
						}
						else
						{
							m_eDirMarkerPending = DO_LTR;
						}
					}
					else if(m_eSectionDir != DO_UNSET)
					{
						m_eDirMarkerPending = m_eSectionDir;
					}
					else
					{
						m_eDirMarkerPending = m_eDocDir;
					}
				}
			}
			
			return true;
		}

		// Be nice about these until we figure out what to do with 'em
	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_EndTable:
	case PTX_EndCell:
	case PTX_EndFrame:
	case PTX_EndMarginnote:
	case PTX_EndFootnote:
	case PTX_SectionFrame:
	case PTX_SectionMarginnote:
	case PTX_SectionFootnote:
	case PTX_EndEndnote:
	case PTX_SectionTOC:
	case PTX_EndTOC:
	case PTX_SectionAnnotation:
	case PTX_EndAnnotation:
	    return true ;

	default:
		UT_ASSERT_HARMLESS(UT_TODO);
		return true;
	}
}

bool Text_Listener::change(fl_ContainerLayout* /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool Text_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 pf_Frag_Strux* /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																 PL_ListenerId /* lid */,
																 fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool Text_Listener::signal(UT_uint32 /* iSignal */)
{
  UT_ASSERT_NOT_REACHED();
	return false;
}

Text_Listener::~Text_Listener ()
{
}

