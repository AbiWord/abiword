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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_exp_XML2PS.h"
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
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"
#include "ap_Prefs.h"
#include "ut_string_class.h"
#ifdef WIN32
  #include "ut_Win32OS.h"
#endif
#include <fribidi.h>


/*****************************************************************/
/*****************************************************************/

IE_Exp_XML2PS::IE_Exp_XML2PS(PD_Document * pDocument, bool bEncoded)
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
	getDoc()->getApp()->getPrefsValueBool(AP_PREF_KEY_AlwaysPromptEncoding, &bAlwaysPrompt);

	m_bIsEncoded = bAlwaysPrompt | bEncoded;

	const char *szEncodingName = pDocument->getEncodingName();
	if (!szEncodingName || !*szEncodingName)
		szEncodingName = XAP_EncodingManager::get_instance()->getNativeEncodingName();

	_setEncoding(szEncodingName);
}

IE_Exp_XML2PS::IE_Exp_XML2PS(PD_Document * pDocument, const char * encoding)
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

/*****************************************************************/
/*****************************************************************/

IE_Exp_XML2PS_Sniffer::IE_Exp_XML2PS_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_TEXT, true)
{
	// 
}

UT_Confidence_t IE_Exp_XML2PS_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (szMIME, IE_MIME_Text) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	if (strncmp (szMIME, "text/xml2ps", 11) == 0)
		{
			return UT_CONFIDENCE_SOSO;
		}
	return UT_CONFIDENCE_ZILCH;
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_XML2PS_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".xml2ps"));
}

UT_Error IE_Exp_XML2PS_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_XML2PS * p = new IE_Exp_XML2PS(pDocument,false);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_XML2PS_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "XML2PSToo (.xml2ps )";
	*pszSuffixList = "*.xml2ps";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_EncodedXML2PS_Sniffer::IE_Exp_EncodedXML2PS_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_TEXTENC, false)
{
	// 
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_EncodedXML2PS_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".xml2ps"));
}

UT_Error IE_Exp_EncodedXML2PS_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_XML2PS * p = new IE_Exp_XML2PS(pDocument,true);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_EncodedXML2PS_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Encoded XML2PS (.xml2ps)";
	*pszSuffixList = "*.xml2ps";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

PL_Listener * IE_Exp_XML2PS::_constructListener(void)
{
	if (!m_bExplicitlySetEncoding) {
		const UT_UTF8String * prop;

		prop = getProperty ("encoding");
		if (prop) {
			_setEncoding (prop->utf8_str());
		}
	}

	return new XML2PS_Listener(getDoc(),this,(getDocRange()!=NULL),m_szEncoding,
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

UT_Error IE_Exp_XML2PS::_writeDocument(void)
{
	// TODO If we're going to the clipboard and the OS supports unicode, set encoding.
	// TODO Only supports Windows so far.
	// TODO Should use a finer-grain technique than IsWinNT() since Win98 supports unicode clipboard.
	if (getDocRange())
	{
#ifdef WIN32
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
  Open the file to export to
 \param szFilename File to open
 */
bool IE_Exp_XML2PS::_openFile(const char * szFilename)
{
	// Don't call base method if user cancels encoding dialog
	if (!m_bIsEncoded || m_bExplicitlySetEncoding || _doEncodingDialog(m_szEncoding))
		return IE_Exp::_openFile(szFilename);
	else
	{
		_cancelExport ();
		return false;
	}
}

/*!
  Request file encoding from user
 \param szEncoding Encoding to export file into

 This function should be identical to the one in ie_Imp_XML2PS
 */
bool IE_Exp_XML2PS::_doEncodingDialog(const char *szEncoding)
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_ENCODING;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(getDoc()->getApp()->getDialogFactory());

	XAP_Dialog_Encoding * pDialog
		= static_cast<XAP_Dialog_Encoding *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);

	pDialog->setEncoding(szEncoding);

	// run the dialog
	XAP_Frame * pFrame = getDoc()->getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Encoding::a_OK);

	if (bOK)
	{
		const XML_Char * s;
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
 This function should be identical to the one in IE_Imp_XML2PS.
 */
void IE_Exp_XML2PS::_setEncoding(const char *szEncoding)
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
		m_bUnicode = true;
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
		m_bUnicode = true;
	}
	else if(szEncoding && !UT_strnicmp(szEncoding,"UTF-",4))
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
void XML2PS_Listener::_genBOM(void)
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
void XML2PS_Listener::_genLineBreak(void)
{
	char *pMB = static_cast<char *>(m_mbLineBreak);
	const UT_UCSChar *pWC = 0;
	int mbLen = 0;

	// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
	static const UT_UCSChar wcLineBreak[3] = {'\r', '\n', 0};
#else
	static const UT_UCSChar wcLineBreak[3] = {'\n', 0, 0};
#endif

	for (pWC = wcLineBreak; *pWC; ++pWC)
	{
		if (_wctomb(pMB,mbLen,*pWC))
			pMB += mbLen;
		else
		  UT_ASSERT_NOT_REACHED();
	}

	m_iLineBreakLen = pMB - m_mbLineBreak;

	UT_ASSERT_HARMLESS(m_iLineBreakLen && m_iLineBreakLen < 20);
}




/*!
  Output text buffer to stream
 \param data Buffer to output
 \param length Size of buffer
 */
void XML2PS_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
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

void XML2PS_Listener::_openBlock(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;

	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;
	const char* pszParaAlign = NULL;
	const char* pszParaLineHeight = NULL;

        UT_UTF8String TempStr;

        double pszLeftMarginDouble, pszRightMarginDouble, pszTopMarginDouble,pszBottomMarginDouble;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

//	Get a bunch of block level and other default properties.

	pAP->getProperty("page-margin-left", (const XML_Char *&)pszLeftMargin);
	pAP->getProperty("page-margin-right", (const XML_Char *&)pszRightMargin);
	pAP->getProperty("page-margin-top", (const XML_Char *&)pszTopMargin);
	pAP->getProperty("page-margin-bottom", (const XML_Char *&)pszBottomMargin);
        pAP->getProperty("font-family", (const XML_Char *&) pszFontFamily);
        pAP->getProperty("font-size", (const XML_Char *&) pszFontSize );
	pAP->getProperty("text-align", (const XML_Char *&) pszParaAlign );
	pAP->getProperty("line-height", (const XML_Char *&) pszParaLineHeight );


//	Manipulate them a little. Not sure this is needed.

        pszLeftMarginDouble   = UT_convertToPoints(pszLeftMargin);
        pszRightMarginDouble  = UT_convertToPoints(pszRightMargin);
        pszTopMarginDouble    = UT_convertToPoints(pszTopMargin);
        pszBottomMarginDouble = UT_convertToPoints(pszBottomMargin);

	m_bInBlock = true;

	if (bHaveProp && pAP)
	{
		m_pie->write("<para");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszParaAlign != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" align=\"%s\"", pszParaAlign );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszParaLineHeight != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" line-height=\"%s\"", pszParaLineHeight );
                   m_pie->write(TempStr.utf8_str());
                 }
               	if( pszTopMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-top=\"%gpt\"", pszTopMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszBottomMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-bottom=\"%gpt\"", pszBottomMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszLeftMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-right=\"%gpt\"", pszRightMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszRightMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-left=\"%gpt\"", pszLeftMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
                m_pie->write(">");
	}
        else
        {
		m_pie->write("<para>\n");
	}

}
void XML2PS_Listener::_openFont(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;

        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;

        UT_UTF8String TempStr;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

//	Get a bunch of block level and other default properties.

        pAP->getProperty("font-family", (const XML_Char *&) pszFontFamily);
        pAP->getProperty("font-size", (const XML_Char *&) pszFontSize );

        m_inFont = true;

	if (bHaveProp && pAP)
	{
		m_pie->write("<font");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
                m_pie->write(">");
	}
        else
        {
		m_pie->write("<font>\n");
	}

}

void XML2PS_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

        if (m_inFont)
          _closeFont();

	if (!m_bFirstWrite)
		m_pie->write("</para>\n");

	m_bInBlock = false;
	m_eDirOverride = DO_UNSET;
	m_eDirMarkerPending = DO_UNSET;
	return;
}
void XML2PS_Listener::_closeFont(void)
{
	if (!m_inFont)
		return;

	if (!m_bFirstWrite)
		m_pie->write("</font>");

	m_inFont = false;
	m_eDirOverride = DO_UNSET;
	m_eDirMarkerPending = DO_UNSET;
	return;
}



void XML2PS_Listener::_handleDirMarker(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	
	UT_UCS4Char * pMarker = NULL;
		
	if (bHaveProp && pAP)
	{
		UT_UCS4Char cRLO = UCS_RLO;
		UT_UCS4Char cLRO = UCS_LRO;
		UT_UCS4Char cPDF = UCS_PDF;

		const XML_Char *szValue = NULL;
		if(pAP->getProperty("dir-override", szValue))
		{
			if(m_eDirOverride == DO_UNSET)
			{
				if(!UT_stricmp(szValue, "rtl"))
				{
					m_eDirOverride = DO_RTL;
					pMarker = &cRLO;
				}
				else if(!UT_stricmp(szValue, "ltr"))
				{
					m_eDirOverride = DO_LTR;
					pMarker = &cLRO;
				}
			}
			else if(m_eDirOverride == DO_RTL)
			{
				if(!UT_stricmp(szValue, "rtl"))
				{
					// no change
				}
				else if(!UT_stricmp(szValue, "ltr"))
				{
					m_eDirOverride = DO_LTR;
					pMarker = &cLRO;
				}
			}
			else if(m_eDirOverride == DO_LTR)
			{
				if(!UT_stricmp(szValue, "ltr"))
				{
					// no change
				}
				else if(!UT_stricmp(szValue, "rtl"))
				{
					m_eDirOverride = DO_RTL;
					pMarker = &cRLO;
				}
			}
			else
			{
				UT_DEBUGMSG(("XML2PS_Listener::_handleDirMarker: dir-override value '%s'\n",
							 szValue));
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
		UT_DEBUGMSG(("XML2PS_Listener::_handleDirMarker: no props! (bHaveProp %d, pAP 0x%x)\n",
					 bHaveProp, pAP));
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
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



XML2PS_Listener::XML2PS_Listener(PD_Document * pDocument,
							 IE_Exp_XML2PS * pie,
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
	  m_bToClipboard(bToClipboard),
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
	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;
        
        const char* FontFamilyStr = NULL;
        const char* FontSizeStr = NULL;

        UT_UTF8String TempStr;

        double pszLeftMarginDouble, pszRightMarginDouble, pszTopMarginDouble,pszBottomMarginDouble;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);


	m_inFont	= false;
	m_inParagraph	= false;


//	Get a bunch of block level and other default properties.

	pAP->getProperty("margin-left", (const XML_Char *&)pszLeftMargin);
	pAP->getProperty("margin-right", (const XML_Char *&)pszRightMargin);
	pAP->getProperty("margin-top", (const XML_Char *&)pszTopMargin);
	pAP->getProperty("margin-bottom", (const XML_Char *&)pszBottomMargin);
        pAP->getProperty("font-family", (const XML_Char *&) pszFontFamily);
        pAP->getProperty("font-size", (const XML_Char *&) pszFontSize );


//	Manipulate them a little. Not sure this is needed.

        pszLeftMarginDouble   = UT_convertToPoints(pszLeftMargin);
        pszRightMarginDouble  = UT_convertToPoints(pszRightMargin);
        pszTopMarginDouble    = UT_convertToPoints(pszTopMargin);
        pszBottomMarginDouble = UT_convertToPoints(pszBottomMargin);

        m_pie->write("<?xml version=\"1.0\" ?>\n");
	if (bHaveProp && pAP)
	{
		m_pie->write("<block-container");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
		if( pszTopMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-top=\"%gpt\"", pszTopMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszBottomMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-bottom=\"%gpt\"", pszBottomMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszRightMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-right=\"%gpt\"", pszRightMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszLeftMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-left=\"%gpt\"", pszLeftMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
	                                   
                m_pie->write(">");
	}
        else
        {
		m_pie->write("<block-container>\n");
	}


	if (bHaveProp && pAP)
	{
		const XML_Char *szValue = NULL;
		if(pAP->getProperty("dom-dir", szValue))
		{
			if(!UT_stricmp("rtl",szValue))
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

/***************************************************************/
/***************************************************************/

XML2PS_Listener::~XML2PS_Listener()
{
  _closeFont();
  _closeBlock();
  m_pie->write("\n</block-container>");
}

/***************************************************************/
/***************************************************************/

bool XML2PS_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_closeFont();
			_openFont(api);

			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCS4Char * pData = m_pDocument->getPointer(bi);
			_outputData(pData,pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool XML2PS_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
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
				const XML_Char *szValue = NULL;
				if(pAP->getProperty("dom-dir", szValue))
				{
					if(!UT_stricmp("rtl",szValue))
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
			PT_AttrPropIndex api = pcr->getIndexAP();
			_closeFont();
			_closeBlock();
			_openBlock(api);
			m_bInBlock = true;
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
	    return true ;

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool XML2PS_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool XML2PS_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool XML2PS_Listener::signal(UT_uint32 /* iSignal */)
{
  UT_ASSERT_NOT_REACHED();
	return false;
}
