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


#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_exp_Text.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_Text::IE_Exp_Text(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
	m_lid = 0;
}

IE_Exp_Text::~IE_Exp_Text()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_Text::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".txt") == 0);
}

IEStatus IE_Exp_Text::StaticConstructor(PD_Document * pDocument,
										IE_Exp ** ppie)
{
	IE_Exp_Text * p = new IE_Exp_Text(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Exp_Text::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Text (.txt)";
	*pszSuffixList = "*.txt";
	*ft = IEFT_Text;
	return UT_TRUE;
}


UT_Bool IE_Exp_Text::SupportsFileType(IEFileType ft)
{
	return (IEFT_Text == ft);
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_Text::writeFile(const char * szFilename)
{
	UT_ASSERT(m_pDocument);
	UT_ASSERT(szFilename && *szFilename);

	if (!_openFile(szFilename))
		return IES_CouldNotOpenForWriting;

	IEStatus status = _writeDocument();
	if (status == IES_OK)
		_closeFile();
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return status;
}

void IE_Exp_Text::write(const char * sz)
{
	if (m_error)
		return;
	m_error |= ! _writeBytes((UT_Byte *)sz);
	return;
}

void IE_Exp_Text::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;
	if (_writeBytes((UT_Byte *)sz,length) != length)
		m_error = UT_TRUE;
	
	return;
}

/*****************************************************************/
/*****************************************************************/

class s_Text_Listener : public PL_Listener
{
public:
	s_Text_Listener(PD_Document * pDocument,
					IE_Exp_Text * pie);
	virtual ~s_Text_Listener();

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
	IE_Exp_Text *		m_pie;
	UT_Bool				m_bInBlock;
};

void s_Text_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_pie->write("\n");
	m_bInBlock = UT_FALSE;
	return;
}

void s_Text_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	// TODO deal with unicode.
	// TODO for now, just squish it into ascii.
	
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

		UT_ASSERT(*pData < 256);
		// We let any UCS_LF's (forced line breaks) go out as is.
		*pBuf++ = (UT_Byte)*pData++;
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

s_Text_Listener::s_Text_Listener(PD_Document * pDocument,
								 IE_Exp_Text * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInBlock = UT_FALSE;
}

s_Text_Listener::~s_Text_Listener()
{
	_closeBlock();
}

UT_Bool s_Text_Listener::populate(PL_StruxFmtHandle /*sfh*/,
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
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				_openTag("i","/",UT_FALSE,api);
				return UT_TRUE;

			case PTO_Field:
				_openTag("f","/",UT_FALSE,api);
				return UT_TRUE;

			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
#else
			return UT_FALSE;
#endif
		}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_Text_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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

UT_Bool s_Text_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_Text_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

UT_Bool s_Text_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}



/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_Text::_writeDocument(void)
{
	m_pListener = new s_Text_Listener(m_pDocument,this);
	if (!m_pListener)
		return IES_NoMemory;
	if (!m_pDocument->addListener(static_cast<PL_Listener *>(m_pListener),&m_lid))
		return IES_Error;
	m_pDocument->removeListener(m_lid);
	delete m_pListener;

	m_lid = 0;
	m_pListener = NULL;
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
}
