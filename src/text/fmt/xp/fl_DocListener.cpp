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

#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Glob.h"
#include "fv_View.h"
#include "fl_DocListener.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"

#define UPDATE_LAYOUT_ON_SIGNAL

/*!
 Create DocListener
 \param doc Client of this DocListener
 \param pLayout Layout notified by this DocListener
*/
fl_DocListener::fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout)
{
	m_pDoc = doc;
	m_pLayout = pLayout;
	m_bScreen = pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN);
	m_iGlobCounter = 0;
	m_pCurrentSL = NULL;
}

/*!
 Destruct DocListener
*/
fl_DocListener::~fl_DocListener()
{
}

/*!
 */
bool fl_DocListener::populate(PL_StruxFmtHandle sfh,
							  const PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);
	//UT_DEBUGMSG(("fl_DocListener::populate\n"));

	bool bResult = false;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		if(pBL->getPrev()!= NULL && pBL->getPrev()->getLastLine()==NULL)
		{
			UT_DEBUGMSG(("In DocListner no LastLine in Previous Block Fixing this now \n"));
			UT_DEBUGMSG(("getPrev = %d this = %d \n",pBL->getPrev(),pBL));
			if( pBL->getSectionLayout()->getType() != FL_SECTION_HDRFTR)
				pBL->getPrev()->format();
		}

		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_populateSpan(pBL, pcrs, blockOffset, len);
		if(pBL->getLastLine()==NULL)
		{
			UT_DEBUGMSG(("In  DocListner no LastLine in this block fixing this now \n"));
			UT_DEBUGMSG(("getPrev = %d this = %d \n",pBL->getPrev(),pBL));
			if(pBL->getSectionLayout()->getType() != FL_SECTION_HDRFTR && pBL->getPrev()!= NULL)
				pBL->format();
			//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_populateObject(pBL, blockOffset,pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_insertFmtMark(pBL, pcrfm);
		goto finish_up;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

 finish_up:
	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return bResult;
}

/*!
 */
bool fl_DocListener::populateStrux(PL_StruxDocHandle sdh,
								   const PX_ChangeRecord * pcr,
								   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pLayout);
	UT_DEBUGMSG(("fl_DocListener::populateStrux in doclistner \n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
			
		if (m_pDoc->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				// Append a SectionLayout to this DocLayout
				fl_DocSectionLayout* pSL = new fl_DocSectionLayout(m_pLayout, sdh, pcr->getIndexAP());
				if (!pSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return false;
				}
			
				m_pLayout->addSection(pSL);

				*psfh = (PL_StruxFmtHandle)pSL;
				
				m_pCurrentSL = pSL;
			}
			else
			{
				if (0 == UT_strcmp(pszSectionType, "header"))
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute("id", pszID);

					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					UT_ASSERT(pDocSL);
			
					// Append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_HEADER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return false;
					}
					//
					// Add the hdrFtr section to the linked list of SectionLayouts
					//
					m_pLayout->addHdrFtrSection(pSL);
					pDocSL->setHdrFtr(FL_HDRFTR_HEADER, pSL);
					*psfh = (PL_StruxFmtHandle)pSL;
					
					m_pCurrentSL = pSL;
				}
				else if (0 == UT_strcmp(pszSectionType, "footer"))
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute("id", pszID);

					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					UT_ASSERT(pDocSL);
			
					// Append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_FOOTER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return false;
					}
					//
					// Add the hdrFtr section to the linked list of SectionLayouts
					//
					m_pLayout->addHdrFtrSection(pSL);
					pDocSL->setHdrFtr(FL_HDRFTR_FOOTER, pSL);

					*psfh = (PL_StruxFmtHandle)pSL;
					
					m_pCurrentSL = pSL;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			// TODO fail?
			return false;
		}
	}
	break;

	case PTX_Block:
	{
		UT_ASSERT(m_pCurrentSL);
		
		// Append a new BlockLayout to that SectionLayout
		fl_BlockLayout*	pBL = m_pCurrentSL->appendBlock(sdh, pcr->getIndexAP());
		if (!pBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout"));
			return false;
		}
		UT_DEBUGMSG(("SEVIOR: appending block %x to Section %x \n",pBL,m_pCurrentSL));
		UT_DEBUGMSG(("SEVIOR: Section Type = %d \n",m_pCurrentSL->getType()));

		// BUGBUG: this is *not* thread-safe, but should work for now
		if (m_bScreen)
		{
			UT_uint32 reason =  0;
			if( m_pLayout->getAutoSpellCheck())
			{
				reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
			}
			m_pLayout->queueBlockForBackgroundCheck(reason, pBL,true);
		}
		*psfh = (PL_StruxFmtHandle)pBL;
		if(pBL->getLastLine()==NULL)
		{
			if(pBL->getSectionLayout()->getType() != FL_SECTION_HDRFTR && pBL->getPrev() != NULL)
			{
				UT_DEBUGMSG(("In DocListner no LastLine in block append. Fixing this now \n"));
				UT_DEBUGMSG(("getPrev = %d this = %d \n",pBL->getPrev(),pBL));
				pBL->format();
			}
		}

	}
	break;
			
	default:
		UT_ASSERT(0);
		return false;
	}

	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return true;
}

/*!
 */
bool fl_DocListener::change(PL_StruxFmtHandle sfh,
							const PX_ChangeRecord * pcr)
{
	//UT_DEBUGMSG(("fl_DocListener::change\n"));
	bool bResult = false;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_GlobMarker:
	{
		UT_ASSERT(sfh == 0);							// globs are not strux-relative
		const PX_ChangeRecord_Glob * pcrg = static_cast<const PX_ChangeRecord_Glob *> (pcr);
		switch (pcrg->getFlags())
		{
		default:
		case PX_ChangeRecord_Glob::PXF_Null:			// not a valid glob type
			UT_ASSERT(0);
			bResult = false;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_MultiStepStart:
			m_iGlobCounter++;
			bResult = true;
			goto finish_up;
			
		case PX_ChangeRecord_Glob::PXF_MultiStepEnd:
			m_iGlobCounter--;
			bResult = true;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_UserAtomicStart:	// TODO decide what (if anything) we need
		case PX_ChangeRecord_Glob::PXF_UserAtomicEnd:	// TODO to do here.
			bResult = true;
			goto finish_up;
		}
	}
			
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_insertSpan(pBL, pcrs);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_deleteSpan(pBL, pcrs);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeSpan:
	{
		const PX_ChangeRecord_SpanChange * pcrsc = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_changeSpan(pBL, pcrsc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_insertFmtMark(pBL, pcrfm);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_deleteFmtMark(pBL, pcrfm);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeFmtMark:
	{
		const PX_ChangeRecord_FmtMarkChange * pcrfmc = static_cast<const PX_ChangeRecord_FmtMarkChange *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_changeFmtMark(pBL, pcrfmc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteStrux:
	{
		const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

		switch (pcrx->getStruxType())
		{
		case PTX_Section:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Section);
			fl_DocSectionLayout * pSL = static_cast<fl_DocSectionLayout *>(pL);
			bResult = pSL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_Block:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			bResult = pBLSL->bl_doclistener_deleteStrux(pBL, pcrx);
			goto finish_up;
		}

		default:
			UT_ASSERT(0);
			bResult = false;
			goto finish_up;
		}
	}
					
	case PX_ChangeRecord::PXT_ChangeStrux:
	{
		const PX_ChangeRecord_StruxChange * pcrxc = static_cast<const PX_ChangeRecord_StruxChange *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;

		// TODO getOldIndexAP() is only intended for use by the document.
		// TODO this assert is probably wrong. --- BUT EVERYTIME IT HAS
		// TODO GONE OFF, I'VE FOUND A BUG, SO MAYBE WE SHOULD KEEP IT :-)
		UT_ASSERT(pL->getAttrPropIndex() == pcrxc->getOldIndexAP());
		UT_ASSERT(pL->getAttrPropIndex() != pcr->getIndexAP());

		switch (pL->getType())
		{
		case PTX_Section:
		{
			fl_DocSectionLayout* pSL = static_cast<fl_DocSectionLayout*>(pL);
			
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			
			bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
			UT_ASSERT(bres);
			PL_StruxDocHandle sdh = pL->getStruxDocHandle();
	
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);

			//
			// OK Sevior adds code to actually change a 
			// sectionlayout to
			// a header/footer layout
			//
			// Strategy: Collapse all the blocks in this section.
			// create a header/footer sectionlayout ala populate_strux
			// transfer the blocks in this sectionlayout to the
			// new header/footer and format just the shadows
			//
			if(pszSectionType && UT_strcmp(pszSectionType,"header") == 0)
			{
				//
				//  OK first we need a previous section with a
				//  matching ID
				//
				const XML_Char* pszID = NULL;
				pAP->getAttribute("id", pszID);

				fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
				UT_ASSERT(pDocSL); 
			        
				// Append a HdrFtrSectionLayout to this DocLayout
				fl_HdrFtrSectionLayout* pHeadSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_HEADER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
				if (!pHeadSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return false;
				}
				//
				// Add the hdrFtr section to the linked list of SectionLayouts
				//
				m_pLayout->addHdrFtrSection(pHeadSL);
				//
				// Set the pointers to this header/footer
				//
				pDocSL->setHdrFtr(FL_HDRFTR_HEADER, pHeadSL);

				// OK now clean up the old section and transfer
				// blocks into this header section.
				
				pHeadSL->changeStrux(pSL);

				bResult = true;
				goto finish_up;
			}
			else if(pszSectionType && UT_strcmp(pszSectionType,"footer") == 0)
			{
				//
				//  OK first we need a previous section with a
				//  matching ID
				//
				const XML_Char* pszID = NULL;
				pAP->getAttribute("id", pszID);

				fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
				UT_ASSERT(pDocSL); 
			        
				// Append a HdrFtrSectionLayout to this DocLayout
				fl_HdrFtrSectionLayout* pFootSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_FOOTER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
				if (!pFootSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return false;
				}
				//
				// Add the hdrFtr section to the linked list of SectionLayouts
				//
				m_pLayout->addHdrFtrSection(pFootSL);
				//
				// Set the pointers to this header/footer
				//
				pDocSL->setHdrFtr(FL_HDRFTR_FOOTER, pFootSL);

				// OK Now clean up the old section and transfer
				// blocks into this header section.
				pFootSL->changeStrux(pSL);
			        
				bResult = true;
				goto finish_up;
			}

 			bResult = pSL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		
		case PTX_Block:
		{
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			bResult = pBLSL->bl_doclistener_changeStrux(pBL, pcrxc);
			goto finish_up;
		}
					
		default:
			UT_ASSERT(0);
			bResult = false;
			goto finish_up;
		}
	}

	case PX_ChangeRecord::PXT_InsertStrux:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		bResult = false;
		goto finish_up;

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_insertObject(pBL, pcro);
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_DeleteObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_deleteObject(pBL, pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeObject:
	{
		const PX_ChangeRecord_ObjectChange * pcroc = static_cast<const PX_ChangeRecord_ObjectChange *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		fl_SectionLayout* pBLSL = pBL->getSectionLayout();
		bResult = pBLSL->bl_doclistener_changeObject(pBL, pcroc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangePoint:
	{
		FV_View* pView = m_pLayout->getView();
		if (pView && pView->isActive())
			pView->_setPoint(pcr->getPosition());
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_ListUpdate:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		pBL->listUpdate();
		goto finish_up;

	}
	case PX_ChangeRecord::PXT_StopList:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		pBL->StopList();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_DontChangeInsPoint:
	{
		FV_View* pView = m_pLayout->getView();
		pView->setDontChangeInsPoint();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_AllowChangeInsPoint:
	{
		FV_View* pView = m_pLayout->getView();
		pView->allowChangeInsPoint();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_UpdateField:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		pBL->format();
		FV_View* pView = m_pLayout->getView();
		pView->updateScreen();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_RemoveList:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		pBL->m_bStopList = true;
		pBL->_deleteListLabel();
		pBL->m_pAutoNum = NULL;
		pBL->m_bListItem = false;
		pBL->m_bStopList = false;
		goto finish_up;
	}
	default:
		UT_ASSERT(0);
		bResult = false;
		goto finish_up;
	}

 finish_up:
	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return bResult;
}

/*!
 */
bool fl_DocListener::insertStrux(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr,
								 PL_StruxDocHandle sdh,
								 PL_ListenerId lid,
								 void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
														 PL_ListenerId lid,
														 PL_StruxFmtHandle sfhNew))
{
	UT_DEBUGMSG(("fl_DocListener::insertStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	fl_Layout * pL = (fl_Layout *)sfh;
	switch (pL->getType())				// see what the immediately prior strux is.
	{
	case PTX_Section:					// the immediately prior strux is a section.

		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
			// We are inserting a section immediately after a section (with no
			// intervening block).  This is probably a bug, because there should
			// at least be an empty block between them (so that the user can set
			// the cursor there and start typing, if nothing else).
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
				
		case PTX_Block:					// we are inserting a block.
		{
			// The immediately prior strux is a section.  So, this
			// will become the first block of the section and have no
			// text.

			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			bool bResult = pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
			if (0 == m_iGlobCounter)
			{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
				m_pLayout->updateLayout();
#endif
			}
	
			return bResult;
		}

		default:						// unknown strux.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		
	case PTX_Block:						// the immediately prior strux is a block.

		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
		{
			// The immediately prior strux is a block.  Everything
			// from this point forward (to the next section) needs to
			// be re-parented to this new section.  We also need to
			// verify that there is a block immediately after this new
			// section -- a section must be followed by a block
			// because a section cannot contain content.
			
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			bool bResult = pBLSL->bl_doclistener_insertSection(pBL, pcrx,sdh,lid,pfnBindHandles);
			if (0 == m_iGlobCounter)
			{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
				m_pLayout->updateLayout();
#endif
			}
	
			return bResult;
		}
		
		case PTX_Block:					// we are inserting a block.
		{
			// The immediately prior strux is also a block.  Insert the new
			// block and split the content between the two blocks.
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			bool bResult = pBLSL->bl_doclistener_insertBlock(pBL, pcrx,sdh,lid,pfnBindHandles);
			if (0 == m_iGlobCounter)
			{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
				m_pLayout->updateLayout();
#endif
			}
	
			return bResult;
		}
			
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	return false;
}

/*!
 */
bool fl_DocListener::signal(UT_uint32 iSignal)
{
	bool bCursorErased = false;
	FV_View* pView = m_pLayout->getView();

	switch (iSignal)
	{
	case PD_SIGNAL_UPDATE_LAYOUT:
#ifdef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
		if(pView->isCursorOn()== true)
		{
			pView->eraseInsertionPoint();
			bCursorErased = true;
		}
		pView->updateScreen();
		if(bCursorErased == true)
		{
			pView->drawInsertionPoint();
		}

		break;
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return true;
}
