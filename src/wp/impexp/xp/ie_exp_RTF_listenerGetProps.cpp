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

/******************************************************************
** This class is to be considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to gather information
** from the document (the font table and color table and anything
** else) that must be written to the rtf header.
******************************************************************/

#include "ut_string.h"
#include "ie_exp_RTF_listenerGetProps.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

void s_RTF_ListenerGetProps::_closeSection(void)
{
	if (!m_bInSection)
		return;
	
	m_bInSection = false;
	m_apiThisSection = 0;

	return;
}

void s_RTF_ListenerGetProps::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_bInBlock = false;
	m_apiThisBlock = 0;
	
	return;
}

void s_RTF_ListenerGetProps::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_bInSpan = false;
	return;
}

void s_RTF_ListenerGetProps::_openSpan(PT_AttrPropIndex apiSpan)
{
	if (m_bInSpan)
	{
		if (m_apiLastSpan == apiSpan)
			return;
		_closeSpan();
	}

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	m_pDocument->getAttrProp(apiSpan,&pSpanAP);

	_compute_span_properties(pSpanAP,pBlockAP,pSectionAP);
	
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;
	return;
}

void s_RTF_ListenerGetProps::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (m_pie->m_bNeedUnicodeText)		/* we already have an answer */
		return;
	
	const UT_UCSChar * pData;
	for (pData=data; (pData<data+length); pData++)
	{
		if (*pData > 0x00ff)
		{
			m_pie->m_bNeedUnicodeText = true;
			return;
		}
	}

	return;
}

s_RTF_ListenerGetProps::s_RTF_ListenerGetProps(PD_Document * pDocument,
										 IE_Exp_RTF * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	m_apiLastSpan = 0;
	m_apiThisSection = 0;
	m_apiThisBlock = 0;
}

s_RTF_ListenerGetProps::~s_RTF_ListenerGetProps()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
}

bool s_RTF_ListenerGetProps::populate(PL_StruxFmtHandle /*sfh*/,
										 const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_openSpan(api);
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				_closeSpan();
				_openTag("image",api);
				return true;

			case PTO_Field:
				_closeSpan();
				_openTag("field",api);
				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_RTF_ListenerGetProps::populateStrux(PL_StruxDocHandle /*sdh*/,
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
			_closeSpan();
			_closeBlock();
			_closeSection();
			m_bInSection = true;
			m_apiThisSection = pcr->getIndexAP();
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();
			m_bInSection = true;
			m_apiThisSection = pcr->getIndexAP();
			return true;
		}

	case PTX_Block:
		{
			_closeSpan();
			_closeBlock();
			m_bInBlock = true;
			m_apiThisBlock = pcr->getIndexAP();
			return true;
		}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_RTF_ListenerGetProps::change(PL_StruxFmtHandle /*sfh*/,
									   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	// this function is not used.
	return false;
}

bool s_RTF_ListenerGetProps::insertStrux(PL_StruxFmtHandle /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  PL_StruxDocHandle /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	// this function is not used.
	return false;
}

bool s_RTF_ListenerGetProps::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

void s_RTF_ListenerGetProps::_compute_span_properties(const PP_AttrProp * pSpanAP,
													  const PP_AttrProp * pBlockAP,
													  const PP_AttrProp * pSectionAP)
{
	// see if we have a previously unused color reference.
	
	const XML_Char * szColor = PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	UT_sint32 ndxColor = m_pie->_findColor((char*)szColor);
	if (ndxColor == -1)
		m_pie->_addColor((char*)szColor);
	
	szColor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);

	if (UT_stricmp (szColor, "transparent") != 0)
	{
		ndxColor = m_pie->_findColor((char*)szColor);
		if (ndxColor == -1)
			m_pie->_addColor((char*)szColor);
	}
	
	// convert our font properties into an item for the rtf font table.
	// in this pass thru the document we are just collecting all the
	// info that we need to put into the rtf header, so we can't just
	// write it out now.  so, we build a vector of the stuff we want
	// to write (and make sure it's unique).

	_rtf_font_info fi(pSpanAP,pBlockAP,pSectionAP);
	UT_sint32 ndxFont = m_pie->_findFont(&fi);
	if (ndxFont == -1)
		m_pie->_addFont(&fi);

	return;
}
