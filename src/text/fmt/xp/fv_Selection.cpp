/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_Selection.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "fv_View.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "fl_TableLayout.h"
#include "pd_Document.h"
#include "ie_exp.h"
#include "ie_exp_RTF.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"

#include "ut_bytebuf.h"

FV_Selection::FV_Selection (FV_View * pView)
	: m_pView (pView), 
	  m_iSelectionMode(FV_SelectionMode_NONE),
	  m_iSelectAnchor(0),
	  m_iSelectLeftAnchor(0),
	  m_iSelectRightAnchor(0),
	  m_pTableOfSelectedColumn(NULL)
{
	UT_ASSERT (pView);
	m_vecSelRanges.clear();
	m_vecSelRTFBuffers.clear();
}

FV_Selection::~FV_Selection()
{
	m_pTableOfSelectedColumn = NULL;
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
	UT_VECTOR_PURGEALL(UT_ByteBuf  *,m_vecSelRTFBuffers);
	UT_VECTOR_PURGEALL(FV_SelectionCellProps *,m_vecSelCellProps);
}

void FV_Selection::setMode(FV_SelectionMode iSelMode)
{
	m_iSelectionMode = iSelMode;
	if((m_iSelectionMode == FV_SelectionMode_NONE) || (m_iSelectionMode == FV_SelectionMode_Single) )
	{
		m_pTableOfSelectedColumn = NULL;
		UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
		UT_VECTOR_PURGEALL(UT_ByteBuf  *,m_vecSelRTFBuffers);
		UT_VECTOR_PURGEALL(FV_SelectionCellProps *,m_vecSelCellProps);
		m_vecSelRanges.clear();
		m_vecSelRTFBuffers.clear();
		m_vecSelCellProps.clear();
	}
}

PD_Document * FV_Selection::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout * FV_Selection::getLayout(void) const
{
	return m_pView->getLayout();
}

PT_DocPosition FV_Selection::getSelectionAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) ||  (m_vecSelRanges.getItemCount() == 0))
	{
		return m_iSelectAnchor;
	}
	PD_DocumentRange * pDocRange = static_cast<PD_DocumentRange *>(m_vecSelRanges.getNthItem(0));
	return pDocRange->m_pos1;
}

void FV_Selection::setSelectionAnchor(PT_DocPosition pos)
{
		m_iSelectAnchor = pos;
}


PT_DocPosition FV_Selection::getSelectionLeftAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0))
	{
		return m_iSelectLeftAnchor;
	}
	PD_DocumentRange * pDocRange = static_cast<PD_DocumentRange *>(m_vecSelRanges.getNthItem(0));
	return pDocRange->m_pos1;
}

void FV_Selection::setSelectionLeftAnchor(PT_DocPosition pos)
{
	m_iSelectLeftAnchor = pos;
}


PT_DocPosition FV_Selection::getSelectionRightAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0) )
	{
		return m_iSelectRightAnchor;
	}
	PD_DocumentRange * pDocRange = static_cast<PD_DocumentRange *>(m_vecSelRanges.getNthItem(0));
	return pDocRange->m_pos2;
}

void FV_Selection::setSelectionRightAnchor(PT_DocPosition pos)
{
	m_iSelectRightAnchor = pos;
}

bool FV_Selection::isPosSelected(PT_DocPosition pos) const
{
	if(m_iSelectionMode < FV_SelectionMode_Multiple)
	{
		PT_DocPosition posLow = m_iSelectAnchor;
		PT_DocPosition posHigh = m_pView->getPoint();
		if(posHigh < posLow)
		{
			posHigh = m_iSelectAnchor;
			posLow = m_pView->getPoint();
		}
		return (pos >= posLow) && (pos <=posHigh);
	}
	UT_sint32 i =0;
	for(i=0; i < static_cast<UT_sint32>(m_vecSelRanges.getItemCount()); i++)
	{
		PD_DocumentRange * pDocRange = static_cast<PD_DocumentRange *>(m_vecSelRanges.getNthItem(i));
		xxx_UT_DEBUGMSG(("Looking at pos %d low %d hight %d \n",pos, pDocRange->m_pos1,pDocRange->m_pos2 ));
		if ((pos >= pDocRange->m_pos1) && (pos <= pDocRange->m_pos2))
		{
			return true;
		}
	}
	return false;
		
}

bool FV_Selection::isSelected(void) const
{
	return FV_SelectionMode_NONE != m_iSelectionMode;
}

void FV_Selection::clearSelection(void)
{
	setMode(FV_SelectionMode_NONE);
}

void FV_Selection::setTableLayout(fl_TableLayout * pFL)
{
	UT_ASSERT((m_iSelectionMode == 	FV_SelectionMode_TableColumn) 
			  || ( m_iSelectionMode == 	FV_SelectionMode_TableRow));
	m_pTableOfSelectedColumn = pFL;
}

/*!
 * Add a range to the list of selected regions as defined by posLow, posHigh.
 * If bAddData is true also make a copy of the selected text in RTF format.
 */
void FV_Selection::addSelectedRange(PT_DocPosition posLow, PT_DocPosition posHigh, bool bAddData)
{

}


/*!
 * Add a cell to the list of selected regions.
 */
void FV_Selection::addCellToSelection(fl_CellLayout * pCell)
{
	UT_ASSERT((m_iSelectionMode == 	FV_SelectionMode_TableColumn) 
			  || ( m_iSelectionMode == 	FV_SelectionMode_TableRow));
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = pCell->getStruxDocHandle();
	PT_DocPosition posLow = getDoc()->getStruxPosition(sdhStart) +1; // First block

	bool bres;
	bres = getDoc()->getNextStruxOfType(sdhStart,PTX_EndCell,&sdhEnd);
	PT_DocPosition posHigh = getDoc()->getStruxPosition(sdhEnd) -1;
	UT_ASSERT(bres && sdhEnd);
	PD_DocumentRange * pDocRange = new PD_DocumentRange(getDoc(),posLow,posHigh);
	m_vecSelRanges.addItem(static_cast<void *>(pDocRange));
	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	UT_ByteBuf * pByteBuf = new UT_ByteBuf;
    if (pExpRtf)
    {
		pExpRtf->copyToBuffer(pDocRange,pByteBuf);
		DELETEP(pExpRtf);
    }
	m_vecSelRTFBuffers.addItem(static_cast<void *>(pByteBuf));
	FV_SelectionCellProps * pCellProps = new FV_SelectionCellProps;
	UT_sint32 iLeft,iRight,iTop,iBot;
	m_pView->getCellParams(posLow,&iLeft,&iRight,&iTop,&iBot);
	pCellProps->m_iLeft = iLeft;
	pCellProps->m_iRight = iRight;
	pCellProps->m_iTop = iTop;
	pCellProps->m_iBot = iBot;
	m_vecSelCellProps.addItem(static_cast<void *>(pCellProps));
}

/*!
 * Return the ith selection.
 */
PD_DocumentRange * FV_Selection::getNthSelection(UT_sint32 i)
{
	if(i >= getNumSelections())
	{
		return NULL;
	}
	PD_DocumentRange * pDocRange = static_cast<PD_DocumentRange *>(m_vecSelRanges.getNthItem(i));
	return pDocRange;
}

/*!
 * Return the number of active selections.
 */
UT_sint32 FV_Selection::getNumSelections(void)
{
	return static_cast<UT_sint32>(m_vecSelRanges.getItemCount());
}
