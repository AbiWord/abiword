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
#include "ie_exp_UTF8.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

//////////////////////////////////////////////////////////////////
// a private listener class to help us translate the document
// into a UTF8 text stream.  code is at the bottom of this file.
//////////////////////////////////////////////////////////////////

class s_UTF8_Listener : public PL_Listener
{
public:
	s_UTF8_Listener(PD_Document * pDocument,
					IE_Exp_UTF8 * pie,
					UT_Bool bToClipboard);
	virtual ~s_UTF8_Listener();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual UT_Bool		signal(UT_uint32 iSignal);

protected:
	void				_closeBlock(void);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	
	PD_Document *		m_pDocument;
	IE_Exp_UTF8 *		m_pie;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bToClipboard;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_UTF8::IE_Exp_UTF8(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_UTF8::~IE_Exp_UTF8()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_UTF8::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".utf8") == 0);
}

IEStatus IE_Exp_UTF8::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_UTF8 * p = new IE_Exp_UTF8(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Exp_UTF8::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "UTF8 (.utf8)";
	*pszSuffixList = "*.utf8";
	*ft = IEFT_UTF8;
	return UT_TRUE;
}


UT_Bool IE_Exp_UTF8::SupportsFileType(IEFileType ft)
{
	return (IEFT_UTF8 == ft);
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_UTF8::_writeDocument(void)
{
	m_pListener = new s_UTF8_Listener(m_pDocument,this, (m_pDocRange!=NULL));
	if (!m_pListener)
		return IES_NoMemory;

	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_UTF8_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

#ifdef WIN32							// we need to generate CRLFs on Win32
	if (m_bToClipboard)					// when writing to the clipboard.  we
		m_pie->write("\r");				// use text mode when going to a file
#endif									// so we don't need to then.
	m_pie->write("\n");
	m_bInBlock = UT_FALSE;
	return;
}

void s_UTF8_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
#define MY_BUFFER_SIZE		1024
#define MY_HIGHWATER_MARK	20
	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
		{
			m_pie->write(buf,(pBuf-buf));
			pBuf = buf;
		}

		if (*pData > 0x007f)
		{
			const XML_Char * s = UT_encodeUTF8char(*pData++);
			while (*s)
				*pBuf++ = *s++;
		}
		else
		{
			// We let any UCS_LF's (forced line breaks) go out as is.
#ifdef WIN32
			if (m_bToClipboard && *pData==UCS_LF)
				*pBuf++ = '\r';
#endif
			*pBuf++ = (UT_Byte)*pData++;
		}
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

s_UTF8_Listener::s_UTF8_Listener(PD_Document * pDocument,
								 IE_Exp_UTF8 * pie,
								 UT_Bool bToClipboard)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bToClipboard = bToClipboard;
	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a block.
	// when going to a file we should not.
	m_bInBlock = m_bToClipboard;
}

s_UTF8_Listener::~s_UTF8_Listener()
{
	_closeBlock();
}

UT_Bool s_UTF8_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return UT_TRUE;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0
			// TODO decide how to indicate objects in text output.
			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				return UT_TRUE;

			case PTO_Field:
				return UT_TRUE;

			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
#else
			return UT_FALSE;
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return UT_TRUE;

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_UTF8_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
			return UT_TRUE;
		}

	case PTX_Block:
		{
			_closeBlock();
			m_bInBlock = UT_TRUE;
			return UT_TRUE;
		}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_UTF8_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_UTF8_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_UTF8_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}
