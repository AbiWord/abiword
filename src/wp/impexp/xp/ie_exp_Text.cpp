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
#include "ut_wctomb.h"

#include "ut_string_class.h"

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
					bool bToClipboard);
	virtual ~s_Text_Listener();

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
	bool				m_bBigEndian;
	bool				m_bUseBOM;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
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
	IE_Exp_Text * p = new IE_Exp_Text(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_Text_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Text (.txt, .text)";
	*pszSuffixList = "*.text; *.txt";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_Text::_writeDocument(void)
{
	m_pListener = new s_Text_Listener(m_pDocument,this, (m_pDocRange!=NULL));
	if (!m_pListener)
		return UT_IE_NOMEMORY;

	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_Text_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	// TODO All writes should be re-routed via iconv since UCS-2
	// TODO uses two bytes for each character.
	// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
	// TODO We always add an extra line break at the end of a file but shouldn't
#ifdef WIN32
	m_pie->write("\r\n");
#else
	m_pie->write("\n");
#endif
	m_bInBlock = false;
	return;
}

void s_Text_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (m_szEncoding && !strncmp(m_szEncoding,"UCS-2",5))
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

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	if (m_bFirstWrite)
	{
		if (m_szEncoding)
			m_wctomb.setOutCharset(m_szEncoding);
		if (m_bUseBOM)
		{
			// TODO There may be reason for using a BOM in UTF-8 text.
			// TODO I've seen MS software do it.
			m_pie->write("\xef\xbb\xbf",3);
		}
		m_bFirstWrite = false;
	}

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
			sBuf += pC;
		}
		else
		{
			// We let any UCS_LF's (forced line breaks) go out as is.
			// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
			if (pC[0]==UCS_LF)
				sBuf += "\r";
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
	const UT_UCSChar * pInData;
	char * pOutData;
	
	int mbLen;
	unsigned char pC[MY_MB_LEN_MAX];
	char * pConvertedData = 0;
	
	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	pConvertedData = new char[length * sizeof(UT_UCSChar)];
	pOutData = pConvertedData;

	UT_ASSERT(pConvertedData);

	if (m_bFirstWrite)
	{
		if (m_szEncoding)
			m_wctomb.setOutCharset(m_szEncoding);
		if (m_bUseBOM)
		{
			if (m_bBigEndian)
				m_pie->write("\xfe\xff",2);
			else
				m_pie->write("\xff\xfe",2);
		}
		m_bFirstWrite = false;
	}

	for (pInData=data; (pInData<data+length); /**/)
	{
		if(!m_wctomb.wctomb(reinterpret_cast<char *>(pC),mbLen,(wchar_t)*pInData))
		{
			// TODO U+FFFD "REPLACEMENT CHARACTER" is the
			// TODO correct unicode equivalent of '?' isn't it?
			mbLen=2;
			if (m_bBigEndian)
			{
				pC[0]=0xff;
				pC[1]=0xfd;
			}
			else
			{
				pC[0]=0xfd;
				pC[1]=0xff;
			}
			m_wctomb.initialize();
		}
		// We let any UCS_LF's (forced line breaks) go out as is.
		if (*pInData == UCS_LF)
		{
			// TODO Old Mac should use "\r".  Mac OSX should Use U+2028 or U+2029.
#ifdef WIN32
			// TODO Win needs to *insert* an extra CR character before the LF.
			// TODO The old 8-bit code used UT_String which could grow dynamically
			// TODO but the 16-bit code uses a fixed size buffer.
			// TODO What is an appropriate solution?
#endif
		}
		*pOutData++ = pC[0];
		*pOutData++ = pC[1];
		++pInData;
	}

	m_pie->write(pConvertedData,length * sizeof(UT_UCSChar));

	delete [] pConvertedData;
}

s_Text_Listener::s_Text_Listener(PD_Document * pDocument,
								 IE_Exp_Text * pie,
								 bool bToClipboard)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bToClipboard = bToClipboard;
	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a block.
	// when going to a file we should not.
	m_bInBlock = m_bToClipboard;
	m_bFirstWrite = true;
	m_szEncoding = 0;
	m_bBigEndian = true;
	m_bUseBOM = false;
}

/*!
  Destruct listener
 */
s_Text_Listener::~s_Text_Listener()
{
	_closeBlock();
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
