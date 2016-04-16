/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior (msevior@physics.unimelb.edu.au>
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
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"

#include "ap_Prefs.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_TOCLayout.h"
#include "fl_BlockLayout.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
#include "fb_LineBreaker.h"
#include "fb_ColumnBreaker.h"
#include "fp_FootnoteContainer.h"
#include "fp_TOCContainer.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "fg_Graphic.h"
#include "pt_Types.h"
#include "xap_App.h"
/*
  TODO this file is now really too long.  divide it up
  into smaller ones.
*/

fl_SectionLayout::fl_SectionLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, SectionType iType, fl_ContainerType iCType, PTStruxType iStrux, fl_ContainerLayout * pMyContainerLayout)
	: fl_ContainerLayout(pMyContainerLayout, sdh, indexAP,iStrux, iCType),
	  m_iType(iType),
	  m_pLayout(pLayout),
	  m_bIsCollapsed(false), // collapsed layouts cannot contain point, and this value never changes
	  m_bNeedsReformat(true),
	  m_bNeedsRedraw(true),	
	  m_pGraphicImage(NULL),
	  m_pImageImage(NULL),
	  m_iGraphicTick(0),
	  m_iDocImageWidth(0),
	  m_iDocImageHeight(0)

{
	UT_ASSERT(pLayout);
	m_pDoc = pLayout->getDocument();
}

fl_SectionLayout::~fl_SectionLayout()
{
	DELETEP(m_pGraphicImage);
	DELETEP(m_pImageImage);
}

FL_DocLayout* fl_SectionLayout::getDocLayout(void) const
{
	if(m_pLayout == NULL)
	{
		return fl_ContainerLayout::getDocLayout();
	}
	return m_pLayout;
}

bool fl_SectionLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;

	fl_ContainerLayout*	pL = getFirstLayout();

	while (pL)
	{
		bResult = pL->recalculateFields(iUpdateCount) || bResult;

		pL = pL->getNext();
	}

	return bResult;
}


void fl_SectionLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pL = getFirstLayout();
	while (pL)
	{
		pL->markAllRunsDirty();
		pL = pL->getNext();
	}
}

void fl_SectionLayout::_purgeLayout()
{
	fl_ContainerLayout*	pL = getLastLayout();

	while (pL)
	{
		fl_ContainerLayout* pNuke = pL;

		pL = pL->getPrev();
		pNuke->setNext(NULL);
		delete pNuke;
	}

	return;
}

void fl_SectionLayout::removeFromUpdate(fl_ContainerLayout * pCL)
{
  while((m_vecFormatLayout.getItemCount() > 0) && (m_vecFormatLayout.findItem(pCL) >= 0))
  {
    UT_sint32 i = m_vecFormatLayout.findItem(pCL);
    m_vecFormatLayout.deleteNthItem(i);
  }
}


void fl_SectionLayout::clearNeedsReformat(fl_ContainerLayout * pCL)
{
       UT_sint32 i = m_vecFormatLayout.findItem(pCL);
       if(i>= 0)
       {
	   m_vecFormatLayout.deleteNthItem(i);
       }
       if(m_vecFormatLayout.getItemCount() == 0)
       {
	   m_bNeedsReformat = false;
       }
}

void fl_SectionLayout::setNeedsReformat(fl_ContainerLayout * pCL, UT_uint32 /*offset*/)
{
        UT_sint32 i = m_vecFormatLayout.findItem(pCL);
	if(i< 0)
	{
	  m_vecFormatLayout.addItem(pCL);
	}
	m_bNeedsReformat = true;
	xxx_UT_DEBUGMSG(("SetNeedsReformat in %s from %s number to format %d\n",getContainerString(),pCL->getContainerString(),m_vecFormatLayout.getItemCount()));
	if(myContainingLayout() != NULL && (static_cast<fl_SectionLayout *>(myContainingLayout()) != this) && (getContainerType() != FL_CONTAINER_SHADOW))
	{
		static_cast<fl_SectionLayout *>(myContainingLayout())->setNeedsReformat(this);
	}
	if(getContainerType() == FL_CONTAINER_SHADOW)
	{
		fl_HdrFtrShadow * pShad = static_cast<fl_HdrFtrShadow *>(this);
		pShad->getHdrFtrSectionLayout()->setNeedsReformat(this,0);
	}
}


void fl_SectionLayout::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	if(myContainingLayout() != NULL  && static_cast<fl_SectionLayout *>(myContainingLayout()) != this)
	{
		static_cast<fl_SectionLayout *>(myContainingLayout())->setNeedsRedraw();
	}
}

bool fl_SectionLayout::bl_doclistener_populateSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	if(pBL->getPrev()!= NULL && pBL->getPrev()->getLastContainer()==NULL)
	{
		UT_DEBUGMSG(("In bl_doclistner_pop no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %p this = %p \n",pBL->getPrev(),pBL));
		//  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_populateSpan(pBL,pcrs,blockOffset,len);
		}
		else
		{
			return false;
		}
		return bres;
	}

	return static_cast<fl_BlockLayout *>(pBL)->doclistener_populateSpan(pcrs, blockOffset, len);
}

bool fl_SectionLayout::bl_doclistener_populateObject(fl_ContainerLayout* pBL, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_populateObject(pBL,blockOffset,pcro);
		}
		else
		{
			return false;
		}
		return bres;
	}
	return static_cast<fl_BlockLayout *>(pBL)->doclistener_populateObject(blockOffset, pcro);
}

bool fl_SectionLayout::bl_doclistener_insertSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_insertSpan(pBL,pcrs);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
    bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertSpan(pcrs);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_deleteSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_deleteSpan(pBL,pcrs);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteSpan(pcrs);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_changeSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_SpanChange * pcrsc)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_changeSpan(pBL,pcrsc);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeSpan(pcrsc);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_deleteStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_deleteStrux(pBL,pcrx);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteStrux(pcrx);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_changeStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_StruxChange * pcrxc)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_changeStrux(pBL,pcrxc);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeStrux(pcrxc);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_insertBlock(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
												  pf_Frag_Strux* sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																		  PL_ListenerId lid,
																		  fl_ContainerLayout* sfhNew))
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	UT_ASSERT(m_pDoc->getAllowChangeInsPoint());
	if(pHFSL)
	{
		if(pBL)
		{
			pHFSL->bl_doclistener_insertBlock(pBL,pcrx, sdh, lid, pfnBindHandles);
		}
		else
		{
			// Insert the block at the beginning of the section
			fl_BlockLayout*	pNewBL = static_cast<fl_BlockLayout *>(insert(sdh, NULL, pcrx->getIndexAP(),FL_CONTAINER_BLOCK));
			if (!pNewBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout\n"));
				return false;
			}
			bres = pNewBL->doclistener_insertFirstBlock(pcrx, sdh,
													lid, pfnBindHandles);			// Insert the block at the beginning of the section in the HDrFtr
            // Typically a cell of a table
			bres = pHFSL->bl_doclistener_insertFirstBlock(this,pcrx, sdh, lid);
		}
		pHFSL->checkAndAdjustCellSize(this);
		UT_ASSERT(m_pDoc->getAllowChangeInsPoint());
		return bres;
	}
	if (pBL)
		return static_cast<fl_BlockLayout *>(pBL)->doclistener_insertBlock(pcrx, sdh, lid, pfnBindHandles);
	else
	{
		// Insert the block at the beginning of the section
		fl_BlockLayout*	pNewBL = static_cast<fl_BlockLayout *>(insert(sdh, NULL, pcrx->getIndexAP(),FL_CONTAINER_BLOCK));
		if (!pNewBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout\n"));
			return false;
		}
		UT_ASSERT(m_pDoc->getAllowChangeInsPoint());
		return pNewBL->doclistener_insertFirstBlock(pcrx, sdh,
													lid, pfnBindHandles);

	}
}

void fl_SectionLayout::checkAndAdjustCellSize(void)
{
	if(getContainerType() != FL_CONTAINER_CELL)
	{
		return;
	}	
	fl_CellLayout * pCell = static_cast<fl_CellLayout *>(this);
	pCell->checkAndAdjustCellSize();
}

bool fl_SectionLayout::bl_doclistener_insertSection(fl_ContainerLayout* pPrevL,
													SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{
	if(pPrevL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		bool bres = static_cast<fl_BlockLayout *>(pPrevL)->doclistener_insertSection(pcrx, iType, sdh, lid, pfnBindHandles);
		return bres;
	}
	else if(iType == FL_SECTION_TOC)
	{
		PT_AttrPropIndex indexAP = pcrx->getIndexAP();
		fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(insert(sdh,pPrevL,indexAP, FL_CONTAINER_TOC));

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).

		fl_ContainerLayout* sfhNew = pSL;
		//
		// Don't bind to shadows
		//
		if(pfnBindHandles)
		{
			pfnBindHandles(sdh,lid,sfhNew);
		}
		//
		// That's all we need to do except update the view pointers I guess..
		//
		FV_View* pView = m_pLayout->getView();
		if (pView && (pView->isActive() || pView->isPreview()))
		{
			pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
		}
		else if(pView && pView->getPoint() > pcrx->getPosition())
		{
			//
			// For EndTOC
			//
			pView->setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
		}
		if(pView)
		        pView->updateCarets(pcrx->getPosition(),1);
		return true;

	}
	else if(((pPrevL->getContainerType() == FL_CONTAINER_FRAME) ||(pPrevL->getContainerType() == FL_CONTAINER_TABLE)) && (iType == FL_SECTION_HDRFTR))
	{
		fl_SectionLayout * pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_NONE,m_pLayout,NULL, sdh, pcrx->getIndexAP());
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pSL);
		m_pLayout->addHdrFtrSection(pHFSL);
//
// Need to find the DocSectionLayout associated with this.
//
		const PP_AttrProp* pHFAP = NULL;
		PT_AttrPropIndex indexAP = pcrx->getIndexAP();
		bool bres = (m_pDoc->getAttrProp(indexAP, &pHFAP) && pHFAP);
		UT_UNUSED(bres);
		UT_ASSERT(bres);
		const gchar* pszNewID = NULL;
		pHFAP->getAttribute("id", pszNewID);
//
// pszHFID may not be defined yet. If not we can't do this stuff. If it is defined
// this step is essential
//
		if(pszNewID)
		{
		  // plam mystery code
			// plam, MES here, I need this code for inserting headers/footers.
		  UT_DEBUGMSG(("new id: tell plam if you see this message\n"));
//		  UT_ASSERT(0);
			fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr(static_cast<const char*>(pszNewID));
			UT_ASSERT(pDocSL);
//
// Determine if this is a header or a footer.
//
			const gchar* pszSectionType = NULL;
			pHFAP->getAttribute("type", pszSectionType);

			HdrFtrType hfType = FL_HDRFTR_NONE;
			if (pszSectionType && *pszSectionType)
			{
				if(strcmp(pszSectionType,"header") == 0)
					hfType = FL_HDRFTR_HEADER;
				else if (strcmp(pszSectionType,"header-even") == 0)
					hfType = FL_HDRFTR_HEADER_EVEN;
				else if (strcmp(pszSectionType,"header-first") == 0)
					hfType = FL_HDRFTR_HEADER_FIRST;
				else if (strcmp(pszSectionType,"header-last") == 0)
					hfType = FL_HDRFTR_HEADER_LAST;
				else if (strcmp(pszSectionType,"footer") == 0)
					hfType = FL_HDRFTR_FOOTER;
				else if (strcmp(pszSectionType,"footer-even") == 0)
					hfType = FL_HDRFTR_FOOTER_EVEN;
				else if (strcmp(pszSectionType,"footer-first") == 0)
					hfType = FL_HDRFTR_FOOTER_FIRST;
				else if (strcmp(pszSectionType,"footer-last") == 0)
					hfType = FL_HDRFTR_FOOTER_LAST;

				if(hfType != FL_HDRFTR_NONE)
				{
					pHFSL->setDocSectionLayout(pDocSL);
					pHFSL->setHdrFtr(hfType);
					//
					// Set the pointers to this header/footer
					//
					pDocSL->setHdrFtr(hfType, pHFSL);
				}
			}
		}
		else
		{
			UT_DEBUGMSG(("NO ID found with insertSection HdrFtr \n"));
		}

	// Must call the bind function to complete the exchange of handles
	// with the document (piece table) *** before *** anything tries
	// to call down into the document (like all of the view
	// listeners).

		fl_ContainerLayout* sfhNew = pSL;
		//
		// Don't bind to shadows
		//
		if(pfnBindHandles)
		{
			pfnBindHandles(sdh,lid,sfhNew);
		}

		fl_SectionLayout* pOldSL = getDocSectionLayout();
//
// Now move all the containers following into the new section
//
		fl_ContainerLayout* pCL = pPrevL->getNext();
	//
	// BUT!!! Don't move the immediate Footnotes, Endnotes or Annotations
	//
		fl_ContainerLayout * pLastCL = pPrevL;

		while(pCL && (static_cast<fl_SectionLayout *>(pCL) == pSL))
		{
			pCL = pCL->getNext();
		}
		while(pCL && ((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
					  (pCL->getContainerType() == FL_CONTAINER_ENDNOTE)||
					  (pCL->getContainerType() == FL_CONTAINER_ANNOTATION)))
		{
			pLastCL = pCL;
			pCL = pCL->getNext();
		}
		fl_BlockLayout * pBL = NULL;
		while (pCL)
		{
			fl_ContainerLayout* pNext = pCL->getNext();
			pBL = NULL;
			pCL->collapse();
			if(pCL->getContainerType()==FL_CONTAINER_BLOCK)
			{
				pBL = static_cast<fl_BlockLayout *>(pCL);
			} 
			if(pBL && pBL->isHdrFtr())
			{
				fl_HdrFtrSectionLayout * pHF = static_cast<fl_HdrFtrSectionLayout *>(pBL->getSectionLayout());
				pHF->collapseBlock(pBL);
			}
			pOldSL->remove(pCL);
			pSL->add(pCL);
			if(pBL)
			{
				pBL->setSectionLayout( pSL);
				pBL->setNeedsReformat(pBL,0);
			}
			pCL = pNext;
		}

//
// Terminate blocklist here. This Block is the last in this section.
//
		if (pLastCL)
		{
			pLastCL->setNext(NULL);
			pOldSL->setLastLayout(pLastCL);
		}
		if(pszNewID)
		{
			pSL->format();
			pSL->redrawUpdate();
		}
		else
			return true;

		FV_View* pView = m_pLayout->getView();
		if (pView && (pView->isActive() || pView->isPreview()))
		{
			pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
		}
		else if(pView && pView->getPoint() > pcrx->getPosition())
		{
			pView->setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
		}
		if(pView)
		  pView->updateCarets(pcrx->getPosition(),1);
		return true;
	}
	return false;
}


fl_SectionLayout * fl_SectionLayout::bl_doclistener_insertTable(fl_ContainerLayout* pBL,
													SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{
	fl_SectionLayout * pSL = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertTable(pcrx, iType, sdh, lid, pfnBindHandles);
	checkAndAdjustCellSize();
	return pSL;
}



fl_SectionLayout * fl_SectionLayout::bl_doclistener_insertTable(SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{
	UT_UNUSED(iType);
	UT_ASSERT(iType == FL_SECTION_TABLE);
	UT_return_val_if_fail(pcrx, NULL);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
	PT_DocPosition pos1;
//
// This is to clean the fragments
//
	m_pDoc->getBounds(true,pos1);

	fl_SectionLayout* pSL = NULL;
	bool bFrame = (getContainerType() == FL_CONTAINER_FRAME);
	PT_DocPosition pos = getPosition(true)+1;
	bool bTooFar = (pcrx->getPosition() > pos);
	if(bFrame && bTooFar)
	{
	  //
	  // This happens if a table is inserted right after an end frame strux
	  //
	  fl_DocSectionLayout * pDSL = getDocSectionLayout();
	  fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(this);
	  pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(pDSL)->insert(sdh,pCL,pcrx->getIndexAP(), FL_CONTAINER_TABLE));

	}
	else
	{
	  pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(this)->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_TABLE));
	}
	fl_ContainerLayout* sfhNew = pSL;
	//
	// Don't bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
//
// OK that's it!
//
	checkAndAdjustCellSize();
	return pSL;
}

fl_BlockLayout * fl_SectionLayout::getFirstBlock(void) const
{
  fl_ContainerLayout * pCL = getFirstLayout();
  if(pCL == NULL)
  {
    return NULL;
  }
  if(pCL->getContainerType() == FL_CONTAINER_BLOCK)
  {
    return static_cast<fl_BlockLayout *>(pCL);
  }
  return pCL->getNextBlockInDocument();
}

fl_SectionLayout * fl_SectionLayout::bl_doclistener_insertFrame(fl_ContainerLayout* pBL,
													SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{
	fl_SectionLayout * pSL = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertFrame(pcrx, iType, sdh, lid, pfnBindHandles);
	return pSL;
}

bool fl_SectionLayout::bl_doclistener_insertObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_insertObject(pBL,pcro);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertObject(pcro);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_deleteObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_deleteObject(pBL,pcro);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteObject(pcro);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_changeObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_ObjectChange * pcroc)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_changeObject(pBL,pcroc);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}

	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeObject(pcroc);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_insertFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_insertFmtMark(pBL,pcrfm);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertFmtMark(pcrfm);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_deleteFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_deleteFmtMark(pBL,pcrfm);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteFmtMark(pcrfm);
	checkAndAdjustCellSize();
	return bres;
}

bool fl_SectionLayout::bl_doclistener_changeFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMarkChange * pcrfmc)
{
    fl_HdrFtrSectionLayout * pHFSL = getHdrFtrLayout();
	bool bres = true;
	if(pHFSL)
	{
		if(pBL)
		{
			bres = pHFSL->bl_doclistener_changeFmtMark(pBL,pcrfmc);
		}
		else
		{
			return false;
		}
		pHFSL->checkAndAdjustCellSize(this);
		return bres;
	}
	bres = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeFmtMark(pcrfmc);
	checkAndAdjustCellSize();
	return bres;
}


void fl_SectionLayout::setImageWidth(UT_sint32 iWidth)
{
    m_iDocImageWidth = iWidth;
}

void  fl_SectionLayout::setImageHeight(UT_sint32 iHeight)
{
    m_iDocImageHeight = iHeight;
}

void fl_SectionLayout::checkGraphicTick(GR_Graphics * pG)
{
  if(getDocLayout()->getGraphicTick() != m_iGraphicTick)
    {
      xxx_UT_DEBUGMSG(("Current tick == %d layout Tick == %d \n",m_iGraphicTick,getDocLayout()->getGraphicTick()));
    }
  if(m_pGraphicImage && ((getDocLayout()->getGraphicTick() != m_iGraphicTick) || (m_pImageImage == NULL) ))
	{
		DELETEP(m_pImageImage);
		m_pImageImage = m_pGraphicImage->regenerateImage(pG);
		const UT_Rect rec(0,0,m_iDocImageWidth,m_iDocImageHeight);	
		m_pImageImage->scaleImageTo(pG,rec);
		m_iGraphicTick = getDocLayout()->getGraphicTick();
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif
fl_DocSectionLayout::fl_DocSectionLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, SectionType iType)
	: fl_SectionLayout(pLayout, sdh, indexAP, iType, FL_CONTAINER_DOCSECTION,PTX_Section, this),
	  m_ColumnBreaker(this),
	  m_pHeaderSL(NULL),
	  m_pFooterSL(NULL),
	  m_pHeaderEvenSL(NULL),
	  m_pFooterEvenSL(NULL),
	  m_pHeaderFirstSL(NULL),
	  m_pFooterFirstSL(NULL),
	  m_pHeaderLastSL(NULL),
	  m_pFooterLastSL(NULL),
	  m_iNumColumns(1),
	  m_iColumnGap(0),
	  m_bColumnLineBetween(false),
	  m_iColumnOrder(0),
	  m_iSpaceAfter(0),
	  m_bRestart(false),
	  m_iRestartValue(0),

	  m_iLeftMargin(0),
	  m_dLeftMarginUserUnits(0.0),
	  m_iRightMargin(0),
	  m_dRightMarginUserUnits(0.0),
	  m_iTopMargin(0),
	  m_dTopMarginUserUnits(0.0),
	  m_iBottomMargin(0),
	  m_dBottomMarginUserUnits(0.0),
	  m_iFooterMargin(0),
	  m_dFooterMarginUserUnits(0.0),
	  m_iHeaderMargin(0),
	  m_dHeaderMarginUserUnits(0.0),
	  m_iMaxSectionColumnHeight(0),
	  m_dMaxSectionColumnHeight(0.0),
	  m_iFootnoteLineThickness(0),
	  m_iFootnoteYoff(0),

	  m_pFirstColumn(NULL),
	  m_pLastColumn(NULL),
	  m_pFirstOwnedPage(NULL),
	  m_iPageCount(0),
	  m_bNeedsFormat(false),
	  m_bNeedsRebuild(false),
	  m_bNeedsSectionBreak(true),
	  m_pFirstEndnoteContainer(NULL),
	  m_pLastEndnoteContainer(NULL),
	  m_bDeleteingBrokenContainers(false),
	  m_iNewHdrHeight(0),
	  m_iNewFtrHeight(0),
	  m_pHdrFtrChangeTimer(NULL),
	  m_bDoingCollapse(false)
{
	UT_ASSERT(iType == FL_SECTION_DOC);

	m_pDoc= pLayout->getDocument();
	
	m_sPaperColor.clear();
	m_sScreenColor.clear();
	lookupProperties();
}

fl_DocSectionLayout::~fl_DocSectionLayout()
{
// Remove any background HdrFtr change callbacks

	if(m_pHdrFtrChangeTimer)
	{
		m_pHdrFtrChangeTimer->stop();
		DELETEP(m_pHdrFtrChangeTimer);
	}
	// Don't delete broken tables since their pages have been removed
	
	// NB: be careful about the order of these
	_purgeLayout();

	UT_GenericVector<fl_HdrFtrSectionLayout*> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = vecHdrFtr.getNthItem(i);
		delete pHdrFtr;
	}

	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		fp_Column* pNext = static_cast<fp_Column *>(pCol->getNext());

		delete pCol;

		pCol = pNext;
	}
}

void fl_DocSectionLayout::setFirstEndnoteContainer(fp_EndnoteContainer * pECon)
{
	m_pFirstEndnoteContainer = pECon;
}


void fl_DocSectionLayout::setLastEndnoteContainer(fp_EndnoteContainer * pECon)
{
	m_pLastEndnoteContainer = pECon;
}


fp_Container * fl_DocSectionLayout::getFirstEndnoteContainer(void)
{
	fp_Container * pCon = static_cast<fp_Container *>(m_pFirstEndnoteContainer);
	return pCon;
}

fp_Container * fl_DocSectionLayout::getLastEndnoteContainer(void)
{
	fp_Container * pCon = static_cast<fp_Container *>(m_pLastEndnoteContainer);
	return pCon;
}


fl_FootnoteLayout * fl_DocSectionLayout::getFootnoteLayout(UT_uint32 pid)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	fl_FootnoteLayout * pFL = NULL;
	bool bFound = false;
	while(pCL && !bFound)
	{
		if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			pFL = static_cast<fl_FootnoteLayout *>(pCL);
			if(pFL->getFootnotePID() == pid)
			{
				bFound = true;
				break;
			}
		}
		pCL = pCL->getNext();
	}
	if(bFound)
	{
		return pFL;
	}
	return NULL;
}


fl_AnnotationLayout * fl_DocSectionLayout::getAnnotationLayout(UT_uint32 pid)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	fl_AnnotationLayout * pAL = NULL;
	bool bFound = false;
	while(pCL && !bFound)
	{
		if(pCL->getContainerType() == FL_CONTAINER_ANNOTATION)
		{
			pAL = static_cast<fl_AnnotationLayout *>(pCL);
			if(pAL->getAnnotationPID() == pid)
			{
				bFound = true;
				break;
			}
		}
		pCL = pCL->getNext();
	}
	if(bFound)
	{
		return pAL;
	}
	return NULL;
}

/*!
 * Returns the usuable height of the Column in logical units (after subtracting
 * top and bottom margins)
 */ 
UT_sint32 fl_DocSectionLayout::getActualColumnHeight(void)
{
	UT_sint32 Height = static_cast<UT_sint32>(m_pLayout->m_docViewPageSize.Height(DIM_IN) * UT_LAYOUT_RESOLUTION /m_pLayout->m_docViewPageSize.getScale());
	Height -= (getTopMargin() + getBottomMargin());
	if(m_iMaxSectionColumnHeight > 0)
	{
		Height = m_iMaxSectionColumnHeight;
	}
	return Height;
}


/*!
 * Returns the usuable width of the Column in logical units (after subtracting
 * left and right margins)
 */ 

UT_sint32 fl_DocSectionLayout::getActualColumnWidth(void)
{
	UT_sint32 width = static_cast<UT_sint32>(m_pLayout->m_docViewPageSize.Width(DIM_IN) * UT_LAYOUT_RESOLUTION /m_pLayout->m_docViewPageSize.getScale());
	width -= (getLeftMargin() + getRightMargin());
	if(m_iNumColumns > 1)
	{
		width -= m_iNumColumns*m_iColumnGap;
		width = width/m_iNumColumns;
	}
	return width;
}
			
UT_sint32 fl_DocSectionLayout::getWidth(void)
{
	UT_sint32 ires = m_pLayout->getGraphics()->getResolution();
	UT_sint32 width = static_cast<UT_sint32>(ires * m_pLayout->m_docViewPageSize.Width(DIM_IN));
	return width;
}

void fl_DocSectionLayout::setHdrFtr(HdrFtrType iType, fl_HdrFtrSectionLayout* pHFSL)
{
	if(pHFSL == NULL)
	{
		switch (iType)
		{
		case FL_HDRFTR_HEADER:
			m_pHeaderSL = pHFSL; break;
		case FL_HDRFTR_HEADER_EVEN:
			m_pHeaderEvenSL = pHFSL; break;
		case FL_HDRFTR_HEADER_FIRST:
			m_pHeaderFirstSL = pHFSL; break;
		case FL_HDRFTR_HEADER_LAST:
			m_pHeaderLastSL = pHFSL; break;
		case FL_HDRFTR_FOOTER:
			m_pFooterSL = pHFSL; break;
		case FL_HDRFTR_FOOTER_EVEN:
			m_pFooterEvenSL = pHFSL; break;
		case FL_HDRFTR_FOOTER_FIRST:
			m_pFooterFirstSL = pHFSL; break;
		case FL_HDRFTR_FOOTER_LAST:
			m_pFooterLastSL = pHFSL; break;
		case FL_HDRFTR_NONE:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
		}
		checkAndRemovePages();
		return;
	}
	const char* pszID = pHFSL->getAttribute("id");

	const char* pszAtt = NULL;

	pszAtt = getAttribute("header");
	if (pszAtt && (0 == strcmp(pszAtt, pszID)) && (iType == FL_HDRFTR_HEADER) )
	{
		m_pHeaderSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-even");
	if (pszAtt && (0 == strcmp(pszAtt, pszID))&& (iType == FL_HDRFTR_HEADER_EVEN) )
	{
		m_pHeaderEvenSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-first");
	if (pszAtt && (0 == strcmp(pszAtt, pszID))&& (iType == FL_HDRFTR_HEADER_FIRST) )
	{
		m_pHeaderFirstSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-last");
	if (pszAtt && (0 == strcmp(pszAtt, pszID))&& (iType == FL_HDRFTR_HEADER_LAST) )
	{
		m_pHeaderLastSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer");
	if (pszAtt && (0 == strcmp(pszAtt, pszID))&& (iType == FL_HDRFTR_FOOTER))
	{
		m_pFooterSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-even");
	if (pszAtt && (0 == strcmp(pszAtt, pszID)) && (iType == FL_HDRFTR_FOOTER_EVEN)	)
	{
		m_pFooterEvenSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-first");
	if (pszAtt && (0 == strcmp(pszAtt, pszID)) && (iType == FL_HDRFTR_FOOTER_FIRST)	)
	{
		m_pFooterFirstSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-last");
	if (pszAtt && (0 == strcmp(pszAtt, pszID)) && (iType == FL_HDRFTR_FOOTER_LAST)	)
	{
		m_pFooterLastSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

/*!
 * The calback that implements the HdrFtr size change
 */
void fl_DocSectionLayout::_HdrFtrChangeCallback(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);
	UT_DEBUGMSG(("Doing HdrFtr change callback %p \n",pWorker));
	// Get the docSectionLayout
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pWorker->getInstanceData());
	UT_return_if_fail(pDSL);

	// Win32 timers can fire prematurely on asserts (the dialog's
	// message pump releases the timers)
	if (!pDSL->getDocument())
	{
		return;
	}
	// Don't do anything while PT is changing
	PD_Document * pDoc = pDSL->getDocument();
	if(pDoc->isPieceTableChanging())
	{
		return;
	}
	if(pDSL->m_pLayout->isLayoutFilling())
	{
// FIXME:
// Don't resize on load for now. Put this back laters when I can workout
// how to avoid horrible slowdowns and crashes.
//
		pDSL->m_sHdrFtrChangeProps.clear();
		pDSL->m_pHdrFtrChangeTimer->stop();
		DELETEP(pDSL->m_pHdrFtrChangeTimer);
		return;
	}
	// Don't do anything while a redrawupdate is happening either...
	if(pDoc->isRedrawHappenning())
	{
		return;
	}
// Don't do anything until insertion point is allowed to change
	if (!pDoc->getAllowChangeInsPoint())
	{
		return;
	}
	fl_DocSectionLayout * pPrev = static_cast<fl_DocSectionLayout *>(pDSL->getPrev());
	bool bDoit = true;
	while(pPrev && bDoit)
	{
//
// If a timer has been set on a previous docsection don't do this until it's
// cleared.
//
		if(pPrev->m_pHdrFtrChangeTimer != NULL)
		{
			return;
		}
		fl_DocSectionLayout * pPPrev = static_cast<fl_DocSectionLayout *>(pDSL->getPrev());
		if(pPPrev != pPrev)
		{
			pPrev = pPPrev;
		}
		else
		{
			bDoit = false;
			break;
		}
	}
	const char * pProps = pDSL->m_sHdrFtrChangeProps.c_str();
	UT_DEBUGMSG(("Header/Footer change props %s \n",pProps));
	PP_PropertyVector atts = {
		"props", pProps
	};
	pDoc->notifyPieceTableChangeStart();
	FV_View * pView =  pDSL->m_pLayout->getView();
	pf_Frag_Strux* sdh = pDSL->getStruxDocHandle();
    PT_DocPosition insPos = pView->getPoint();
	fl_HdrFtrShadow * pShadow = pView->getEditShadow();
	HdrFtrType hfType = FL_HDRFTR_HEADER;
	if(pShadow)
	{
		hfType = pShadow->getHdrFtrSectionLayout()->getHFType();
	}
	UT_sint32 iPage = -1;
	if(pShadow)
	{
		iPage = pDSL->m_pLayout->findPage(pShadow->getPage());
	}
	pDoc->setMarginChangeOnly( true);
	pDoc->changeStruxFmtNoUndo(PTC_AddFmt, sdh, atts, PP_NOPROPS);
	pDoc->setMarginChangeOnly(false);
//
// Stop the resizer and delete and clear it's pointer. It's job is done now.
//
	pDSL->m_pHdrFtrChangeTimer->stop();
//
// update the screen
//
	pDSL->format();
	pDSL->formatAllHdrFtr();
	pDSL->updateLayout(true);
	pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	pDoc->notifyPieceTableChangeEnd();
	pDSL->m_sHdrFtrChangeProps.clear();
//
// Put the point at the right point in the header/footer on the right page.
//
	if(iPage >= 0)
	{
		fp_Page * pPage = pDSL->m_pLayout->getNthPage(iPage);
		if(pPage)
		{
			fp_ShadowContainer* pShadowC = pPage->getHdrFtrP(hfType);
			pShadow = pShadowC->getShadow();
			pView->setHdrFtrEdit(pShadow);
		}
	}
    pView->setPoint(insPos);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
    pView->setPoint(insPos);
	pView->ensureInsertionPointOnScreen();
	DELETEP(pDSL->m_pHdrFtrChangeTimer);
	pDSL->m_pHdrFtrChangeTimer = NULL;
}

/*!
 * This method is called if the top and bottom margins of the pages
 * have changed following an auto-resize request from the header.
 * We try to avoid a complete rebuild.
 */
void fl_DocSectionLayout::doMarginChangeOnly(void)
{
	const PP_AttrProp* pAP = NULL;
	getAP(pAP);
	UT_return_if_fail(pAP);

	const gchar* pszSectionType = NULL;
	pAP->getAttribute("type", pszSectionType);
	lookupProperties();
	fp_Page * pMyPage = m_pLayout->getFirstPage();
	while(pMyPage && pMyPage->getOwningSection() != this)
	{
		pMyPage = pMyPage->getNext();
	}
	if(pMyPage == NULL)
	{
		return;
	}
//
// Remove broken tables. They need to be rebroken.
//
	deleteBrokenTablesFromHere(NULL);
	while(pMyPage && pMyPage->getOwningSection() == this)
	{
		pMyPage->TopBotMarginChanged();
		pMyPage = pMyPage->getNext();
	}
//
// Rebreak the document. Place containers on their new pages.
//
	fl_DocSectionLayout * pDSL = this;
	while(pDSL)
	{
		pDSL->completeBreakSection();
		pDSL = pDSL->getNextDocSection();
	}
}
/*!
 * Signal a PT change at the next opportunity to change the height of a Hdr
 * (true) or footer (false)
 *
 * newHeight is the value in layout units of the new height of the 
 * header/footer
 *
 * In both caes the header/footers grow "into" the document area.
 */
bool fl_DocSectionLayout::setHdrFtrHeightChange(bool bHdrFtr, UT_sint32 newHeight)
{
//
// Look to see if we've already sent a signal and if we have to adjust
// the height of the HdrFtr
//
	if(bHdrFtr)
	{
	        UT_DEBUGMSG(("newHeight %d  m_iNewHdrHeight %d \n",newHeight,m_iNewHdrHeight));
		if(newHeight <= m_iNewHdrHeight)
		{
			return false;
		}
		m_iNewHdrHeight = newHeight;
		getDocument()->setNewHdrHeight(newHeight);
		UT_sint32 fullHeight = newHeight + getHeaderMargin();
		UT_String sHeight = m_pLayout->getGraphics()->invertDimension(DIM_IN, static_cast<double>(fullHeight));
		UT_String sProp = "page-margin-top";
		UT_String_setProperty(m_sHdrFtrChangeProps,sProp,sHeight);
	}
	else
	{
		if(newHeight <= m_iNewFtrHeight)
		{
			return false;
		}
		m_iNewFtrHeight = newHeight;
		getDocument()->setNewFtrHeight(newHeight);
		UT_sint32 fullHeight = newHeight + getFooterMargin();
		UT_String sHeight = m_pLayout->getGraphics()->invertDimension(DIM_IN, static_cast<double>(fullHeight));
		UT_String sProp = "page-margin-bottom";
		UT_String_setProperty(m_sHdrFtrChangeProps,sProp,sHeight);
	}
//
// OK the idea is to run the timer in the idle loop until the Piecetable
// is clear and we're not redrawing
//
// This means the resize will happen at the first opportunity after the
// current edit is finished.
//
	if(m_pHdrFtrChangeTimer == NULL)
	{
	    int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	    UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;

	    m_pHdrFtrChangeTimer = UT_WorkerFactory::static_constructor (_HdrFtrChangeCallback, this, inMode, outMode);

	    UT_ASSERT(m_pHdrFtrChangeTimer);
	    UT_ASSERT(outMode != UT_WorkerFactory::NONE);

		// If the worker is working on a timer instead of in the idle
		// time, set the frequency of the checks.
	    if ( UT_WorkerFactory::TIMER == outMode )
		{
			// this is really a timer, so it's safe to static_cast it
			static_cast<UT_Timer*>(m_pHdrFtrChangeTimer)->set(100);
		}
	    m_pHdrFtrChangeTimer->start();
	}
	return true;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeader(void) const
{
	return m_pHeaderSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooter(void) const
{
	return m_pFooterSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderEven(void) const
{
	return m_pHeaderEvenSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterEven(void) const
{
	return m_pFooterEvenSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderFirst(void) const
{
	return m_pHeaderFirstSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterFirst(void) const
{
	return m_pFooterFirstSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderLast(void) const
{
	return m_pHeaderLastSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterLast(void) const
{
	return m_pFooterLastSL;
}


fp_Container* fl_DocSectionLayout::getFirstContainer() const
{
	return m_pFirstColumn;
}

fp_Container* fl_DocSectionLayout::getLastContainer() const
{
	return m_pLastColumn;
}

void fl_DocSectionLayout::setFirstContainer(fp_Container * pCon)
{
	UT_DEBUGMSG(("docSectionLayout: DocSec %p First container set to %p \n",this,pCon));
	m_pFirstColumn = static_cast<fp_Column *>(pCon);
}


void fl_DocSectionLayout::setLastContainer(fp_Container * pCon)
{
	m_pLastColumn = static_cast<fp_Column *>(pCon);
}

/*!
  Create new container
  \return The newly created container

  This creates a new column or row of same.

*/
fp_Container* fl_DocSectionLayout::getNewContainer(fp_Container * pFirstContainer)
{
	fp_Page* pPage = NULL;
	fp_Column* pLastColumn = static_cast<fp_Column*>(getLastContainer());
	fp_Column* pAfterColumn = NULL;
	UT_sint32 iNextCtrHeight = 0;

	if (pLastColumn)
	{
		fp_Container * prevContainer = NULL;
		fp_Page* pTmpPage = NULL;
		UT_sint32 pageHeight = 0;
		pTmpPage = pLastColumn->getPage();
		iNextCtrHeight = 0;
		if(pFirstContainer != NULL)
		{
			prevContainer = static_cast<fp_Container *>(pFirstContainer->getPrevContainerInSection());
		}
		//
		// Look to see if this page already has columns from this docSections
		//
		bool bColAlready = false;
		UT_sint32 iCol = 0;
		for(iCol =0; pTmpPage->countColumnLeaders(); iCol++)
		{
		    if(pTmpPage->getNthColumnLeader(iCol)->getDocSectionLayout() == this)
		    {
		 	 bColAlready = true;
			 break;
		    }
		}
//
// Calculate from the page height up to prevContainer
//
		pageHeight = pTmpPage->getFilledHeight(prevContainer);
		UT_sint32 avail =  pTmpPage->getAvailableHeight();
		UT_sint32 newHeight = pageHeight+ 3*iNextCtrHeight;

		if(pFirstContainer != NULL)
		{
			iNextCtrHeight = pFirstContainer->getHeight();
		}
		else if( pLastColumn->getLastContainer())
		{
			iNextCtrHeight = pLastColumn->getLastContainer()->getHeight();
		}
		else
		{
			iNextCtrHeight =12*14; // approximately one average line
		}
		xxx_UT_DEBUGMSG(("SEVIOR: Pageheight =%d nextlineheight =%d newheight = %d availableheight =%d linepos %d \n",pageHeight,iNextCtrHeight,newHeight,avail));
		if( (newHeight  >= avail) || (pFirstContainer == NULL) || bColAlready)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Container on new page \n"));
			if (pTmpPage->getNext())
			{
				pPage = pTmpPage->getNext();
			}
			else
			{
				bool bIsFilling = m_pLayout->isLayoutFilling();
				pPage = m_pLayout->addNewPage(this,bIsFilling);
			}
		}
		else
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Container on current page \n"));
			pPage = pTmpPage;
			if(prevContainer == NULL)
			{
				pAfterColumn = pPage->getNthColumnLeader(pPage->countColumnLeaders()-1);
			}
			else
			{
				pAfterColumn = static_cast<fp_Column *>(prevContainer->getContainer())->getLeader();
			}

		}
	}
	else
	{
		// We currently have no columns in this section.  Time to
		// create some.  If there is a previous section, then we need
		// to start our section right after that one.  If not, then we
		// start our section on the first page.  If there is no first
		// page, then we need to create one.
		fl_DocSectionLayout* pPrevSL = getPrevDocSection();
		if (pPrevSL)
		{
			fp_Column * pPrevCol = static_cast<fp_Column *>(pPrevSL->getLastContainer());
			while(pPrevCol == NULL)
			{
				UT_DEBUGMSG(("BUG! BUG! Prev section has no last container! Attempting to fix this \n"));
				pPrevSL->format();
				pPrevCol = static_cast<fp_Column *>(pPrevSL->getLastContainer());
			}
			pPage = pPrevCol->getPage();
			pAfterColumn = pPage->getNthColumnLeader(pPage->countColumnLeaders()-1);
		}
		else
		{
			if (m_pLayout->countPages() > 0)
			{
				pPage = m_pLayout->getFirstPage();
			}
			else
			{
				pPage = m_pLayout->addNewPage(this,true);
			}
		}
	}

	UT_ASSERT(pPage);

	// Create row of columns
	fp_Column* pLeaderColumn = NULL;
	fp_Column* pTail = NULL;
	UT_uint32 i = 0;
	for (i=0; i<m_iNumColumns; i++)
	{
		fp_Column* pCol = new fp_Column(this);
		if (pTail)
		{
			pCol->setLeader(pLeaderColumn);
			pTail->setFollower(pCol);
			pTail->setNext(pCol);
			pCol->setPrev(pTail);

			pTail = pCol;
		}
		else
		{
			pLeaderColumn = pTail = pCol;
			pLeaderColumn->setLeader(pLeaderColumn);
		}
	}

	// Append added columns to any previous columns in this section.
	if (m_pLastColumn)
	{
		UT_ASSERT(m_pFirstColumn);

		m_pLastColumn->setNext(pLeaderColumn);
		pLeaderColumn->setPrev(m_pLastColumn);
	}
	else
	{
		UT_ASSERT(!m_pFirstColumn);
		UT_return_val_if_fail(pLeaderColumn, NULL);
		m_pFirstColumn = pLeaderColumn;
	}

	// Find last added column and set that as the last in the section.
	fp_Column* pLastNewCol = pLeaderColumn;
	while (pLastNewCol->getFollower())
	{
		pLastNewCol = pLastNewCol->getFollower();
	}
	m_pLastColumn = pLastNewCol;
	UT_ASSERT(!(m_pLastColumn->getNext()));
	UT_ASSERT(!(m_pLastColumn->getFollower()));

	pPage->insertColumnLeader(pLeaderColumn, pAfterColumn);

	fp_Column* pTmpCol = pLeaderColumn;
	i = 0;
 	while (pTmpCol)
	{
		UT_ASSERT(pTmpCol->getPage());

		pTmpCol = pTmpCol->getFollower();
		i++;
	}

	// Check if a frame needs to be inserted on this page
	if (m_pLayout->isLayoutFilling())
	{
	    fp_FrameContainer * pFrame = m_pLayout->findFramesToBeInserted(pPage);
	    while(pFrame)
	    {
		if (pPage->findFrameContainer(pFrame) < 0)
		{
		    pPage->insertFrameContainer(pFrame);
		}
		m_pLayout->removeFramesToBeInserted(pFrame);
		pFrame = m_pLayout->findFramesToBeInserted(pPage);
	    }
	}

	return pLeaderColumn;
}

void fl_DocSectionLayout::format(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	FV_View * pView = m_pLayout->getView();

	bool bShowHidden = pView && pView->getShowPara();
	FPVisibility eHidden;
	bool bHidden;

	while (pBL)
	{
		eHidden  = pBL->isHidden();
		bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		              || eHidden == FP_HIDDEN_REVISION
		              || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

		if(!bHidden)
		{
			pBL->format();
			UT_sint32 count = 0;
			while(pBL->getLastContainer() == NULL || pBL->getFirstContainer()==NULL)
			{
				UT_DEBUGMSG(("Error formatting a block try again \n"));
				count = count + 1;
				pBL->format();
				if(count > 3)
				{
					UT_DEBUGMSG(("Give up trying to format. Hope for the best :-( \n"));
					break;
				}
			}
		}

		pBL = pBL->getNext();
	}
	fp_Column * pCol = static_cast<fp_Column *>(getFirstContainer());
	if(pCol == NULL)
	{
	        m_bNeedsFormat = false;
		return;
	}
	//
	// When the document is first loaded, all the lines
	// in the section have been stuffed into the first column. 
	// When we do a break section, the lines that don't
	// fit in the first column are shuffled into the
	// second, then the ones that don't fit are shuffled
	// into the third etc. This leads to a N^2 slow down
	// in Breaksection. So instead we empty this column
	// and let BreakSection fill each empty column as 
	// needed.
	// 
      	if(pCol && m_pLayout->isLayoutFilling())
	{
	      pCol->removeAll();
	}

	m_ColumnBreaker.breakSection();
	m_bNeedsFormat = false;
}

void fl_DocSectionLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->markAllRunsDirty();
		pBL = pBL->getNext();
	}
	if(m_pHeaderSL)
	{
		m_pHeaderSL->markAllRunsDirty();
	}
	if(m_pHeaderEvenSL)
	{
		m_pHeaderEvenSL->markAllRunsDirty();
	}
	if(m_pHeaderFirstSL)
	{
		m_pHeaderFirstSL->markAllRunsDirty();
	}
	if(m_pHeaderLastSL)
	{
		m_pHeaderLastSL->markAllRunsDirty();
	}
	if(m_pFooterSL)
	{
		m_pFooterSL->markAllRunsDirty();
	}
	if(m_pFooterEvenSL)
	{
		m_pFooterEvenSL->markAllRunsDirty();
	}
	if(m_pFooterFirstSL)
	{
		m_pFooterFirstSL->markAllRunsDirty();
	}
	if(m_pFooterLastSL)
	{
		m_pFooterLastSL->markAllRunsDirty();
	}
}

void fl_DocSectionLayout::updateLayout(bool bDoFull)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	FV_View * pView = m_pLayout->getView();
	bool bShowHidden = pView && pView->getShowPara();
	FPVisibility eHidden;
	bool bHidden;
	//
	// FIXME!! Do extensive tests to see if we can remove this line!
	//
	bDoFull = true;
	UT_DEBUGMSG(("Doing DocSection Update layout (section %p)\n",this));
	if (!bDoFull || (m_vecFormatLayout.getItemCount() > 0))
	{
	        UT_sint32 i =0;
		UT_sint32 j = 0;
		UT_sint32 count = m_vecFormatLayout.getItemCount();
		for(i=0; i<count; i++)
		{  
		        if(j >= m_vecFormatLayout.getItemCount())
			    break;
		        pBL = m_vecFormatLayout.getNthItem(j);
			j++;
		        eHidden  = pBL->isHidden();
			bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
				   || eHidden == FP_HIDDEN_REVISION
				   || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

			if(!bHidden)
			{
			  xxx_UT_DEBUGMSG(("container %x type %s needformat %d \n",pBL,pBL->getContainerString(),pBL->needsReformat()));
			     if (pBL->needsReformat())
			     {
			          if(!(m_pLayout->isLayoutFilling() && pBL->getContainerType() == FL_CONTAINER_TOC))
				  {
				       pBL->format();
				       j--;
				       if(j < m_vecFormatLayout.getItemCount())
				       {
					    UT_sint32 k = m_vecFormatLayout.findItem(pBL);
					    if(k == j)
					         m_vecFormatLayout.deleteNthItem(j);
				       }
				  }
			     }
			     if (pBL->getContainerType() != FL_CONTAINER_BLOCK && !getDocument()->isDontImmediateLayout())
			     {
			          pBL->updateLayout(false);
			     }
			}
			
		}
	}
	else if(bDoFull)
	{
	        while (pBL)
		{
		        eHidden  = pBL->isHidden();
			bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
				   || eHidden == FP_HIDDEN_REVISION
				   || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

			if(!bHidden)
			{
			     if (pBL->needsReformat())
			     {
			          if(!(m_pLayout->isLayoutFilling() && pBL->getContainerType() == FL_CONTAINER_TOC))
				  {
				       pBL->format();
				  }
			     }
			     if (pBL->getContainerType() != FL_CONTAINER_BLOCK && !getDocument()->isDontImmediateLayout())
			     {
			          pBL->updateLayout(false);
			     }
			}

			pBL = pBL->getNext();
		}
	}
	m_vecFormatLayout.clear();
	if(needsSectionBreak() && !getDocument()->isDontImmediateLayout() )
	{
		if (!isFirstPageValid())
		{
			// The previous section ends on a page farther in the document than the present section
			// first page. We reformat completely the section
			// TODO: a better method would be to create one new page and move the present section
			//       first page columns to that new page. Then insert the page at the correct place 
			//       in the page list. If the section starts with a page break, there would not be any 
			//       other changes necessary to the document.
			collapse();
			format();
			return;
		}
		m_ColumnBreaker.breakSection();
	}
	if(needsRebuild() && !getDocument()->isDontImmediateLayout() )
	{
		checkAndRemovePages();
		addValidPages();
	}
}

void fl_DocSectionLayout::setNeedsSectionBreak(bool bSet, fp_Page * pPage)
{
	m_bNeedsSectionBreak = bSet;
	fp_Page * pOldP = m_ColumnBreaker.getStartPage();
	UT_sint32 iOldP = 999999999;
	if(pPage == NULL)
	{
	  UT_DEBUGMSG(("SectionBreak from start \n"));
		m_ColumnBreaker.setStartPage(pPage);
		return;
	}	
	if(pPage->getOwningSection() != this)
	{
		m_ColumnBreaker.setStartPage(NULL);
		return;
	}	
	if(pOldP)
	{
		iOldP = getDocLayout()->findPage(pOldP);
	}
	UT_sint32 iNewP = getDocLayout()->findPage(pPage);
	xxx_UT_DEBUGMSG(("setNeedsSectionBreak: new Page %d OldPage %d \n",iNewP,iOldP));	
	if( (iNewP > -1) && (iNewP < iOldP))
	{
		xxx_UT_DEBUGMSG(("setNeedsSectionBreak: Rebuild from Page %x \n",pPage));	
		m_ColumnBreaker.setStartPage(pPage);
	}
}


void fl_DocSectionLayout::completeBreakSection(void)
{
	m_bNeedsSectionBreak = true;
	updateLayout(true);
	m_ColumnBreaker.setStartPage(NULL);
	m_ColumnBreaker.breakSection();
	m_bNeedsSectionBreak = false;
}


void fl_DocSectionLayout::redrawUpdate(void)
{
        if(getDocLayout()->isLayoutFilling())
	         return;
	fl_ContainerLayout*	pBL = getFirstLayout();

	// we only need to break and redo this section if its contents
	// have changed, i.e., if the field values changed
	xxx_UT_DEBUGMSG(("Doing redraw update \n"));
	while (pBL)
	{
		if(pBL->getContainerType() == FL_CONTAINER_BLOCK && static_cast<fl_BlockLayout *>(pBL)->hasUpdatableField())
		{
			bool bReformat = pBL->recalculateFields(getDocLayout()->getRedrawCount());
			if(bReformat)
			{
				pBL->format();
			}
		}
		else
		{
			pBL->recalculateFields(getDocLayout()->getRedrawCount());
		}
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}

		pBL = pBL->getNext();
	}
	xxx_UT_DEBUGMSG(("Finished Doing redraw update \n"));
	fp_EndnoteContainer * pECon =  static_cast<fp_EndnoteContainer *>(getFirstEndnoteContainer());
	if(pECon)
	{
		fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
		while(pEL)
		{
			pEL->redrawUpdate();
			pEL = static_cast<fl_EndnoteLayout *>(pEL->getNext());
		}
	}
	if(!getDocLayout()->isLayoutFilling() && (needsSectionBreak() || needsRebuild()))
	{
		m_ColumnBreaker.breakSection();
		m_bNeedsSectionBreak = false;
	
		if(needsRebuild())
		{
//			UT_ASSERT(0);
			checkAndRemovePages();
			addValidPages();
			m_bNeedsRebuild = false;
		}
	}

}

bool fl_DocSectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	PT_AttrPropIndex IndexOld = getAttrPropIndex();
	
	setAttrPropIndex(pcrxc->getIndexAP());

	const PP_AttrProp * pAP1;
	getDocument()->getAttrProp(IndexOld, &pAP1);

	const PP_AttrProp * pAP2;
	getDocument()->getAttrProp(pcrxc->getIndexAP(), &pAP2);

	if(!pAP1 || !pAP2)
	{
		getDocLayout()->rebuildFromHere(this);
	}

	const gchar * prop = "dom-dir";
	const gchar * val1 = NULL;
	const gchar * val2 = NULL;

	pAP1->getProperty(prop, val1);
	pAP2->getProperty(prop, val2);

	if(!val1 || !val2 || strcmp(val1, val2))
	{
		lookupProperties();
		fl_ContainerLayout * pCL = getFirstLayout();
		while(pCL)
		{
			pCL->lookupProperties();
			pCL = pCL->getNext();
		}
		
		getDocLayout()->rebuildFromHere(this);
	}
	
	
	return true;
}

void fl_DocSectionLayout::updateDocSection(void)
{

	const PP_AttrProp* pAP = NULL;
	getAP(pAP);
	UT_return_if_fail(pAP);

	const gchar* pszSectionType = NULL;
	pAP->getAttribute("type", pszSectionType);
	lookupProperties();

	// clear all the columns
    // Assume that all columns and formatting have already been removed via a collapseDocSection()
    //

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}
	setNeedsSectionBreak(true,NULL);
	format();
	checkAndRemovePages();
	formatAllHdrFtr();
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

//	if (pView)
//	{
//		pView->updateScreen(false);
//		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTSECTION);
//	}

	return;
}

void fl_DocSectionLayout::_lookupMarginProperties(const PP_AttrProp* /*pSectionAP*/)
{
	// force lookup on all container layouts in this section

	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->lookupMarginProperties();
		pBL = pBL->getNext();
	}

	// header/footers
	UT_GenericVector<fl_HdrFtrSectionLayout*> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = vecHdrFtr.getNthItem(i);
		pHdrFtr->lookupMarginProperties();
	}
	
}

void fl_DocSectionLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);

	m_iNewHdrHeight = getDocument()->getNewHdrHeight();
	m_iNewFtrHeight = getDocument()->getNewFtrHeight();

	m_sHdrFtrChangeProps.clear();


	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/

	const char* pszNumColumns = NULL;
	pSectionAP->getProperty("columns", (const gchar *&)pszNumColumns);
	if (pszNumColumns && pszNumColumns[0])
	{
		m_iNumColumns = atoi(pszNumColumns);
	}
	else
	{
		m_iNumColumns = 1;
	}

	const char* pszColumnGap = NULL;
	pSectionAP->getProperty("column-gap", (const gchar *&)pszColumnGap);
	if (pszColumnGap && pszColumnGap[0])
	{
		m_iColumnGap = UT_convertToLogicalUnits(pszColumnGap);
	}
	else
	{
		m_iColumnGap = UT_convertToLogicalUnits("0.25in");
	}
	UT_ASSERT(m_iColumnGap < 2000000);
	const char* pszColumnLineBetween = NULL;
	pSectionAP->getProperty("column-line", (const gchar *&)pszColumnLineBetween);
	if (pszColumnLineBetween && pszColumnLineBetween[0])
	{
		m_bColumnLineBetween = (strcmp(pszColumnLineBetween, "on") == 0) ? true : false;
	}
	else
	{
		m_bColumnLineBetween = false;
	}

	/* column-order */
	//we use the mechanism used by BlockLayout, since otherwise we
	//cannot recode the default value
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;

	const char * pszColumnOrder = PP_evalProperty("dom-dir",pSpanAP,pBlockAP,pSectionAP,m_pDoc,false);
    UT_DEBUGMSG(("column order: %s\n", pszColumnOrder));

	FV_View * pView = m_pLayout->getView();

	if(pView && pView->getBidiOrder() != FV_Order_Visual)
	{
		if(pView->getBidiOrder() == FV_Order_Logical_LTR)
			m_iColumnOrder = 0;
		else
			m_iColumnOrder = 0;
	}
	else if(pszColumnOrder && pszColumnOrder[0])
	{
		m_iColumnOrder = strcmp(pszColumnOrder, "ltr")	? 1 : 0;
	}
	else
	{
		m_iColumnOrder = 0;
	}

	const char* pszSpaceAfter = NULL;
	pSectionAP->getProperty("section-space-after", (const gchar *&)pszSpaceAfter);
	if (pszSpaceAfter && pszSpaceAfter[0])
	{
		m_iSpaceAfter = UT_convertToLogicalUnits(pszSpaceAfter);
	}
	else
	{
		m_iSpaceAfter = UT_convertToLogicalUnits("0in");
	}

	const char* pszRestart = NULL;
	pSectionAP->getProperty("section-restart", (const gchar *&)pszRestart);
	if (pszRestart && pszRestart[0])
	{
		m_bRestart = (strcmp(pszRestart,"1")==0);
	}
	else
	{
		m_bRestart = false;
	}

	const char* pszRestartValue = NULL;
	pSectionAP->getProperty("section-restart-value", (const gchar *&)pszRestartValue);
	if (pszRestartValue && pszRestartValue[0])
	{
		m_iRestartValue = atoi(pszRestartValue);
	}
	else
	{
		m_iRestartValue = 1;
	}

	const char* pszLeftMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszBottomMargin = NULL;
	const char* pszFooterMargin = NULL;
	const char* pszHeaderMargin = NULL;
	const char* pszMaxColumnHeight = NULL;
	pSectionAP->getProperty("page-margin-left", (const gchar *&)pszLeftMargin);
	pSectionAP->getProperty("page-margin-top", (const gchar *&)pszTopMargin);
	pSectionAP->getProperty("page-margin-right", (const gchar *&)pszRightMargin);
	pSectionAP->getProperty("page-margin-bottom", (const gchar *&)pszBottomMargin);
	pSectionAP->getProperty("page-margin-footer", (const gchar *&)pszFooterMargin);
	pSectionAP->getProperty("page-margin-header", (const gchar *&)pszHeaderMargin);


	const gchar * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_UTF8String defaultMargin = fp_PageSize::getDefaultPageMargin(dim);

	if(pszLeftMargin && pszLeftMargin[0])
	{
		m_iLeftMargin = UT_convertToLogicalUnits(pszLeftMargin);
		m_dLeftMarginUserUnits = UT_convertDimensionless(pszLeftMargin);
	}
	else
	{
		m_iLeftMargin = UT_convertToLogicalUnits(defaultMargin.utf8_str());
		m_dLeftMarginUserUnits = UT_convertDimensionless(defaultMargin.utf8_str());
	}

	if(pszTopMargin && pszTopMargin[0])
	{
		m_iTopMargin = UT_convertToLogicalUnits(pszTopMargin);
		m_dTopMarginUserUnits = UT_convertDimensionless(pszTopMargin);
	}
	else
	{
		m_iTopMargin = UT_convertToLogicalUnits(defaultMargin.utf8_str());
		m_dTopMarginUserUnits = UT_convertDimensionless(defaultMargin.utf8_str());
	}

	if(pszRightMargin && pszRightMargin[0])
	{
		m_iRightMargin = UT_convertToLogicalUnits(pszRightMargin);
		m_dRightMarginUserUnits = UT_convertDimensionless(pszRightMargin);
	}
	else
	{
		m_iRightMargin = UT_convertToLogicalUnits(defaultMargin.utf8_str());
		m_dRightMarginUserUnits = UT_convertDimensionless(defaultMargin.utf8_str());
	}

	if(pszBottomMargin && pszBottomMargin[0])
	{
		m_iBottomMargin = UT_convertToLogicalUnits(pszBottomMargin);
		m_dBottomMarginUserUnits = UT_convertDimensionless(pszBottomMargin);
	}
	else
	{
		m_iBottomMargin = UT_convertToLogicalUnits(defaultMargin.utf8_str());
		m_dBottomMarginUserUnits = UT_convertDimensionless(defaultMargin.utf8_str());
	}

	if(pszFooterMargin && pszFooterMargin[0])
	{
		m_iFooterMargin = UT_convertToLogicalUnits(pszFooterMargin);
		m_dFooterMarginUserUnits = UT_convertDimensionless(pszFooterMargin);
	}
	else
	{
		m_iFooterMargin = UT_convertToLogicalUnits("0.0in");
		m_dFooterMarginUserUnits = UT_convertDimensionless("0.0in");
	}

	if(pszHeaderMargin && pszHeaderMargin[0])
	{
		m_iHeaderMargin = UT_convertToLogicalUnits(pszHeaderMargin);
		m_dHeaderMarginUserUnits = UT_convertDimensionless(pszHeaderMargin);
	}
	else
	{
		m_iHeaderMargin = UT_convertToLogicalUnits("0.0in");
		m_dHeaderMarginUserUnits = UT_convertDimensionless("0.0in");
	}

	pSectionAP->getProperty("section-max-column-height", (const gchar *&)pszMaxColumnHeight);
	if (pszMaxColumnHeight && pszMaxColumnHeight[0])
	{
		m_iMaxSectionColumnHeight = UT_convertToLogicalUnits(pszMaxColumnHeight);
	}
	else
	{
		m_iMaxSectionColumnHeight = UT_convertToLogicalUnits("0in");
	}

	const gchar * pszFootnoteLine = NULL;
	pSectionAP->getProperty("section-footnote-line-thickness", (const gchar *&)pszFootnoteLine);
	if (pszFootnoteLine && pszFootnoteLine[0])
	{
		m_iFootnoteLineThickness = UT_convertToLogicalUnits(pszFootnoteLine);
	}
	else
	{
		m_iFootnoteLineThickness = UT_convertToLogicalUnits("0.005in");
	}


	const gchar * pszFootnoteYoff = NULL;
	pSectionAP->getProperty("section-footnote-yoff", (const gchar *&)pszFootnoteYoff);
	if (pszFootnoteYoff && pszFootnoteYoff[0])
	{
		m_iFootnoteYoff = UT_convertToLogicalUnits(pszFootnoteYoff);
	}
	else
	{
		m_iFootnoteYoff = UT_convertToLogicalUnits("0.01in");
	}

	const gchar * pszDataID = NULL;
	pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&)pszDataID);
	DELETEP(m_pGraphicImage);
	DELETEP(m_pImageImage);
	if(pszDataID && *pszDataID)
	{
		m_pGraphicImage = FG_Graphic::createFromStrux(this);
	}
	setPaperColor();
}

UT_sint32 fl_DocSectionLayout::getTopMargin(void) const
{
	return m_iTopMargin;
}

UT_sint32 fl_DocSectionLayout::getBottomMargin(void) const
{
	return m_iBottomMargin;
}

/*!
 * Set the color of the background paper in the following order of precedence
 * 1. If The section level proper "background-color" is present and is
 *    not transparent use that.
 * 2. If this section is being displayed to the screen use the
 *     ColorForTransparency preference item color.
 * 3. Otherwise use white
 */
void fl_DocSectionLayout::setPaperColor(void)
{
	const PP_AttrProp* pSectionAP = NULL;
	getAP(pSectionAP);
	UT_return_if_fail(pSectionAP);

	const char* pszClrPaper = NULL;
	pSectionAP->getProperty("background-color", (const gchar *&)pszClrPaper);
	FV_View * pView = m_pLayout->getView();
	if(pszClrPaper && strcmp(pszClrPaper,"transparent") != 0)
	{
		m_sPaperColor = pszClrPaper;
		m_sScreenColor.clear();
	}
	else if( pView && pView->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) )
	{
		XAP_App * pApp = pView->getApp();
		XAP_Prefs * pPrefs = pApp->getPrefs();
		const gchar * pszTransparentColor = NULL;
		pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForTransparent),&pszTransparentColor);
		m_sPaperColor.clear();
		m_sScreenColor = pszTransparentColor;
	}
	else
	{
		m_sPaperColor.clear();
		m_sScreenColor.clear();
	}
}

/*!
 * Delete Empty Column containers in this section.
 */
void fl_DocSectionLayout::deleteEmptyColumns(void)
{
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			fp_Column* pCol2 = pCol;
			bool bAllEmpty = true;
			fp_Column* pLastInGroup = NULL;

			while (pCol2)
			{
				if (!pCol2->isEmpty())
				{
					bAllEmpty = false;
				}

				if (!(pCol2->getFollower()))
				{
					pLastInGroup = pCol2;
				}

				pCol2 = pCol2->getFollower();
			}

			if (bAllEmpty)
			{
				UT_ASSERT(pLastInGroup);
				if(pCol->getPage() != NULL)
				{
					pCol->getPage()->removeColumnLeader(pCol);
				}
				if (pCol == m_pFirstColumn)
				{
					m_pFirstColumn = static_cast<fp_Column *>(pLastInGroup->getNext());
					UT_ASSERT(m_pFirstColumn);
				}

				if (pLastInGroup == m_pLastColumn)
				{
					m_pLastColumn = static_cast<fp_Column *>(pCol->getPrev());
				}

				if (pCol->getPrev())
				{
					pCol->getPrev()->setNext(pLastInGroup->getNext());
				}

				if (pLastInGroup->getNext())
				{
					pLastInGroup->getNext()->setPrev(pCol->getPrev());
				}

				fp_Column* pCol3 = pCol;
				pCol = static_cast<fp_Column *>(pLastInGroup->getNext());
				while (pCol3)
				{
					fp_Column* pNext = pCol3->getFollower();

					delete pCol3;

					pCol3 = pNext;
				}
			}
			else
			{
				pCol = static_cast<fp_Column *>(pLastInGroup->getNext());
			}
		}
		else
		{
			pCol = static_cast<fp_Column *>(pCol->getNext());
		}
	}
}

UT_uint32 fl_DocSectionLayout::getNumColumns(void) const
{
	return m_iNumColumns;
}

UT_sint32 fl_DocSectionLayout::getColumnGap(void) const
{
	UT_ASSERT( m_iColumnGap < 200000);
	return m_iColumnGap;
}

UT_uint32 fl_DocSectionLayout::getColumnOrder(void) const
{
	return m_iColumnOrder;
}

void fl_DocSectionLayout::deleteBrokenTablesFromHere(fl_ContainerLayout * pTL)
{
	xxx_UT_DEBUGMSG(("Doing delete broken tables from here \n"));
	if(m_bDeleteingBrokenContainers)
	{
		return;
	}
	if(!m_pLayout || m_pLayout->isLayoutDeleting())
	{
	        return;
	}
	m_bDeleteingBrokenContainers = true;
	if(pTL == NULL)
	{
		pTL = getFirstLayout();
	}
	fl_ContainerLayout * pCL = pTL->getNext();
	while(pCL != NULL)
	{
		if(pCL->getContainerType() == FL_CONTAINER_TABLE)
		{
			fl_TableLayout * pTabL = static_cast<fl_TableLayout *>(pCL);
			fp_TableContainer * pTabC = static_cast<fp_TableContainer *>(pTabL->getFirstContainer());
			if(pTabC != NULL)
			{
				pTabC->deleteBrokenTables(true);
			}
		}
		else if(pCL->getContainerType() == FL_CONTAINER_TOC)
		{
			fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(pCL);
			fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(pTOCL->getFirstContainer());
			if(pTOC != NULL)
			{
				pTOC->deleteBrokenTOCs(true);
			}
		}
		pCL = pCL->getNext();
	}
	m_bDeleteingBrokenContainers = false;
}


fl_DocSectionLayout* fl_DocSectionLayout::getNextDocSection(void) const
{
	fl_SectionLayout * pSL = static_cast<fl_SectionLayout*>(getNext());
	if(pSL != NULL && pSL->getType()== FL_SECTION_DOC)
		return static_cast<fl_DocSectionLayout*>(pSL);
	return NULL;
}

fl_DocSectionLayout* fl_DocSectionLayout::getPrevDocSection(void) const
{
	fl_SectionLayout * pSL = static_cast<fl_SectionLayout*>(getPrev());
	while(pSL != NULL && pSL->getType()!= FL_SECTION_DOC)
	{
		pSL = static_cast<fl_SectionLayout*>(pSL->getPrev());
	}
	return static_cast<fl_DocSectionLayout*>(pSL);
}

void fl_DocSectionLayout::collapse(void)
{
	UT_DEBUGMSG(("DocSectionLayout: Collapsing all content in %p \n",this));
	fp_Column* pCol2 = m_pFirstColumn;
	m_bDoingCollapse = true;
	while (pCol2)
	{
		pCol2->clearScreen();

		pCol2 = static_cast<fp_Column *>(pCol2->getNext());
	}
	//
	// Clear the header/footers too
	//
	UT_GenericVector<fl_HdrFtrSectionLayout*> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = vecHdrFtr.getNthItem(i);
		pHdrFtr->clearScreen();
	}
	//
	// Collapse the header/footers now
	//
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = vecHdrFtr.getNthItem(i);
		pHdrFtr->collapse();
	}
	// remove all the columns from their pages
	pCol2 = m_pFirstColumn;
	while (pCol2)
	{
//
// The endnote in a column may originate from a totally different 
// docsection. We must collapse these endnotes first
//
		pCol2->collapseEndnotes();
		if (pCol2->getLeader() == pCol2)
		{
			pCol2->getPage()->removeColumnLeader(pCol2);
		}

		pCol2 = static_cast<fp_Column *>(pCol2->getNext());
	}

	// get rid of all the layout information for every block
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if(pBL->getContainerType() == FL_CONTAINER_ENDNOTE)
		{
			fp_Container * pCon = pBL->getFirstContainer();
			UT_ASSERT_HARMLESS( pCon );
			if(pCon)
			{
				fp_Column * pCol = static_cast<fp_Column *>(pCon->getColumn());
				UT_DEBUGMSG(("Got and endnote in this section!! \n"));
				UT_DEBUGMSG(("Remove Endnote con %p from col %p \n",pCon,pCol));
				pCol->removeContainer(pCon);
			}
		}
		pBL->collapse();
		pBL = pBL->getNext();

	}

	// delete all our columns
	pCol2 = m_pFirstColumn;
	while (pCol2)
	{
		fp_Column* pNext = static_cast<fp_Column *>(pCol2->getNext());
		delete pCol2;
		pCol2 = pNext;	
	}
	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;
	setFirstEndnoteContainer(NULL);
	setLastEndnoteContainer(NULL);
//
// Remove all the empty pages thus created. Don't notify of the deletion though.
//
	fp_Page* pPage = m_ColumnBreaker.getStartPage();
	if (pPage && pPage->isEmpty())
		m_ColumnBreaker.setStartPage(NULL);
	getDocLayout()->deleteEmptyPages(true);
//
// This Doc Section No longer owns pages so this becomes NULL
//
	m_pFirstOwnedPage = NULL;
	m_bDoingCollapse = false;
}

bool fl_DocSectionLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Section);
	UT_DEBUGMSG(("Doing Section delete \n"));
	fl_DocSectionLayout* pPrevSL = getPrevDocSection();
	UT_DEBUGMSG(("Deleting DocSec %p Prev DocSec %p \n",this,pPrevSL));
	if (!pPrevSL)
	{
		// TODO shouldn't this just assert?
		UT_DEBUGMSG(("no prior SectionLayout"));
		return false;
	}

//
// Collapse previous section too. We need this so it can be rebuilt properly.
//
	pPrevSL->collapse();

	// clear all the columns
	collapse();

	DELETEP(m_pHeaderSL);
	DELETEP(m_pHeaderEvenSL);
	DELETEP(m_pHeaderFirstSL);
	DELETEP(m_pHeaderLastSL);
	DELETEP(m_pFooterSL);
	DELETEP(m_pFooterEvenSL);
	DELETEP(m_pFooterFirstSL);
	DELETEP(m_pFooterLastSL);

//
// Collapse the subsequent sections too. These will be reformatted in a few lines.
//
	fl_DocSectionLayout * pDSL = getNextDocSection();
	while(pDSL != NULL)
	{
		pDSL->collapse();
		pDSL = pDSL->getNextDocSection();
	}
//
// OK set the links and move all blocks in this section into the previous section.
	if(getFirstLayout())
	{
		fl_ContainerLayout * pBCur = getFirstLayout();
		fl_ContainerLayout * pBPrev = pPrevSL->getLastLayout();
		UT_ASSERT(pBCur && pBPrev);
	
		pBCur->setPrev(pBPrev);
		pBPrev->setNext(pBCur);
		while(pBCur != NULL)
		{
			xxx_UT_DEBUGMSG(("updating block %x \n",pBCur));
			pBCur->setContainingLayout(pPrevSL);
			if(pBCur->getContainerType() == FL_CONTAINER_BLOCK)
			{
				static_cast<fl_BlockLayout *>(pBCur)->
					setSectionLayout(pPrevSL);
			}
			if(pBCur->getContainerType() == FL_CONTAINER_FOOTNOTE)
			{
				static_cast<fl_FootnoteLayout *>(pBCur)->
					setDocSectionLayout(pPrevSL);
			}
			if(pBCur->getContainerType() == FL_CONTAINER_ANNOTATION)
			{
				static_cast<fl_AnnotationLayout *>(pBCur)->
					setDocSectionLayout(pPrevSL);
			}
			if(pBCur->getContainerType() == FL_CONTAINER_ENDNOTE)
			{
				static_cast<fl_EndnoteLayout *>(pBCur)->
					setDocSectionLayout(pPrevSL);
			}

			pPrevSL->setLastLayout(pBCur);
			pBCur = pBCur->getNext();
		}
	}
	setFirstLayout(NULL);
	setLastLayout(NULL);

//
// Get this before we remove this section from the run list!
//
    pDSL = getNextDocSection();
	m_pLayout->removeSection(this);
	pPrevSL->format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
  	{
  		pView->_setPoint(pcrx->getPosition());
  	}
//
// Update the following sections.
//
	while(pDSL != NULL)
	{
		pDSL->updateDocSection();
		pDSL = pDSL->getNextDocSection();
	}
	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

void fl_DocSectionLayout::addOwnedPage(fp_Page* pPage)
{
	// TODO do we really need the vecOwnedPages member? YES!!!

	if(m_pFirstOwnedPage == NULL)
		m_pFirstOwnedPage = pPage;
	fp_Page * pPrev = m_pFirstOwnedPage;
	pPage->getFillType().setDocLayout(getDocLayout());
	setImageWidth(pPage->getWidth());
	setImageHeight(pPage->getHeight());
	if(m_pGraphicImage)
	{
		if(m_pImageImage == NULL)
		{
			const PP_AttrProp * pAP = NULL;
			getAP(pAP);
			GR_Image * pImage = m_pGraphicImage->generateImage(getDocLayout()->getGraphics(),pAP,pPage->getWidth(),pPage->getHeight());
			m_iGraphicTick = getDocLayout()->getGraphicTick();
			UT_Rect rec(0,0,pPage->getWidth(),pPage->getHeight());
			pImage->scaleImageTo(getDocLayout()->getGraphics(),rec);
			m_pImageImage = pImage;
		}
		pPage->getFillType().setImagePointer(&m_pGraphicImage,&m_pImageImage);
	}
	else if(m_sPaperColor.size() > 0)
	{
		pPage->getFillType().setColor(m_sPaperColor.c_str());
	}
	else if(m_sScreenColor.size() > 0)
	{
		pPage->getFillType().setTransColor(m_sScreenColor.c_str());
		pPage->getFillType().markTransparentForPrint();
	}		

//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() < FL_HDRFTR_FOOTER)
		{
			if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_HEADER) == NULL )
				prependOwnedHeaderPage(pPrev);

			pHdrFtr->addPage(pPage);
		}
		else
		{
			if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_FOOTER) == NULL)
			{
				prependOwnedFooterPage(pPrev);
			}
			pHdrFtr->addPage(pPage);
		}
	}

	// Keep track of the number of pages in the section
	m_iPageCount++;

	fl_DocSectionLayout * pDSL = this;
	while(pDSL != NULL)
	{
		pDSL->checkAndRemovePages();
		pDSL->addValidPages();
		pDSL = pDSL->getNextDocSection();
	}
}

void fl_DocSectionLayout::prependOwnedHeaderPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_HEADER) == NULL)
	{
		prependOwnedHeaderPage(pPrev);
	}
//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() < FL_HDRFTR_FOOTER)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: prepending page %x \n",pPage));
			pHdrFtr->addPage(pPage);
		}
	}
}


void fl_DocSectionLayout::prependOwnedFooterPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_FOOTER) == NULL)
	{
		prependOwnedFooterPage(pPrev);
	}
//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() >= FL_HDRFTR_FOOTER)
		{
			pHdrFtr->addPage(pPage);
		}
	}
}


/*!
 * This fills a vector with all the valid header/footers.
 */
void fl_DocSectionLayout::getVecOfHdrFtrs(UT_GenericVector<fl_HdrFtrSectionLayout *> * vecHdrFtr)
{
	vecHdrFtr->clear();
	if (m_pHeaderFirstSL != NULL)
	{
		vecHdrFtr->addItem(m_pHeaderFirstSL);
	}
	if (m_pHeaderLastSL  != NULL)
	{
		vecHdrFtr->addItem(m_pHeaderLastSL);
	}
	if (m_pHeaderEvenSL  != NULL)
	{
		vecHdrFtr->addItem(m_pHeaderEvenSL);
	}
	if (m_pHeaderSL  != NULL)
	{
		vecHdrFtr->addItem(m_pHeaderSL);
	}
	if (m_pFooterFirstSL != NULL)
	{
		vecHdrFtr->addItem(m_pFooterFirstSL);
	}
	if (m_pFooterLastSL != NULL)
	{
		vecHdrFtr->addItem(m_pFooterLastSL);
	}
	if (m_pFooterEvenSL != NULL)
	{
		vecHdrFtr->addItem(m_pFooterEvenSL);
	}
	if (m_pFooterSL != NULL)
	{
		vecHdrFtr->addItem(m_pFooterSL);
	}
}

/*!
 * This method formats all the header/footers
 */
void fl_DocSectionLayout::formatAllHdrFtr(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Doing formatAllHdrFtr \n"));
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		xxx_UT_DEBUGMSG(("SEVIOR: Doing formatting %x in formatAllHdrFtr \n",pHdrFtr));
		pHdrFtr->format();
	}
}

/*!
 * This method checks each header for valid pages and removes the page if it's not
 * valid. ie it remove odd pages from even headers etc.
 */
void fl_DocSectionLayout::checkAndRemovePages(void)
{
	UT_GenericVector <fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		pHdrFtr->checkAndRemovePages();
	}
}


/*!
 * This method adds valid pages to every valid header/footer in the docsection if
 * they're not there already.
 */
void fl_DocSectionLayout::addValidPages(void)
{
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		pHdrFtr->addValidPages();
	}
}

/*!
 * This method deletes the owned page from the DocSectionLayout and all
 * the header files.
 */
void fl_DocSectionLayout::deleteOwnedPage(fp_Page* pPage, bool bReallyDeleteIt)
{
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_sint32 i = 0;
	xxx_UT_DEBUGMSG(("Delete Owned Page %x \n",pPage));
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = vecHdrFtr.getNthItem(i);
		if(pHdrFtr->isPageHere(pPage))
		{
			pHdrFtr->deletePage(pPage);
			xxx_UT_DEBUGMSG(("Delete Owned Page %x from HdrFtr %x \n",pPage,pHdrFtr));
		}
	}
//
// Remove this page from the list of owned pages
//
	if(m_pFirstOwnedPage == pPage)
	{
		fp_Page * pNext = pPage->getNext();
		if(pNext && pNext->getOwningSection() == this)
		{
			m_pFirstOwnedPage = pNext;
		}
		else
		{
			m_pFirstOwnedPage = NULL;
		}
	}

	// Keep track of the number of pages in the section
	m_iPageCount--;

	fl_DocSectionLayout * pDSL = this;
	if(!getDocLayout()->isLayoutDeleting() && bReallyDeleteIt)
	{
		if(m_pLayout->findPage(pPage) > 0)
		{
			UT_DEBUGMSG(("fl_DocSec: deleting page %p ReallyDeleteIt %d \n",pPage,bReallyDeleteIt));
			m_pLayout->deletePage(pPage,true);
		}
		while(pDSL != NULL)
		{
			pDSL->checkAndRemovePages();
			pDSL->addValidPages();
			pDSL = pDSL->getNextDocSection();
		}
	}
}

/*
  This method returns true if the first column of the section starts on the 
  same page as the last column of the previous section and false otherwise. 
  If this is the first section, it tests whether the section starts on the first page.
 */

bool fl_DocSectionLayout::isFirstPageValid(void) const
{
	fp_Container * pFirstCon = getFirstContainer();
	fp_Page * pFirstPage = NULL;
	if (!pFirstCon)
	{
		return true;
	}
	pFirstPage = pFirstCon->getPage();
	UT_return_val_if_fail(pFirstPage,true);
	if (!getPrev())
	{
		// This is the first section of the document
		return (pFirstPage->getPageNumber() == 0)? true:false;
	}

	fp_Container * pPrevLastCon = getPrev()->getLastContainer();
	fp_Page * pPrevLastPage = NULL;
	UT_return_val_if_fail(pPrevLastCon,false);
	pPrevLastPage = pPrevLastCon->getPage();
	UT_return_val_if_fail(pPrevLastPage,false);

	return (pFirstPage == pPrevLastPage)? true:false;
}

/*!
 * This method returns true if the pPage pointer matches the header/footer type
 * given.
\param hfType The type of the header/Footer
\param fp_Page * pThisPage pointer to the page queried.
 */
bool fl_DocSectionLayout::isThisPageValid(HdrFtrType hfType, fp_Page * pThisPage)
{
	if (!m_pFirstOwnedPage)
		return false;

    // No header/footerness assigned yet. Page is invalid.
	if(hfType == FL_HDRFTR_NONE)
		return false;

	if(hfType == FL_HDRFTR_HEADER_FIRST ||
	   hfType == FL_HDRFTR_FOOTER_FIRST)
		return (pThisPage == m_pFirstOwnedPage);

//
// If there is a header page defined and this is a header page bail now!
//
	if ((pThisPage == m_pFirstOwnedPage) &&
		 ((m_pHeaderFirstSL && hfType < FL_HDRFTR_FOOTER) ||
		  (m_pFooterFirstSL && hfType >= FL_HDRFTR_FOOTER)))
		return false;

	fp_Page * pPage = m_pFirstOwnedPage;
	fp_Page * pNext = pPage->getNext();
	while(pNext && (pNext->getOwningSection() == this))
	{
		pPage = pNext;
		pNext = pNext->getNext();
	}

	if(hfType == FL_HDRFTR_HEADER_LAST ||
	   hfType == FL_HDRFTR_FOOTER_LAST)
	{
		return (pPage == pThisPage);
	}
//
// If there is a Last SL defined and this is the last page in the SLpage bail now!
//
	if ((pThisPage == pPage) &&
		 ((m_pHeaderLastSL && hfType < FL_HDRFTR_FOOTER) ||
		  (m_pFooterLastSL && hfType >= FL_HDRFTR_FOOTER)))
		return false;

	UT_sint32 i = 0;
	for(i=0; i < getDocLayout()->countPages(); i++)
	{
		if (getDocLayout()->getNthPage(i) == pThisPage)
			break;
	}

	UT_ASSERT(i < getDocLayout()->countPages());

	if((hfType == FL_HDRFTR_HEADER_EVEN) ||
	   (hfType == FL_HDRFTR_FOOTER_EVEN))
	{
		if(i % 2 == 0)
			return true;
		else
			return false;
	}
//
// If there is an Even SL defined and this is an even page in the SL page bail now!
//
	if ((i % 2 == 0) &&
		((m_pHeaderEvenSL && hfType < FL_HDRFTR_FOOTER) ||
		 (m_pFooterEvenSL && hfType >= FL_HDRFTR_FOOTER)))
		return false;

	return true; //if we're here all pages are valid.
}

void fl_DocSectionLayout::checkAndAdjustColumnGap(UT_sint32 iLayoutWidth)
{
	// Check to make sure column gap is not to wide to fit on the page with the
	// given number of columns.
	UT_sint32 minColumnWidth =0;
	UT_sint32 iColWidth= 0;
	if(m_iNumColumns > 1)
	{
		minColumnWidth = UT_convertToLogicalUnits("0.5in");	//TODO should this dimension be hard coded.
		iColWidth = (iLayoutWidth - static_cast<UT_sint32>(((m_iNumColumns - 1) * m_iColumnGap))) / static_cast<UT_sint32>(m_iNumColumns);

		if(iColWidth < minColumnWidth)
		{
			m_iColumnGap = (iLayoutWidth - minColumnWidth * static_cast<UT_sint32>(m_iNumColumns)) / (static_cast<UT_sint32>(m_iNumColumns) - 1);
		}

	}
	UT_ASSERT(m_iColumnGap < 2000000);
	if(m_iColumnGap < 30 || (m_iColumnGap > 200000))
	{
		m_iColumnGap = 30;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT _PageHdrFtrShadowPair
{
public:
	_PageHdrFtrShadowPair(void)
	   {
			m_pPage = NULL;
			m_pShadow = NULL;
		}
	virtual ~_PageHdrFtrShadowPair(void)
		{
			m_pPage = NULL;
			m_pShadow = NULL;
		}
	void setPage (fp_Page * pPage) { m_pPage = pPage;}
	void setShadow (fl_HdrFtrShadow * pShadow) { m_pShadow = pShadow;}
	fp_Page * getPage(void) const {return m_pPage;}
	fl_HdrFtrShadow * getShadow(void) const {return m_pShadow;}
private:
	fp_Page*			m_pPage;
	fl_HdrFtrShadow*	m_pShadow;
};


fl_HdrFtrSectionLayout::fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_HDRFTR,FL_CONTAINER_HDRFTR,PTX_SectionHdrFtr,pDocSL),
	  m_pDocSL(pDocSL),
	  m_iHFType(iHFType),
	  m_pHdrFtrContainer(NULL)
{
	fl_Layout::setType(PTX_SectionHdrFtr); // Set the type of this strux
	UT_DEBUGMSG(("SEVIOR: Creating HFType =%d \n",m_iHFType));
	xxx_UT_DEBUGMSG(("DocSectionLayout is %p \n",getDocSectionLayout()));
//
// Since we're almost certainly removing blocks at the end of the doc, tell the
// view to remember the current position on the active view.
//
// 	FV_View * pView = m_pLayout->getView();
// 	if(pView && pView->isActive())
// 	{
// 		pView->markSavedPositionAsNeeded();
// 	}
}

fl_HdrFtrSectionLayout::~fl_HdrFtrSectionLayout()
{
	xxx_UT_DEBUGMSG(("SEVIOR: Deleting HFType =%d \n",m_iHFType));
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = static_cast<_PageHdrFtrShadowPair*>(m_vecPages.getNthItem(i));

		delete pPair->getShadow();
	}
	_purgeLayout();
	DELETEP(m_pHdrFtrContainer);
//
// Take this section layout out of the linked list
//
	m_pLayout->removeHdrFtrSection(static_cast<fl_SectionLayout *>(this));
	m_pDocSL->removeFromUpdate(this);
//
// Null out pointer to this HdrFtrSection in the attached DocLayoutSection
//
	m_pDocSL->setHdrFtr(m_iHFType, NULL);
//
// Since we're almost certainly removing blocks at the end of the doc, tell the
// view to remember the current position on the active view.
//
// 	FV_View * pView = m_pLayout->getView();
// 	if(pView && pView->isActive())
// 	{
// 		pView->markSavedPositionAsNeeded();
// 	}
//
	UT_VECTOR_PURGEALL(_PageHdrFtrShadowPair*, m_vecPages);
}

/*!
 * This method removes all the lines and containers associated with the shadows
 * and the lines associated with this HdrFtrSectionLayout.
 *
 */
void fl_HdrFtrSectionLayout::collapse(void)
{
//
// If a view exists and we're editting a header footer take the pointer out of
// the header/footer. This will also clear the box around the header/footer
//
	FV_View * pView = m_pLayout->getView();
	if(pView && pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
		pView->rememberCurrentPosition();
	}

	_localCollapse();
	UT_uint32 iCount = m_vecPages.getItemCount();
	UT_uint32 i;
	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		fp_Page * ppPage = pPair->getPage();
		delete pPair->getShadow();
		ppPage->removeHdrFtr(getHFType());
		delete pPair;
	}
	m_vecPages.clear();
	DELETEP(m_pHdrFtrContainer);
}

/*!
 * This method removes the block pBlock from all the shadowLayouts.
 */
void fl_HdrFtrSectionLayout::collapseBlock(fl_ContainerLayout *pBlock)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	UT_uint32 i;
	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		fl_ContainerLayout * pShadowBL = pPair->getShadow()->findMatchingContainer(pBlock);
		UT_ASSERT(pShadowBL);
		UT_DEBUGMSG(("Doing collapseBlock %p \n",pBlock));
		if(pShadowBL)
		{
#ifdef ENABLE_SPELL
			// In case we've never checked this one
			if(pShadowBL->getContainerType() == FL_CONTAINER_BLOCK)
			{
				m_pLayout->dequeueBlockForBackgroundCheck(static_cast<fl_BlockLayout *>(pShadowBL));
			}
#endif
			pPair->getShadow()->remove( pShadowBL);
			delete pShadowBL;
			pPair->getShadow()->format();
		}
	}
}

bool fl_HdrFtrSectionLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;

	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		UT_continue_if_fail(pPair->getShadow());
		bResult = pPair->getShadow()->recalculateFields(iUpdateCount) || bResult;
	}
	return bResult;
}


fl_HdrFtrShadow * fl_HdrFtrSectionLayout::getFirstShadow(void)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	if(iCount != 0)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(0);
		return pPair->getShadow();
	}
	return NULL;
}

fp_Container* fl_HdrFtrSectionLayout::getFirstContainer() const
{
	return m_pHdrFtrContainer;
}


fp_Container* fl_HdrFtrSectionLayout::getLastContainer() const
{
	return m_pHdrFtrContainer;
}

fp_Container* fl_HdrFtrSectionLayout::getNewContainer(fp_Container * /*pFirstContainer*/)
{
	DELETEP(m_pHdrFtrContainer);
	UT_sint32 iWidth = m_pDocSL->getFirstContainer()->getPage()->getWidth(); // why is this different than the next one ?
	m_pHdrFtrContainer = static_cast<fp_Container *>(new fp_HdrFtrContainer(iWidth, static_cast<fl_SectionLayout *>(this)));
	return m_pHdrFtrContainer;
}

bool fl_HdrFtrSectionLayout::isPageHere(fp_Page * pPage)
{
	return (_findShadow(pPage) >=0 );
}

fl_HdrFtrShadow *  fl_HdrFtrSectionLayout::findShadow(fp_Page* pPage)
{
       UT_sint32 iPage = _findShadow(pPage);
       if(iPage < 0)
	        return NULL;
       _PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(iPage);
       return pPair->getShadow();
}

UT_sint32 fl_HdrFtrSectionLayout::_findShadow(fp_Page* pPage)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		if (pPair->getPage() == pPage)
		{
			return i;
		}
	}

	return -1;
}
/*!
 * This method converts a previously existing section to this header/footer.
 * Code liberally stolen from fl_DocSectionLayout::doclistener_deleteStrux
 \param fl_DocSectionLayout * pSL sectionlayout to be converted to a
 *     HdrFtrSectionLayout
*/
void fl_HdrFtrSectionLayout::changeIntoHdrFtrSection( fl_DocSectionLayout * pSL)
{
	UT_ASSERT(pSL->getPrevDocSection());
	// clear all the columns
	fp_Column* pCol =NULL;

	pCol = static_cast<fp_Column *>(pSL->getFirstContainer());
	while (pCol)
	{
		pCol->clearScreen();

		pCol = static_cast<fp_Column *>(pCol->getNext());
	}

	// remove all the columns from their pages
	pCol = static_cast<fp_Column *>(pSL->getFirstContainer());
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			pCol->getPage()->removeColumnLeader(pCol);
		}

		pCol = static_cast<fp_Column *>(pCol->getNext());
	}


	// get rid of all the layout information for every block
	fl_ContainerLayout*	pBL = pSL->getFirstLayout();
	while (pBL)
	{
		pBL->collapse();

		pBL = pBL->getNext();
	}

	//
	// Change the section type
	//

	// transfer the Sections' blocks into this header/footer

	while (pSL->getFirstLayout())
	{
		pBL = pSL->getFirstLayout();
		pSL->remove(pBL);
		add(pBL);
		static_cast<fl_BlockLayout *>(pBL)->setSectionLayout(this);
		static_cast<fl_BlockLayout *>(pBL)->setHdrFtr();
	}
	//
	// Remove old section from the section linked list!!
	//
	m_pLayout->removeSection(pSL);
//
	DELETEP(pSL); // Old Section layout is totally gone
	//
	// Create and Format the shadows
	//
	format();

	// Finished! we now have a header/footer
}

/*!
 * Remove the strux identifing this as a seperate section has been deleted so
 * we have to remove this HdrFtrSectionLayout class and all the shadow sections
 * attached to it. The blocks in this class are moved to the DocSectionLayout
 * associated with this class.
 * I do this because I expect that this will be called as part
 * on an undo "Insert Header" command. The rest of the undo needs blocks to
 * delete so I'm putting them there to keep the rest of the undo code happy
\param pcr the changerecord identifying this action as necesary.
\returns true
*/
bool fl_HdrFtrSectionLayout::doclistener_deleteStrux(const PX_ChangeRecord * pcr)
{
	UT_ASSERT(pcr->getType()==PX_ChangeRecord::PXT_DeleteStrux ||
			  pcr->getType()==PX_ChangeRecord::PXT_ChangeStrux);
	

	if(pcr->getType()==PX_ChangeRecord::PXT_ChangeStrux)
	{
		PX_ChangeRecord_StruxChange * pcrxc = (PX_ChangeRecord_StruxChange *) pcr;
		UT_UNUSED(pcrxc);
		UT_ASSERT_HARMLESS( pcrxc->isRevisionDelete() );
	}
	else
	{
		PX_ChangeRecord_Strux * pcrx = (PX_ChangeRecord_Strux *) pcr;
		UT_UNUSED(pcrx);
		UT_ASSERT(pcrx->getStruxType()==PTX_SectionHdrFtr);
	}
	
//
// Get last doc section. Move all the blocks from here to there after deleting
// this strux.
//
	fl_DocSectionLayout* pPrevSL = m_pDocSL;
	if (!pPrevSL)
	{
		UT_DEBUGMSG(("no prior SectionLayout"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
//
// Get rid of all the shadows, all the containers, and all the layout
// information for all the blocks.
//
	collapse();
//
// Now copy these line-less blocks into the previous docSectionLayout.
// Note: I expect that these blocks will be deleted by a later delete strux
// on these blocks.
//
	fl_ContainerLayout * pBL = NULL;
	while (getFirstLayout())
	{
		pBL = getFirstLayout();
		remove(pBL);
		pPrevSL->add(pBL);
	}
//
// Remove the pointer to this hdrftr
//
//
// Null out pointer to this HdrFtrSection in the attached DocLayoutSection
// This prevent a new page being created in the format statement that follows.
//
	m_pDocSL->setHdrFtr(m_iHFType, NULL);
//
// Format the new section containing the blocks.
//
	pPrevSL->format();
//
// Finally delete this HdrFtrSectionLayout. This could be done the docListener
// class but here I'm following the convention for the DocSectionLayout. It
// works there so I hope it works here. The HdrFtrSection destructor takes care
// of the details of unlinking the section etc.
//
	delete this;
	return true;
}

void fl_HdrFtrSectionLayout::addPage(fp_Page* pPage)
{
//
//  Sevior:
//  This triggers if we're rebuilding a section before page is defined like in a section change
//  strux. Reinstate if needed to find other bugs.
//	UT_ASSERT(0 > _findShadow(pPage));
//
//
// This might actually be called before we have any content to put in it. If so
// return and hope we get called later.
//
	if(getFirstLayout() == NULL)
	{
		UT_DEBUGMSG(("HdrFtr BUG. No content but we're asking to create a shadow !! \n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	if(_findShadow(pPage) > -1)
		return;
//
// Check this page is valid for this type of hdrftr
//
	if(!getDocSectionLayout()->isThisPageValid(m_iHFType, pPage))
	{
		return;
	}
	//
	// see if this page has a shadow attached already. This can happen
    // is a page goes from being odd to even.
	//
	fp_ShadowContainer* pOldShadow = pPage->getHdrFtrP(m_iHFType);
	//
	// If so remove it.
	//
	if(pOldShadow != NULL)
	{
		pOldShadow->getHdrFtrSectionLayout()->deletePage(pPage);
		pPage->removeHdrFtr(m_iHFType);
	}

	_PageHdrFtrShadowPair* pPair = new _PageHdrFtrShadowPair();
	UT_return_if_fail( pPair );
	
	// TODO outofmem
	xxx_UT_DEBUGMSG(("SEVIOR: Add page %x to pair %x \n",pPage,pPair));
	pPair->setPage(pPage);
	pPair->setShadow(new fl_HdrFtrShadow(m_pLayout, pPage, this, getStruxDocHandle(), m_apIndex));
	//
	// Make sure we register the shadow before populating it.
	//
	m_vecPages.addItem(pPair);
	//
	// Populate the shadow
	//
	fl_ShadowListener* pShadowListener = new fl_ShadowListener(this, pPair->getShadow());
	UT_return_if_fail( pShadowListener );
	if(m_iHFType < FL_HDRFTR_FOOTER) {
	  UT_DEBUGMSG(("shadow listener %p created For Header \n",pShadowListener));
	}
	else {
	  UT_DEBUGMSG(("shadow listener %p created For Footer \n",pShadowListener));
	}

//
// Populate with just this section so find the start and end of it
//
	PT_DocPosition posStart,posEnd,posDocEnd;
	m_pDoc->getBounds(true,posDocEnd);
	posStart = getFirstLayout()->getPosition(true) - 1;
	pf_Frag_Strux* sdStart = getFirstLayout()->getStruxDocHandle();
	pf_Frag_Strux* sdEnd = NULL;
	m_pDoc->getNextStruxOfType(sdStart,PTX_SectionHdrFtr,&sdEnd);
	if(sdEnd)
	{
	    posEnd = m_pDoc->getStruxPosition(sdEnd);
	}
	else
	{
	    posEnd = posDocEnd;
	}
	UT_ASSERT(posEnd > posStart);
	PD_DocumentRange * docRange = new PD_DocumentRange(m_pDoc,posStart,posEnd);
	m_pDoc->tellListenerSubset(pShadowListener, docRange);
	delete docRange;
	delete pShadowListener;
	markAllRunsDirty();
}

bool fl_HdrFtrSectionLayout::isPointInHere(PT_DocPosition pos)
{
//
// Skip through the Containers in this shadow to find the one containing this
// point.
//
    fl_ContainerLayout*	pBL = getFirstLayout();
	if(pBL == NULL)
		return false;
	if(pos < pBL->getPosition())
	{
//
// This corner case is that pos == position of the HdrFtr strux
//
		if(pos == (pBL->getPosition() - 1))
		{
			return true;
		}
		return false;
	}
//
// OK see if the next hdrftr is ahead of the pos
//
	fl_HdrFtrSectionLayout * pHF = static_cast<fl_HdrFtrSectionLayout *>(getNext());
	if(pHF == NULL)
	{
		PT_DocPosition posEOD;
		m_pDoc->getBounds(true,posEOD);
		if(pos <= posEOD)
		{
			return true;
		}
		// This happens when you're erasing the last character in the document.
		// Not sure if assert should stay or not.
		return false;
	}
	fl_ContainerLayout * ppBL = pHF->getFirstLayout();
	if(ppBL != NULL)
	{
		if(pos < (ppBL->getPosition()-1))
		{
			return true;
		}
		return false;
	}

	fl_ContainerLayout* pNext = pBL->getNext();
	while(pNext != NULL && pNext->getPosition( true) < pos)
	{
		pBL = pNext;
		pNext = pNext->getNext();
	}
	if(pNext != NULL)
	{
		return true;
	}
	else if(pBL && pBL->getPosition() == pos)
	{
		return true;
	}
//
// Now the point MIGHT be in this last block. Use code from pd_Document
// to find out. Have to check whether we're out of docrange first
//
	pf_Frag_Strux* sdh=NULL;
	bool bres;
	bres = m_pDoc->getStruxOfTypeFromPosition(pos, PTX_Block, &sdh);
	if(bres && sdh == pBL->getStruxDocHandle())
	{
		return true;
	}
	return false;
}

/*!
 * Removes the shadow and the corresponding element pointing to the shadow for this
 * Page.
 */
void fl_HdrFtrSectionLayout::deletePage(fp_Page* pPage)
{
	UT_sint32 iShadow = _findShadow(pPage);
//
// This shadow might have already been deleted via the collapse method
//
	if(iShadow <  0)
		return;
	_PageHdrFtrShadowPair* pPair = static_cast<_PageHdrFtrShadowPair*>(m_vecPages.getNthItem(iShadow));
	UT_return_if_fail(pPair);

	UT_ASSERT(pPair->getShadow());


	fp_Page * ppPage = pPair->getPage();
	UT_ASSERT(pPage == ppPage);
	delete pPair->getShadow();
	xxx_UT_DEBUGMSG(("Doing deletePage %x \n",pPage));
	if(getDocLayout()->findPage(ppPage) >= 0)
	{
			ppPage->removeHdrFtr(getHFType());
	}
	delete pPair;
	m_vecPages.deleteNthItem(iShadow);
}


/*!
 *  Just format the HdrFtrSectionLayout blocks for an insertBlock method.
 *  these blocks will be collapsed afterwards.
 */
void fl_HdrFtrSectionLayout::localFormat(void)
{
	xxx_UT_DEBUGMSG(("Doing a Local Format of the hdrftr section \n"));
	if(!getDocSectionLayout())
		return;
	fl_ContainerLayout*	pBL = getFirstLayout();
	UT_ASSERT(pBL->getContainerType() != FL_CONTAINER_HDRFTR);
	while (pBL)
	{
		if(pBL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			static_cast<fl_BlockLayout *>(pBL)->setHdrFtr();
		}
		pBL->format();
		pBL = pBL->getNext();
	}
}

/*!
 *  Just collapse the HdrFtrSectionLayout blocks for an insertBlock method.
 *  This removes all lines and references to containers but leaves the blocks
 *  and runs intact.
 */
void fl_HdrFtrSectionLayout::_localCollapse(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->collapse();
		pBL = pBL->getNext();
	}
}

/*!
 * This routine returns the matching block within this HdrFtrSectionLayout of the
 * shadow.
 \param fl_ContainerLayout * Pointer to block in shadow
 \returns the pinter to the matching block in the HdrFtr
 */
fl_ContainerLayout* fl_HdrFtrSectionLayout::findMatchingContainer(fl_ContainerLayout* pBL)
{
	// This routine returns the matching block within the
	// hdrftrSectionlayout.
	//
	fl_ContainerLayout* ppBL = getFirstLayout();
	bool bInTable = false;
	while(ppBL && (ppBL->getStruxDocHandle() != pBL->getStruxDocHandle()))
	{
		if(ppBL && (ppBL->getContainerType() == FL_CONTAINER_TABLE))
		{
			ppBL = ppBL->getFirstLayout();
			bInTable = true;
		}
		else if(bInTable && ppBL->getContainerType() == FL_CONTAINER_CELL)
		{
			ppBL = ppBL->getFirstLayout();
		}
		else if(bInTable && (ppBL->getNext() == NULL))
		{
			if(ppBL->myContainingLayout()->getNext() == NULL)
			{
				ppBL = ppBL->myContainingLayout();
				ppBL = ppBL->myContainingLayout()->getNext();
				bInTable = false;
			}
			else
			{
				ppBL = ppBL->myContainingLayout()->getNext();
			}
		}
		else
		{
			ppBL = ppBL->getNext();
		}
	}
	UT_ASSERT(ppBL);
	//xxx_UT_DEBUGMSG(("This header/footer is %x in findmatchingBlock \n",this));
	return ppBL;
}

/*!
 * This method checks that the pages in this header are valid and removes them if
 * they're not.
 */
void fl_HdrFtrSectionLayout::checkAndRemovePages(void)
{
	UT_sint32 iCount = m_vecPages.getItemCount();
//
// Check that the pages we have are still valid. Delete them if they're not.
//
	UT_sint32 i = 0;
	UT_GenericVector<fp_Page*> pageForDelete;
	for(i =0; i< iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		UT_continue_if_fail(pPair);
		UT_ASSERT(pPair->getShadow());

		fp_Page * ppPage = pPair->getPage();
		if(getDocLayout()->findPage(ppPage) >= 0)
		{
			if(!getDocSectionLayout()->isThisPageValid(getHFType(),ppPage))
			{
				pageForDelete.addItem(ppPage);
			}
		}
		else
		{
			pageForDelete.addItem(ppPage);
		}
	}
	for(i=0; i< pageForDelete.getItemCount(); i++)
	{
		fp_Page * pPage = pageForDelete.getNthItem(i);
		deletePage(pPage);
	}
	if( pageForDelete.getItemCount() > 0)
	{
		markAllRunsDirty();
	}
}

/*!
 * This method adds valid pages to the collection of shadows.
 */
void fl_HdrFtrSectionLayout::addValidPages(void)
{
	//
	// Check that all the pages this header/footer should have are
    // in place.
	// We have to extract this information from m_pDocSL
	// Loop through all the columns in m_pDocSl and find the pages owned
	// by m_pDocSL
	//
        fp_Column * pCol = NULL;
	pCol = static_cast<fp_Column *>(m_pDocSL->getFirstContainer());
	fp_Page * pOldPage = NULL;
	fp_Page * pNewPage = NULL;
	while(pCol)
	{
		pNewPage = pCol->getPage();
		if((pNewPage != NULL) && (pNewPage != pOldPage) && (getDocLayout()->findPage(pNewPage) >=0))
		{
			fl_DocSectionLayout* pDocSec = pNewPage->getOwningSection();
			if(pDocSec == m_pDocSL && _findShadow(pNewPage) < 0)
			{
//
// The addPage Method checks that only valid pages are added to this HdrFtr based on
// the HFType
//
				addPage(pNewPage);
			}
		}
		pCol = static_cast<fp_Column *>(pCol->getNext());
	}
}

/*!
 * Format the overall HdrFtrSectionLayout in it's virtual container.
 * Also check that all the correct pages have been found for this HdrFtr. Then
 * format the Shadows.
 */
void fl_HdrFtrSectionLayout::format(void)
{
	if(getFirstLayout() == NULL)
	{
		return;
	}
	UT_sint32 iCount =0;
	UT_sint32 i = 0;
	localFormat();
	//
	// Fail safe code to add pages if we don't have them.
	//
	addValidPages();

	iCount = m_vecPages.getItemCount();

	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		pPair->getShadow()->format();
	}
	layout();
}

void fl_HdrFtrSectionLayout::updateLayout(bool /*bDoFull*/)
{
	bool bredraw = false;
	fl_ContainerLayout*	pBL = getFirstLayout();
	if(needsReformat())
	{
		format();
		bredraw = true;
		m_bNeedsReformat = false;
	}
	m_vecFormatLayout.clear();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			bredraw = true;
			pBL->format();
		}

		pBL = pBL->getNext();
	}
	if(bredraw == true)
	{
		if(m_pHdrFtrContainer)
			static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
 	}
	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	if(bredraw)
	{
	      for (UT_uint32 i=0; i<iCount; i++)
	      {
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		pPair->getShadow()->updateLayout(false);
	      }
	}
}

/*!
 * Mark all runs and lines in the all shadows for redraw.
 */
void fl_HdrFtrSectionLayout::markAllRunsDirty(void)
{
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		pPair->getShadow()->markAllRunsDirty();
	}
}

/*!
 * Layout the overall HdrFtr and everything underneath it.
 */
void fl_HdrFtrSectionLayout::layout(void)
{
    if(m_pHdrFtrContainer)
	static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
	//
	// update the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		pPair->getShadow()->layout();
	}
}

void fl_HdrFtrSectionLayout::clearScreen(void)
{
	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		pPair->getShadow()->clearScreen();
	}
}

void fl_HdrFtrSectionLayout::redrawUpdate(void)
{
//
// Do another layout but don't redraw.
//
	if(m_pHdrFtrContainer)
		static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
	//
	// Don't need to draw here since this is never displayed on the screen?
	//
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		if(m_pLayout->findPage(pPair->getPage()) >= 0)
		{
			pPair->getShadow()->redrawUpdate();
		}
	}

}

bool fl_HdrFtrSectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	setAttrPropIndex(pcrxc->getIndexAP());

	// TODO what happens here?

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_HdrFtrSectionLayout::_lookupProperties(const PP_AttrProp* /*pAP*/)
{
}

void fl_HdrFtrSectionLayout::_lookupMarginProperties(const PP_AttrProp* /*pAP*/)
{
	fl_ContainerLayout * pShadow = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();

	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadow = pPair->getShadow();
		if(pShadow)
		{
			pShadow->lookupMarginProperties();
		}
	}
}

bool fl_HdrFtrSectionLayout::bl_doclistener_populateSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
//
// We need to populate block in the header/footer but to do that we need the
// header/footer to be fomatted. So do it then unformat after.
//
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();

	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_populateSpan(pcrs,blockOffset,len)
				&& bResult;
		}
		else
		{
			UT_ASSERT(0);
			break;
		}
	}
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
	  bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_populateSpan(pcrs,blockOffset,len)
	    && bResult;
	}
	return bResult;
}

/*!
 * Now for all these methods which manipulate the shadow sections, turn off
 * Insertion Point changes while the shadows are manipulated.
 * Re Enabled insertion point changes for the overall hdrftrsection so it
 * is changed just once.
 */

bool fl_HdrFtrSectionLayout::bl_doclistener_populateObject(fl_ContainerLayout* pBL, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
//
// We need to populate block in the header/footer but to do that we need the
// header/footer to be fomatted. So do it then unformat after.
//
  	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_populateObject(blockOffset,pcro)
				&& bResult;
		}
		else
		{
			bResult = false;
		}
	}
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_populateObject(blockOffset,pcro)
			&& bResult;
	}
	else
	{
		bResult = false;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_insertSpan(pcrs)
				&& bResult;
		}
	}
	m_pDoc->allowChangeInsPoint();
	// Update the overall block too.
	fl_BlockLayout * ppBL = static_cast<fl_BlockLayout *>(findMatchingContainer(pBL));
	if(ppBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertSpan(pcrs)
	&& bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
		  bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_deleteSpan(pcrs)
		    && bResult;
		}
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
        {
	  bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteSpan(pcrs)
	    && bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeSpan(fl_ContainerLayout* pBL, const PX_ChangeRecord_SpanChange * pcrsc)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
		  bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_changeSpan(pcrsc) && bResult;
		}
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
	  bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeSpan(pcrsc)
		&& bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_deleteStrux(pcrx)
				&& bResult;
		}
		else
		{
			UT_ASSERT(0);
		}
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteStrux(pcrx) && bResult;
	}
	return bResult;
}

/*!
 * Delete Just the cell struxes from the HdrFtr shadows
 */
bool fl_HdrFtrSectionLayout::bl_doclistener_deleteCellStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
	bool bResult = true;
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_CELL);
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	if(iCount <=0)
	{
		UT_ASSERT(0);
	}
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			UT_ASSERT(pShadowBL->getContainerType() == FL_CONTAINER_CELL);
			bResult = static_cast<fl_CellLayout *>(pShadowBL)->doclistener_deleteStrux(pcrx)
				&& bResult;
		}
		else
		{
			UT_ASSERT(0);
		}
	}
	return bResult;
}


/*!
 * Delete Just the table struxes from the HdrFtr shadows
 */
bool fl_HdrFtrSectionLayout::bl_doclistener_deleteTableStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
	bool bResult = true;
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_TABLE);
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	if(iCount <=0)
	{
		UT_ASSERT(0);
	}
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			UT_ASSERT(pShadowBL->getContainerType() == FL_CONTAINER_TABLE);
			bResult = static_cast<fl_TableLayout *>(pShadowBL)->doclistener_deleteStrux(pcrx)
				&& bResult;
		}
		else
		{
			UT_ASSERT(0);
		}
	}
	return bResult;
}


bool fl_HdrFtrSectionLayout::bl_doclistener_changeFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMarkChange * pcrfmc)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
		  bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_changeFmtMark(pcrfmc)
		    && bResult;
		}
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
	  bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeFmtMark(pcrfmc) && bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeStrux(fl_ContainerLayout* pBL, const PX_ChangeRecord_StruxChange * pcrxc)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			if(pShadowBL->getContainerType() == FL_CONTAINER_BLOCK)
			{
				bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_changeStrux(pcrxc)
					&& bResult;
			}
			else if(pShadowBL->getContainerType() == FL_CONTAINER_TABLE)
			{
				bResult = static_cast<fl_TableLayout *>(pShadowBL)->doclistener_changeStrux(pcrxc)
				&& bResult;
			}
			else if(pShadowBL->getContainerType() == FL_CONTAINER_CELL)
			{
				bResult = static_cast<fl_CellLayout *>(pShadowBL)->doclistener_changeStrux(pcrxc)
				&& bResult;
			}
		}
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL && (pBL->getContainerType() == FL_CONTAINER_BLOCK))
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeStrux(pcrxc)
			&& bResult;
	}
	return bResult;
}

/*!
 * Insert a cell into every table in the HdrFtr
 */
bool fl_HdrFtrSectionLayout::bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  fl_TableLayout * pTL)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_ContainerLayout * pShadowBL = NULL;
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertCells into shadows \n"));
	m_pDoc->setDontChangeInsPoint();
	bool bResult = true;
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching Table in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pTL);
		fl_ContainerLayout * pPrevCell = NULL;
		if(pCell)
		{
			pPrevCell = pPair->getShadow()->findMatchingContainer(pCell);
		}
		UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertCell into Table shadow \n"));
		if(pShadowBL)
		{
			bResult = static_cast<fl_TableLayout *>(pShadowBL)->bl_doclistener_insertCell(pPrevCell,pcrx,sdh,lid,NULL)
				&& bResult;
		}
	}
	m_pDoc->allowChangeInsPoint();
	return true;

}


/*!
 * Insert an endTable cell into every table in the HdrFtr
 */
bool fl_HdrFtrSectionLayout::bl_doclistener_insertEndTable(fl_ContainerLayout* pTab,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_ContainerLayout * pShadowBL = NULL;
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertEndTables into shadows \n"));
	m_pDoc->setDontChangeInsPoint();
	bool bResult = true;
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching Table in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pTab);
		UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertEndTable into shadow \n"));
		if(pShadowBL)
		{
			bResult = static_cast<fl_TableLayout *>(pShadowBL)->bl_doclistener_insertEndTable(NULL,pcrx,sdh,lid,NULL)
				&& bResult;
		}
	}
	m_pDoc->allowChangeInsPoint();
	return true;

}

fl_SectionLayout * fl_HdrFtrSectionLayout::bl_doclistener_insertTable(fl_ContainerLayout* pBL,
													SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{
	fl_SectionLayout * pSL = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertTable(pcrx, iType, sdh, lid, pfnBindHandles);
//	UT_ASSERT(0);
	fl_SectionLayout::checkAndAdjustCellSize();
//
// FIXME: Propagate this to the shadows
//      : Write code to handle populate cases in the shadows
	bool bResult = true;
//
// Now insert it into all the shadows.
//
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_ContainerLayout * pShadowBL = NULL;
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertTable \n"));
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.
		if(pBL)
		{
			pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
			UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertTable into shadow 1 \n"));
			if(pShadowBL)
			{
				bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_insertTable(pcrx,iType,sdh,lid,NULL)
					&& bResult;
			}
			pPair->getShadow()->checkAndAdjustCellSize();
		}
		else
//
// FIXME: Handle case of inserting a table at the start of the HdrFtr
//
		{
			UT_ASSERT(0);
		}
	}
	m_pDoc->allowChangeInsPoint();
	return pSL;
}

/*!
 * Insert a Table at the start of a HdrFtr
 */
fl_SectionLayout * fl_HdrFtrSectionLayout::bl_doclistener_insertTable(SectionType /*iType*/,
													const PX_ChangeRecord_Strux * pcrx,
													pf_Frag_Strux* sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																			PL_ListenerId lid,
																			fl_ContainerLayout* sfhNew))
{

	fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(this)->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_TABLE));

	fl_ContainerLayout* sfhNew = pSL;
	//
	// Don't bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}
//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);

	fl_SectionLayout::checkAndAdjustCellSize();
//
// Now insert it into all the shadows.
//
	UT_uint32 iCount = m_vecPages.getItemCount();
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertTable At start \n"));
	m_pDoc->setDontChangeInsPoint();
	fl_HdrFtrShadow * pShadowL = NULL;
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		pShadowL = pPair->getShadow();

		UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertTable into start of shadow 1 \n"));
		if(pShadowL)
		{
		     pShadowL->bl_doclistener_insertTable(FL_SECTION_TABLE, pcrx,sdh,lid,NULL);
		     pShadowL->checkAndAdjustCellSize();
		}
	}
	m_pDoc->allowChangeInsPoint();
	return pSL;
}

/*!
 * Insert the first block of the container in HdrFtr. We need to propage this
 * to all the shadows.
 */
bool fl_HdrFtrSectionLayout::bl_doclistener_insertFirstBlock(fl_ContainerLayout* pCL, const PX_ChangeRecord_Strux * pcrx,pf_Frag_Strux* sdh,PL_ListenerId lid)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_ContainerLayout * pShadowBL = NULL;
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insert First block into shadow Cells \n"));
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching Table in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingContainer(pCL);
		UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertFirstBlock into (CELL) shadow 1 \n"));
		if(pShadowBL)
		{
			fl_BlockLayout*	pNewBL = static_cast<fl_BlockLayout *>(pShadowBL->insert(sdh, NULL, pcrx->getIndexAP(),FL_CONTAINER_BLOCK));
			pNewBL->doclistener_insertFirstBlock(pcrx, sdh,	lid, NULL);
		}
	}
	m_pDoc->allowChangeInsPoint();
	return true;

}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertBlock(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx,pf_Frag_Strux* sdh,PL_ListenerId lid,void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,	PL_ListenerId lid, fl_ContainerLayout* sfhNew))
{
	bool bResult = true;
//
// Now insert it into all the shadows.
//
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_ContainerLayout * pShadowBL = NULL;
	UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertBlock \n"));
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		// Find matching block in this shadow.
		if(pBL)
		{
			pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
			xxx_UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertBlock into shadow 1 \n"));
			if(pShadowBL)
			{
				bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_insertBlock(pcrx,sdh,lid,NULL)
					&& bResult;
			}
		}
		else
//
// This is the first block in the shadow
//
		{
			fl_ContainerLayout*	pNewBL = pPair->getShadow()->insert(sdh, NULL, pcrx->getIndexAP(),FL_CONTAINER_BLOCK);
			if (!pNewBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout\n"));
				return false;
			}
			xxx_UT_DEBUGMSG(("fl_HdrFtrSectionLayout: insertBlock into shadow 2 \n"));
			bResult = bResult && static_cast<fl_BlockLayout *>(pNewBL)->doclistener_insertFirstBlock(pcrx, sdh,
													lid, NULL);
		}
	}
//
// Find Matching Block in this HdrFtrSectionLayout!!
//
	m_pDoc->allowChangeInsPoint();
	if(pBL)
	{
		fl_ContainerLayout * ppBL = findMatchingContainer(pBL);
		if(ppBL)
		{
		  static_cast<fl_BlockLayout *>(ppBL)->setHdrFtr();
		  bResult = static_cast<fl_BlockLayout *>(ppBL)->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles)
			&& bResult;
//
// Mark the Block as HdrFtr
//
		  static_cast<fl_BlockLayout *>(ppBL->getNext())->setHdrFtr();
		}
		setNeedsReformat(this);
	}
	else
//
// First block in the section
//
	{
		fl_ContainerLayout*	pNewBL = insert(sdh, NULL, pcrx->getIndexAP(),FL_CONTAINER_BLOCK);
		if (!pNewBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout\n"));
			return false;
		}
		bResult = bResult && static_cast<fl_BlockLayout *>(pNewBL)->doclistener_insertFirstBlock(pcrx, sdh,
													lid, pfnBindHandles);
		static_cast<fl_BlockLayout *>(pNewBL)->setHdrFtr();
		setNeedsReformat(this);
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertSection(fl_ContainerLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
														  pf_Frag_Strux* sdh,
														  PL_ListenerId lid,
														  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																				  PL_ListenerId lid,
																				  fl_ContainerLayout* sfhNew))
{
	// TODO this should NEVER happen, right?
	UT_DEBUGMSG(("Insert Section is header/footer!!! \n"));
	UT_ASSERT(0);
	bool bResult = true;
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);

		bResult = pPair->getShadow()->bl_doclistener_insertSection(pBL, FL_SECTION_DOC, pcrx, sdh, lid, pfnBindHandles)
			&& bResult;
	}

	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_insertObject(pcro)
				&& bResult;
		}
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertObject(pcro) && bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_deleteObject(pcro)
				&& bResult;
		}
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteObject(pcro) && bResult;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeObject(fl_ContainerLayout* pBL, const PX_ChangeRecord_ObjectChange * pcroc)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_changeObject(pcroc)
				&& bResult;
		}
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_changeObject(pcroc) && bResult;
	}

	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_insertFmtMark(pcrfm)
				&& bResult;
		}
		else
		{
			bResult = false;
		}
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_insertFmtMark(pcrfm) && bResult;
	}
	else
	{
		bResult = false;
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteFmtMark(fl_ContainerLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	bool bResult = true;
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pBL);
		if(pShadowBL)
		{
			bResult = static_cast<fl_BlockLayout *>(pShadowBL)->doclistener_deleteFmtMark(pcrfm)
				&& bResult;
		}
		else
		{
			bResult = false;
		}
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingContainer(pBL);
	if(pBL)
	{
		bResult = static_cast<fl_BlockLayout *>(pBL)->doclistener_deleteFmtMark(pcrfm)	&& bResult;
	}
	else
	{
		bResult = false;
	}
	return bResult;
}

void fl_HdrFtrSectionLayout::checkAndAdjustCellSize(fl_ContainerLayout * pCL)
{
	if(pCL->getContainerType() != FL_CONTAINER_CELL)
	{
		return;
	}
	fl_ContainerLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingContainer(pCL);
		if(pShadowBL)
		{
			static_cast<fl_SectionLayout *>(pShadowBL)->checkAndAdjustCellSize();
		}
	}
	// Update the overall block too.

	fl_ContainerLayout * pBL = findMatchingContainer(pCL);
	if(pBL)
	{
		static_cast<fl_CellLayout *>(pBL)->checkAndAdjustCellSize();
	}
	return;
}

////////////////////////////////////////////////////////////////////////////

bool fl_DocSectionLayout::bl_doclistener_insertFootnote(fl_ContainerLayout* pFootnote,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	fl_DocSectionLayout * pCol = static_cast<fl_DocSectionLayout *>(myContainingLayout());
	pNewCL = pCol->insert(sdh,pFootnote,pcrx->getIndexAP(), FL_CONTAINER_FOOTNOTE);
	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	fl_ContainerLayout* sfhNew = pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);


//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	return true;
}


bool fl_DocSectionLayout::bl_doclistener_insertAnnotation(fl_ContainerLayout* pFootnote,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	fl_DocSectionLayout * pCol = static_cast<fl_DocSectionLayout *>(myContainingLayout());
	pNewCL = pCol->insert(sdh,pFootnote,pcrx->getIndexAP(), FL_CONTAINER_ANNOTATION);
	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	fl_ContainerLayout* sfhNew = pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);


//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_HdrFtrShadow::fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pHdrFtrSL, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_SHADOW,FL_CONTAINER_SHADOW,PTX_Section,pHdrFtrSL->getDocSectionLayout()),
	  m_pPage(pPage),
	  m_pHdrFtrSL(pHdrFtrSL)
{
	// Force creation of the appropriate container object;
	// throw away return value
        xxx_UT_DEBUGMSG(("In Shadow Constructor %p DocSectionLayout %p \n",this,getDocSectionLayout()));
	m_pPage->getHdrFtrContainer(m_pHdrFtrSL);
	xxx_UT_DEBUGMSG(("check that m_iType is indeed FL_SECTION_SHADOW \n"));
	UT_ASSERT(m_iType == FL_SECTION_SHADOW);
//	UT_ASSERT(0);
	fl_Layout::setType(PTX_Section); // Set the type of this strux
}

fl_HdrFtrShadow::~fl_HdrFtrShadow()
{
	_purgeLayout();
}

fp_Container* fl_HdrFtrShadow::getFirstContainer() const
{
	return m_pPage->getHdrFtrContainer(m_pHdrFtrSL);
}

fp_Container* fl_HdrFtrShadow::getLastContainer() const
{
	return m_pPage->getHdrFtrContainer(m_pHdrFtrSL);
}

fp_Container* fl_HdrFtrShadow::getNewContainer(fp_Container * /*pFirstContainer*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return NULL;
}

fl_ContainerLayout* fl_HdrFtrShadow::findMatchingContainer(fl_ContainerLayout* pBL)
{
	// This routine returns the matching block within this shadow of the
	// hdrftrSectionlayout.
	//
	fl_ContainerLayout* ppBL = getFirstLayout();
	bool bInTable = false;
	while(ppBL && (ppBL->getStruxDocHandle() != pBL->getStruxDocHandle()))
	{
		if(ppBL->getContainerType() == FL_CONTAINER_TABLE)
		{
			ppBL = ppBL->getFirstLayout();
			bInTable = true;
		}
		else if(bInTable && ppBL->getContainerType() == FL_CONTAINER_CELL)
		{
			ppBL = ppBL->getFirstLayout();
		}
		else if(bInTable && (ppBL->getNext() == NULL))
		{
			if(ppBL->myContainingLayout()->getNext() == NULL)
			{
				ppBL = ppBL->myContainingLayout();
				ppBL = ppBL->myContainingLayout()->getNext();
				bInTable = false;
			}
			else
			{
				ppBL = ppBL->myContainingLayout()->getNext();
			}
		}
		else
		{
			ppBL = ppBL->getNext();
		}
	}
	if(ppBL == NULL)
	{
		//UT_ASSERT(0);
		m_pDoc->miniDump(pBL->getStruxDocHandle(),6);
		if(pBL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			ppBL = getFirstLayout();
			while(ppBL && ppBL->getStruxDocHandle() != pBL->getStruxDocHandle())
			{
				ppBL = ppBL->getNextBlockInDocument();
			}
		}
	}
	//UT_ASSERT(ppBL);
	xxx_UT_DEBUGMSG(("Search for block in shadow %x \n",this));
	return ppBL;
}

void fl_HdrFtrShadow::format(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if(pBL->getContainerType() == FL_CONTAINER_TABLE)
		{
			UT_DEBUGMSG(("!!!!Format Shadow Table!!!!!!!!!!\n"));
		}
		pBL->format();
		pBL = pBL->getNext();
	}
}

/*!
 * Scans through the shadow looking for the block at the specified Document
 * Position.
 \param pos the Document position
 \return A pointer to the block containing the point. Returns NULL if no block
         is found
 */
fl_ContainerLayout * fl_HdrFtrShadow::findBlockAtPosition(PT_DocPosition pos)
{
//
// Skip through the blocks in this shadow to find the one containing this
// point.
//
    fl_ContainerLayout*	pBL = getFirstBlock();
    if(pBL == NULL)
		return NULL;
	if(pos < pBL->getPosition(true))
	{
//
// This corner case is that pos == position of the HdrFtr strux
//
		if(pos == (pBL->getPosition(true) - 1))
		{
			if(pBL->getContainerType() != FL_CONTAINER_BLOCK)
			{
				pBL= pBL->getNextBlockInDocument();
			}
			return pBL;
		}
		return NULL;
	}
	fl_ContainerLayout* pNext = NULL;
	pNext = pBL->getNextBlockInDocument();
	bool doNext = false;
	if(pNext != NULL)
	{
		doNext = (pNext->getPosition(true) < pos);
	}
	while(doNext)
	{
		pBL = pNext;
		pNext = pNext->getNextBlockInDocument();
		if(pNext && (pNext->getPosition(true) < pos))
		{
			if(getNext())
			{
				if(getNext()->getPosition(true) > pNext->getPosition(true))
				{
					doNext = true;
				}
				else
				{
					doNext = false;
				}
			}
			else
			{
				doNext = true;
			}
		}
		else
		{
			doNext = false;
		}
	}

	if(pNext != NULL)
	{
		UT_ASSERT(pNext->getPosition(true) >= pos);
		if(pBL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return pBL;
		}
		else if(pNext->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return pNext;
		}
	}
	else if(pBL && pBL->getPosition() == pos)
	{
		return pBL;
	}
	else
	{
		return NULL;
	}
//
// Next corner case. See if position is inside the edittableBounds of this
// section
//
	PT_DocPosition posEnd;
	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->getEditableBounds(true,posEnd);
		if(pos <= posEnd)
			return pBL;
	}
//
// Now the point MIGHT be in this last block. Use code from pd_Document
// to find out. Have to check whether we're out of docrange first
//
	m_pDoc->getBounds(true,posEnd);
	if(pos > posEnd)
		return NULL;
	pf_Frag_Strux* sdh=NULL;
	bool bres;
	bres = m_pDoc->getStruxOfTypeFromPosition(pos, PTX_Block, &sdh);
	if(bres && sdh == pBL->getStruxDocHandle())
		return pBL;
//
// Not here!!
//
	return NULL;
}

void fl_HdrFtrShadow::updateLayout(bool /*bDoAll*/)
{
	bool bredraw = false;
	xxx_UT_DEBUGMSG(("Doing Update layout in shadow %x \n",this));
	fl_ContainerLayout*	pBL = getFirstLayout();
	m_vecFormatLayout.clear();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			bredraw = true;
			pBL->format();
		}

		pBL = pBL->getNext();
	}
	if(bredraw)
	{
		//    clearScreen();
		static_cast<fp_ShadowContainer *>(getFirstContainer())->layout();
 	}
}

void fl_HdrFtrShadow::layout(void)
{
	if(needsReformat())
	{
		format();
	}
	static_cast<fp_ShadowContainer *>(getFirstContainer())->layout();
}

void fl_HdrFtrShadow::clearScreen(void)
{
	UT_ASSERT(getFirstContainer());
	if(getFirstContainer())
		static_cast<fp_ShadowContainer *>(getFirstContainer())->clearScreen();
}

void fl_HdrFtrShadow::redrawUpdate(void)
{
	FV_View * pView = m_pLayout->getView();
	fl_ContainerLayout*	pBL = getFirstLayout();
	bool bDoLayout = false;
	while (pBL && (pView != NULL))
	{
		if(pBL->getContainerType() == FL_CONTAINER_BLOCK && static_cast<fl_BlockLayout *>(pBL)->hasUpdatableField())
		{
			bool bReformat = pBL->recalculateFields(getDocLayout()->getRedrawCount());
			if(bReformat)
			{
				pBL->format();
				bDoLayout = true;
			}
		}
		else
		{
			pBL->recalculateFields(getDocLayout()->getRedrawCount());
		}
		
		if(pView && pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}
		pBL = pBL->getNext();
	}
	if(bDoLayout)
	{
	       static_cast<fp_ShadowContainer *>(getFirstContainer())->layout();
	}
}
bool fl_HdrFtrShadow::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	setAttrPropIndex(pcrxc->getIndexAP());

	// TODO

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_HdrFtrShadow::_lookupProperties(const PP_AttrProp* /*pAP*/)
{
}

void fl_HdrFtrShadow::_lookupMarginProperties(const PP_AttrProp* /*pAP*/)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->lookupMarginProperties();
		pBL = pBL->getNext();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_ShadowListener::fl_ShadowListener(fl_HdrFtrSectionLayout* pHFSL, fl_HdrFtrShadow* pShadow): 
	m_pDoc(pHFSL->getDocLayout()->getDocument()),
	m_pShadow(pShadow),
	m_bListening(false),
	m_pCurrentBL(NULL),
	m_pHFSL(pHFSL),
	m_pCurrentTL(NULL),
	m_pCurrentCell(NULL)
{
}

fl_ShadowListener::~fl_ShadowListener()
{
}

bool fl_ShadowListener::populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr)
{
	if (!m_bListening)
	{
		return true;
	}

	UT_ASSERT(m_pShadow);
//	UT_DEBUGMSG(("fl_ShadowListener::populate shadow %x \n",m_pShadow));

	bool bResult = false;
	FV_View* pView = m_pHFSL->getDocLayout()->getView();
	PT_DocPosition oldPos = 0;
	//
	// We're not printing
	//
	if(pView != NULL)
	{
		oldPos = pView->getPoint();
	}
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		{
			const fl_Layout * pL = static_cast<const fl_Layout *>(sfh);
			UT_UNUSED(pL);
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();


		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateSpan(pcrs, blockOffset, len);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		{
			const fl_Layout * pL = static_cast<const fl_Layout *>(sfh);
			UT_UNUSED(pL);
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

// sterwill -- is this call to getSectionLayout() needed?  pBLSL is not used.

//			fl_SectionLayout* pBLSL = m_pCurrentBL->getSectionLayout();
		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateObject(blockOffset,pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		//	const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		{
			const fl_Layout * pL = static_cast<const fl_Layout *>(sfh);
			UT_UNUSED(pL);
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(pL)));
		}
		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_insertFmtMark( reinterpret_cast<const PX_ChangeRecord_FmtMark *>(pcr));
		goto finish_up;
	}

	default:
		UT_DEBUGMSG(("Unknown Change record = %d \n",pcr->getType()));
		UT_ASSERT(0);
		//
		// We're not printing
		//
		if(pView != NULL && m_pDoc->getAllowChangeInsPoint())
		{
			pView->setPoint(oldPos);
		}
		return false;
	}

 finish_up:
	//
	// We're not printing
	//
	if(pView != NULL && m_pDoc->getAllowChangeInsPoint())
	{
		pView->setPoint(oldPos);
	}
	return bResult;
}

bool fl_ShadowListener::populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh)
{
	UT_ASSERT(m_pShadow);
	xxx_UT_DEBUGMSG(("fl_ShadowListener::populateStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;

		// need to explode revision attribute
		m_pDoc->getAttrProp(indexAP, &pAP);
		if(pAP)
		{
			UT_return_val_if_fail(m_pHFSL && m_pHFSL->getDocLayout(), false);
			FV_View* pView = m_pHFSL->getDocLayout()->getView();
			UT_return_val_if_fail(pView, false);

			UT_uint32 iId  = pView->getRevisionLevel();
			bool bShow     = pView->isShowRevisions();

			PP_RevisionAttr * pRevisions = NULL; // must be NULL

			
			if(  pAP->getRevisedIndex() != 0xffffffff
			  && pAP->getRevisionState().isEqual(iId, bShow, m_pDoc->isMarkRevisions()))
			{
				// the revision has a valid index to an inflated AP, so we use it
				PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

				m_pDoc->getAttrProp(revAPI, &pAP);
			}
			else
			{
				bool bHiddenRevision;
				const PP_AttrProp * pNewAP = m_pDoc->explodeRevisions(pRevisions, pAP, bShow,
																	  iId, bHiddenRevision);

				if(pNewAP)
					pAP = pNewAP;
			}

			delete pRevisions;
		}
		
		if (pAP)
		{
			const gchar* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == strcmp(pszSectionType, "doc"))
				)
			{
				m_bListening = false;
			}
			else
			{
				if ( (0 == strcmp(pszSectionType, "header"))
					|| (0 == strcmp(pszSectionType, "footer"))
					 || (0 == strcmp(pszSectionType, "header-first"))
					|| (0 == strcmp(pszSectionType, "footer-first"))
					 || (0 == strcmp(pszSectionType, "header-even"))
					|| (0 == strcmp(pszSectionType, "footer-even"))
					 || (0 == strcmp(pszSectionType, "header-last"))
					|| (0 == strcmp(pszSectionType, "footer-last"))
					)
				{
					// TODO verify id match

					m_bListening = true;
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

	case PTX_SectionHdrFtr:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		// need to explode revision attribute
		m_pDoc->getAttrProp(indexAP, &pAP);
		if(pAP)
		{
			UT_return_val_if_fail(m_pHFSL && m_pHFSL->getDocLayout(), false);
			FV_View* pView = m_pHFSL->getDocLayout()->getView();
			UT_return_val_if_fail(pView, false);

			UT_uint32 iId  = pView->getRevisionLevel();
			bool bShow     = pView->isShowRevisions();

			PP_RevisionAttr * pRevisions = NULL; // must be NULL

			
			if(  pAP->getRevisedIndex() != 0xffffffff
			  && pAP->getRevisionState().isEqual(iId, bShow, m_pDoc->isMarkRevisions()))
			{
				// the revision has a valid index to an inflated AP, so we use it
				PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

				m_pDoc->getAttrProp(revAPI, &pAP);
			}
			else
			{
				bool bHiddenRevision;
				const PP_AttrProp * pNewAP = m_pDoc->explodeRevisions(pRevisions, pAP, bShow,
																	  iId, bHiddenRevision);

				if(pNewAP)
					pAP = pNewAP;
			}

			delete pRevisions;
		}
		
		if (pAP)
		{
			const gchar* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == strcmp(pszSectionType, "doc"))
				)
			{
				m_bListening = false;
			}
			else
			{
				if ( (0 == strcmp(pszSectionType, "header"))
					|| (0 == strcmp(pszSectionType, "footer"))
					 || (0 == strcmp(pszSectionType, "header-first"))
					|| (0 == strcmp(pszSectionType, "footer-first"))
					 || (0 == strcmp(pszSectionType, "header-even"))
					|| (0 == strcmp(pszSectionType, "footer-even"))
					 || (0 == strcmp(pszSectionType, "header-last"))
					|| (0 == strcmp(pszSectionType, "footer-last"))
					)
				{
					// TODO verify id match

					m_bListening = true;
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
		if (m_bListening)
		{
			// append a new BlockLayout to that SectionLayout
			fl_ContainerLayout*	pBL = NULL;
			if(m_pCurrentCell == NULL)
			{
				pBL = m_pShadow->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			}
			else
			{
				pBL = m_pCurrentCell->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			}
			xxx_UT_DEBUGMSG(("New Shadow block %x created and set as current \n",pBL));
			if (!pBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			m_pCurrentBL = pBL;
			*psfh = pBL;
		}

	}
	break;


	case PTX_SectionTable:
	{
		if (m_bListening)
		{
			// append a new BlockLayout to that SectionLayout
			fl_ContainerLayout*	pTL = m_pShadow->append(sdh, pcr->getIndexAP(),FL_CONTAINER_TABLE);
			UT_DEBUGMSG(("New Shadow Table %p created and set as current \n",pTL));
			m_pCurrentTL = static_cast<fl_TableLayout *>(pTL);
			*psfh = pTL;
		}

	}
	break;


	case PTX_SectionCell:
	{
		if (m_bListening && m_pCurrentTL)
		{

			// append a new BlockLayout to that SectionLayout
			fl_ContainerLayout*	pCell = m_pCurrentTL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_CELL);
			UT_DEBUGMSG(("New Shadow Cell %p created and set as current \n",pCell));
			m_pCurrentCell = static_cast<fl_CellLayout *>(pCell);
			*psfh = m_pCurrentCell;
		}

	}
	break;

	case PTX_EndTable:
	{
		if(m_pCurrentTL == NULL)
		{
			m_pDoc->miniDump(sdh,6);
		}

		UT_return_val_if_fail(m_pCurrentTL,false);
		UT_DEBUGMSG(("!!!! Append End Table to Shadow \n"));
		if(m_pCurrentTL->getContainerType() != FL_CONTAINER_TABLE)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		*psfh = m_pCurrentTL;
		UT_DEBUGMSG(("SEVIOR: End table in  shadow listener \n"));
		m_pCurrentTL->setDirty();
		m_pCurrentTL->setEndTableIn();
		m_pCurrentTL = NULL;
	}
	break;
	case PTX_EndCell:
	{
		UT_ASSERT(m_pCurrentCell);
		UT_DEBUGMSG(("!!!! Append End Cell in Shadow Listener\n"));
		*psfh = m_pCurrentCell;
		m_pCurrentCell = NULL;
	}
	break;

	default:
		UT_ASSERT(0);
		return false;
	}
	//
	// We're not printing
	//
	return true;
}

bool fl_ShadowListener::change(fl_ContainerLayout* /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fl_ShadowListener::insertStrux(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * /*pcr*/,
									pf_Frag_Strux* /*sdh*/,
									PL_ListenerId /*lid*/,
									void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
																PL_ListenerId lid,
																fl_ContainerLayout* sfhNew))
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fl_ShadowListener::signal(UT_uint32 /*iSignal*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}


