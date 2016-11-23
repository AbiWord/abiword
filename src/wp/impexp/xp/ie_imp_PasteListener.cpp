/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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


#include "ie_imp_PasteListener.h"
#include "pp_AttrProp.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"

/*!
 * This nifty little class allows all importers to also be used for pasting
 * into the document.
 * The idea is that we create a dummy document which we import to as usual
 * with the impoters.
 * After the Dummy document is completed we do a PD_Document::tellListener on
 * it with this class as the listner class.
 * This class translates all the populate().... and populateStrux()...
 * methods into insertSpan(..) insertStrux(...) methods at the current 
 * insertion point.
 *
 * Hey presto we have pasted into the current document. Pretty cool eh?
 */
IE_Imp_PasteListener::IE_Imp_PasteListener(PD_Document * pDocToPaste, PT_DocPosition insPoint, PD_Document * pSourceDoc) : 
	m_pPasteDocument(pDocToPaste),
	m_insPoint(insPoint),
	m_bFirstSection(true),
	m_bFirstBlock(true),
	m_pSourceDoc(pSourceDoc)
{
}	
bool  IE_Imp_PasteListener::populate(fl_ContainerLayout* /* sfh */,
					 const PX_ChangeRecord * pcr)
{
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	const PP_AttrProp* pAP = NULL;
	UT_DEBUGMSG(("SEVIOR: Doing Populate Section in PasteListener \n"));
	PP_PropertyVector atts;
	PP_PropertyVector props;
	if (m_pSourceDoc->getAttrProp(indexAP, &pAP) && pAP)
	{
		atts = pAP->getAttributes();
		props = pAP->getProperties();
	}
	else
	{
		return false;
	}

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
		UT_uint32 len = pcrs->getLength();
  
		PT_BufIndex bi = pcrs->getBufIndex();
		const UT_UCSChar* pChars = 	m_pSourceDoc->getPointer(bi);
		PP_AttrProp* pfAP = const_cast<PP_AttrProp *>(pAP);
		m_pPasteDocument->insertSpan(m_insPoint,pChars,len,pfAP);
		m_insPoint += len;
		return true;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
		m_pPasteDocument->insertObject(m_insPoint,pcro->getObjectType(),atts,props);
		m_insPoint++;
		return true;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		m_pPasteDocument->changeSpanFmt(PTC_SetExactly,m_insPoint,m_insPoint,atts,props);
		return true;
	}
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	return true;
}

bool  IE_Imp_PasteListener::populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
										  fl_ContainerLayout* * /* psfh */)
{
//
// TODO graphics in struxes
// TODO UID stuff
//
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	const PP_AttrProp* pAP = NULL;
	UT_DEBUGMSG(("SEVIOR: Doing Populate Strux in PasteListener \n"));
	PP_PropertyVector atts;
	PP_PropertyVector props;
	if (m_pSourceDoc->getAttrProp(indexAP, &pAP) && pAP)
	{
		atts = pAP->getAttributes();
		props = pAP->getProperties();
	}
	else
	{
		return false;
	}
	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		if(m_bFirstSection)
		{
//
// Every doc has a first section. Now is good time to extract all the 
// data items from the source document and stuff them into pasted doc
//
// Now these can be found via the properties of the spans and strux's
//
			PD_DataItemHandle pHandle = NULL;
			std::string mimeType;
			const char * szName= NULL;
			UT_ConstByteBufPtr pBuf;
			UT_sint32 k = 0;
			while (m_pSourceDoc->enumDataItems(k, &pHandle, &szName, pBuf, &mimeType))
			{
				m_pPasteDocument->createDataItem(szName,false,pBuf,mimeType,&pHandle);
				k++;
			}
			m_bFirstSection = false;
			if (sdh->getNext() && (sdh->getNext()->getType() == pf_Frag::PFT_Strux) &&
			    (static_cast<pf_Frag_Strux*>(sdh->getNext())->getStruxType() != PTX_Block))
			{
			    // The second frag is not a PXT_Block (it is probably a PTX_SectionTable)
			    // The first block encountered needs to be inserted in the piece table
			    m_bFirstBlock = false;
			}
			return true;
		}
		//
		// We don't actually paste in a section though. Since a paste 
		// is not meant to insert a section break
		//
		//m_pPasteDocument->insertStrux(m_insPoint,PTX_Section,atts,props);
		// m_insPoint++;
		return true;
	}
	case PTX_SectionFootnote:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionFootnote,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionEndnote:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionEndnote,atts,props);
		m_insPoint++;
		return true;
	}

	case PTX_EndFootnote:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndFootnote,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_EndEndnote:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndEndnote,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionTOC:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionTOC,atts,props);
		m_insPoint++;
		return true;
	}

	case PTX_EndTOC:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndTOC,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionHdrFtr:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionHdrFtr,atts,props);
		m_insPoint++;
		return true;
	}

	case PTX_Block:
	{
		if(m_bFirstBlock)
		{
			m_bFirstBlock = false;
			return true;
		}
		m_pPasteDocument->insertStrux(m_insPoint,PTX_Block,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionTable:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionTable,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionFrame:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionFrame,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_EndFrame:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndFrame,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_SectionCell:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_SectionCell,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_EndTable:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndTable,atts,props);
		m_insPoint++;
		return true;
	}
	case PTX_EndCell:
	{
		m_pPasteDocument->insertStrux(m_insPoint,PTX_EndCell,atts,props);
		m_insPoint++;
		return true;
	}
	default:
	{
		m_pPasteDocument->insertStrux(m_insPoint,pcrx->getStruxType(),atts,props);
		m_insPoint++;
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return true;
	}
	}

	return true;
}

PD_Document * IE_Imp_PasteListener::getDoc(void) const
{
	return m_pPasteDocument;
}

