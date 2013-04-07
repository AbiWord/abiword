/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Patrick Lam <plam@mit.edu>
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fp_FrameContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_FrameLayout.h"
#include "fp_TableContainer.h"
#include "fv_View.h"
#include "gr_Painter.h"
#include "fl_BlockLayout.h"

/*!
  Create Frame container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_FrameContainer::fp_FrameContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_FRAME, pSectionLayout),
	  m_pPage(NULL),
	  m_iXpad(0),
	  m_iYpad(0),
	  m_bNeverDrawn(true),
	  m_bOverWrote(false),
	  m_bIsWrapped(false),
	  m_bIsTightWrapped(false),
	  m_bIsAbove(true),
	  m_bIsTopBot(false),
	  m_bIsLeftWrapped(false),
	  m_bIsRightWrapped(false),
	  m_iPreferedPageNo(-1),
	  m_iPreferedColumnNo(0)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_FrameContainer::~fp_FrameContainer()
{
  UT_DEBUGMSG(("Delete FrameContainer %p \n",this));
	m_pPage = NULL;
}

void fp_FrameContainer::setPage(fp_Page * pPage)
{
	if(pPage && (m_pPage != NULL) && m_pPage != pPage)
	{
		clearScreen();
		m_pPage->removeFrameContainer(this);
		getSectionLayout()->markAllRunsDirty();
		
		UT_GenericVector<fl_ContainerLayout *> AllLayouts;
		AllLayouts.clear();
		m_pPage->getAllLayouts(AllLayouts);
		UT_sint32 i = 0;
		for(i=0; i<AllLayouts.getItemCount(); i++)
		{
		      fl_ContainerLayout * pCL = AllLayouts.getNthItem(i);
		      pCL->collapse();
		      pCL->format();
	        }
		m_pPage->getOwningSection()->setNeedsSectionBreak(true,m_pPage);
		
	}
	m_pPage = pPage;
	if(pPage)
	{
		getFillType().setParent(&pPage->getFillType());
	}
	else
	{
		getFillType().setParent(NULL);
	}
}


bool fp_FrameContainer::isAbove(void)
{
  return  m_bIsAbove;
}
/*!
 * Returns true if the supplied screen rectangle overlaps with frame
 * container. This method takes account of transparening and tight wrapping.
 */
bool fp_FrameContainer::overlapsRect(UT_Rect & rec)
{
     UT_Rect * pMyFrameRec = getScreenRect();
     fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
     UT_sint32 iextra = pFL->getBoundingSpace() -2;
     pMyFrameRec->left -= iextra;
     pMyFrameRec->top -= iextra;
     pMyFrameRec->width += 2*iextra;
     pMyFrameRec->height += 2*iextra;
     xxx_UT_DEBUGMSG(("look at rec.left %d top %d width %d  \n",rec.left,rec.top,rec.width));
     if(rec.intersectsRect(pMyFrameRec))
     {
         if(!isTightWrapped())
	 {
	      delete pMyFrameRec;
	      return true;
	 }
	 UT_sint32 iTweak = getGraphics()->tlu(2);
	 pMyFrameRec->left += iextra + iTweak;
	 pMyFrameRec->top += iextra + iTweak;
	 pMyFrameRec->width -= (2*iextra + 2*iTweak);
	 pMyFrameRec->height -= (2*iextra + 2*iTweak);

	 UT_sint32 y = rec.top - pMyFrameRec->top;
	 UT_sint32 h = rec.height;
	 if(pFL->getBackgroundImage() == NULL)
	 {
	      delete pMyFrameRec;
	      return true;
	 }
	 UT_sint32 pad = pFL->getBoundingSpace();
	 UT_sint32 iLeft = pFL->getBackgroundImage()->GetOffsetFromLeft(getGraphics(),pad,y,h);
	 xxx_UT_DEBUGMSG(("iLeft projection %d \n",iLeft));
	 if(iLeft < -getWidth())
	 {
	   //
	   // Pure transparent.
	   //
	   xxx_UT_DEBUGMSG(("Overlaps pure transparent line top %d line height %d image top %d \n",rec.top,rec.height,y));
	      delete pMyFrameRec;
	      return false;
	 }
	 xxx_UT_DEBUGMSG(("iLeft in overlapRect %d Y %d \n",iLeft,y));
	 if(rec.left < pMyFrameRec->left)
	 {
              pMyFrameRec->left -= iLeft;
	      xxx_UT_DEBUGMSG(("Moves Image left border by %d to %d \n",-iLeft,pMyFrameRec->left));
	 }
	 else
	 {
	      UT_sint32 iRight = pFL->getBackgroundImage()->GetOffsetFromRight(getGraphics(),pad,y,h);
              pMyFrameRec->width += iRight;
	      xxx_UT_DEBUGMSG(("Reduce Image width by %d to %d \n",iRight,pMyFrameRec->width));
	 }
	 if(rec.intersectsRect(pMyFrameRec))
	 {
	   xxx_UT_DEBUGMSG(("Frame Still overlaps \n"));
	   delete pMyFrameRec;
	   return true;
	 }
	 xxx_UT_DEBUGMSG(("Tight Frame no longer overlaps \n"));
	 xxx_UT_DEBUGMSG(("Line Top %d Height %d left %d width %d \n",rec.top,rec.height,rec.left,rec.width));
	 xxx_UT_DEBUGMSG(("Image Top %d Height %d left %d width %d \n",pMyFrameRec->top,pMyFrameRec->height,pMyFrameRec->left,pMyFrameRec->width));
	 xxx_UT_DEBUGMSG(("Relative Top of line %d \n",y));
     }
     delete pMyFrameRec;
     return false;
}

void fp_FrameContainer::setPreferedPageNo(UT_sint32 i)
{
     if(m_iPreferedPageNo == i)
       return;
     m_iPreferedPageNo =  i;
     fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
     FL_DocLayout * pDL = pFL->getDocLayout();
     if(pDL->isLayoutFilling())
       return;
     PD_Document * pDoc = pDL->getDocument();
     UT_UTF8String sVal;
     UT_UTF8String_sprintf(sVal,"%d",i);
     const char * attr = PT_PROPS_ATTRIBUTE_NAME;
     UT_UTF8String sAttVal = "frame-pref-page:";
     sAttVal += sVal.utf8_str();
     
     pDoc->changeStruxAttsNoUpdate(pFL->getStruxDocHandle(),attr,sAttVal.utf8_str());
}

void fp_FrameContainer::setPreferedColumnNo(UT_sint32 i)
{
     if(m_iPreferedColumnNo == i)
       return;
     m_iPreferedColumnNo =  i;
     fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
     FL_DocLayout * pDL = pFL->getDocLayout();
     if(pDL->isLayoutFilling())
       return;
     PD_Document * pDoc = pDL->getDocument();
     UT_UTF8String sVal;
     UT_UTF8String_sprintf(sVal,"%d",i);
     const char * attr = PT_PROPS_ATTRIBUTE_NAME;
     UT_UTF8String sAttVal = "frame-pref-column:";
     sAttVal += sVal.utf8_str();
     
     pDoc->changeStruxAttsNoUpdate(pFL->getStruxDocHandle(),attr,sAttVal.utf8_str());
}

/*!
 * This method returns the padding to be applied between a line approaching
 * wrapped frame or image from the left. 
 * y Is the top of the line in logical units as defined relative to the
 * y position on the screen.
 * height is the height of the line.
 * If tight wrapping is set on a positioned object this number can be negative
 * which means the line can encroach into the rectangular region of the
 * image provided the region is transparent.
 */
UT_sint32 fp_FrameContainer::getLeftPad(UT_sint32 y, UT_sint32 height)
{
  fl_FrameLayout *pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
  UT_sint32 pad = pFL->getBoundingSpace();	
  UT_Rect * pRect = getScreenRect();
  UT_sint32 yC = pRect->top;
  delete pRect;
  if(!isTightWrapped() || !isWrappingSet())
  {
    return pad;
  }
  if(FL_FRAME_TEXTBOX_TYPE == pFL->getFrameType())
  {
    return pad;
  }
  if(pFL->getBackgroundImage() == NULL)
  {
    return pad;
  }
  UT_sint32 iLeft = pFL->getBackgroundImage()->GetOffsetFromLeft(getGraphics(),pad,y - yC,height);
  xxx_UT_DEBUGMSG(("Local Y %d iLeft %d width %d \n",y-yC,iLeft,getFullWidth()));  return iLeft;
}


/*!
 * This method returns the padding to be applied between a line approaching
 * wrapped frame or image from the right. 
 * y Is the top of the line in logical units as defined relative to the
 * y position on the screen.
 * height is the height of the line.
 * If tight wrapping is set on a positioned object this number can be negative
 * which means the line can encroach into the rectangular region of the
 * image provided the region is transparent.
 */
UT_sint32 fp_FrameContainer::getRightPad(UT_sint32 y, UT_sint32 height)
{
  fl_FrameLayout *pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
  UT_sint32 pad = pFL->getBoundingSpace();
  UT_Rect * pRect = getScreenRect();
  UT_sint32 yC = pRect->top;
  if(!isTightWrapped() || !isWrappingSet())
  {
    return pad;
  }
  if(FL_FRAME_TEXTBOX_TYPE == pFL->getFrameType())
  {
    return pad;
  }
  if(pFL->getBackgroundImage() == NULL)
  {
    return pad;
  }
  UT_sint32 iRight = pFL->getBackgroundImage()->GetOffsetFromRight(getGraphics(),pad,y - yC,height);
  xxx_UT_DEBUGMSG(("Local Y %d iRight %d width %d \n",y-yC,iRight,getFullWidth()));
  return iRight;
}

void fp_FrameContainer::clearScreen(void)
{
	fp_Page * pPage = getPage();
	if(pPage == NULL)
	{
		return;
	}
	if(getView() == NULL)
	{
		return;
	}

	UT_sint32 srcX,srcY;
	UT_sint32 xoff,yoff;
	getView()->getPageScreenOffsets(pPage,xoff,yoff);
	xxx_UT_DEBUGMSG(("pagescreenoffsets xoff %d yoff %d \n",xoff,yoff));
	UT_sint32 leftThick = m_lineLeft.m_thickness;
	UT_sint32 rightThick = m_lineRight.m_thickness;
	UT_sint32 topThick = m_lineTop.m_thickness;
	UT_sint32 botThick = m_lineBottom.m_thickness;

	srcX = getFullX() - leftThick;
	srcY = getFullY() - topThick;

	xoff += getFullX() - leftThick;
	yoff += getFullY() - topThick;
	getFillType().getParent()->Fill(getGraphics(),srcX,srcY,xoff,yoff,getFullWidth()+leftThick+rightThick,getFullHeight()+topThick+botThick+getGraphics()->tlu(1) +1);
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< countCons(); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		pCon->clearScreen();
	}
	m_bNeverDrawn = true;
}

/*!
 * All these methods are used to implement an X and Y padding around the
 * Frame
 */
UT_sint32 fp_FrameContainer::getFullWidth(void) const
{
	return fp_VerticalContainer::getWidth();
}

UT_sint32 fp_FrameContainer::getFullHeight(void) const
{
	return fp_VerticalContainer::getHeight();
}

UT_sint32 fp_FrameContainer::getFullX(void) const
{
	return fp_VerticalContainer::getX();
}

UT_sint32 fp_FrameContainer::getFullY(void) const
{
	return fp_VerticalContainer::getY();
}


UT_sint32 fp_FrameContainer::getWidth(void) const
{
	UT_sint32 iWidth = fp_VerticalContainer::getWidth() - m_iXpad*2;
	return iWidth;
}

UT_sint32 fp_FrameContainer::getX(void) const
{
	UT_sint32 iX = fp_VerticalContainer::getX() + m_iXpad;
	return iX;
}


UT_sint32 fp_FrameContainer::getY(void) const
{
	UT_sint32 iY = fp_VerticalContainer::getY() + m_iYpad;
	return iY;
}

UT_sint32 fp_FrameContainer::getHeight(void) const
{
	UT_sint32 iHeight = fp_VerticalContainer::getHeight() - m_iYpad*2;
	return iHeight;
}

	
void fp_FrameContainer::setContainer(fp_Container * /*pContainer*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

fl_DocSectionLayout * fp_FrameContainer::getDocSectionLayout(void)
{
	fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pFL->getSectionLayout());
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}

/*!
 * Fill the supplied vector with a list of the blocks whose lines are affected
 * by the Frame.
 */ 
void fp_FrameContainer::getBlocksAroundFrame(UT_GenericVector<fl_BlockLayout *> & vecBlocks)
{
  fp_Page * pPage = getPage();
  if(pPage == NULL)
  {
    return;
  }
  UT_sint32 iColLeader = 0;
  fp_Column * pCol = NULL;
  fl_BlockLayout * pCurBlock = NULL;
  fp_Line * pCurLine = NULL;
  fp_Container * pCurCon = NULL;
  if(pPage->countColumnLeaders() == 0)
  {
      UT_sint32 iPage = getPreferedPageNo();
      if(iPage >0)
          setPreferedPageNo(iPage-1);
      return;
  }
  for(iColLeader = 0; iColLeader < pPage->countColumnLeaders(); iColLeader++)
  {
      pCol = pPage->getNthColumnLeader(iColLeader);
      while(pCol)
      {
          UT_sint32 i = 0;
          UT_sint32 iYCol = pCol->getY(); // Vertical position relative to page.
          for(i=0; i< pCol->countCons(); i++)
          {
              pCurCon = static_cast<fp_Container *>(pCol->getNthCon(i));
              if(pCurCon->getContainerType() == FP_CONTAINER_LINE)
              {
                  pCurLine = static_cast<fp_Line *>(pCurCon);
                  UT_sint32 iYLine = iYCol + pCurLine->getY();
                  xxx_UT_DEBUGMSG(("iYLine %d FullY %d FullHeight %d \n",iYLine,getFullY(),getFullHeight()));
                  if((iYLine + pCurLine->getHeight() > getFullY()) && (iYLine < (getFullY() + getFullHeight())))
                  {
                      //
                      // Line overlaps frame in Height. Add it's block to the vector.
                      //
                      if(pCurLine->getBlock() != pCurBlock)
                      {
                          pCurBlock = pCurLine->getBlock();
                          vecBlocks.addItem(pCurBlock);
                          xxx_UT_DEBUGMSG(("Add Block %x to vector \n",pCurBlock));
                      }
                  }
              }
          }
          pCol = pCol->getFollower();
      }
  }
  if(vecBlocks.getItemCount() == 0)
  {
      pCol = pPage->getNthColumnLeader(0);
      fp_Container * pCon = pCol->getFirstContainer();
      fl_BlockLayout * pB = NULL;
      if(pCon && pCon->getContainerType() == FP_CONTAINER_LINE)
      {
          pB = static_cast<fp_Line *>(pCon)->getBlock();
      }
      else if(pCon)
      {
          fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pCon->getSectionLayout());
          pB = pCL->getNextBlockInDocument();
      }
      if(pB != NULL)
          vecBlocks.addItem(pB);
  }

}

/* just a little helper function
 */
void fp_FrameContainer::_drawLine (const PP_PropertyMap::Line & style,
								  UT_sint32 left, UT_sint32 top, UT_sint32 right, UT_sint32 bot,GR_Graphics * pGr)
{
	GR_Painter painter(pGr);

	if (style.m_t_linestyle == PP_PropertyMap::linestyle_none)
		return; // do not draw	
	
	GR_Graphics::JoinStyle js = GR_Graphics::JOIN_MITER;
	GR_Graphics::CapStyle  cs = GR_Graphics::CAP_PROJECTING;

	UT_sint32 iLineWidth = static_cast<UT_sint32>(style.m_thickness);
	pGr->setLineWidth (iLineWidth);
	pGr->setColor (style.m_color);

	switch (style.m_t_linestyle)
	{
		case PP_PropertyMap::linestyle_dotted:
			pGr->setLineProperties (iLineWidth, js, cs, GR_Graphics::LINE_DOTTED);
			break;
		case PP_PropertyMap::linestyle_dashed:
			pGr->setLineProperties (iLineWidth, js, cs, GR_Graphics::LINE_ON_OFF_DASH);
			break;
		case PP_PropertyMap::linestyle_solid:
			pGr->setLineProperties (iLineWidth, js, cs, GR_Graphics::LINE_SOLID);
			break;
		default: // do nothing; shouldn't happen
			break;
	}

	xxx_UT_DEBUGMSG(("_drawLine: top %d bot %d \n",top,bot));

	painter.drawLine (left, top, right, bot);
	
	pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
}

/*!
 * Draw the frame boundaries
 */
void  fp_FrameContainer::drawBoundaries(dg_DrawArgs * pDA)
{
	UT_sint32 iXlow = pDA->xoff - m_iXpad;
	UT_sint32 iXhigh = iXlow + getFullWidth() ;
	UT_sint32 iYlow = pDA->yoff - m_iYpad;
	UT_sint32 iYhigh = iYlow + getFullHeight();
	GR_Graphics * pG = pDA->pG;
	if(getPage())
	{
		getPage()->expandDamageRect(iXlow,iYlow,getFullWidth(),getFullHeight());

		//
		// Only fill to the bottom of the viewed page.
		//
		UT_sint32 iFullHeight = getFullHeight();
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		UT_sint32 iMaxHeight = 0;
		if(!pG->queryProperties(GR_Graphics::DGP_PAPER) && (getView()->getViewMode() != VIEW_PRINT))
		{
		        iMaxHeight = pDSL->getActualColumnHeight();
		}
		else
		{
		        iMaxHeight = getPage()->getHeight();
		}
		UT_sint32 iBot = getFullY()+iFullHeight;
		if(iBot > iMaxHeight)
		{
		        iFullHeight = iFullHeight - (iBot-iMaxHeight);
			iYhigh = iFullHeight;
		}
	}
	_drawLine(m_lineTop,iXlow,iYlow,iXhigh,iYlow,pDA->pG); // top
	_drawLine(m_lineRight,iXhigh,iYlow,iXhigh,iYhigh,pDA->pG); // right
	_drawLine(m_lineBottom,iXlow,iYhigh,iXhigh,iYhigh,pDA->pG); // bottom
	_drawLine(m_lineLeft,iXlow,iYlow,iXlow,iYhigh,pDA->pG); // left
}


/*!
 * Draw the frame handles
 */
void  fp_FrameContainer::drawHandles(dg_DrawArgs * pDA)
{
        if(getView() == NULL)
	{
	     getSectionLayout()->format();
	     getSectionLayout()->setNeedsReformat(getSectionLayout());
	}
        if(getView() == NULL)
	{
	     return;
	}
	if(!getPage())
	{
	     return;
	}
	//
	// Only fill to the bottom of the viewed page.
	//
	GR_Graphics * pG = pDA->pG;
	UT_sint32 iFullHeight = getFullHeight();
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	UT_sint32 iMaxHeight = 0;
	if(!pG->queryProperties(GR_Graphics::DGP_PAPER) && (getView()->getViewMode() != VIEW_PRINT))
	{
	    iMaxHeight = pDSL->getActualColumnHeight();
	}
	else
	{
	    iMaxHeight = getPage()->getHeight();
	}
	UT_sint32 iBot = getFullY()+iFullHeight;
	if(iBot > iMaxHeight)
	{
	    iFullHeight = iFullHeight - (iBot-iMaxHeight);
	}
	UT_sint32 iXlow = pDA->xoff - m_iXpad;
	UT_sint32 iYlow = pDA->yoff - m_iYpad;

	UT_Rect box(iXlow + pDA->pG->tlu(2), iYlow + pDA->pG->tlu(2), getFullWidth() - pDA->pG->tlu(4), iFullHeight - pDA->pG->tlu(4));
	getPage()->expandDamageRect(box.left,box.top,box.width,box.height);
	getView()->drawSelectionBox(box, true);
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_FrameContainer::draw(dg_DrawArgs* pDA)
{
	FV_View * pView = getView();
	UT_return_if_fail( pView);
	
	xxx_UT_DEBUGMSG(("FrameContainer %x called, page %x \n",this,getPage()));
	if(getPage() == NULL)
	{
	     getSectionLayout()->format();
	     getSectionLayout()->setNeedsReformat(getSectionLayout());
	     if(getPage() == NULL)
	     {
			 return;
	     }
	}
	if(pView)
	{
		if(pView->getFrameEdit()->getFrameEditMode() == FV_FrameEdit_DRAG_EXISTING)
		{
			if((pView->getFrameEdit()->getFrameContainer() == this))
			{
				return;
			}
		}
	}
//
// Only draw the lines in the clipping region.
//
/*
	[Somewhere down here is where the logic to only draw the region of the frame which
	is within the complement of the union of all higher frames needs to be. We need to
	draw the applicable region of the rectangle we're on, then unify it with (if
	applicable) the higher union.] <-- Possibly obsolete comment, not sure.
	I think I might have landed on an alternative solution involving more rearranging
	of the storage of the FrameContainers, based on their z-index.  Not sure how far
	I got with that or if it worked either.  See also abi bug 7664 and the original
	discussions about defining the undefinedness of layered frame behaviour.
*/

	if(m_bOverWrote)
	{
		pDA->bDirtyRunsOnly = false;
	}
	dg_DrawArgs da = *pDA;
	GR_Graphics * pG = da.pG;
	UT_return_if_fail( pG);

	UT_sint32 x = pDA->xoff - m_iXpad;
	UT_sint32 y = pDA->yoff - m_iYpad;
	getPage()->expandDamageRect(x,y,getFullWidth(),getFullHeight());
	if(!pDA->bDirtyRunsOnly || m_bNeverDrawn)
	{
		if(m_bNeverDrawn)
		{
			pDA->bDirtyRunsOnly= false;
		} 
		UT_sint32 srcX,srcY;
		getSectionLayout()->checkGraphicTick(pG);
		srcX = -m_iXpad;
		srcY = -m_iYpad;
		//
		// Only fill to the bottom of the viewed page.
		//
		UT_sint32 iFullHeight = getFullHeight();
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		UT_sint32 iMaxHeight = 0;
		if(!pG->queryProperties(GR_Graphics::DGP_PAPER) && (pView->getViewMode() != VIEW_PRINT))
		{
		        iMaxHeight = pDSL->getActualColumnHeight();
		}
		else
		{
		        iMaxHeight = getPage()->getHeight();
		}
		UT_sint32 iBot = getFullY()+iFullHeight;
		if(iBot > iMaxHeight)
		{
		        iFullHeight = iFullHeight - (iBot-iMaxHeight);
		}
		getFillType().Fill(pG,srcX,srcY,x,y,getFullWidth(),iFullHeight);
		m_bNeverDrawn = false;
	}
	UT_uint32 count = countCons();
	xxx_UT_DEBUGMSG(("Number of containers in frame %d \n",count));
	const UT_Rect * pPrevRect = pDA->pG->getClipRect();
	UT_Rect * pRect = getScreenRect();
	UT_Rect newRect;
	bool bRemoveRectAfter = false;
	bool bSetOrigClip = false;
	bool bSkip = false;
	if((pPrevRect == NULL) && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		pDA->pG->setClipRect(pRect);
		UT_DEBUGMSG(("Clip bottom is %d \n",pRect->top + pRect->height));
		bRemoveRectAfter = true;
	}
	else if(pPrevRect && !pRect->intersectsRect(pPrevRect))
	{
		bSkip = true;
		xxx_UT_DEBUGMSG(("External Clip bottom is %d \n",pRect->top + pRect->height));
	}
	else if(pPrevRect)
	{
		newRect.top = UT_MAX(pPrevRect->top,pRect->top);
		UT_sint32 iBotPrev = pPrevRect->height + pPrevRect->top;
		UT_sint32 iBot = pRect->height + pRect->top;
		newRect.height = UT_MIN(iBotPrev,iBot) - newRect.top;
		newRect.width = pPrevRect->width;
		newRect.left = pPrevRect->left;
		if((newRect.height > 0) && pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			pDA->pG->setClipRect(&newRect);
			bSetOrigClip = true;
		}
		else
		{
			bSkip = true;
		}
	}
	if(!bSkip)
	{
		for (UT_uint32 i = 0; i<count; i++)
		{
			fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
			da.xoff = pDA->xoff + pContainer->getX();
			da.yoff = pDA->yoff + pContainer->getY();
			pContainer->draw(&da);
		}
	}
	m_bNeverDrawn = false;
	m_bOverWrote = false;
	if(bRemoveRectAfter)
	{
		pDA->pG->setClipRect(NULL);
	}
	if(bSetOrigClip)
	{
		pDA->pG->setClipRect(pPrevRect);
	}
	delete pRect;
	drawBoundaries(pDA);
}

void fp_FrameContainer::setBackground (const PP_PropertyMap::Background & style)
{
	m_background = style;
	PP_PropertyMap::Background background = m_background;
	if(background.m_t_background == PP_PropertyMap::background_solid)
	{
		getFillType().setColor(background.m_color);
	}
}


/*!
 * FrameContainers are not in the linked list of physical containers
 */
fp_Container * fp_FrameContainer::getNextContainerInSection() const
{
	return NULL;
}

/*!
 * FrameContainers are not in the linked list of physical containers
 */
fp_Container * fp_FrameContainer::getPrevContainerInSection() const
{
	return NULL;
}

void fp_FrameContainer::layout(void)
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
		if(iY > getHeight())
		{
			pContainer->setY(-1000000);
		}
		else
		{
			pContainer->setY(iY);
		}
		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pContainer);
			iContainerHeight = pTab->getHeight();
			if(!pTab->isThisBroken() && (pTab->getFirstBrokenTable() == NULL))
			{
				/*fp_Container * pBroke = static_cast<fp_Container *> */(pTab->VBreakAt(0));
			}
		}

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
		if(iY > getHeight())
		{
			pPrevContainer->setAssignedScreenHeight(-1000000);
		}
		else
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
		}
	}
	fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(getSectionLayout());
	if(pFL->expandHeight() && (iY > pFL->minHeight()))
	{
	     setHeight(iY+m_iYpad*2);
	}
}

void fp_FrameContainer::setHeight(UT_sint32 iY)
{
        if(iY != getFullHeight())
	{
	     xxx_UT_DEBUGMSG((" SetHeight Frame iY %d Fullheight %d Height %d \n",iY,getFullHeight(),getHeight()));
	     clearScreen();
	     fp_VerticalContainer::setHeight(iY);
	     fp_Page * pPage = getPage();
	     getDocSectionLayout()->setNeedsSectionBreak(true,pPage);
	}
}
