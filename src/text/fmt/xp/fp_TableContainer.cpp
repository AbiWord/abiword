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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

//
// Uncomment to benchmark our table layout time with release builds
//#define BENCHLAYOUT 1
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "fp_TableContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fv_View.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_std_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_FootnoteLayout.h"
#include "fp_FootnoteContainer.h"
#include "fp_FrameContainer.h"
#include "gr_Painter.h"
#include "pd_Document.h"
#if BENCHLAYOUT
#include <time.h>
#endif

fp_TableRowColumn::fp_TableRowColumn(UT_sint32 defaultSpacing) :
		requisition(0),
		allocation(0),
		spacing(defaultSpacing),
		position(0),
		need_expand(false),
		need_shrink(false),
		expand(false),
		shrink(false),
		empty(true)
{
}

fp_TableRowColumn::~fp_TableRowColumn(void)
{
}

bool fp_TableRowColumn::comparePosition(UT_sint32 y, const fp_TableRowColumn * pRow)
{
	return (y < pRow->position);
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
	  m_bIsRepeated(false),
	  m_iVertAlign(0)
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
fp_TableContainer * fp_CellContainer::getBrokenTable(const fp_Container * pCon) const
{
	fp_TableContainer * pMaster = static_cast<fp_TableContainer *>(getContainer());
	if(!pMaster)
	{
		return NULL;
	}
	fp_TableContainer * pBroke = pMaster->getFirstBrokenTable();
	if (!pBroke)
	{
		return pMaster;
	}

	UT_sint32 yPos = getY() + pCon->getY() + 1;
	while(pBroke && (pBroke->getYBottom() < yPos))
    {
		pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
	}

	return ((pBroke) ? pBroke : pMaster);
}

/*!
 * This Method returns the column or shadow that embeds the container given.
 */
fp_VerticalContainer * fp_CellContainer::getColumn(const fp_Container * _pCon) const
{
	fp_TableContainer * pBroke = getBrokenTable(_pCon);
	if(pBroke == NULL)
	{
		return NULL;
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
		if(pCon == NULL)
		{
			return NULL;
		}
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
			pCell = static_cast<fp_CellContainer *>(pCon);
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
		if (!pCol)
		{
			return NULL;
		}
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

	return static_cast<fp_VerticalContainer *>(pCol);
}

bool fp_CellContainer::containsNestedTables(void) const
{
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
	return (pCL->getNumNestedTables() > 0);
}

/*!
 * Return the screen rectangle that is the intersection of the cell and the
 * broken table.
 * If the height or width is negative, the cell is not in the broken table.
 */
void fp_CellContainer::_getBrokenRect(fp_TableContainer * pBroke, fp_Page * &pPage, UT_Rect &bRec, GR_Graphics * pG) const
{
	UT_ASSERT(getSectionLayout()->myContainingLayout() &&
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
				fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pBroke->getContainer());
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
			const fp_Container * pCon = static_cast<const fp_Container *>(this);
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
	xxx_UT_DEBUGMSG(("_getBrokenRect Returned Top %d height = %d \n",iTop,iBot-iTop));
	bRec = UT_Rect(iLeft,iTop,iRight-iLeft,iBot-iTop);
}


/*! 
 * Returns true if the cell in a broken table overlaps the supplied clip Rect
 */
bool fp_CellContainer::doesIntersectClip(fp_TableContainer * pBroke, const UT_Rect * rClip) const
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
	if(pUpCon->getY() == INITIAL_OFFSET)
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
			for(i=0; i< countCons(); i++)
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


UT_sint32 fp_CellContainer::getSpannedHeight(void) const
{
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return 0;
	}
	fp_CellContainer * pCell2 = pTab->getCellAtRowColumn(getBottomAttach(),getLeftAttach());
	UT_sint32 height = 0;
	if(pCell2)
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
	if(!pBroke->getPage() || !pBroke->getPage()->isOnScreen())
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
	UT_sint32 onePix = getGraphics()->tlu(1)+1;
	
	//
	// This fix makes win32 look bad. FIXME fix this later
	//
	onePix = 0.0;
	if((bRec.top + bRec.height) < 0)
	{
		return;
	}
	lineTop.m_thickness += 3*onePix;
	lineLeft.m_thickness += 3*onePix;
	lineRight.m_thickness += 3*onePix;
	lineBottom.m_thickness += 3*onePix;
	UT_RGBColor pageCol(255,255,255);
	if(pPage)
	{
		pageCol = *pPage->getFillType().getColor();
	}
	markAsDirty();
	if (pPage != NULL)
	{
		xxx_UT_DEBUGMSG(("_clear: top %d bot %d cell left %d top %d \n",bRec.top,bRec.top+bRec.height,m_iLeftAttach,m_iTopAttach));

		lineLeft.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineLeft.m_color = pageCol;
		drawLine (lineLeft, bRec.left, bRec.top, bRec.left,  bRec.top + bRec.height,getGraphics());

		lineTop.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineTop.m_color =  pageCol;
		drawLine (lineTop, bRec.left, bRec.top, bRec.left + bRec.width,  bRec.top,getGraphics()); 
		if(pBroke && pBroke->getPage() && pBroke->getBrokenTop())
		{
			UT_sint32 col_x,col_y;
			fp_Column * pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
			pBroke->getPage()->getScreenOffsets(pCol, col_x,col_y);
			drawLine (lineTop, bRec.left, col_y, bRec.left + bRec.width,  col_y,getGraphics());
		}
		lineRight.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineRight.m_color =  pageCol;
		drawLine (lineRight, bRec.left + bRec.width, bRec.top, bRec.left + bRec.width, bRec.top + bRec.height,getGraphics()); 
		
		lineBottom.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		lineBottom.m_color = pageCol;
		drawLine (lineBottom, bRec.left, bRec.top + bRec.height, bRec.left + bRec.width , bRec.top + bRec.height,getGraphics());
		xxx_UT_DEBUGMSG(("_Clear: pBroke %x \n",pBroke));
		if(pBroke && pBroke->getPage() && pBroke->getBrokenBottom())
		{
			fp_Column * pCol = static_cast<fp_Column *>(pBroke->getBrokenColumn());
			if (pCol)
			{
				UT_sint32 col_x,col_y;
				pBroke->getPage()->getScreenOffsets(pCol, col_x,col_y);
				UT_sint32 bot = col_y + pCol->getHeight();
				xxx_UT_DEBUGMSG(("_clear: Clear broken bottom %d \n",bot));
				drawLine (lineBottom, bRec.left, bot, bRec.left + bRec.width,  bot,getGraphics());
			}
		}
		getGraphics()->setLineWidth(1 );
		xxx_UT_DEBUGMSG(("_clear: BRec.top %d  Brec.height %d \n",bRec.top,bRec.height));
		//			UT_ASSERT((bRec.left + bRec.width) < getPage()->getWidth());
		UT_sint32 srcX = 0;
		UT_sint32 srcY = 0;
		getFillType().setWidthHeight(getGraphics(),bRec.width,bRec.height);
		getLeftTopOffsets(srcX,srcY);
		if(getFillType().getParent())
		{
			srcX += getX();
			srcY += getY();
			getFillType().getParent()->Fill(getGraphics(),srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
		}
		else
		{
			getFillType().Fill(getGraphics(),srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
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
	for(i =0; i< countCons(); i++)
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

bool fp_CellContainer::isInNestedTable(void) const
{
	fp_TableContainer * pMaster = static_cast<fp_TableContainer *>(getContainer());
	if(pMaster && pMaster->getContainer() && !pMaster->getContainer()->isColumnType())
	{
		return true;
	}
	return false;
}

void fp_CellContainer::setBackground (const PP_PropertyMap::Background & style)
{
	m_background = style;
	PP_PropertyMap::Background background = getBackground ();
	if(background.m_t_background == PP_PropertyMap::background_solid)
	{
		getFillType().setColor(background.m_color);
	}
}

/*!
 * Given the broken table that contains this cell, calculate the positions
 * of the left,right,top and bottom edges of the cell.
 */
void fp_CellContainer::getScreenPositions(fp_TableContainer * pBroke,GR_Graphics * pG, UT_sint32 & iLeft, UT_sint32 & iRight,UT_sint32 & iTop,UT_sint32 & iBot, UT_sint32 & col_y, fp_Column *& pCol, fp_ShadowContainer *& pShadow, bool & bDoClear ) const
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
 * Draw background and lines around a cell in a broken table. Return true if lines were drawn, false otherwise.
 */
bool fp_CellContainer::drawLines(fp_TableContainer * pBroke,GR_Graphics * pG, bool bDoClear)
{
	xxx_UT_DEBUGMSG(("Doing drawlines for cell %x \n",this));
	UT_return_val_if_fail(getPage(), false);

	if(pBroke == NULL)
	{
		pBroke = static_cast<fp_TableContainer *>(getContainer());
	}
	UT_return_val_if_fail(pBroke, false);
	if(pBroke && pBroke->getPage())
	{
		if(pG->queryProperties(GR_Graphics::DGP_SCREEN) && !pBroke->getPage()->isOnScreen())
		{
			return false;
		}
	}
// Lookup table properties to get the line thickness, etc.

	fl_ContainerLayout * pLayout = getSectionLayout()->myContainingLayout ();
	UT_return_val_if_fail(pLayout->getContainerType () == FL_CONTAINER_TABLE, false);
	fp_Page * pPage = pBroke->getPage();
	UT_return_val_if_fail(pPage, false);

	fl_TableLayout * pTableLayout = static_cast<fl_TableLayout *>(pLayout);

	PP_PropertyMap::Line lineBottom = getBottomStyle (pTableLayout);
	PP_PropertyMap::Line lineLeft   = getLeftStyle   (pTableLayout);
	PP_PropertyMap::Line lineRight  = getRightStyle  (pTableLayout);
	PP_PropertyMap::Line lineTop    = getTopStyle    (pTableLayout);
	bool bDrawTop = true;
	bool bDrawBot = true;
	xxx_UT_DEBUGMSG(("m_iBotY %d \n",m_iBotY));
	m_bLinesDrawn = true;
	UT_sint32 iLeft,iRight,iTop,iBot = 0;
	UT_sint32 col_y = 0;
	fp_Column * pCol = NULL;
	fp_ShadowContainer * pShadow = NULL;
	bool doClear2 =false;
	bool bTopScreen = false;
	bool bBotScreen = false;
	getScreenPositions(pBroke,pG,iLeft,iRight,iTop,iBot,col_y,pCol,pShadow,doClear2 );
	if ((m_iBotY < pBroke->getYBreak()) || (m_iTopY > pBroke->getYBottom()))
	{
		// Cell is above or below the page
		return false;
	}

	iTop -= pBroke->getYBreak();
	iBot -= pBroke->getYBreak();
	xxx_UT_DEBUGMSG(("drawLines: ibot = %d col_y %d m_iBotY %d pCol->getHeight() %d left %d top %d \n",iBot,col_y,m_iBotY,pCol->getHeight(),m_iLeftAttach,m_iTopAttach));
	if(iTop < col_y)
	{
		xxx_UT_DEBUGMSG(("iTop < col_y !! iTop %d col_y %d row is %d \n",iTop,col_y,getTopAttach()));
		iTop = col_y;
		bDrawTop = true;
		bTopScreen = true;
		if(pBroke != NULL)
		{
			pBroke->setBrokenTop(true);
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

	if(iBot > col_y + iColHeight && (!pBroke || pBroke->getNext()))
	{
		if (pBroke)
		{
			iBot += pBroke->getYBottom() + 1 - pBroke->getYOfRow(getBottomAttach());
			iBot +=	pBroke->getAdditionalBottomSpace();
			pBroke->setBrokenBottom(true);
		}
		else
		{
			iBot =  col_y + iColHeight;
		}
		bDrawBot = true;
		bBotScreen = true;
	}

	m_bDrawRight = true;
	UT_sint32 onePix = pG->tlu(1)+1;
	//
	// the was put in to fix cairo draws but it makes windows look bad.
	// Fixme for cairo a different way.
	//
	onePix = 0;
	//
	// Have to draw white first because drawing is additive
	//
	PP_PropertyMap::Line clineBottom = getBottomStyle (pTableLayout);
	PP_PropertyMap::Line clineLeft   = getLeftStyle   (pTableLayout);
	PP_PropertyMap::Line clineRight  = getRightStyle  (pTableLayout);
	PP_PropertyMap::Line clineTop    = getTopStyle    (pTableLayout);

	UT_RGBColor white(255,255,255);
	white = *pPage->getFillType().getColor();

	//
	// Might needs these later
	//
	UT_sint32 iextLeft=0;
	UT_sint32 iextRight=0;
	UT_sint32 iextTop=0;
	UT_sint32 iextBot= 0;

	if (m_bDrawLeft)
	{
		if(bDoClear)
		{
			clineLeft.m_color = white;
			clineLeft.m_thickness  += 3*onePix;
			drawLine (clineLeft, iLeft, iTop, iLeft,  iBot,pG);
		}
		else
		{
			if(bTopScreen)
				iextTop = 0;
			if(bBotScreen)
				iextBot = 0;
			drawLine(lineLeft, iLeft, iTop-iextTop, iLeft, iBot+iextBot,pG);
		}
	}
	if(m_bDrawTop || bDrawTop)
	{
		if(bDoClear)
		{
			clineTop.m_color = white;
			clineTop.m_thickness  += 3*onePix;
			drawLine(clineTop, iLeft, iTop, iRight, iTop,pG);
		}
		else
		{
			drawLine(lineTop, iLeft-iextLeft, iTop, iRight+iextRight, iTop,pG);
		}
	}
	if(m_bDrawRight)
	{
		if(bDoClear)
		{
			clineRight.m_color = white;
			clineRight.m_thickness  += 3*onePix;
			drawLine(clineRight, iRight, iTop, iRight, iBot,pG);
		}
		else
		{
			if(bTopScreen)
				iextTop = 0;
			if(bBotScreen)
				iextBot = 0;
			drawLine(lineRight, iRight, iTop-iextTop, iRight, iBot+iextBot,pG);
		}
	}
	if(m_bDrawBot || bDrawBot)
	{
		if(bDoClear)
		{
			clineBottom.m_color = white;
			clineBottom.m_thickness  += 3*onePix;
			drawLine(clineBottom, iLeft, iBot, iRight, iBot,pG);
		}
		else
		{
			drawLine(lineBottom, iLeft-iextLeft, iBot, iRight+iextRight, iBot,pG);
		}
	}
	return true;
}

/*!
 *   |
 *   |_ 
 *
 * Extend the cell line at the left top corner if needed
 */
#if 0
//
// These methods not needed for 2.8 but might be for 2.10
//
void  fp_CellContainer::extendLeftTop(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextTop)
{
	iextTop = 0;
	if(getTopAttach() == 0)
	{
		return;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach()-1,getLeftAttach());
	if(pCell->getLeftAttach() != getLeftAttach())
	{
		return;
	}
	iextTop = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextTop += 3*onePix;
}


/*!
 *    _
 *   |
 *   | 
 *
 * Extend the cell line at the left bot corner if needed
 */
void  fp_CellContainer::extendLeftBot(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextBot)
{
	iextBot = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getBottomAttach() == pTab->getNumRows())
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach()+1,getLeftAttach());
	if(!pCell)
		return;
	if(pCell->getLeftAttach() != getLeftAttach())
	{
		return;
	}
	iextBot = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextBot += 3*onePix;
}

/*!
 *    |
 *   _|
 *
 * Extend the cell line at the right top corner if needed
 */
void  fp_CellContainer::extendRightTop(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextTop)
{
	iextTop = 0;
	if(getTopAttach() == 0)
	{
		return;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach()-1,getLeftAttach());
	if(!pCell)
		return;
	if(pCell->getRightAttach() != getRightAttach())
	{
		return;
	}
	iextTop = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextTop += 3*onePix;
}


/*!
 *   _
 *    |
 *    |
 *
 * Extend the cell line at the right bot corner if needed
 */
void  fp_CellContainer::extendRightBot(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextBot)
{
	iextBot = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getBottomAttach() == pTab->getNumRows())
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach()+1,getLeftAttach());
	if(!pCell)
		return;
	if(pCell->getRightAttach() != getRightAttach())
	{
		return;
	}
	iextBot = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextBot += 3*onePix;
}

/*!
 *  _ _
 *     |
 *    
 *
 * Extend the cell line at the left top corner if needed
 */
void  fp_CellContainer::extendTopLeft(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextLeft)
{
	iextLeft = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getLeftAttach() == 0)
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach(),getLeftAttach()-1);
	if(!pCell)
		return;
	if(pCell->getTopAttach() != getTopAttach())
	{
		return;
	}
	iextLeft = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextLeft += 3*onePix;
}


/*!
 *     
 *   _ _
 *  |   
 *
 * Extend the cell line at the right top corner if needed
 */
void  fp_CellContainer::extendTopRight(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextRight)
{
	iextRight = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getRightAttach() >=  pTab->getNumCols())
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach(),getRightAttach());
	if(!pCell)
		return;
	if(pCell->getTopAttach() != getTopAttach())
	{
		return;
	}
	iextRight = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextRight += 3*onePix;
}


/*!
 *  _ _|
 *     
 *    
 * Extend the cell line at the left Bot corner if needed
 */
void  fp_CellContainer::extendBotLeft(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextLeft)
{
	iextLeft = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getLeftAttach() == 0)
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach(),getLeftAttach()-1);
	if(!pCell)
		return;
	if(pCell->getBottomAttach() != getBottomAttach())
	{
		return;
	}
	iextLeft = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextLeft += 3*onePix;
}


/*!
 *     
 *  |_ _
 * 
 * Extend the cell line at the right bot corner if needed
 */
void  fp_CellContainer::extendBotRight(PP_PropertyMap::Line & line,GR_Graphics * pG,UT_sint32 & iextRight)
{
	iextRight = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	if(pTab == NULL)
	{
		return;
	}
	if(getRightAttach() >=  pTab->getNumCols())
	{
		return;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(getTopAttach(),getRightAttach());
	if(!pCell)
		return;
	if(pCell->getBottomAttach() != getBottomAttach())
	{
		return;
	}
	iextRight = line.m_thickness;
	UT_sint32 onePix = pG->tlu(1)+1;
	iextRight += 3*onePix;
}
#endif

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
		drawLines(pBroke,getGraphics(),true);
		if(bDoRight)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_right);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics(),true);
			}
		}
		if(bDoLeft)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_left);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics(),true);
			}
		}
		drawLines(pBroke,getGraphics(),false);
		if(bDoRight)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_right);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics(),false);
			}
		}
		if(bDoLeft)
		{
			fp_CellContainer * pCell = pTab->getCellAtRowColumn(row,col_left);
			if(pCell)
			{
				pCell->drawLines(pBroke,getGraphics(),false);
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
 * Return true if the segment of the cell within a broken table pBroke contains a footnote references
 */
bool fp_CellContainer::containsFootnoteReference(const fp_TableContainer * pBroke) const
{
	// First check if there are footnotes in the whole cell
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
	if (!pCL->containsFootnoteLayouts())
	{
		return false;
	}
	else if (!pBroke || ((getY() >= pBroke->getYBreak()) && 
						 (getY() + getHeight() <= pBroke->getYBottom())))
	{
		// the complete cell is within the broken table
		return true;
	}

	// Check if there are footnotes in the segment of the cell within the broken table

	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	bool bFirst = false;
	while(pCon && !bFound)
	{
		if (pBroke->isInBrokenTable(this,pCon))
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
			bFirst = true;
		}
		else if (bFirst)
		{
			// this container is in the following broken table
			break;
		}
		pCon = static_cast<fp_Container*>(pCon->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of all the footnote layouts in the segment of the cell 
 within a broke table pBroke
 */
bool fp_CellContainer::getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pVecFoots,
											 const fp_TableContainer * pBroke) const
{
	bool bWholeCell = (!pBroke || ((getY() >= pBroke->getYBreak()) && 
								   (getY() + getHeight() <= pBroke->getYBottom())));
	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	bool bFirst = false;
	while(pCon)
	{
		if (bWholeCell || pBroke->isInBrokenTable(this,pCon))
		{
			if(pCon->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pLine = static_cast<fp_Line *>(pCon);
				UT_GenericVector<fp_FootnoteContainer*> vecFC;
				pLine->getFootnoteContainers(&vecFC);
				if (vecFC.getItemCount() > 0)
				{
					bFound = true;
					UT_sint32 i = 0;
					for(i = 0; i < vecFC.getItemCount();i++)
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
					UT_sint32 i = 0;
					for(i = 0; i < vecFC.getItemCount();i++)
					{
						pVecFoots->addItem(vecFC.getNthItem(i));
					}
				}
			}
			bFirst = true;
		}
		else if (bFirst)
		{
			break;
		} 
		pCon = static_cast<fp_Container*>(pCon->getNext());
	}
	return bFound;
}


/*!
 * Return true if the segment of the cell within a broken table pBroke contains an annotation
 */
bool fp_CellContainer::containsAnnotations(const fp_TableContainer * pBroke) const
{
	// First check if there are annotations in the whole cell
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
	if (!pCL->containsAnnotationLayouts())
	{
		return false;
	}
	else if (!pBroke || ((getY() >= pBroke->getYBreak()) && 
						 (getY() + getHeight() <= pBroke->getYBottom())))
	{
		// the complete cell is within the broken table
		return true;
	}

	// Check if there are annotations in the segment of the cell within the broken table

	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	bool bFirst = false;
	while(pCon && !bFound)
	{
		if (pBroke->isInBrokenTable(this,pCon))
		{
			if(pCon->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pLine = static_cast<fp_Line *>(pCon);
				if(pLine->containsAnnotations())
				{
					bFound = true;
				}
			}
			else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
				if(pTab->containsAnnotations())
				{
					bFound = true;
				}
			}
			bFirst = true;
		}
		else if (bFirst)
		{
			// this container is in the following broken table
			break;
		}
		pCon = static_cast<fp_Container*>(pCon->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of all the annotation layouts in the segment of the cell 
 within a broke table pBroke
 */
bool fp_CellContainer::getAnnotationContainers(UT_GenericVector<fp_AnnotationContainer*>* pVecAnns,
											 const fp_TableContainer * pBroke) const
{
	bool bWholeCell = (!pBroke || ((getY() >= pBroke->getYBreak()) && 
								   (getY() + getHeight() <= pBroke->getYBottom())));
	fp_Container * pCon = getFirstContainer();
	bool bFound = false;
	bool bFirst = false;
	while(pCon)
	{
		if (bWholeCell || pBroke->isInBrokenTable(this,pCon))
		{
			if(pCon->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pLine = static_cast<fp_Line *>(pCon);
				UT_GenericVector<fp_AnnotationContainer*> vecAC;
				pLine->getAnnotationContainers(&vecAC);
				if (vecAC.getItemCount() > 0)
				{
					bFound = true;
					UT_sint32 i = 0;
					for(i = 0; i < vecAC.getItemCount();i++)
					{
						pVecAnns->addItem(vecAC.getNthItem(i));
					}
				}
			}
			else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pCon);
				if(pTab->containsAnnotations())
				{
					bFound = true;
					UT_GenericVector<fp_AnnotationContainer*> vecAC;
					pTab->getAnnotationContainers(&vecAC);
					UT_sint32 i = 0;
					for(i = 0; i < vecAC.getItemCount();i++)
					{
						pVecAnns->addItem(vecAC.getNthItem(i));
					}
				}
			}
			bFirst = true;
		}
		else if (bFirst)
		{
			break;
		} 
		pCon = static_cast<fp_Container*>(pCon->getNext());
	}
	return bFound;
}

/*!
 * Return the x coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellX(fp_Line * /*pLine*/) const
{
	return 0;
}

/*!
 * Return the y coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellY(fp_Line * /*pLine*/) const
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
	if(pG->queryProperties(GR_Graphics::DGP_SCREEN))
		drawLines(NULL,pG,true);
	drawLines(NULL,pG,false);
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
		xxx_UT_DEBUGMSG(("Looking at CL %x for delete \n",pCL));
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


void fp_CellContainer::deleteBrokenAfter(bool bClearFirst,UT_sint32 iOldBottom)
{
	if(!containsNestedTables())
	{
		return;
	}

	UT_sint32 i = 0;
	fp_Container * pCon;
	for(i=0; i< countCons(); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		if (pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer*>(pCon);
			UT_sint32 iYTab = getY() + pTab->getY();
			if (iYTab > iOldBottom)
			{
				UT_ASSERT(!pTab->getMasterTable());
				pTab->deleteBrokenAfter(bClearFirst);
			}
			else if (iYTab + pTab->getTotalTableHeight() < iOldBottom)
			{
				continue;
			}
			else
			{
				while(pTab)
				{
					if (iYTab + pTab->getYBreak() >= iOldBottom)
					{
						break;
					}
					pTab = static_cast<fp_TableContainer*>(pTab->getNext());
				}
				
				if(pTab && pTab->getPrev())
				{
					pTab = static_cast<fp_TableContainer*>(pTab->getPrev());
					pTab->deleteBrokenAfter(bClearFirst);
				}
			}
		}
	}
}



/*!
 * Draw the whole cell with the selection colour background.
 * returns the last drawn cell in the container or NULL
\Param pLine pointer to the line contained within the cell the cell.
*/
fp_Container * fp_CellContainer::drawSelectedCell(fp_Line * /*pLine*/)
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
bool fp_CellContainer::doesOverlapBrokenTable(const fp_TableContainer * pBroke) const
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
	fp_TableContainer * pTab2 = NULL;
	if(pBroke && pBroke->isThisBroken())
	{
		pTab2 = pBroke->getMasterTable();
	}
	else
	{
		pTab2 = static_cast<fp_TableContainer *>(getContainer());
	}
// draw bottom if this cell is the last of the table and fully contained on the page

	m_bDrawBot = (pTab2->getCellAtRowColumn(getBottomAttach(),getLeftAttach()) == NULL);

// draw right if this cell is the rightmost of the table

	m_bDrawRight = (pTab2->getCellAtRowColumn(getTopAttach(),getRightAttach()) == NULL);
	m_bDrawRight = true;
	m_bDrawLeft = true;
   
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop,ybot;
	UT_sint32 i;
	UT_sint32 imax = static_cast<UT_sint32>((static_cast<UT_uint32>(1<<29)) - 1);
	UT_Rect bRec;
	fp_Page * pPage;
	_getBrokenRect(pBroke, pPage, bRec,pG);
	xxx_UT_DEBUGMSG(("Draw Broken Table %p ybreak %d On Page %d cell %p \n",pBroke,pBroke->getYBreak(),pPage->getPageNumber(),this));
	if((bRec.height < 0) || (bRec.width < 0))
	{
		xxx_UT_DEBUGMSG(("brokenRect off page - bailing out \n"));
		return;
	}
	if(getFillType().getFillType() == FG_FILL_IMAGE && (getContainer() != NULL))
	{
		fl_DocSectionLayout * pDSL = getSectionLayout()->getDocSectionLayout();
		if(pDSL && (bRec.height < pDSL->getActualColumnHeight()) && (bRec.height > pG->tlu(3)))
		{
			getSectionLayout()->setImageHeight(bRec.height);
			getSectionLayout()->setImageWidth(bRec.width);
			getFillType().setWidthHeight(pG,bRec.width,bRec.height,true);
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
		getFillType().setWidthHeight(pG,bRec.width,bRec.height);
		getLeftTopOffsets(srcX,srcY);
		getFillType().Fill(pG,srcX,srcY,bRec.left,bRec.top,bRec.width,bRec.height);
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
		FV_View * pView = getPage()->getDocLayout()->getView();
		xxx_UT_DEBUGMSG(("drawBroke: fill rect: Final top %d bot %d  pBroke %x \n",bRec.top,bRec.top + bRec.height,pBroke));
		UT_ASSERT((bRec.left + bRec.width) < static_cast<UT_sint32>(pView->getWidthPagesInRow(getPage())+ pView->getPageViewLeftMargin()) );
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
	for ( i = 0; (i< countCons() && !bStop); i++)
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
				UT_sint32 TabHeight= 0;
				fp_TableContainer * pTCon = static_cast<fp_TableContainer *>(pContainer);
				if(pTCon->isThisBroken())
				{
					TabHeight = pTCon->getHeight();
				}
				else if(pTCon->getFirstBrokenTable())
				{
					pContainer = pTCon->getFirstBrokenTable();
					TabHeight = static_cast<fp_TableContainer *>(pContainer)->getHeight();
				}
				else
				{
					TabHeight = static_cast<fp_TableContainer *>(pContainer)->getHeight();
				}
				ydiff = da.yoff + TabHeight;
			}

			if((da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
			{
				// Draw the top of the cell if the cell starts on this page.
				if(i == 0)
				{
					m_bDrawTop = true;
				}
				bStart = true;

				if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
					if(pTab->isThisBroken())
					{
						xxx_UT_DEBUGMSG(("Draw nested broken table %p da.yoff %d yBreak %d \n",pTab,da.yoff,pTab->getYBreak()));
						dg_DrawArgs daa = da;
						pTab->draw(&daa);
						iLastDraw = i;
					}
					else
					{
						fp_TableContainer * pT= pTab->getFirstBrokenTable();
						if(pT == NULL)
						{
							UT_DEBUGMSG(("No Broken Table in draw !! \n"));
							UT_sint32 iY = pTab->getY();
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
					pContainer->setBreakTick(getBreakTick());
					pContainer->draw(&da);
					iLastDraw = i;
				}
			}
			else if(bStart)
			{
				bStop = true;
			}
		}
		else if(bStart)
		{
			bStop = true;
		}
	}
	if((iLastDraw >= countCons()-1) && !bStop)
	{
		m_bDirty = false;
		getSectionLayout()->clearNeedsRedraw();
	}
	drawLines(pBroke,pG,true);
	drawLines(pBroke,pG,false);
	pTab2->setRedrawLines();
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
	xxx_UT_DEBUGMSG(("iBreakTick is %d \n",getBreakTick()));
	if(!containsNestedTables())
	{
		return NULL;
	}
	UT_sint32 count = countCons();
	UT_DEBUGMSG(("vBREAK cell at  %d \n",vpos));
	UT_sint32 i = 0;
	fp_Container * pCon;
	fp_TableContainer * pBroke = NULL;
	UT_sint32 iY = 0;
	for(i=0; (i < count) || (iY <= vpos); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		xxx_UT_DEBUGMSG(("Looking Container iY  %d height %d \n",iY,pCon->getHeight()));
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
							UT_DEBUGMSG(("Break Nested table Con %p at 0 y %d height %d \n",pCon,pCon->getY(),pCon->getHeight()));
							UT_DEBUGMSG(("Break Nested table Table %p at 0 y %d height %d \n",pBTab,pBTab->getY(),pBTab->getHeight()));
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
					pBroke = static_cast<fp_TableContainer *>(pPrevTab->VBreakAt(vpos - pMaster->getY() - pPrevTab->getYBreak()));
					if (pBroke)
					{
						pBroke->setY(vpos);
						static_cast<fp_TableContainer *>(pBroke)->setY(pBroke->getY());
						UT_DEBUGMSG(("Made broken nested Table %p Y %d height %d \n",pBroke,pBroke->getY(),pBroke->getHeight()));
						UT_ASSERT(pBroke->getContainer() == this);
					}
					break;
				}
			}
		}
		iY += pCon->getHeight();
		iY += pCon->getMarginAfter();
	}
	return static_cast<fp_ContainerObject *>(pBroke);
}


/*!
 * This routine requests that the cell be broken at the specfied height.
 * the return value of the method is the actual height it can be broken
 * which is less than or equal to the requested height.
 */
UT_sint32 fp_CellContainer::wantCellVBreakAt(UT_sint32 vpos, UT_sint32 yCellMin) const
{
	UT_sint32 i =0;
	UT_sint32 iYBreak = vpos;
	fp_Container * pCon;
	//fp_Line * pLine = NULL;
	UT_sint32 footHeight = 0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	UT_return_val_if_fail(pTab,0);
	for(i=0; i < countCons(); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		if (yCellMin > pCon->getY() + 1)
		{
			// pCon is in a segment of the cell contained within a previous broken table
			// This condition matches the condition in fp_TableContainer::isInBrokenTable
			continue;
		}
		UT_sint32 iY = pCon->getY() + getY();
		UT_sint32 conHeight = pCon->getHeight();
		bool bConBroken = false;
		if (pCon->isVBreakable() && pCon->getNext())
		{
			bConBroken = true;
			if (pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				conHeight = static_cast<fp_TableContainer*>(pCon)->getTotalTableHeight();
			}
		}

		if(iY <= vpos && iY + conHeight > vpos)
		{
			//
			// Container overlaps break point. Find break point in the 
			// container if possible.
			//
			UT_sint32 iCur =0;
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				iCur = pCon->wantVBreakAt(vpos - iY);
				fp_TableContainer* pNestedTab = static_cast<fp_TableContainer*>(pCon);
				if (!pNestedTab->isThisBroken() && pNestedTab->getFirstBrokenTable())
				{
					pNestedTab = pNestedTab->getFirstBrokenTable();
				}
				if (pNestedTab->getYBottom() != iCur - 1)
				{
					pNestedTab->deleteBrokenAfter(true);
				}
				iCur = iCur + iY + 1;
			}
			else
			{
				iCur = iY + 1;
			}
			if(iCur < iYBreak)
			{
				iYBreak = iCur;
			}
			break;
		}
		else if (bConBroken)
		{
			// The whole container fits in the table. Delete broken segments
			static_cast<fp_VerticalContainer*>(pCon)->deleteBrokenAfter(true);
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


/*
  This function returns the first container inside a cell that is also inside
  a broken table. It returns NULL if no container is inside  the broken table
*/

fp_Container * fp_CellContainer::getFirstContainerInBrokenTable(const fp_TableContainer * pBroke) const
{
	if (!pBroke->isThisBroken())
	{
		return NULL;
	}

	UT_sint32 count = countCons();
	UT_sint32 k = 0;
	fp_Container * pCon = NULL;
	for (k = 0; k < count; k++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(k));
		if (pBroke->isInBrokenTable(this, pCon))
		{
			return pCon;
		}
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
			xxx_UT_DEBUGMSG(("sizeRequest: Height of Line %d %d tot %d \n",i,pCon->getHeight(),height));
			height = height + pCon->getHeight();
			height = height + pCon->getMarginAfter();
		}
		else
		{
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer * pTab = static_cast<fp_TableContainer*>(pCon);
				fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pTab->getSectionLayout());
				if (pTL->isInitialLayoutCompleted())
				{
					fp_Requisition pReq;
					pTab->sizeRequest(&pReq);
					if(width < pReq.width)
					{
						width = pReq.width;
					}
					height = height + pReq.height;
				}
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		xxx_UT_DEBUGMSG(("Total height %d \n",height));
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
	if(countCons() == 0)
	{
		return;
	}
	for (UT_sint32 i=0; i < countCons(); i++)
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
	layout();
}


void fp_CellContainer::getLeftTopOffsets(UT_sint32 & xoff, UT_sint32 & yoff) const
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
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getContainer());
	UT_return_if_fail(pTab);

	m_iLeft  = pTab->getXOfColumn(getLeftAttach());
	m_iRight = pTab->getXOfColumn(getRightAttach());
	m_iTopY  = pTab->getYOfRow(getTopAttach(), false);
	m_iBotY  = pTab->getYOfRow(getBottomAttach(), false);
}

void fp_CellContainer::doVertAlign(void)
{
	// Vertical alignment - EA
	// Note, must be called right after the cell's boundary lines have been determined with setLineMarkers.
	// Currently, doVertAlign is called fp_TableContainer::setToAllocation() although the most direct method
	// would be to call it at the end of fp_CellContainer::setLineMarkers() because I don't want the call to
	// be too deeply buried in code.

	setY( m_MyAllocation.y - (getHeight() * (m_iVertAlign/100.0)) + ((m_iBotY - m_iTopY) * (m_iVertAlign/100.0)) );
	if( (getY() + getHeight()) > m_iBotY - getBotPad() ) // Make sure not to exceed cell's bottom padding
		setY( m_iBotY - getBotPad() - getHeight() );
	if( getY() < m_iTopY + getTopPad() ) // Make sure not to exceed cell's top padding
		setY( m_iTopY + getTopPad() );
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
	  m_bIsHomogeneous(true),
	  m_iRowSpacing(0),
	  m_iColSpacing(0),
	  m_pFirstBrokenTable(NULL),
	  m_pLastBrokenTable(NULL),
	  m_bIsBroken(false),
	  m_pMasterTable(NULL),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_iAdditionalBottomSpace(0),
	  m_bBrokenTop(false),
	  m_bBrokenBottom(false),
	  m_bRedrawLines(false),
	  m_iLineThickness(1),
	  m_iRowHeightType(FL_ROW_HEIGHT_NOT_DEFINED),
	  m_iRowHeight(0),
	  m_iLastWantedVBreak(-1),
	  m_iNextWantedVBreak(-1),
	  m_pFirstBrokenCell(NULL),
	  m_iAdditionalMarginAfter(0)
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
	  m_bIsHomogeneous(true),
	  m_iRowSpacing(0),
	  m_iColSpacing(0),
	  m_pFirstBrokenTable(NULL),
	  m_pLastBrokenTable(NULL),
	  m_bIsBroken(true),
	  m_pMasterTable(pMaster),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_iAdditionalBottomSpace(0),
	  m_bBrokenTop(false),
	  m_bBrokenBottom(false),
	  m_bRedrawLines(false),
	  m_iLineThickness(1),
	  m_iRowHeightType(FL_ROW_HEIGHT_NOT_DEFINED),
	  m_iRowHeight(0),
	  m_iLastWantedVBreak(-1),
	  m_iNextWantedVBreak(-1),
	  m_pFirstBrokenCell(NULL),
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
	UT_std_vector_purgeall(m_vecRows);
	UT_std_vector_purgeall(m_vecColumns);
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

fp_Column * fp_TableContainer::getBrokenColumn(void) const
{
	if(!isThisBroken())
	{
		return static_cast<fp_Column *>(fp_VerticalContainer::getColumn());
	}
	const fp_TableContainer * pBroke = this;
	bool bStop = false;
	fp_Column * pCol = NULL;
	while(pBroke && pBroke->isThisBroken() && !bStop)
	{
		fp_Container * pCon = pBroke->getContainer();
		if (!pCon)
		{
			return NULL;
		}
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
			pBroke = pCell->getBrokenTable(static_cast<const fp_Container *>(pBroke));
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

bool fp_TableContainer::containsNestedTables(void) const
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
				pCell->drawLines(pBroke,getGraphics(),true);
				pCell->drawLines(pBroke,getGraphics(),false);
				pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
			}
		}
		else
		{
			pCell->drawLines(NULL,getGraphics(),true);
			pCell->drawLines(NULL,getGraphics(),false);
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

UT_sint32 fp_TableContainer::getBrokenNumber(void) const
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
		getMasterTable()->setFirstBrokenTable(pBroke);
	}
	m_pFirstBrokenTable = pBroke;

}

void fp_TableContainer::setLastBrokenTable(fp_TableContainer * pBroke) 
{
	if(isThisBroken())
	{
		getMasterTable()->setLastBrokenTable(pBroke);
	}
	m_pLastBrokenTable = pBroke;
}

/* Return the first cell of a broken table.
*/

fp_CellContainer *  fp_TableContainer::getFirstBrokenCell(bool bCacheResultOnly) const
{
	if (bCacheResultOnly || m_pFirstBrokenCell)
	{
		return m_pFirstBrokenCell;
	}

	if (getPrev())
	{
		fp_TableContainer * pPrevTable = static_cast<fp_TableContainer*>(getPrev());
		if (pPrevTable->getFirstBrokenCell(true))
		{
			return pPrevTable->getFirstBrokenCell(true);
		}
	}

	if (!isThisBroken())
	{
		return static_cast<fp_CellContainer *>(getNthCon(0));
	}

	return static_cast<fp_CellContainer *>(getMasterTable()->getNthCon(0));
}



UT_sint32 fp_TableContainer::getYOfRowOrColumn(UT_sint32 row, bool bRow) const
{
	return ((bRow) ? getYOfRow(row) : getXOfColumn(row));
}

/*!
 * Return the Y location of row number row
 */
UT_sint32 fp_TableContainer::getYOfRow(UT_sint32 row, bool bBottomOffset) const
{
	if (getMasterTable())
	{
		return getMasterTable()->getYOfRow(row);
	}
	UT_sint32 numRows = getNumRows();
	if((row > numRows) || (numRows == 0))
	{
		return 0;
	}

	UT_sint32 iYRow = 0;
	fp_TableRowColumn *pRow = NULL;
	if (row < numRows)
	{
		pRow = getNthRow(row);
		iYRow = pRow->position;
	}
	else
	{
		pRow = getNthRow(numRows - 1);
		iYRow = pRow->position + pRow->allocation + pRow->spacing;
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
		if (bBottomOffset)
		{
			iYRow += pTL->getBottomOffset();
		}
	}

	return iYRow;
 }


/*!
 * Return the X location of column number col
 */
UT_sint32 fp_TableContainer::getXOfColumn(UT_sint32 col) const
{
	if (getMasterTable())
	{
		return getMasterTable()->getXOfColumn(col);
	}
	UT_sint32 numCols = getNumCols();
	if((col > numCols) || (numCols == 0))
	{
		return 0;
	}

	UT_sint32 iXCol = 0;
	fp_TableRowColumn *pCol = NULL;
	if (col < numCols)
	{
		pCol = getNthCol(col);
		iXCol = pCol->position;
	}
	else
	{
		pCol = getNthCol(numCols - 1);
		iXCol = pCol->position + pCol->allocation + pCol->spacing;
	}

	return iXCol;
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
fp_CellContainer * fp_TableContainer::getCellAtRowColumnLinear(UT_sint32 row, UT_sint32 col) const
{
	UT_sint32 i = 0;
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
fp_CellContainer * fp_TableContainer::getCellAtRowColumn(UT_sint32 row, UT_sint32 col) const
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
UT_sint32 fp_TableContainer::getRowHeight(UT_sint32 iRow, UT_sint32 iMeasHeight) const
{
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	UT_return_val_if_fail(pTL, 0);
	const  UT_GenericVector<fl_RowProps*>* pVecRow = pTL->getVecRowProps();
	if(pVecRow->getItemCount() < (iRow + 1))
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
	if (y <= 0)
	{
		y = 1;
	}

	fp_TableContainer * pMaster = NULL;
	if(isThisBroken())
	{
		pMaster = getMasterTable();
		y = y + getYBreak();
		if (y >= getYBottom())
		{
			y = getYBottom() - 1;
		}
	}
	else
	{
		pMaster = this;
		if (getFirstBrokenTable() && y >= getFirstBrokenTable()->getYBottom())
		{
			y = getFirstBrokenTable()->getYBottom() - 1;
		}
	}

	if(!pMaster->countCons())
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		pos = 2;
		bBOL = true;
		bEOL = true;
		return;
	}

	xxx_UT_DEBUGMSG(("SEVIOR: Table %x Looking for location x %d y %d \n",this,x,y));
	UT_sint32 row = pMaster->getRowOrColumnAtPosition(y,true);
	UT_sint32 col = pMaster->getRowOrColumnAtPosition(x,false);
	fp_CellContainer * pCell = pMaster->getCellAtRowColumn(row,col);
	if (!pCell)
	{
		col--;
		while (!pCell && col >= 0)
		{
			pCell = pMaster->getCellAtRowColumn(row,col);
			col--;
		}
		if (!pCell)
		{
			pCell = static_cast<fp_CellContainer*>(pMaster->getFirstContainer());
		}
	}
	UT_sint32 xCell = x - pCell->getX();
	UT_sint32 yCell = y - pCell->getY();
	pCell->mapXYToPosition(xCell, yCell, pos, bBOL, bEOL,isTOC);
	return;
}


UT_sint32 fp_TableContainer::getRowOrColumnAtPosition(UT_sint32 y, bool bRow) const
{
	if (isThisBroken())
	{
		return getMasterTable()->getRowOrColumnAtPosition(y,bRow);
	}

	UT_sint32 k = 0;
	std::vector<fp_TableRowColumn *>::const_iterator it;
	if (bRow)
	{
		it = std::upper_bound(m_vecRows.begin(), m_vecRows.end(),
							  y, fp_TableRowColumn::comparePosition);
		k = it - m_vecRows.begin();
	}
	else
	{
		it = std::upper_bound(m_vecColumns.begin(), m_vecColumns.end(),
							  y, fp_TableRowColumn::comparePosition);
		k = it - m_vecColumns.begin();
	}

	if (k > 0)
	{
		k--;
	}

	return k;
}


void fp_TableContainer::resize(UT_sint32 n_rows, UT_sint32 n_cols)
{
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (!pTL->isInitialLayoutCompleted() || (n_rows != m_iRows) ||
		( n_cols != m_iCols))
	{
		if (!pTL->isInitialLayoutCompleted() || (n_rows != m_iRows))
		{
			UT_sint32 i;
			m_iRows = n_rows;
			UT_std_vector_purgeall(m_vecRows);
			m_vecRows.clear();
			for(i=0; i< m_iRows; i++)
			{
				m_vecRows.push_back(new fp_TableRowColumn(m_iRowSpacing));
			}
		}
		
		if (!pTL->isInitialLayoutCompleted() || (n_cols != m_iCols))
		{
			UT_sint32 i;
			m_iCols = n_cols;
			UT_std_vector_purgeall(m_vecColumns);
			m_vecColumns.clear();
			for(i=0; i< m_iCols; i++)
			{
				m_vecColumns.push_back(new fp_TableRowColumn(m_iColSpacing));
			}
		}
	}
}

/*!
 * Returns true if this is a broken table
 */
bool fp_TableContainer::isThisBroken(void) const
{
	return m_bIsBroken;
}
/*!
 * Returns true since a table can be broken vertically.
 */
bool fp_TableContainer::isVBreakable(void)
{
	return true;
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
	pBroke = getFirstBrokenTable();
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
						xxx_UT_DEBUGMSG(("Also remove table %x from column %x \n",pBroke,pPrevCon)); 
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


/*
  Delete all broken tables that follows this table. The first broken table
  is kept if the function is called by the master table.
*/

void fp_TableContainer::deleteBrokenAfter(bool bClearFirst)
{
	if (!isThisBroken())
	{
		if (getFirstBrokenTable())
		{
			return getFirstBrokenTable()->deleteBrokenAfter(bClearFirst);
		}
		return;
	}

	if (bClearFirst)
	{
		clearScreen();
		getMasterTable()->clearBrokenContainers();
	}

	fp_TableContainer * pBroke = static_cast<fp_TableContainer *>(getNext());
	fp_TableContainer * pNext = NULL;
	while(pBroke)
	{
		pNext = static_cast<fp_TableContainer *> (pBroke->getNext());
		if (pBroke->getContainer())
		{
			UT_sint32 i = pBroke->getContainer()->findCon(pBroke);
			if (i >= 0)
			{
				pBroke->getContainer()->deleteNthCon(i);
				pBroke->setContainer(NULL);
			}
		}
		delete pBroke;
		pBroke = pNext;
	}

	setNext(NULL);
	if (!getPrev())
	{
		getMasterTable()->setNext(NULL);
	}
	getMasterTable()->setLastBrokenTable(this);
	UT_sint32 iOldYBottom = getYBottom();
	setYBottom(getTotalTableHeight());

	if (containsNestedTables())
	{
		// delete nested broken tables
		fp_CellContainer * pCell = m_pFirstBrokenCell;
		if (!pCell)
		{
			pCell = static_cast<fp_CellContainer*>(getMasterTable()->getFirstContainer());
		}
		while (pCell)
		{
			if (pCell->getY() + pCell->getHeight() > iOldYBottom)
			{
				pCell->deleteBrokenAfter(bClearFirst,iOldYBottom);
			}
			pCell = static_cast<fp_CellContainer*>(pCell->getNext());
		}
	}
}

/*
 * Return the margin before the table. Note that TopOffset (set by the property table-margin-top)
 * is not considered a margin, but as a gap at the top of the table (it is included in the height of
 * the fp_TableContainer object). This allows to move down a table that is placed at the top of a page.
 */

UT_sint32 fp_TableContainer::getMarginBefore(void) const
{

	if(!isThisBroken() || !getPrev())
	{
		// getMargin of previous block
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
		fl_ContainerLayout * pCL = pTL->getPrev();
		if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return static_cast<fl_BlockLayout *>(pCL)->getBottomMargin();
		}
		return 0;
	}
	return 0;
}

/*
 * Return the margin after the table. Note that BottomOffset (set by the property table-margin-bottom)
 * is considered a margin and the function returns the maximum of BottomOffset and the margin before of
 * the following block.
 */


UT_sint32 fp_TableContainer::getMarginAfter(void) const
{
	if(!isThisBroken() || !getNext())
	{
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
		fl_ContainerLayout * pCL = pTL->getNext();
		if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return UT_MAX(static_cast<fl_BlockLayout *>(pCL)->getTopMargin(),pTL->getBottomOffset());
		}
		return pTL->getBottomOffset();
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
	while(pCell)
	{
		if(pCell->getY() >= vpos)
		{
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
/*
 If this table is nested in a cell we break it and leave it in the cell. 

 cell->Master------>more lines
       \
        Broke->Broke->Broke|more lines
 The offsets for each line
 will be calculated only with the top level broken table.
*/

//
// Do the case of creating the first broken table from the master table.
// 
	fp_TableContainer * pBroke = NULL;
	if(!isThisBroken() && getLastBrokenTable() == NULL)
	{
		UT_return_val_if_fail(getFirstBrokenTable() == NULL, NULL);
		pBroke = new fp_TableContainer(getSectionLayout(),this);
		xxx_UT_DEBUGMSG(("SEVIOR:!!!!!!! First broken table %x \n",pBroke));
		pBroke->setYBreakHere(vpos);
		pBroke->setYBottom(getTotalTableHeight());
		setFirstBrokenTable(pBroke);
		setLastBrokenTable(pBroke);
		pBroke->setContainer(getContainer());
		pBroke->setHeight(pBroke->getHeight());
		pBroke->setY(getY());
		pBroke->breakCellsAt(vpos);
		return pBroke;
	}
//
// Now do the case of breaking a Master table.
//
	UT_return_val_if_fail(vpos > 0, NULL);
	if(getMasterTable() == NULL)
	{
		return getLastBrokenTable()->VBreakAt(vpos);
	}
	pBroke = new fp_TableContainer(getSectionLayout(),getMasterTable());
	UT_sint32 iTotalHeight = getTotalTableHeight();
	UT_sint32 iNewYBreak = vpos + getYBreak();
	if(getContainer() && getContainer()->getContainerType() == FP_CONTAINER_CELL)
	{
		if (m_iNextWantedVBreak <= 0)
		{
			return NULL;
		}
		iNewYBreak = m_iNextWantedVBreak + getYBreak();
	}
	UT_return_val_if_fail(iNewYBreak < iTotalHeight, NULL);

//
// vpos is relative to the container that contains this height but we need
// to add in the height above it.
//
	pBroke->setYBreakHere(iNewYBreak);
	setYBottom(iNewYBreak -1);
	pBroke->setYBottom(iTotalHeight);
	pBroke->setHeight(pBroke->getHeight());
	UT_ASSERT(getHeight() > 0);
	UT_ASSERT(pBroke->getHeight() > 0);

//
// The structure of table linked list is as follows.
// NULL <= Master <==> Next <==> Next => NULL
//          first 
// ie terminated by NULL's in the getNext getPrev list. The second
// broken table points and is pointed to by the Master table
// 

	fp_TableContainer * pThisTable = this;
	if(getMasterTable()->getFirstBrokenTable() == this)
	{
  		getMasterTable()->setNext(pBroke);
		pThisTable = getMasterTable();
	}
	setNext(pBroke);
	pBroke->setPrev(pThisTable);
	pBroke->setNext(NULL);
	getMasterTable()->setLastBrokenTable(pBroke);

	// TODO TODO TODO : This part should only be needed for nested tables as the insertion of
	// containers inside columns should be left to fb_ColumnBreaker::_break. However,
	// not inserting the broken table inside a column causes crashes in fp_CellContainer::clear.
	// This function needs to be revised before limiting the following code to nested tables.
	fp_Container * pUpCon = pThisTable->getContainer();
	if (pUpCon)
	{
		pBroke->setContainer(pUpCon);
		UT_sint32 i = pUpCon->findCon(pThisTable);
		UT_ASSERT(i >= 0);
		if (i >= 0)
		{
			if (i < pUpCon->countCons() - 1)
			{
				pUpCon->insertConAt(pBroke,i+1);
			}
			else
			{
				pUpCon->addCon(pBroke);
			}
		}
	}

	//
	// The cells are broken relative to the top of the table 
	//
	breakCellsAt(getYBottom());
	return pBroke;
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

	xxx_UT_DEBUGMSG(("Set Reformat 1 now from table %x in TableLayout %x \n",this,getSectionLayout()));
	fp_VerticalContainer::setY(i);
}


void fp_TableContainer::setYBreakHere(UT_sint32 i)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Ybreak set to %d \n",i));
	m_iYBreakHere = i;
}

void fp_TableContainer::setYBottom(UT_sint32 i)
{
	m_iYBottom = i;
	//	UT_ASSERT(getHeight() > 0);
}

/*!
 * The caller to this method requests a break at the vertical height
 * given. It returns the actual break height, which will always be
 * less than or equal to the requested height. The function returns -1
 * if the table does not need to be broken.
 */
UT_sint32 fp_TableContainer::wantVBreakAt(UT_sint32 vpos)
{
	if (!isThisBroken())
	{
		if (!getFirstBrokenTable())
		{
			VBreakAt(0);
			UT_ASSERT(getFirstBrokenTable());
		}
		return getFirstBrokenTable()->wantVBreakAt(vpos);
	}

	// Check if the table has footnotes and call the appropriate function
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (pTL->containsFootnoteLayouts() || 
		(pTL->getDocLayout()->displayAnnotations() && pTL->containsAnnotationLayouts()))
	{
		return wantVBreakAtWithFootnotes(vpos);
	}
	else
	{
		return wantVBreakAtNoFootnotes(vpos);
	}
}

UT_sint32 fp_TableContainer::wantVBreakAtNoFootnotes(UT_sint32 vpos)
{
	UT_sint32 iYBreakMax = vpos + getYBreak();
	UT_sint32 iTotHeight = getTotalTableHeight();
	if (iYBreakMax > iTotHeight)
	{
		// The table fits in the column. No need to break it
		return -1;
	}
	else if (iYBreakMax > iTotHeight - FP_TABLE_MIN_BROKEN_HEIGHT)
	{
		iYBreakMax = iTotHeight - FP_TABLE_MIN_BROKEN_HEIGHT;
	}

	fp_CellContainer * pCell = getFirstBrokenCell(false);

	// To avoid breaking small cells, we first check if the table can be broken
	// along a cell boundary. We test if the height of the gap left at the bottom
	// of the page is smaller than a fraction of a full column height (colHeight).

	UT_sint32 row = getRowOrColumnAtPosition(iYBreakMax,true);
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	UT_sint32 colHeight = pTL->getDocSectionLayout()->getActualColumnHeight();
	if ((row > 0) && (colHeight*pTL->getMaxExtraMargin() > iYBreakMax - getYOfRow(row)))
	{
		// Check that the row is cell boundary for all columns
		while (pCell)
		{
			if (!m_pFirstBrokenCell && (getYOfRow(pCell->getBottomAttach()) >= getYBreak()))
			{
				m_pFirstBrokenCell = pCell;
			}
			if (pCell->getBottomAttach() <= row)
			{
				pCell = static_cast<fp_CellContainer*>(pCell->getNext());
			}
			else if (pCell->getTopAttach() == row)
			{
				setAdditionalBottomSpace(0);
				m_iNextWantedVBreak = getYOfRow(row) - getYBreak();
				return (m_iNextWantedVBreak);
			}
			else
			{
				break;
			}
		}
	}

	// The table could not be broken along a cell boundary. Break table within cells

	UT_sint32 iYBreak = iYBreakMax;
	UT_sint32 iYBreakLine = 0;
	while (pCell)
	{
		if (!m_pFirstBrokenCell && (getYOfRow(pCell->getBottomAttach()) >= getYBreak()))
		{
			m_pFirstBrokenCell = pCell;
		}
		if (getYOfRow(pCell->getTopAttach()) >= iYBreakMax)
		{
			break;
		}
		if(pCell->getY() <= iYBreakMax && pCell->getY() + pCell->getHeight() > iYBreakMax)
		{
			//
			// Cell overlaps break point. Find break point in the cell.
			//
			UT_sint32 yCellMin = UT_MAX(getYBreak() - pCell->getY(),0);
			UT_ASSERT(iYBreakMax > yCellMin);
			UT_sint32 iCur = pCell->wantCellVBreakAt(iYBreakMax,yCellMin);
			if(iCur < iYBreak)
			{
				iYBreak = iCur;
			}
			if (iCur > iYBreakLine)
			{
				iYBreakLine = iCur;
			}
		}
		pCell = static_cast<fp_CellContainer*>(pCell->getNext());
	}

	// We need to do a second pass to find if there are some cells that could have fitted completely with the
	// original break point that will be affected by the new break point. If that is the case, we adjust the
	// bottom line position.
	pCell = getFirstBrokenCell(false);
	while (pCell)
	{
		if (getYOfRow(pCell->getTopAttach()) >= iYBreakMax)
		{
			break;
		}
		UT_sint32 iCellBottom = pCell->getY() + pCell->getHeight();
		if((iCellBottom < iYBreakMax) && (iCellBottom > iYBreak) &&
		   (pCell->getY() <= iYBreak) && (iCellBottom > iYBreakLine))
		{
			iYBreakLine = iCellBottom;
		}
		pCell = static_cast<fp_CellContainer*>(pCell->getNext());
	}
	setAdditionalBottomSpace(iYBreakLine - iYBreak);
	m_iNextWantedVBreak = iYBreak;
	return (iYBreak - getYBreak());
}


UT_sint32 fp_TableContainer::wantVBreakAtWithFootnotes(UT_sint32 vpos)
{
	// This function is not optimized at all as it may require building several times
	// nearly identical footnote vectors.
	UT_sint32 iYBreakMax = vpos + getYBreak();
	UT_sint32 iTotHeight = getTotalTableHeight();
	if (iYBreakMax > iTotHeight)
	{
		// check if there is enough space to fit the table and the footnotes on the page
		if (iYBreakMax > iTotHeight + sumFootnoteHeight())
		{
			return -1;
		}
	}

	UT_sint32 iOrigBottom = getYBottom();
	UT_sint32 vposHigh = vpos;
	UT_sint32 vposLow = 0;
	UT_sint32 iSumHigh = 0;
	UT_sint32 iSumLow = vpos;
	UT_sint32 k = 0;
	for (k = 0; k < 10; k++)
	{
		setYBottom(getYBreak() + vposHigh);
		iSumHigh = sumFootnoteHeight();
		if (vpos - iSumHigh != vposLow)
		{
			vposLow = vpos - iSumHigh;
		}
		else
		{
			break;
		}
		setYBottom(getYBreak() + vposLow);
		iSumLow = sumFootnoteHeight();
		if (vpos - iSumLow != vposHigh)
		{
			vposHigh = vpos - iSumLow;
		}
		else
		{
			break;
		}
		if (vposHigh == vposLow)
		{
			break;
		}
	}

	setYBottom(iOrigBottom);
	return wantVBreakAtNoFootnotes(vposLow);
}


/* 
   This function calculates the total height of all the footnotes in the broken table.
*/

UT_sint32 fp_TableContainer::sumFootnoteHeight(void) const
{
	UT_sint32 iSum = 0;
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (pTL->containsFootnoteLayouts())
	{
		UT_GenericVector<fp_FootnoteContainer*> vecFootnotes;
		getFootnoteContainers(&vecFootnotes);
		UT_sint32 i = 0;
		for(i = 0; i < vecFootnotes.getItemCount(); i++)
		{
			fp_FootnoteContainer * pFC = vecFootnotes.getNthItem(i);
			iSum += pFC->getHeight();
		}
		vecFootnotes.clear();
	}

	if (pTL->getDocLayout()->displayAnnotations() && pTL->containsAnnotationLayouts())
	{
		UT_GenericVector<fp_AnnotationContainer*> vecAnnotations;
		getAnnotationContainers(&vecAnnotations);
		UT_sint32 i = 0;
		for(i = 0; i < vecAnnotations.getItemCount(); i++)
		{
			fp_AnnotationContainer * pAC = vecAnnotations.getNthItem(i);
			iSum += pAC->getHeight();
		}
		vecAnnotations.clear();
	}

	return iSum;
}


fp_Container * fp_TableContainer::getFirstBrokenContainer() const
{
	return getFirstBrokenTable();
}


/*
  Return the height of the complete table as if it was not broken
*/

UT_sint32 fp_TableContainer::getTotalTableHeight(void) const
{
	if (getMasterTable())
	{
		return getMasterTable()->getTotalTableHeight();
	}

	return getYOfRow(getNumRows());
}





/*!
 * returns the first fp_Line of the table in this column by recursively 
 * searching down the table structure.
 */
fp_Line * fp_TableContainer::getFirstLineInColumn(fp_Column * pCol) const
{
	const fp_TableContainer * pTab = NULL;
	const fp_TableContainer * pBroke = NULL;
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
fp_Line * fp_TableContainer::getLastLineInColumn(fp_Column * pCol) const
{
	const fp_TableContainer * pTab = NULL;
	const fp_TableContainer * pBroke = NULL;
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
				if(!pLast)
					return NULL;
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

	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (!pTL->isInitialLayoutCompleted())
	{
		m_iCols = UT_MAX(m_iCols, child->getRightAttach());
		m_iRows = UT_MAX(m_iRows, child->getBottomAttach());
	}
	else
	{
		if (child->getRightAttach() >= m_iCols)
		{
			resize (m_iRows, child->getRightAttach());
		}

		if (child->getBottomAttach() >=  m_iRows)
		{
			resize (child->getBottomAttach(), m_iCols);
		}
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
	for (row = 0; row < getNumRows(); row++)
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

#if BENCHLAYOUT
	printf("Doing Table layout \n");
	timespec t1;
	clock_gettime(CLOCK_REALTIME, &t1);
#endif

	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (!pTL->isInitialLayoutCompleted())
	{
		resize(m_iRows,m_iCols);
	}
	static fp_Requisition requisition;
	static fp_Allocation alloc;
	sizeRequest(&requisition);
	setX(pTL->getLeftOffset());
	alloc.x = getX();
	alloc.y = getY();
	alloc.width = getWidth();
	alloc.height = requisition.height;
	sizeAllocate(&alloc);
	setToAllocation();
#if BENCHLAYOUT
	timespec t2;
	clock_gettime(CLOCK_REALTIME, &t2);	
	double millidiff = (t2.tv_sec-t1.tv_sec)*1e3 + (t2.tv_nsec-t1.tv_nsec)/1e6;
	printf("Layout TIME: %lf milliseconds\n", millidiff);  
#endif
}

void fp_TableContainer::setToAllocation(void)
{
	setWidth(m_MyAllocation.width);
	if(fp_VerticalContainer::getHeight() != m_MyAllocation.height)
	{
		//
		// clear and delete broken tables before their height changes.
		// Doing this clear at this point makes a table flicker when changing
		// height but it does remove the last the pixel dirt with tables.
		// 
		deleteBrokenTables(true,true);
	}
	setHeight(getTotalTableHeight());
	setMaxHeight(getTotalTableHeight());
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
		pCon->doVertAlign();
		pCon = static_cast<fp_CellContainer *>(pCon->getNext());
	}
	setYBottom(getTotalTableHeight());
}

void  fp_TableContainer::_size_request_init(void)
{
  UT_sint32 row, col;
  
  for (row = 0; row < m_iRows; row++)
  {
	  getNthRow(row)->requisition = 0;
  }
  m_iCols =  m_vecColumns.size();
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
	if(isThisBroken())
	{
		iWidth = getMasterTable()->getWidth();
	}
	else
	{
		iWidth = getWidth();
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
  	    fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff +  iWidth + 2 - pTL->getLeftOffset() - pTL->getRightOffset();
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
	xxx_UT_DEBUGMSG(("TablecONTAINER enter draw table yoff %ld \n",pDA->yoff));
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
	return m_vecRows.size();
}


UT_sint32 fp_TableContainer::getNumCols(void) const
{
	return m_vecColumns.size();
}

/*! 
 * Return the height of this Table taking into account the possibility
 * of it being broken.
 */
UT_sint32 fp_TableContainer::getHeight(void) const
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
bool fp_TableContainer::containsFootnoteReference(void) const
{
	// First check if there are footnotes in the whole table
	// This operation is quite fast
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (!pTL->containsFootnoteLayouts())
	{
		return false;
	}

	// Check if there are footnotes in the broken table
	fp_CellContainer * pCell = getFirstBrokenCell(false);

	bool bFound = false;
	while(pCell && !bFound)
	{
		if (getYOfRow(pCell->getTopAttach()) >= getYBottom())
		{
			break;
		}
		if ((pCell->getY() < getYBottom()) && (pCell->getY() + pCell->getHeight() >= getYBreak()))
		{
			bFound = pCell->containsFootnoteReference(this);
		}
		pCell = static_cast<fp_CellContainer*>(pCell->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of all the footnote object in the broken table
 */
bool fp_TableContainer::getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pVecFoots) const
{
	fp_CellContainer * pCell = getFirstBrokenCell(false);
	bool bFound = false;
	while(pCell)
	{
		if (getYOfRow(pCell->getTopAttach()) >= getYBottom())
		{
			break;
		}
		if ((pCell->getY() < getYBottom()) && (pCell->getY() + pCell->getHeight() >= getYBreak()) &&
			pCell->containsFootnoteReference(this))
		{
			bFound |= pCell->getFootnoteContainers(pVecFoots,this);
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	return bFound;
}


/*!
 * Return true if the table contains annotation references
 */
bool fp_TableContainer::containsAnnotations(void) const
{
	// First check if there are annotations in the whole table
	// This operation is quite fast
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	if (!pTL->containsAnnotationLayouts())
	{
		return false;
	}

	// Check if there are annotations in the broken table
	fp_CellContainer * pCell = getFirstBrokenCell(false);

	bool bFound = false;
	while(pCell && !bFound)
	{
		if (getYOfRow(pCell->getTopAttach()) >= getYBottom())
		{
			break;
		}
		if ((pCell->getY() < getYBottom()) && (pCell->getY() + pCell->getHeight() >= getYBreak()))
		{
			bFound = pCell->containsAnnotations(this);
		}
		pCell = static_cast<fp_CellContainer*>(pCell->getNext());
	}
	return bFound;
}

/*!
 * This method returns a vector of all the annotation object in the broken table
 */
bool fp_TableContainer::getAnnotationContainers(UT_GenericVector<fp_AnnotationContainer*>* pVecAnns) const
{
	fp_CellContainer * pCell = getFirstBrokenCell(false);
	bool bFound = false;
	while(pCell)
	{
		if (getYOfRow(pCell->getTopAttach()) >= getYBottom())
		{
			break;
		}
		if ((pCell->getY() < getYBottom()) && (pCell->getY() + pCell->getHeight() >= getYBreak()) &&
			pCell->containsAnnotations(this))
		{
			bFound |= pCell->getAnnotationContainers(pVecAnns,this);
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	return bFound;
}


/*!
 * Return true if the supplied Cell and its container are within this
 * broken container.
 */
bool fp_TableContainer::isInBrokenTable(const fp_CellContainer * pCell, fp_Container * pCon) const
{
	xxx_UT_DEBUGMSG(("isInBrokenTable %p pcell %p container %p\n",this,pCell,pCon));
	UT_sint32 iTop = pCell->getY() + pCon->getY();

	if ((iTop >= getYBreak() - 1) && (iTop < getYBottom()))
	{
		return true;
	}

	return false;
}


/*!
 * Draw that segment of the table that fits within the Y offsets of this
 * Broken table.
 */
void fp_TableContainer::_brokenDraw(dg_DrawArgs* pDA)
{
	xxx_UT_DEBUGMSG(("Drawing %d table in broken chain yoff %ld \n",getBrokenDraw(),pDA->yoff));
	xxx_UT_DEBUGMSG(("SEVIOR: _brokenDraw table %p getYBreak %d getYBottom %d \n",this, getYBreak(),getYBottom()));
	UT_sint32 iCountCells = 0;
	const UT_Rect * pClipRect = pDA->pG->getClipRect();

	fp_CellContainer * pCell = getFirstBrokenCell(false);

	xxx_UT_DEBUGMSG(("Drawing cells for table %p starting at cell %p \n",this,pCell));
	while(pCell)
	{
		dg_DrawArgs da = *pDA;
		da.yoff = da.yoff - getYBreak();

		if(getYOfRow(pCell->getTopAttach()) > getYBottom())
		{
			break;
		}
		else if (getYOfRow(pCell->getBottomAttach()) > getYBreak())
		{
			if(!pClipRect || (pCell->doesIntersectClip(this,pClipRect)))
			{
				pCell->drawBroken(&da, this);
				iCountCells++;
			}
			if(!m_pFirstBrokenCell)
			{
				m_pFirstBrokenCell = pCell;
			}
		}

		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}

	xxx_UT_DEBUGMSG(("_brokenDraw: Draw %d cells and check %d cells for table %p\n",iCountCells,this));
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
	if(!getPage() || (getPage()->getDocLayout()->getView() == NULL))
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
      m_iCols = m_vecColumns.size();
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
			  width += getNthCol(col)->spacing;
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
			  extra = (child_requisition.height + child->getTopPad() + child->getBotPad() - height)/
				  (child->getBottomAttach() - child->getTopAttach());		  
			  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
			  {
				  getNthRow(row)->requisition += extra;
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
  m_iCols = m_vecColumns.size();
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

  m_iCols = m_vecColumns.size();
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
  UT_sint32 width, height;
  UT_sint32 row, col;
  UT_sint32 nexpand;
  UT_sint32 nshrink;
  UT_sint32 extra;
  
  /* If we were allocated more space than we requested
   *  then we have to expand any expandable rows and columns
   *  to fill in the extra space.
   */
  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
  UT_sint32 real_width = m_MyAllocation.width - pTL->getLeftOffset() - pTL->getRightOffset();
  UT_sint32 real_height = m_MyAllocation.height - pTL->getTopOffset() - pTL->getBottomOffset();
  
  if (m_bIsHomogeneous)
  {
      nexpand = 0;
	  m_iCols = m_vecColumns.size();
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
      
	  m_iCols = m_vecColumns.size();
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
      for (col = 0; col < m_iCols; col++)
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
			  m_iCols = m_vecColumns.size();
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
	  for (row = 0; row < m_iRows; row++)
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
	UT_sint32 max_width;
	UT_sint32 max_height;
	UT_sint32 x, y;
	UT_sint32 row, col;
	fp_Allocation allocation;
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(getSectionLayout());
	const UT_GenericVector<fl_ColProps*> * pVecColProps = pTL->getVecColProps();
	if(pVecColProps->getItemCount() > 0)
	{
		for (col = 0; (col < pVecColProps->getItemCount()) && (col <getNumCols()); col++)
		{
			fl_ColProps * pColProp = pVecColProps->getNthItem(col);
			getNthCol(col)->allocation = pColProp->m_iColWidth - getNthCol(col)->spacing;
			if(col == (getNumCols() - 1) )
			{
				getNthCol(col)->allocation += 2 * getNthCol(col)->spacing;
			}
		}
	}
	m_MyAllocation.x = pTL->getLeftColPos() - pTL->getLeftOffset();

	x = m_MyAllocation.x + pTL->getLeftOffset();
	y = m_MyAllocation.y + pTL->getTopOffset();

	for (col = 0; col < m_iCols; col++)
	{
		fp_TableRowColumn * pCol = getNthCol(col);
		pCol->position = x;
		x += pCol->allocation + pCol->spacing;
	}
	UT_sint32 totalWidth = x;

	for (row = 0; row < m_iRows; row++)
	{
		fp_TableRowColumn * pRow = getNthRow(row);
		pRow->position = y;
		y += pRow->allocation + pRow->spacing;
	}
	UT_sint32 totalHeight = y;

	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getNthCon(0));
	while (pCell)
	{
		fp_Requisition pCell_requisition;
		pCell->sizeRequest(&pCell_requisition);

		UT_sint32 iLeft = pCell->getLeftAttach();
		UT_sint32 iRight = pCell->getRightAttach();
		x = getNthCol(iLeft)->position;
		UT_sint32 xspace = getNthCol(iLeft)->spacing;
		max_width = ((iRight < m_iCols) ? (getNthCol(iRight)->position) : totalWidth) - (x + xspace); 
		if (pCell->getXfill())
		{
			allocation.width = UT_MAX (1, max_width - static_cast<UT_sint32>(pCell->getLeftPad()) - pCell->getRightPad());
		}
		else
		{
			allocation.width = pCell_requisition.width;
		}
		allocation.x = x + pCell->getLeftPad() + xspace/2;

		UT_sint32 iTop = pCell->getTopAttach();
		UT_sint32 iBottom = pCell->getBottomAttach();
		y = getNthRow(iTop)->position;
		UT_sint32 yspace = getNthRow(iTop)->spacing;
		max_height = ((iBottom < m_iRows) ? (getNthRow(iBottom)->position) : totalHeight) - (y + yspace);
		if (pCell->getYfill())
		{
			allocation.height = UT_MAX (1, max_height - static_cast<UT_sint32>(pCell->getTopPad()) - pCell->getBotPad());
		}
		else
		{
			allocation.height = pCell_requisition.height;
		}
		allocation.y = y + pCell->getTopPad() + yspace/2;

		xxx_UT_DEBUGMSG(("SEVIOR!!!!!!: max_height = %d width =%d \n",max_height,allocation.width));
		pCell->sizeAllocate( &allocation);
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
}

fp_TableRowColumn * fp_TableContainer::getNthCol(UT_uint32 i) const
{
	UT_ASSERT(i < m_vecColumns.size());
	return m_vecColumns[i];
}

fp_TableRowColumn * fp_TableContainer::getNthRow(UT_uint32 i) const
{
	UT_ASSERT(i < m_vecRows.size());
	return m_vecRows[i];
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
  
  m_iCols = m_vecColumns.size();
  for (col = 0; col < m_iCols; col++)
  {
	  if(bDefinedColWidth && (col < pVecColProps->getItemCount()) )
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
	  pRequisition->height += pRow->requisition + pRow->spacing;
  }
  pRequisition->height += pTL->getTopOffset() + pTL->getBottomOffset();
  xxx_UT_DEBUGMSG(("SEVIOR: requisition height %d \n", pRequisition->height));
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
}
