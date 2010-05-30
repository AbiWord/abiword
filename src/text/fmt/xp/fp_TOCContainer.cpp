/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fp_TOCContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_TOCLayout.h"
#include "fv_View.h"
#include "gr_Painter.h"

/*!
  Create Table Of Contents container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TOCContainer::fp_TOCContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_TOC, pSectionLayout),
	  m_pFirstBrokenTOC(NULL),
	  m_pLastBrokenTOC(NULL),
	  m_bIsBroken(false),
	  m_pMasterTOC(NULL),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_iBrokenTop(0),
	  m_iBrokenBottom(0),
	  m_iLastWantedVBreak(0)
{
}

fp_TOCContainer::fp_TOCContainer(fl_SectionLayout* pSectionLayout, fp_TOCContainer * pMaster) 
	: fp_VerticalContainer(FP_CONTAINER_TOC, pSectionLayout),
	  m_pFirstBrokenTOC(NULL),
	  m_pLastBrokenTOC(NULL),
	  m_bIsBroken(true),
	  m_pMasterTOC(pMaster),
	  m_iYBreakHere(0),
	  m_iYBottom(0),
	  m_iBrokenTop(0),
	  m_iBrokenBottom(0),
	  m_iLastWantedVBreak(0)
{
	setY(0);
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_TOCContainer::~fp_TOCContainer()
{
	clearCons();
	deleteBrokenTOCs(false);
	UT_DEBUGMSG(("SEVIOR: deleting TOC %p \n",this));
//
// For debugging...
//
	setContainer(NULL);
	setPrev(NULL);
	setNext(NULL);
	m_pMasterTOC = NULL;

}

/*!
  Find document position from X and Y coordinates.  Note that the TOC
  only has one document position, so that mapXYToPosition is rather
  unhelpful for scrolling purposes.
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
 */
void fp_TOCContainer::mapXYToPosition(UT_sint32 x, UT_sint32 y, 
									  PT_DocPosition& pos,
									  bool& bBOL, bool& bEOL, bool &isTOC)
{
	isTOC = true;
	fp_VerticalContainer::mapXYToPosition(x, y, pos, bBOL, bEOL, isTOC);
}

/*! 
 * This method returns the value of the TOC reference (or anchor)
 */
UT_sint32 fp_TOCContainer::getValue(void)
{
	fl_TOCLayout * pTL = static_cast<fl_TOCLayout *>(getSectionLayout());
	return pTL->getTOCPID();
}

void fp_TOCContainer::clearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	if(isThisBroken() && getContainer())
	{
		xxx_UT_DEBUGMSG(("Doing Clear Screen on Broken TOC %x \n",this));
		UT_sint32 iHeight = getHeight();
		UT_sint32 iWidth = getContainer()->getWidth();
		UT_sint32 srcX  = getX();
		UT_sint32 srcY = getY();
		if(getFirstBrokenTOC() == this)
		{
			srcY = getMasterTOC()->getY();
		}
		fp_Column * pCol = static_cast<fp_Column *>(getColumn());
		UT_sint32 x,y;
		getPage()->getScreenOffsets(pCol,x,y);
		x += srcX;
		y += srcY;
		getFillType()->setWidthHeight(getGraphics(),iWidth,iHeight);
		getFillType()->Fill(getGraphics(),srcX,srcY,x,y,iWidth,iHeight);
		xxx_UT_DEBUGMSG(("x %d y %d width %d height %d \n",x,y,iWidth,iHeight));
		return;
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< countCons(); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		pCon->clearScreen();
	}
}


void fp_TOCContainer::forceClearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< countCons(); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->setScreenCleared(false);
		}
		pCon->clearScreen();
	}
}

fl_DocSectionLayout * fp_TOCContainer::getDocSectionLayout(void)
{
	fl_TOCLayout * pTL = static_cast<fl_TOCLayout *>(getSectionLayout());
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pTL->myContainingLayout());
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}


/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_TOCContainer::draw(GR_Graphics * /*pG*/)
{
}
/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_TOCContainer::draw(dg_DrawArgs* pDA)
{
	if(getPage() == NULL)
	{
		return;
	}
	if(!isThisBroken() && getFirstBrokenTOC())
	{
		getFirstBrokenTOC()->draw(pDA);
		return;
	}
	fp_TOCContainer * pMaster = this;
	if(getMasterTOC())
	{
		pMaster = getMasterTOC();
	}
	xxx_UT_DEBUGMSG(("TOC: Drawing broken TOC %x x %d, y %d width %d height %d \n",this,pDA->xoff,pDA->yoff,getWidth(),getHeight()));

//
// Only draw the lines in the clipping region.
//
	dg_DrawArgs da = *pDA;
	
	UT_uint32 count = pMaster->countCons();
	UT_sint32 iYStart = getYBreak();
	UT_sint32 iYBottom = getYBottom();
	xxx_UT_DEBUGMSG(("Drawing TOC, yBreak %d ybottom %d \n",iYStart,iYBottom));
	for (UT_uint32 i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(pMaster->getNthCon(i));
		if(pContainer->getY() < iYStart)
		{
			continue;
		}
		if(pContainer->getY() > iYBottom)
		{
			break;
		}
		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY() - iYStart;
		pContainer->draw(&da);
	}
    _drawBoundaries(pDA);
}

fp_Container * fp_TOCContainer::getNextContainerInSection() const
{
	if(getNext())
	{
		return static_cast<fp_Container *>(getNext());
	}
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pNext = pCL->getNext();
	while(pNext && pNext->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		pNext = pNext->getNext();
	}
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Column * fp_TOCContainer::getBrokenColumn(void)
{
	if(!isThisBroken())
	{
		return static_cast<fp_Column *>(fp_VerticalContainer::getColumn());
	}
	fp_TOCContainer * pBroke = this;
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
			UT_ASSERT(0);
		}
	}
	if(pBroke && !bStop)
	{
		pCol = static_cast<fp_Column *>(pBroke->getContainer());
	}
	return pCol;
}

fp_Container * fp_TOCContainer::getPrevContainerInSection() const
{
	if(getPrev())
	{
		return static_cast<fp_Container *>(getPrev());
	}

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pPrev = pCL->getPrev();
	while(pPrev && pPrev->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		pPrev = pPrev->getPrev();
	}
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}

bool fp_TOCContainer::isVBreakable(void)
{
	return true;
}

bool fp_TOCContainer::isInBrokenTOC(fp_Container * pCon)
{
//
// OK A container is allowed in this broken TOC if it's
// Y location plus height lie between getYBreak() and getYBottom.
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
	iTop = pCon->getY();
	UT_sint32 iHeight = pCon->getHeight();

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

fp_TOCContainer * fp_TOCContainer::getLastBrokenTOC(void) const
{
	if(isThisBroken())
	{
		return getMasterTOC()->getLastBrokenTOC();
	}
	return m_pLastBrokenTOC;
}

UT_sint32 fp_TOCContainer::getBrokenNumber(void)
{
	if(!isThisBroken())
	{
		return 0;
	}
	fp_TOCContainer * pTOC = getMasterTOC()->getFirstBrokenTOC();
	UT_sint32 i = 1;
	while(pTOC && pTOC != this)
	{
		pTOC = static_cast<fp_TOCContainer *>(pTOC->getNext());
		i++;
	}
	if(!pTOC)
	{
		return -1;
	}
	return i;
}
		

void fp_TOCContainer::setFirstBrokenTOC(fp_TOCContainer * pBroke) 
{
	if(isThisBroken())
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		fp_TOCContainer * pMaster = getMasterTOC();
		pMaster->setFirstBrokenTOC(pBroke);
		fp_TOCContainer * pNext = static_cast<fp_TOCContainer *>(pMaster);
		while(pNext)
		{
			pNext->setFirstBrokenTOC( pBroke);
			pNext = static_cast<fp_TOCContainer *>(pNext->getNext());
		}
	}
	m_pFirstBrokenTOC = pBroke;

}

void fp_TOCContainer::setLastBrokenTOC(fp_TOCContainer * pBroke) 
{
	if(isThisBroken())
	{
		fp_TOCContainer * pMaster = getMasterTOC();
		pMaster->setLastBrokenTOC(pBroke);
		fp_TOCContainer * pNext = static_cast<fp_TOCContainer *>(pMaster);
		while(pNext)
		{
			pNext->setLastBrokenTOC( pBroke);
			pNext = static_cast<fp_TOCContainer *>(pNext->getNext());
		}
	}
	m_pLastBrokenTOC = pBroke;
}
	
/*!
 * This method creates a new broken toccontainer, broken at the
 * offset given. 
 * If the new TOCcontainer is broken from a pre-existing 
 * broken TOC it is inserted into the holding vertical container after
 * the old broken TOC.
 * It also inserted into the linked list of containers in the vertical
 * container.
 * vpos is relative to the either the start of the TOC if it's the first
 * non-zero vpos or relative to the previous ybreak if it's further down.
 */
fp_ContainerObject * fp_TOCContainer::VBreakAt(UT_sint32 vpos)
{
//
// Do the case of creating the first broken TOC from the master TOC.
// 
	fp_TOCContainer * pBroke = NULL;
	if(!isThisBroken() && getLastBrokenTOC() == NULL)
	{
		if(getFirstBrokenTOC() != NULL)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return NULL;
		}
		pBroke = new fp_TOCContainer(getSectionLayout(),this);
		UT_DEBUGMSG(("SEVIOR:!!!!!!! First broken TOC %p \n",pBroke));
		pBroke->setYBreakHere(vpos);
		pBroke->setYBottom(fp_VerticalContainer::getHeight());
		// leave this in!		UT_ASSERT(pBroke->getHeight());
		setFirstBrokenTOC(pBroke);
		setLastBrokenTOC(pBroke);
		pBroke->setContainer(getContainer());
		static_cast<fp_VerticalContainer *>(pBroke)->setHeight(pBroke->getHeight());
		static_cast<fp_VerticalContainer *>(pBroke)->setY(getY());
		return pBroke;
	}
//
// Now do the case of breaking a Master TOC.
//
	if(getMasterTOC() == NULL)
	{
		return getLastBrokenTOC()->VBreakAt(vpos);
	}
	if(getContainer() == NULL)
	{
	    return NULL;
	}
	pBroke = new fp_TOCContainer(getSectionLayout(),getMasterTOC());
	getMasterTOC()->setLastBrokenTOC(pBroke);

	xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!!  New broken TOC %x \n",getLastBrokenTOC()));

//
// vpos is relative to the container that contains this height but we need
// to add in the height above it.
//
	pBroke->setYBreakHere(getYBreak()+vpos);
	setYBottom(getYBreak() + vpos -1);
	UT_ASSERT(getHeight() >0);
	fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(getMasterTOC());
	if(pVCon == NULL)
	{

	}
	pBroke->setYBottom(pVCon->getHeight());
	xxx_UT_DEBUGMSG(("SEVIOR????????: YBreak %d YBottom  %d Height of broken TOC %d \n",pBroke->getYBreak(),pBroke->getYBottom(),pBroke->getHeight()));
	xxx_UT_DEBUGMSG(("SEVIOR????????: Previous TOC YBreak %d YBottom  %d Height of broken TOC %d \n",getYBreak(),getYBottom(),getHeight()));
	UT_ASSERT(pBroke->getHeight() > 0);
	UT_sint32 i = 0;
//
// The structure of TOC linked list is as follows.
// NULL <= Master <==> Next <==> Next => NULL
//          first 
// ie terminated by NULL's in the getNext getPrev list. The second
// broken TOC points and is pointed to by the Master TOC
// 
	pBroke->setPrev(this);
	fp_Container * pUpCon = NULL;
	if(getMasterTOC()->getFirstBrokenTOC() == this)
	{
		i = getContainer()->findCon(getMasterTOC());
		pUpCon = getMasterTOC()->getContainer();
  		pBroke->setPrev(getMasterTOC());
  		pBroke->setNext(NULL);
  		getMasterTOC()->setNext(pBroke);
		setNext(pBroke);
	}
	else
	{
  		pBroke->setNext(NULL);
  		setNext(pBroke);
		if(getYBreak() == 0 )
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			pUpCon = getMasterTOC()->getContainer();
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
			i = pUpCon->findCon(getMasterTOC());
		}
		else
		{
			i = pUpCon->findCon(this);
		}
	}
	if(i >=0 && i < pUpCon->countCons() -1)
	{
		pUpCon->insertConAt(pBroke,i+1);
	}
	else if( i == pUpCon->countCons() -1)
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
	// of the new broken TOC.
	//
// Skip this for now. Look at fp_TableContainer to see if it's needed later
//
	static_cast<fp_VerticalContainer *>(pBroke)->setHeight(pBroke->getHeight());	return pBroke;
}


/*!
 * Overload the setY method
 */
void fp_TOCContainer::setY(UT_sint32 i)
{
	bool bIsFirstBroken = false;
	UT_sint32 iOldY = getY();
	xxx_UT_DEBUGMSG(("fp_TOCContainer: setY set to %d \n",i));
	if(isThisBroken())
	{
		xxx_UT_DEBUGMSG(("setY: getMasterTOC %x FirstBrokenTOC %x this %x \n",getMasterTOC(),getMasterTOC()->getFirstBrokenTOC(),this));
		//	if(getMasterTOC()->getFirstBrokenTOC() != this)
		{
			xxx_UT_DEBUGMSG(("setY: Later broken TOC set to %d \n",i));
			fp_VerticalContainer::setY(i);
			return;
		}
		bIsFirstBroken = true;
	}
//
// Create an initial broken TOC if none exists
//
	if(!bIsFirstBroken && (getFirstBrokenTOC() == NULL))
	{
		VBreakAt(0);
	}
	iOldY = getY();
	if(i == iOldY)
	{
		return;
	}
	clearScreen();
//
// FIXME: Do I need to force another breakSection or will happen 
// automatically?
//
	xxx_UT_DEBUGMSG(("Set Reformat 1 now from TOC %x in TOCLayout %x \n",this,getSectionLayout()));
	getSectionLayout()->setNeedsReformat(getSectionLayout());
	fp_VerticalContainer::setY(i);
	adjustBrokenTOCs();
}


void fp_TOCContainer::setYBreakHere(UT_sint32 i)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Ybreak set to %d \n",i));
	m_iYBreakHere = i;
	if(i > 0)
	{
		//	UT_ASSERT(getHeight() > 0);
	}
}

void fp_TOCContainer::setYBottom(UT_sint32 i)
{
	m_iYBottom = i;
	//	UT_ASSERT(getHeight() > 0);
}

/*!
 * The caller to this method requests a break at the vertical height
 * given. It returns the actual break height, which will always be
 * less than or equal to the requested height.
 */
UT_sint32 fp_TOCContainer::wantVBreakAt(UT_sint32 vpos)
{
	if(isThisBroken())
	{
		return getMasterTOC()->wantVBreakAt(vpos);
	}
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 iYBreak = vpos;
	fp_Line * pLine;
	for(i=0; i< count; i++)
	{
		pLine = static_cast<fp_Line *>(getNthCon(i));
		if((pLine->getY() <= vpos) && (pLine->getY() + pLine->getHeight() +pLine->getMarginAfter() > vpos))
		{
			//
			// Line overlaps break point. Find break here
			//
			iYBreak = pLine->getY();
		}
	}
	return iYBreak;
}


/*! 
 * Return the height of this Table taking into account the possibility
 * of it being broken.
 */
UT_sint32 fp_TOCContainer::getHeight(void)
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
		if(getFirstBrokenTOC() != NULL)
		{
			return getFirstBrokenTOC()->getHeight();
		}
		return iFullHeight;
	}
	UT_sint32 iMyHeight = getYBottom() - getYBreak();
	return iMyHeight;
}

/*!
 * This method adjusts the m_iYBreak and m_iYBottom variables after a 
 * setY method changes the start position of the top of the table.
 */
void fp_TOCContainer::adjustBrokenTOCs(void)
{
	if(isThisBroken())
	{
		//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	if(getFirstBrokenTOC() == NULL)
	{
		return;
	}
	if(getFirstBrokenTOC() == getLastBrokenTOC())
	{
		return;
	}
	//
	// FIXME. Both this code and the code in fp_TableContainer, somehow leads to bugs. I've clearly found
	// workarounds to what this is trying to achive. In pricinple this code should make laying out TOC's
	// faster. In parctice I suspect it leads to bugs in fb_ColumnBreaker. I'll leave these returns in place
	// for now, pending removal of the methods.
	//
	return;
	fp_TOCContainer * pBroke = getFirstBrokenTOC();
	fp_VerticalContainer * pVC = static_cast<fp_VerticalContainer *>(getContainer());
	UT_sint32 iNewHeight = pVC->getMaxHeight() - getY();	
	UT_sint32 ishift = iNewHeight - pBroke->getYBottom();
	UT_sint32 iNewBot = pBroke->getYBottom() + ishift;
	UT_sint32 iTOCHeight = fp_VerticalContainer::getHeight();
	UT_DEBUGMSG(("SEVIOR: ishift = %d iNewHeight %d  pBroke->getYBottom() %d \n",ishift,iNewHeight,pBroke->getYBottom()));
	if(ishift == 0)
	{
		return;
	}
	if(iNewBot > iTOCHeight)
	{
		iNewBot = iTOCHeight;
	}
	pBroke->setYBottom(iNewBot);
	UT_ASSERT(pBroke->getHeight());

	pBroke = static_cast<fp_TOCContainer *>(pBroke->getNext());
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
			pBroke->setYBottom(iTOCHeight);
			UT_ASSERT(pBroke->getHeight());
		}
		xxx_UT_DEBUGMSG(("SEVIOR: Broken TOC %x YBreak adjusted to %d Shift is %d height is %d \n",pBroke,iNewTop+ishift,ishift,pBroke->getHeight()));
		fp_TOCContainer * pPrev = static_cast<fp_TOCContainer *>(pBroke->getPrev());
//
// If the height of the previous plus the height of pBroke offset from
// the previous position is less that the column height we can delete
// this broken TOC. 
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
				pPrev = getFirstBrokenTOC();
			}
			xxx_UT_DEBUGMSG(("SEVIOR; In adjust - Deleting TOC. Max height %d prev Y %d prev Height %d cur Height %d \n",iMaxHeight, pPrev->getY(),pPrev->getHeight(),pBroke->getHeight()));
//
// Don't need this TOC any more. Delete it and all following TOCs.
// after adjusting the previous TOC.
//
			pPrev->setYBottom(iTOCHeight);
			UT_ASSERT(pPrev->getHeight());
			pPrev->setNext( NULL);
			if(pPrev == getFirstBrokenTOC())
			{
				setNext(NULL);
				getFirstBrokenTOC()->setYBreakHere(0);
				UT_ASSERT(getFirstBrokenTOC()->getHeight());
			}
			setLastBrokenTOC(pPrev);
			xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!! 2 last broken TOC %x deleting %x Master TOC %x  \n",getLastBrokenTOC(),pBroke,this));
			xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!!! 2 get first %x get last broken TOC %x \n",getFirstBrokenTOC(),getLastBrokenTOC()));
			fp_TOCContainer * pT = getFirstBrokenTOC();
			UT_sint32 j = 0;
			while(pT)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: TOC %d is %x \n",j,pT));
				j++;
				pT = static_cast<fp_TOCContainer *>(pT->getNext());
			}
			while(pBroke)
			{
				fp_TOCContainer * pNext = static_cast<fp_TOCContainer *>(pBroke->getNext());
				UT_sint32 i = pBroke->getContainer()->findCon(pBroke);
				if(i >=0)
				{
					pBroke->getContainer()->deleteNthCon(i);
				}
				else
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
				xxx_UT_DEBUGMSG(("SEVIOR: Adjust  - Delete TOC %x \n",pBroke));
				delete pBroke;
				pBroke = pNext;
			}
		}
		else
		{
			pBroke = static_cast<fp_TOCContainer *>(pBroke->getNext());
		}
	}
}

/*!
 * This deletes all the broken TOCs from this master TOC.
 * This routine assumes that a clear screen has been set already.
 */
void fp_TOCContainer::deleteBrokenTOCs(bool bClearFirst)
{
	if(isThisBroken())
	{
		return;
	}
	if(bClearFirst)
	{
		clearScreen();
		//
		// Remove broken TOC pointers
		//
		clearBrokenContainers();
	}
	if(getFirstBrokenTOC() == NULL)
	{
		return;
	}
	fp_TOCContainer * pBroke = NULL;
	fp_TOCContainer * pNext = NULL;
	fp_TOCContainer * pLast = NULL;
	pBroke = getFirstBrokenTOC();
	bool bFirst = true;
	while(pBroke )
	{
		pNext = static_cast<fp_TOCContainer *>(pBroke->getNext());
		pLast = pBroke;
		if(!bFirst)
		{
		        fp_Container * pConBroke =  pBroke->getContainer();
			if(pConBroke)
			{
			    UT_sint32 i = pBroke->getContainer()->findCon(pBroke);
//
// First broken TOC is not in the container.
//
			    if(i >=0)
			    {
			        fp_Container * pCon = pBroke->getContainer();
				pBroke->setContainer(NULL);
				pCon->deleteNthCon(i);
			    }
			}
		}
		bFirst = false;
		xxx_UT_DEBUGMSG(("SEVIOR: Deleting broken TOC %x \n",pBroke));
		delete pBroke;
		if(pBroke == getLastBrokenTOC())
		{
			pBroke = NULL;
		}
		else
		{
			pBroke = pNext;
		}
	}
	setFirstBrokenTOC(NULL);
	setLastBrokenTOC(NULL);
	setNext(NULL);
	setPrev(NULL);
//	if(bClearFirst)
	{
		fl_TOCLayout * pTL = static_cast<fl_TOCLayout *>(getSectionLayout());
		fl_DocSectionLayout * pDSL = pTL->getDocSectionLayout();
		pDSL->deleteBrokenTablesFromHere(pTL);
	}
}

fp_TOCContainer * fp_TOCContainer::getFirstBrokenTOC(void) const
{
	if(isThisBroken())
	{
		return getMasterTOC()->getFirstBrokenTOC();
	}
	return m_pFirstBrokenTOC;
}


void fp_TOCContainer::setContainer(fp_Container * pContainer)
{
	xxx_UT_DEBUGMSG(("!!!!!-----!!!!TOC Container set to %x \n",pContainer));
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
	fp_TOCContainer * pBroke = getFirstBrokenTOC();
	if(pBroke)
	{
		pBroke->setContainer(pContainer);
	}
	if(pContainer == NULL)
	{
		xxx_UT_DEBUGMSG(("Set master TOC %x container to NULL \n",this));
		return;
	}
	setWidth(pContainer->getWidth());
}

void fp_TOCContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
	iY= 0;
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = static_cast<fp_Container*>(getNthCon(i));
//
// This is to speedup redraws.
//
		if(pContainer->getHeight() > _getMaxContainerHeight())
			_setMaxContainerHeight(pContainer->getHeight());

		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
		}
			
		pContainer->setY(iY);

		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		//iY +=  0.5;
		if (pPrevContainer)
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

	if (fp_VerticalContainer::getHeight() == iY)
	{
		return;
	}
	UT_DEBUGMSG(("Height in TOCContainer set to %d Old Height %d \n",iY,getHeight()));
	setHeight(iY);
	deleteBrokenTOCs(true);
}

