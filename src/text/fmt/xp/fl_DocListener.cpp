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

fl_DocListener::fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout)
{
	m_pDoc = doc;
	m_pLayout = pLayout;
	m_bScreen = pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN);
	m_iGlobCounter = 0;
	m_pCurrentSL = NULL;
}

fl_DocListener::~fl_DocListener()
{
}

UT_Bool fl_DocListener::populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);
	//UT_DEBUGMSG(("fl_DocListener::populate\n"));

	UT_Bool bResult = UT_FALSE;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			PT_BlockOffset blockOffset = pcrs->getBlockOffset();
			UT_uint32 len = pcrs->getLength();
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			bResult = pBLSL->bl_doclistener_populateSpan(pBL, pcrs, blockOffset, len);
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
		return UT_FALSE;
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

UT_Bool fl_DocListener::populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pLayout);
	//UT_DEBUGMSG(("fl_DocListener::populateStrux\n"));

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
			pAP->getAttribute((XML_Char*)"type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_stricmp(pszSectionType, "doc"))
				)
			{
				// append a SectionLayout to this DocLayout
				fl_DocSectionLayout* pSL = new fl_DocSectionLayout(m_pLayout, sdh, pcr->getIndexAP());
				if (!pSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return UT_FALSE;
				}
			
				m_pLayout->addSection(pSL);

				*psfh = (PL_StruxFmtHandle)pSL;
				
				m_pCurrentSL = pSL;
			}
			else
			{
				if (0 == UT_stricmp(pszSectionType, "header"))
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute((XML_Char*)"id", pszID);

					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					UT_ASSERT(pDocSL);
			
					// append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_HEADER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return UT_FALSE;
					}
			
					pDocSL->setHdrFtr(FL_HDRFTR_HEADER, pSL);

					*psfh = (PL_StruxFmtHandle)pSL;
					
					m_pCurrentSL = pSL;
				}
				else if (0 == UT_stricmp(pszSectionType, "footer"))
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute((XML_Char*)"id", pszID);

					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					UT_ASSERT(pDocSL);
			
					// append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_FOOTER, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return UT_FALSE;
					}
			
					pDocSL->setHdrFtr(FL_HDRFTR_FOOTER, pSL);

					*psfh = (PL_StruxFmtHandle)pSL;
					
					m_pCurrentSL = pSL;
				}
				else
				{
					return UT_FALSE;
				}
			}
		}
		else
		{
			// TODO fail?
			return UT_FALSE;
		}
	}
	break;

	case PTX_Block:
	{
		UT_ASSERT(m_pCurrentSL);
		
		// append a new BlockLayout to that SectionLayout
		fl_BlockLayout*	pBL = m_pCurrentSL->appendBlock(sdh, pcr->getIndexAP());
		if (!pBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout"));
			return UT_FALSE;
		}

		// BUGBUG: this is *not* thread-safe, but should work for now
		if (m_bScreen)
			m_pLayout->queueBlockForSpell(pBL);

		*psfh = (PL_StruxFmtHandle)pBL;
	}
	break;
			
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return UT_TRUE;
}

UT_Bool fl_DocListener::change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr)
{
	//UT_DEBUGMSG(("fl_DocListener::change\n"));
	UT_Bool bResult = UT_FALSE;

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
			bResult = UT_FALSE;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_MultiStepStart:
			m_iGlobCounter++;
			bResult = UT_TRUE;
			goto finish_up;
			
		case PX_ChangeRecord_Glob::PXF_MultiStepEnd:
			m_iGlobCounter--;
			bResult = UT_TRUE;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_UserAtomicStart:	// TODO decide what (if anything) we need
		case PX_ChangeRecord_Glob::PXF_UserAtomicEnd:	// TODO to do here.
			bResult = UT_TRUE;
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
			bResult = UT_FALSE;
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
			bResult = UT_FALSE;
			goto finish_up;
		}
	}

	case PX_ChangeRecord::PXT_InsertStrux:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		bResult = UT_FALSE;
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
		if (pView)
			pView->_setPoint(pcr->getPosition());
		goto finish_up;
	}
		
	default:
		UT_ASSERT(0);
		bResult = UT_FALSE;
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

UT_Bool fl_DocListener::insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew))
{
	//UT_DEBUGMSG(("fl_DocListener::insertStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	fl_Layout * pL = (fl_Layout *)sfh;
	switch (pL->getType())				// see what the immediately prior strux is.
	{
	case PTX_Section:					// the immediately prior strux is a section.

		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
			// we are inserting a section immediately after a section (with no
			// interviening block).  this is probably a bug, because there should
			// at least be an empty block between them (so that the user can set
			// the cursor there and start typing, if nothing else).
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_FALSE;
				
		case PTX_Block:					// we are inserting a block.
		{
			// the immediately prior strux is a section.  So, this will
			// become the first block of the section and have no text.

			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			UT_Bool bResult = pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
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
			return UT_FALSE;
		}
		
	case PTX_Block:						// the immediately prior strux is a block.

		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
		{
			// the immediately prior strux is a block.  everything from this point
			// forward (to the next section) needs to be re-parented to this new
			// section.  we also need to verify that there is a block immediately
			// after this new section -- a section must be followed by a block
			// because a section cannot contain content.

			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			UT_Bool bResult = pBLSL->bl_doclistener_insertSection(pBL, pcrx,sdh,lid,pfnBindHandles);
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
			// the immediately prior strux is also a block.  insert the new
			// block and split the content between the two blocks.
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			fl_SectionLayout* pBLSL = pBL->getSectionLayout();
			UT_Bool bResult = pBLSL->bl_doclistener_insertBlock(pBL, pcrx,sdh,lid,pfnBindHandles);
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
			return UT_FALSE;
		}

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool fl_DocListener::signal(UT_uint32 iSignal)
{
        UT_Bool bCursorErased = UT_FALSE;
        FV_View* pView = m_pLayout->getView();
	switch (iSignal)
	{
	case PD_SIGNAL_UPDATE_LAYOUT:
#ifdef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
		if(pView->isCursorOn()== UT_TRUE)
		{
		      pView->eraseInsertionPoint();
		      bCursorErased = UT_TRUE;
		}
		pView->updateScreen();
		if(bCursorErased == UT_TRUE)
		{
		      pView->drawInsertionPoint();
		}

		break;
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return UT_TRUE;
}
