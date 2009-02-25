/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fp_TableContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fv_View.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_FootnoteLayout.h"
#include "fp_FootnoteContainer.h"
#include "fp_FrameContainer.h"
#include "gr_Painter.h"
#include "pd_Document.h"

fp_TableRowColumn::fp_TableRowColumn(void) :
		requisition(0),
        allocation(0),
        spacing(0),
        need_expand(false),
		need_shrink(true),
		expand(true),
        shrink(true),
        empty(true)
{
}

fp_TableRowColumn::~fp_TableRowColumn(void)
{
}


/*!
  Create Cell container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_CellContainer::fp_CellContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_CELL, pSectionLayout),
	  m_iLeftAttach(0),
	  m_iRightAttach(0),
	  m_iTopAttach(0),
	  m_iBottomAttach(0),
	  m_borderColorNone(127,127,127),
	  m_iLeftPad(0),
	  m_iRightPad(0),
	  m_iTopPad(0),
	  m_iBotPad(0),
	  m_pNextInTable(NULL),
	  m_pPrevInTable(NULL),
	  m_bXexpand(true),
	  m_bYexpand(false),
	  m_bXshrink(false),
	  m_bYshrink(true),
	  m_bXfill(true),
	  m_bYfill(false),
	  m_iLeft(0),
	  m_iRight(0),
	  m_iTopY(0),
	  m_iBotY(0),
	  m_bDrawLeft(false),
	  m_bDrawTop(false),
	  m_bDrawBot(false),
	  m_bDrawRight(false),
	  m_bLinesDrawn(false),
	  m_bBgDirty(true),
	  m_bIsSelected(false),
	  m_bDirty(true),
	  m_bIsRepeated(false)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_CellContainer::~fp_CellContainer()
{
	xxx_UT_DEBUGMSG(("Deleting Cell container %x \n",this));
	setNext(NULL);
	setPrev(NULL);
}

bool fp_CellContainer::isRepeated(void) const
{
	return m_bIsRepeated;
}

void fp_CellContainer::setHeight(UT_sint32 iHeight)
{
	xxx_UT_DEBUGMSG(("cell: Height was %d \n",getHeight()));
	if ((iHeight == getHeight()) || (iHeight== 0))
	{
		return;
	}
	clearScreen();
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab && getBottomAttach() == pTab->getNumRows())
	{
		fp_CellContainer * pCell = pTab->getCellAtRowColumn(pTab->getNumRows() -1,0);
		while(pCell)
		{
			pCell->clearScreen();
			pCell->getSectionLayout()->setNeedsRedraw();
			pCell->getSectionLayout()->markAllRunsDirty();
			pCell = static_cast<fp_CellContainer *>(pCell->getNext());
		}
	}
	fp_VerticalContainer::setHeight(iHeight);
	xxx_UT_DEBUGMSG(("cell: Height set to %d \n",getHeight()));
	fl_SectionLayout * pSL = getSectionLayout();
	pSL = static_cast<fl_SectionLayout *>(pSL->myContainingLayout());
	UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_TABLE);
	static_cast<fl_TableLayout *>(pSL)->setDirty();
	static_cast<fl_TableLayout *>(pSL)->setHeightChanged(this);
}

/*!
 * Return the broken table that contains this cell and the given Container
 */
fp_TableContainer * fp_CellContainer::getBrokenTable(fp_Container * pCon)
{
	fp_TableContainer * pMaster = static_cast<fp_TableContainer *>(getContainer());
	fp_CellContainer * pTopCell = this;
	bool bNest = false;
	if(pMaster == NULL)
	{
		return pMaster;
	}
	fp_TableContainer * pBroke = pMaster->getFirstBrokenTable();
	while(pBroke != NULL)
    {
		if(pTopCell->doesOverlapBrokenTable(pBroke))
		{
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer * pT = static_cast<fp_TableContainer *>(pCon);
				if(!pT->isThisBroken())
				{
					fp_TableContainer * pTT = pT->getFirstBrokenTable();
					if(pTT == NULL)
					{
						UT_sint32 iY = pT->getY();
						UT_DEBUGMSG(("No Broken Table!! \n"));
						pT = static_cast<fp_TableContainer *>(pT->VBreakAt(0));
						pT->setY(iY);
					}
					else
					{
						pT = pTT;
					}
				}
				xxx_UT_DEBUGMSG(("Y of broken table %x is %d yBreak is %d height %d \n",pT,pT->getY(),pT->getYBreak(),pT->getHeight()));
				bNest = true;
			}
			if(pBroke->isInBrokenTable(this,pCon))
			{
				if(bNest)
				{
					xxx_UT_DEBUGMSG(("Y of Found broken table %x is %d yBreak is %d height %d \n",pBroke,pBroke->getY(),pBroke->getYBreak(),pBroke->getHeight()));
				}
				return pBroke;
			}
		}
		pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	}
	return pMaster;
}

/*!
 * This Method returns the column or shadow that embeds the container given.
 */
fp_VerticalContainer * fp_CellContainer::getColumn(fp_Container * pCon)
{
	fp_TableContainer * pBroke = getBrokenTable(pCon);
	if(pBroke == NULL)
	{
		pBroke = static_cast<fp_TableContainer *>(getContainer());
	}
	if(pBroke == NULL)
	{
		return NULL;
	}
	if(isInNestedTable())
	{
		xxx_UT_DEBUGMSG(("getColumn in nested table \n"));
	}
	bool bStop = false;
	fp_CellContainer * pCell = NULL;
	fp_Column * pCol = NULL;
	//
	// Now FIXED for nested tables off first page
	//
	while(pBroke && pBroke->isThisBroken() && !bStop)
	{
		fp_Container * pCon = pBroke->getContainer();
		if(pCon->isColumnType())
		{
			if(pCon->getContainerType() == FP_CONTAINER_COLUMN)
			{
				pCol = static_cast<fp_Column *>(pCon);
			}
			else if(pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
			{
				return static_cast<fp_VerticalContainer *>(pCon);
			}
			else
			{
				pCol = static_cast<fp_Column *>(pCon->getColumn());
			}
			bStop = true;
		}
		else
		{
			pCell = static_cast<fp_CellContainer *>(pBroke->getContainer());
			UT_ASSERT(pCell->getContainerType() == FP_CONTAINER_CELL);
			pBroke = pCell->getBrokenTable(static_cast<fp_Container *>(pBroke));
		}
	}
	if(pCell && (pBroke == NULL))
	{
		return static_cast<fp_Column *>(static_cast<fp_Container *>(pCell)->getColumn());
	}
	else if(pBroke == NULL)
	{
		return NULL;
	}
	if(!bStop)
	{
		pCol = static_cast<fp_Column *>(pBroke->getContainer());
	}
	//	UT_ASSERT(pCol->getContainerType() != FP_CONTAINER_CELL);
	if(pCol->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_Container * pCon = static_cast<fp_Container *>(pCol);
		while(pCon && !pCon->isColumnType())
		{
			pCon = pCon->getContainer();
		}
		if(pCon)
		{
			pCol = static_cast<fp_Column *>(pCon);
		}
		else
		{
			pCol = NULL;
		}
	}
	if(isInNestedTable())
	{
		xxx_UT_DEBUGMSG(("getColumn in nested table got column %x \n",pCol));
	}
	else
    {
		xxx_UT_DEBUGMSG(("getColumn in table got column %x \n",pCol));
	}
	return static_cast<fp_VerticalContainer *>(pCol);
}

bool fp_CellContainer::containsNestedTables(void)
{
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
	return (pCL->getNumNestedTables() > 0);
}

/*!
 * Return the screen rectangle that is the intersection of the cell and the
 * broken table.
 */
void fp_CellContainer::_getBrokenRect(fp_TableContainer * pBroke, fp_Page * &pPage, UT_Rect &bRec, GR_Graphics * pG)
{
	UT_ASSERT(static_cast<fl_TableLayout *>(getSectionLayout()->myContainingLayout()) &&
	static_cast<fl_TableLayout *>(getSectionLayout()->myContainingLayout())->getContainerType() == FL_CONTAINER_TABLE);

	fp_Column * pCol = NULL;
	UT_sint32 col_y =0;
	UT_sint32 col_x =0;
	UT_sint32 iLeft = m_iLeft;
	UT_sint32 iRight = m_iRight;
	UT_sint32 iTop = m_iTopY;
	UT_sint32 iBot = m_iBotY;
	bool bIsNested = false;
	if(pBroke && (pBroke->getContainer()->getContainerType() == FP_CONTAINER_CELL))
	{
		bIsNested = true;
		if(pBroke->getYBreak() > 0)
		{
			fp_TableContainer * pPrev = static_cast<fp_TableContainer *>(pBroke->getPrev());
			UT_DEBUGMSG(("Broken nested table y %d ybreak %d height %d prev Y %d height %d yBreak %d \n",pBroke->getY(),pBroke->getYBreak(),pBroke->getHeight(),pPrev->getY(),pPrev->getHeight(),pPrev->getYBreak()));
		}
	}
	UT_sint32 offx = 0;
	UT_sint32 offy = 0;
	bool bFrame = false;
	if(pBroke)
	{
		pPage = pBroke->getPage();
		if(pPage)
		{
			if(pBroke->getContainer()->getContainerType() == FP_CONTAINER_FRAME)
		    {
				fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pBroke->getContainer())
;
				getView()->getPageScreenOffsets(pPage,col_x,col_y);
				//
				// Use col_x, col_y later.
				//
				offx = pFC->getX();
				offy = pFC->getY();
				bFrame = true;
			}
			else
			{
				pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
				pBroke->getPage()->getScreenOffsets(pCol,col_x,col_y);
			}
			if(pBroke->getMasterTable())
			{
				if(pBroke->getMasterTable()->getFirstBrokenTable() == pBroke)
				{
					if(!bFrame)
					{
						offy = pBroke->getMasterTable()->getY();
					}
					else
					{
						offy += pBroke->getMasterTable()->getY();
					}
					if(iBot > pBroke->getYBottom())
					{
						iBot = pBroke->getYBottom();
					}
				}
				else if(bIsNested)
				{
					if(iTop < pBroke->getYBreak())
					{
						iTop = 0;
					}
					else
					{
						iTop = iTop - pBroke->getYBreak();
					}
					if(iBot > pBroke->getYBottom())
					{
						iBot = pBroke->getYBottom() - pBroke->getYBreak();
					}
					else
					{
						iBot = iBot - pBroke->getYBreak();
					}
				}
				else
				{
					offy = 0;
					if(iTop < pBroke->getYBreak())
					{
						iTop = 0;
					}
					else
					{
						iTop = iTop - pBroke->getYBreak();
					}
					if(iBot > pBroke->getYBottom())
					{
						iBot = pBroke->getYBottom() - pBroke->getYBreak();
					}
					else
					{
						iBot = iBot - pBroke->getYBreak();
					}
					xxx_UT_DEBUGMSG(("fp_Tablecontainer:: second broken table col_y %d iTop %d iBot %d m_iTop %d m_iBot %d YBreak %d yBottom %d \n",col_y,iTop,iBot,m_iTopY,m_iBotY,pBroke->getYBreak(),pBroke->getYBottom()));
				}
			}
			else
			{
				offy = pBroke->getY();
			}
			if(pBroke->getMasterTable())
			{
				offx += pBroke->getMasterTable()->getX();
			}
			else
			{
				offx += pBroke->getX();
			}
			fp_Container * pCon = pBroke;
			fp_TableContainer * pCurTab = pBroke;
			UT_sint32 iPrevCellY = 0;
			UT_sint32 iPrevTabY = pBroke->getY();
			UT_sint32 iPrevYBreak = pBroke->getYBreak();
			while(pCon->getContainer() && !pCon->getContainer()->isColumnType())
			{
				pCon = pCon->getContainer();
				offx += pCon->getX();
				UT_sint32 iycon = pCon->getY();
				offy += iycon; // this is not quite finished for nested tables!
				if(pCon->getContainerType() == FP_CONTAINER_CELL)
				{
					iPrevCellY = iycon;
				}
				if(pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					if(pCol)
					{
						pCurTab = pCol->getCorrectBrokenTable(static_cast<fp_Container *>(pCurTab));
					}
					else
					{
						pCurTab = static_cast<fp_TableContainer *>(pCon);
					}
					if(pCurTab->isThisBroken() && (pCurTab != pCurTab->getMasterTable()->getFirstBrokenTable()))
					{

						offy = offy -iycon;
					}
					if(iPrevCellY > 0 && (iPrevCellY < pCurTab->getYBreak()))
					{
						offy -= iPrevCellY;
						if((iPrevTabY > 0) && (iPrevYBreak == 0))
						{
							offy -= pCurTab->getYBreak() - iPrevCellY;
						}
					}
					else
					{
						offy -= pCurTab->getYBreak();
					}
					pCon = static_cast<fp_Container *>(pCurTab);
					iPrevYBreak = pCurTab->getYBreak();
					iPrevTabY = pCurTab->getY();
				}
			}
			col_x += offx;
			col_y += offy;
			iLeft += col_x;
			iRight += col_x;
			iTop += col_y;
			iBot += col_y;
			xxx_UT_DEBUGMSG(("fp_Tablecontainer:: Final iTop %d iBot %d \n",iTop,iBot));
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
	else
	{
		pPage = getPage();
		if(pPage)
		{
			pCol = static_cast<fp_Column *>(fp_Container::getColumn());
			pPage->getScreenOffsets(pCol,col_x,col_y);
			fp_Container * pCon = static_cast<fp_Container *>(this);
			while(!pCon->isColumnType())
			{
				col_x += pCon->getX();
				col_y += pCon->getY();
				pCon = pCon->getContainer();
			}
			if(pCon->getContainerType() != FP_CONTAINER_FRAME)
			{
				iLeft += col_x;
				iRight += col_x;
				iTop += col_y;
				iBot += col_y;
			}
			else
			{
				UT_sint32 iTmpX,iTmpY;
				pPage->getScreenOffsets(pCol,iTmpX,iTmpY);
				iLeft -= iTmpX;
				iTop -= iTmpY;
			}
		}
	}

//
// Now correct for printing
//
	if(pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		UT_sint32 xdiff,ydiff;
		pPage->getDocLayout()->getView()->getPageScreenOffsets(pPage, xdiff, ydiff);
		pPage = getPage();
		if(pPage)
		{
			if(pPage->getDocLayout()->getView()->getViewMode() != VIEW_PRINT)
			{
				ydiff -= static_cast<fl_DocSectionLayout *>(getSectionLayout()->getDocSectionLayout())->getTopMargin();
			}
		}
		iLeft -= xdiff;
		iRight -= xdiff;
		iTop -= ydiff;
		iBot -= ydiff;
	}
	bRec = UT_Rect(iLeft,iTop,iRight-iLeft,iBot-iTop);
}


/*! 
 * Returns true if the cell in a broken table overlaps the supplied clip Rect
 */
bool fp_CellContainer::doesIntersectClip(fp_TableContainer * pBroke, const UT_Rect * rClip)
{
	fp_Page * pPage = NULL;
	UT_Rect CellRect;
	_getBrokenRect(pBroke, pPage, CellRect,getGraphics());
	return CellRect.intersectsRect(rClip);
}

	
void fp_CellContainer::clearScreen(void)
{
	clearScreen(false);
}

void fp_CellContainer::clearScreen(bool bNoRecursive)
{
	fp_Container * pUpCon = getContainer();
	if(pUpCon == NULL)
	{
		return;
	}
	if(pUpCon->getY() == INITIAL_Y_POS)
	{
		return;
	}
	if(getPage() == NULL)
	{
		return;
	}
	markAsDirty();
	xxx_UT_DEBUGMSG(("Doing cell clearscreen \n"));
// only clear the embeded containers if no background is set: the background clearing will also these containers
// FIXME: should work, but doesn't??
//	if (m_iBgStyle == FS_OFF)
//	{
		fp_Container * pCon = NULL;
		UT_sint32 i = 0;
		if(!bNoRecursive)
		{
			for(i=0; i< static_cast<UT_sint32>(countCons()); i++)
			{
				pCon = static_cast<fp_Container *>(getNthCon(i));
				pCon->clearScreen();
			}
		}
	//}
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
		m_bDirty = true;
	if(pTab)
	{
		fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
		if(pBroke == NULL)
		{
			_clear(pBroke);
			return;
		}
		if(!m_bLinesDrawn)
		{
			return;
		}
		while(pBroke)
		{
			xxx_UT_DEBUGMSG(("cell:clearscreean looking at pBroke %x Ybreak %d spannedHeight %d getY %d \n",pBroke,pBroke->getYBreak(),getSpannedHeight(),getY()));
			if((getY() >= pBroke->getYBreak() && getY() < pBroke->getYBottom())
				|| ( (getY()+getSpannedHeight()) >= pBroke->getYBreak() && 
					 (getY()  < pBroke->getYBreak())))
			{
				_clear(pBroke);
				m_bLinesDrawn = true;
			}
			pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
		}
		m_bLinesDrawn = false;
	}
}

/*!
 * This method fills the broken table with containers from this cell contained
 * by the broken table and returns the offset to current Ybreak of the 
 * supplied broken
 * table required to wholly contain the first container of this cell in the
 * broken table.
 */
UT_sint32 fp_CellContainer::tweakBrokenTable(fp_TableContainer * pBroke)
{
	UT_sint32 iTop = getY();
	UT_sint32 iBot = iTop + getHeight();
	UT_sint32 iBreak = pBroke->getYBreak();
	UT_sint32 iBottom = pBroke->getYBottom();
	xxx_UT_DEBUGMSG(("Doing TweakTable on %x iTop %d iBot %d iBreak %d iBottom %d \n",pBroke,iTop,iBot, iBreak,iBottom));
	if(iBot < iBreak)
	{
		return 0;
	}
	if(iTop > iBottom)
	{
		return 0;
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	bool bFound = false;
	bool bStop = false;
	UT_sint32 iTweak =0;
	bool bIsMaster = (static_cast<fp_TableContainer *>(pBroke->getPrev()) == pBroke->getMasterTable());
	fp_TableContainer * pFirst = NULL;
	if(bIsMaster)
	{
		pFirst = pBroke->getMasterTable()->getFirstBrokenTable();
	}
	for(i=0; !bStop && (i<static_cast<UT_sint32>(countCons())); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			continue;
		}
		iTop = getY() + pCon->getY();
		UT_sint32 iBot = iTop + pCon->getHeight();
		UT_sint32 iBreak = pBroke->getYBreak();
		UT_sint32 iBottom = pBroke->getYBottom();
		bool bInBroke = (iBot >= iBreak) && (iBot < iBottom);
		if(!bFound)
		{

			if(bInBroke)
			{
				bFound = true;
				iTweak = pBroke->getYBreak() - getY() - pCon->getY();
				xxx_UT_DEBUGMSG(("Doing TweakTable on %x Tweak %d YBreak %d Cell Y %d Con Y %d \n",pBroke,iTweak,pBroke->getYBreak(),getY(), pCon->getY()));
				if((i> 0) && (iTweak>0))
				{
					pCon = static_cast<fp_Container *>(getNthCon(i-1));
					if(!bIsMaster && pBroke->getPrev())
					{
						xxx_UT_DEBUGMSG(("SetMyBrokenContainer %x \n",pBroke->getPrev()));
						pCon->setMyBrokenContainer(static_cast<fp_Container *>(pBroke->getPrev()));
					}
					else if(bIsMaster)
					{
						xxx_UT_DEBUGMSG(("SetMyBrokenContainer %x \n",pFirst));
						pCon->setMyBrokenContainer(static_cast<fp_Container *>(pFirst));
					}
				}
			}
		}
		else
		{
			if(!bInBroke)
			{
				bStop = true;
			}
		}
	}
	if(iTweak > 0)
	{
		return iTweak;
	}
	return 0;
}

UT_sint32 fp_CellContainer::getSpannedHeight(void)
{
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return 0;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getBottomAttach(),getLeftAttach());
	UT_sint32 height = 0;
	if(pCell)
	{
		height = pTab->getYOfRow(getBottomAttach()) - getY();
	}
	else
	{
		fp_CellContainer * pCell = pTab->getCellAtRowColumn(pTab->getNumRows() -1,0);
		fp_CellContainer * pMaxH = pCell;
		if(pMaxH == NULL)
		{
			return 0;
		}
		while(pCell)
		{
			if(pCell->getHeight() > pMaxH->getHeight())
			{
				pMaxH = pCell;
			}
			pCell = static_cast<fp_CellContainer *>(pCell->getNext());
		}
		height = pMaxH->getY() - getY() + pMaxH->getHeight();
	}
	return height;
}

void fp_CellContainer::_clear(fp_TableContainer * pBroke)
{
// Lookup table properties to get the line thickness, etc.

	fl_ContainerLayout * pLayout = getSectionLayout()->myContainingLayout ();
	UT_return_if_fail(pLayout);

	if(pBroke == NULL)
	{
		return;
	}
	if(pBroke->getPage() && !pBroke->getPage()->isOnScreen())
	{
		return;
	}

	UT_ASSERT(pLayout->getContainerType () == FL_CONTAINER_TABLE);
	if (pLayout->getContainerType () != FL_CONTAINER_TABLE) return;

	fl_TableLayout * pTableLayout = static_cast<fl_TableLayout *>(pLayout);

	PP_PropertyMap::Background background = getBackground ();

	PP_PropertyMap::Line lineBottom = getBottomStyle (pTableLayout);
	PP_PropertyMap::Line lineLeft   = getLeftStyle   (pTableLayout);
	PP_PropertyMap::Line lineRight  = getRightStyle  (pTableLayout);
	PP_PropertyMap::Line lineTop    = getTopStyle    (pTableLayout);

	fp_Container * pCon = getContainer();
	if(pCon->getContainer() && !pCon->getContainer()->isColumnType())
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Clearing nested cell lines \n"));
	}
	UT_Rect bRec;
	fp_Page * pPage = NULL;
	_getBrokenRect(pBroke, pPage, bRec,getGraphics());
	if((bRec.top + bRec.height) < 0)
	{
		return;
	}
	markAsDirty();
	if (pPage != NULL)
	{
		xxx_UT_DEBUGMSG(("_clear: top %d bot %d cell left %d top %d \n",bRec.top,bRec.top+bRec.height,m_iLeftAttach,m_iTopAttach));

		lineLeft.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineLeft.m_color = *getFillType()->getColor();
		_drawLine (lineLeft, bRec.left, bRec.top, bRec.left,  bRec.top + bRec.height,getGraphics());

		lineTop.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineTop.m_color =  *getFillType()->getColor();
		_drawLine (lineTop, bRec.left, bRec.top, bRec.left + bRec.width,  bRec.top,getGraphics()); 
		if(pBroke && pBroke->getPage() && pBroke->getBrokenTop() > 0)
		{
			UT_sint32 col_x,col_y;
			fp_Column * pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
			pBroke->getPage()->getScreenOffsets(pCol, col_x,col_y);
			_drawLine (lineTop, bRec.left, col_y, bRec.left + bRec.width,  col_y,getGraphics());
		}
		lineRight.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineRight.m_color =  *getFillType()->getColor();
		_drawLine (lineRight, bRec.left + bRec.width, bRec.top, bRec.left + bRec.width, bRec.top + bRec.height,getGraphics()); 
		
		lineBottom.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineBottom.m_color =  *getFillType()->getColor();
		_drawLine (lineBottom, bRec.left, bRec.top + bRec.height, bRec.left + bRec.width , bRec.top + bRec.height,getGraphics());
		xxx_UT_DEBUGMSG(("_Clear: pBroke %x \n",pBroke));
		if(pBroke && pBroke->getPage() && pBroke->getBrokenBot() >= 0)
		{
			UT_sint32 col_x,col_y;
			fp_Column * pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
			pBroke->getPage()->getScreenOffsets(pCol, col_x,col_y);
			UT_sint32 bot = col_y + pCol->getHeight();
			xxx_UT_DEBUGMSG(("_clear: Clear broken bottom %d \n",bot));
			_drawLine (lineBottom, bRec.left, bot, bRec.left + bRec.width,  bot,getGraphics());
		}
		getGraphics()->setLineWidth(1 );
		xxx_UT_DEBUGMSG(("_clear: BRec.top %d  Brec.height %d \n",bRec.top,bRec.height));
		//			UT_ASSERT((bRec.left + bRec.width) < getPage()->getWidth());
		UT_sint32 srcX = 0;
		UT_sint32 srcY = 0;
		getFillType()->setWidthHeight(getGraphics(),bRec.width,bRec.height);
		getLeftTopOffsets(srcX,srcY);
		if(getFillType()->getParent())
		{
			srcX += getX();
			srcY += getY();
			getFillType()->getParent()->Fill(getGraphics(),srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
		}
		else
		{
			getFillType()->Fill(getGraphics(),srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
		}
		if(getPage())
		{
			getPage()->expandDamageRect(bRec.left,bRec.top,bRec.width,bRec.height);
		}
	}
	m_bDirty = true;
	m_bBgDirty = true;
	m_bLinesDrawn = false;
}

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_CellContainer::setWidth(UT_sint32 iWidth)
{
	UT_sint32 myWidth = getWidth();
	if (iWidth == myWidth)
	{
		return;
	}
	if(iWidth < 2)
	{
		iWidth = 2;
	}
	clearScreen();
	fp_VerticalContainer::setWidth(iWidth);
	fl_SectionLayout * pSL = getSectionLayout();
	pSL = static_cast<fl_SectionLayout *>(pSL->myContainingLayout());
	UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_TABLE);
	static_cast<fl_TableLayout *>(pSL)->setDirty();
	fl_CellLayout * pCellL = static_cast<fl_CellLayout *>(getSectionLayout());
	xxx_UT_DEBUGMSG(("Set Reformat 2 now from table %x in TableLayout %x \n",this,getSectionLayout()));
	pCellL->setNeedsReformat(pCellL);
	pCellL->_localCollapse();
	pCellL->format();
	UT_sint32 i = 0;
	for(i =0; i< static_cast<UT_sint32>(countCons()); i++)
	{
		fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->layout();
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			static_cast<fp_TableContainer *>(pCon)->layout();
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}
	
void fp_CellContainer::setContainer(fp_Container * pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer())
	{
		clearScreen();
	}
	fp_Container::setContainer(pContainer);
	if(pContainer == NULL)
	{
		return;
	}
	UT_ASSERT(pContainer->getContainerType() == FP_CONTAINER_TABLE);
	fp_TableContainer * pTable = static_cast<fp_TableContainer *>(pContainer);
	UT_sint32 iWidth = pTable->getWidth();

	fp_CellContainer::setWidth(iWidth);
}

/* just a little helper function
 */
void fp_CellContainer::_drawLine (const PP_PropertyMap::Line & style,
								  UT_sint32 left, UT_sint32 top, UT_sint32 right, UT_sint32 bot,GR_Graphics * pGr)
{

	if (style.m_t_linestyle == PP_PropertyMap::linestyle_none &&
		!pGr->queryProperties(GR_Graphics::DGP_SCREEN))
		return; // do not draw the dotted line when printing	
	
	GR_Graphics::JoinStyle js = GR_Graphics::JOIN_MITER;
	GR_Graphics::CapStyle  cs = GR_Graphics::CAP_PROJECTING;

	switch (style.m_t_linestyle)
	{
		case PP_PropertyMap::linestyle_none:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_DOTTED);
			break;
		case PP_PropertyMap::linestyle_dotted:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_DOTTED);
			break;
		case PP_PropertyMap::linestyle_dashed:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_ON_OFF_DASH);
			break;
		case PP_PropertyMap::linestyle_solid:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
			break;
		default: // do nothing; shouldn't happen
			break;
	}

	pGr->setLineWidth (static_cast<UT_sint32>(style.m_thickness));
	if (style.m_t_linestyle == PP_PropertyMap::linestyle_none)
	{
		pGr->setColor (m_borderColorNone);
	}
	else
	{
		pGr->setColor (style.m_color);
	}

	xxx_UT_DEBUGMSG(("_drawLine: top %d bot %d \n",top,bot));

	GR_Painter painter(pGr);

	painter.drawLine (left, top, right, bot);
	
	pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
}

PP_PropertyMap::Background fp_CellContainer::getBackground () const
{
	PP_PropertyMap::Background background(m_background);

	fl_ContainerLayout * pLayout = getSectionLayout()->myContainingLayout ();
	UT_return_val_if_fail(pLayout, background);
	UT_return_val_if_fail(pLayout->getContainerType () == FL_CONTAINER_TABLE, background);

	fl_TableLayout * table = static_cast<fl_TableLayout *>(pLayout);

	const PP_PropertyMap::Background & table_background = table->getBackground ();

	/* (unclear how "inherit" differs from "transparent")
	 */
	if (background.m_t_background != PP_PropertyMap::background_solid)
		{
			background.m_t_background = table_background.m_t_background;
			if (background.m_t_background == PP_PropertyMap::background_solid)
				background.m_color = table_background.m_color;
		}
	if ((background.m_t_background == PP_PropertyMap::background_inherit) ||
		(background.m_t_background == PP_PropertyMap::background__unset))
		{
			background.m_t_background = PP_PropertyMap::background_none;
		}

	return background;
}

/* sort out inheritance and defaults of cell-border properties
 */
void s_cell_border_style (PP_PropertyMap::Line & line, const PP_PropertyMap::Line & table_line,
						  const fl_TableLayout * table)
{
	if (line.m_t_color == PP_PropertyMap::color_inherit)
		{
			line.m_t_color = table_line.m_t_color;
			if (line.m_t_color == PP_PropertyMap::color_color)
				line.m_color = table_line.m_color;
		}
	if ((line.m_t_color == PP_PropertyMap::color_inherit) ||
		(line.m_t_color == PP_PropertyMap::color__unset))
		{
			line.m_t_color = PP_PropertyMap::color_color;
			line.m_color = table->getDefaultColor ();
		}

	if (line.m_t_linestyle == PP_PropertyMap::linestyle_inherit)
		line.m_t_linestyle = table_line.m_t_linestyle;
	if ((line.m_t_linestyle == PP_PropertyMap::linestyle_inherit) ||
		(line.m_t_linestyle == PP_PropertyMap::linestyle__unset))
		line.m_t_linestyle = PP_PropertyMap::linestyle_solid;

	if (line.m_t_thickness == PP_PropertyMap::thickness_inherit)
		{
			line.m_t_thickness = table_line.m_t_thickness;
			if (line.m_t_thickness == PP_PropertyMap::thickness_length)
				line.m_thickness = table_line.m_thickness;
		}
	if ((line.m_t_thickness == PP_PropertyMap::thickness_inherit) ||
		(line.m_t_thickness == PP_PropertyMap::thickness__unset))
		{
			line.m_t_thickness = table_line.m_t_thickness;
			UT_sint32 defaultThickness = table->getLineThickness ();
			line.m_thickness = (defaultThickness > 0) ? static_cast<UT_uint32>(defaultThickness) : 0;
		}

	/* if the color is transparent or the thickness is 0, then set the line-style to none
	 */
	if ((line.m_thickness == 0) || (line.m_t_color == PP_PropertyMap::color_transparent))
		{
			line.m_t_linestyle = PP_PropertyMap::linestyle_none;
		}
}

PP_PropertyMap::Line fp_CellContainer::getBottomStyle (const fl_TableLayout * table) const
{
	PP_PropertyMap::Line line(m_lineBottom);
	if (table == 0) return line;
	const PP_PropertyMap::Line & table_line = table->getBottomStyle ();
	s_cell_border_style (line, table_line, table);
	return line;
}

PP_PropertyMap::Line fp_CellContainer::getLeftStyle (const fl_TableLayout * table) const
{
	PP_PropertyMap::Line line(m_lineLeft);
	if (table == 0) return line;
	const PP_PropertyMap::Line & table_line = table->getLeftStyle ();
	s_cell_border_style (line, table_line, table);
	return line;
}

PP_PropertyMap::Line fp_CellContainer::getRightStyle (const fl_TableLayout * table) const
{
	PP_PropertyMap::Line line(m_lineRight);
	if (table == 0) return line;
	const PP_PropertyMap::Line & table_line = table->getRightStyle ();
	s_cell_border_style (line, table_line, table);
	return line;
}

PP_PropertyMap::Line fp_CellContainer::getTopStyle (const fl_TableLayout * table) const
{
	PP_PropertyMap::Line line(m_lineTop);
	if (table == 0) return line;
	const PP_PropertyMap::Line & table_line = table->getTopStyle ();
	s_cell_border_style (line, table_line, table);
	return line;
}

bool fp_CellContainer::isInNestedTable(void)
{
	fp_TableContainer * pMaster = static_cast<fp_TableContainer *>(getContainer());
	fp_CellContainer * pTopCell = static_cast<fp_CellContainer *>(this);
	UT_sint32 icount = 0;
	while(pMaster && pMaster->getContainer() && !pMaster->getContainer()->isColumnType())
	{
		pTopCell = static_cast<fp_CellContainer *>(pMaster->getContainer());
		pMaster = static_cast<fp_TableContainer *>(pTopCell->getContainer());
		icount++;
	}
	if(icount < 1)
	{
		return false;
	}
	return true;
}

void fp_CellContainer::setBackground (const PP_PropertyMap::Background & style)
{
	m_background = style;
	PP_PropertyMap::Background background = getBackground ();
	if(background.m_t_background == PP_PropertyMap::background_solid)
	{
		getFillType()->setColor(background.m_color);
	}
}

/*!
 * Given the broken table that contains this cell, calculate the positions
 * of the left,right,top and bottom edges of the cell.
 */
void fp_CellContainer::getScreenPositions(fp_TableContainer * pBroke,GR_Graphics * pG, UT_sint32 & iLeft, UT_sint32 & iRight,UT_sint32 & iTop,UT_sint32 & iBot, UT_sint32 & col_y, fp_Column *& pCol, fp_ShadowContainer *& pShadow, bool & bDoClear )
{
	UT_return_if_fail(getPage());

	bool bNested = false;
	if(pBroke == NULL)
	{
		pBroke = static_cast<fp_TableContainer *>(getContainer());
	}
	if(isInNestedTable())
	{ 
		bNested = true;
	}
	if(pBroke && pBroke->getPage())
	{
		if(pG->queryProperties(GR_Graphics::DGP_SCREEN) && !pBroke->getPage()->isOnScreen())
		{
			return;
		}
	}
//
// Now correct if iTop or iBot is off the page.
//
	UT_sint32 col_x;
	UT_sint32 offy =0;
	UT_sint32 offx =0;
	fp_Page * pPage = pBroke->getPage(); 
	if(pPage == NULL)
	{
//
// Can happen while loading.
//
		return;
	}

	pPage = pBroke->getPage(); 
	if(getContainer()->getContainerType() == FP_CONTAINER_FRAME)
	{
		fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(getContainer());
		getView()->getPageScreenOffsets(pPage,col_x,col_y);
		col_x += pFC->getX();
		col_y += pFC->getY();
		pCol = static_cast<fp_Column *>(pFC->getColumn());
	}
	else if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		pShadow =  static_cast<fp_ShadowContainer *>(pBroke->getContainer());
		if(pShadow)
		{
			pShadow->getPage()->getScreenOffsets(pShadow, col_x,col_y);
		}
		else
		{
			pPage->getScreenOffsets(pShadow, col_x,col_y);
		}

	}
	else if(pBroke && (pBroke->getBrokenColumn()->getContainerType()  == FP_CONTAINER_COLUMN_SHADOW))
	{
		pShadow =  static_cast<fp_ShadowContainer *>(pBroke->getContainer());
		if(pShadow)
		{
			pShadow->getPage()->getScreenOffsets(pShadow, col_x,col_y);
		}
		else
		{
			pPage->getScreenOffsets(pShadow, col_x,col_y);
		}

	}
	else
	{
		pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
		if(pCol)
		{
			pCol->getPage()->getScreenOffsets(pCol, col_x,col_y);
		}
		else
		{
			pPage->getScreenOffsets(pCol, col_x,col_y);
		}
	}
	bDoClear = true;
	if(pPage->getDocLayout()->getView() && pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
//
// Now correct for printing
//
		bDoClear = false;
		UT_sint32 xdiff,ydiff;
		pPage->getDocLayout()->getView()->getPageScreenOffsets(pPage, xdiff, ydiff);
		col_y = col_y - ydiff;
		col_x = col_x - xdiff;
		if(pPage->getDocLayout()->getView()->getViewMode() != VIEW_PRINT)
		{
			col_y += static_cast<fl_DocSectionLayout *>(getSectionLayout()->getDocSectionLayout())->getTopMargin();
		}
	}
	if(pBroke->getMasterTable())
	{
		offx = pBroke->getMasterTable()->getX();
		if(pBroke->getMasterTable()->getFirstBrokenTable() == pBroke)
		{
			offy = pBroke->getMasterTable()->getY();
		}
		else
		{
			offy = 0;
		}
	}
	if(bNested && pBroke)
	{
		fp_Container * pCon = static_cast<fp_Container *>(pBroke->getContainer());
		fp_TableContainer * pCurTab = pBroke;
		while(!pCon->isColumnType())
		{
			UT_sint32 iycon = pCon->getY();
			offy += iycon;
			offx += pCon->getX();
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				if(pCol)
				{
					pCurTab = pCol->getCorrectBrokenTable(static_cast<fp_Container *>(pCurTab));
				}
				else
				{
					pCurTab = static_cast<fp_TableContainer *>(pCon);
			    }
				if(pCurTab->isThisBroken() && (pCurTab != pCurTab->getMasterTable()->getFirstBrokenTable()))
				{

					offy = offy + pCurTab->getY() - iycon;
				}
				if(offy < pCurTab->getYBreak())
				{
					offy = 0;
				}
				else
				{
					offy -= pCurTab->getYBreak();
				}
			}
			pCon = pCon->getContainer();
		}
	}
	iLeft = col_x + m_iLeft + offx;
	iRight = col_x + m_iRight + offx;
	iTop = col_y + m_iTopY + offy;
	iBot = col_y + m_iBotY + offy;
}
/*!
 * Draw background and lines around a cell in a broken table.
 */
void fp_CellContainer::drawLines(fp_TableContainer * pBroke,GR_Graphics * pG)
{
	UT_return_if_fail(getPage());

	bool bNested = false;
	if(pBroke == NULL)
	{
		pBroke = static_cast<fp_TableContainer *>(getContainer());
	}
	if(isInNestedTable())
	{ 
		bNested = true;
	}
	if(pBroke && pBroke->getPage())
	{
		if(pG->queryProperties(GR_Graphics::DGP_SCREEN) && !pBroke->getPage()->isOnScreen())
		{
			return;
		}
	}
// Lookup table properties to get the line thickness, etc.

	fl_ContainerLayout * pLayout = getSectionLayout()->myContainingLayout ();
	UT_return_if_fail(pLayout->getContainerType () == FL_CONTAINER_TABLE);

	fl_TableLayout * pTableLayout = static_cast<fl_TableLayout *>(pLayout);

	PP_PropertyMap::Line lineBottom = getBottomStyle (pTableLayout);
	PP_PropertyMap::Line lineLeft   = getLeftStyle   (pTableLayout);
	PP_PropertyMap::Line lineRight  = getRightStyle  (pTableLayout);
	PP_PropertyMap::Line lineTop    = getTopStyle    (pTableLayout);
	fp_Page * pPage = pBroke->getPage();
	
	if(pPage == NULL)
	{
//
// Can happen while loading.
//
		return;
	}
	bool bDrawTop = true;
	bool bDrawBot = true;
	xxx_UT_DEBUGMSG(("m_iBotY %d \n",m_iBotY));
	m_bLinesDrawn = true;
	UT_sint32 iLeft,iRight,iTop,iBot = 0;
	UT_sint32 col_y = 0;
	fp_Column * pCol = NULL;
	fp_ShadowContainer * pShadow = NULL;
	bool bDoClear = true;
	getScreenPositions(pBroke,pG,iLeft,iRight,iTop,iBot,col_y,pCol,pShadow,bDoClear );
	if(pBroke != NULL)
	{
		if(m_iBotY < pBroke->getYBreak())
		{
//
// Cell is above this page
//
			xxx_UT_DEBUGMSG(("Don't drawlines because M-IBotY < pBroke->getYbreak \n",m_iBotY,pBroke->getYBreak()));
			return;
		}
		if(m_iTopY > pBroke->getYBottom())
		{
//
// Cell is below this page
//
			xxx_UT_DEBUGMSG(("Don't drawlines because m_iTopY > pBroke->getYBottom \n",m_iTopY, pBroke->getYBottom()));
			return;
		}
		iTop -= pBroke->getYBreak();
		iBot -= pBroke->getYBreak();
		xxx_UT_DEBUGMSG(("drawLines: ibot = %d col_y %d m_iBotY %d pCol->getHeight() %d left %d top %d \n",iBot,col_y,m_iBotY,pCol->getHeight(),m_iLeftAttach,m_iTopAttach));
		if(iTop < col_y)
		{
			xxx_UT_DEBUGMSG(("iTop < col_y !! iTop %d col_y %d row is %d \n",iTop,col_y,getTopAttach()));
			iTop = col_y;
			bDrawTop = true;
			if(pBroke != NULL)
			{
				pBroke->setBrokenTop(1);
			}
		}
		xxx_UT_DEBUGMSG(("drawlines: After iTop %d iBot = %d  sum %d left %d top %d  \n",iTop,iBot,col_y + pCol->getHeight(),m_iLeftAttach,m_iTopAttach));
		UT_sint32 iColHeight = 0;
		if(pCol)
		{
			iColHeight = pCol->getHeight();
		}
		else if(pShadow)
		{
			iColHeight = pShadow->getHeight();
		}

		if(iBot > col_y + iColHeight)
		{
   			iBot =  col_y + iColHeight;
			bDrawBot = true;
			if(pBroke != NULL)
			{
				pBroke->setBrokenBot(1);
			}
		}
		//
		// Now get a rectangle to calculate draw arguments
		//
		// This code might eventually replace a lot of the code above but
		// it needs more testing and tweaking - particularly for nested tables.
		//
// 		UT_Rect bRec;
// 		fp_Page * pLinePage;
// 		_getBrokenRect(pBroke, pLinePage, bRec);
// 		if(pLinePage != pPage)
// 		{
// 			UT_DEBUGMSG(("Pages don't match! \n"));
// 			return;
// 		}
// 		iLeft = bRec.left;
// 		iTop = bRec.top;
// 		iBot = iTop + bRec.height;
// 		iRight = iLeft + bRec.width;
		m_bDrawRight = true;

		//
		// Have to draw white first because drawing is additive
		//
		PP_PropertyMap::Line clineBottom = getBottomStyle (pTableLayout);
		PP_PropertyMap::Line clineLeft   = getLeftStyle   (pTableLayout);
		PP_PropertyMap::Line clineRight  = getRightStyle  (pTableLayout);
		PP_PropertyMap::Line clineTop    = getTopStyle    (pTableLayout);
		UT_RGBColor white(255,255,255);
		if (m_bDrawLeft)
		{
			if(bDoClear)
			{
				clineLeft.m_color = white;
				_drawLine (clineLeft, iLeft, iTop, iLeft,  iBot,pG);
			}
			_drawLine(lineLeft, iLeft, iTop, iLeft, iBot,pG);
		}
		if(m_bDrawTop || bDrawTop)
		{
			if(bDoClear)
			{
				clineTop.m_color = white;
				_drawLine(clineTop, iLeft, iTop, iRight, iTop,pG);
			}
			_drawLine(lineTop, iLeft, iTop, iRight, iTop,pG);
		}
		if(m_bDrawRight)
		{
			if(bDoClear)
			{
				clineRight.m_color = white;
				_drawLine(clineRight, iRight, iTop, iRight, iBot,pG);
			}
			_drawLine(lineRight, iRight, iTop, iRight, iBot,pG);
		}
		if(m_bDrawBot || bDrawBot)
		{
			if(bDoClear)
			{
				clineBottom.m_color = white;
				_drawLine(clineBottom, iLeft, iBot, iRight, iBot,pG);
			}
			_drawLine(lineBottom, iLeft, iBot, iRight, iBot,pG);
		}
	}
}

/*!
 * Draw lines around neighbouring cells. Use to fix artifacts of editting.
 */
void fp_CellContainer::drawLinesAdjacent(void)
{
	UT_sint32 row = getTopAttach();
	UT_sint32 col_right = getRightAttach();
	UT_sint32 col_left = getLeftAttach();
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	bool bDoRight = false;
	if(col_right < pTab->getNumCols())
	{
		bDoRight = true;
	}
	bool bDoLeft = false;
	if(col_left >= 0)
	{
		bDoLeft = true;
	}
	fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
	while(pBroke)
	{
		drawLines(pBroke,getGraphics());
		if(bDoRight)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_right);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics());
			}
		}
		if(bDoLeft)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_left);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics());
			}
		}
		pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	}
}

	
/*!
 Draw container outline
 \param pDA Draw arguments
 \param pBroke fp_TableContainer pointer to broken table
 */
void fp_CellContainer::_drawBoundaries(dg_DrawArgs* pDA, fp_TableContainer * pBroke)
{
	UT_return_if_fail(getPage());

	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
	if(pBroke && pBroke->getPage())
	{
		if(pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN) && !pBroke->getPage()->isOnScreen())
		{
			return;
		}
		if(pBroke->getYBreak() > (getY() + getHeight()))
		{
			return;
		}
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff + getX();
        UT_sint32 yoffBegin = pDA->yoff + getY();
        UT_sint32 xoffEnd = pDA->xoff + getX() + getWidth() - getGraphics()->tlu(1);
        UT_sint32 yoffEnd = pDA->yoff + getY() + getHeight() - getGraphics()->tlu(1);

		UT_RGBColor clrShowPara(127,127,127);

		GR_Painter painter(getGraphics());

		getGraphics()->setColor(clrShowPara);
		xxx_UT_DEBUGMSG(("SEVIOR: cell boundaries xleft %d xright %d ytop %d ybot %d \n",xoffBegin,xoffEnd,yoffBegin,yoffEnd));
        painter.drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        painter.drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        painter.drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        painter.drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 * Return the topmost table in this structure. The one embedded in the 
 * column.
 */
fp_TableContainer * fp_CellContainer::getTopmostTable() const
{
	fp_Container * pUp = getContainer();
	fp_Container * pPrev = pUp;
	while(pUp->getContainerType() != FP_CONTAINER_COLUMN)
	{
		pPrev = pUp;
		pUp = pUp->getContainer();
	}
	if(pPrev->getContainerType() == FP_CONTAINER_TABLE)
	{
		return static_cast<fp_TableContainer *>(pPrev);
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}
/*!
 * Return true if the cell contains footnote references
 */
bool fp_CellContainer::containsFootnoteReference(void)
{
	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	while(pCon && !bFound)
	{
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			fp_Line * pLine = static_cast<fp_Line *>(pCon);
			if(pLine->containsFootnoteReference())
			{
				bFound = true;
			}
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
			if(pTab->containsFootnoteReference())
			{
				bFound = true;
			}
		}
		pCon = static_cast<fp_Container *>(pCon->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of all the footnote layouts in this cell
 */
bool fp_CellContainer::getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pVecFoots)
{
	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	while(pCon)
	{
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			fp_Line * pLine = static_cast<fp_Line *>(pCon);
			if(pLine->containsFootnoteReference())
			{
				bFound = true;
				UT_GenericVector<fp_FootnoteContainer*> vecFC;
				pLine->getFootnoteContainers(&vecFC);
				UT_uint32 i =0;
				for(i=0; i< vecFC.getItemCount();i++)
				{
					pVecFoots->addItem(vecFC.getNthItem(i));
				}
			}
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
			if(pTab->containsFootnoteReference())
			{
				bFound = true;
				UT_GenericVector<fp_FootnoteContainer*> vecFC;
				pTab->getFootnoteContainers(&vecFC);
				UT_uint32 i =0;
				for(i=0; i< vecFC.getItemCount();i++)
				{
					pVecFoots->addItem(vecFC.getNthItem(i));
				}
			}
		}
 		pCon = static_cast<fp_Container *>(pCon->getNext());
	}
	return bFound;
}

/*!
 * Return the x coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellX(fp_Line * pLine) const
{
	return 0;
}

/*!
 * Return the y coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellY(fp_Line * pLine) const
{
	fp_TableContainer * pTab = getTopmostTable();
	return pTab->getY();
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_CellContainer::draw(dg_DrawArgs* pDA)
{
	m_bDrawTop = false;
	GR_Graphics * pG = pDA->pG;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
// draw bottom if this cell is the last of the table and fully contained on the page

	m_bDrawBot = (pTab->getNumRows() == getBottomAttach());

	m_bDrawLeft = true;

	UT_sint32 count = countCons();
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop,ybot;
	UT_sint32 i;
	UT_sint32 imax = static_cast<UT_sint32>((static_cast<UT_uint32>(1<<31)) - 1);
	if(pClipRect)
	{
		ybot = UT_MAX(pClipRect->height,_getMaxContainerHeight());
		ytop = pClipRect->top;
        ybot += ytop + 1;
		xxx_UT_DEBUGMSG(("SEVIOR: clip top %d clip bot %d \n",ytop,ybot));
	}
	else
	{
		ytop = 0;
		ybot = imax;
	}
	bool bStop = false;
	bool bStart = false;
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing unbroken cell %x x %d, y %d width %d height %d \n",this,getX(),getY(),getWidth(),getHeight()));
	
//
// Only draw the lines in the clipping region.
//
	for ( i = 0; (i<count && !bStop); i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
		dg_DrawArgs da = *pDA;
//
// pDA has xoff set at the columns left edge, we need to offset this
//     by the cell x position.
// pDA has yoffset at the last ypos in the column relative to the screen
//     The position Ypos is the absolute position on the screen we need
//     to offset this with the position of the container holding this
//     cell.

		da.xoff += pContainer->getX() + getX();
		da.yoff += pContainer->getY() + getY();
		UT_sint32 ydiff = da.yoff + pContainer->getHeight();
		if((da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
		{
//
// Always draw the top of the cell.
//
			m_bDrawTop = true;
			bStart = true;
			pContainer->draw(&da);
		}
		else if(bStart)
		{
			bStop = true;
		}
	}
	if(i == count)
	{
		m_bDirty = false;
	}
	drawLines(NULL,pG);
	pTab->setRedrawLines();
    _drawBoundaries(pDA,NULL);
}


/*!
 * Draw the whole cell with the selection colour background.
\Param pLine pointer to the line contained within the cell the cell.
*/
void fp_CellContainer::draw(fp_Line * pLine)
{
	UT_return_if_fail(getPage());

	m_bDirty = false;
	FV_View * pView = getView();
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	UT_ASSERT(pTab->getContainerType() == FP_CONTAINER_TABLE);
	fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
	if(pBroke == NULL)
	{
		return;
	}
	bool bFound = false;

	while(pBroke && !bFound)
	{
		if(pBroke->isInBrokenTable(this,pLine))
		{
			bFound = true;
			break;
		}
		pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	}
	if(!bFound)
	{
		return;
	}
	fp_Container * pLast = static_cast<fp_Container *>(pLine);
	while(pLast->getNext() && pBroke->isInBrokenTable(this,pLast))
	{
		pLast = static_cast<fp_Container*>(pLast->getNext());
	}
	//
	// Now get a rectangle to calculate draw arguments
	//
	UT_Rect bRec;
	fp_Page * pLinePage;
	_getBrokenRect(pBroke, pLinePage, bRec,getGraphics());
	dg_DrawArgs da;
	UT_sint32 xoff,yoff;
	fp_Container * pCon = static_cast<fp_Container *>(this);
	pView->getPageScreenOffsets(pLinePage,xoff,yoff);
	//
	// Just need to get the offsets of the container that contains 
	// this table.
	//
	pCon = pCon->getContainer();
	while(pCon && !pCon->isColumnType())
	{
		xoff += pCon->getX();
		yoff += pCon->getY();
		pCon = pCon->getContainer();
	}
	if(pCon)
	{
		xoff += pCon->getX();
		yoff += pCon->getY();
	}
	if(getY() < pBroke->getYBreak())
	{
		//
		// Have to account for the bit of the cell on the previous page
		//
		da.yoff = yoff - pBroke->getYBreak();
	}
	da.xoff = xoff;
	da.yoff = yoff;
	da.bDirtyRunsOnly = false;
	da.pG = pView->getGraphics();
	drawBroken(&da,pBroke);
	return ;
}

/*!
 * Deletes any broken tables in this cell.
 */
void fp_CellContainer::deleteBrokenTables(bool bClearFirst)
{
	if(!containsNestedTables())
	{
		return;
	}
	fl_CellLayout * pCell = static_cast<fl_CellLayout *>(getSectionLayout());
	fl_ContainerLayout * pCL = pCell->getFirstLayout();
	while(pCL)
	{
		UT_DEBUGMSG(("Looking at CL %x for delete \n",pCL));
		if(pCL->getContainerType() == FL_CONTAINER_TABLE)
		{
			fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCL);
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pTL->getFirstContainer());
			xxx_UT_DEBUGMSG(("Doing delete broken tables on %x \n",pTab));
			if(pTab)
			{
				pTab->deleteBrokenTables(bClearFirst,false);
			}
		}
		pCL = pCL->getNext();
	}
}


/*!
 * Draw the whole cell with the selection colour background.
 * returns the last drawn cell in the container or NULL
\Param pLine pointer to the line contained within the cell the cell.
*/
fp_Container * fp_CellContainer::drawSelectedCell(fp_Line * pLine)
{
	UT_return_val_if_fail(getPage(), NULL);

	FV_View * pView = getPage()->getDocLayout()->getView();
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return NULL;
	}
	UT_ASSERT(pTab->getContainerType() == FP_CONTAINER_TABLE);
	fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
	if(pBroke == NULL)
	{
		return NULL;
	}
	bool bFound = false;
	bool bEnd = false;
	UT_sint32 iBroke = 0;
	while(pBroke && !bEnd)
	{
		if(doesOverlapBrokenTable(pBroke))
		{
			bFound = true;
			//
			// Set a flag to indicate the cell is selected
			//
			m_bIsSelected = true;
			//
			// Now get a rectangle to calculate draw arguments
			//
			UT_Rect bRec;
			fp_Page * pLinePage;
			_getBrokenRect(pBroke, pLinePage, bRec,getGraphics());
			dg_DrawArgs da;
			UT_sint32 xoff,yoff;
			fp_Container * pCon = static_cast<fp_Container *>(this);
			pView->getPageScreenOffsets(pLinePage,xoff,yoff);
			//
			// Just need to get the offsets of the container that contains 
			// this table.
			//
			pCon = static_cast<fp_Container *>(pBroke);
			fp_TableContainer * pMaster = pBroke->getMasterTable();
			if(pMaster->getFirstBrokenTable() == pBroke)
			{
				pCon = static_cast<fp_Container *>(pBroke->getMasterTable());
			}
			while(pCon && !pCon->isColumnType())
			{
				xoff += pCon->getX();
				yoff += pCon->getY();
				pCon = pCon->getContainer();
			}
			if(pCon)
			{
				xoff += pCon->getX();
				yoff += pCon->getY();
			}
			yoff -= pBroke->getYBreak();
			da.xoff = xoff;
   			da.yoff = yoff;
			da.bDirtyRunsOnly = false;
			da.pG = pView->getGraphics();
			drawBroken(&da,pBroke);
			m_bBgDirty = true;
		}
		else if(bFound)
		{
			bEnd = true;
		}
		iBroke++;
		pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	}
	fp_Container * pLast = NULL;
	if(getNext())
	{
		pLast = static_cast<fp_Container *>(static_cast<fp_Container *>(getNext())->getNthCon(0));
		while(pLast && pLast->getContainerType() !=FP_CONTAINER_LINE)
		{
			pLast = static_cast<fp_Container *>(pLast->getNthCon(0));
		}
	}
	else
	{
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
		pCL = pCL->getNext();
		if(pCL)
		{
			pLast = pCL->getFirstContainer();
			while(pLast && pLast->getContainerType() !=FP_CONTAINER_LINE)
			{
				pLast = static_cast<fp_Container *>(pLast->getNthCon(0));
			}
		}
	}
	return pLast;
}

/*!
 * This method returns true if the cell overlaps the supplied broken
 * table.
 */
bool fp_CellContainer::doesOverlapBrokenTable(fp_TableContainer * pBroke)
{
	UT_sint32 nextRow = m_iBottomAttach;
	UT_sint32 yCellBot = 0;
	if(nextRow <= pBroke->getMasterTable()->getNumRows())
	{
		yCellBot = pBroke->getMasterTable()->getYOfRow(nextRow);
	}
	else
	{
		yCellBot = pBroke->getMasterTable()->getY() + pBroke->getMasterTable()->getHeight();
	}
	if((pBroke->getYBreak() <= getY()) && (getY() <= pBroke->getYBottom()))
	{
		return true;
	}
	if((pBroke->getYBreak() < yCellBot) && (yCellBot <= pBroke->getYBottom()))
	{
		return true;
	}
	//
	// The broken table is containtained within this cell. 
	// ie The cell spans several pages.
	//
	if((pBroke->getYBreak() >= getY()) && (yCellBot >= pBroke->getYBottom()))
	{
		return true;
	}
	return false;
}

/*!
 Draw container content visible within the supplied broken table
 \param pDA Draw arguments
 \param pBroke broken table that cell is part of
 */
void fp_CellContainer::drawBroken(dg_DrawArgs* pDA,
								  fp_TableContainer * pBroke)
{
	GR_Graphics * pG = pDA->pG;
	m_bDrawLeft = false;
	m_bDrawTop = false;
	fp_TableContainer * pTab = NULL;
	bool bIsNested = isInNestedTable();
	if(pBroke && pBroke->isThisBroken())
	{
		pTab = pBroke->getMasterTable();
	}
	else
	{
		pTab = static_cast<fp_TableContainer *>(getContainer());
	}
// draw bottom if this cell is the last of the table and fully contained on the page

	m_bDrawBot = (pTab->getCellAtRowColumn(getBottomAttach(),getLeftAttach()) == NULL);

// draw right if this cell is the rightmost of the table

	m_bDrawRight = (pTab->getCellAtRowColumn(getTopAttach(),getRightAttach()) == NULL);
	m_bDrawRight = true;
	m_bDrawLeft = true;
   
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop,ybot;
	UT_sint32 i;
	UT_sint32 imax = static_cast<UT_sint32>((static_cast<UT_uint32>(1<<29)) - 1);
	UT_Rect bRec;
	fp_Page * pPage;
	_getBrokenRect(pBroke, pPage, bRec,pG);
	if(getFillType()->getFillType() == FG_FILL_IMAGE && (getContainer() != NULL))
	{
		fl_DocSectionLayout * pDSL = getSectionLayout()->getDocSectionLayout();
		if(pDSL && (bRec.height < pDSL->getActualColumnHeight()) && (bRec.height > pG->tlu(3)))
		{
			getSectionLayout()->setImageHeight(bRec.height);
			getSectionLayout()->setImageWidth(bRec.width);
			getFillType()->setWidthHeight(pG,bRec.width,bRec.height,true);
		}
	}

	if(pClipRect)
	{
		ybot = UT_MAX(pClipRect->height,_getMaxContainerHeight());
		ytop = pClipRect->top;
		ybot = ybot + ytop + pG->tlu(1);
	}
	else
	{
		ytop = 0;
		ybot = imax;
	}
	xxx_UT_DEBUGMSG(("cliprect %x ytop %d ybot %d \n",pClipRect,ytop,ybot));
	bool bStop = false;
	bool bStart = false;
	xxx_UT_DEBUGMSG(("drawBroken: Drawing broken cell %x x %d, y %d width %d height %d ncons %d \n",this,getX(),getY(),getWidth(),getHeight(),countCons()));

//
// Now draw the cell background.
//

	GR_Painter painter(pG);

	if (((m_bIsSelected == false) || (!pG->queryProperties(GR_Graphics::DGP_SCREEN))) && (m_bBgDirty || !pDA->bDirtyRunsOnly))
	{
		UT_sint32 srcX = 0;
		UT_sint32 srcY = 0;
		getFillType()->setWidthHeight(pG,bRec.width,bRec.height);
		getLeftTopOffsets(srcX,srcY);
		getFillType()->Fill(pG,srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
		if(getPage())
		{
			getPage()->expandDamageRect(bRec.left,bRec.top,bRec.width,bRec.height);
		}
		m_bBgDirty = false;
	}
	//
	// This cell is selected, fill it with colour
	//
	else if(m_bIsSelected && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		xxx_UT_DEBUGMSG(("drawBroke: fill rect: Final top %d bot %d  pBroke %x \n",bRec.top,bRec.top + bRec.height,pBroke));
			UT_ASSERT((bRec.left + bRec.width) < getPage()->getWidth());
		FV_View * pView = getPage()->getDocLayout()->getView();
		if(bIsNested)
		{
			xxx_UT_DEBUGMSG(("Drawing nested selected cell top %d height %d \n",bRec.top,bRec.height));
		}
		painter.fillRect(pView->getColorSelBackground(),bRec.left,bRec.top,bRec.width,bRec.height);
		if(getPage())
		{
			getPage()->expandDamageRect(bRec.left,bRec.top,bRec.width,bRec.height);
		}
	}

//
// Only draw the lines in the clipping region.
//
	xxx_UT_DEBUGMSG(("number containers %d \n",countCons()));
	UT_sint32 iLastDraw = 0;
	for ( i = 0; (i< static_cast<UT_sint32>(countCons()) && !bStop); i++)
	{
		fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));
		if(pBroke->isInBrokenTable(this, pContainer))
		{
			dg_DrawArgs da = *pDA;
//
// pDA has xoff set at the columns left edge, we need to offset this
//     by the cell x position.
// pDA has yoffset at the last ypos in the column relative to the screen
//     The position Ypos is the absolute position on the screen we need
//     to offset this with the position of the container holding this
//     cell.

			da.xoff += pContainer->getX() + getX();
			da.yoff += pContainer->getY() + getY();
			UT_sint32 ydiff = da.yoff + pContainer->getHeight();
			if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				ydiff = da.yoff + static_cast<fp_TableContainer *>(pContainer)->getHeight();
			}
			if(containsNestedTables())
			{
				xxx_UT_DEBUGMSG(("draw nested somewhere da.yoff %d ytop %d ybot %d \n",da.yoff,ytop,ybot));
			}
			if((da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
			{
//
// Draw the top of the cell if the cell starts on this page.
//
				if(i == 0)
				{
					m_bDrawTop = true;
				}
				bStart = true;

				if(containsNestedTables())
					{
						xxx_UT_DEBUGMSG((" drawBroken drawing container %d da.yoff %d pContainer->getY()  %d \n",i,da.yoff,pContainer->getY()));
					}
				if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
					if(pTab->isThisBroken())
					{
						xxx_UT_DEBUGMSG(("Draw nested broken table \n"));
						da.yoff -= pTab->getYBreak();
						pTab->draw(&da);
						iLastDraw = i;
					}
					else
					{
						fp_TableContainer * pT= pTab->getFirstBrokenTable();
						if(pT == NULL)
						{
							UT_sint32 iY = pTab->getY();
							xxx_UT_DEBUGMSG(("No Broken Table!! \n"));
							pTab = static_cast<fp_TableContainer *>(pTab->VBreakAt(0));
							pTab->setY(iY);
						}
						else
						{
							pTab = pT;
						}
						if(pTab)
						{
							pTab->draw(&da);
							iLastDraw = i;
						}
					}

				}
				else
				{
					if((pContainer->getMyBrokenContainer() == NULL ) || (pContainer->getMyBrokenContainer() == pBroke) || (pContainer->getBreakTick() != getBreakTick()))
					{
						pContainer->setBreakTick(getBreakTick());
						pContainer->setMyBrokenContainer(pBroke);
						pContainer->draw(&da);
						iLastDraw = i;
					}
					else
					{
						UT_DEBUGMSG(("pCon->BrokenContainer %x pBroke %x \n",pContainer->getMyBrokenContainer(),pBroke));
					}
				}
			}
			else if(bStart)
			{
				xxx_UT_DEBUGMSG(("drawBroken: Skipping container: ypos %d height %d ytop %d ybot %d \n",da.yoff,pContainer->getY(),ytop,ybot));
				bStop = true;
			}
		}
		else if(bStart)
		{
			xxx_UT_DEBUGMSG(("drawBroken: Skipping container: 1 container pos %d ytop %d ybot %d \n",pContainer->getY(),ytop,ybot));
				bStop = true;
		}
		else
		{
			xxx_UT_DEBUGMSG(("drawBroken: Container line:2  container pos  %d ytop %d ybot %d pBroke %x \n",pContainer->getY(),ytop,ybot,pBroke));
		}
	}
	if((iLastDraw >= static_cast<UT_sint32>(countCons())-1) && !bStop)
	{
		m_bDirty = false;
		if(bIsNested)
		{
			UT_DEBUGMSG(("Clearing nested cell redraw %x \n",this));
		}
		else if(containsNestedTables())
		{
			UT_DEBUGMSG(("Clearing cell contained nested table of redraw %x \n",this));
		}
		getSectionLayout()->clearNeedsRedraw();
	}
	drawLines(pBroke,pG);
	pTab->setRedrawLines();
    _drawBoundaries(pDA,pBroke);
}

/*!
 * Returns true since cells can be broken vertically.
 */
bool fp_CellContainer::isVBreakable(void)
{
	return true;
}

/*!
 * Break the cell at the specified location. This is mostly to handle
 * the case of tables embedded in the cell.
 * vpos is relative to the start of the cell.
 */
fp_ContainerObject * fp_CellContainer::VBreakAt(UT_sint32 vpos)
{
	UT_sint32 iBreakTick = getBreakTick();
	iBreakTick++;
	setBreakTick(iBreakTick);
	UT_DEBUGMSG(("iBreakTick is %d \n",getBreakTick()));
	if(!containsNestedTables())
	{
		return NULL;
	}
	vpos -= 2; // make broken container wholly within the cell.
	UT_sint32 count = countCons();
	UT_DEBUGMSG(("vBREAK cell at  %d \n",vpos));
	UT_sint32 i =0;
	fp_Container * pCon;
	bool bBreak = false;
	fp_ContainerObject * pBroke = NULL;
	UT_sint32 iY = 0;
	for(i=0; i< count; i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		UT_DEBUGMSG(("Looking Container iY  %d height %d \n",iY,pCon->getHeight()));
		if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			 UT_DEBUGMSG(("Looking at nested table to break at  %d height %d \n",vpos,pCon->getHeight()));
  		}
		if(iY <= vpos && iY + pCon->getHeight() > vpos)
		{
			//
			// Container overlaps break point. See if container is 
			// is a table
			// container if possible.
			//
			if(pCon->isVBreakable())
			{
				if(pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					UT_DEBUGMSG(("Want to break nested table at %d \n",vpos));
					fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCon);
					if(!pTab->isThisBroken())
					{
						if(pTab->getY() < -999999)
						{
							pTab->setY(iY);
						}
						if(pTab->getFirstBrokenTable())
						{
							pCon = static_cast<fp_Container *>(pTab->getFirstBrokenTable());
						}
						else
						{
							pCon = static_cast<fp_Container *>(pTab->VBreakAt(0));
							fp_TableContainer * pBTab = static_cast<fp_TableContainer *>(pCon);
							pBTab->setY(iY);
							UT_DEBUGMSG(("Break Nested table Con %x at 0 y %d height %d \n",pCon,pCon->getY(),pCon->getHeight()));
							UT_DEBUGMSG(("Break Nested table Table %x at 0 y %d height %d \n",pBTab,pBTab->getY(),pBTab->getHeight()));
						}
					}
				}
				if(vpos > 0)
				{
					//
					// Tables are broken from the start of the table.
					// vpos is preseneted as the distance from start of the 
					// cell so we have to subtract the y position of the 
					// first table.
					// if there is previous non-zero ybreak table this is added
					// so we
					// subtract that off too. This is because the top-level
					// tables are broken over columns and heights are 
					// calculated from within these columns.
					//
					fp_TableContainer * pPrevTab = static_cast<fp_TableContainer *>(pCon);
					fp_TableContainer * pMaster = pPrevTab->getMasterTable();
					pBroke = pPrevTab->VBreakAt(vpos - pMaster->getY() - pPrevTab->getYBreak());
					pBroke->setY(vpos);
					static_cast<fp_TableContainer *>(pBroke)->setY(pBroke->getY());
					UT_DEBUGMSG(("Made broken nested Table %x Y %d height %d \n",pBroke,pBroke->getY(),pBroke->getHeight()));
					bBreak = true;
					break;
				}
			}
		}
		iY += pCon->getHeight();
		iY += pCon->getMarginAfter();
	}
	if(bBreak)
	{
		//layout();
	}
	return pBroke;
}

/*!
 * This routine requests that the cell be broken at the specfied height.
 * the return value of the method is the actual height it can be broken
 * which is less than or equal to the requested height.
 */
UT_sint32 fp_CellContainer::wantVBreakAt(UT_sint32 vpos)
{
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 iYBreak = vpos;
	fp_Container * pCon;
	fp_Line * pLine = NULL;
	UT_sint32 footHeight = 0;
	for(i=0; i< count; i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		UT_sint32 iY = pCon->getY() + getY();
		UT_sint32 conHeight = pCon->getHeight();
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			pLine = static_cast<fp_Line *>(pCon);
			if(pLine->containsFootnoteReference())
			{
				UT_GenericVector<fp_FootnoteContainer*> vecFC;
				if(pLine->getFootnoteContainers(&vecFC))
				{
					UT_uint32 k  = 0;
					for(k=0; k < vecFC.getItemCount(); k++)
					{
						fp_FootnoteContainer * pFC = vecFC.getNthItem(k);
						conHeight += pFC->getHeight();
						if((pFC->getPage() == NULL) || (pFC->getPage() != pLine->getPage()))
						{
							footHeight += pFC->getHeight();
						}
					}
				}
			}
		}
		if(iY <= vpos && iY + conHeight > vpos)
		{
			//
			// Container overlaps break point. Find break point in the 
			// container if possible.
			//
			UT_sint32 iCur =0;
			if(pCon->isVBreakable())
			{
				iCur = pCon->wantVBreakAt(vpos - iY);
				iCur = iCur + iY;
			}
			else
			{
				iCur = iY;
			}
			if(iCur  < iYBreak)
			{
				iYBreak = iCur;
			}
			break;
		}
	}
	if((iYBreak == vpos) && (footHeight > 0))
	{
		iYBreak = iYBreak - footHeight;
	}
	return iYBreak;
}


fp_Container * fp_CellContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pNext = pCL->getNext();
	while(pNext && ((pNext->getContainerType() == FL_CONTAINER_ENDNOTE) || 
		  (pNext->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pNext->isHidden() == FP_HIDDEN_FOLDED)))
	{
		pNext = pNext->getNext();
	}
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_CellContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pPrev = pCL->getPrev();
	while(pPrev && ((pPrev->getContainerType() == FL_CONTAINER_ENDNOTE) ||
		  (pPrev->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pPrev->isHidden() == FP_HIDDEN_FOLDED)))

	{
		pPrev = pPrev->getPrev();
	}
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}


void fp_CellContainer::sizeRequest(fp_Requisition * pRequest)
{
	xxx_UT_DEBUGMSG(("Doing size request on %x \n",pRequest));
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 height = 0;
	UT_sint32 width = 0;
	for(i=0 ; i < count; i++)
	{
		fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->recalcHeight();
			if(width < pCon->getWidth())
			{
				width = pCon->getWidth();

			}
			xxx_UT_DEBUGMSG(("sizeRequest: Height of Line %d %d \n",i,pCon->getHeight()));
			height = height + pCon->getHeight();
			height = height + pCon->getMarginAfter();
		}
		else
		{
			fp_Requisition pReq;
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				static_cast<fp_TableContainer *>(pCon)->sizeRequest(&pReq);
			}
			else if(pCon->getContainerType() == FP_CONTAINER_CELL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			if(width < pReq.width)
			{
				width = pReq.width;
			}
			height = height + pReq.height;
		}
	}
	UT_sint32 maxwidth = 0;
	fl_CellLayout * pCellL = static_cast<fl_CellLayout *>(getSectionLayout());
	fl_ContainerLayout * pCL = pCellL->getFirstLayout();
	while(pCL)
	{
		if(pCL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pCL);
			UT_sint32 iw = pBL->getMaxNonBreakableRun();
			if(maxwidth < iw)
			{
				maxwidth = iw;
			}
		}
		pCL = pCL->getNext();
	}

	if(maxwidth > width)
	{
		width = maxwidth;
	}
	if(pRequest)
	{
		pRequest->width = width;
		pRequest->height = height;
	}

	fp_Column * pCol = static_cast<fp_Column *>(fp_Container::getColumn());
	if(pCol && (width == 0))
	{
		width = pCol->getWidth();
	}

	m_MyRequest.width = width;
	m_MyRequest.height = height;
	xxx_UT_DEBUGMSG(("Size Request: Cell Total height  %d width %d \n",height,width));
}

void fp_CellContainer::sizeAllocate(fp_Allocation * pAllocate)
{
	m_MyAllocation.width = pAllocate->width;
	m_MyAllocation.height = pAllocate->height;
	m_MyAllocation.x = pAllocate->x;
	m_MyAllocation.y = pAllocate->y;
}

void fp_CellContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
	fp_Container *pContainer, *pPrevContainer = NULL;
	xxx_UT_DEBUGMSG(("Doing Cell layout %x \n",this));
	fp_TableContainer * pMyBrokenTable = NULL;
	if(countCons() == 0)
	{
		return;
	}
	pMyBrokenTable = getBrokenTable(static_cast<fp_Container*>(getNthCon(0)));
	for (UT_uint32 i=0; i < countCons(); i++)
	{
		pContainer = static_cast<fp_Container*>(getNthCon(i));
//
// This is to speedup redraws.
//
		if(pContainer->getHeight() > _getMaxContainerHeight())
			_setMaxContainerHeight(pContainer->getHeight());

		fp_TableContainer * pTab = NULL;
		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
			if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				pTab = static_cast<fp_TableContainer *>(pContainer);
				if(!pTab->isThisBroken())
				{
					//
					// The position of the master table has changed.
					// All broken tables need to be rebroken
					pTab->deleteBrokenTables(false,true);
				}
			}
		}
			
		pContainer->setY(iY);

		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			xxx_UT_DEBUGMSG(("Doing Nested table layout Y 1 is %d \n",pContainer->getY()));
			pTab = static_cast<fp_TableContainer *>(pContainer);
			if(!pTab->isThisBroken())
			{
				if(pTab->getFirstBrokenTable() == NULL)
				{
					static_cast<fp_TableContainer *>(pTab->VBreakAt(0));
					pTab = pTab->getFirstBrokenTable();
					if(pContainer->getY() == iY)
					{
						static_cast<fp_VerticalContainer *>(pTab)->setY(iY);
					}
				}
				pTab = pTab->getFirstBrokenTable();
			}
			pTab->setY(iY);
			iContainerHeight = pTab->getHeight();
			xxx_UT_DEBUGMSG(("Doing Nested table layout Y 2 is %d height is %d YBottom %d \n",pTab->getY(),pTab->getHeight(),pTab->getYBottom()));
		}

		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		//iY +=  0.5;

		// Update height of previous line now we know the gap between
		// it and the current line.
		if (pPrevContainer && pPrevContainer->getContainerType() != FP_CONTAINER_TABLE)
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
		}
		pPrevContainer = pContainer;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

	if (getHeight() == iY)
	{
		return;
	}
	setHeight(iY);
}

void fp_CellContainer::setToAllocation(void)
{
	m_bBgDirty = true;
	setWidth(static_cast<UT_sint32>(m_MyAllocation.width));
	setHeight(m_MyAllocation.height);
	setX(static_cast<UT_sint32>(m_MyAllocation.x));
	xxx_UT_DEBUGMSG(("SEVIOR: set to width %d, height %d,y %d,x %d \n", m_MyAllocation.width,m_MyAllocation.height,m_MyAllocation.y,m_MyAllocation.x));
	setMaxHeight(m_MyAllocation.height);
	setY(m_MyAllocation.y);
	layout();
}


void fp_CellContainer::getLeftTopOffsets(UT_sint32 & xoff, UT_sint32 & yoff)
{
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	UT_return_if_fail(pTab);
	xoff = -static_cast<UT_sint32>(pTab->getNthCol(getLeftAttach())->spacing);
	yoff = m_iTopY - getY();
}

/*!
 * This method sets the line markers between the rows and columns. It must be called after
 * the setToAllocation() for all cells.
 */
void fp_CellContainer::setLineMarkers(void)
{
//
// Set the boundary markers for line draing.
//
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	UT_return_if_fail(pTab);

	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pTab->getSectionLayout());
	fp_TableRowColumn *pCol = pTab->getNthCol(getLeftAttach());
	if(pCol)
		m_iLeft = getX() - static_cast<UT_sint32>(pCol->spacing);

	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach(),getRightAttach());
	if(pCell)
	{
		m_iRight = pCell->getX();
		m_iRight -= static_cast<UT_sint32>(pTab->getNthCol(getRightAttach())->spacing);
	}
	else
	{
		//		m_iRight = getX() + getWidth() - getGraphics()->tlu(1);
   		m_iRight = getX() + getWidth();
		m_iRight += static_cast<UT_sint32> (0.5 * static_cast<double>(pTab->getBorderWidth()));
	}
	m_iTopY = pTab->getYOfRow(getTopAttach());
	if(getTopAttach() == 0)
	{
		m_iTopY -= static_cast<UT_sint32>(0.5 * static_cast<double>(pTab->getBorderWidth()));
	}
	else
	{
		fp_TableRowColumn *pRow = pTab->getNthRow(getTopAttach());
		if(pRow)
			m_iTopY -= pRow->spacing/2;
	}
	if(getTopAttach() > 0)
	{
		UT_sint32 cLeft = 0;
		for(cLeft = getLeftAttach(); cLeft < getRightAttach(); cLeft++)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach() -1,cLeft);
			if(pCell)
			{
				pCell->m_iBotY = m_iTopY;
			}
			else
			{
				break;
			}
		}
	}
	xxx_UT_DEBUGMSG(("getBottomAttach %d \n",getBottomAttach()));
	if(getBottomAttach() <= pTab->getNumRows())
	{
		m_iBotY = pTab->getYOfRow(getBottomAttach());
		if(getBottomAttach() < pTab->getNumRows()) 
		{
			fp_TableRowColumn *pRow = pTab->getNthRow(getBottomAttach());
			if(pRow)
				m_iBotY += pRow->spacing/2;
		}
	}
	else
	{
//
// Have to cast the MasterTable to a vertical container to get the full height of a broken
// table. Otherwise we just get height of the first broken table.
//
		fp_VerticalContainer * pVert = static_cast<fp_VerticalContainer *>(pTab);
		m_iBotY = pTab->getYOfRow(0) + pVert->getHeight() -pTL->getBottomOffset() - getGraphics()->tlu(1);
		m_iBotY -= static_cast<UT_sint32>(2.0 * static_cast<double>(pTab->getBorderWidth()));
		m_iBotY +=  pTab->getNthRow(pTab->getNumRows()-1)->spacing/2;
	}
	
	xxx_UT_DEBUGMSG(("SEVIOR getX %d left %d right %d top %d bot %d \n",getX(),m_iLeft,m_iRight,m_iTopY,m_iBotY));

}

//---------------------------------------------------------------------

/*!
  Create a Master Table container. This is broken across other vertical
  Containers.
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TableContainer::fp_TableContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_TABLE, pSectionLayout),
	  m_iRows(0),
	  m_iCols(0),
	  m_iBorderWidth(0),
	  m_bIsHomogeneous(true),
	  m_iRowSpacing(0),
	  m_iColSpacing(0),
	  m_pFirstBrokenTable(NULL),
	  m_pLastBrokenTable(NULL),
	  m_bIsBroken(false),
	  m_pMasterTable(NULL),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_bRedrawLines(false),
	  m_iRepeatedRowNumber(-1),
	  m_iRepeatedRowHeight(0),
	  m_iLeftOffset(0),
	  m_iRightOffset(0),
	  m_iTopOffset(0),
	  m_iBottomOffset(0),
	  m_iLineThickness(1),
	  m_iRowHeightType(FL_ROW_HEIGHT_NOT_DEFINED),
	  m_iRowHeight(0),
	  m_iLastWantedVBreak(-1),
	  m_pFirstBrokenCell(NULL),
	  m_pLastBrokenCell(NULL),
	  m_bRecursiveClear(false)

{
	if(getSectionLayout())
	{
		getSectionLayout()->setNeedsRedraw();
		getSectionLayout()->markAllRunsDirty();
	}
}


/*!
  Create a broken Table container. This is placed between the cells and
  drawing. A vertical offset is subtracted from the Cells Y location for
  all manipulations.
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TableContainer::fp_TableContainer(fl_SectionLayout* pSectionLayout, fp_TableContainer * pMaster) 
	: fp_VerticalContainer(FP_CONTAINER_TABLE, pSectionLayout),
	  m_iRows(0),
	  m_iCols(0),
	  m_iBorderWidth(0),
	  m_bIsHomogeneous(true),
	  m_iRowSpacing(0),
	  m_iColSpacing(0),
	  m_pFirstBrokenTable(NULL),
	  m_pLastBrokenTable(NULL),
	  m_bIsBroken(true),
	  m_pMasterTable(pMaster),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_iBrokenTop(0),
	  m_iBrokenBottom(0),
	  m_bRedrawLines(false),
	  m_iRepeatedRowNumber(-1),
	  m_iRepeatedRowHeight(0),
	  m_iLeftOffset(0),
	  m_iRightOffset(0),
	  m_iTopOffset(0),
	  m_iBottomOffset(0),
	  m_iLineThickness(1),
	  m_iRowHeightType(FL_ROW_HEIGHT_NOT_DEFINED),
	  m_iRowHeight(0),
	  m_iLastWantedVBreak(-1),
	  m_pFirstBrokenCell(NULL),
	  m_pLastBrokenCell(NULL),
	  m_bRecursiveClear(false),
	  m_iAdditionalMarginAfter(0)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_TableContainer::~fp_TableContainer()
{
	UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecRows);
	UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecColumns);
	clearCons();
	deleteBrokenTables(false,false);
	xxx_UT_DEBUGMSG(("SEVIOR: deleting table %x \n",this));
//
// For debugging...
//
	setContainer(NULL);
	setPrev(NULL);
	setNext(NULL);
	m_pMasterTable = NULL;
}

fp_Column * fp_TableContainer::getBrokenColumn(void)
{
	if(!isThisBroken())
	{
		return static_cast<fp_Column *>(fp_VerticalContainer::getColumn());
	}
	fp_TableContainer * pBroke = this;
	bool bStop = false;
	fp_Column * pCol = NULL;
	while(pBroke && pBroke->isThisBroken() && !bStop)
	{
		fp_Container * pCon = pBroke->getContainer();
		if(pCon->isColumnType())
		{
			if(pCon->getContainerType() == FP_CONTAINER_COLUMN)
			{
				pCol = static_cast<fp_Column *>(pCon);
			}
			else
			{
				pCol = static_cast<fp_Column *>(pCon->getColumn());
			}
			bStop = true;
		}
		else
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pBroke->getContainer());
			UT_ASSERT(pCell->getContainerType() == FP_CONTAINER_CELL);
			pBroke = pCell->getBrokenTable(static_cast<fp_Container *>(pBroke));
		}
	}
	if(pBroke && !bStop)
	{
		pCol = static_cast<fp_Column *>(pBroke->getContainer());
	}
	//	UT_ASSERT(pCol->getContainerType() != FP_CONTAINER_CELL);
	if(pCol && pCol->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_Container * pCon = static_cast<fp_Container *>(pCol);
		while(pCon && !pCon->isColumnType())
		{
			pCon = pCon->getContainer();
		}
		if(pCon)
		{
			pCol = static_cast<fp_Column *>(pCon);
		}
		else
		{
			pCol = NULL;
		}
	}
	return pCol;
}

bool fp_TableContainer::containsNestedTables(void)
{
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	return (pTL->getNumNestedTables() > 0);
}

fp_TableContainer * fp_TableContainer::getFirstBrokenTable(void) const
{
	if(isThisBroken())
	{
		return getMasterTable()->getFirstBrokenTable();
	}
	return m_pFirstBrokenTable;
}

/*
 * Just draw the lines around a table
 */
void fp_TableContainer::drawLines(void)
{
	if(isThisBroken())
	{
		m_bRedrawLines = false;
		getMasterTable()->drawLines();
		return;
	}
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getNthCon(0));
	while(pCell)
	{
		fp_TableContainer * pBroke = getFirstBrokenTable();
		if(pBroke)
		{
			while(pBroke)
			{
				pCell->drawLines(pBroke,getGraphics());
				pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
			}
		}
		else
		{
			pCell->drawLines(NULL,getGraphics());
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	m_bRedrawLines = false;
}

fp_TableContainer * fp_TableContainer::getLastBrokenTable(void) const
{
	if(isThisBroken())
	{
		return getMasterTable()->getLastBrokenTable();
	}
	return m_pLastBrokenTable;
}

UT_sint32 fp_TableContainer::getBrokenNumber(void)
{
	if(!isThisBroken())
	{
		return 0;
	}
	fp_TableContainer * pTab = getMasterTable()->getFirstBrokenTable();
	UT_sint32 i = 1;
	while(pTab && pTab != this)
	{
		pTab = static_cast<fp_TableContainer *>(pTab->getNext());
		i++;
	}
	if(!pTab)
	{
		return -1;
	}
	return i;
}
		

void fp_TableContainer::setFirstBrokenTable(fp_TableContainer * pBroke) 
{
	if(isThisBroken())
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		fp_TableContainer * pMaster = getMasterTable();
		pMaster->setFirstBrokenTable(pBroke);
		fp_TableContainer * pNext = static_cast<fp_TableContainer *>(pMaster);
		while(pNext)
		{
			pNext->setFirstBrokenTable( pBroke);
			pNext = static_cast<fp_TableContainer *>(pNext->getNext());
		}
	}
	m_pFirstBrokenTable = pBroke;

}

void fp_TableContainer::setLastBrokenTable(fp_TableContainer * pBroke) 
{
	if(isThisBroken())
	{
		fp_TableContainer * pMaster = getMasterTable();
		pMaster->setLastBrokenTable(pBroke);
		fp_TableContainer * pNext = static_cast<fp_TableContainer *>(pMaster);
		while(pNext)
		{
			pNext->setLastBrokenTable( pBroke);
			pNext = static_cast<fp_TableContainer *>(pNext->getNext());
		}
	}
	m_pLastBrokenTable = pBroke;
}

/*!
 * Return the Y location of row number row
 */
UT_sint32 fp_TableContainer::getYOfRow(UT_sint32 row)
{
	UT_sint32 i =0;
	UT_sint32 numCols = getNumCols();
	if(row > getNumRows())
	{
		return 0;
	}
	fp_CellContainer * pCell = getCellAtRowColumn(0,0);

	UT_sint32 iYRow = 0;
	if(pCell != NULL)
	{
		iYRow = pCell->getY();
	}
	for(i=0; i< numCols; i++)
	{
		pCell = getCellAtRowColumn(0,i);
		if(pCell)
		{
			UT_sint32 Y = pCell->getY();
			if(Y < iYRow)
			{
				iYRow = Y;
			}
		}
	}
	if(row == 0)
	{
		return iYRow;
	}
	xxx_UT_DEBUGMSG(("Looking for row %d numrows %d \n",row,getNumRows()));
	for(i=0; i < row; i++)
	{
		fp_TableRowColumn *pRow = getNthRow(i);
		if(pRow)
		{
			iYRow += pRow->allocation;
			iYRow += pRow->spacing;
		}
		xxx_UT_DEBUGMSG((" row %d Height here %d \n",i,iYRow));
	}
	if((row < getNumRows()) && (i > 0))
	{
		fp_TableRowColumn *pRow = getNthRow(i-1);
		if(pRow)
		{
			iYRow -= pRow->spacing;
			iYRow += pRow->spacing/2;
		}
	}
	return iYRow;
}

/*!
 * This method returns the Y Location of the line drawn across the top of
 * broken table.
 * It returns -1 if the table is not broken or it is the first broken table
 * of the chain.
 */
UT_sint32 fp_TableContainer::getBrokenTop(void)
{
	if(getMasterTable() == NULL)
	{
		return -1;
	}
	else if(getYBreak() == 0)
	{
		return -1;
	}
	return m_iBrokenTop;
}

/*!
 * This method returns the Y Location of the line drawn across the Bottom of
 * broken table.
 * It returns -1 if the table is not broken.
 */
UT_sint32 fp_TableContainer::getBrokenBot(void)
{
	if(getMasterTable() == NULL)
	{
		return -1;
	}
	return m_iBrokenBottom;
}


/*!
 * This static function is used to compare cells' position for the
 * UT_Vector::binarysearch
\param vX1 pointer to a Point value.
\param vX2 pointer to a fp_CellContainer object
*/
static UT_sint32 compareCellPosBinary(const void * vX1, const void * vX2)
{
	const UT_Point *pt = static_cast<const UT_Point *>(vX1);
	const fp_ContainerObject *pc = *(fp_ContainerObject **)(vX2);
	const fp_CellContainer *pCell = static_cast<const fp_CellContainer *>(pc);

	if((pCell->getTopAttach()) <= pt->y && (pCell->getBottomAttach() > pt->y)
	   && (pCell->getLeftAttach() <= pt->x) && (pCell->getRightAttach() > pt->x))
	{
		return 0;
	}
	// compare cell's top and bottom first
	if (pCell->getTopAttach() > pt->y)
	{
		return -1;
	}
	if (pCell->getBottomAttach() <= pt->y)
	{
		return 1;
	}
	// then compare cell's left position
	if (pCell->getLeftAttach() > pt->x)
	{
		return -1;
	}
	if (pCell->getRightAttach() <= pt->x)
	{
		return 1;
	}

	return 0;
}

/*!
 * Binary search failed. Do a simple Linear search instead
 */
fp_CellContainer * fp_TableContainer::getCellAtRowColumnLinear(UT_sint32 row, UT_sint32 col)
{
	UT_uint32 i = 0;
	fp_CellContainer * pCell = NULL;
	bool bFound = false;
	for(i=0; (i<countCons()) && !bFound; i++)
	{
		pCell = static_cast<fp_CellContainer *>(getNthCon(i));
		if((pCell->getTopAttach()) <= row && (pCell->getBottomAttach() > row)
		   && (pCell->getLeftAttach() <= col) && (pCell->getRightAttach() > col))
		{
			return pCell;
		}
	}
	return NULL;
}
/*!
 * Return the cell container at the specified row and column
 */
fp_CellContainer * fp_TableContainer::getCellAtRowColumn(UT_sint32 row, UT_sint32 col)
{
	UT_Point pt;
	pt.x = col;
	pt.y = row;
	if((row >= getNumRows()) || (row <0))
	{
		return NULL;
	}
	if((col >= getNumCols()) || (col < 0))
	{
		return NULL;
	}
	UT_sint32 u =-1;
	u = binarysearchCons(&pt, compareCellPosBinary);
	if (u != -1)
	{
		fp_CellContainer *pSmall = static_cast<fp_CellContainer *>(getNthCon(u));
		if((pSmall->getTopAttach() > row) || (pSmall->getBottomAttach() <= row)
		   || (pSmall->getLeftAttach() > col) || (pSmall->getRightAttach() <= col))
			{
				xxx_UT_DEBUGMSG(("No cell found 1 at %d %d \n",row,col));
				xxx_UT_DEBUGMSG(("Returned cell left %d right %d top %d bot %d pt.y %d \n",pSmall->getLeftAttach(),pSmall->getRightAttach(),pSmall->getTopAttach(),pSmall->getBottomAttach(),pt.y));
				return getCellAtRowColumnLinear(row,col);
			}
		return pSmall;
	}
	xxx_UT_DEBUGMSG(("No cell found -2 at %d %d \n",row,col));
	return getCellAtRowColumnLinear(row,col);
}

/* 
 * This method looks up the various propeties of the table and returns the
 * height of a given row. The input parameter iMeasHeight is the height 
 * the row would have if it's height was automatically calculated from the
 * height of the rows.
 */
UT_sint32 fp_TableContainer::getRowHeight(UT_sint32 iRow, UT_sint32 iMeasHeight)
{
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	UT_return_val_if_fail(pTL, 0);
	const  UT_GenericVector<fl_RowProps*>* pVecRow = pTL->getVecRowProps();
	if(static_cast<UT_sint32>(pVecRow->getItemCount()) < (iRow + 1))
	{
		if(m_iRowHeight == 0)
		{
			return iMeasHeight;
		}
		if(m_iRowHeightType == FL_ROW_HEIGHT_EXACTLY)
		{
			return m_iRowHeight;
		}
		if(m_iRowHeightType == FL_ROW_HEIGHT_AT_LEAST)
		{
			if(iMeasHeight < m_iRowHeight)
			{
				return m_iRowHeight;
			}
			else
			{
				return iMeasHeight;
			}
		}
		return iMeasHeight;
	}
	fl_RowProps * pRowProps = pVecRow->getNthItem(iRow);
	UT_sint32 iRowHeight = pRowProps->m_iRowHeight;
	FL_RowHeightType rowType = pRowProps->m_iRowHeightType;
	if(rowType == FL_ROW_HEIGHT_EXACTLY )
	{
		return iRowHeight;
	}
	if(rowType == FL_ROW_HEIGHT_AT_LEAST)
	{
		if(iMeasHeight < iRowHeight)
		{
			return iRowHeight;
		}
		else
		{
			return iMeasHeight;
		}
	}
	if(rowType == FL_ROW_HEIGHT_AUTO)
	{
		return iMeasHeight;
	}
//
// Here is the row type is undefined
//
	if(m_iRowHeightType == FL_ROW_HEIGHT_EXACTLY)
	{
		if(m_iRowHeight == 0 )
		{
			if(iRowHeight > 0)
			{
				return iRowHeight;
			}
			else
			{
				return iMeasHeight;
			}
		}
		else
		{
			return m_iRowHeight;
		}
	}
	if(m_iRowHeightType == FL_ROW_HEIGHT_AT_LEAST)
	{
		if(m_iRowHeight > 0)
		{
			if(iMeasHeight < m_iRowHeight)
			{
				return m_iRowHeight;
			}
			else
			{
				return iMeasHeight;
			}
			return iMeasHeight;
		}
		if(iMeasHeight > iRowHeight)
		{
			return iMeasHeight;
		}
		else
		{
			return iRowHeight;
		}
	}
	if(m_iRowHeightType == FL_ROW_HEIGHT_AUTO)
	{
		return iMeasHeight;
	}
//
// Undefined on undefined with a defined height - assume AT_LEAST
//
	if(iMeasHeight > iRowHeight)
	{
		return iMeasHeight;
	}
	return iRowHeight;
}


/*!
  Find document position from X and Y coordinates
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
 */
void fp_TableContainer::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
								   bool& bBOL, bool& bEOL, bool & isTOC)
{
	if(isThisBroken())
	{
		y = y + getYBreak();
		getMasterTable()->mapXYToPosition(x,y, pos,bBOL,bEOL,isTOC);
		return;
	}
	UT_sint32 count = countCons();
	if(count == 0)
	{
		pos = 2;
		bBOL = true;
		bEOL = true;
		return;
	}
	y = y + getYBreak();
	xxx_UT_DEBUGMSG(("SEVIOR: Table %x Looking for location x %d y %d \n",this,x,y));
	fp_VerticalContainer* pC = NULL;
	UT_sint32 i = 0;
	bool bFound = false;

	// First see if there is a container with the point inside it.
	for(i=0; (i< count) && !bFound; i++)
	{
		pC = static_cast<fp_VerticalContainer*>(getNthCon(i));
		if(x >= pC->getX() && x <  pC->getX() + pC->getWidth() &&
		   y >=  pC->getY() && y < pC->getY()+ pC->getMaxHeight())
		{
			bFound = true;
		}
	}
	if(bFound)
	{
		pC->mapXYToPosition(x - pC->getX(), y - pC->getY(), pos, bBOL, bEOL,isTOC);
		return;
	}
//
// No cell directly overlaps. Look first for a column that overlaps and
// then choose the cell that is closest within that. Otherwise choose
// the closest cell.
//
	fp_VerticalContainer * pCloseX = NULL;
	fp_VerticalContainer * pCloseTot = NULL;
	UT_sint32 dclosex = 231456789;
	UT_sint32 dclosetot = 231456789;
	UT_sint32 d = 0;
	for(i=0; (i< count) && !bFound; i++)
	{
		pC = static_cast<fp_VerticalContainer*>(getNthCon(i));
		if(pC->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pC);
			if(!pTab->isThisBroken())
			{
				pC = static_cast<fp_VerticalContainer *>(pTab->getFirstBrokenTable());
				if(pC == NULL)
				{
					pC = static_cast<fp_VerticalContainer *>(pC);
				}
			}
		}
		if(x >= pC->getX() && x < pC->getX() + pC->getWidth())
		{
			d = y - pC->getY();
			if(d < 0) 
			    d = - d;
			if(d < dclosex)
			{
				dclosex = d;
				pCloseX = pC;
			}
		}
		d = pC->distanceFromPoint(x,y);
		if(d < dclosetot)
		{
			pCloseTot = pC;
			dclosetot = d;
		}
	}
	if(pCloseX != NULL)
	{
		pC = pCloseX;
	}
	else
	{
		pC = pCloseTot;
	}
	UT_return_if_fail( pC != NULL);
	pC->mapXYToPosition(x - pC->getX(), y  - pC->getY(), pos, bBOL, bEOL,isTOC);
}

void fp_TableContainer::resize(UT_sint32 n_rows, UT_sint32 n_cols)
{
  
  if ((n_rows != m_iRows) ||
     ( n_cols != m_iCols))
  {
	  fp_CellContainer * child = static_cast<fp_CellContainer *>(getNthCon(0));
      while(child)
	  {
		  n_rows = UT_MAX (n_rows, child->getBottomAttach());
		  n_cols = UT_MAX (n_cols, child->getRightAttach());
		  child = static_cast<fp_CellContainer *>(child->getNext());
	  }
      
      if (n_rows != m_iRows)
	  {
		  UT_sint32 i;

		  i = m_iRows;
	      m_iRows = n_rows;
		  UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecRows);
		  m_vecRows.clear();
		  for(i=0; i< m_iRows; i++)
		  {
			  m_vecRows.addItem(new fp_TableRowColumn());
			  fp_TableRowColumn * pRow = getNthRow(i);
			  pRow->requisition = 0;
			  pRow->allocation = 0;
			  pRow->spacing = m_iRowSpacing;
			  pRow->need_expand = 0;
			  pRow->need_shrink = 0;
			  pRow->expand = 0;
			  pRow->shrink = 0;
		  }
	  }

      if (n_cols != m_iCols)
	  {
		  UT_sint32 i;

		  i = m_iCols;
	      m_iCols = n_cols;
		  UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecColumns);
		  m_vecColumns.clear();
		  for(i=0; i< m_iCols; i++)
		  {
			  m_vecColumns.addItem(new fp_TableRowColumn());
			  fp_TableRowColumn *pCol= getNthCol(i);
			  pCol->requisition = 0;
			  pCol->allocation = 0;
			  pCol->spacing = m_iColSpacing;
			  pCol->need_expand = 0;
			  pCol->need_shrink = 0;
			  pCol->expand = 0;
			  pCol->shrink = 0;
		  }
	  }
  }
  xxx_UT_DEBUGMSG(("SEVIOR: m_iRowSpacing = %d \n",m_iRowSpacing));
}

/*!
 * Returns true since a table can be broken vertically.
 */
bool fp_TableContainer::isVBreakable(void)
{
	//
	// Fixme remove this when nested tables break across pages.
	//
#if 0
	if(getContainer() && (getContainer()->getContainerType() == FP_CONTAINER_CELL))
	{
		return false;
	}
#endif
	return true;
}

/*!
 * This method adjusts the m_iYBreak and m_iYBottom variables after a 
 * setY method changes the start position of the top of the table.
 */
void fp_TableContainer::adjustBrokenTables(void)
{
	if(isThisBroken())
	{
		//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	if(getFirstBrokenTable() == NULL)
	{
		return;
	}
	if(getFirstBrokenTable() == getLastBrokenTable())
	{
		return;
	}
	return;
	fp_TableContainer * pBroke = getFirstBrokenTable();
	fp_VerticalContainer * pVC = static_cast<fp_VerticalContainer *>(getContainer());
	UT_sint32 iNewHeight = pVC->getMaxHeight() - getY();	
	UT_sint32 ishift = iNewHeight - pBroke->getYBottom();
	UT_sint32 iNewBot = pBroke->getYBottom() + ishift;
	UT_sint32 iTableHeight = fp_VerticalContainer::getHeight();
	xxx_UT_DEBUGMSG(("SEVIOR: ishift = %d iNewHeight %d  pBroke->getYBottom() %d \n",ishift,iNewHeight,pBroke->getYBottom()));
	if(ishift == 0)
	{
		return;
	}
	if(iNewBot > iTableHeight)
	{
		iNewBot = iTableHeight;
	}
	pBroke->setYBottom(iNewBot);
	UT_ASSERT(pBroke->getHeight());

	pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	while(pBroke)
	{
		UT_sint32 iNewTop = pBroke->getYBreak();
		iNewBot = pBroke->getYBottom();
		pBroke->setYBreakHere(iNewTop + ishift);
		if(pBroke->getNext())
		{
			pBroke->setYBottom(iNewBot+ishift);
			UT_ASSERT(pBroke->getHeight());
		}
		else
		{
			pBroke->setYBottom(iTableHeight);
			UT_ASSERT(pBroke->getHeight());
		}
		xxx_UT_DEBUGMSG(("SEVIOR: Broken table %x YBreak adjusted to %d Shift is %d height is %d \n",pBroke,iNewTop+ishift,ishift,pBroke->getHeight()));
		fp_TableContainer * pPrev = static_cast<fp_TableContainer *>(pBroke->getPrev());
//
// If the height of the previous plus the height of pBroke offset from
// the previous position is less that the column height we can delete
// this broken table. FIXE: This won't work for nested tables.
//
		UT_sint32 iMaxHeight = 0;
		bool bDeleteOK = false;
		if(pPrev)
		{
			iMaxHeight = static_cast<fp_VerticalContainer *>(pPrev->getContainer())->getMaxHeight();
			xxx_UT_DEBUGMSG(("SEVIOR: sum %d maxheight %d \n",(pPrev->getY() + pPrev->getHeight() + pBroke->getHeight()), iMaxHeight));
		}
		if(bDeleteOK && pPrev && (pPrev->getY() + pPrev->getHeight() + pBroke->getHeight() < iMaxHeight))
		{
//
// FIXME: This if should be unnested....
//
			if(pPrev == this)
			{
				pPrev = getFirstBrokenTable();
			}
			xxx_UT_DEBUGMSG(("SEVIOR; In adjust - Deleting table. Max height %d prev Y %d prev Height %d cur Height %d \n",iMaxHeight, pPrev->getY(),pPrev->getHeight(),pBroke->getHeight()));
//
// Don't need this table any more. Delete it and all following tables.
// after adjusting the previous table.
//
			pPrev->setYBottom(iTableHeight);
			UT_ASSERT(pPrev->getHeight());
			pPrev->setNext( NULL);
			if(pPrev == getFirstBrokenTable())
			{
				setNext(NULL);
				getFirstBrokenTable()->setYBreakHere(0);
				UT_ASSERT(getFirstBrokenTable()->getHeight());
			}
			setLastBrokenTable(pPrev);
			xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!! 2 last broken table %x deleting %x Master Table %x  \n",getLastBrokenTable(),pBroke,this));
			xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!! 2 get first %x get last broken table %x \n",getFirstBrokenTable(),getLastBrokenTable()));
			fp_TableContainer * pT = getFirstBrokenTable();
			UT_sint32 j = 0;
			while(pT)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Table %d is %x \n",j,pT));
				j++;
				pT = static_cast<fp_TableContainer *>(pT->getNext());
			}
			while(pBroke)
			{
				fp_TableContainer * pNext = static_cast<fp_TableContainer *>(pBroke->getNext());
				UT_sint32 i = pBroke->getContainer()->findCon(pBroke);
				if(i >=0)
				{
					pBroke->getContainer()->deleteNthCon(i);
				}
				else
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
				xxx_UT_DEBUGMSG(("SEVIOR: Adjust  - Delete Table %x \n",pBroke));
				delete pBroke;
				pBroke = pNext;
			}
		}
		else
		{
			pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
		}
	}
}

/*!
 * This deletes all the broken tables from this master table.
 * This routine assumes that a clear screen has been set already.
 */
void fp_TableContainer::deleteBrokenTables(bool bClearFirst, bool bRecurseUp)
{
	if(isThisBroken())
	{
		return;
	}
	if(bClearFirst)
	{
		clearScreen();
		//
		// Remove broken Table pointers
		//
		clearBrokenContainers();
	}
	if(getFirstBrokenTable() == NULL)
	{
		return;
	}
	fp_TableContainer * pUpTab = this;
	if(bRecurseUp)
	{
		while(pUpTab && pUpTab->getContainer() && pUpTab->getContainer()->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pUpCell = static_cast<fp_CellContainer *>(pUpTab->getContainer());
			pUpTab = static_cast<fp_TableContainer *>(pUpCell->getContainer());
		}
		if(pUpTab && (pUpTab != this))
		{
			pUpTab->deleteBrokenTables(bClearFirst,false);
			return;
		}
	}
	if(containsNestedTables())
	{
		xxx_UT_DEBUGMSG(("Deleting nested broken tables \n"));
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getFirstContainer());
		while(pCell)
		{
			xxx_UT_DEBUGMSG(("Deleting broken tables in Cell Con %x \n",pCell));
			pCell->deleteBrokenTables(bClearFirst);
			pCell = static_cast<fp_CellContainer *>(pCell->getNext());
		}
	}

	fp_TableContainer * pBroke = NULL;
	fp_TableContainer * pNext = NULL;
	fp_TableContainer * pLast = NULL;
	pBroke = getFirstBrokenTable();
	bool bFirst = true;
	bool bDontRemove = false;
	fl_ContainerLayout * pMyConL = getSectionLayout()->myContainingLayout();
	if(pMyConL && pMyConL->getContainerType() == FL_CONTAINER_CELL)
	{
		pMyConL = pMyConL->myContainingLayout();
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pMyConL);
		if(pTL->isDoingDestructor())
		{
				bDontRemove = true;
		}
	}
	while(pBroke )
	{
		pNext = static_cast<fp_TableContainer *>(pBroke->getNext());
		//
		// Remove from list
		//
		if(pBroke->getPrev())
		{
			pBroke->getPrev()->setNext(pBroke->getNext());
		}
		if(pBroke->getNext())
		{
			pBroke->getNext()->setPrev(pBroke->getPrev());
		}
		pLast = pBroke;
		if(pBroke->getContainer() && !bDontRemove)
		{
			UT_sint32 i = pBroke->getContainer()->findCon(pBroke);
//
// First broken table is not in the container.
//
			if(i >=0)
			{
				fp_Container * pCon = pBroke->getContainer();
				pBroke->setContainer(NULL);
				pCon->deleteNthCon(i);
				xxx_UT_DEBUGMSG(("Delete %x from column %x \n",pBroke,pCon));
				//
				// Search before and after. This should not happen!
				// FIXME put in some code to detect this in breakSection
				//
				fp_Container * pPrevCon = static_cast<fp_Container *>(pCon->getPrev());
				while(pPrevCon && i >=0)
				{
					i = pPrevCon->findCon(pBroke);
					UT_sint32 j = i;
					while(j >= 0)
					{
						UT_DEBUGMSG(("Also remove table %x from column %x \n",pBroke,pPrevCon)); 
						pPrevCon->deleteNthCon(j);
						j = pPrevCon->findCon(pBroke);
					}
					pPrevCon = static_cast<fp_Container *>(pPrevCon->getPrev());
				}
				fp_Container * pNextCon = static_cast<fp_Container *>(pCon->getNext());
				i = 0;
				while(pNextCon && (i>=0))
				{
					i = pNextCon->findCon(pBroke);
					UT_sint32 j = i;
					while(j >= 0)
					{
						xxx_UT_DEBUGMSG(("Also remove table %x from column %x \n",pBroke,pNextCon)); 
						pNextCon->deleteNthCon(j);
						j = pNextCon->findCon(pBroke);
					}
					pNextCon = static_cast<fp_Container *>(pNextCon->getNext());
				}
			}
		}
		bFirst = false;
		xxx_UT_DEBUGMSG(("SEVIOR: table %x  Deleting broken table %x \n",this,pBroke));
		delete pBroke;
		if(pBroke == getLastBrokenTable())
		{
			pBroke = NULL;
		}
		else
		{
			pBroke = pNext;
		}
	}
	setFirstBrokenTable(NULL);
	setLastBrokenTable(NULL);
	setNext(NULL);
	setPrev(NULL);
//	if(bClearFirst)
	{
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
		if(pTL->myContainingLayout()->getContainerType() != FL_CONTAINER_CELL)
		{
			fl_DocSectionLayout * pDSL = pTL->getDocSectionLayout();
			pDSL->deleteBrokenTablesFromHere(pTL);
		}
	}
}

void fp_TableContainer::setHeight(UT_sint32 iHeight)
{
	if(!isThisBroken())
	{
		xxx_UT_DEBUGMSG(("Unbroken Table Height set to %d from %d \n",iHeight,getHeight()));
	}
	fp_VerticalContainer::setHeight(iHeight);
}

UT_sint32 fp_TableContainer::getMarginBefore(void) const
{
	if(isThisBroken())
	{
		if(getPrev() && getPrev() == static_cast<fp_ContainerObject *>(getMasterTable()))
		{
			// getMargin of revious block
			fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
			fl_ContainerLayout * pCL = pTL->getPrev();
			if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
			{
				return static_cast<fl_BlockLayout *>(pCL)->getBottomMargin();
			}
			return 0;
		}
		else if( getPrev() == NULL)
		{
			// getMargin of revious block
			fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
			fl_ContainerLayout * pCL = pTL->getPrev();
			if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
			{
				return static_cast<fl_BlockLayout *>(pCL)->getBottomMargin();
			}
			return 0;
		}
		else
		{
			return 0;
		}
	}
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	fl_ContainerLayout * pCL = pTL->getPrev();
	if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		return static_cast<fl_BlockLayout *>(pCL)->getBottomMargin();
	}
	return 0;
}


UT_sint32 fp_TableContainer::getMarginAfter(void) const
{
	if(isThisBroken())
	{
		if(getNext())
		{
			return 0; // still in the table list
		}
	}
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	fl_ContainerLayout * pCL = pTL->getNext();
	if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		return static_cast<fl_BlockLayout *>(pCL)->getTopMargin()+ 0;
	}
	return 0;
}

/*!
 * vpos is the location from the top of the table that holds these
 * cells.
 */
void fp_TableContainer::breakCellsAt(UT_sint32 vpos)
{
	if(!containsNestedTables())
	{
		return;
	}
	fp_TableContainer * pMaster = NULL;
	if(isThisBroken())
	{
		pMaster = getMasterTable();
	}
	else
	{
		pMaster = this;
	}
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pMaster->getNthCon(0));
	bool bStop = false;
	while(pCell && !bStop)
	{
		if(pCell->getY() >= vpos)
		{
			bStop = true;
			break;
		}
		if((pCell->getY() + pCell->getHeight()) > vpos)
		{
			pCell->VBreakAt(vpos - pCell->getY());
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
}
	
/*!
 * This method creates a new broken tablecontainer, broken at the
 * offset given. 
 * If the new tablecontainer is broken from a pre-existing 
 * broken table it is inserted into the holding vertical container after
 * the old broken table.
 * It also inserted into the linked list of containers in the vertical
 * container.
 * vpos is relative to the either the start of the table if it's the first
 * non-zero vpos or relative to the previous ybreak if it's further down.
 */
fp_ContainerObject * fp_TableContainer::VBreakAt(UT_sint32 vpos)
{
//
// If this table is nested in a cell we don't break. The offsets for each line
// will be calculated only with the top level broken table.
//
	fp_Container * pCon = getContainer();
	UT_return_val_if_fail(pCon, NULL);
	bool bIsNested = false;
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
		bIsNested = true;
		xxx_UT_DEBUGMSG(("ypos of nested table being broken %d \n",getY()));
	}
//
// Do the case of creating the first broken table from the master table.
// 
	fp_TableContainer * pBroke = NULL;
	if(!isThisBroken() && getLastBrokenTable() == NULL)
	{
		if(getFirstBrokenTable() != NULL)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return NULL;
		}
		pBroke = new fp_TableContainer(getSectionLayout(),this);
		xxx_UT_DEBUGMSG(("SEVIOR:!!!!!!! Frist broken table %x \n",pBroke));
		pBroke->setYBreakHere(vpos);
		pBroke->setYBottom(fp_VerticalContainer::getHeight());
		// leave this in!		UT_ASSERT(pBroke->getHeight());
		setFirstBrokenTable(pBroke);
		setLastBrokenTable(pBroke);
		pBroke->setContainer(getContainer());
		/*UT_sint32 iTweak =*/ tweakBrokenTable(pBroke);
		static_cast<fp_VerticalContainer *>(pBroke)->setHeight(pBroke->getHeight());
		static_cast<fp_VerticalContainer *>(pBroke)->setY(getY());
		pBroke->breakCellsAt(vpos);
		return pBroke;
	}
//
// Now do the case of breaking a Master table.
//
	if(getMasterTable() == NULL)
	{
		return getLastBrokenTable()->VBreakAt(vpos);
	}
	pBroke = new fp_TableContainer(getSectionLayout(),getMasterTable());
	getMasterTable()->setLastBrokenTable(pBroke);

	xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!!  New broken table %x \n",getLastBrokenTable()));

//
// vpos is relative to the container that contains this height but we need
// to add in the height above it.
//
	pBroke->setYBreakHere(getYBreak()+vpos);
	setYBottom(getYBreak() + vpos -1);
	UT_ASSERT(getHeight() >0);
	fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(getMasterTable());
	if(pVCon == NULL)
	{

	}
	pBroke->setYBottom(pVCon->getHeight());
	xxx_UT_DEBUGMSG(("SEVIOR????????: YBreak %d YBottom  %d Height of broken table %d \n",pBroke->getYBreak(),pBroke->getYBottom(),pBroke->getHeight()));
	xxx_UT_DEBUGMSG(("SEVIOR????????: Previous table YBreak %d YBottom  %d Height of broken table %d \n",getYBreak(),getYBottom(),getHeight()));
	UT_ASSERT(pBroke->getHeight() > 0);
	UT_sint32 i = 0;
//
// The structure of table linked list is as follows.
// NULL <= Master <==> Next <==> Next => NULL
//          first 
// ie terminated by NULL's in the getNext getPrev list. The second
// broken table points and is pointed to by the Master table
// 
	pBroke->setPrev(this);
	fp_Container * pUpCon = NULL;
	if(getMasterTable()->getFirstBrokenTable() == this)
	{
		i = getContainer()->findCon(getMasterTable());
		pUpCon = getMasterTable()->getContainer();
  		pBroke->setPrev(getMasterTable());
  		pBroke->setNext(NULL);
  		getMasterTable()->setNext(pBroke);
		setNext(pBroke);
	}
	else
	{
  		pBroke->setNext(NULL);
  		setNext(pBroke);
		if(getYBreak() == 0 )
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			pUpCon = getMasterTable()->getContainer();
//
// Fallback for loads...
//
			if(pUpCon == NULL)
			{
				pUpCon = getContainer();
			}
		}
		else
		{
			pUpCon = getContainer();
		}
		if(getYBreak() == 0)
		{
			i = pUpCon->findCon(getMasterTable());
		}
		else
		{
			i = pUpCon->findCon(this);
		}
	}
	if(i >=0 && i < static_cast<UT_sint32>(pUpCon->countCons()) -1)
	{
		pUpCon->insertConAt(pBroke,i+1);
	}
	else if( i == static_cast<UT_sint32>(pUpCon->countCons()) -1)
	{
		pUpCon->addCon(pBroke);
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return NULL;
	}
	pBroke->setContainer(pUpCon);
	//
	// Now deal with issues from a container overlapping the top of the
	// of the new broken table.
	//
	UT_sint32 iTweak = tweakBrokenTable(pBroke);
	xxx_UT_DEBUGMSG(("BrakeTable: Tweak Result is %d !!!!!!!!!!!\n",iTweak));
 	if(iTweak > 0)
 	{

		xxx_UT_DEBUGMSG(("Ybreak of %x set to %d after tweak \n",pBroke,pBroke->getYBreak() - iTweak));
 		pBroke->setYBreakHere(pBroke->getYBreak() - iTweak);
		xxx_UT_DEBUGMSG(("YBottom set to %d after tweak \n",getYBottom() - iTweak -1));
 	}
	static_cast<fp_VerticalContainer *>(pBroke)->setHeight(pBroke->getHeight());
	//
	// The cells are broken relative to the top of the table 
	//
	breakCellsAt(getYBottom() - iTweak);
	return pBroke;
}

UT_sint32 fp_TableContainer::tweakBrokenTable(fp_TableContainer * pBroke)
{
	fp_TableContainer * pTab = getMasterTable();
	if(!pTab)
	{
		return 0;
	}
	UT_sint32 iTweak = 0;
	fp_CellContainer * pCell = NULL;
	UT_sint32 i = 0;
	for(i =0; i<static_cast<UT_sint32>(pTab->countCons()); i++)
	{
		pCell = static_cast<fp_CellContainer *>(pTab->getNthCon(i));
		UT_sint32 iTwk = pCell->tweakBrokenTable(pBroke);
		if(iTwk > iTweak)
		{
			iTweak = iTwk;
		}
	}
	return iTweak;
}

/*!
 * Overload the setY method
 */
void fp_TableContainer::setY(UT_sint32 i)
{
	bool bIsFirstBroken = false;
	xxx_UT_DEBUGMSG(("fp_TableContainer: setY set to %d \n",i));
	if(isThisBroken())
	{
		xxx_UT_DEBUGMSG(("setY: getMasterTable %x FirstBrokenTable %x this %x \n",getMasterTable(),getMasterTable()->getFirstBrokenTable(),this));
		if(getMasterTable()->getFirstBrokenTable() != this)
		{
			xxx_UT_DEBUGMSG(("setY: Later broken table set to %d \n",i));
			fp_VerticalContainer::setY(i);
			return;
		}
		bIsFirstBroken = true;
	}
//
// Create an initial broken table if none exists
//
	if(!bIsFirstBroken && (getFirstBrokenTable() == NULL))
	{
		VBreakAt(0);
	}
	UT_sint32 iOldY = getY();
	if(i == iOldY)
	{
		return;
	}
	clearScreen();
//
// FIXME: Do I need to force another breakSection or will happen 
// automatically?
//
	xxx_UT_DEBUGMSG(("Set Reformat 1 now from table %x in TableLayout %x \n",this,getSectionLayout()));
	//	getSectionLayout()->setNeedsReformat();
	fp_VerticalContainer::setY(i);
	adjustBrokenTables();
}


void fp_TableContainer::setYBreakHere(UT_sint32 i)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Ybreak set to %d \n",i));
	m_iYBreakHere = i;
	if(i > 0)
	{
		//	UT_ASSERT(getHeight() > 0);
	}
}

void fp_TableContainer::setYBottom(UT_sint32 i)
{
	m_iYBottom = i;
	//	UT_ASSERT(getHeight() > 0);
}

/*!
 * The caller to this method requests a break at the vertical height
 * given. It returns the actual break height, which will always be
 * less than or equal to the requested height.
 */
UT_sint32 fp_TableContainer::wantVBreakAt(UT_sint32 vpos)
{
	if(isThisBroken())
	{
		return getMasterTable()->wantVBreakAt(vpos);
	}
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 iYBreak = vpos;
	fp_CellContainer * pCell;
	for(i=0; i< count; i++)
	{
		pCell = static_cast<fp_CellContainer *>(getNthCon(i));
		if(pCell->getY() <= vpos && pCell->getY() + pCell->getHeight() > vpos)
		{
			//
			// Cell overlaps break point. Find break point in the cell.
			//
			UT_sint32 iCur = pCell->wantVBreakAt(vpos);
			if(iCur < iYBreak)
			{
				iYBreak = iCur;
			}
		}
	}
	return iYBreak;
}

/*!
 * returns the first fp_Line of the table in this column by recursively 
 * searching down the table structure.
 */
fp_Line * fp_TableContainer::getFirstLineInColumn(fp_Column * pCol)
{
	fp_TableContainer * pTab = NULL;
	fp_TableContainer * pBroke = NULL;
	fp_CellContainer * pCell = NULL;
	if(!isThisBroken())
	{
		pTab = this;
	}
	else
	{
		pBroke = this;
		pTab = getMasterTable();
	}
	pCell = static_cast<fp_CellContainer *>(pTab->getNthCon(0));
	if(!pBroke)
	{
		while(pCell)
		{
			fp_Container * pFirst = static_cast<fp_Container *>(pCell->getNthCon(0));
			while(pFirst && pCell->getColumn(pFirst) != static_cast<fp_VerticalContainer *>(pCol))
			{
				pFirst = static_cast<fp_Container *>(pFirst->getNext());
			}
			if(pFirst)
			{
				if(pFirst->getContainerType() == FP_CONTAINER_LINE)
				{
					return static_cast<fp_Line *>(pFirst);
				}
				if(pFirst->getContainerType() == FP_CONTAINER_TABLE)
				{
					return static_cast<fp_TableContainer *>(pFirst)->getFirstLineInColumn(pCol);
				}
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return NULL;
			}
			pCell = static_cast<fp_CellContainer *>(pCell->getNext());
		}
		return NULL;
	}
	while(pCell)
	{
		if(pCell->doesOverlapBrokenTable(pBroke))
		{
			fp_Container * pFirst = static_cast<fp_Container *>(pCell->getNthCon(0));
			while(pFirst && pCell->getColumn(pFirst) != static_cast<fp_VerticalContainer *>(pCol))
			{
				pFirst =  static_cast<fp_Container *>(pFirst->getNext());
			}
			if(pFirst)
			{
				if(pFirst->getContainerType() == FP_CONTAINER_LINE)
				{
					return static_cast<fp_Line *>(pFirst);
				}
				if(pFirst->getContainerType() == FP_CONTAINER_TABLE)
				{
					return static_cast<fp_TableContainer *>(pFirst)->getFirstLineInColumn(pCol);
				}
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return NULL;
			}
		}
		pCell =  static_cast<fp_CellContainer *>(pCell->getNext());
	}
	return NULL;
}


/*!
 * returns the Last fp_Line of the table in this column by recursively 
 * searching down the table structure.
 */
fp_Line * fp_TableContainer::getLastLineInColumn(fp_Column * pCol)
{
	fp_TableContainer * pTab = NULL;
	fp_TableContainer * pBroke = NULL;
	fp_CellContainer * pCell = NULL;
	if(!isThisBroken())
	{
		pTab = this;
	}
	else
	{
		pBroke = this;
		pTab = getMasterTable();
	}
	UT_return_val_if_fail(pTab,NULL);
	if(pTab->countCons() == 0)
	{
		return NULL;
	}
	pCell = static_cast<fp_CellContainer *>(pTab->getNthCon(pTab->countCons()-1));
	if(!pBroke)
	{
		while(pCell)
		{
			if(pCell->countCons() > 0)
			{
				fp_Container * pLast = static_cast<fp_Container *>(pCell->getNthCon(pCell->countCons()-1));
				while(pLast && pCell->getColumn(pLast) != static_cast<fp_VerticalContainer *>(pCol))
				{
					pLast = static_cast<fp_Container *>(pLast->getPrev());
				}
				if(pLast->getContainerType() == FP_CONTAINER_LINE)
				{
					return static_cast<fp_Line *>(pLast);
				}
				if(pLast->getContainerType() == FP_CONTAINER_TABLE)
				{
					return static_cast<fp_TableContainer *>(pLast)->getLastLineInColumn(pCol);
				}
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return NULL;
			}
			pCell = static_cast<fp_CellContainer *>(pCell->getPrev());
		}
		return NULL;
	}
	while(pCell)
	{
		if(pCell->doesOverlapBrokenTable(pBroke) && (pCell->countCons() > 0))
		{
			fp_Container * pLast = static_cast<fp_Container *>(pCell->getNthCon(pCell->countCons()-1));
			while(pLast && pCell->getColumn(pLast) != static_cast<fp_VerticalContainer *>(pCol))
			{
				pLast =  static_cast<fp_Container *>(pLast->getNext());
			}
			if(pLast)
			{
				if(pLast->getContainerType() == FP_CONTAINER_LINE)
				{
					return static_cast<fp_Line *>(pLast);
				}
				if(pLast->getContainerType() == FP_CONTAINER_TABLE)
				{
					return static_cast<fp_TableContainer *>(pLast)->getLastLineInColumn(pCol);
				}
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return NULL;
			}
		}
		pCell =  static_cast<fp_CellContainer *>(pCell->getPrev());
	}
	return NULL;
}


fp_Page * fp_TableContainer::getPage(void)
{
	if(getContainer() && getContainer()->getContainerType() == FP_CONTAINER_CELL)
	{
		if(!isThisBroken())
		{
			return fp_Container::getPage();
		}
		fp_Column * pCol = getBrokenColumn();
		if(pCol)
		{
			fp_Page * pPage = pCol->getPage();
			return pPage;
		}
		if(getMasterTable() && getMasterTable()->getFirstBrokenTable() == this)
		{
			return fp_Container::getPage();
		}
		//
		// OK all the easy cases dealt with.Now we have to find the page
		// associated with this broken table.
		//
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getContainer());
		return pCell->getColumn(this)->getPage();
	}
	return fp_Container::getPage();
}

fp_Container * fp_TableContainer::getNextContainerInSection() const
{
	if(getNext())
	{
		return static_cast<fp_Container *>(getNext());
	}
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pNext = pCL->getNext();
	while(pNext && ((pNext->getContainerType() == FL_CONTAINER_ENDNOTE) ||
		  (pNext->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pNext->isHidden() == FP_HIDDEN_FOLDED)))
	{
		pNext = pNext->getNext();
	}
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_TableContainer::getPrevContainerInSection() const
{
	if(getPrev())
	{
		return static_cast<fp_Container *>(getPrev());
	}

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pPrev = pCL->getPrev();
	while(pPrev && ((pPrev->getContainerType() == FL_CONTAINER_ENDNOTE) ||
		  (pPrev->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pPrev->isHidden() == FP_HIDDEN_FOLDED)))
	{
		pPrev = pPrev->getPrev();
	}
	if(pPrev)
	{
		fp_Container * pPrevCon = static_cast<fp_Container *>(pPrev->getLastContainer());
//
// Have to handle broken tables in the previous layout..
//
		if(pPrevCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pPrevCon);
			fp_TableContainer * pLLast = pTab;
			fp_TableContainer * pNext = static_cast<fp_TableContainer *>(pTab->getNext());
			while(pNext)
			{
				pLLast = pNext;
				pNext = static_cast<fp_TableContainer *>(pNext->getNext());
			}
			pPrevCon = static_cast<fp_Container *>(pLLast);
		}
		return pPrevCon;
	}
	return NULL;
}


void fp_TableContainer::tableAttach (fp_CellContainer *child)
{
	UT_sint32 count = countCons();
	if(count > 0)
	{
		fp_Container * pLast = static_cast<fp_Container *>(getNthCon(count - 1));
		pLast->setNext(child);
		child->setPrev(pLast);
	}
    if (child->getRightAttach() >= m_iCols)
	{
		resize (m_iRows, child->getRightAttach());
	}

	if (child->getBottomAttach() >=  m_iRows)
	{
		resize (child->getBottomAttach(), m_iCols);
	}
	xxx_UT_DEBUGMSG(("tableAttach: Attaching cell %x to table \n",child));
	addContainer(child);
	child->setContainer(static_cast<fp_Container *>(this));
	queueResize();
}

void fp_TableContainer::setContainer(fp_Container * pContainer)
{
	if(isThisBroken())
	{
		fp_Container::setContainer(pContainer);
		return;
	}
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer() && (pContainer != NULL))
	{
		clearScreen();
	}
	fp_Container::setContainer(pContainer);
	fp_TableContainer * pBroke = getFirstBrokenTable();
	if(pBroke)
	{
		pBroke->setContainer(pContainer);
	}
	if(pContainer == NULL)
	{
		xxx_UT_DEBUGMSG(("Set master table %x container to NULL \n",this));
		return;
	}
	setWidth(pContainer->getWidth());
}


void fp_TableContainer::setRowSpacing (UT_sint32 row, UT_sint32  spacing)
{
  if (getNthRow(row)->spacing != spacing)
  {
      getNthRow(row)->spacing = spacing;
	  queueResize();
  }
}

void fp_TableContainer::setColSpacing(UT_sint32 column,UT_sint32 spacing)
{
  if (getNthCol(column)->spacing != spacing)
  {
      getNthCol(column)->spacing = spacing;
	  queueResize();
  }
}

void fp_TableContainer::setRowSpacings ( UT_sint32 spacing)
{
    UT_sint32 row;
	m_iRowSpacing = spacing;
	for (row = 0; row < m_iRows; row++)
	{
		getNthRow(row)->spacing = spacing;
	}
	queueResize();
}

void fp_TableContainer::setColSpacings (UT_sint32  spacing)
{
  UT_sint32 col;
  m_iColSpacing = spacing;
  for (col = 0; col < m_iCols; col++)
  {
	  getNthCol(col)->spacing = spacing;
  }
  queueResize();
}

void fp_TableContainer::setHomogeneous (bool bIsHomogeneous)
{
  if (bIsHomogeneous != m_bIsHomogeneous)
  {
      m_bIsHomogeneous = bIsHomogeneous;
	  queueResize();
  }
}

void fp_TableContainer::setBorderWidth(UT_sint32 iBorder)
{
	if(iBorder == m_iBorderWidth)
	{
		return;
	}
	m_iBorderWidth = iBorder;
	queueResize();
}

void fp_TableContainer::queueResize(void)
{
	static_cast<fl_TableLayout *>(getSectionLayout())->setDirty();
	if(getContainer() && getContainer()->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer()->getContainer());
		if(pTab && pTab->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab->queueResize();
		}
	}
}
void fp_TableContainer::layout(void)
{
	if(isThisBroken())
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	xxx_UT_DEBUGMSG(("Doing Table layout %x \n",this));
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	static fp_Requisition requisition;
	static fp_Allocation alloc;
	sizeRequest(&requisition);
	setX(m_iBorderWidth);
	alloc.x = getX();
	alloc.y = getY() + pTL->getTopOffset();
	alloc.width = getWidth();
	alloc.height = requisition.height + pTL->getTopOffset() + pTL->getBottomOffset();
	sizeAllocate(&alloc);
	setToAllocation();
}

void fp_TableContainer::setToAllocation(void)
{
	bool bDeleteBrokenTables = false;
	setWidth(m_MyAllocation.width);
	if(fp_VerticalContainer::getHeight() != m_MyAllocation.height)
	{
		xxx_UT_DEBUGMSG(("Old table Height %d New allocation %d \n",fp_VerticalContainer::getHeight(),m_MyAllocation.height));
		bDeleteBrokenTables = true;
	}
	//
	// clear and delete broken tables before their height changes.
	// Doing this clear at this point makes a table flicker when changing
	// height but it does remove the last the pixel dirt with tables.
	// 
	if(bDeleteBrokenTables)
	{
		deleteBrokenTables(true,true);
	}
	setHeight(m_MyAllocation.height);
	setMaxHeight(m_MyAllocation.height);
	xxx_UT_DEBUGMSG(("SEVIOR: Height is set to %d \n",m_MyAllocation.height));

	fp_CellContainer * pCon = static_cast<fp_CellContainer *>(getNthCon(0));
	while(pCon)
	{
		pCon->setToAllocation();
		pCon = static_cast<fp_CellContainer *>(pCon->getNext());
	}
	pCon = static_cast<fp_CellContainer *>(getNthCon(0));
	while(pCon)
	{
		pCon->setLineMarkers();
		pCon = static_cast<fp_CellContainer *>(pCon->getNext());
	}
	setYBottom(m_MyAllocation.height);
}

void  fp_TableContainer::_size_request_init(void)
{
  UT_sint32 row, col;
  
  for (row = 0; row < m_iRows; row++)
  {
	  getNthRow(row)->requisition = 0;
  }
  m_iCols =  m_vecColumns.getItemCount();
  for (col = 0; col < m_iCols; col++)
  {
	  getNthCol(col)->requisition = 0;
  }

  fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getNthCon(0));
  while (pCell)
  {
	  UT_ASSERT(pCell->getContainerType() == FP_CONTAINER_CELL);
	  pCell->sizeRequest(NULL);
	  pCell = static_cast<fp_CellContainer *>(pCell->getNext());
  }
}

void  fp_TableContainer::_drawBoundaries(dg_DrawArgs* pDA)
{
	UT_ASSERT(getPage());
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
	UT_sint32 iWidth =0;
	UT_sint32 iBorderWidth =0;
	if(isThisBroken())
	{
		iWidth = getMasterTable()->getWidth();
		iBorderWidth = getMasterTable()->m_iBorderWidth;
	}
	else
	{
		iWidth = getWidth();
		iBorderWidth = m_iBorderWidth;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff +  iWidth + 2 - static_cast<UT_sint32> (iBorderWidth * 2.0);
        UT_sint32 yoffEnd = pDA->yoff + getHeight() + 2;

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);
		xxx_UT_DEBUGMSG(("SEVIOR: Table Top (getY()) = %d \n",getY()));
		xxx_UT_DEBUGMSG(("SEVIOR: Table boundaries xleft %d xright %d ytop %d ybot %d \n",xoffBegin,xoffEnd,yoffBegin,yoffEnd));

		GR_Painter painter (getGraphics());

        painter.drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        painter.drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        painter.drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        painter.drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }

}


void  fp_TableContainer::_size_request_pass1(void)
{
  UT_sint32 width;
  UT_sint32 height;
  
  fp_CellContainer * child = static_cast<fp_CellContainer *>(getNthCon(0));
  while (child)
  {
//
// OK send down
//
	  fp_Requisition child_requisition;
	  child->sizeRequest(&child_requisition);

	  /* Child spans a single column.
	   */
	  if (child->getLeftAttach() == (child->getRightAttach() - 1))
	  {
	      width = child_requisition.width + child->getLeftPad() + child->getRightPad();
	      getNthCol(child->getLeftAttach())->requisition = UT_MAX (getNthCol(child->getLeftAttach())->requisition, width);
	  }
	  
	  /* Child spans a single row.
	   */
	  if (child->getTopAttach() == (child->getBottomAttach() - 1))
	  {
	      height = child_requisition.height + child->getTopPad() + child->getBotPad();
	      getNthRow(child->getTopAttach())->requisition = UT_MAX (getNthRow(child->getTopAttach())->requisition, height);
	  }
	  child = static_cast<fp_CellContainer *>(child->getNext());
  }
}

void  fp_TableContainer::clearScreen(void)
{
	//
	// If table is nested, do a clear screen on the topmost cell that 
	// contains it
	// This should be fixed later.
	if(getSectionLayout() && getSectionLayout()->getDocLayout())
	{
		if(getSectionLayout()->getDocLayout()->isLayoutDeleting())
		{
			return;
		}
	}
	fp_Container *pUp = getContainer();
	bool bIsNested = (pUp && (pUp->getContainerType() == FP_CONTAINER_CELL));
	if(isThisBroken()  && !bIsNested)
	{
		return;
	}
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->isLayoutFilling())
	{
		return;
	}
	UT_sint32 yoff,xoff;
	getPage()->getScreenOffsets(static_cast<fp_Container *>(this),xoff,yoff);
	if(yoff > getPage()->getHeight())
	{
		return;
	}
	xxx_UT_DEBUGMSG(("Doing clear screen on %x \n",this));
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getNthCon(0));
	while(pCell)
	{
		//		if(pCell->containsNestedTables())
		//	{
		// 	pCell->clearScreen(true);
		//  }
		//  else
		{
			pCell->clearScreen();
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	if(getSectionLayout())
	{
		getSectionLayout()->setNeedsRedraw();
		getSectionLayout()->markAllRunsDirty();
	}
}

void fp_TableContainer::draw(dg_DrawArgs* pDA)
{
//
// Don't draw if the table is still being constructed.
//
	xxx_UT_DEBUGMSG(("TablecONTAINER enter draw table %x \n",this));
	if(getSectionLayout()->getDocument()->isDontImmediateLayout())
	{
		xxx_UT_DEBUGMSG(("TablecONTAINER leave draw dont immediately layout \n"));
		return;
	}
	if(pDA->bDirtyRunsOnly)
	{
		if(getSectionLayout() && !getSectionLayout()->needsRedraw())
		{
			xxx_UT_DEBUGMSG(("TablecONTAINER leave draw section does not want redraw \n"));
//			return;
		}
	}
	if(isThisBroken())
	{
		_brokenDraw(pDA);
		return;
	}
	else if(getFirstBrokenTable() != NULL)
	{
		getFirstBrokenTable()->draw( pDA);
		return;
	}
	fp_Container * pCell = static_cast<fp_Container *>(getNthCon(0));
	while(pCell)
	{
		xxx_UT_DEBUGMSG(("!!!!!!!!!!!!Draw unbroken table cell !!!!!!!!!!\n"));
		pCell->draw(pDA);
		pCell = static_cast<fp_Container *>(pCell->getNext());
	}
    _drawBoundaries(pDA);

}

UT_sint32 fp_TableContainer::getNumRows(void) const
{
	return static_cast<UT_sint32>(m_vecRows.getItemCount());
}


UT_sint32 fp_TableContainer::getNumCols(void) const
{
	return static_cast<UT_sint32>(m_vecColumns.getItemCount());
}

/*! 
 * Return the height of this Table taking into account the possibility
 * of it being broken.
 */
UT_sint32 fp_TableContainer::getHeight(void)
{
	UT_sint32 iFullHeight =  fp_VerticalContainer::getHeight();
	if(!isThisBroken())
	{
//
// If this is a master table but it contains broken tables, we actually
// want the height of the first broken table. The Master table is the 
// one that actually has a relevant Y value in the vertical container.
// All other Y offsets from the broken tables are calculated relative to
// it.
//
		if(getFirstBrokenTable() != NULL)
		{
			return getFirstBrokenTable()->getHeight();
		}
		return iFullHeight;
	}
	UT_sint32 iMyHeight = getYBottom() - getYBreak();
	return iMyHeight;
}
/*!
 * Return true if the table contains footnote references
 */
bool fp_TableContainer::containsFootnoteReference(void)
{
	fp_Container * pCon = getFirstContainer();
	if(isThisBroken())
	{
		pCon = getMasterTable()->getFirstContainer();
	}
	bool bFound = false;
	while(pCon && !bFound)
	{
		if(pCon->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCon);
			if(pCell->containsFootnoteReference())
			{
				if(isThisBroken())
				{
//
// If broken check to see if the container containing the footnote reference
// is on screen.
//
					fp_Container * pCellCon = pCell->getFirstContainer();
					while(pCellCon && !bFound)
					{
						if(isInBrokenTable(pCell,pCellCon))
						{
							if(pCellCon->getContainerType() == FP_CONTAINER_LINE)
							{
								fp_Line * pLine = static_cast<fp_Line *>(pCellCon);
								if(pLine->containsFootnoteReference())
								{
									bFound = true;
								}
							}
							else if(pCellCon->getContainerType() == FP_CONTAINER_TABLE)
							{
								fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCellCon);
								if(pTab->containsFootnoteReference())
								{
									bFound = true;
								}
							}
						}
						pCellCon = static_cast<fp_Container *>(pCellCon->getNext());
 					}
  				}
				else
				{
					bFound = true;
				}
			}
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
			bFound = pTab->containsFootnoteReference();
		}
		pCon = static_cast<fp_Container *>(pCon->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of call the footnote layouts in this table
 */
bool fp_TableContainer::getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pVecFoots)
{
	fp_Container * pCon = getFirstContainer();
	if(isThisBroken())
	{
		pCon = getMasterTable()->getFirstContainer();
	}
	bool bFound = false;
	while(pCon)
	{
		if(pCon->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCon);
			if(pCell->containsFootnoteReference())
			{
				if(isThisBroken())
				{
//
// If broken check to see if the container containing the footnote reference
// is on screen.
//
					fp_Container * pCellCon = pCell->getFirstContainer();
					while(pCellCon)
					{
						if(isInBrokenTable(pCell,pCellCon))
						{
							if(pCellCon->getContainerType() == FP_CONTAINER_LINE)
							{
								fp_Line * pLine = static_cast<fp_Line *>(pCellCon);
								if(pLine->containsFootnoteReference())
								{
									bFound = true;
									UT_GenericVector<fp_FootnoteContainer*> vecFC;
									pLine->getFootnoteContainers(&vecFC);
									UT_uint32 i =0;
									for(i=0; i< vecFC.getItemCount();i++)
									{
										pVecFoots->addItem(vecFC.getNthItem(i));
									}
								}
							}
							else if(pCellCon->getContainerType() == FP_CONTAINER_TABLE)
							{
								fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCellCon);
								if(pTab->containsFootnoteReference())
								{
									bFound = true;
									UT_GenericVector<fp_FootnoteContainer*> vecFC;
									pTab->getFootnoteContainers(&vecFC);
									UT_uint32 i =0;
									for(i=0; i< vecFC.getItemCount();i++)
									{
										pVecFoots->addItem(vecFC.getNthItem(i));
									}
								}
							}
						}
						pCellCon = static_cast<fp_Container *>(pCellCon->getNext());
 					}
  				}
				else
				{
					UT_GenericVector<fp_FootnoteContainer*> vecFC;
					pCell->getFootnoteContainers(&vecFC);
					UT_uint32 i =0;
					for(i=0; i< vecFC.getItemCount();i++)
					{
						pVecFoots->addItem(vecFC.getNthItem(i));
					}
					bFound = true;
				}
			}
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
			bFound = pTab->containsFootnoteReference();
			if(bFound)
			{
				UT_GenericVector<fp_FootnoteContainer*> vecFC;
				pTab->getFootnoteContainers(&vecFC);
				UT_uint32 i =0;
				for(i=0; i< vecFC.getItemCount();i++)
				{
					pVecFoots->addItem(vecFC.getNthItem(i));
				}
			}
		}
		pCon = static_cast<fp_Container *>(pCon->getNext());
	}
	return bFound;

}

/*!
 * Return true if the supplied Cell and it's container are within this
 * broken container.
 */
bool fp_TableContainer::isInBrokenTable(fp_CellContainer * pCell, fp_Container * pCon)
{
//
// OK A container in a cell is allowed in this broken table if it's
// Y location plus height lie between getYBreak() and getYBottom.
//
// If the container starts within the table but it's height is not 
// contained in the table it is not allowed in here unless it is the only
// container in the cell in the table. If this the case the rest of the
// drawing code in AbiWord will have to clip it on the bottom.
//
// We don't assume that this is the top level table
//
	//
	// Short circuit things if the BrokenContainer pointer is set.
    //
 	if(pCon->getMyBrokenContainer() == static_cast<fp_Container *>(this))
 	{
 		return true;
 	}
 	if(pCon->getMyBrokenContainer() != NULL)
 	{
 		return false;
 	}
	UT_sint32 iTop = 0;
	iTop = pCell->getY() + pCon->getY();
	UT_sint32 iHeight = pCon->getHeight();
	if(pCon->getContainerType() == FP_CONTAINER_TABLE)
	{
		fp_TableContainer * ppTab = static_cast<fp_TableContainer *>(pCon);
		xxx_UT_DEBUGMSG(("Nested table Y %d height %d yBottom %d contained in broken table height %d \n",ppTab->getY(),ppTab->getHeight(),ppTab->getYBottom(),getHeight()));
		ppTab = static_cast<fp_TableContainer *>(ppTab->getNext());
		if(ppTab)
		{
			xxx_UT_DEBUGMSG(("Next table yBreak %d \n",ppTab->getYBreak()));
		}
		iHeight = static_cast<fp_TableContainer *>(pCon)->getHeight();
	}
	UT_sint32 iBot = iTop + iHeight;

	UT_sint32 iBreak = getYBreak();
	UT_sint32 iBottom = getYBottom();
	xxx_UT_DEBUGMSG(("Column %x iTop = %d ybreak %d iBot= %d ybottom= %d \n",getBrokenColumn(),iTop,iBreak,iBot,iBottom));
	if(iBot >= iBreak)
	{
		if(iBot < iBottom)
		{
			//			pCon->setMyBrokenContainer(this);
			return true;
		}

	}
	return false;
}	


/*!
 * Draw that segment of the table that fits within the Y offsets of this
 * Broken table.
 */
void fp_TableContainer::_brokenDraw(dg_DrawArgs* pDA)
{
	bool bIsNested = false;
	UT_sint32 iMyY = 0;
	fp_TableContainer *pMaster = getMasterTable();
	UT_sint32 iCount = 0;
	fp_TableContainer * pPrev = this;
	while(pPrev != NULL)
		{
			iCount++;
			pPrev = static_cast<fp_TableContainer *>(pPrev->getPrev());
		}
	xxx_UT_DEBUGMSG(("Drawing %d table in broken chain \n",iCount));
	if(iCount > 0)
	{
		xxx_UT_DEBUGMSG(("Draw the repeated Row here \n"));
	}
	if(getContainer()->getContainerType() == FP_CONTAINER_CELL)
	{
		xxx_UT_DEBUGMSG(("Draw broken nested Table %x yBreak %d \n",this,getYBreak()));
		bIsNested = true;
		if(getYBreak() > 0)
		{
			iMyY = getY();
		}
	}
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getMasterTable()->getNthCon(0));
	xxx_UT_DEBUGMSG(("SEVIOR: _brokenDraw table %x getYBreak %d getYBottom %d \n",this, getYBreak(),getYBottom()));
    xxx_UT_DEBUGMSG(("SEVIOR: _brokenDraw table Initial Y-offset %d \n",pDA->yoff));
	UT_sint32 iCountCells = 0;
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	fp_TableContainer * pBroke = const_cast<fp_TableContainer *>(this);
	bool bDirtyOnly = pDA->bDirtyRunsOnly;
	if(m_pFirstBrokenCell)
	{
		pCell = m_pFirstBrokenCell;
		xxx_UT_DEBUGMSG(("Starting at cell %x \n",pCell));
		while(pCell)
		{
			xxx_UT_DEBUGMSG(("Look at Cell %x isdirty %d \n",pCell,pCell->isDirty()));
			if(pCell->containsNestedTables())
				{	
				xxx_UT_DEBUGMSG(("SEVIOR: 2 Cell Contains nested tables cell %x cellY %d cellHeight %d YBreak %d yBottom %d \n",pCell,pCell->getY(), pCell->getHeight(), getYBreak(),getYBottom()));
				}	
			dg_DrawArgs da = *pDA;
			if(!bIsNested)
			{
				da.yoff = da.yoff - getYBreak();
			}
			if(bDirtyOnly)
			{
				if(pCell->containsNestedTables())
				{
					xxx_UT_DEBUGMSG(("da dirty only, pCell->isDirty %d needsredraw %d \n",pCell->isDirty(), pCell->getSectionLayout()->needsRedraw()));
				}
				if(pCell->isDirty() || pCell->getSectionLayout()->needsRedraw())
				{
					pCell->drawBroken(&da, this);
					iCountCells++;
				}
			}
			else
			{
				if(pClipRect)
				{
					if(pCell->doesIntersectClip(pBroke,pClipRect))
				    {
						pCell->drawBroken(&da, this);
						iCountCells++;
					}
				}
				else
				{
					pCell->drawBroken(&da, this);
					iCountCells++;
				}
			}
			if(pCell != m_pLastBrokenCell)
			{
				pCell = static_cast<fp_CellContainer *>(pCell->getNext());
			}
			else
			{
				pCell = NULL;
			}
		}
	}
	else
	{
		while(pCell)
		{
			UT_sint32 botY = 0;
			if(pCell->containsNestedTables())
				{
					xxx_UT_DEBUGMSG(("SEVIOR: 1 Cell Contains nested tables cell %x cellY %d cellHeight %d YBreak %d yBottom %d \n",pCell,pCell->getY(), pCell->getHeight(), getYBreak(),getYBottom()));
				}	
			if(pCell->getBottomAttach() < pMaster->getNumRows())
			{
				botY = pMaster->getYOfRow(pCell->getBottomAttach());
			}
			else
			{
				fp_VerticalContainer * pVert = static_cast<fp_VerticalContainer *>(pMaster);
				botY = pMaster->getYOfRow(0) + pVert->getHeight();
				botY -= 2 * pMaster->getBorderWidth();
				botY +=  pMaster->getNthRow(pMaster->getNumRows()-1)->spacing/2;
			}

			if(pCell->getY() > getYBottom())
			{
				xxx_UT_DEBUGMSG(("SEVIOR: _drawBroken skipping cell %x cellY %d cellHeight %d YBreak %d yBottom %d \n",pCell,pCell->getY(), pCell->getHeight(), getYBreak(),getYBottom()));
				break;
				pCell = static_cast<fp_CellContainer *>(pCell->getNext());
			}
			else
			{
				dg_DrawArgs da = *pDA;
				if(pCell->containsNestedTables())
					{
						xxx_UT_DEBUGMSG(("SEVIOR: drawing cell with nested tables yoff %d cellY %d cellHeight %d YBreak %d yBottom %d \n",da.yoff,pCell->getY(), pCell->getHeight(), getYBreak(),getYBottom()));
					}
				da.yoff = da.yoff - getYBreak();
				iCountCells++;
				pCell->drawBroken(&da, this);
				if(m_pFirstBrokenCell == NULL)
				{
					m_pFirstBrokenCell = pCell;
				}
				m_pLastBrokenCell = pCell;
				pCell = static_cast<fp_CellContainer *>(pCell->getNext());
			}
		}
	}
	xxx_UT_DEBUGMSG(("_brokenDraw: Draw %d cells \n",iCountCells));
    _drawBrokenBoundaries(pDA);
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	pTL->clearNeedsRedraw();
}


void fp_TableContainer::_drawBrokenBoundaries(dg_DrawArgs* pDA)
{

	UT_ASSERT(getPage());
	if(!pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff + getX();
        UT_sint32 yoffBegin = pDA->yoff;
        UT_sint32 xoffEnd = pDA->xoff + getX() + getWidth() - getGraphics()->tlu(1);
        UT_sint32 yoffEnd = pDA->yoff + getHeight() - getGraphics()->tlu(1);

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);
		xxx_UT_DEBUGMSG(("SEVIOR: Table Top (getY()) = %d \n",getY()));
		xxx_UT_DEBUGMSG(("SEVIOR: Table boundaries xleft %d xright %d ytop %d ybot %d \n",xoffBegin,xoffEnd,yoffBegin,yoffEnd));

		GR_Painter painter (getGraphics());

        painter.drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        painter.drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        painter.drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        painter.drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }

}

void  fp_TableContainer::_size_request_pass2(void)
{
  UT_sint32 max_width;
   UT_sint32 col;
  
  if (m_bIsHomogeneous)
  {
      max_width = 0;
      m_iCols = m_vecColumns.getItemCount();
      for (col = 0; col < m_iCols; col++)
	  {
		  max_width = UT_MAX (max_width, getNthCol(col)->requisition);
	  }
      for (col = 0; col < m_iCols; col++)
	  {
		  getNthCol(col)->requisition = max_width;
	  }
//
// Don't want homogeneous in height
//
#if 0
      UT_sint32 max_height = 0;
	  UT_sint32 row = 0;
	  for (row = 0; row < m_iRows; row++)
	  {
		  max_height = UT_MAX (max_height, getNthRow(row)->requisition);
      }
      for (row = 0; row < m_iRows; row++)
	  {
		  getNthRow(row)->requisition = max_height;
	  }
#endif
  }
}

void  fp_TableContainer::_size_request_pass3(void)
{
  fp_CellContainer  *child;
  UT_sint32 width, height;
  UT_sint32 row, col;
  UT_sint32 extra;
  
  child = static_cast<fp_CellContainer *>(getNthCon(0));
  while (child)
  {
	  /* Child spans multiple columns.
	   */
	  if (child->getLeftAttach() != (child->getRightAttach() - 1))
	  {
	      fp_Requisition child_requisition;

	      child->sizeRequest(&child_requisition);
	      
	      /* Check and see if there is already enough space
	       *  for the child.
	       */
	      width = 0;
	      for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
		  {
			  width += getNthCol(col)->requisition;
			  if ((col + 1) < child->getRightAttach())
			  {
				  width += getNthCol(col)->spacing;
			  }
		  }
	      
	      /* If we need to request more space for this child to fill
	       *  its requisition, then divide up the needed space evenly
	       *  amongst the columns it spans.
	       */
	      if (width < child_requisition.width + child->getLeftPad() + child->getRightPad())
		  {
			  width = child_requisition.width + child->getLeftPad() + child->getRightPad();
		  
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  extra = width / (child->getRightAttach() - col);
				  getNthCol(col)->requisition += extra;
				  width -= extra;
			  }
		  }
	  }
	  
	  /* Child spans multiple rows.
	   */
	  if (child->getTopAttach() != (child->getBottomAttach() - 1))
	  {
	      fp_Requisition child_requisition;

	      child->sizeRequest(&child_requisition);

	      /* Check and see if there is already enough space
	       *  for the child.
	       */
	      height = 0;
	      for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
		  {
			  height += getNthRow(row)->requisition;
			  if ((row + 1) < child->getBottomAttach())
				  height +=  getNthRow(row)->spacing;
		  }
	      
	      /* If we need to request more space for this child to fill
	       *  its requisition, then divide up the needed space evenly
	       *  amongst the columns it spans.
	       */
	      if (height < child_requisition.height + child->getTopPad() + child->getBotPad())
		  {
			  height = child_requisition.height + child->getTopPad() + child->getBotPad() - height;
		  
			  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
			  {
				  extra = height / (child->getBottomAttach() - row);
				  getNthRow(row)->requisition += extra;
				  height -= extra;
			  }
		  }
	  }
	  child = static_cast<fp_CellContainer *>(child->getNext());
    }
}

void  fp_TableContainer::_size_allocate_init(void)
{
  fp_CellContainer * child;
  UT_sint32 row, col;
  UT_sint32 has_expand;
  UT_sint32 has_shrink;
  
  /* Initialize the rows and cols.
   *  By default, rows and cols do not expand and do shrink.
   *  Those values are modified by the children that occupy
   *  the rows and cols.
   */
  m_iCols = m_vecColumns.getItemCount();
  for (col = 0; col < m_iCols; col++)
  {
	  fp_TableRowColumn *pCol= getNthCol(col);

      pCol->allocation = pCol->requisition;
      pCol->need_expand = false;
      pCol->need_shrink = true;
      pCol->expand = false;
      pCol->shrink = true;
      pCol->empty = true;
  }
  for (row = 0; row < m_iRows; row++)
  {
	  fp_TableRowColumn * pRow = getNthRow(row);
      pRow->allocation = pRow->requisition;
      pRow->need_expand = false;
      pRow->need_shrink = true;
      pRow->expand = false;
      pRow->shrink = true;
      pRow->empty = true;
  }
  
  /* Loop over all the children and adjust the row and col values
   *  based on whether the children want to be allowed to expand
   *  or shrink. This loop handles children that occupy a single
   *  row or column.
   */
  child = static_cast<fp_CellContainer *>(getNthCon(0));
  while (child)
  {
	  if (child->getLeftAttach() == (child->getRightAttach() - 1))
	  {
		  if (child->getXexpand())
		  {
			  getNthCol(child->getLeftAttach())->expand = true;
		  }
		  if (!child->getXshrink())
		  {
			  getNthCol(child->getLeftAttach())->shrink = false;
		  }
		  getNthCol(child->getLeftAttach())->empty = false;
	  }
	  
	  if (child->getTopAttach() == (child->getBottomAttach() - 1))
	  {
		  if (child->getYshrink())
		  {
			  getNthRow(child->getTopAttach())->expand = true;
		  }
		  if (!child->getYshrink())
		  {			
			  getNthRow(child->getTopAttach())->shrink = false;
		  }
		  getNthRow(child->getTopAttach())->empty = false;
	  }
	  child = static_cast<fp_CellContainer *>(child->getNext());
  }
  
  /* Loop over all the children again and this time handle children
   *  which span multiple rows or columns.
   */
  child = static_cast<fp_CellContainer *>(getNthCon(0));
  while (child)
  {
	  if (child->getLeftAttach() != (child->getRightAttach() - 1))
	  {
		  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
		  {
			  getNthCol(col)->empty = false;
		  }
		  if (child->getXexpand())
		  {
			  has_expand = false;
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  if (getNthCol(col)->expand)
				  {
					  has_expand = true;
					  break;
				  }
			  }
			  if (!has_expand)
			  {
				  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
				  {
					  getNthCol(col)->need_expand = true;
				  }
			  }
		  }
			  
		  if (!child->getXshrink())
		  {
			  has_shrink = true;
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  if (!getNthCol(col)->shrink)
				  {
					  has_shrink = false;
					  break;
				  }
			  }
			  if (has_shrink)
			  {
				  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
				  {
						  getNthCol(col)->need_shrink = false;
				  }
			  }
		  }
	  
		  if (child->getTopAttach() != (child->getBottomAttach() - 1))
		  {
			  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
			  {
				  getNthRow(row)->empty = false;
			  }
			  if (child->getYexpand())
			  {
				  has_expand = false;
				  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
				  {
					  if (getNthRow(row)->expand)
					  {
						  has_expand = true;
						  break;
					  }
				  }		  
				  if (!has_expand)
				  {
					  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
						{
							getNthRow(row)->need_expand = true;
						}
				  }
			  }
	      
			  if (!child->getYshrink())
			  {
				  has_shrink = true;
				  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
				  {
					  if (!getNthRow(row)->shrink)
					  {
						  has_shrink = false;
						  break;
					  }
				  }
				  if (has_shrink)
				  {
					  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
					  {
						  getNthRow(row)->need_shrink = false;
					  }
				  }
			  }
		  }
	  }
	  child = static_cast<fp_CellContainer *>(child->getNext());
  }
  
  /* Loop over the columns and set the expand and shrink values
   *  if the column can be expanded or shrunk.
   */

  m_iCols = m_vecColumns.getItemCount();
  for (col = 0; col < m_iCols; col++)
  {
	  fp_TableRowColumn *pCol= getNthCol(col);

      if (pCol->empty)
	  {
		  pCol->expand = false;
		  pCol->shrink = false;
	  }
      else
	  {
		  if (pCol->need_expand)
		  {
			  pCol->expand = true;
		  }
		  if (!pCol->need_shrink)
		  {
			  pCol->shrink = false;
		  }
	  }
  }
  
  /* Loop over the rows and set the expand and shrink values
   *  if the row can be expanded or shrunk.
   */
  for (row = 0; row < m_iRows; row++)
  {
	  fp_TableRowColumn * pRow = getNthRow(row);
      if (pRow->empty)
	  {
		  pRow->expand = false;
		  pRow->shrink = false;
	  }
      else
	  {
		  if (pRow->need_expand)
		  {
			  pRow->expand = true;
		  }
		  if (!pRow->need_shrink)
		  {
			  pRow->shrink = false;
		  }
	  }
  }
}

void  fp_TableContainer::_size_allocate_pass1(void)
{

  UT_sint32 real_width;
  UT_sint32 real_height;
  UT_sint32 width, height;
  UT_sint32 row, col;
  UT_sint32 nexpand;
  UT_sint32 nshrink;
  UT_sint32 extra;
  
  /* If we were allocated more space than we requested
   *  then we have to expand any expandable rows and columns
   *  to fill in the extra space.
   */
  
  real_width = m_MyAllocation.width - m_iBorderWidth * 2;
  double dHeight = static_cast<double>(m_MyAllocation.height);
  double dBorder = static_cast<double>(m_iBorderWidth);
  real_height = static_cast<UT_sint32> (dHeight - dBorder * 2.0);
  // real_height =  m_MyAllocation.height;
  
  if (m_bIsHomogeneous)
  {
      nexpand = 0;
	  m_iCols = m_vecColumns.getItemCount();
      for (col = 0; col < m_iCols; col++)
	  {
		  if (getNthCol(col)->expand)
		  {
			  nexpand += 1;
			  break;
		  }
	  }
      
      if (nexpand > 0)
	  {
		  width = real_width;
	  
		  for (col = 0; col + 1 < m_iCols; col++)
		  {
			  width -= getNthCol(col)->spacing;
		  }
	  
		  for (col = 0; col < m_iCols; col++)
		  {
			  extra = width / (m_iCols - col);
			  getNthCol(col)->allocation = UT_MAX (1, extra);
			  width -= extra;
		  }
	  }
  }
  else
  {
      width = 0;
      nexpand = 0;
      nshrink = 0;
      
	  m_iCols = m_vecColumns.getItemCount();
      for (col = 0; col < m_iCols; col++)
	  {
		  width += getNthCol(col)->requisition;
		  if (getNthCol(col)->expand)
		  {
			  nexpand += 1;
		  }
		  if (getNthCol(col)->shrink)
		  {
			  nshrink += 1;
		  }
	  }
      for (col = 0; col + 1 < m_iCols; col++)
	  {
		  width += getNthCol(col)->spacing;
      }
      /* Check to see if we were allocated more width than we requested.
       */
      if ((width < real_width) && (nexpand >= 1))
	  {
		  width = real_width - width;
	  
		  for (col = 0; col < m_iCols; col++)
		  {
			  if (getNthCol(col)->expand)
			  {
				  extra = width / nexpand;
				  getNthCol(col)->allocation += extra;
				  width -= extra;
				  nexpand -= 1;
			  }
		  }
	  }
      
      /* Check to see if we were allocated less width than we requested,
       * then shrink until we fit the size give.
       */
      if (width > real_width)
	  {
		  UT_sint32 total_nshrink = nshrink;

		  extra = width - real_width;
		  while (total_nshrink > 0 && extra > 0)
		  {
			  nshrink = total_nshrink;
			  m_iCols = m_vecColumns.getItemCount();
			  for (col = 0; col < m_iCols; col++)
			  {
				  fp_TableRowColumn *pCol= getNthCol(col);

				  if (pCol->shrink)
				  {
					  UT_sint32 allocation = pCol->allocation;
					  pCol->allocation = UT_MAX (1, static_cast<UT_sint32>(pCol->allocation) - extra / nshrink);
					  extra -= allocation - pCol->allocation;
					  nshrink -= 1;
					  if (pCol->allocation < 2)
					  {
						  total_nshrink -= 1;
						  pCol->shrink = false;
					  }
				  }
			  }
		  }
	  }
  }
  
//
// Don't want homogenous in height
//
  if (m_bIsHomogeneous && false)
  {
	  nexpand = 0;
	  for (row = 0; row < m_iRows; row++)
	  {
		  if (getNthRow(row)->expand)
		  {
			  nexpand += 1;
			  break;
		  }
	  }
      
	  if (nexpand > 0)
	  {
		  height = real_height;
		  for (row = 0; row + 1 < m_iRows; row++)
		  {
			  height -= getNthRow(row)->spacing;
		  }
		  for (row = 0; row < m_iRows; row++)
		  {
			  extra = height / (m_iRows - row);
			  getNthRow(row)->allocation = UT_MAX (1, extra);
			  height -= extra;
		  }
	  }
  }
  else
  {
	  height = 0;
	  nexpand = 0;
	  nshrink = 0;
	  for (row = 0; row < m_iRows; row++)
	  {


		  height += getNthRow(row)->requisition;
		  if (getNthRow(row)->expand)
		  {
			  nexpand += 1;
		  }
		  if (getNthRow(row)->shrink)
		  {
			  nshrink += 1;
		  }
	  }
	  for (row = 0; row + 1 < m_iRows; row++)
	  {
		  height += getNthRow(row)->spacing;
	  }      
      /* Check to see if we were allocated more height than we requested.
       */
      if ((height < real_height) && (nexpand >= 1))
	  {
		  height = real_height - height;
		  for (row = 0; row < m_iRows; row++)
		  {
			  if (getNthRow(row)->expand)
			  {
				  extra = height / nexpand;
				  getNthRow(row)->allocation += extra;
				  height -= extra;
				  nexpand -= 1;
			  }
		  }
	  }
      
      /* Check to see if we were allocated less height than we requested.
       * then shrink until we fit the size give.
       */
      if (height > real_height)
	  {
		  UT_sint32 total_nshrink = nshrink;
		  extra = height - real_height;
		  while (total_nshrink > 0 && extra > 0)
		  {
			  nshrink = total_nshrink;
			  for (row = 0; row < m_iRows; row++)
			  {
				  fp_TableRowColumn * pRow = getNthRow(row);
				  if (pRow->shrink)
				  {
					  UT_sint32 allocation = pRow->allocation;
		    
					  pRow->allocation = UT_MAX (1, static_cast<UT_sint32>(pRow->allocation) - extra / nshrink);
					  extra -= allocation - pRow->allocation;
					  nshrink -= 1;
					  if (pRow->allocation < 2)
					  {
						  total_nshrink -= 1;
						  pRow->shrink = false;
					  }
				  }
			  }
		  }
	  }
  }
}

void  fp_TableContainer::_size_allocate_pass2(void)
{
  fp_CellContainer  *child;
  UT_sint32 max_width;
  UT_sint32 max_height;
  UT_sint32 x, y;
  UT_sint32 row, col;
  fp_Allocation allocation;
  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
  const UT_GenericVector<fl_ColProps*> * pVecColProps = pTL->getVecColProps();
  if(pVecColProps->getItemCount() > 0)
  {
	  for (col = 0; (col < static_cast<UT_sint32>(pVecColProps->getItemCount())) && (col <getNumCols()); col++)
	  {
		  fl_ColProps * pColProp = pVecColProps->getNthItem(col);
		  getNthCol(col)->allocation = pColProp->m_iColWidth - getNthCol(col)->spacing;
		  if(col == (getNumCols() - 1) )
		  {
			  getNthCol(col)->allocation += 2 * getNthCol(col)->spacing;
		  }
		  xxx_UT_DEBUGMSG(("Sevior: table %x column %d set to width %d spacing %d pColProp width %d \n",this,col,getNthCol(col)->allocation,getNthCol(col)->spacing,pColProp->m_iColWidth));
	  }
  }
  m_MyAllocation.x = pTL->getLeftColPos() - m_iBorderWidth;
  
  child = static_cast<fp_CellContainer *>(getNthCon(0));
  double dBorder = static_cast<double>(m_iBorderWidth);
  while (child)
  {
	  fp_Requisition child_requisition;
	  child->sizeRequest(&child_requisition);

	  x = m_MyAllocation.x + m_iBorderWidth;
	  double dy = static_cast<double>(m_MyAllocation.y); 
	  xxx_UT_DEBUGMSG(("SEVIOR: Vertical border %f \n", dBorder));
	  y = static_cast<UT_sint32> (dy + dBorder);

	  xxx_UT_DEBUGMSG(("SEVIOR: 1 initial y %d for top-attach %d \n",y,child->getTopAttach()));
	  max_width = 0;
	  max_height = 0;
	  
	  for (col = 0; col < child->getLeftAttach(); col++)
	  {
		  x += getNthCol(col)->allocation;
		  x += getNthCol(col)->spacing;
	  }
	  
	  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
	  {
		  max_width += getNthCol(col)->allocation;
		  if ((col + 1) < child->getRightAttach())
		  {
			  max_width += getNthCol(col)->spacing;
		  }
	  }
	  
	  for (row = 0; row < child->getTopAttach(); row++)
	  {
		  fp_TableRowColumn * pRow = getNthRow(row);
		  UT_continue_if_fail(pRow);

		  UT_sint32 iOldAlloc = pRow->allocation;
		  UT_sint32 iNewAlloc = getRowHeight(row,iOldAlloc);
		  if(iNewAlloc > iOldAlloc)
		  {
			  iNewAlloc -= pRow->spacing;
		  }
		  pRow->allocation = iNewAlloc;
		  y += pRow->allocation;
		  y += pRow->spacing;
		  xxx_UT_DEBUGMSG(("SEVIOR: Adding Height %d spacing is %d \n", getNthRow(row)->allocation,getNthRow(row)->spacing));
	  }
	  xxx_UT_DEBUGMSG(("SEVIOR: 2 next y %d for top-attach %d \n",y,child->getTopAttach()));
	  
	  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
	  {
		  max_height += getNthRow(row)->allocation;
		  if ((row + 1) < child->getBottomAttach())
		  {
			  max_height += getNthRow(row)->spacing;
		  }
	  }
	  
	  if (child->getXfill())
	  {
		  allocation.width = UT_MAX (1, max_width - static_cast<UT_sint32>(child->getLeftPad()) - child->getRightPad());
		  allocation.x = x + (max_width - allocation.width) / 2;
	  }
	  else
	  {
		  allocation.width = child_requisition.width;
		  allocation.x = x + (max_width - allocation.width) / 2;
	  }
	  // fixme sevior look here!!!
 	  if (child->getYfill())
 	  {
 		  allocation.height = UT_MAX (1, max_height - static_cast<UT_sint32>(child->getTopPad()) - child->getBotPad());
 		  allocation.y = y;
 	  }
 	  else
	  {
		  allocation.height = child_requisition.height;
		  allocation.y = y;
		  xxx_UT_DEBUGMSG(("SEVIOR: top attach %d height %d  y %d \n",child->getTopAttach(),allocation.height,allocation.y));
	  }
	  xxx_UT_DEBUGMSG(("SEVIOR!!!!!!: max_height = %d width =%d \n",max_height,allocation.width));
	  child->sizeAllocate( &allocation);
	  child = static_cast<fp_CellContainer *>(child->getNext());
  }
}

fp_TableRowColumn * fp_TableContainer::getNthCol(UT_sint32 i)
{
	UT_ASSERT(i < static_cast<UT_sint32>(m_vecColumns.getItemCount()));
	return m_vecColumns.getNthItem(i);
}

fp_TableRowColumn * fp_TableContainer::getNthRow(UT_sint32 i)
{
	UT_ASSERT(i < static_cast<UT_sint32>(m_vecRows.getItemCount()));
	return m_vecRows.getNthItem(i);
}


void fp_TableContainer::sizeRequest(fp_Requisition * pRequisition)
{
  UT_sint32 row, col;
  
  pRequisition->width = 0;
  pRequisition->height = 0;
  bool bDefinedColWidth = false;
  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
  const UT_GenericVector<fl_ColProps *> * pVecColProps = pTL->getVecColProps();
  if(pVecColProps->getItemCount() > 0)
  {
	  bDefinedColWidth = true;
  }
  _size_request_init ();
  _size_request_pass1 ();
  _size_request_pass2 ();
  _size_request_pass3 ();
  _size_request_pass2 ();
  
  m_iCols = m_vecColumns.getItemCount();
  for (col = 0; col < m_iCols; col++)
  {
	  if(bDefinedColWidth && (col < static_cast<UT_sint32>(pVecColProps->getItemCount())) )
	  {
		  fl_ColProps * pColProp = pVecColProps->getNthItem(col);
		  getNthCol(col)->requisition = pColProp->m_iColWidth;
	  }
	  pRequisition->width += getNthCol(col)->requisition;
  }
  for (col = 0; col + 1 < m_iCols; col++)
  {
	  pRequisition->width += getNthCol(col)->spacing;
  }
//  for (row = 0; row + 1 < m_iRows; row++)
  for (row = 0; row < m_iRows; row++)
  {
	  fp_TableRowColumn * pRow = getNthRow(row);
	  UT_sint32 iOldReq = pRow->requisition;
	  UT_sint32 iNewReq = getRowHeight(row,iOldReq);
	  if(iNewReq > iOldReq)
	  {
		  iNewReq -= pRow->spacing;
	  }
	  pRow->requisition = iNewReq;
	  pRequisition->height += pRow->spacing;

	  xxx_UT_DEBUGMSG(("SEVIOR: requisition spacing 2 is %d \n", getNthRow(row)->spacing));
	  xxx_UT_DEBUGMSG(("SEVIOR: requisition height 2 is %d \n", pRequisition->height));
  }
  for (row = 0; row < m_iRows; row++)
  {
	  pRequisition->height += getNthRow(row)->requisition;
	  xxx_UT_DEBUGMSG(("SEVIOR: requisition height %d \n", pRequisition->height));
  }
  pRequisition->height += 2 * m_iBorderWidth;
}

void fp_TableContainer::sizeAllocate(fp_Allocation * pAllocation)
{
	m_MyAllocation.width = pAllocation->width; 
	m_MyAllocation.height = pAllocation->height;
	m_MyAllocation.x = pAllocation->x;
	m_MyAllocation.y = pAllocation->y;
	m_MyAllocation.y = 0;
	xxx_UT_DEBUGMSG(("SEVIOR: Initial allocation height is %d \n", pAllocation->height));
	
	_size_allocate_init ();
	xxx_UT_DEBUGMSG(("SEVIOR: Initial allocation height 1 is %d \n", m_MyAllocation.height));
	_size_allocate_pass1 ();
	xxx_UT_DEBUGMSG(("SEVIOR: Initial allocation height 2 is %d \n", m_MyAllocation.height));
	_size_allocate_pass2 ();
	xxx_UT_DEBUGMSG(("SEVIOR: Initial allocation height 3 is %d \n", m_MyAllocation.height));
//	fp_Requisition pReq;
//	sizeRequest(&pReq);
//	m_MyAllocation.height = pReq.height;
}
