/* AbiWord
 * Copyright (C) 2010 Martin Sevior <msevior@physics.unimelb.edu.au>
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


#include "ie_imp_PasteListener.h"
#include "pp_AttrProp.h"
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
 * This nifty little class allows a docrange to be exported into blank
 * document.
 * We can then export the content of this document to any of our supported
 * formats. This allows us to export any selection into any document
 * format. We can then place this content on the clipbaord for other
 * application to read.
 */
IE_Exp_DocRangeListener::IE_Exp_DocRangeListener(PD_DocumentRange * pDocRange, PD_Document * pOutDoc) : 
  m_pOutDocument(pOutDoc)
  m_bFirstSection(false),
  m_bFirstBlock(false),
  m_pSourceDoc(pDocRange->m_pDoc),
  m_pDocRange(pDocRange)
{
  //
  // Start by exporting the data items to the document
  //
     void *pHandle = NULL;
     std::string mimeType;
     const char * szName= NULL;
     const UT_ByteBuf * pBuf = NULL;
     UT_sint32 k = 0;
     while(m_pSourceDoc->enumDataItems(k,&pHandle,&szName,&pBuf,&mimeType))
     {
          getDoc()->createDataItem(szName,false,pBuf,mimeType,&pHandle);
	  k++;
     }
  //
  // Next epxort all the styles
  //
     UT_GenericVector<PD_Style*> * pVecStyles = NULL;
     m_pSourceDoc->getAllUsedStyles(pVecStyles);
     UT_sint32 i = 0;
     for(i=0;i<pVecStyles->getitemCount();i++)
     {
          PD_Style * pStyle = pVecStyle->getNthItem(i);
          PT_AttrPropIndex iAP = pStyle->getIndexAP();
          const char ** atts = NULL;
          const PP_AttrProp* pAP = NULL;
          if (m_pSourceDoc->getAttrProp(iAP, &pAP) && pAP)
          {
		atts = pAP->getAttributes();
	  } 
	  getDoc()->appendStyle(atts);
     }
}
	
bool  IE_Exp_DocRangeListener::populate(PL_StruxFmtHandle /* sfh */,
					 const PX_ChangeRecord * pcr)
{
	if(!m_bFirstSection)
	{
	     getDoc()->appendStrux(PTX_Section,NULL);
	     m_bFirstSection = true;
	}
	if(!m_bFirstBlock)
	{
	     getDoc()->appendStrux(PTX_Block,NULL);
	     m_bFirstBlock = true;
	}
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	const PP_AttrProp* pAP = NULL;
	UT_DEBUGMSG(("SEVIOR: Doing Populate Section in PasteListener \n"));
	const char ** atts = NULL;
	const char ** props = NULL;
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
		getDoc()->appendSpan(pChars,len);
		return true;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
		getDoc()->appendObject(pcro->getObjectType(),atts);
		return true;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
	        getDoc()->appendFmtt(atts);
		return true;
	}
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	return true;
}

bool  IE_Exp_DocRangeListener::populateStrux(PL_StruxDocHandle /*sdh*/,
					     const PX_ChangeRecord * pcr,
					     PL_StruxFmtHandle * /* psfh */)
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
	const char ** atts = NULL;
	const char ** props = NULL;
	if (m_pSourceDoc->getAttrProp(indexAP, &pAP) && pAP)
	{
		atts = pAP->getAttributes();
		props = pAP->getProperties();
	}
	else
	{
		return false;
	}
	if((pcrx->getStruxType()== PTX_Section) && !m_bFirstSection)
	{
	    m_bFirstSection = true;
	}
	if((pcrx->getStruxType()== PTX_Block) && !m_bFirstBlock)
	{
	    m_bFirstBlock = true;
	}
	if(!m_bFirstSection && pcrx->getStruxType() != PTX_Section)
	{
	     getDoc()->appendStrux(PTX_Section,NULL);
	     m_bFirstSection = true;
	}
	if(!m_bFirstBlock && (pcrx->getStruxType() != PTX_Section) && (pcrx->getStruxType() != PTX_Block))
	{
	     getDoc()->appendStrux(PTX_Block,NULL);
	     m_bFirstBlock = true;
	}
	getDoc()->appendStrux(pcrx->getStruxType(),atts);
	return true;
}

PD_Document * IE_Exp_DocRangeListener::getDoc(void) const
{
	return m_pOutDocument;
}

