/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#include "rl_DocListener.h"
#include "px_ChangeRecord.h"
#include "px_CR_Strux.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "rl_ParagraphLayout.h"
#include "rl_DocSectionLayout.h"
#include "rl_TableLayout.h"
#include "xav_Listener.h"
#include "rv_View.h"

rl_DocListener::rl_DocListener(PD_Document* doc, rl_DocLayout *pLayout) :
	m_pDoc(doc),
	m_pLayout(pLayout)
{
}

rl_DocListener::~rl_DocListener()
{
}

bool rl_DocListener::populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr)
{
	UT_DEBUGMSG(("rl_DocListener::populate type %d\n", pcr->getType()));

	bool bResult = false;

	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			UT_DEBUGMSG(("rl_DocListener::populate PX_ChangeRecord::PXT_InsertSpan\n"));
			
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
			rl_Layout * pL = (rl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);			

			rl_ParagraphLayout * pPL = static_cast<rl_ParagraphLayout *>(pL);
			pPL->doclistener_insertSpan(pcrs);
			
			bResult = true;
			break;
		}
		case PX_ChangeRecord::PXT_InsertObject:
		{
			UT_DEBUGMSG(("rl_DocListener::populate PX_ChangeRecord::PXT_InsertObject\n"));
			bResult = true;
			break;
		}
		case PX_ChangeRecord::PXT_InsertFmtMark:
		{
			UT_DEBUGMSG(("rl_DocListener::populate PX_ChangeRecord::PXT_InsertFmtMark\n"));
			bResult = true;
			break;
		}	
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			bResult = false;	
			break;
	}
	
	return bResult;	
}

bool rl_DocListener::populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);	
	
	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_Section\n"));
			
			// close any open section container
			m_pLayout->closeAll();
			
			rl_ContainerLayout* pCL = new rl_DocSectionLayout(PTX_Section, sdh, pcr->getIndexAP(), m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_SectionFootnote:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionFootnote\n"));
			
			rl_ContainerLayout* pCL = new rl_ContainerLayout(PTX_SectionFootnote, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}			
		case PTX_SectionEndnote:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionEndnote\n"));
			
			rl_ContainerLayout* pCL = new rl_ContainerLayout(PTX_SectionEndnote, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_EndFootnote:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndFootnote\n"));
			
			m_pLayout->closeContainer(PTX_SectionFootnote, true);
			*psfh = NULL;

			break;
		}
		case PTX_EndEndnote:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndEndnote\n"));
			
			m_pLayout->closeContainer(PTX_SectionEndnote, true);
			*psfh = NULL;

			break;
		}
		case PTX_SectionTOC:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionTOC\n"));
			
			// close any open paragraph
			m_pLayout->closeContainer(PTX_Block, false);		
			
			rl_ContainerLayout* pCL = new rl_ContainerLayout(PTX_SectionTOC, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_EndTOC:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndTOC\n"));
			
			m_pLayout->closeContainer(PTX_SectionTOC, true);
			*psfh = NULL;
			
			break;
		}
		case PTX_SectionHdrFtr:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionHdrFtr\n"));
			
			// close any open section container
			m_pLayout->closeAll();			
			
			rl_ContainerLayout* pCL = new rl_ContainerLayout(PTX_SectionHdrFtr, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_Block:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_Block\n"));

			// close any open paragraph
			m_pLayout->closeContainer(PTX_Block, false);
							
			rl_ContainerLayout* pCL = new rl_ParagraphLayout(PTX_Block, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_SectionTable:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionTable\n"));

			// close any open paragraph
			m_pLayout->closeContainer(PTX_Block, false);
			
			rl_ContainerLayout* pCL = new rl_TableLayout(PTX_SectionTable, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_SectionFrame:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionFrame\n"));

			// close any open paragraph
			m_pLayout->closeContainer(PTX_Block, false);
			
			rl_ContainerLayout* pCL = new rl_ContainerLayout(PTX_SectionFrame, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_EndFrame:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndFrame\n"));
			
			m_pLayout->closeContainer(PTX_SectionFrame, true);
			*psfh = NULL;
			
			break;
		}
		case PTX_SectionCell:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionCell\n"));
			
			rl_ContainerLayout* pCL = new rl_CellLayout(PTX_SectionCell, sdh/*, pcr->getIndexAP()*/, m_pLayout->getCurrentContainer(), m_pLayout);
			if (!pCL)
			{
				UT_DEBUGMSG(("rl_DocListener error: no memory to create rl_ContainerLayout"));
				return false;
			}
			
			m_pLayout->add(pCL);
			*psfh = (PL_StruxFmtHandle)pCL;
			break;
		}
		case PTX_EndTable:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndTable\n"));
			
			m_pLayout->closeContainer(PTX_SectionTable, true);
			*psfh = NULL;

			break;
		}
		case PTX_EndCell:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_EndCell\n"));
			
			m_pLayout->closeContainer(PTX_SectionCell, true);
			*psfh = NULL;

			break;
		}
		default:
			UT_ASSERT(0);
			return false;
	}
	
	return true;	
}

bool rl_DocListener::change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr)
{
	UT_DEBUGMSG(("rl_DocListener::change\n"));

	bool bResult = true;
	AV_ChangeMask chgMask = AV_CHG_NONE;
	
	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_GlobMarker:
		{
			break;
		}
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
			rl_Layout * pL = (rl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);			

			rl_ParagraphLayout * pPL = static_cast<rl_ParagraphLayout *>(pL);
			pPL->doclistener_insertSpan(pcrs);
			
			bResult = true;
			break;
		}
		case PX_ChangeRecord::PXT_DeleteSpan:
		{
			break;
		}
		case PX_ChangeRecord::PXT_ChangeSpan:
		{
			break;
		}
		case PX_ChangeRecord::PXT_InsertFmtMark:
		{
			break;
		}
		case PX_ChangeRecord::PXT_DeleteFmtMark:
		{
			break;
		}
		case PX_ChangeRecord::PXT_ChangeFmtMark:
		{
			break;
		}
		case PX_ChangeRecord::PXT_DeleteStrux:
		{
			break;
		}
		case PX_ChangeRecord::PXT_ChangeStrux:
		{
			break;
		}
		case PX_ChangeRecord::PXT_InsertStrux:
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			bResult = false;
			break;
		}
		case PX_ChangeRecord::PXT_InsertObject:
		{
			break;
		}
		case PX_ChangeRecord::PXT_DeleteObject:
		{
			break;
		}
		case PX_ChangeRecord::PXT_ChangeObject:
		{
			break;			
		}
		case PX_ChangeRecord::PXT_ChangePoint:
		{
			break;
		}
		case PX_ChangeRecord::PXT_ListUpdate:
		{
			break;
		}
		case PX_ChangeRecord::PXT_StopList:
		{
			break;
		}
		case PX_ChangeRecord::PXT_UpdateField:
		{
			break;
		}
		case PX_ChangeRecord::PXT_RemoveList:
		{
			break;
		}
		case PX_ChangeRecord::PXT_UpdateLayout:
		{
			break;
		}
		default:
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			bResult = false;
			break;
		}
	}
	
	// make the view notify its various listeners to update
	if (chgMask != AV_CHG_NONE)
	{
		rv_View* pView = m_pLayout->getView();
		UT_ASSERT(pView);
		if (!pView) return false;
		
		pView->notifyListeners(chgMask);
	}
	
	return bResult;
}
	
bool rl_DocListener::insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdhNew,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew))
{
	UT_DEBUGMSG(("rl_DocListener::insertStrux()\n"));
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	
	rl_Layout * pL = (rl_Layout *)sfh;
	UT_ASSERT(pL);
	if (!pL) return false;
	
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);	
		
	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_Section\n"));
			break;
		}
		case PTX_SectionFootnote:
		case PTX_SectionEndnote:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_SectionFootnote/PTX_SectionEndnote\n"));
			break;
		}
		case PTX_EndFootnote:
		case PTX_EndEndnote:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_EndFootnote/PTX_EndEndnote\n"));
			break;
		}
		case PTX_SectionTOC:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_SectionTOC\n"));
			break;
		}
		case PTX_EndTOC:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_EndTOC\n"));
			break;
		}
		case PTX_SectionHdrFtr:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_SectionHdrFtr\n"));
			break;
		}
		case PTX_Block:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_Block\n"));
			break;
		}
		case PTX_SectionTable:
		{
			UT_DEBUGMSG(("rl_DocListener::populateStrux PX_ChangeRecord_Strux::PTX_SectionTable\n"));
			break;
		}
		case PTX_SectionFrame:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_SectionFrame\n"));
			break;
		}
		case PTX_EndFrame:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_EndFrame\n"));
			break;
		}
		case PTX_SectionCell:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_SectionCell\n"));
			break;
		}
		case PTX_EndTable:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_EndTable\n"));
			break;
		}
		case PTX_EndCell:
		{
			UT_DEBUGMSG(("rl_DocListener::insertStrux PX_ChangeRecord_Strux::PTX_EndCell\n"));
			break;
		}
		default:
			UT_ASSERT(0);
			return false;
	}	
	
	return true;
}

bool rl_DocListener::signal(UT_uint32 iSignal)
{
	UT_DEBUGMSG(("rl_DocListener::signal()\n"));

	switch (iSignal)
	{
		case PD_SIGNAL_UPDATE_LAYOUT:
/*#ifdef UPDATE_LAYOUT_ON_SIGNAL
			m_pLayout->updateLayout();
#endif*/
			// pView->updateScreen();
			m_pLayout->formatAll();		
			break;
		case PD_SIGNAL_REFORMAT_LAYOUT:
			m_pLayout->formatAll();
			break;
		case PD_SIGNAL_DOCPROPS_CHANGED_REBUILD:
			/*m_pLayout->updatePropsRebuild();*/
			m_pLayout->formatAll();
			break;
		case PD_SIGNAL_DOCPROPS_CHANGED_NO_REBUILD:
			/*m_pLayout->updatePropsNoRebuild();*/
			m_pLayout->formatAll();
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
	}
	return true;
}
