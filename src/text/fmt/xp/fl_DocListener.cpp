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
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "px_CR_TempSpanFmt.h"
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

// #define UPDATE_LAYOUT_ON_SIGNAL

fl_DocListener::fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout)
{
	m_pDoc = doc;
	m_pLayout = pLayout;
	m_bScreen = pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN);
	m_iGlobCounter = 0;
}

fl_DocListener::~fl_DocListener()
{
}

UT_Bool fl_DocListener::populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);
	UT_DEBUGMSG(("fl_DocListener::populate\n"));

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
			bResult = pBL->doclistener_populateSpan(pcrs, blockOffset, len);
			goto finish_up;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			PT_BlockOffset blockOffset = pcro->getBlockOffset();

			bResult = pBL->doclistener_populateObject(blockOffset,pcro);
			goto finish_up;
		}

	default:
		UT_ASSERT(0);
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
	UT_DEBUGMSG(("fl_DocListener::populateStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			// append a SectionLayout to this DocLayout
			fl_SectionLayout* pSL = new fl_SectionLayout(m_pLayout, sdh, pcr->getIndexAP());
			if (!pSL)
			{
				UT_DEBUGMSG(("no memory for SectionLayout"));
				return UT_FALSE;
			}
			
			m_pLayout->addSection(pSL);

			*psfh = (PL_StruxFmtHandle)pSL;
		}
		break;

	case PTX_Block:
		{
			// locate the last SectionLayout
			fl_SectionLayout* pSL = m_pLayout->getLastSection();
			UT_ASSERT(pSL);

			// append a new BlockLayout to that SectionLayout
			fl_BlockLayout*	pBL = pSL->appendBlock(sdh, pcr->getIndexAP());
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
	UT_DEBUGMSG(("fl_DocListener::change\n"));
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
		bResult = pBL->doclistener_insertSpan(pcrs);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		bResult = pBL->doclistener_deleteSpan(pcrs);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeSpan:
	{
		const PX_ChangeRecord_SpanChange * pcrsc = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		bResult = pBL->doclistener_changeSpan(pcrsc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_TempSpanFmt:
	{
		const PX_ChangeRecord_TempSpanFmt * pcrTSF = static_cast<const PX_ChangeRecord_TempSpanFmt *>(pcr);
		if (pcrTSF->getEnabled())
		{
			/*
			  This is just a temporary change at the insertion 
			  point.  It won't take effect unless something's 
			  typed -- but it will cause the toolbars and etc.
			  to be updated.
			*/

			FV_View* pView = m_pLayout->m_pView;
			if (pView)
			{
				UT_ASSERT(pView->isSelectionEmpty());
				pView->_setPoint(pcrTSF->getPosition());
				pView->_setPointAP(pcrTSF->getIndexAP());
#if 0				
				pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
#endif				
			}
		}
		else
		{
			// we have been asked to turn off the temporary change at the
			// insertion point.  we need to update any toolbars.

			FV_View* pView = m_pLayout->m_pView;
			if (pView)
			{
				UT_ASSERT(pView->isSelectionEmpty());
				// TODO decide if we need to call "pView->_setPoint(pcrTSF->getPosition());"
				// TODO and if so, add AV_CHG_TYPING to the following notifyListeners().
				pView->_clearPointAP(UT_FALSE);
#if 0				
				pView->notifyListeners(AV_CHG_FMTCHAR);
#endif				
			}
		}

		bResult = UT_TRUE;
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
			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			bResult = pSL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_Block:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			bResult = pBL->doclistener_deleteStrux(pcrx);
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
			fl_SectionLayout* pSL = static_cast<fl_SectionLayout*>(pL);
			bResult = pSL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		
		case PTX_Block:
		{
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
			bResult = pBL->doclistener_changeStrux(pcrxc);
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
		bResult = pBL->doclistener_insertObject(pcro);
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_DeleteObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		bResult = pBL->doclistener_deleteObject(pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeObject:
	{
		const PX_ChangeRecord_ObjectChange * pcroc = static_cast<const PX_ChangeRecord_ObjectChange *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
		bResult = pBL->doclistener_changeObject(pcroc);
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
			// we are inserting a section immediately after a section (with no
			// interviening block).  this is probably a bug, because there should
			// at least be an empty block between them (so that the user can set
			// the cursor there and start typing, if nothing else).
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_FALSE;
				
		case PTX_Block:					// we are inserting a block.
			// the immediately prior strux is a section.  this probably cannot
			// happen because the insertion point would have been immediately
			// after the existing first block in the section rather than between
			// the section and block.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_FALSE;

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
			UT_Bool bResult = pBL->doclistener_insertSection(pcrx,sdh,lid,pfnBindHandles);
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
			UT_Bool bResult = pBL->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles);
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
	switch (iSignal)
	{
	case PD_SIGNAL_UPDATE_LAYOUT:
#ifdef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
		m_pLayout->getView()->updateScreen();
		break;
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return UT_TRUE;
}
