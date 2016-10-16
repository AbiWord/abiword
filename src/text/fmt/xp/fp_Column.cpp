/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_TOCContainer.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "fp_TableContainer.h"
#include "fp_FootnoteContainer.h"
#include "fp_FrameContainer.h"
#include "fl_FootnoteLayout.h"
#include "fp_Run.h"
#include "fl_TOCLayout.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fv_View.h"
#include "gr_Painter.h"


/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_VerticalContainer::fp_VerticalContainer(FP_ContainerType iType, fl_SectionLayout* pSectionLayout) :
	fp_Container(iType, pSectionLayout),
	m_iRedrawHeight(-1),
	m_iWidth(0),
	m_iHeight(0),
	m_iMaxHeight(0),
	m_iX(0),
	m_iY(INITIAL_OFFSET),
	m_bIntentionallyEmpty(0),
	m_imaxContainerHeight(0)
{
	clearWrappedLines();
	UT_ASSERT(getDocSectionLayout());
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_VerticalContainer::~fp_VerticalContainer()
{
}

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_VerticalContainer::setWidth(UT_sint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	m_iWidth = iWidth;
	if(getContainerType() != FP_CONTAINER_COLUMN)
	{
	    getSectionLayout()->setImageWidth(iWidth);
	    getFillType().setWidth(getGraphics(),iWidth);
	}

	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

/*!
 * return a pointer to the current view.
 */
FV_View * fp_VerticalContainer::getView(void) const
{
  fp_Page * pPage = getPage();
  if(pPage == NULL)
  {
    return NULL;
  }
  FL_DocLayout * pDL = pPage->getDocLayout();
  if(pDL == NULL)
  {
    return NULL;
  }
  return pDL->getView();
}
/*!
 Set height
 \param iHeight Height of container
 */
void fp_VerticalContainer::setHeight(UT_sint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	if(getContainerType() == FP_CONTAINER_TABLE)
	{
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(this);
		if(!pTab->isThisBroken())
		{
			xxx_UT_DEBUGMSG(("Unbroken Table container set to %d from %d \n",iHeight,pTab->getHeight()));
		}
	}
	if(getContainerType() == FP_CONTAINER_TOC)
	{
		fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(this);
		if(!pTOC->isThisBroken())
		{
			UT_DEBUGMSG(("Unbroken TOC container set to %d from %d \n",iHeight,pTOC->getHeight()));
		}
	}
	m_iHeight = iHeight;
	if(getContainerType() != FP_CONTAINER_COLUMN)
	{
	  if(getContainerType() == FP_CONTAINER_CELL)
	  {
	      getSectionLayout()->setImageHeight(getMaxHeight()); // was iHeight
	  }
	  getFillType().setHeight(getGraphics(),iHeight);
	}
}

/*!
 Set maximum height
 \param iMaxHeight Maximum height of container
 */
void fp_VerticalContainer::setMaxHeight(UT_sint32 iMaxHeight)
{
	//UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}

	m_iMaxHeight = iMaxHeight;
}

/*!
  Get container's X position
  \return X position
*/
UT_sint32 fp_VerticalContainer::getX(void) const
{
	return m_iX;
}

/*!
  Get container's Y position.
  \return Y position
*/
UT_sint32 fp_VerticalContainer::getY(void) const
{
	if(getSectionLayout()->getDocLayout()->getView()  && (getSectionLayout()->getDocLayout()->getView()->getViewMode() != VIEW_PRINT))
	{
		fl_SectionLayout * pSL = getSectionLayout();
		fl_DocSectionLayout * pDSL = NULL;
		if(static_cast<fl_ContainerLayout *>(pSL)->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			pDSL = static_cast<fl_DocSectionLayout *>(pSL);
		}
		else
		{
			pDSL =  static_cast<fl_DocSectionLayout *>(pSL->getDocSectionLayout());
		}
		if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return m_iY - pDSL->getTopMargin();
		}
		return m_iY;
	}
	return m_iY;
}
/*!
  Get container's Y position. This version checks for a mismatch between view
 mode and if we're printing.
  \return Y position
*/
UT_sint32 fp_VerticalContainer::getY(GR_Graphics * pG) const
{
	if(getSectionLayout()->getDocLayout()->getView()  && (getSectionLayout()->getDocLayout()->getView()->getViewMode() != VIEW_PRINT) && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return m_iY - static_cast<fl_DocSectionLayout *>(getSectionLayout())->getTopMargin();
	}
	return m_iY;
}

/*!
 * This method returns the vertical offset due to a table broken
 * across more than 1 page.
 */
UT_sint32 fp_VerticalContainer::getYoffsetFromTable(fp_Container * pT,
													fp_Container* pCell,
													fp_ContainerObject * pCon)
{
	fp_TableContainer * pFirstTable = static_cast<fp_TableContainer *>(pT)->getFirstBrokenTable();
	fp_TableContainer * pTable = pFirstTable;
//	UT_ASSERT(pTable);
	UT_sint32 offset = 0;
	bool bFound = false;
	while(pTable != NULL && !bFound)
	{
		bFound = pTable->isInBrokenTable(static_cast<fp_CellContainer *>(pCell),static_cast<fp_Container *>(pCon));
		if(bFound)
		{
			offset = -pTable->getYBreak();
		}
		pTable = static_cast<fp_TableContainer *>(pTable->getNext());
	}
	return offset;
}


/*!
 * This method returns the correct broken table for this line.
 */
fp_TableContainer * fp_VerticalContainer::getCorrectBrokenTable(fp_Container * pCon)
{
	xxx_UT_DEBUGMSG(("VerticalContainer: In get Correct proken table \n"));
	bool bFound = false;
	fp_CellContainer * pCell = NULL;
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
	     pCell = static_cast<fp_CellContainer *>(pCon);
	     pCon = pCell->getFirstContainer();
	}
	else
	{ 
	     pCell = static_cast<fp_CellContainer *>(pCon->getContainer());
	     if(!pCell)
	     {
		   return NULL;
	     }
	}
	UT_return_val_if_fail(pCell->getContainerType() == FP_CONTAINER_CELL,NULL);
//
// OK scan through the broken tables and look for the table that contains this
//
	fp_Container * pCur = static_cast<fp_Container *>(pCell->getContainer());
	UT_return_val_if_fail(pCur->getContainerType() == FP_CONTAINER_TABLE,NULL);
	fp_TableContainer * pMasterTab = static_cast<fp_TableContainer *>(pCur);
	UT_return_val_if_fail(pMasterTab && pMasterTab->getContainerType() == FP_CONTAINER_TABLE,NULL);
	fp_TableContainer * pTab = pMasterTab->getFirstBrokenTable();
	bFound = false;
	UT_sint32 iCount  =0;
	while(pTab && !bFound)
	{
	        xxx_UT_DEBUGMSG(("getCorrectBrokenTable YBreak %d height %d \n",pTab->getYBreak(),pTab->getHeight()));
		if(pTab->isInBrokenTable(pCell,pCon))
		{
			bFound = true;
		}
		else
		{
			pTab = static_cast<fp_TableContainer *>(pTab->getNext());
		}
		if(!bFound)
		{
			iCount++;
		}
	}
	if(bFound)
	{
		xxx_UT_DEBUGMSG(("getCorrect: Found table after %d tries \n",iCount));
		xxx_UT_DEBUGMSG(("Container y %d height %d was found in table %d ybreak %d ybottom y %d \n",pCon->getY(),pCon->getHeight(),iCount,pTab->getYBreak(),pTab->getYBottom()));
		return  pTab;
	}
     
	xxx_UT_DEBUGMSG(("getCorrectBroken: No table found after %d tries, Y of Con \n",iCount,pCon->getY()));
	if(pMasterTab)
	{
//		UT_ASSERT(pMasterTab->getFirstBrokenTable() == NULL);
	}
	return pMasterTab;
}



/*!
 * This method returns the correct broken TOC for this line.
 */
fp_TOCContainer * fp_VerticalContainer::getCorrectBrokenTOC(fp_Container * pCon)
{
	xxx_UT_DEBUGMSG(("VerticalContainer: In get Correct proken TOC \n"));
	bool bFound = false;
//
// OK scan through the broken TOC's and look for the TOC that contains this
//
	fp_Container * pCur = static_cast<fp_Container *>(pCon->getContainer());
	UT_return_val_if_fail(pCur->getContainerType() == FP_CONTAINER_TOC,NULL);
	fp_TOCContainer * pMasterTOC = static_cast<fp_TOCContainer *>(pCur);
	UT_return_val_if_fail(pMasterTOC && pMasterTOC->getContainerType() == FP_CONTAINER_TOC,NULL);
	fp_TOCContainer * pTOC = pMasterTOC->getFirstBrokenTOC();
	bFound = false;
	UT_sint32 iCount  =0;
	while(pTOC && !bFound)
	{
		if(pTOC->isInBrokenTOC(pCon))
		{
			bFound = true;
		}
		else
		{
			pTOC = static_cast<fp_TOCContainer *>(pTOC->getNext());
		}
		if(!bFound)
		{
			iCount++;
		}
	}
	if(bFound)
	{
		xxx_UT_DEBUGMSG(("getCorrect: Found table after %d tries \n",iCount));
		xxx_UT_DEBUGMSG(("Container y %d height %d was found in table %d ybreak %d ybottom y %d \n",pCon->getY(),pCon->getHeight(),iCount,pTab->getYBreak(),pTab->getYBottom()));
		return  pTOC;
	}
	xxx_UT_DEBUGMSG(("getCorrectBrokenTOC: NoTOC found after %d tries, Y of Con \n",iCount,pCon->getY()));
	if(pMasterTOC)
	{
//		UT_ASSERT(pMasterTOC->getFirstBrokenTOC() == NULL);
	}
	return pMasterTOC;
}

/*!
  Get line's offsets relative to this container
 \param  pContainer Container
 \retval xoff Container's X offset relative to container
 \retval yoff Container's Y offset relative to container
 */
void fp_VerticalContainer::getOffsets(fp_ContainerObject* pContainer, UT_sint32& xoff, UT_sint32& yoff)
{
	fp_ContainerObject * pOrig = pContainer;
	UT_sint32 my_xoff = 0;
	UT_sint32 my_yoff = 0;
	fp_Container * pCon = static_cast<fp_Container *>(this);
	fp_Container * pPrev = NULL;
	fp_TableContainer * pTab = NULL;
	while(pCon && !pCon->isColumnType())
	{
		my_xoff += pCon->getX();
		xxx_UT_DEBUGMSG(("my_xoff = %d pCon %x Type is %s \n",my_xoff,pCon,pCon->getContainerString()));
		UT_sint32 iycon = pCon->getY();
		my_yoff += iycon;
//
// Handle offsets from tables broken across pages.
//
// We detect
// line->cell->table->cell->table->cell->table->column
//
		if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_VerticalContainer * pVCon= static_cast<fp_VerticalContainer *>(pCon);
//
// Lines and Cells are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we hijack the recursive search and
// move it up the correct broken table line when we come across a cell
//
			pVCon = getCorrectBrokenTable(static_cast<fp_Container *>(pContainer));
			if(pPrev && pPrev->getContainerType() == FP_CONTAINER_CELL)
			{
				UT_sint32 iTable =  getYoffsetFromTable(pCon,pPrev,pContainer);
				my_yoff += iTable;
				pTab = static_cast<fp_TableContainer *>(pVCon);
				if(pTab->isThisBroken() && (pTab != pTab->getMasterTable()->getFirstBrokenTable()))
				{
					my_yoff = my_yoff + pVCon->getY() -iycon;
				}
			}
			if(pVCon && pVCon->getContainer() && (pVCon->getContainer()->getContainerType() == FP_CONTAINER_CELL))
			{
				pContainer = static_cast<fp_Container *>(pVCon);
				xxx_UT_DEBUGMSG(("pContainer set to %p height %d \n",pContainer,pContainer->getHeight()));
			}
			else if(pVCon && (pVCon->getContainer() == NULL))
			{
			  //
			  // Just bail out for now
			  //
			        return;
			}
			if(pVCon == NULL)
			{
			        pCon = NULL;
				break;
			}
			pCon = static_cast<fp_Container *>(pVCon);
		}
		if(pCon->getContainerType() == FP_CONTAINER_TOC)
		{
			fp_VerticalContainer * pVCon= static_cast<fp_VerticalContainer *>(pCon);
//
// Lines and Cells are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we hijack the recursive search and
// move it up the correct broken table line when we come across a cell
//
			pVCon = getCorrectBrokenTOC(static_cast<fp_Container *>(pContainer));
			pCon = static_cast<fp_Container *>(pVCon);
		}
		pPrev = pCon;
		pCon = pCon->getContainer();
	}
	if(pCon && pCon->getContainerType() == FP_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout*	pHFSL = static_cast<fp_HdrFtrContainer *>(pCon)->getHdrFtrSectionLayout();
		fp_Page * pPage = getPage();
		fl_HdrFtrShadow * pShadowL = NULL;
		if(pPage == NULL)
		{
			pShadowL = pHFSL->getFirstShadow();
		}
		else
		{
			pShadowL = pHFSL->findShadow(pPage);
		}
		if(pShadowL == NULL)
		{
			return;
		}
//		UT_ASSERT(pShadowL);
		if(pShadowL)
		{
			pCon = static_cast<fp_Container *>(pShadowL->getFirstContainer());
		}
		else
		{
			return;
		}
	}

	UT_sint32 col_x =0;
	UT_sint32 col_y =0;
	if(pPrev && ((pPrev->getContainerType() == FP_CONTAINER_TABLE) ||
				 (pPrev->getContainerType() == FP_CONTAINER_TOC)))
	{
		if(pCon->getContainerType() == FP_CONTAINER_COLUMN)
		{
			UT_sint32 col_xV =0;
			UT_sint32 col_yV =0;
			fp_Column * pCol = static_cast<fp_Column *>(pCon);
			pCol->getPage()->getScreenOffsets(pCol, col_xV, col_yV);
			pCol =static_cast<fp_Column *>(pCon->getColumn());
			pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);
			UT_sint32 ydiff = col_yV - col_y;
			my_yoff += ydiff;
		}
		xoff = pCon->getX() + my_xoff + pOrig->getX();
		yoff = pCon->getY() + my_yoff + pOrig->getY();
		if ((pPrev->getContainerType() == FP_CONTAINER_TOC) &&
			(pCon->getContainerType() != FP_CONTAINER_COLUMN_SHADOW))
		{
			xxx_UT_DEBUGMSG(("Not in shadow final xoff %d \n",xoff));
			return;
		}
	}

	if(pCon && pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		xoff = pCon->getX() + my_xoff + pOrig->getX();
		yoff = pCon->getY() + my_yoff + pOrig->getY();
		xxx_UT_DEBUGMSG(("Offsets in FP_CONTAINER_COLUMN_SHADOW x= %d \n",xoff));
		return;
	}
	if(pCon)
	{
		xoff = pCon->getX() + my_xoff + pOrig->getX();
		yoff = pCon->getY() + my_yoff + pOrig->getY();
	}
	else
	{
		xoff = 0;
		yoff = 0;
	}
	if(pCon && pCon->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
	        if(getPage() && getView() && (getView()->getViewMode() != VIEW_PRINT))
		{
		       fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		       yoff -= pDSL->getTopMargin();
		}
	}
	if(pCon && getPage() && (pCon->getContainerType() == FP_CONTAINER_ANNOTATION) && 
	   getPage()->getDocLayout()->displayAnnotations())
	{
	        if(getPage() && getView() && (getView()->getViewMode() != VIEW_PRINT))
		{
		       fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		       yoff -= pDSL->getTopMargin();
		}
	}
}



/*!
 * return an rectangle that covers this object on the screen
 * The calling routine is resposible for deleting the returned struct
 */
UT_Rect * fp_VerticalContainer::getScreenRect(void)
{
	UT_sint32 xoff = 0;
	UT_sint32 yoff = 0;
	UT_Rect * pRec = NULL;
	if(getContainerType() == FP_CONTAINER_FRAME)
	{
		fp_Page * pPage = getPage();
		if(pPage == NULL)
		{
			return NULL;
		}
		fp_FrameContainer * pFrameC = static_cast<fp_FrameContainer *>(this);
		getView()->getPageScreenOffsets(pPage,xoff,yoff);
		xoff += pFrameC->getFullX();
		yoff += pFrameC->getFullY();
		pRec= new UT_Rect(xoff,yoff,pFrameC->getFullWidth(),pFrameC->getFullHeight());
		return pRec;
	}
	fp_Container * pCon = static_cast<fp_Container *>(fp_Container::getNthCon(0));
	if(pCon == NULL)
	{
		return NULL;
	}
	getScreenOffsets(pCon,xoff,yoff);
	xoff -= pCon->getX();
	yoff -= pCon->getY();
	pRec= new UT_Rect(xoff,yoff,getWidth(),getHeight());
	return pRec;
}

/*!
 * Marks Dirty any runs that overlap the supplied rectangle. This rectangle
 * is relative to the screen.
 */
void fp_VerticalContainer::markDirtyOverlappingRuns(UT_Rect & recScreen)
{
	UT_Rect * pRec = NULL;
	pRec = getScreenRect();
	if(pRec && recScreen.intersectsRect(pRec))
	{
		DELETEP(pRec);
		UT_sint32 count = countCons();
		UT_sint32 i = 0;
		for(i = 0; i < count;i++)
		{
			fp_Container * pCon = static_cast<fp_Container * >(getNthCon(i));
			pCon->markDirtyOverlappingRuns(recScreen);
		}
		return;
	}
	DELETEP(pRec);
}


/*!
  Get Containers' offsets relative to the screen
 \param  pContainer Container which we want to find the absolute
                    position of.
 \retval xoff Container's X offset relative the screen
 \retval yoff Container's Y offset relative the screen
 */
void fp_VerticalContainer::getScreenOffsets(fp_ContainerObject* pContainer,
									UT_sint32& xoff, UT_sint32& yoff)
{
	fp_ContainerObject * pOrig = pContainer;
	UT_sint32 my_xoff =0;
	UT_sint32 my_yoff =0;

	if((getPage() == NULL) || (pContainer == NULL))
	{
		xoff = 0;
		yoff = 0;
		return;
	}

	fp_Container * pCon = static_cast<fp_Container *>(this);
	bool bCell = false;
	bool bTable = false;
	UT_sint32 xcell = 0;
	UT_sint32 ycell = 0;
	if((getContainerType() == FP_CONTAINER_TABLE) && (pContainer->getContainerType() == FP_CONTAINER_CELL))
	{
	        pCon =  static_cast<fp_Container *>(pContainer);
	        pContainer = static_cast<fp_CellContainer *>(pContainer)->getNthCon(0);
		if(pContainer != NULL)
		{
		  bCell = true;
		  xcell = pContainer->getX();
		  ycell = pContainer->getY();
		}
		else
		{
		  bTable = true;
		  pContainer = pCon;
		  pCon = static_cast<fp_Container *>(this);
		  my_yoff = getY();
		  my_xoff = getX();
		}
	}
	fp_Container * pPrev = NULL;
	fp_TableContainer * pTab = NULL;
	while(pCon && !pCon->isColumnType() && !bTable)
	{
		my_xoff += pCon->getX();
		xxx_UT_DEBUGMSG(("Screen offsets my_xoff %d pCon %x type %s \n",my_xoff,pCon,pCon->getContainerString()));
		UT_sint32 iycon = pCon->getY();
		my_yoff += iycon;
//
// Handle offsets from tables broken across pages.
//
// We detect
// line->cell->table->cell->table->cell->table->column
//
		if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_VerticalContainer * pVCon= static_cast<fp_VerticalContainer *>(pCon);
//
// Lines and Cells are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we put in a recursive search find
// the correct broken table line when we come across a cell
//
// Then we have to get all the offsets right for the broken table.
//

			pVCon = getCorrectBrokenTable(static_cast<fp_Container *>(pContainer));
//
// Can happen during loading.
//
			if(pVCon == NULL)
			{
				xoff = 0;
				yoff = 0;
				return;
			}
			if(pPrev && pPrev->getContainerType() == FP_CONTAINER_CELL)
			{
				my_yoff += getYoffsetFromTable(pCon,pPrev,pContainer);
				pTab = static_cast<fp_TableContainer *>(pVCon);
				if(pTab->isThisBroken() && pTab != pTab->getMasterTable()->getFirstBrokenTable())
				{
					my_yoff = my_yoff + pVCon->getY() -iycon;
				}
				pCon = static_cast<fp_Container *>(pVCon);
			}
			else if(pPrev == NULL)
			{
			        my_yoff = 0;
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			if(pVCon->getContainer() && pVCon->getContainer()->getContainerType() == FP_CONTAINER_CELL)
			{
				pContainer = static_cast<fp_ContainerObject *>(pVCon);
			}
			pCon = static_cast<fp_Container *>(pVCon);
		}
		if(pCon->getContainerType() == FP_CONTAINER_TOC)
		{
			fp_VerticalContainer * pVCon= static_cast<fp_VerticalContainer *>(pCon);
//
// Lines are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we put in a recursive search find
// the correct broken table line when we come across a cell
//
// Then we have to get all the offsets right for the broken table.
//

			pVCon = getCorrectBrokenTOC(static_cast<fp_Container *>(pContainer));
			pCon = static_cast<fp_Container *>(pVCon);
		}
		pPrev = pCon;
		pCon = pCon->getContainer();
		if (!pCon)
		{
			// Can happen during loading
			xoff = 0;
			yoff = 0;
			return;
		}
	}
	UT_return_if_fail(pCon);
	UT_sint32 col_x =0;
	UT_sint32 col_y =0;
	xoff = my_xoff + pOrig->getX();
	yoff = my_yoff + pOrig->getY();
	if(bCell)
	{
	        xoff -= xcell;
		yoff -= ycell;
	}
	if (pCon->getContainerType() == FP_CONTAINER_COLUMN)
	{
		fp_Column * pCol = static_cast<fp_Column *>(pCon);
		pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	}
	else if (pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		fp_ShadowContainer * pCol = static_cast<fp_ShadowContainer *>(pCon);
		pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	}
	else if(pCon->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
		fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(pCon);
		pFC->getPage()->getScreenOffsets(pFC, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	        if(pFC->getPage() && getView() && (getView()->getViewMode() != VIEW_PRINT))
		{
		       fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		       yoff -= pDSL->getTopMargin();
		}
	}
	else if(pCon->getContainerType() == FP_CONTAINER_ANNOTATION)
	{
		fp_AnnotationContainer * pAC = static_cast<fp_AnnotationContainer *>(pCon);
		pAC->getPage()->getScreenOffsets(pAC, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	        if(pAC->getPage() && getView() && (getView()->getViewMode() != VIEW_PRINT))
		{
		       fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		       yoff -= pDSL->getTopMargin();
		}
	}
	else if(pCon->getContainerType() == FP_CONTAINER_FRAME)
	{
		fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pCon);
		pFC->getPage()->getScreenOffsets(pFC, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
}


/*!
 * remove all contains from this.
 */
void fp_VerticalContainer::removeAll(void)
{
        UT_sint32 iCount = countCons();
	UT_sint32 i = 0;
	for(i=0; i< iCount; i++)
        {
	     deleteNthCon(0);
	}
}
/*!
 Remove line from container
 \param pContainer Container
 \param bClear if true clear screen.
 \note The line is not destructed, as it is owned by the logical
       hierarchy.
 */
void fp_VerticalContainer::removeContainer(fp_Container* pContainer,bool bClear)
{
	UT_sint32 iCount = countCons();
	if(iCount == 0)
		return;
	UT_sint32 ndx = findCon(pContainer);
	UT_ASSERT(ndx >= 0);
	if(ndx < 0)
	{
		return;
	}
	if(bClear && (pContainer->getContainerType() == FP_CONTAINER_LINE))
	{
		pContainer->clearScreen();
	}
	xxx_UT_DEBUGMSG(("Removing Container %x from column %x \n",pContainer,this));
	pContainer->setContainer(NULL);
	deleteNthCon(ndx);

	// don't delete the line here, it's deleted elsewhere.
}

/*!
 Insert line at the front/top of the container
 \param pNewContainer Container
 */
bool fp_VerticalContainer::insertContainer(fp_Container* pNewContainer)
{
	UT_return_val_if_fail(pNewContainer,false);
	UT_return_val_if_fail((pNewContainer->getContainerType() == FP_CONTAINER_ENDNOTE) || (pNewContainer->getDocSectionLayout() == getDocSectionLayout()),false);
	UT_ASSERT(pNewContainer->getContainerType() != FP_CONTAINER_ANNOTATION);
	pNewContainer->clearScreen();
	xxx_UT_DEBUGMSG(("Insert  Container after CS %x in column %x \n",pNewContainer,this));
	insertConAt(pNewContainer, 0);
	pNewContainer->setContainer(static_cast<fp_Container *>(this));
	pNewContainer->recalcMaxWidth(true);

	return true;
}

/*!
  Get column gap from page the container is located on
  \return Column gap
*/
UT_sint32	fp_VerticalContainer::getColumnGap(void) const
{
	return getColumn()->getPage()->getColumnGap();
}

/*!
 Append line at the end/bottom of the container
 \param pNewContainer Container
 */
bool fp_VerticalContainer::addContainer(fp_Container* pNewContainer)
{
	UT_return_val_if_fail(pNewContainer,false);
	UT_return_val_if_fail((pNewContainer->getContainerType() == FP_CONTAINER_ENDNOTE) || (pNewContainer->getDocSectionLayout() == getDocSectionLayout()),false);
	UT_ASSERT(pNewContainer->getContainerType() != FP_CONTAINER_ANNOTATION);
	if(pNewContainer->getContainer() != NULL)
	{
		pNewContainer->clearScreen();
	}
	xxx_UT_DEBUGMSG(("Add  Container after CS %x in column %x \n",pNewContainer,this));
	addCon(pNewContainer);
	pNewContainer->setContainer(this);
	pNewContainer->recalcMaxWidth(true);
	return true;
}

/*!
 Insert line in container after specified line
 \param pNewContainer   Container to be inserted
 \param pAfterContainer After this line
 \todo This function has been hacked to handle the case where
       pAfterContainer is NULL. That case should not happen. Bad callers
       should be identified and fixed, and this function should be
       cleaned up.
 */
bool fp_VerticalContainer::insertContainerAfter(fp_Container*	pNewContainer, fp_Container*	pAfterContainer)
{
	UT_ASSERT(pAfterContainer);
	UT_return_val_if_fail(pNewContainer, false);
	UT_return_val_if_fail((pNewContainer->getContainerType() == FP_CONTAINER_ENDNOTE) || (pNewContainer->getDocSectionLayout() == getDocSectionLayout()),false);
	UT_ASSERT(pNewContainer->getContainerType() != FP_CONTAINER_ANNOTATION);

	UT_sint32 count = countCons();
	UT_sint32 ndx = findCon(pAfterContainer);
	UT_ASSERT( (count > 0) || (ndx == -1) );

	/*
	  TODO this routine should not be allowing pAfterContainer to be NULL.
	  Right now, we've fixed the symptom, but we really should fix
	  the problem.  */
	UT_ASSERT(ndx >= 0);
	pNewContainer->clearScreen();
	if ( (ndx+1) == count )				// append after last line in vector
		addCon(pNewContainer);
	else if (ndx >= 0)					// append after this item within the vector
		insertConAt(pNewContainer, ndx+1);
	else
	{
		// TODO remove this....
		insertConAt(pNewContainer, 0);
	}

	pNewContainer->setContainer(this);
	if(pNewContainer->getContainerType() == FP_CONTAINER_LINE)
	{
		if(static_cast<fp_Line *>(pNewContainer)->isWrapped())
		{
			return true;
		}
	}
	pNewContainer->recalcMaxWidth(true);

	return true;
}

/*!
  Clear container content from screen.

  \fixme Needs to clear outline as well
*/
void fp_VerticalContainer::clearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	if(!getPage()->isOnScreen())
	{
		return;
	}
	int count = countCons();
	for (int i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));

		pContainer->clearScreen();
	}
}

/*!
 Draw container outline
 \param pDA Draw arguments
 */
void fp_VerticalContainer::_drawBoundaries(dg_DrawArgs* pDA)
{
    if(pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}
	UT_return_if_fail(getPage());
	UT_return_if_fail(getPage()->getDocLayout()->getView());

    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff - getGraphics()->tlu(1);
        UT_sint32 yoffBegin = pDA->yoff - getGraphics()->tlu(1);
        UT_sint32 xoffEnd = pDA->xoff + m_iWidth + getGraphics()->tlu(2);
        UT_sint32 yoffEnd = pDA->yoff + m_iMaxHeight + getGraphics()->tlu(2);

		UT_RGBColor clrShowPara(127,127,127);

		GR_Painter painter(getGraphics());

		getGraphics()->setColor(clrShowPara);

        painter.drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        painter.drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        painter.drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        painter.drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 * Returns the maximum line height as determined from the layout method
 * This used by the draw method to determine if a line should be drawn in
 * a clipping rectangle
 */
UT_sint32 fp_VerticalContainer::_getMaxContainerHeight(void) const
{
	return m_imaxContainerHeight;
}

/*!
 * Set the maximum line Height
\param UT_sint32 iLineHeight the largest line height yet found.
 */
void fp_VerticalContainer::_setMaxContainerHeight( UT_sint32 iLineHeight)
{
	m_imaxContainerHeight = iLineHeight;
}


bool fp_VerticalContainer::validate(void)
{
#if DEBUG
	UT_sint32 curTop =0;
	UT_sint32 curBot = 0;
	UT_sint32 oldTop = -1;
	UT_sint32 oldBot = -1;
	UT_sint32 i =0;
	bool bValid = true;
	for(i=0; i<countCons();i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
		curTop = pContainer->getY();
		UT_sint32 iH = pContainer->getHeight();
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
			iH = pTab->getHeight();
		}
		if(pContainer->getContainerType() == FP_CONTAINER_TOC)
		{
			fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(pContainer);
			iH = pTOC->getHeight();
		}
		if(pContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			fp_Line * pLine = static_cast<fp_Line *>(pContainer);
			if(pLine->isSameYAsPrevious())
			{
				continue;
			}
		}
		curBot = curTop + iH;
		UT_ASSERT(oldBot <= curTop);
		if(oldBot > curTop)
		{
			bValid =false;
		}
		UT_ASSERT(curBot >= curTop);
		UT_ASSERT(curTop >=oldTop);
		oldBot = curBot;
		oldTop = curTop;
	}
	return bValid;
#else
	return true;
#endif
}


/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_VerticalContainer::draw(dg_DrawArgs* pDA)
{
#if DEBUG
//	validate();
#endif
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop = 0, ybot = (UT_sint32)(((UT_uint32)(1<<31)) - 1);

	if(pClipRect)
	{
		ytop = pClipRect->top;
		ybot = UT_MAX(pClipRect->height,_getMaxContainerHeight())
			+ ytop + pDA->pG->tlu(1);
		xxx_UT_DEBUGMSG(("clipRect height %d \n",pClipRect->height));
	}

//
// Only draw the lines in the clipping region.
//
	bool bStartedDrawing = false;
	dg_DrawArgs da = *pDA;
	UT_uint32 count = countCons();
	UT_sint32 iCurrHeight = 0;
	xxx_UT_DEBUGMSG(("number of container %d \n",count));
	for (UT_uint32 i = 0; i < count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
		if(pContainer->getY() == INITIAL_OFFSET)
		  continue ; // container is not yet placed 
		bool bInTable = false;
		bool bInTOC = false;

		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();
		iCurrHeight = pContainer->getY() + pContainer->getHeight();
		xxx_UT_DEBUGMSG(("iCurrHeight %d | m_iRedrawHeight %d\n", iCurrHeight, m_iRedrawHeight));
		if((m_iRedrawHeight > 0) && (iCurrHeight >  m_iRedrawHeight))
		{
		    da.bDirtyRunsOnly = false;
		}
		xxx_UT_DEBUGMSG(("Draw container %x yoff %d\n",pContainer,da.yoff));
		xxx_UT_DEBUGMSG(("Draw container %x xoff %d\n",pContainer,da.xoff));
#if 0
		if(pContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			fp_Line * pLine = static_cast<fp_Line *>(pContainer);
			if(pLine->isSameYAsPrevious())
			{
				UT_DEBUGMSG((" !!!!!! Same previous!!!!!!!!\n"));
			}
		}
#endif
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
			if(pTab->isThisBroken())
				da.xoff = pDA->xoff + pTab->getMasterTable()->getX();

			UT_sint32 iTableBot = da.yoff + pTab->getHeight();
			/* we're in the table if iTableBot < ytop, or table top > ybot */
			bInTable = !(iTableBot < ytop || da.yoff > ybot);
		}

		if(pContainer->getContainerType() == FP_CONTAINER_TOC)
		{
			fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(pContainer);
			xxx_UT_DEBUGMSG(("Draw a TOC getY is %d \n",pContainer->getY()));
			if(pTOC->isThisBroken())
				da.xoff = pDA->xoff + pTOC->getMasterTOC()->getX();

			UT_sint32 iTOCBot = da.yoff + pTOC->getHeight();
			/* we're in the table if iTableBot < ytop, or table top > ybot */
			bInTOC = !(iTOCBot < ytop || da.yoff > ybot);
		}

		UT_sint32 sumHeight = static_cast<long>(pContainer->getHeight()) + (ybot-ytop);
		UT_sint32 totDiff;
		if(da.yoff < ytop)
			totDiff = ybot - da.yoff;
		else
			totDiff = da.yoff + pContainer->getHeight() - ytop;

//		if(bTable || (da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
		if((bInTable || bInTOC) || (totDiff < sumHeight)  || (pClipRect == NULL))
		{
			bStartedDrawing = true;
			pContainer->draw(&da);
		}
		else if(bStartedDrawing)
		{
			// we've started drawing and now we're not, so we're done.
			break;
		}
	}
	m_iRedrawHeight = -1;
	_drawBoundaries(pDA);
}

/*!
  Find document position from X and Y coordinates
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
 */
void fp_VerticalContainer::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
								   bool& bBOL, bool& bEOL, bool &isTOC)
{
	int count = countCons();
	if(getContainerType() == FP_CONTAINER_TOC)
	{
		fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getSectionLayout());
		getPage()-> setLastMappedTOC(pTOCL);
		isTOC = true;
	}
	else if(getContainerType() == FP_CONTAINER_COLUMN)
	{
		isTOC = false;
	}
	xxx_UT_DEBUGMSG(("SEVIOR: count cons %d x %d y %d \n",count,x,y));
	if(count == 0)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: In container type %d return with bBOL set \n",getContainerType()));
		if(getContainerType() == FP_CONTAINER_TABLE)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Table container with no containers \n"));
			return;
		}
		if(getContainerType() == FP_CONTAINER_TOC)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: TOC container with no containers \n"));
			return;
		}
		pos = 2;
		bBOL = true;
		bEOL = true;
		return;
	}

	fp_ContainerObject* pContainer = NULL;

	if (getContainerType() == FP_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(this);
		UT_sint32 i = 0;
		while((i < count - 1) && (getNthCon(i)->getY() < y))
		{
			i++;
		}

		if((i == 0) && (x < getX()) && (pCell->getLeftAttach() == 0))
		{
			fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
			pos = pCL->getPosition(true)+2;
			bBOL = true;
			bEOL = false;
			return;
	    }

		if ((i == 0) || (getNthCon(i)->getY() <= y))
		{
			pContainer = getNthCon(i);
		}
		else
		{
			fp_ContainerObject * pContainerAbove = getNthCon(i - 1);
			fp_ContainerObject * pContainerBelow = getNthCon(i);

			if (pContainerAbove->getY() + pContainerAbove->getHeight() < y)
			{
				pContainer = pContainerAbove;
			}
			else
			{
				// y falls between two containers. Check if the containers are in two different broken tables.
				// If this is the case, choose the container in the same table as y. Else choose the container
				// closest to y.
				fp_TableContainer * pBrokeAbove = pCell->getBrokenTable(static_cast<fp_Container*>(pContainerAbove));
				fp_TableContainer * pBrokeBelow = pCell->getBrokenTable(static_cast<fp_Container*>(pContainerBelow));
				if (pBrokeAbove != pBrokeBelow)
				{
					pContainer = ((y + getY() < pBrokeAbove->getYBottom()) ? pContainerAbove : pContainerBelow);
				}
				else
				{
					UT_sint32 disAbove = y - pContainerAbove->getY() - pContainerAbove->getHeight();
					UT_sint32 disBelow = pContainerBelow->getY() - y;
					pContainer = ((disAbove <= disBelow) ? pContainerAbove : pContainerBelow);
				}
			}
		}
	}
	else
	{
		UT_sint32 i = 0;
		// Find first container that contains the point. First has its lower level below the desired Y
		// position. Note that X-positions are completely ignored here.
		UT_sint32 iHeight = 0;
		do
		{
			pContainer = static_cast<fp_ContainerObject*>(getNthCon(i++));
			iHeight = pContainer->getHeight();
			xxx_UT_DEBUGMSG(("SEVIOR: IN column looking at x %d y %d height %d \n",pContainer->getX(),pContainer->getY(),iHeight));
		} while ((i < count) && (y > (pContainer->getY() + iHeight)));
		// Undo the postincrement.
		i--;
		// Now check if the position is actually between the found container
		// and the line before it (ignore check on the top-most line).
		UT_sint32 iUHeight =0;
		if (i > 0 && y < pContainer->getY())
		{
			fp_ContainerObject* pContainerUpper = static_cast<fp_ContainerObject*>(getNthCon(i-1));
			iUHeight = pContainer->getHeight();

			// Be careful with the signedness here - bug 172 leared us a
			// lesson!

			// Now pick the line that is closest to the point - or the
			// upper if it's a stalemate.
			if ((pContainer->getY() - y) >= (y - (pContainerUpper->getY() + static_cast<UT_sint32>(iUHeight))))
			{
				pContainer = pContainerUpper;
			}
		}
	}

	if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Looking in a table \n"));
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
		xxx_UT_DEBUGMSG(("SEVIOR: do map to position for %x \n",pContainer));
		pTab->mapXYToPosition(x - pContainer->getX(),
								y - pContainer->getY() ,
								pos, bBOL, bEOL,isTOC);
	}
	else if(pContainer->getContainerType() == FP_CONTAINER_FRAME)
	{
		fp_FrameContainer * pFrame = static_cast<fp_FrameContainer *>(pContainer);
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFrame->getSectionLayout());
		if(pFL->getFrameType() == FL_FRAME_WRAPPER_IMAGE)
		{
			pos = pFL->getPosition(true);
			return;
		}
		else
		{
			pContainer->mapXYToPosition(x - pContainer->getX(),
										y - pContainer->getY() ,
										pos, bBOL, bEOL,isTOC);
		}

	}
	else if(pContainer->getContainerType() == FP_CONTAINER_LINE)
	{
//
// Deal with wrapped lines where more than one line can have the same Y
//
		fp_Line * pLine2 = static_cast<fp_Line *>(pContainer);
		if(pLine2->isWrapped())
		{
			fp_Line * pNext = static_cast<fp_Line *>(pLine2->getNext());
			if(pNext && pNext->isSameYAsPrevious())
			{
				fp_ContainerObject *pBest = pContainer;
				UT_sint32 xmin = UT_MIN(abs(pNext->getX() - x),abs(pNext->getX()+pNext->getMaxWidth() -x));
				while(pNext && pNext->isSameYAsPrevious())
				{
					if((pNext->getX() < x) && (x < pNext->getX() + pNext->getMaxWidth()))
					{
						pNext->mapXYToPosition(x - pNext->getX(),
											   y - pNext->getY() ,
											   pos, bBOL, bEOL,isTOC);
						return;
					}
					UT_sint32 xmin1 = UT_MIN(abs(pNext->getX() - x),abs(pNext->getX()+pNext->getMaxWidth() -x));
					if(xmin1 < xmin)
					{
						xmin = xmin1;
						pBest = static_cast<fp_ContainerObject *>(pNext);
					}
					pNext = static_cast<fp_Line *>(pNext->getNext());
				}
				pBest->mapXYToPosition(x - pContainer->getX(),
									   y - pContainer->getY() ,
									   pos, bBOL, bEOL,isTOC);
				return;
			}
			else
			{
				pContainer->mapXYToPosition(x - pContainer->getX(),
											y - pContainer->getY() ,
											pos, bBOL, bEOL,isTOC);
			}
		}
		else if(!pLine2->canContainPoint())
		{
			// lines that cannot contain point are those that are located in blocks that
			// cannot contain point (hidden, collapsed, etc. So we need to find the block
			// that can contain point
			fl_BlockLayout * pBlock = pLine2->getBlock();
			UT_return_if_fail( pBlock );

			pBlock = pBlock->getNextBlockInDocument();

			while(pBlock && !pBlock->canContainPoint())
			{
				pBlock = pBlock->getNextBlockInDocument();
			}

			if(!pBlock)
			{
				// look the other way (reusing pNext, even though it will be previous)
				pBlock = pLine2->getBlock();

				pBlock = pBlock->getPrevBlockInDocument();

				while(pBlock && !pBlock->canContainPoint())
				{
					pBlock = pBlock->getPrevBlockInDocument();
				}

			}

			if(!pBlock)
			{
				// we are in trouble
				// no blocks that can take point in this document !!!
				// one of the scenarios in which this happens is when the user did ctrl+a -> del
				// in revisions mode or marked everything hidden while fmt marks are not showing
				// If there is a block and it is not visible, we return that block
				// Note that it is difficult to prevent this from happening on the PT level, since
				// just because text is marked as hidden or deleted does not mean it is not
				// visible in a given view.
				fp_Page * pPage = getPage();
				if(pPage && pPage->getDocLayout() && pPage->getDocLayout()->getFirstSection())
				{
					pBlock = pPage->getDocLayout()->getFirstSection()->getFirstBlock();
				}

				if(!pBlock)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
				else
				{
					fp_Run * pFirstRun = pBlock->getFirstRun();

					if(pFirstRun)
					{
						fp_Line * pLine = pFirstRun->getLine();
						if(pLine)
						{
							pLine->mapXYToPosition(x - pLine->getX(),
												   y - pLine->getY() ,
												   pos, bBOL, bEOL,isTOC);
						}

					}
				}
			}
			else
			{
				fp_Run * pFirstRun = pBlock->getFirstRun();

				if(pFirstRun)
				{
					fp_Line * pVisibleLine = pFirstRun->getLine();

					if(pVisibleLine)
					{
#if 0
						// !!! This results in an endless loop (bug 7420)
						// get the container that holds this line, so we deal with wrapped
						// lines, etc.
						fp_Container * pVisibleContainer = pVisibleLine->getContainer();

						pVisibleContainer->mapXYToPosition(x - pContainer->getX(),
													  y - pContainer->getY() ,
													  pos, bBOL, bEOL,isTOC);

#else
						pVisibleLine->mapXYToPosition(x - pVisibleLine->getX(),
													  y - pVisibleLine->getY() ,
													  pos, bBOL, bEOL,isTOC);
#endif
						return;
					}

					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
				else
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
			}
		}

		pContainer->mapXYToPosition(x - pContainer->getX(),
								y - pContainer->getY() ,
									pos, bBOL, bEOL,isTOC);
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: do map to position for %x \n",pContainer));
		pContainer->mapXYToPosition(x - pContainer->getX(),
								y - pContainer->getY() ,
									pos, bBOL, bEOL,isTOC);
		xxx_UT_DEBUGMSG(("SEVIOR: Found pos %d in column \n",pos));
	}
}

/*!
 Compute the distance from point to the container's circumference
 \param x X coordinate of point
 \param y Y coordinate of point
 \return Distance between container's circumference and point
 */
UT_uint32 fp_VerticalContainer::distanceFromPoint(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 dx;
	UT_sint32 dy;

	if (x < m_iX)
	{
		dx = m_iX - x;
	}
	else if (x > (m_iX + m_iWidth - getGraphics()->tlu(1)))
	{
		dx = x - (m_iX + m_iWidth - getGraphics()->tlu(1));
	}
	else
	{
		dx = 0;
	}

	if (y < m_iY)
	{
		dy = m_iY - y;
	}
	else if (y > (m_iY + m_iHeight - getGraphics()->tlu(1)))
	{
		dy = y - (m_iY + m_iHeight - getGraphics()->tlu(1));
	}
	else
	{
		dy = 0;
	}

	if (dx == 0)
	{
		return dy;
	}

	if (dy == 0)
	{
		return dx;
	}

	UT_uint32 dist = (UT_uint32) (sqrt((float)(dx * dx) + (dy * dy)));

//	UT_ASSERT(dist > 0);

	return dist;
}

/*!
 Set X position of container
 \param iX New X position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
 */
void fp_VerticalContainer::setX(UT_sint32 iX, bool /*bDontClearIfNeeded*/)
{
	if (iX == m_iX)
	{
		return;
	}

	clearScreen();

	m_iX = iX;
}

/*!
 Set Y position of container
 \param iY New Y position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
 */
void fp_VerticalContainer::setY(UT_sint32 iY)
{
	if (iY == m_iY)
	{
		return;
	}
	if(m_iY != -99999999)
	{
		clearScreen();
	}
	m_iY = iY;
}

/*!
 Return first line in the container
 \return The first line, or NULL if the container is empty
 */
fp_Container* fp_VerticalContainer::getFirstContainer(void) const
{
	if (countCons() > 0)
	{
		return static_cast<fp_Container*>(getNthCon(0));
	}
	else
	{
		return NULL;
	}
}

UT_sint32  fp_VerticalContainer::countWrapped(void)
{
  UT_sint32 nWrapped = 0;
  UT_sint32 i = 0;
  for(i=0; i<countCons();i++)
  {
      fp_Container * pCon = static_cast<fp_Container*>(getNthCon(i));
      if(pCon->getContainerType() == FP_CONTAINER_LINE)
      {
	  fp_Line * pLine = static_cast<fp_Line *>(pCon);
	  xxx_UT_DEBUGMSG(("Line %d MaxWidth %d \n",i,pLine->getMaxWidth()));
	  if(pLine->isWrapped())
	  {
	      nWrapped++;
	  }
	  else if(pLine->isSameYAsPrevious())
	  {
	      nWrapped++;
	  }
	  else if((pLine->getMaxWidth() > 0) && (pLine->getMaxWidth() < getWidth()))
	  {
	      nWrapped++;

	  }
      }
  }
  return nWrapped;
}


/*!
 Return last line in the container
 \return The last line, or NULL if the container is empty
 */
fp_Container* fp_VerticalContainer::getLastContainer(void) const
{
	UT_uint32 iCount = countCons();

	if (iCount > 0)
	{
		return static_cast<fp_Container*>(getNthCon(iCount - 1));
	}
	else
	{
		return NULL;
	}
}


/*!
  Bump Containers from this Container to the next
  \param pLastContainerToKeep Last line to keep in this column or NULL for none
*/
void fp_VerticalContainer::bumpContainers(fp_ContainerObject* pLastContainerToKeep)
{
	UT_sint32 ndx = (NULL == pLastContainerToKeep) ? 0 : (findCon(pLastContainerToKeep)+1);
	xxx_UT_DEBUGMSG(("!!!---Bump Containers LastToKeep %x Index %d \n",pLastContainerToKeep,ndx));
	UT_ASSERT(ndx >= 0);
	UT_sint32 i;
	fp_TOCContainer *pTOC2 = NULL;
	fp_VerticalContainer* pNextContainer = static_cast<fp_VerticalContainer*>(getNext());
	UT_return_if_fail(pNextContainer);
	UT_return_if_fail((pNextContainer->getContainerType() == FP_CONTAINER_ENDNOTE) || (pNextContainer->getDocSectionLayout() == getDocSectionLayout()));
	if (pNextContainer->isEmpty())
	{
		for (i=ndx; i< countCons(); i++)
		{
			if(i >= countCons())
			         continue;
			fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));
			if(pContainer == NULL)
			        continue;
			pContainer->clearScreen();
//
// Experimental code: FIXME: Might remove after a while - check
// that large tables broken over many pages work fine.
//
#if 1
			if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pContainer);
				if(!pTab->isThisBroken())
				{
					pTab->deleteBrokenTables(true);
				}
			}
			if(pContainer->getContainerType() == FP_CONTAINER_TOC)
			{
				fp_TOCContainer *pTOC = static_cast<fp_TOCContainer *>(pContainer);
				xxx_UT_DEBUGMSG(("Found TOC %x index %d prev %x to bump to Empty Col \n",pTOC,i,pTOC->getPrevContainerInSection()));
				if(!pTOC->isThisBroken())
				{
					pTOC->deleteBrokenTOCs(true);
				}
			}
#endif
			pNextContainer->addContainer(pContainer);
		}
	}
	else
	{
		bool bTOC = false;
		for (i=countCons() - 1; i >= ndx; i--)
		{
			bTOC = false;
			if(i >= countCons())
			         continue;
			fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));
			if(pContainer == NULL)
			        continue;
			xxx_UT_DEBUGMSG(("clearScreen on %x in bumpContainers \n",pContainer));
			pContainer->clearScreen();
//
// Experimental code: FIXME: Might remove after a while - check
// that large tables broken over many pages work fine.
//
#if 1
			if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer *pTab = static_cast<fp_TableContainer *>(pContainer);
				if(!pTab->isThisBroken())
				{
					pTab->deleteBrokenTables(true);
				}
			}
			if(pContainer->getContainerType() == FP_CONTAINER_TOC)
			{
				pTOC2 = static_cast<fp_TOCContainer *>(pContainer);
				xxx_UT_DEBUGMSG(("Found TOC %x index %d prev %x to bump to filled Col \n",pTOC2,i,pTOC2->getPrevContainerInSection()));
				if(!pTOC2->isThisBroken())
				{
					pTOC2->deleteBrokenTOCs(true);
				}
				bTOC = true;
			}
#endif
			fp_Line * pLine = NULL;
			UT_sint32 iOldMaxWidth = 0;
			if(pContainer->getContainerType() == FP_CONTAINER_LINE)
			{
			    pLine =  static_cast<fp_Line *>(pContainer);
			    iOldMaxWidth = pLine->getMaxWidth();
			}
			pNextContainer->insertContainer(pContainer);
			//
			// Max Line widths can change after a bump. If so
			// we must reformat from the start of the line
			//
			if(pLine && (pLine->getMaxWidth() != iOldMaxWidth))
			{
			    UT_DEBUGMSG(("MaxWidthChanged from container bump \n"));
			    pLine->setReformat();
			}
			if(bTOC)
			{
				//UT_sint32 iTOC = pNextContainer->findCon(pContainer);
				xxx_UT_DEBUGMSG(("TOC insert at location %d in next Container \n",iTOC));
			}
		}
	}
	if(pTOC2)
	{
		//UT_sint32 iTOC = pNextContainer->findCon(pTOC);
		xxx_UT_DEBUGMSG(("TOC Final location %d in next Container \n",iTOC));
	}
	for (i=countCons() - 1; i >= ndx; i--)
	{
		deleteNthCon(i);
	}
}


/*!
  Create column
  \param pSectionLayout Section layout type used for this container

  The section the column is created in specifies the number of column
  rows. There is always created columns for all rows at the same
  time. The first (left-most) column is the leader.

  \fixme I suspect BIDI does not work with multiple columns since the
         leader would then have to be the right-most column.
*/
fp_Column::fp_Column(fl_SectionLayout* pSectionLayout) : fp_VerticalContainer(FP_CONTAINER_COLUMN, pSectionLayout),
  m_pLeader(NULL),
  m_pFollower(NULL),
  m_pPage(NULL)
{
}

fp_Column::~fp_Column()
{
	xxx_UT_DEBUGMSG(("Deleting Column %x Number containers left %d \n",this,countCons()));
//	UT_ASSERT(countCons() == 0);
}

/*!
 * This method should be called before a docsection collapse since we can't
 * be sure that the docsection that owns this column also contains the endnote
 * in this column
 */
void fp_Column::collapseEndnotes(void)
{
	UT_sint32 i = 0;
	for(i=countCons()-1; i>= 0; i--)
	{
		fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pCon->getSectionLayout());
			pEL->collapse();
			UT_sint32 ndx = findCon(pCon);
			if(ndx >= 0)
			{
				justRemoveNthCon(ndx);
			}
		}
	}
}


/*!
 * Returns true of the column contains a page break.
 */
bool fp_Column::containsPageBreak(void) const
{
    fp_Container * pCon = getLastContainer();
    if(pCon && (pCon->getContainerType() ==FP_CONTAINER_LINE))
    {
	fp_Line * pLine = static_cast<fp_Line *>(pCon);
	return pLine->containsForcedPageBreak();
    }
    return false;
}

void fp_Column::setPage(fp_Page * pPage)
{
	if(pPage == NULL)
	{
		getFillType().setParent(NULL);
	}
	else
	{
		getFillType().setParent(&pPage->getFillType());
	}
	m_pPage = pPage;
}

/* Return the index of the column on its page*/
UT_sint32 fp_Column::getColumnIndex(void)
{
    fp_Page * pPage = getPage();
    fl_DocSectionLayout * pSection = getDocSectionLayout();
    fp_Column * pCol = NULL;
    if (!pPage || !pSection)
    {return 0;}
    UT_sint32 kmax = static_cast<UT_sint32>(pSection->getNumColumns());
    UT_sint32 j;
    for(j=0;j<pPage->countColumnLeaders();j++)
    {
	pCol = pPage->getNthColumnLeader(j); 
	if (pCol && (pCol->getDocSectionLayout() == pSection))
	{
	    UT_sint32 k = 0;
	    while(pCol && k<kmax)
	    {
		if (pCol == this)
		{return k;}
		pCol = static_cast<fp_Column *> (pCol->getNext());
		k++;
	    }
	}
    }
    return 0;
}
/*!
 Draw column outline
 \param pDA Draw arguments

 This differs from the container function in that it will use draw the
 outline based on the tallest column in the row.
*/
void fp_Column::_drawBoundaries(dg_DrawArgs* pDA)
{
	if(!pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
    {
        getGraphics()->setColor(getPage()->getDocLayout()->getView()->getColorShowPara());
        UT_sint32 xoffBegin = pDA->xoff - getGraphics()->tlu(1);
        UT_sint32 yoffBegin = pDA->yoff - getGraphics()->tlu(1);
        UT_sint32 xoffEnd = pDA->xoff + getWidth() + getGraphics()->tlu(2);

        UT_sint32 iHeight = 0;
		fp_Column* pCol = getLeader();
		if (getPage()->getNthColumnLeader(getPage()->countColumnLeaders()-1) == pCol)
		{
			// If there's no column rows after this one on the page, use max height
			iHeight = getMaxHeight();
		}
		else
		{
			// Find max column height in row
			while (pCol)
			{
				if (pCol->getHeight() > iHeight)
					iHeight = pCol->getHeight();
				pCol = pCol->getFollower();
			}
		}
		UT_sint32 yoffEnd = pDA->yoff + iHeight + getGraphics()->tlu(2);

		GR_Painter painter(getGraphics());

		getGraphics()->setLineProperties(getGraphics()->tlu(1),
											GR_Graphics::JOIN_MITER,
											GR_Graphics::CAP_PROJECTING,
											GR_Graphics::LINE_SOLID);

       	painter.drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
		painter.drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        painter.drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        painter.drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
  Layout lines in the column

  This function iterates over the lines in the column and computes
  their screen position from their accumulative heights in layout
  units.

  Since this code accumulates fractions of the conversion process, the
  difference between Y positions of two lines may differ from the
  pre-computed height of the upper line. This is due to simple
  rounding errors and general lack of precision (screen coordinates
  are integer while the computation yields fractions).

  To make XY/position conversion precise, remove the gaps by updating
  the line heights. Note that the line heights get updated next time
  there's a line lookup - so this does not in any way affect layout,
  only the precision of the XY/position conversion code.

  Sevior: I put in the 0.5 to deal with truncation errors and the +1 to deal
  with the last line.

  \see fp_Line::setAssignedScreenHeight, fp_Container::recalcHeight
*/
void fp_Column::layout(void)
{
	clearWrappedLines();
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY2 = 0;
	UT_sint32 iContainerMarginAfter = 0;
	UT_GenericVector<fl_BlockLayout *> vecBlocks;
	fp_Line * pLastLine = NULL;
	fp_Container *pContainer = NULL;
	fp_Container *pPrevContainer = NULL;
	UT_sint32 i  = 0;
	//
	// RedrawHeight makes sure we redraw from whereever a line
	// changes position.
	//
	m_iRedrawHeight = -1;
	for (i=0; i < countCons(); i++)
	{
		pContainer = static_cast<fp_Container*>(getNthCon(i));

		// ignore footnotes
		if (pContainer->getContainerType() == FP_CONTAINER_FOOTNOTE)
			continue;
		// ignore annotations
		if (pContainer->getContainerType() == FP_CONTAINER_ANNOTATION)
			continue;

		xxx_UT_DEBUGMSG(("Column Layout: Container %d Container %x Type %d \n",i,pContainer,pContainer->getContainerType()));
//
// Set the location first so the height of a table can be calculated
// and adjusted.
//
		if(pContainer->getContainerType() == FP_CONTAINER_LINE)
		{
//
// Handle case of lines broken around a positioned object with text wrap on
//
			fp_Line * pLine = static_cast<fp_Line *>(pContainer);
			xxx_UT_DEBUGMSG(("Line %x X %d Y %d MaxWidth %d Width %d Height %d \n",pLine,pLine->getX(),pLine->getY(),pLine->getMaxWidth(),pLine->getWidth(),pLine->getHeight()));
			if(pLine->isWrapped())
			{
				addWrappedLine(pLine);
			}
#if 0
			else if((pLine->getMaxWidth() > 0) && (pLine->getMaxWidth() < getWidth()))
			{
				addWrappedLine(pLine);
			}
#endif
			if(pLine->isSameYAsPrevious() && pLine->getPrev())
			{
				UT_sint32 iPrevY = static_cast<fp_Line *>(pLine->getPrev())->getY();
				if(pLine->getY() != iPrevY)
				{
					pLine->clearScreen();
					pLine->setY(iPrevY);
				}
				pPrevContainer = pLine;
				xxx_UT_DEBUGMSG(("Layout: %x SameY container %d getY %d getX %d width %d \n",pLine,i,pLine->getY(),pLine->getX(),pLine->getMaxWidth()));
				continue;
			}
		}
		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
			if((m_iRedrawHeight == -1) && (pContainer->getY() > 0))
			  m_iRedrawHeight = pContainer->getY();

		}
		xxx_UT_DEBUGMSG(("Layout: container %d Height %d setY %d \n",i,iY));
//
// fxime comeback and re-evaluate this
//		UT_ASSERT(iY>=0);
		pContainer->setY(iY);
//		UT_ASSERT(iY == pContainer->getY());
		//UT_ASSERT(pContainer->getY() == iY);
//
// This is to speedup redraws.
//
		fp_TableContainer * pTab = NULL;
		fp_TOCContainer * pTOC = NULL;
		UT_sint32 iHeight = pContainer->getHeight();
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = static_cast<fp_TableContainer *>(pContainer);
			iHeight = pTab->getHeight();
		}
		if(pContainer->getContainerType() == FP_CONTAINER_TOC)
		{
			pTOC = static_cast<fp_TOCContainer *>(pContainer);
			iHeight = pTOC->getHeight();
			// This is incorrect; if the TOC has been delete in revisions mode, is will
			// have 0 height
			// UT_ASSERT(iHeight > 0);
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			pLastLine = static_cast<fp_Line *>(pContainer);
			iHeight = pLastLine->getHeight();
			UT_sint32 count = vecBlocks.getItemCount();
			if(count == 0)
			{
				vecBlocks.addItem(pLastLine->getBlock());
			}
			else
			{
				if(vecBlocks.getNthItem(count-1) != pLastLine->getBlock())
				{
					vecBlocks.addItem(pLastLine->getBlock());
				}
			}
		}
		if(iHeight > _getMaxContainerHeight())
		{
			_setMaxContainerHeight(iHeight);
		}
		UT_sint32 iContainerHeight = iHeight;
		if(pTab)
		{
			iContainerHeight = pTab->getHeight();
		}
		if(pTOC)
		{
			iContainerHeight = pTOC->getHeight();
		}
		iContainerMarginAfter = pContainer->getMarginAfter();
		xxx_UT_DEBUGMSG(("Layout: container %d Height %d Margin %d setY %d \n",i,iContainerHeight,iContainerMarginAfter,iY));
		//	UT_ASSERT(iContainerHeight > 0);
		// Update height of previous line now we know the gap between
		// it and the current line.
		if (pPrevContainer)
		{
			if(pPrevContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pLine = static_cast<fp_Line *>(pPrevContainer);
				while(pLine && pLine->isSameYAsPrevious())
				{
					pLine->setAssignedScreenHeight(iY - iPrevY2);
					pLine = static_cast<fp_Line *>(pLine->getPrev());
				}
				if(pLine)
				{
					pLine->setAssignedScreenHeight(iY - iPrevY2);
				}
			}
			else
			{
				xxx_UT_DEBUGMSG(("layout: Assigned screen height %x %d \n",pPrevContainer,iY-iPrevY2));
				pPrevContainer->setAssignedScreenHeight(iY - iPrevY2);
			}
		}
		iPrevY2 = iY;
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
#if DEBUG
		if (iY - iContainerMarginAfter > getMaxHeight())
		{
			UT_DEBUGMSG(("Problem; MaxColHeight: %d present height: %d\n",
						 getMaxHeight(), iY - iContainerMarginAfter));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
#endif
		pPrevContainer = pContainer;
	}
	// Correct height position of the last line
	if (pPrevContainer)
	{
		UT_ASSERT((iY - iPrevY2 + getGraphics()->tlu(1)) > 0);
		iY -= iContainerMarginAfter;
		if(pPrevContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			fp_Line * pLine = static_cast<fp_Line *>(pPrevContainer);
			while(pLine && pLine->isSameYAsPrevious())
			{
				pLine->setAssignedScreenHeight(iY - iPrevY2);
				pLine = static_cast<fp_Line *>(pLine->getPrev());
			}
			if(pLine)
			{
				pLine->setAssignedScreenHeight(iY - iPrevY2);
			}
		}
	}
//
// Set the frames on the page
//
	UT_sint32 count = vecBlocks.getItemCount();
	for(i=0; i < count; i++)
	{
		fl_BlockLayout * pBlock = vecBlocks.getNthItem(i);
		if(i < count -1)
		{
			pBlock->setFramesOnPage(NULL);
		}
		else
		{
			pBlock->setFramesOnPage(pLastLine);
		}
	}
//	validate();

	if (getHeight() == iY)
	{
		return;
	}

	setHeight(iY);
	getPage()->columnHeightChanged(this);
	fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
	pDSL = pDSL->getNextDocSection();
	while(pDSL)
	{
		pDSL->setNeedsSectionBreak(true,NULL);
		pDSL = pDSL->getNextDocSection();
	}
}

UT_sint32 fp_Column::getMaxHeight(void) const
{
	const fp_VerticalContainer * pVC = static_cast<const fp_VerticalContainer *>(this);
	UT_sint32 iMaxHeight = 0;
	if(!getPage())
	{
		iMaxHeight = pVC->getMaxHeight();
	}
	else
	{
	        iMaxHeight = getPage()->getAvailableHeightForColumn(this);
	}
	//UT_ASSERT(iMaxHeight > 0);
	return iMaxHeight;
}

fl_DocSectionLayout* fp_Column::getDocSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_DOC ||
			  getSectionLayout()->getType() == FL_SECTION_HDRFTR ||
			  getSectionLayout()->getType() == FL_SECTION_ENDNOTE);

	return static_cast<fl_DocSectionLayout*>(getSectionLayout());
}

/*!
 * This container is actually to display HdrFtrShadows which are repeated
 * for every page in the document. If the text is too high it is clipped to
 * to fit in the container. It's up to the user to adjust the height of the
 * header/footer region to fit the text.
 */
fp_ShadowContainer::fp_ShadowContainer(UT_sint32 iX,
									   UT_sint32 iY,
									   UT_sint32 iWidth,
									   UT_sint32 iHeight,
									   fl_SectionLayout* pSectionLayout)
	: fp_VerticalContainer(FP_CONTAINER_COLUMN_SHADOW, pSectionLayout)
{
	_setX(iX);
	_setY(iY);
	setWidth(iWidth);
	setHeight(iHeight);
	setMaxHeight(iHeight);
   m_bHdrFtrBoxDrawn = false;
}

fp_ShadowContainer::~fp_ShadowContainer()
{
  xxx_UT_DEBUGMSG(("Delete Shadow Container %x from shadow Layout %x \n",this,getSectionLayout()));
  getSectionLayout()->setFirstContainer(NULL);
}


void fp_ShadowContainer::layout(void)
{
	layout(false);
}

void fp_ShadowContainer::layout(bool bForceLayout)
{
	UT_sint32 iY = 5;
	UT_uint32 iCountContainers = countCons();
	FV_View * pView = getPage()->getDocLayout()->getView();
	bool doLayout = true;
	if(pView)
	{
	    doLayout =	pView->getViewMode() == VIEW_PRINT;
	}
	if(bForceLayout)
	{
		doLayout = true;
	}
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));
		fp_TableContainer * pTab = NULL;
		fp_TOCContainer * pTOC = NULL;
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = static_cast<fp_TableContainer *>(pContainer);
			xxx_UT_DEBUGMSG(("Found Table in shadow!!! height = %d \n",pTab->getHeight()));
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_TOC)
		{
			pTOC = static_cast<fp_TOCContainer *>(pContainer);
			UT_DEBUGMSG(("Found TOC in shadow!!!\n"));
		}
//
// FIXME: Implement this one day. Tables in header/footers.
//		if((pTab!= NULL) && !pTab->isThisBroken())
//		{
//			fp_Container * pBroke = static_cast<fp_Container *>(pTab->VBreakAt(0));
//		}
		UT_sint32 iContainerHeight = pContainer->getHeight();
		if(pTab != NULL)
		{
			iContainerHeight = pTab->getHeight();
		}
		if(pTOC != NULL)
		{
			iContainerHeight = pTOC->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
		UT_sint32 sum = iContainerHeight + iContainerMarginAfter;
		if(((iY + sum) <= (getMaxHeight())) && doLayout)
		{
			pContainer->setY(iY);
		}
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
	}

	UT_sint32 iNewHeight = iY;
	if (getHeight() == iNewHeight)
	{
		return;
	}

        if(iY <= getMaxHeight())
	{
		setHeight(iNewHeight);
	}
	else
	{
		fl_HdrFtrSectionLayout * pHFSL = getHdrFtrSectionLayout();
		fl_DocSectionLayout * pDSL = pHFSL->getDocSectionLayout();
		bool bHdrFtr = (pHFSL->getHFType() <= FL_HDRFTR_HEADER_LAST);
		if(iNewHeight > getPage()->getHeight()/3)
		{
			iNewHeight = getPage()->getHeight()/3;
		}
		pDSL->setHdrFtrHeightChange(bHdrFtr,iNewHeight+getGraphics()->tlu(3));
		setHeight(getMaxHeight());
	}
}

/*!
 *    get the shadow associated with this hdrftrContainer
 */

fl_HdrFtrShadow * fp_ShadowContainer::getShadow(void)
{
    fl_HdrFtrSectionLayout* pHdrFtrSL = getHdrFtrSectionLayout();
	return  pHdrFtrSL->findShadow( getPage() );
}

/*!
 *  Set the page for this shadow. Also set the fg_FillType parent.
 */

void fp_ShadowContainer::setPage(fp_Page *pPage)
{
	m_pPage = pPage;
	if(pPage)
	{
		getFillType().setParent(&pPage->getFillType());
	}
}

fl_HdrFtrSectionLayout* fp_ShadowContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_HDRFTR);

	return static_cast<fl_HdrFtrSectionLayout*>(getSectionLayout());
}


/*!
  Clear container content from screen.
*/
void fp_ShadowContainer::clearScreen(void)
{
	FV_View * pView = getPage()->getDocLayout()->getView();
	if(pView->getViewMode() !=  VIEW_PRINT)
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to clear Header/Footer in Normal Mode \n"));
		return;
	}
	int count = countCons();
	for (int i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));

		pContainer->clearScreen();
	}
	clearHdrFtrBoundaries();
}



/*!
 Draw container content
 \param pDA Draw arguments
 */

void fp_ShadowContainer::draw(dg_DrawArgs* pDA)
{
	FV_View * pView = getPage()->getDocLayout()->getView();
	if((pView->getViewMode() !=  VIEW_PRINT) && pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN) )
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to draw Header/Footer in Normal Mode \n"));
		return;
	}
	if((pView->getViewMode() !=  VIEW_PRINT) && pDA->pG->queryProperties(GR_Graphics::DGP_PAPER) )
	{
		layout(true);
	}

	UT_sint32 count = countCons();
	UT_sint32 iY= 0;
	for (UT_sint32 i = 0; i<count; i++)
	{
		fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));

		dg_DrawArgs da = *pDA;
		da.xoff += pContainer->getX();
		da.yoff += pContainer->getY();

		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
//
// Clip to keep inside header/footer container
//
		if(iY > getMaxHeight())
			break;
		pContainer->draw(&da);
	}
    if(pView && pView->isHdrFtrEdit() && pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN) && pView->getEditShadow() == getShadow())
	{
		_drawHdrFtrBoundaries(pDA);
	}
	else
	{
        clearHdrFtrBoundaries();
		_drawBoundaries(pDA);
	}
	if((pView->getViewMode() !=  VIEW_PRINT) && pDA->pG->queryProperties(GR_Graphics::DGP_PAPER) )
	{
		layout(false);
	}
}

/*!
 * This method draws a solid box around the currently editted Header/Footer
 */
void fp_ShadowContainer::_drawHdrFtrBoundaries(dg_DrawArgs * pDA)
{
	if(!pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}
	FV_View * pView = getPage()->getDocLayout()->getView();
	if(pView->getViewMode() !=  VIEW_PRINT)
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to draw Header/Footer in Normal Mode \n"));
		return;
	}
//
// Can put this in to speed things up.
//
//	if(m_bHdrFtrBoxDrawn)
//		return;
	UT_RGBColor clrDrawHdrFtr(127,127,127);
	getGraphics()->setLineWidth(getGraphics()->tlu(1));
	getGraphics()->setColor(clrDrawHdrFtr);
//
// These magic numbers stop clearscreens from blanking the lines
//
	m_ixoffBegin = pDA->xoff-2;
	m_iyoffBegin = pDA->yoff+2;
	m_ixoffEnd = pDA->xoff + getWidth() + getGraphics()->tlu(1);
	m_iyoffEnd = pDA->yoff + getMaxHeight() - getGraphics()->tlu(1);

	GR_Painter painter(getGraphics());

	painter.drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	painter.drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	painter.drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	painter.drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->setLineWidth(getGraphics()->tlu(1));
    m_bHdrFtrBoxDrawn = true;
}


/*!
 * This method clears the solid box around the curently editted Header/Footer
 */
void fp_ShadowContainer::clearHdrFtrBoundaries(void)
{
	if(!m_bHdrFtrBoxDrawn)
		return;
	const UT_RGBColor * pClr = getPage()->getFillType().getColor();
	getGraphics()->setLineWidth(getGraphics()->tlu(1));
	getGraphics()->setColor(*pClr);
//
// Paint over the previous lines with the page color
//

	GR_Painter painter(getGraphics());

	painter.drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	painter.drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	painter.drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	painter.drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->setLineWidth(getGraphics()->tlu(1));
	m_bHdrFtrBoxDrawn = false;
}

/*!
 * Ok this container class is for the hdrftrSectionLayout. It never gets drawn
 * on the screen, only the shadows get drawn. The page pointer contains a NULL.
 * This makes it possible to format the hdrftrSectionLayout and to do
 * editting operations on header/footers like regular text.
\param iwidth width of the page in pixels?? I think.
\param IwidthLayout width of the screen in layout units
\param fl_SectionLayout * pSectionLayout pointer to the
       fl_HdrFtrSectionLayout that owns this container.
*/

fp_HdrFtrContainer::fp_HdrFtrContainer(UT_sint32 iWidth,
									   fl_SectionLayout* pSectionLayout)
	: fp_VerticalContainer(FP_CONTAINER_HDRFTR, pSectionLayout)
{
	_setX(0);
	_setY(0);
	setWidth(iWidth);
	setHeight(0);
}

fp_HdrFtrContainer::~fp_HdrFtrContainer()
{
}

/*!
 * Overloaded layout for VirtualCOntainer. We don't care about the height or
 * of the container.
 */

void fp_HdrFtrContainer::layout(void)
{
	UT_sint32 iY = 0;

	UT_uint32 iCountContainers = countCons();

	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		fp_Container* pContainer = static_cast<fp_Container*>(getNthCon(i));
		fp_TableContainer * pTab = NULL;
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = static_cast<fp_TableContainer *>(pContainer);
		}

		UT_sint32 iContainerHeight = pContainer->getHeight();
		if(pTab)
		{
			iContainerHeight = pTab->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		pContainer->setY(iY);
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
	}

	UT_sint32 iNewHeight = iY;

	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
}

/*!
 * Returns a pointer to the HdrFtrSectionLayout that owns this container
 */
fl_HdrFtrSectionLayout* fp_HdrFtrContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_HDRFTR);

	return static_cast<fl_HdrFtrSectionLayout*>(getSectionLayout());
}


/*!
  Get line's offsets relative to the screen for this method we just return
  * -100000 since virtual containers are never drawn.
 \param  pContainer Container
 \retval xoff Container's X offset relative the screen actually -10000
 \retval yoff Container's Y offset relative the screen actually -10000
 */
void fp_HdrFtrContainer::getScreenOffsets(fp_ContainerObject* /*pContainer*/,
									UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = -100000;
	yoff = -100000;
}


/*!
  NOP's for clear screen.
*/
void fp_HdrFtrContainer::clearScreen(void)
{

}

/*!
 NOP for Draw's
 \param pDA Draw arguments
 */
void fp_HdrFtrContainer::draw(dg_DrawArgs* /*pDA*/)
{

}
