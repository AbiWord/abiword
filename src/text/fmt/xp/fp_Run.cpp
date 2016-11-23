/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"

#include "fp_Column.h"
#include "fp_Page.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "gr_DrawArgs.h"
#include "pp_AttrProp.h"
#include "fd_Field.h"
#include "po_Bookmark.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_growbuf.h"
#include "ut_go_file.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_FootnoteLayout.h"
#include "fp_FootnoteContainer.h"
#include "fl_AutoNum.h"
#include "fv_View.h"
#include "fl_TOCLayout.h"
#include "ap_Prefs.h"
#include "ap_Strings.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "gr_Painter.h"

#include "ut_sleep.h"

// define this if you want fp_Run::lookupProperties() to dump props and attr belonging to
// this run. NB this has a very adverse effect on performance, so it should be turned off
// before committing
//#define FPRUN_PROPS_MINI_DUMP

// TODO can we use the indexAP provided in the change records
// TODO to remember the attr/prop for each run rather than
// TODO looking it up each time we call lookupProperties() -- jeff 4/19/99


// findPointCoords:
//  Can be called to find the position and size of the point (cursor)
//  for an offset both before, inside and after the Run.
//   Before: When the Run is the first on a Line and point is a BOL.
//   Inside: When the point is inside the Run.
//   After : Point is at the start of the next Run, but insertion is
//           done with the properties of this Run so cursor size/position
//           must reflect that.
//
//  Previous implementations would assert that the offset was within the
//  Run, but that would always fail for the 'After' case.

/*****************************************************************/

/*
  TODO this file is too long -- it needs to be broken
  up into several pieces.
*/

fp_Run::fp_Run(fl_BlockLayout* pBL,
			   UT_uint32 iOffsetFirst,
			   UT_uint32 iLen,
			   FP_RUN_TYPE iType) : 
	fp_ContainerObject(FP_CONTAINER_RUN, pBL->getSectionLayout()),
	m_iType (iType),
	m_pLine(0),
	m_pBL(pBL),
	m_pNext(0),
	m_pPrev(0),
	m_iX(0),
	m_iOldX(0),
	m_iY(0),
	m_iWidth(0),
	m_iHeight(0),
	m_iAscent(0),
	m_iDescent(0),
	m_iOffsetFirst(iOffsetFirst),
	m_iLen(iLen),
	m_bDirty(true),	// a run which has just been created is not onscreen, therefore it is dirty
	m_pField(0),
	m_iDirection(UT_BIDI_WS), //by default all runs are whitespace
	m_iVisDirection(UT_BIDI_UNSET),
	m_eRefreshDrawBuffer(GRSR_Unknown), // everything
	m_pColorHL(255,255,255,true), // set highlight colour to transparent
	m_pFont(0),
	m_bRecalcWidth(false),
	m_fDecorations(0),
	m_iLineWidth(0),
	m_iLinethickness(0),
	m_iUnderlineXoff(0),
	m_imaxUnderline(0),
	m_iminOverline(0),
	m_iOverlineXoff(0),
	m_pHyperlink(0),
	m_pRevisions(NULL),
	m_eVisibility(FP_VISIBLE),
	m_bIsCleared(true),
	m_FillType(NULL,this,FG_FILL_TRANSPARENT),
	m_bPrinting(false),
	m_iTmpX(0),
	m_iTmpY(0),
	m_iTmpWidth(0),
	m_pTmpLine(NULL),
	m_bDrawSelection(false),
	m_iSelLow(0),
	m_iSelHigh(0),
	m_bMustClearScreen(false),
	m_iAuthorColor(0)
#ifdef DEBUG
	,m_iFontAllocNo(0)
#endif
{
	xxx_UT_DEBUGMSG(("fp_Run %x created!!! \n",this));
	pBL->setPrevListLabel(false);
	m_FillType.setDocLayout(m_pBL->getDocLayout());
}

fp_Run::~fp_Run()
{
  // no impl.
	xxx_UT_DEBUGMSG(("~fp_Run %x deleted!!! \n",this));

// Zero these to trap mem errors.
#if 1
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pBL = NULL;
	m_pLine = NULL;
#endif

	DELETEP(m_pRevisions);
}

UT_sint32 fp_Run::getHeight() const
{
	if(isHidden() == FP_VISIBLE)
	{
		return m_iHeight;
	}
	return 0;
}

UT_sint32 fp_Run::getWidth() const
{
	if(isHidden() == FP_VISIBLE)
		return m_iWidth;
	
	return 0;
}

UT_uint32 fp_Run::getAscent() const
{
	if(isHidden() == FP_VISIBLE)
	{
		FL_DocLayout * pLayout = getBlock()->getDocLayout();
		if(getGraphics() && pLayout->isQuickPrint() && getGraphics()->queryProperties(GR_Graphics::DGP_PAPER) && (getType() != FPRUN_IMAGE) && (getType() != FPRUN_TEXT)&& (getType() != FPRUN_FIELD))
		{
			return static_cast<UT_uint32>(static_cast<double>(m_iAscent)*getGraphics()->getResolutionRatio());
		}
		return m_iAscent;
	}
	return 0;
}

UT_uint32 fp_Run::getDescent() const
{
	if(isHidden() == FP_VISIBLE)
	{
		FL_DocLayout * pLayout = getBlock()->getDocLayout();
		if(getGraphics() && pLayout->isQuickPrint() && getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
		{
			return static_cast<UT_uint32>(static_cast<double>(m_iDescent)*getGraphics()->getResolutionRatio());
		}
		return m_iDescent;
	}

	return 0;
}

UT_sint32 fp_Run::getDrawingWidth() const
{
	if(isHidden() == FP_VISIBLE)
		return static_cast<UT_sint32>(m_iWidth);

	return 0;
}


fg_FillType & fp_Run::getFillType(void)
{
	return m_FillType;
}

const fg_FillType & fp_Run::getFillType(void) const
{
	return m_FillType;
}

bool fp_Run::isInSelectedTOC(void)
{
	if(getBlock()->isContainedByTOC())
	{
		fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getBlock()->myContainingLayout());
		return pTOCL->isSelected();
	}
	else
	{
		return false;
	}
			
}

/*!
 * The following are methods to avoid flickering while changing a selection.
 * The idea is to generate a cliprect around just the selected text.
 */

/*!
 * Set the selection draw mode.
 */
void fp_Run::setSelectionMode(PT_DocPosition posLow, PT_DocPosition posHigh)
{
	m_bDrawSelection = true;
	m_iSelLow = posLow;
	m_iSelHigh = posHigh;
}

/*!
 * Clear the selection mode
 */
void fp_Run::clearSelectionMode(void)
{
	m_bDrawSelection = false;
	m_iSelLow = 0;
	m_iSelHigh = 0;
}

bool fp_Run::isSelectionDraw(void) const
{
	return m_bDrawSelection;
}

PT_DocPosition fp_Run::posSelLow(void) const
{
	return m_iSelLow;
}

PT_DocPosition fp_Run::posSelHigh(void) const
{
	return m_iSelHigh;
}
/*!
 * This method looks at the values of TmpX and TmpWidth and compares them
 * to the new ones. If they're different we do a clearscreen on them.
 */
bool fp_Run::clearIfNeeded(void)
{
	// only do this on runs that have not been cleared already
	// see bug 8154
	if(m_bIsCleared &&!getMustClearScreen() )
		return true;
	
	//	if((getTmpX() == getX()) && (getTmpWidth() == getWidth()) && (getTmpY() == getY()))
	if((getTmpX() == getX()) && (getTmpY() == getY()) && (getTmpLine() == getLine()) && !getMustClearScreen() )
	{
		return true;
	}
	if(getTmpLine() && (getLine() != getTmpLine()))
	{
		fp_Line * pTmpLine = getTmpLine();
		UT_sint32 i = getBlock()->findLineInBlock(pTmpLine);
		if(i < 0)
		{
			markWidthDirty();
			return false;
		}
		fp_Run * pLastRun = pTmpLine->getLastRun();
		pTmpLine->clearScreenFromRunToEnd(pLastRun);
		markWidthDirty();
		return false;
	}
	UT_sint32 iWidth = getWidth();
	UT_sint32 iX = getX();
	UT_sint32 iY = getY();
	_setWidth(getTmpWidth());
	//
	// Special case of merging the first char into the first run of
	// block
	//
	if(getMustClearScreen() && (getTmpWidth() == 0) && (getX() == getTmpX()))
		_setWidth(iWidth);
	_setX(getTmpX());
	_setY(getTmpY());
	//
	// If a run was created by "split" it initially has X value zero. Don't
    // reset the "clear" in this.
    //
    // If the run was just newly inserted (and is, therefore, not on screen) do not reset
    // its clear flag
	//
	if(getTmpX() != 0 && getTmpWidth() != 0)
	{
		m_bIsCleared = false;
	}
	clearScreen();
	markWidthDirty();
	_setX(iX);
	_setWidth(iWidth);
	_setY(iY);
	return false;
}
void fp_Run::Fill(GR_Graphics * pG, UT_sint32 x, UT_sint32 y, UT_sint32 width,
				  UT_sint32 height)
{
	xxx_UT_DEBUGMSG(("-------------------Fill called!!!!---- x %d width %d \n",x,width));
	if((width < 1) || (height < 1))
	{
		return;
	}
	if(y < -9999999)
	{
			// Whoops! object is offscreen!
		    // Bailout
			return;
	}
	UT_sint32 srcX = 0;
	UT_sint32 srcY = 0;
	fp_Line * pLine = getLine();
	UT_sint32 xoff=0,yoff=0;
	if(pLine)
	{
		pLine->getScreenOffsets(this,xoff,yoff);
		fp_Page * pPage = pLine->getPage();
		srcX = x - xoff;
		if(pPage)
		{
			pPage->expandDamageRect(xoff+getX()+srcX,yoff+getY(),width,height);
		}
	}
	bool bDoGrey = (pG->queryProperties(GR_Graphics::DGP_SCREEN) && 
					((getType() ==  FPRUN_FIELD) || getBlock()->isContainedByTOC()));
	if(bDoGrey && isInSelectedTOC())
	{
		bDoGrey = false;
	}
	if(!bDoGrey)
	{
		m_FillType.Fill(pG,srcX,srcY,x,y,width,height);
	}
	else
	{
		if(x>=xoff && width <= getWidth())
		{
			UT_RGBColor grey(192,192,192);	
		//		UT_RGBColor white(255,255,255);	
			GR_Painter painter(pG);
		//	painter.fillRect(white,x,y,width,height);
			painter.fillRect(grey,x,y,width,height);
		}
		else
		{
			m_FillType.Fill(pG,srcX,srcY,x,y,width,height);
		}
	}
}

void fp_Run::lookupProperties(GR_Graphics * pG)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	bool bGraphicsNull = false;
	getBlockAP(pBlockAP);

	PD_Document * pDoc = m_pBL->getDocument();

#ifdef FPRUN_PROPS_MINI_DUMP
	UT_DEBUGMSG(("fp_Run::lookupProperties: dumping block AP\n"));
	if(pBlockAP)
		pBlockAP->miniDump(pDoc);
#endif
	// examining the m_pRevisions contents is too involved, it is
	// faster to delete it and create a new instance if needed
	DELETEP(m_pRevisions);

	setVisibility(FP_VISIBLE); // set default visibility
	
	// NB the call will recreate m_pRevisions for us and it will
	// change visibility if it is affected by the presence of revisions
	if(!getBlock()->isContainedByTOC())
	{
		getSpanAP(pSpanAP);
	}
	else
	{
		pSpanAP = pBlockAP;
	}
	xxx_UT_DEBUGMSG(("fp_Run: pSpanAP %x \n",pSpanAP));

#ifdef FPRUN_PROPS_MINI_DUMP
	UT_DEBUGMSG(("fp_Run::lookupProperties: dumping span AP\n"));
	if(pSpanAP)
		pSpanAP->miniDump(pDoc);
#endif
	
	//evaluate the "display" property and superimpose it over anything
	//we got as the result of revisions
	const gchar *pszDisplay = PP_evalProperty("display",pSpanAP,pBlockAP,
												 pSectionAP, pDoc, true);

	if(pszDisplay && !strcmp(pszDisplay,"none"))
	{
		if(m_eVisibility == FP_VISIBLE)
			setVisibility(FP_HIDDEN_TEXT);
		else
			setVisibility(FP_HIDDEN_REVISION_AND_TEXT);
	}

	// here we handle background colour -- we parse the property into
	// m_pColorHL and then call updateHighlightColor() to overlay any
	// colour from higher layout elements
	const char * pszBGcolor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	_setColorHL(pszBGcolor);
	//	m_FillType.setColor(pszBGcolor); // we should clear with screen color
	// and draw with background color
	if(pG == NULL)
	{
		m_bPrinting = false;
		pG = getGraphics();
		bGraphicsNull = true;
	}
	else if(pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		m_bPrinting = true;
	}
	if(!m_pBL->isContainedByTOC())
	{
		if(bGraphicsNull)
			_lookupProperties(pSpanAP, pBlockAP, pSectionAP,NULL);
		else
			_lookupProperties(pSpanAP, pBlockAP, pSectionAP,pG);
	}
	else
	{
		if(bGraphicsNull)
			_lookupProperties(NULL, pBlockAP, pSectionAP,NULL);
		else
			_lookupProperties(NULL, pBlockAP, pSectionAP,pG);
	}
	const char * szAuthorInt = NULL;	
	if(pSpanAP && pDoc->isShowAuthors())
	{
		if(pSpanAP->getAttribute(PT_AUTHOR_NAME,szAuthorInt))
		{
			if(szAuthorInt)
				m_iAuthorColor = atoi(szAuthorInt);
		}
	}
	else
	{
		m_iAuthorColor = 0;
	}
	// here we used to set revision-based visibility, but that has to
	// be done inside getSpanAP() because we need to know whether the
	// revision is to be visible or not before we can properly apply
	// any properties the revision contains.

}
#undef FPRUN_PROPS_MINI_DUMP
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

/*
 Determine best split point in Run
 \param iMaxLeftWidth Width to split at
 \retval si Split information (left width, right width, and position)
 \param bForce Force a split at first opportunity (max width)
 \return True if split point was found in this Run, otherwise false.

 This implementation simply returns false, forcing line breaker to
 look for a split point in previous Runs.
*/
bool fp_Run::findMaxLeftFitSplitPoint(UT_sint32 /* iMaxLeftWidth */,
									  fp_RunSplitInfo& /* si */,
									  bool /* bForce */)
{
	return false;
}

bool fp_Run::hasLayoutProperties(void) const
{
	return false;
}

/*!
  Find previous Run in block which holds property information
  \return Run with property information or NULL
*/
fp_Run*
fp_Run::_findPrevPropertyRun(void) const
{
	fp_Run* pRun = getPrevRun();
	while (pRun && (!pRun->hasLayoutProperties() || pRun->isHidden() || (pRun->getType() == FPRUN_IMAGE)))
	    pRun = pRun->getPrevRun();
	if(pRun == NULL)
	{
		pRun = getPrevRun();
		while (pRun && (!pRun->hasLayoutProperties() || pRun->isHidden()))
			pRun = pRun->getPrevRun();

	}
	return pRun;
}

bool fp_Run::displayAnnotations(void) const
{
	if(!getBlock())
		return false;
	if(!getBlock()->getDocLayout())
		return false;
	return getBlock()->getDocLayout()->displayAnnotations();
}

bool fp_Run::displayRDFAnchors(void) const
{
	if(!getBlock())
		return false;
	if(!getBlock()->getDocLayout())
		return false;
	return getBlock()->getDocLayout()->displayRDFAnchors();
}

/*!
  Inherit attribute properties from previous Run

  This is used by Runs for which it does not make sense to have
  properties, such as forced line breaks end EOP Runs.
*/
void
fp_Run::_inheritProperties(void)
{
	fp_Run* pRun = _findPrevPropertyRun();
	if (pRun)
	{
		xxx_UT_DEBUGMSG(("fp_Run::_inheritProperties: from prev run\n"));
		_setAscent(pRun->getAscent());
		_setDescent(pRun->getDescent());
		_setHeight(pRun->getHeight());
		xxx_UT_DEBUGMSG(("fp_Run::_inheritProperties: from prev run height is %d \n",getHeight()));
		
	}
	else
	{
		// look for fonts in this DocLayout's font cache
		//UT_DEBUGMSG(("fp_Run::_inheritProperties: from current font\n"));
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?

		//m_pBL->getSpanAttrProp(getBlockOffset(),true,&pSpanAP);
		getSpanAP(pSpanAP);
		getBlockAP(pBlockAP);

		FL_DocLayout * pLayout = getBlock()->getDocLayout();
		const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());

		if ((pFont != _getFont()) || (getType() == FPRUN_ENDOFPARAGRAPH))
		{
			_setFont(pFont);
		    _setAscent(getGraphics()->getFontAscent(pFont));
			_setDescent(getGraphics()->getFontDescent(pFont));
		    _setHeight(getGraphics()->getFontHeight(pFont));
		}
		xxx_UT_DEBUGMSG(("fp_Run::_inheritProperties: No prev run run height is %d \n",getHeight()));
	}
}

GR_Graphics * fp_Run::getGraphics(void) const
{
	if(m_bPrinting)
	{
		if(getBlock()->getDocLayout()->isQuickPrint())
		{
			return getBlock()->getDocLayout()->getQuickPrintGraphics();
		}
	}
	if(getBlock()->getView())
	{
		return getBlock()->getView()->getGraphics();
	}
	return getBlock()->getDocLayout()->getGraphics();
}

void fp_Run::insertIntoRunListBeforeThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.setNextRun(this);
	if (m_pPrev)
	{
		m_pPrev->setNextRun(&newRun);
		if(newRun.getType()!= FPRUN_HYPERLINK)
		newRun.setHyperlink( m_pPrev->getHyperlink());
	}
	newRun.setPrevRun(m_pPrev);
	setPrevRun(&newRun);

}

void fp_Run::insertIntoRunListAfterThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.setPrevRun(this);
	if(newRun.getType()!= FPRUN_HYPERLINK)
		newRun.setHyperlink(m_pHyperlink);
	if (m_pNext)
	{
		m_pNext->setPrevRun(&newRun);
	}
	newRun.setNextRun(m_pNext);
	setNextRun(&newRun);
}

void fp_Run::unlinkFromRunList()
{
	//first if this is the start of a hyperlink run,
	//remove the references to it from the following runs
	if(getType() == FPRUN_HYPERLINK)
	{
		fp_HyperlinkRun * pH = static_cast<fp_HyperlinkRun*>(this);
		if(pH->isStartOfHyperlink())
		{
			fp_Run * pRun = getNextRun();

			while(pRun && pRun->getHyperlink() == pH)
			{
				pRun->setHyperlink(NULL);
				pRun = pRun->getNextRun();
			}
		}
	}

	if (m_pPrev)
	{
		m_pPrev->setNextRun(m_pNext);
	}
	if (m_pNext)
	{
		m_pNext->setPrevRun(m_pPrev);
		setNextRun(0);
	}
	setPrevRun(0);
}

void	fp_Run::setHyperlink(fp_HyperlinkRun * pH)
{
	if(pH != m_pHyperlink)
	{
		_setHyperlink(pH);
		clearScreen();
	}
}

/*! returns PP_AttrProp associated with this span, taking on board the
    presence of revisions
    \returns pSpan : location to store the PP_AttrProp
*/
const PP_AttrProp * fp_Run::getSpanAP(void)
{
	const PP_AttrProp * pAP = NULL; 
	getSpanAP(pAP); 
	return pAP;
}

/*! returns PP_AttrProp associated with this span, taking on board the
    presence of revisions
    \param pSpan : location to store the PP_AttrProp
*/
void fp_Run::getSpanAP(const PP_AttrProp * &pSpanAP)
{
	if(getBlock()->isContainedByTOC())
	{
		getBlockAP(pSpanAP);
		return;
	}
		
	//first we need to ascertain if this revision is visible
	FV_View* pView = _getView();
	UT_return_if_fail(pView);

	UT_uint32 iId  = pView->getRevisionLevel();
	bool bShow     = pView->isShowRevisions();
	bool bHiddenRevision = false;

	if(getType() != FPRUN_FMTMARK && getType() != FPRUN_DUMMY && getType() != FPRUN_DIRECTIONMARKER)
	{
		getBlock()->getSpanAttrProp(getBlockOffset(),false,&pSpanAP,&m_pRevisions,bShow,iId,bHiddenRevision);
	}
	else
	{
		getBlock()->getSpanAttrProp(getBlockOffset(),true,&pSpanAP,&m_pRevisions,bShow,iId,bHiddenRevision);
	}
	if(pSpanAP == NULL)
	{
		// FIXME for now lets work around this
		//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN); track these down later.
		//
		getBlockAP(pSpanAP);
		return;
	}
	if(bHiddenRevision)
	{
		setVisibility(FP_HIDDEN_REVISION);
	}
	else
	{
		setVisibility(FP_VISIBLE);
	}
}

void fp_Run::setX(UT_sint32 iX, bool /*bDontClearIfNeeded*/)
{
	Run_setX(iX, FP_CLEARSCREEN_AUTO);
}

// the parameter eClearScreen has a default value AUTO
// we need this extra parameter be able to specify false when calling from
// inside of the first pass of fp_Line::layout(), which sets
// only a temporary value of iX which is then adjusted in the
// second pass, without this the run will redraw twice, once always unnecessarily
// and most of the time both times unnecessarily
void	fp_Run::Run_setX(UT_sint32 iX, FPRUN_CLEAR_SCREEN eClearScreen)
{
	switch(eClearScreen)
	{
		case FP_CLEARSCREEN_AUTO:
			if (iX == m_iX)
			{
				return;
			}
			//otherwise fall through
		case FP_CLEARSCREEN_FORCE:
			m_iX = m_iOldX;
			clearScreen();
			m_iOldX = iX;
			m_iX = iX;
			break;
		case FP_CLEARSCREEN_NEVER:
			// only set m_iX and leave m_iOldX alone; this allows for
			// multiple calls to setX with the NEVER parameter without
			// intervening FORCE or AUTO
			m_iX = iX;
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

}

void fp_Run::setY(UT_sint32 iY)
{
	if (iY == m_iY)
	{
		return;
	}

	clearScreen();

	m_iY = iY;
}

void fp_Run::setLine(fp_Line* pLine)
{
	if (pLine == m_pLine)
	{
		return;
	}
	if(!getBlock()->getDocSectionLayout()->isCollapsing())
		clearScreen();

	m_pLine = pLine;
	if(pLine != NULL)
	{
		m_FillType.setParent(&pLine->getFillType());
	}
	else
	{
		m_FillType.setParent(NULL);
	}
}


/*
	In the BIDI build, changing runs next or previous can result in
	change of visual appearance of the run or a run immediately adjucent
	(say deleting last character of a word may require that the new
	last character is displayed using a final-form glyph which).
	Consequently we need to mark the width and draw buffer as dirty,
	and that is what happens when the bRefresh is true (which is the
	default value). However, the refresh is very expensive, and sometimes
	we know that it is not needed (e.g. simply merging neigbouring runs
	does not change context, merely the way we store the stuff) -- in that
	case we specify bRefresh false

*/

void fp_Run::setNextRun(fp_Run* p, bool bRefresh)
{
	if(p != m_pNext)
	{
		// change of context, need to refresh draw buffer if context sensitive
		if(bRefresh)
			orDrawBufferDirty(GRSR_ContextSensitive);
		
		//m_bRecalcWidth |= bRefresh; -- will be taken care of when
		//buffer is recalculated
#if 0
		// we do not do ligatures across run boundaries any more,
		// Tomas, Nov 15, 2003
		// because we support 2-char ligatures, the change of next
		// can also influence the run ahead of us
		// we will just mark it
		if(m_pPrev && bRefresh)
		{
			m_pPrev->markDrawBufferDirty();
			m_pPrev->markWidthDirty();
		}
#endif
		m_pNext = p;
	}
}

void fp_Run::setPrevRun(fp_Run* p, bool bRefresh)
{
	if(p != m_pPrev)
	{
		// change of context, need to refresh draw buffer if context sensitive
		if(bRefresh)
			orDrawBufferDirty(GRSR_ContextSensitive);
		
		// m_bRecalcWidth |= bRefresh;  -- will be taken care of when
		// buffer is recacluated
#if 0
		// we do not do ligatures across run boundaries any more,
		// Tomas, Nov 15, 2003
		// because we support 2-char ligatures, the change of prev
		// can also influence the run that follows us
		// we will just mark it
		if(m_pNext && bRefresh)
		{
			m_pNext->markDrawBufferDirty();
			m_pNext->markWidthDirty();
		}
#endif
		m_pPrev = p;
	}
}

bool fp_Run::isLastRunOnLine(void) const
{
	return (getLine()->getLastRun() == this);
}

bool fp_Run::isFirstRunOnLine(void) const
{
	return (getLine()->getFirstRun() == this);
}

bool fp_Run::isLastVisRunOnLine(void) const
{
	return (getLine()->getLastVisRun() == this);
}

bool fp_Run::isFirstVisRunOnLine(void) const
{
	return (getLine()->getFirstVisRun() == this);
}

void fp_Run::markAsDirty(void)
{
	xxx_UT_DEBUGMSG(("Run %x marked dirty \n"));
	m_bDirty = true;
}


/*!
 * return an rectangle that covers this object on the screen
 * The calling routine is resposible for deleting the returned struct
 */
UT_Rect * fp_Run::getScreenRect(void)
{
	UT_sint32 xoff = 0;
	UT_sint32 yoff = 0;
	UT_Rect * pRec = NULL; 
	fp_Line * pLine = getLine();
	if(pLine)
	{
		pLine->getScreenOffsets(this,xoff,yoff);
		pRec= new UT_Rect(xoff,yoff,getWidth(),getHeight());
		return pRec;
	}
	else
	{
		return NULL;
	}
}
	
/*!
 * Marks Dirty any runs that overlap the supplied rectangle. This rectangle
 * is relative to the screen.
 */
void fp_Run::markDirtyOverlappingRuns(UT_Rect & recScreen)
{
	UT_Rect * pRec = NULL;
	pRec = getScreenRect();
	if(pRec && recScreen.intersectsRect(pRec))
	 {
		 fp_Run::markAsDirty();
		 delete pRec;
		 return;
	 }
	DELETEP(pRec);
	return;
}

void fp_Run::setCleared(void)
{
	m_bIsCleared = true;
}

bool fp_Run::isOnlyRunOnLine(void) const
{
	if (getLine()->countRuns() == 1)
	{
		UT_ASSERT(isFirstRunOnLine());
		UT_ASSERT(isLastRunOnLine());

		return true;
	}

	return false;
}

void fp_Run::setLength(UT_uint32 iLen, bool bRefresh)
{
	if (iLen == getLength())
	{
		return;
	}
    m_bRecalcWidth |= bRefresh;
	if(getWidth() > 0)
		clearScreen();
	//	UT_ASSERT((getType() == FPRUN_FMTMARK) ||  (iLen > 0));
	m_iLen = iLen;

	// change of length generally means something got deleted, and
	// that affects all text in the present run, and shaping in the
	// runs adjacent
	if(bRefresh)
	{
		orDrawBufferDirty(GRSR_Unknown);

		if(m_pPrev)
		{
			m_pPrev->orDrawBufferDirty(GRSR_ContextSensitive);
		}

		if(m_pNext)
		{
			m_pNext->orDrawBufferDirty(GRSR_ContextSensitive);
		}
	}
}

void fp_Run::clearPrint(void)
{
	m_bPrinting =false;
}

void fp_Run::setBlockOffset(UT_uint32 offset)
{
	UT_DebugOnly<UT_sint32> iOff = static_cast<UT_sint32>(offset);
	UT_ASSERT(iOff >=0);
	m_iOffsetFirst = offset;
}

void fp_Run::clearScreen(void)
{
	Run_ClearScreen(false);
}

void fp_Run::Run_ClearScreen(bool bFullLineHeightRect)
{
	if(m_bPrinting)
	{
		return;
	}
	if(!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}
	markAsDirty();
	if (m_bIsCleared && !getMustClearScreen())
	{
		// no need to clear if we've already done so.
		return;
	}
	m_bMustClearScreen = false;

	if (!getLine())
	{
		// nothing to clear if this run is not currently on a line
		return;
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Doing Run_ClearScreen in run %x \n",this));
	getLine()->getFillType().setIgnoreLineLevel(true);
	if(getLine()->getContainer() != NULL)
	{
		if(getLine()->getContainer()->getPage() != 0)
		{
			UT_Rect clip(0,0,0,0);
			if(isSelectionDraw())
			{
				if(getType() == FPRUN_TEXT)
				{
					bool bRTL = (getVisDirection() == UT_BIDI_RTL);
					
					UT_sint32 xoff,yoff;
					getLine()->getScreenOffsets(this, xoff, yoff);
					UT_sint32 xLeft = xoff;
					UT_sint32 xRight = xLeft + getWidth();
					UT_sint32 x1,y1,x2,y2,height;
					bool bDir;
					if(posSelLow() > getBlock()->getPosition(true) + getBlockOffset())
					{
						findPointCoords(posSelLow() - getBlock()->getPosition(true), x1,y1,x2,y2,height,bDir);

					    if(bRTL) //rtl needs translation
						{
							xRight = x1 + _getView()->getPageViewLeftMargin();
							xRight -= _getView()->getXScrollOffset();
						}
						else
						{
							xLeft = x1 + _getView()->getPageViewLeftMargin();
							xLeft -= _getView()->getXScrollOffset();
						}
						
					}
					if(posSelHigh() < getBlock()->getPosition(true) + getBlockOffset() + getLength())
					{
						findPointCoords(posSelHigh() - getBlock()->getPosition(true) +1, x1,y1,x2,y2,height,bDir);
					    if(bRTL) //rtl needs translation
						{
							xLeft = x1 + _getView()->getPageViewLeftMargin();
							xLeft -= _getView()->getXScrollOffset();
						}
						else
						{
							xRight = x1 + _getView()->getPageViewLeftMargin();
							xRight -= _getView()->getXScrollOffset();
						}
					}
					clip.set(xLeft,yoff,xRight-xLeft,getLine()->getHeight());
					getGraphics()->setClipRect(&clip);
				}
			}
			_clearScreen(bFullLineHeightRect);
			if(isSelectionDraw())
			{
				getGraphics()->setClipRect(NULL);
			}
			// make sure we only get erased once
			_setDirty(true);
			m_bIsCleared = true;
		}
		else
		{
			xxx_UT_DEBUGMSG(("fp_Run: Clearscreen on line without page \n"));
		}
	}
	else
	{
		xxx_UT_DEBUGMSG(("fpRun: Clearscreen on line without container \n"));
	}
	fp_Line * pLine = getLine();
	if(pLine)
    {
		pLine->setNeedsRedraw();
		pLine->getFillType().setIgnoreLineLevel(false);
	}

	xxx_UT_DEBUGMSG(("fp_Run: clearscreen applied \n"));
}

static UT_RGBColor s_fgColor;

const UT_RGBColor fp_Run::getFGColor(void) const
{
	// revision colours
	FV_View * pView = _getView();
	UT_return_val_if_fail(pView, s_fgColor);
	bool bShow = pView->isShowRevisions();
	if(getBlock()->getDocLayout()->displayAnnotations())
	{
		if(getLine() && getLine()->getContainer() && getLine()->getContainer()->getContainerType() == FP_CONTAINER_ANNOTATION)
		{
				fp_AnnotationContainer * pAC = static_cast<fp_AnnotationContainer *>(getLine()->getContainer());
				UT_uint32 pid = pAC->getPID();
				s_fgColor =	_getView()->getColorAnnotation(pAC->getPage(),pid);
				return s_fgColor;
		}
	}
	if(m_pRevisions && bShow)
	{
		bool bMark = pView->isMarkRevisions();
		UT_uint32 iId = 0;
		const PP_Revision * r = NULL;
		r = m_pRevisions->getLastRevision();

		UT_return_val_if_fail(r != NULL, _getColorFG());

		bool bRevColor = false;

		if(!bMark)
		{
			// this is the case when we are in non-marking mode ...
			bRevColor = true;
		}


		UT_uint32 iShowId = pView->getRevisionLevel();

		xxx_UT_DEBUGMSG(("fp_Run Revision ID %d ShowRevision ID %d \n",iId,iShowId));
		if(bMark && iShowId == 0)
		{
			// this is the case where we are in marking mode and are
			// supposed to reveal all
			bRevColor = true;
		}
		
		if(bMark && iShowId != 0 && (iId-1 == iShowId))
		{
			// this is the case when we are in marking mode, and are
			// supposed to reveal id > iShowId
			bRevColor = true;
		}
		
		if(!bRevColor)
			return _getColorFG();

		s_fgColor = _getView()->getColorRevisions(iId-1);
	}
	else if(m_pHyperlink && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) && 
			(m_pHyperlink->getHyperlinkType() ==  HYPERLINK_NORMAL))
	{
		s_fgColor = _getView()->getColorHyperLink();
	}
	else if(m_pHyperlink && (m_pHyperlink->getHyperlinkType() == HYPERLINK_ANNOTATION))
	{
		if(getBlock()->getDocLayout()->displayAnnotations())
		{
			s_fgColor =	_getView()->getColorAnnotation(this);
		}
		else
			return _getColorFG();
	}
	else if(m_pHyperlink && (m_pHyperlink->getHyperlinkType() == HYPERLINK_RDFANCHOR))
	{
		if( getBlock()->getDocLayout()->displayRDFAnchors())
		{
			s_fgColor =	_getView()->getColorRDFAnchor(this);
		}
		else
			return _getColorFG();
	}
	//
	// FIXME make priniting author colours a preference.
	//
	else if((m_iAuthorColor > 0) && !m_bPrinting)
	{
		UT_sint32 iRange = m_iAuthorColor % 12;
		//
		// FIXME We can also set this from the color property of the author ID 
		//
		s_fgColor = _getView()->getColorRevisions(iRange);
		return s_fgColor;
	}
	else
		return _getColorFG();

	return s_fgColor;
}

void fp_Run::_setFont(const GR_Font * f)
{
	m_pFont = f;

#ifdef DEBUG
	if(f)
		m_iFontAllocNo = f->getAllocNumber();
#endif
}

const GR_Font * fp_Run::_getFont(void) const
{
#ifdef DEBUG
	if(m_pFont)
	{
		// if this assert fails we are in deep trouble; basically, the font pointer points
		// to some other font that it was when it was set
		UT_ASSERT_HARMLESS( m_iFontAllocNo ==  m_pFont->getAllocNumber());
	}
#endif
	
	return m_pFont;
}

void fp_Run::_setDirty(bool b)
{
	xxx_UT_DEBUGMSG(("fp_Run:: run %x  Dirty state set to %d \n",this,b));
	m_bDirty = b;
}

void fp_Run::draw(dg_DrawArgs* pDA)
{
	if (pDA->bDirtyRunsOnly && !m_bDirty)
	{
		if(!getLine())
			return;
		if(!getLine()->getPage())
				return;
		if(!getLine()->getPage()->intersectsDamagedRect(this))
		{
			xxx_UT_DEBUGMSG(("fp_Run::Run %x not dirty returning \n",this));
			return;
		}
		m_bDirty = true;
	}
	const UT_Rect * pPrevClip = pDA->pG->getClipRect();
	if(isHidden())
	{
		// this run is marked as hidden, nothing to do
		return;
	}
	m_bIsCleared = false;
	fp_Line * pLine = getLine();
	if (pLine)
		pLine->setScreenCleared(false);

	//UT_usleep(100000); // 0.1 seconds useful for debugging
	xxx_UT_DEBUGMSG(("SEVIOR: draw Run this %x line %x \n",this, getLine()));
	GR_Graphics * pG = pDA->pG;
	// shortcircuit drawing if we're way off base.
	long imax = (1 << 30) - 1; // This draws four rows of pages. Increase the 30 to view more rows.
	if (((pDA->yoff < -imax) || (pDA->yoff > imax)) && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	     return;

	if(pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		m_bPrinting = true;
		lookupProperties(pG);
	}
	pG->setColor(getFGColor());
	UT_Rect clip(0,0,0,0);
	bool bSetClip = false;
	if(isSelectionDraw())
	{
		if((getType() == FPRUN_TEXT) && getLine())
		{
			bool bRTL = (getVisDirection() == UT_BIDI_RTL);
			UT_sint32 xLeft = pDA->xoff;
			UT_sint32 xRight = xLeft + getWidth();
			UT_sint32 x1,y1,x2,y2,height;
			bool bDir;
			FL_DocLayout * pLayout = getBlock()->getDocLayout();
			UT_uint32 iPageNumber = pLayout->findPage(pLine->getPage());
			UT_uint32 widthPrevPageInRow = _getView()->getWidthPrevPagesInRow(iPageNumber);
			if(posSelLow() > getBlock()->getPosition(true) + getBlockOffset())
			{
				findPointCoords(posSelLow() - getBlock()->getPosition(true), x1,y1,x2,y2,height,bDir);
				x1 += widthPrevPageInRow;
				x2 += widthPrevPageInRow;
				if(bRTL)
				{
					xRight = x1 + _getView()->getPageViewLeftMargin();
					xRight -= _getView()->getXScrollOffset();
				}
				else
				{
					xLeft = x1 + _getView()->getPageViewLeftMargin();
					xLeft -= _getView()->getXScrollOffset();
				}
			}
			if(posSelHigh() < getBlock()->getPosition(true) + getBlockOffset() + getLength())
			{
				findPointCoords(posSelHigh() - getBlock()->getPosition(true) +1, x1,y1,x2,y2,height,bDir);
				x1 += widthPrevPageInRow;
				x2 += widthPrevPageInRow;
				if(bRTL)
				{
					xLeft = x1 + _getView()->getPageViewLeftMargin();
					xLeft -= _getView()->getXScrollOffset();
				}
				else
				{
					xRight = x1 + _getView()->getPageViewLeftMargin();
					xRight -= _getView()->getXScrollOffset();
				}
			}
			UT_sint32 width = xRight-xLeft;
			clip.set(xLeft,pDA->yoff-getLine()->getAscent(),width,getLine()->getHeight());
			bSetClip = true;
			pDA->pG->setClipRect(&clip);
			xxx_UT_DEBUGMSG(("draw cliprect left %d width %d \n",clip.left,clip.width));
		}
	}
	UT_RGBColor OldCol = *m_FillType.getColor();
	UT_RGBColor bgCol = _getColorHL();
	xxx_UT_DEBUGMSG((" bg red %d rg blue %d bg green %d  trans %d \n",bgCol.m_red,bgCol.m_blu,bgCol.m_grn,bgCol.m_bIsTransparent));
	if(!bgCol.isTransparent())
	{
		m_FillType.setColor(bgCol); 
	}
	xxx_UT_DEBUGMSG(("Drawing pDA->yoff %d \n",pDA->yoff));
	_draw(pDA);
	if(!bgCol.isTransparent())
	{
		m_FillType.setColor(OldCol); // restore the old clear color
	}
	if(bSetClip)
	{
		pDA->pG->setClipRect(pPrevClip);
	}
	FV_View* pView = _getView();
	UT_return_if_fail(pView);
	bool bShowRevs = pView->isShowRevisions();

	UT_uint32 i2Du = pDA->pG->tlu(1); // changed this to 1 to fix various pixel dirt
	
	if(m_pRevisions && bShowRevs)
	{
		GR_Painter painter(pG);
		const PP_Revision * r = m_pRevisions->getLastRevision();
		UT_ASSERT(r != NULL);

		if (r) {
			PP_RevisionType r_type = r->getType();
			UT_uint32 iId = r->getId();
			UT_uint32 iShowId = pView->getRevisionLevel();
			bool bMark = pView->isMarkRevisions();
			
			if(bMark && iShowId != 0)
				iId--;
			
			if(!bMark || !iShowId || iId == iShowId)
				{
					pG->setColor(getFGColor());
					
					UT_uint32 iWidth = getDrawingWidth();
					
					if(r_type == PP_REVISION_ADDITION || r_type == PP_REVISION_ADDITION_AND_FMT)
						{
							painter.fillRect(s_fgColor,pDA->xoff, pDA->yoff + i2Du, iWidth, getGraphics()->tlu(1));
							painter.fillRect(s_fgColor,pDA->xoff, pDA->yoff + i2Du + getGraphics()->tlu(2),
											 iWidth, getGraphics()->tlu(1));
							
						}
					else if(r_type == PP_REVISION_FMT_CHANGE)
						{
							// draw a thick line underneath
							painter.fillRect(s_fgColor,pDA->xoff, pDA->yoff + i2Du, iWidth, getGraphics()->tlu(2));
						}
					else
						{
							// draw a strike-through line
							
							painter.fillRect(s_fgColor,pDA->xoff, pDA->yoff - m_iHeight/3,
											 iWidth, getGraphics()->tlu(2));
						}
				}
		}
	}

	if(m_pHyperlink && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// have to set the colour again, since fp_TextRun::_draw can set it to red
		// for drawing sguiggles
		if(m_pHyperlink->getHyperlinkType() == HYPERLINK_NORMAL)
		{
				GR_Painter painter(pG);
				pG->setColor(_getView()->getColorHyperLink());
				pG->setLineProperties(pG->tluD(1.0),
								GR_Graphics::JOIN_MITER,
								GR_Graphics::CAP_PROJECTING,
								GR_Graphics::LINE_SOLID);

				painter.drawLine(pDA->xoff, pDA->yoff + i2Du, pDA->xoff + m_iWidth, pDA->yoff + i2Du);
		}
		else
		{
            bool display = false;
            GR_Painter painter(pG);
            switch(m_pHyperlink->getHyperlinkType())
            {
                case HYPERLINK_NORMAL:
                    break;
                case HYPERLINK_RDFANCHOR:
                    if( displayRDFAnchors() )
                    {
                        display = true;
                        pG->setColor(_getView()->getColorRDFAnchor(this));
                    }
                    break;
                case HYPERLINK_ANNOTATION:
                    if(displayAnnotations() || pG->queryProperties(GR_Graphics::DGP_SCREEN))
                    {
                        display = true;
                        pG->setColor(_getView()->getColorAnnotation(this));
                    }
                    break;
            }
            
			if( display )
			{
					pG->setLineProperties(pG->tluD(1.0),
										  GR_Graphics::JOIN_MITER,
										  GR_Graphics::CAP_PROJECTING,
										  GR_Graphics::LINE_ON_OFF_DASH);
				
					painter.drawLine(pDA->xoff, pDA->yoff + i2Du, pDA->xoff + m_iWidth, pDA->yoff + i2Du);
					pG->setLineProperties(pG->tluD(1.0),
										  GR_Graphics::JOIN_MITER,
										  GR_Graphics::CAP_PROJECTING,
										  GR_Graphics::LINE_SOLID);
			}	
		}
	}

	if(m_eVisibility == FP_HIDDEN_TEXT || m_eVisibility == FP_HIDDEN_REVISION_AND_TEXT)
	{
		GR_Painter painter(pG);
		pG->setColor(getFGColor());
		pG->setLineProperties(pG->tluD(1.0),
								GR_Graphics::JOIN_MITER,
								GR_Graphics::CAP_PROJECTING,
								GR_Graphics::LINE_DOTTED);

		painter.drawLine(pDA->xoff, pDA->yoff + i2Du, pDA->xoff + m_iWidth, pDA->yoff + i2Du);

	}
	m_bIsCleared = false;
	_setDirty(false);
	if(pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		m_bPrinting = false;
		//
		// The font will be reset by refreshRunProperties()
		// After the print has finished.
		//
		_setFont(NULL);
		lookupProperties(NULL);
	}
}

/*!
    Determines if run is currently visible or hidden
	run is hidden in the following circumstances:
	
	 a) it is formatted as hidden and show para is off
	 
	 b) it is part of a revision that makes it hidden; several cases
	    fall into this category, but that is immaterial here (the
	    decision on this is made in lookupProperties()
bool fp_Run::isHidden() const
*/

/*!
    Determines if run would be hidden if its visibility was set to the
    given value eVisibility
*/
bool fp_Run::_wouldBeHidden(FPVisibility eVisibility) const
{
	FV_View* pView = _getView();
	bool bShowHidden = pView->getShowPara();

	bool bHidden = ((eVisibility == FP_HIDDEN_TEXT && !bShowHidden)
	              || eVisibility == FP_HIDDEN_REVISION
		          || eVisibility == FP_HIDDEN_REVISION_AND_TEXT);

	return bHidden;
}

/*!
    changes the visibility of the present run; this requires
    invalidating different things depending on circumstances
*/
void fp_Run::setVisibility(FPVisibility eVis)
{
	if(m_eVisibility == eVis)
		return;

	if(eVis == FP_HIDDEN_TEXT && !_wouldBeHidden(eVis) && m_iWidth == 0)
	{
		// we are asked to mark text as hidden, and under the present settings it would be
		// visible, but its width == 0. While the 0 width could be legitimate, it is more
		// likely that the run was previously hidden due to ShowPara == false and now is
		// being shown -- we need to make sure everything gets updated
		// (this case avoids the logic below, because of the way that lookupProperties()
		// works -- it might be worth redesigning that
		m_bIsCleared = true;
		m_bDirty = true;
		m_bRecalcWidth = true;
		m_eVisibility = eVis;
		return;
	}
	
	if(    (isHidden() && _wouldBeHidden(eVis))
	    || (!isHidden() && !_wouldBeHidden(eVis)))
	{
		// this run will remain as it is, so we just set visibility to
		// the new value
		m_eVisibility = eVis;
		return;
	}

	if(_wouldBeHidden(eVis))
	{
		// we are going into hiding, so we need to clear screen first
		clearScreen();

		// we do not need to redraw
		m_bDirty = false;
		m_bRecalcWidth = true;
		m_eVisibility = eVis;
		return;
	}

	// we are unhiding: need to mark everything dirty
	m_bIsCleared = true;
	m_bDirty = true;
	m_bRecalcWidth = true;
	m_eVisibility = eVis;

	/* recalculate width immediately so that any calls to getWidth() are
	 * accurate
	 */
	_recalcWidth();
	
	return;
}

bool fp_Run::deleteFollowingIfAtInsPoint() const
{
	if(isHidden())
		return true;
	else
		return _deleteFollowingIfAtInsPoint();
}

bool fp_Run::_deleteFollowingIfAtInsPoint() const
{
	return false;
}


bool fp_Run::canContainPoint(void) const
{
	if(isHidden())
		return false;
	else
		return _canContainPoint();
}

bool fp_Run::_canContainPoint(void) const
{
	return true;
}

bool fp_Run::letPointPass(void) const
{
	if(isHidden())
			return true;
	else
		return _letPointPass();
}

bool fp_Run::_letPointPass(void) const
{
	return true;
}

bool fp_Run::recalcWidth(void)
{
	if(isHidden())
	{
		if(m_iWidth != 0)
		{
			m_iWidth = 0;
			return true;
		}
		
		return false;
	}
	else
		return _recalcWidth();
}

/*!
    update ascent, decent and height
*/
void fp_Run::updateVerticalMetric()
{
	if(m_pFont)
	{
		_setAscent(getGraphics()->getFontAscent(m_pFont));
		_setDescent(getGraphics()->getFontDescent(m_pFont));
		_setHeight(getGraphics()->getFontHeight(m_pFont));
	}
}


bool fp_Run::_recalcWidth(void)
{
	// do nothing.  subclasses may override this.
	return false;
}

void fp_Run::drawDecors(UT_sint32 xoff, UT_sint32 yoff, GR_Graphics * pG)
{
	xxx_UT_DEBUGMSG(("drawDecors xoff %d \n",xoff));
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/

     /*
	    Here is the code to work out the position and thickness of under
        and overlines for a run of text. This is neccessary because an
        underline or overline could shift position depending on the
        text size - particularly for subscripts and superscripts. We
        can't work out where to put the lines until the end of the
        lined span. This info is saved in the fp_Run class. If a
        underline or overline is pending (because the next run
        continues the underline or overline), mark the next run as
        dirty to make sure it is drawn.
     */

	if((_getDecorations() & (TEXT_DECOR_UNDERLINE | TEXT_DECOR_OVERLINE |
			TEXT_DECOR_LINETHROUGH | TEXT_DECOR_TOPLINE | TEXT_DECOR_BOTTOMLINE)) == 0)
	{
		return;
	}

	GR_Painter painter(pG);
	const UT_sint32 old_LineWidth = m_iLineWidth;
	UT_sint32 cur_linewidth = pG->tlu(1) + UT_MAX(pG->tlu(10),static_cast<UT_sint32>(getAscent())-pG->tlu(10))/8;
//
// Line thickness is too thick.
//
	cur_linewidth = UT_MAX(pG->tlu(1),cur_linewidth/2);
	UT_sint32 iDrop = 0;

	// need to do this in the visual space
	fp_Run* P_Run = getPrevVisual();
	fp_Run* N_Run = getNextVisual();

	const bool b_Underline = isUnderline();
	const bool b_Overline = isOverline();
	const bool b_Strikethrough = isStrikethrough();
	const bool b_Topline = isTopline();
	const bool b_Bottomline = isBottomline();

	// again, need to do this in visual space
	const bool b_Firstrun = (P_Run == NULL) || (getLine()->getFirstVisRun()== this);
	const bool b_Lastrun = (N_Run == NULL) || (getLine()->getLastVisRun()== this);

	/* If the previous run is NULL or if this is the first run of a
	   line, we are on the first run of the line so set the linethickness,
	   start of the line span and the overline and underline positions from
	   the current measurements.
	*/
	if(P_Run == NULL || b_Firstrun )
	{
		setLinethickness(cur_linewidth);
		if(b_Underline)
		{
			iDrop = yoff + getAscent() + getDescent()/3 + pG->tlu(1);
			xxx_UT_DEBUGMSG(("underline getAscent() %d getDescent() %d tlu 1 %d \n",getAscent(),getDescent(), pG->tlu(1)));
			setUnderlineXoff( xoff);
			setMaxUnderline(iDrop);
		}
		if(b_Overline)
		{
			iDrop = yoff + pG->tlu(1) + UT_MAX(pG->tlu(10),static_cast<UT_sint32>(getAscent()) - pG->tlu(10))/8;
			setOverlineXoff( xoff);
			setMinOverline(iDrop);
		}
	}
	/*
	  Otherwise look to see if the previous run had an underline or
overline. If it does, merge the information with the present information. Take
the Maximum of the underline offsets and the minimum of the overline offsets.
Always take the maximum of the linewidths. If there is no previous underline
or overline set the underline and overline locations with the current data.
	*/
	else
	{
		if (!P_Run->isUnderline() && !P_Run->isOverline() &&
			!P_Run->isStrikethrough())
		{
			setLinethickness(cur_linewidth);
		}
		else
		{
			setLinethickness(UT_MAX(P_Run->getLinethickness(),cur_linewidth));
		}
 	      if (b_Underline)
	      {
			  iDrop = yoff + getAscent() + getDescent()/3;
			  xxx_UT_DEBUGMSG(("underline getAscent() %d getDescent() %d m_iDescent  %d \n",getAscent(),getDescent(),m_iDescent));
			  if(!P_Run->isUnderline())
			  {
				  setUnderlineXoff( xoff);
				  setMaxUnderline(iDrop);
			  }
			  else
			  {
				  setUnderlineXoff( P_Run->getUnderlineXoff());
				  setMaxUnderline(UT_MAX( P_Run->getMaxUnderline(), iDrop));
			  }
		}
 	      if (b_Overline)
	      {
		     iDrop = yoff + pG->tlu(1) + UT_MAX(pG->tlu(10),static_cast<UT_sint32>(getAscent()) - pG->tlu(10))/8;
		     if(!P_Run->isOverline())
		     {
				 setOverlineXoff( xoff);
				 setMinOverline(iDrop);
		     }
		     else
		     {
				 setOverlineXoff( P_Run->getOverlineXoff());
				 setMinOverline(UT_MIN( P_Run->getMinOverline(), iDrop));
		     }
		  }
	}
	m_iLineWidth = getLinethickness();
	pG->setLineWidth(m_iLineWidth);
	/*
	  If the next run returns NULL or if we are on the last run
 we've reached the of the line of text so the overlines and underlines must
be drawn.
	*/
	if(N_Run == NULL  || b_Lastrun)
	{
		if ( b_Underline)
		{
			iDrop = UT_MAX( getMaxUnderline(), iDrop);
			UT_sint32 totx = getUnderlineXoff();
			painter.drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
		if ( b_Overline)
		{
			iDrop = UT_MIN( getMinOverline(), iDrop);
			UT_sint32 totx = getOverlineXoff();
			painter.drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
	}
	/*
	   Otherwise look to see if the next run has an underline or overline
if not, draw the line, if does mark the next run as dirty to make sure it
is drawn later.
	*/
	else
	{
		if ( b_Underline )
		{
		     if(!N_Run->isUnderline() || isSelectionDraw())
		     {
				 iDrop = UT_MAX( getMaxUnderline(), iDrop);
				 UT_sint32 totx = getUnderlineXoff();
				 xxx_UT_DEBUGMSG(("Underlining y-logical %d \n",iDrop));
				 painter.drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		     }
		     else
		     {
		          N_Run->markAsDirty();
		     }
		}
		if ( b_Overline )
		{
			if(!N_Run->isOverline() || isSelectionDraw())
			{
				iDrop = UT_MIN( getMinOverline(), iDrop);
				UT_sint32 totx = getOverlineXoff();
				painter.drawLine(totx, iDrop, xoff+getWidth(), iDrop);
			}
			else
			{
				N_Run->markAsDirty();
			}
		}
	}
	/*
	  We always want strikethrough to go right through the middle of the
text so we can keep the original code.
	*/
	if ( b_Strikethrough)
	{
		iDrop = yoff + getAscent() * 2 / 3;
		painter.drawLine(xoff, iDrop, xoff+getWidth(), iDrop);
	}
	/*
	   Restore the previous line width.
	*/
	_setLineWidth(old_LineWidth);
	pG->setLineWidth(_getLineWidth());
	if(!b_Topline && !b_Bottomline)
		return;
	/*
	  We always draw Topline right at the top of the line so there is no ambiguity
	*/
	UT_sint32 ithick = getToplineThickness();

	UT_RGBColor clrFG;
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	PD_Document * pDoc = getBlock()->getDocument();
	
	getSpanAP(pSpanAP);
	getBlockAP(pBlockAP);
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);

	// This gives the baseline of the selection.
	// need to clear full height of line, in case we had a selection
//	UT_sint32 xxoff=0 ,ybase =0;
//	getLine()->getScreenOffsets(this, xxoff, ybase);


	if ( b_Topline)
	{
		UT_sint32 ybase = yoff + getAscent() - getLine()->getAscent() + pG->tlu(1);
		painter.fillRect(clrFG, xoff, ybase, getWidth(), ithick);
	}
	/*
	  We always draw bottomline right at the bottom so there is no ambiguity
	*/
	if ( b_Bottomline)
	{
		painter.fillRect(clrFG, xoff, yoff+getLine()->getHeight()-ithick+pG->tlu(1), getWidth(), ithick);
	}
}

inline bool fp_Run::isUnderline(void) const
{
	return ((m_fDecorations & TEXT_DECOR_UNDERLINE) !=  0);
}


inline bool fp_Run::isOverline(void) const
{
	return ((m_fDecorations & TEXT_DECOR_OVERLINE) !=  0);
}

inline bool fp_Run::isStrikethrough(void) const
{
	return ((m_fDecorations & TEXT_DECOR_LINETHROUGH) !=  0);
}


inline bool fp_Run::isTopline(void) const
{
	return ((m_fDecorations & TEXT_DECOR_TOPLINE) !=  0);
}


inline bool fp_Run::isBottomline(void) const
{
	return ((m_fDecorations & TEXT_DECOR_BOTTOMLINE) !=  0);
}

void fp_Run::setLinethickness(UT_sint32 max_linethickness)
{
	m_iLinethickness = max_linethickness;
}

void fp_Run::setUnderlineXoff(UT_sint32 xoff)
{
	m_iUnderlineXoff = xoff;
}

UT_sint32 fp_Run::getUnderlineXoff(void) const
{
	return m_iUnderlineXoff;
}

void fp_Run::setOverlineXoff(UT_sint32 xoff)
{
	m_iOverlineXoff = xoff;
}

UT_sint32 fp_Run::getOverlineXoff(void) const
{
	return m_iOverlineXoff;
}

void fp_Run::setMaxUnderline(UT_sint32 maxh)
{
	m_imaxUnderline = maxh;
}

UT_sint32 fp_Run::getMaxUnderline(void) const
{
	return m_imaxUnderline;
}

void fp_Run::setMinOverline(UT_sint32 minh)
{
	m_iminOverline = minh;
}

UT_sint32 fp_Run::getMinOverline(void) const
{
	return m_iminOverline;
}

UT_sint32 fp_Run::getLinethickness( void) const
{
	return m_iLinethickness;
}

UT_sint32 fp_Run::getToplineThickness(void) const
{
	return UT_convertToLogicalUnits("0.8pt");
}

/*!
 * This draws a line with some text in the center.
 * \param xOff the x offset of the left end of the line
 * \param yOff the y offset of the top (maybe bottom, I don't know) of
 * the line
 * \param iWidth the desired length of the line.
 * \param iHeight the desired height of the line.  Note that the line
 * will almost certainly take up more vertical space than this, since
 * the text will be taller than the line.
 * \param pText the text to be displayed in the middle of the line
 * \bug Currently, this does not detect whether it is on the screen or
 * not, so it redraws way too often.
*/
void fp_Run::_drawTextLine(UT_sint32 xoff,UT_sint32 yoff,UT_uint32 iWidth,UT_uint32 iHeight,UT_UCSChar *pText)
{
    GR_Font *pFont = getGraphics()->getGUIFont();

	GR_Painter painter(getGraphics());
    getGraphics()->setFont(pFont);

    UT_uint32 iTextLen = UT_UCS4_strlen(pText);
    UT_uint32 iTextWidth = getGraphics()->measureString(pText,0,iTextLen,NULL);
    UT_uint32 iTextHeight = getGraphics()->getFontHeight(pFont);

    UT_uint32 xoffText = xoff + (iWidth - iTextWidth) / 2;
    UT_uint32 yoffText = yoff - getGraphics()->getFontAscent(pFont) * 2 / 3;

    painter.drawLine(xoff,yoff,xoff + iWidth,yoff);

    if((iTextWidth < iWidth) && (iTextHeight < iHeight))
	{
		Fill(getGraphics(),xoffText,yoffText,iTextWidth,iTextHeight);
        painter.drawChars(pText,0,iTextLen,xoffText,yoffText);
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_TabRun::fp_TabRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) 
	: fp_Run(pBL, iOffsetFirst, iLen, FPRUN_TAB),
	  m_leader(FL_LEADER_NONE),
	  m_TabType(FL_TAB_NONE),
	  m_bIsTOC(false),	  
	  m_bIsTOCListLabel(false)
{
	lookupProperties();
}

void fp_TabRun::_lookupProperties(const PP_AttrProp * pSpanAP,
								  const PP_AttrProp * pBlockAP,
								  const PP_AttrProp * pSectionAP,
								  GR_Graphics * pG)
{
	bool bChanged = false;

	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	UT_RGBColor clrFG;
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true), clrFG);
	bChanged |= (clrFG != _getColorFG());
	_setColorFG(clrFG);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());

	if (pFont != _getFont())
	{
	    _setFont(pFont);
	    _setAscent(pG->getFontAscent(pFont));
	    _setDescent(pG->getFontDescent(pFont));
	    _setHeight(pG->getFontHeight(pFont));
		bChanged = true;
	}

	if(getDirection() != UT_BIDI_WS)
	{
		_setDirection(UT_BIDI_WS);
		bChanged = true;
		//setDirectionProperty(UT_BIDI_WS);
	}
//
// Lookup Decoration properties for this run
//
	const gchar *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  getBlock()->getDocument(), true);
	_setLineWidth(getToplineThickness());

	UT_uint32 oldDecors = _getDecorations();
	_setDecorations(0);
	gchar* p;
	if (!(p = g_strdup(pszDecor)))
	{
		// TODO outofmem
	}
	UT_ASSERT(p || !pszDecor);
	gchar*	q = strtok(p, " ");

	while (q)
	{
		if (0 == strcmp(q, "underline"))
		{
			_orDecorations(TEXT_DECOR_UNDERLINE);
		}
		else if (0 == strcmp(q, "overline"))
		{
			_orDecorations(TEXT_DECOR_OVERLINE);
		}
		else if (0 == strcmp(q, "line-through"))
		{
			_orDecorations(TEXT_DECOR_LINETHROUGH);
		}
		else if (0 == strcmp(q, "topline"))
		{
			_orDecorations(TEXT_DECOR_TOPLINE);
		}
		else if (0 == strcmp(q, "bottomline"))
		{
			_orDecorations(TEXT_DECOR_BOTTOMLINE);
		}
		q = strtok(NULL, " ");
	}
	g_free(p);

	bChanged |= (oldDecors != _getDecorations());

	if(bChanged)
		clearScreen();

}

bool fp_TabRun::canBreakAfter(void) const
{
	return false;
}

bool fp_TabRun::canBreakBefore(void) const
{
	return false;
}

bool fp_TabRun::_letPointPass(void) const
{
	return true;
}

bool fp_TabRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_TabRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: Find everything that calls this and modify them to allow y-axis.
	
	
	// If X is left of the middle, return offset to the left,
	// otherwise the offset to the right.
	if (x < (getWidth() / 2))
		pos = getBlock()->getPosition() + getBlockOffset();
	else
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();

	bBOL = false;
	bEOL = false;
}

void fp_TabRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: TabRun\n"));
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_sint32 xoff2;
	UT_sint32 yoff2;

	UT_ASSERT(getLine());
	if(getLine()->getY() == -2000000)
	{
		//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN); // might need this later
	}
	getLine()->getOffsets(this, xoff, yoff);

	fp_Run * pRun = 0;
	UT_sint32 iNextDir = getVisDirection();

	if (iOffset == (getBlockOffset() + getLength()))  //#TF is this the right-most logical element of the run?
	{
	    pRun = getNextRun();
	    if(pRun)
	    {
	        pRun->getLine()->getOffsets(pRun, xoff2, yoff2);
	        iNextDir = pRun->getVisDirection();
	    }
	}

	UT_sint32 iDirection = getVisDirection();

    x = xoff;

	if(iDirection == UT_BIDI_LTR)
	{
		xxx_UT_DEBUGMSG(("iOffset %d, getBlockOffset() %d, getLength() %d\n", iOffset,getBlockOffset(),getLength()));
		if(iOffset != getBlockOffset())
		{
			// this happens when a tab run is last run in a block and the eop's run
			//findPointCoords() is called ...
			//UT_ASSERT(iOffset == (getBlockOffset() + getLength()));
			x += getWidth();
		}
	}
	else
	{
		if(iOffset == getBlockOffset())
		{
		    x += getWidth();
		}
	}


	if(pRun && (iNextDir != iDirection)) //if this run precedes run of different direction, we have to split the caret
	{
	    x2 = (iNextDir == UT_BIDI_LTR) ?  xoff + pRun->getWidth() : xoff2;
	    y2 = yoff2;
	}
	else
	{
	    x2 = x;
	    y2 = yoff;
	}

	bDirection = (iDirection != UT_BIDI_LTR);
	y = yoff;
	height = getHeight();
}

bool fp_TabRun::isTOCTab(void)
{
	return m_bIsTOC;
}


void fp_TabRun::setTOCTabListLabel(void)
{
	m_bIsTOCListLabel = true;
	_setLength(0);
	m_leader = FL_LEADER_NONE;
    m_TabType =	FL_TAB_LEFT;	
}

void fp_TabRun::setTabWidth(UT_sint32 iWidth)
{
	clearScreen();
	fp_Run::_setWidth(iWidth);
}

void fp_TabRun::setLeader(eTabLeader iLeader)
{
	clearScreen();
	m_leader = iLeader;
}

eTabLeader fp_TabRun::getLeader(void)
{
	return m_leader;
}


void fp_TabRun::setTabType(eTabType iTabType)
{
	m_TabType = iTabType;
}

eTabType fp_TabRun::getTabType(void) const
{
	return m_TabType;
}


void fp_TabRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	Fill(getGraphics(),xoff, yoff, getWidth(), getLine()->getHeight());
}

void fp_TabRun::_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 /*iHeight*/)
{
    if (!(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

#define NPOINTS 6

    UT_Point points[NPOINTS];

    UT_sint32 cur_linewidth = getGraphics()->tlu(1) + UT_MAX(getGraphics()->tlu(10),static_cast<UT_sint32>(getAscent()) - getGraphics()->tlu(10)) / 8;
    UT_uint32 iyAxis = iTop + getLine()->getAscent() * 2 / 3;
    UT_uint32 iMaxWidth = UT_MIN(iWidth / 10 * 6, static_cast<UT_uint32>(cur_linewidth) * 9);
    UT_uint32 ixGap = (iWidth - iMaxWidth) / 2;

	//UT_DEBUGMSG(("iLeft %d, iWidth %d, visDir \"%s\"\n", iLeft,iWidth, getVisDirection() == UT_BIDI_LTR ? "ltr":"rtl"));
	if(getVisDirection() == UT_BIDI_LTR)
	{
	    points[0].x = iLeft + ixGap + iMaxWidth - cur_linewidth * 4;
    	points[0].y = iyAxis - cur_linewidth * 2;

	    points[1].x = points[0].x + cur_linewidth;
	    points[1].y = points[0].y;

	    points[2].x = iLeft + iWidth - ixGap;
	    points[2].y = iyAxis;
	}
	else
	{
		//iLeftAdj -= getWidth();

	    points[0].x = iLeft + ixGap + cur_linewidth * 4;
    	points[0].y = iyAxis - cur_linewidth * 2;

	    points[1].x = points[0].x - cur_linewidth;
	    points[1].y = points[0].y;

	    points[2].x = iLeft + ixGap;
	    points[2].y = iyAxis;

	}

    points[3].x = points[1].x;
    points[3].y = iyAxis + cur_linewidth * 2;

    points[4].x = points[0].x;
    points[4].y = points[3].y;

    points[5].x = points[0].x;
    points[5].y = points[0].y;

	GR_Painter painter(getGraphics());

    UT_RGBColor clrShowPara(_getView()->getColorShowPara());
    painter.polygon(clrShowPara,points,NPOINTS);

    xxx_UT_DEBUGMSG(("fp_TabRun::_drawArrow: iLeft %d, iyAxis %d, cur_linewidth %d, iMaxWidth %d\n",
    			iLeft, iyAxis, cur_linewidth, iMaxWidth));

    // only draw the rectangle if iMaxWidth - cur_linewidth * 4 > 0, otherwise
    // we get the rect running pass the end of the line and off the screen
    if(static_cast<UT_sint32>(iMaxWidth - cur_linewidth * 4) > 0) 
    {
        if(getVisDirection() == UT_BIDI_LTR)
        {
            painter.fillRect(clrShowPara,iLeft + ixGap,iyAxis - cur_linewidth / 2,iMaxWidth - cur_linewidth * 4,cur_linewidth);
        }
        else
        {
            painter.fillRect(clrShowPara,iLeft + ixGap + cur_linewidth * 4,iyAxis - cur_linewidth / 2,iMaxWidth - cur_linewidth * 4,cur_linewidth);
        }
    }
#undef NPOINTS
}

void fp_TabRun::_draw(dg_DrawArgs* pDA)
{
	xxx_UT_DEBUGMSG(("fp_TabRun::_draw (0x%x)\n",this));
	GR_Graphics * pG = pDA->pG;

	// need to draw to the full height of line to join with line above.
	UT_sint32 xoff= 0, yoff=0, DA_xoff = pDA->xoff;

	getLine()->getScreenOffsets(this, xoff, yoff);

	// need to clear full height of line, in case we had a selection

	UT_sint32 iFillHeight2 = getLine()->getHeight();
	UT_sint32 iFillTop = pDA->yoff - getLine()->getAscent();

	xxx_UT_DEBUGMSG(("iFillTop Tab %d YTopOfRun %d \n",iFillTop, pDA->yoff - getAscent()));
	FV_View* pView = _getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	UT_uint32 iRunBase = getBlock()->getPosition() + getOffsetFirstVis(); //getBlockOffset();

	UT_RGBColor clrFG;
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	PD_Document * pDoc = getBlock()->getDocument();

	getSpanAP(pSpanAP);
	getBlockAP(pBlockAP);
	
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);
	
	GR_Painter painter(pG);

	if (getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) && (isInSelectedTOC() ||
	    /* pView->getFocus()!=AV_FOCUS_NONE && */
		 ((iSel1 <= iRunBase)	&& (iSel2 > iRunBase)))		)
	{
		painter.fillRect(_getView()->getColorSelBackground(), /*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight2);
        if(pView->getShowPara()){
            _drawArrow(/*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight2);
        }
	}
	else
	{
		Fill(pG,DA_xoff, iFillTop, getWidth(), iFillHeight2);
        if(pView->getShowPara()){
            _drawArrow(/*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight2);
        }
	}
	if (m_leader != FL_LEADER_NONE)
	{
		UT_UCSChar tmp[151];
		UT_GrowBufElement wid[151];
		int i, cumWidth;

		tmp[0] = 150;
		switch (m_leader)
		{
		case FL_LEADER_DOT:
			tmp[1] = '.';
			break;
		case FL_LEADER_HYPHEN:
			tmp[1] = '-';
			break;
		case FL_LEADER_UNDERLINE:
			tmp[1] = '_';
			break;
		default:
			tmp[1] = ' ';
			break;
		}

		for (i = 2; i < 151; i++)
			tmp[i] = tmp[1];

		
		pG->setFont(_getFont());
		pG->measureString(tmp, 1, 150, wid);
		// one would think that one could measure
		// one character and divide the needed
		// width by that; would one be so wrong?
		// we're not dealing with different letters
		// here, after all.

		i = 1;
		cumWidth = 0;
		FL_DocLayout * pLayout = getBlock()->getDocLayout();
		UT_sint32 iTabTop = pDA->yoff - getAscent();
		if(pG && pLayout->isQuickPrint() && pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
				iTabTop = pDA->yoff - pG->getFontAscent(_getFont());
		}
		while (cumWidth < getWidth() && i < 151)
		{
			cumWidth += wid[i++];
		}
		i = (i>=3) ? i - 2 : 1;
		pG->setColor(clrFG);
		painter.drawChars(tmp, 1, i, /*pDA->xoff*/DA_xoff, iTabTop,wid);
	}
//
// Draw underline/overline/strikethough
//
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove
	                                                 //character dirt
	drawDecors( xoff, yTopOfRun,pG);
//
// Draw bar seperators
//
	if(FL_TAB_BAR == getTabType())
	{
		// need to draw to the full height of line to join with line above.
		UT_sint32 iFillHeight = getLine()->getHeight();
//
// Scale the vertical line thickness for printers
//
		UT_sint32 ithick = getToplineThickness();
		painter.fillRect(clrFG, /*pDA->xoff*/DA_xoff+getWidth()-ithick, iFillTop, ithick, iFillHeight);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedLineBreakRun::fp_ForcedLineBreakRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FORCEDLINEBREAK)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun constructor\n"));
	lookupProperties();
}

void fp_ForcedLineBreakRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun::lookupProperties\n"));
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	_inheritProperties();
	FV_View* pView = _getView();
	if (pView && pView->getShowPara())
	{
	  //UT_UCSChar pEOP[] = { UCS_LINESEP, 0 }; - see bug 1279
	  UT_UCSChar pEOP[] = { '^', 'l', 0 };
	  UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);

		fp_Run* pPropRun = _findPrevPropertyRun();
		if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
			pG->setFont(pTextRun->getFont());
		}
		else
		{
			// look for fonts in this DocLayout's font cache
			FL_DocLayout * pLayout = getBlock()->getDocLayout();

			const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());
			getGraphics()->setFont(pFont);
		}
		_setWidth(getGraphics()->measureString(pEOP, 0, iTextLen, NULL));
		xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties: width %d\n", getWidth()));
	}
	else
	{
		_setWidth(16);
	}
}

bool fp_ForcedLineBreakRun::canBreakAfter(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_ForcedLineBreakRun::canBreakBefore(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_ForcedLineBreakRun::_letPointPass(void) const
{
	return false;
}

void fp_ForcedLineBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: Find everything that calls this and modify them to allow x-axis and y-axis.
	
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun::mapXYToPosition\n"));
	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_ForcedLineBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	// this assert is wrong -- a run can be asked to return coords for position it does
	// not contain if the run which contains it cannot contain point
	// UT_ASSERT(getBlockOffset() == iOffset || getBlockOffset()+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		if(FPRUN_TEXT == pPropRun->getType())
		{
			pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = getHeight();
			getLine()->getOffsets(this, xoff, yoff);
			x = xoff;
			y = yoff;
		}
	}
	else
	{
		height = getHeight();
		getLine()->getOffsets(this, xoff, yoff);
		x = xoff;
		y = yoff;
	}

	if (iOffset == getBlockOffset()+1)
	{
	    FV_View* pView = _getView();
	    if (pView && pView->getShowPara())
		{
			x += getWidth();
	    }
	}

	x2 = x;
	y2 = y;
	//UT_DEBUGMSG(("fintPointCoords: ForcedLineBreakRun: x=%d, y=%d, h=%d\n", x,y,height));
}

void fp_ForcedLineBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedLineBreakRun::_draw(dg_DrawArgs* pDA)
{

	UT_sint32 iXoffText = 0;
	UT_sint32 iYoffText = 0;

	FV_View* pView = _getView();
	if(!pView || !pView->getShowPara())
    {
    	if(getWidth())
		{
			_setWidth(0);
		}
    	return;
    }

	GR_Painter painter(getGraphics());

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (/* pView->getFocus()!=AV_FOCUS_NONE && */ isInSelectedTOC() ||	
		((iSel1 <= iRunBase) && (iSel2 > iRunBase)))
		bIsSelected = true;

	UT_RGBColor clrShowPara(pView->getColorShowPara());

	//UT_UCSChar pEOP[] = { UCS_LINESEP, 0 };
	UT_UCSChar pEOP[] = { '^', 'l', 0 };
	UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);
	UT_sint32 iAscent;

	fp_Run* pPropRun = _findPrevPropertyRun();
	if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
    {
		fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
		getGraphics()->setFont(pTextRun->getFont());
		iAscent = pTextRun->getAscent();
    }
	else
    {
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;

		getSpanAP(pSpanAP);
		getBlockAP(pBlockAP);
		
		// look for fonts in this DocLayout's font cache
		FL_DocLayout * pLayout = getBlock()->getDocLayout();

		const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());
		getGraphics()->setFont(pFont);
		iAscent = getGraphics()->getFontAscent();
    }

	// if we currently have a 0 width, i.e., we draw in response to the
	// showPara being turned on, then we obtain the new width, and then
	// tell the line to redo its layout and redraw instead of drawing ourselves
	//	bool bWidthChange = false;
	//	if(!getWidth())
	//		bWidthChange = true;

	_setWidth(getGraphics()->measureString(pEOP, 0, iTextLen, NULL));
	// 	if(bWidthChange)
	//	{
	//		getLine()->layout();
	//		getLine()->redrawUpdate();
	//		return;
	//	}

	_setHeight(getGraphics()->getFontHeight());
	iXoffText = pDA->xoff;

	if(getBlock()->getDominantDirection() == UT_BIDI_RTL)
    {
		iXoffText -= getWidth();
    }

	iYoffText = pDA->yoff - iAscent;
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::draw: width %d\n", getWidth()));

	if (bIsSelected)
    {
		painter.fillRect(_getView()->getColorSelBackground(), iXoffText, iYoffText, getWidth(), getLine()->getHeight());
    }
	else
    {
		Fill(getGraphics(),iXoffText, iYoffText, getWidth(), getLine()->getHeight());
    }
	if (pView->getShowPara())
    {
		// Draw pilcrow
		getGraphics()->setColor(clrShowPara);
		painter.drawChars(pEOP, 0, iTextLen, iXoffText, iYoffText);
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldStartRun::fp_FieldStartRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FIELDSTARTRUN)
{
	lookupProperties();
}

void fp_FieldStartRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
										 const PP_AttrProp * /*pBlockAP*/,
										 const PP_AttrProp * /*pSectionAP*/,
										 GR_Graphics *)
{
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);
	_setWidth(0);
}

bool fp_FieldStartRun::canBreakAfter(void) const
{
	return true;
}

bool fp_FieldStartRun::canBreakBefore(void) const
{
	return true;
}

bool fp_FieldStartRun::_letPointPass(void) const
{
	return true;
}

void fp_FieldStartRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}


void fp_FieldStartRun::findPointCoords(UT_uint32 /*iOffset*/, UT_sint32& /*x*/, UT_sint32& /*y*/, UT_sint32& /*x2*/, UT_sint32& /*y2*/, UT_sint32& /*height*/, bool& /*bDirection*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldStartRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldStartRun::_draw(dg_DrawArgs* /*pDA*/)
{

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldEndRun::fp_FieldEndRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FIELDENDRUN)
{
	lookupProperties();
}

void fp_FieldEndRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/,
									   GR_Graphics *)
{
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);
	_setWidth(0);
}

bool fp_FieldEndRun::canBreakAfter(void) const
{
	return true;
}

bool fp_FieldEndRun::canBreakBefore(void) const
{
	return true;
}

bool fp_FieldEndRun::_letPointPass(void) const
{
	return true;
}

void fp_FieldEndRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_FieldEndRun::findPointCoords(UT_uint32 /*iOffset*/, UT_sint32& /*x*/, UT_sint32& /*y*/, UT_sint32& /*x2*/, UT_sint32& /*y2*/, UT_sint32& /*height*/, bool& /*bDirection*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldEndRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldEndRun::_draw(dg_DrawArgs* /*pDA*/)
{

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



fp_BookmarkRun::fp_BookmarkRun( fl_BlockLayout* pBL,
								UT_uint32 iOffsetFirst,
								UT_uint32 iLen)
	: fp_Run(pBL, iOffsetFirst, iLen, FPRUN_BOOKMARK)
{
	m_pBookmark = getBlock()->getBookmark(iOffsetFirst);
	UT_return_if_fail(m_pBookmark);

	_setDirty(true);

	UT_ASSERT((pBL));
	_setDirection(UT_BIDI_WS);

	m_bIsStart = (po_Bookmark::POBOOKMARK_START == m_pBookmark->getBookmarkType());

	// have to cache the name, since we will need to use it for a while
	// after the associated PT fragment has been deleted.
	strncpy(m_pName, m_pBookmark->getName(), BOOKMARK_NAME_SIZE);
	m_pName[BOOKMARK_NAME_SIZE] = 0;

	_setWidth(0);
	_setRecalcWidth(false);
}

bool fp_BookmarkRun::isComrade(fp_BookmarkRun *pBR) const
{
	UT_ASSERT(*m_pName && *pBR->m_pName);
	return (0 == strcmp(m_pName, pBR->m_pName));
}

void fp_BookmarkRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/,
									   GR_Graphics *)
{
}

bool fp_BookmarkRun::canBreakAfter(void) const
{
	return true;
}

bool fp_BookmarkRun::canBreakBefore(void) const
{
	return true;
}

bool fp_BookmarkRun::_letPointPass(void) const
{
	return true;
}

bool fp_BookmarkRun::_canContainPoint(void) const
{
	return false;
}

bool fp_BookmarkRun::_deleteFollowingIfAtInsPoint() const
{
	return true;
}

/*!
    When working with bookmarks, the run block offset does not always adequately represent the
    location of the bookmark. For example, if the user bookmarks the same place in the doc with
    several bookmarks, the run offsets for each associated run will be different, but most of the
    time we are interested in the offset to the left or right of all stacked up bookmarks. Similarly,
    a bookmark that is immediately after a start of block needs to be treated in certain situations
    as if the block strux was also sellected. This function implements the necessary logic.

    In general, when we jump to bookmarks, we go to the range in between the two bookmark
    object. However, for purposes of TOCs, we are interested in the range that is just outside the
    two objects.

    \parameter bAfter: indicates if we want offset to the right (true) or left (false) of the
                       bookmark

    \return: the return value is document offset of the bookmarked position
*/
UT_uint32 fp_BookmarkRun::getBookmarkedDocPosition(bool bAfter) const
{
	if(bAfter)
	{
		fp_Run * pRun = getNextRun();
		const fp_Run * pPrevRun = this;
		
		while(pRun)
		{
			switch (pRun->getType())
			{
				case FPRUN_BOOKMARK:
				case FPRUN_FMTMARK:
					pPrevRun = pRun;
					pRun = pRun->getNextRun();
					break;

				default:
					return getBlock()->getPosition(false) + pRun->getBlockOffset();
			}
		}

		UT_ASSERT_HARMLESS( !pRun );
		return getBlock()->getPosition(false) + pPrevRun->getBlockOffset() + pPrevRun->getLength();
	}
	else
	{
		fp_Run * pRun = getPrevRun();

		while(pRun)
		{
			switch (pRun->getType())
			{
				case FPRUN_BOOKMARK:
				case FPRUN_FMTMARK:
					pRun = pRun->getPrevRun();
					break;

				default:
					return getBlock()->getPosition(false) + pRun->getBlockOffset() + pRun->getLength();
			}
		}

		UT_ASSERT_HARMLESS( !pRun );
		return getBlock()->getPosition(true); // offset of the block strux
	}
}

void fp_BookmarkRun::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC)
{
	fp_Run *pRun = getNextRun();
	UT_ASSERT(pRun);
	pRun->mapXYToPosition(x, y, pos, bBOL, bEOL,isTOC);
}

void fp_BookmarkRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y,  UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	fp_Run * pRun = getNextRun();
	UT_ASSERT(pRun);

	pRun->findPointCoords(iOffset, x, y,  x2, y2, height, bDirection);
}

void fp_BookmarkRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

   	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return;
    }


	UT_sint32 xoff = 0, yoff = 0;
	getLine()->getScreenOffsets(this, xoff, yoff);

	if(m_bIsStart)
		Fill(getGraphics(), xoff, yoff, 4, 8);
	else
		Fill(getGraphics(),xoff - 4, yoff, 4, 8);

}

void fp_BookmarkRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics * pG = pDA->pG;
    if (!(pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

   	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return;
    }

	pG->setColor(_getView()->getColorShowPara());

	#define NPOINTS 4

    UT_Point points[NPOINTS];

	points[0].y = pDA->yoff;


   	if(m_bIsStart)
   	{
	    points[0].x = pDA->xoff - 4;
		points[1].x = pDA->xoff;
   	}
	else
	{
	    points[0].x = pDA->xoff;
		points[1].x = points[0].x - 4;
	}

    points[1].y = points[0].y + 4;

	points[2].x = points[0].x;
	points[2].y = points[0].y + 8;

    points[3].x = points[0].x;
    points[3].y = points[0].y;

    UT_RGBColor clrShowPara(_getView()->getColorShowPara());

	GR_Painter painter(pG);
    painter.polygon(clrShowPara,points,NPOINTS);
    #undef NPOINTS

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_HyperlinkRun::fp_HyperlinkRun( fl_BlockLayout* pBL,
								  UT_uint32 iOffsetFirst,
								UT_uint32 /*iLen*/)
	: fp_Run(pBL, iOffsetFirst, 1, FPRUN_HYPERLINK)
    , m_bIsStart(false)
    , m_pTarget(NULL)
    , m_pTitle(NULL)
{
	_setLength(1);
	_setDirty(false);
	_setWidth(0);
	_setRecalcWidth(false);
	
	UT_ASSERT((pBL));
	_setDirection(UT_BIDI_WS);

	_setTargetFromAPAttribute( "xlink:href");
	_setTitleFromAPAttribute( "xlink:title");
}


fp_HyperlinkRun::~fp_HyperlinkRun()
{
	DELETEPV(m_pTarget);
	DELETEPV(m_pTitle);
}

void fp_HyperlinkRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/,
									   GR_Graphics *)
{
}

bool fp_HyperlinkRun::canBreakAfter(void) const
{
	return false;
}

bool fp_HyperlinkRun::canBreakBefore(void) const
{
	return true;
}

bool fp_HyperlinkRun::_letPointPass(void) const
{
	return true;
}

bool fp_HyperlinkRun::_canContainPoint(void) const
{
	return false;
}

bool fp_HyperlinkRun::_deleteFollowingIfAtInsPoint() const
{
	return true;
}

void fp_HyperlinkRun::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC)
{
	fp_Run *pRun = getNextRun();
	UT_ASSERT(pRun);
	pRun->mapXYToPosition(x, y, pos, bBOL, bEOL,isTOC);
}

void fp_HyperlinkRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y,  UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	fp_Run * pRun = getNextRun();
	UT_ASSERT(pRun);

	pRun->findPointCoords(iOffset, x, y,  x2, y2, height, bDirection);
}

void fp_HyperlinkRun::_clearScreen(bool /* bFullLineHeightRect */)
{
}

void fp_HyperlinkRun::_draw(dg_DrawArgs* /*pDA*/)
{
}

void fp_HyperlinkRun::_setTargetFromAPAttribute( const gchar* pAttrName )
{
	const PP_AttrProp * pAP = NULL;

	getSpanAP(pAP);
	
	const gchar * pTarget;
	const gchar * pName;
	bool bFound = false;
	UT_uint32 k = 0;

	while(pAP->getNthAttribute(k++, pName, pTarget))
	{
		bFound = (0 == g_ascii_strncasecmp(pName,pAttrName,strlen(pAttrName)));
		if(bFound)
			break;
	}

	// we have got to keep a local copy, since the pointer we get
	// is to a potentially volatile location
	if(bFound)
	{
		_setTarget( pTarget );
		m_bIsStart = true;
		//if this is a start of the hyperlink, we set m_pHyperlink to this,
		//so that when a run gets inserted after this one, its m_pHyperlink is
		//set correctly
		_setHyperlink(this);
	}
	else
	{
		m_bIsStart = false;
		m_pTarget = NULL;
		_setHyperlink(NULL);
	}
}

void fp_HyperlinkRun::_setTitleFromAPAttribute( const gchar* pAttrName )
{
	const PP_AttrProp * pAP = NULL;
	getSpanAP(pAP);
	
	const gchar *pTitle;
	if (pAP->getAttribute(pAttrName, pTitle))
	{
	    _setTitle(pTitle);
	} else
	{
	    m_pTitle = NULL;
	}
}

void fp_HyperlinkRun::_setTarget( const gchar * pTarget )
{
    DELETEPV(m_pTarget);
    UT_uint32 iTargetLen = strlen(pTarget);
    m_pTarget = new gchar [iTargetLen + 1];
    strncpy(m_pTarget, pTarget, iTargetLen + 1);
}

void fp_HyperlinkRun::_setTitle( const gchar * pTitle )
{
    DELETEPV(m_pTitle);
    UT_uint32 iTitleLen = strlen(pTitle);
    m_pTitle = new gchar [iTitleLen + 1];
    strncpy(m_pTitle, pTitle, iTitleLen + 1);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
fp_EndOfParagraphRun::fp_EndOfParagraphRun(fl_BlockLayout* pBL,
										   UT_uint32 iOffsetFirst,
										   UT_uint32 iLen)
	: fp_Run(pBL, iOffsetFirst, iLen, FPRUN_ENDOFPARAGRAPH)
{

	_setLength(1);
	_setDirty(true);
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::created this %x block %x \n",this,getBlock()));

	UT_ASSERT((pBL));
	_setDirection(pBL->getDominantDirection());
	lookupProperties();
}


bool fp_EndOfParagraphRun::_recalcWidth(void)
{
	return false;
}

void fp_EndOfParagraphRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP,
											 GR_Graphics * pG)
{
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties this %x block %x \n",this,getBlock()));
	_inheritProperties();
	xxx_UT_DEBUGMSG(("After Inherit props Height is %d \n",getHeight()));
	const gchar* pRevision = NULL;

	if(pBlockAP && pBlockAP->getAttribute("revision", pRevision))
	{
		// we will not in fact be doing anything with the actual
		// properties and attributes contained in the revision
		// we just need its representation so the base class can
		// handle us properly
		PP_RevisionAttr * pRev = getRevisions();
		DELETEP(pRev);

		_setRevisions(new PP_RevisionAttr(pRevision));
	}

	FV_View* pView = _getView();
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	if (pView && pView->getShowPara())
	{
		// Find width of Pilcrow
		UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
		UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);

		fp_Run* pPropRun = _findPrevPropertyRun();
		if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
			pG->setFont(pTextRun->getFont());
		}
		else
		{
			// look for fonts in this DocLayout's font cache
			FL_DocLayout * pLayout = getBlock()->getDocLayout();

			const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());
			pG->setFont(pFont);
		}
		m_iDrawWidth  = pG->measureString(pEOP, 0, iTextLen, NULL);
		xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties: width %d\n", getWidth()));
	}
	else
	{
		// FIXME:jskov This should probably be the width of the
		// document to the right of the pilcrow, see Paul's suggested
		// selection behaviors. Doesn't matter until we get selection
		// support though (which requires PT changes).

		// I have changed this to 0, because otherwise it figures in
		// calculation of line width, and the last line in righ-aligned
		// paragraphs is shifted by the width of the pilcrow.
		// this required some additional changes to the _draw function
		// Tomas
		m_iDrawWidth = 0;
	}
}

bool fp_EndOfParagraphRun::canBreakAfter(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_EndOfParagraphRun::canBreakBefore(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_EndOfParagraphRun::_letPointPass(void) const
{
	return false;
}

void fp_EndOfParagraphRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: Find everything that calls this and modify them to allow y-axis. (I think?)
	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = true;
}

void fp_EndOfParagraphRun::findPointCoords(UT_uint32 iOffset,
										   UT_sint32& x, UT_sint32& y,
										   UT_sint32& x2, UT_sint32& y2,
										   UT_sint32& height,
										   bool& bDirection)
{
	// FIXME:jskov Find out why we are sometimes asked to find pos at
	// right of pilcrow. Should never ever happen... But does.
	// fjsdkjfklsd<forced-column-break>sdfsdsd move cursor back
	//	UT_ASSERT(getBlockOffset() == iOffset);

	fp_Run* pPropRun = _findPrevPropertyRun();

	height = getHeight();
	if(pPropRun && pPropRun->getType() == FPRUN_IMAGE)
	{
		height = static_cast<fp_ImageRun *>(pPropRun)->getPointHeight();
	}
	xxx_UT_DEBUGMSG((" Got initial height of %d \n",height));
	if (pPropRun)
	{
		xxx_UT_DEBUGMSG(("Got propRun in EOPRun \n"));
		height = pPropRun->getHeight();
		if(pPropRun->getType() == FPRUN_IMAGE)
	    {
			height = static_cast<fp_ImageRun *>(pPropRun)->getPointHeight();
		}

		// If property Run is on the same line, get y location from
		// it (to reflect proper ascent).
		if (pPropRun->getLine() == getLine())
		{
			pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
			xxx_UT_DEBUGMSG(("Got propRun in EOPRun inherited height %d \n",height));
			if(pPropRun->getType() == FPRUN_IMAGE)
			{
				height = static_cast<fp_ImageRun *>(pPropRun)->getPointHeight();
			}
			return;
		}
	}
	xxx_UT_DEBUGMSG((" Got final height of %d \n",height));

	getLine()->getOffsets(this, x, y);
	x2 = x;
	y2 = y;
}

void fp_EndOfParagraphRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
	if(m_iDrawWidth == 0 )
	{
		return;
	}
	UT_sint32 xoff = 0, yoff = 0;
	getLine()->getScreenOffsets(this, xoff, yoff);

	if(getBlock()->getDominantDirection() == UT_BIDI_RTL)
	{
		xoff -= m_iDrawWidth;
	}
	Fill(getGraphics(),xoff, yoff+1, m_iDrawWidth, getLine()->getHeight()+1);
}

/*!
  Draw end-of-paragraph Run graphical representation
  \param pDA Draw arguments
  Draws the pilcrow character (reverse P) in show paragraphs mode.
  \fixme Make it use the same typeface as preceding text.
  \note This _draw function is special in that it does (partly) lookup
  as well. That's because the pilcrow's typeface is controlled by the
  preceding character. Eventually, when the PT learns about EOP, it
  should be possible to just deal with this in the lookup function.
*/
void fp_EndOfParagraphRun::_draw(dg_DrawArgs* pDA)
{
	// if showPara is turned off we will not draw anything at all; however,
	// we will ensure that the width is set to 0, and if it is currently not
	// we will get our line to redo its layout and redraw.
	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	if(m_iDrawWidth)
    	{
    		m_iDrawWidth = 0;
    		//getLine()->layout();
    		//getLine()->redrawUpdate();
    	}
    	return;
    }

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (/* pView->getFocus()!=AV_FOCUS_NONE && */isInSelectedTOC() || 
		((iSel1 <= iRunBase) && (iSel2 > iRunBase)))
		bIsSelected = true;

	GR_Painter painter(getGraphics());

	UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
	UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);
	UT_sint32 iAscent;

	fp_Run* pPropRun = _findPrevPropertyRun();
	if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
	{
		fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
		getGraphics()->setFont(pTextRun->getFont());
		iAscent = pTextRun->getAscent();
	}
	else
	{
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;

		getSpanAP(pSpanAP);
		getBlockAP(pBlockAP);

		// look for fonts in this DocLayout's font cache
		FL_DocLayout * pLayout = getBlock()->getDocLayout();

		const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());
		getGraphics()->setFont(pFont);
		iAscent = getGraphics()->getFontAscent();
	}

	// if we currently have a 0 width, i.e., we draw in response to the
	// showPara being turned on, then we obtain the new width, and then
	// tell the line to redo its layout and redraw instead of drawing ourselves
//	bool bWidthChange = false;
//	if(!m_iDrawWidth)
//		bWidthChange = true;

	m_iDrawWidth  = getGraphics()->measureString(pEOP, 0, iTextLen, NULL);
// 	if(bWidthChange)
//	{
//		getLine()->layout();
//		getLine()->redrawUpdate();
//		return;
//	}

	_setHeight(getGraphics()->getFontHeight());
	m_iXoffText = pDA->xoff;

	if(getBlock()->getDominantDirection() == UT_BIDI_RTL)
	{
		m_iXoffText -= m_iDrawWidth;
	}

	m_iYoffText = pDA->yoff - iAscent;
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::draw: width %d\n", m_iDrawWidth));

	if (bIsSelected)
	{
		painter.fillRect(_getView()->getColorSelBackground(), m_iXoffText, m_iYoffText, m_iDrawWidth, getLine()->getHeight());
	}
	else
	{
		Fill(getGraphics(),m_iXoffText, m_iYoffText, m_iDrawWidth, getLine()->getHeight());
	}
	if (getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) && pView->getShowPara())
	{
		// Draw pilcrow
		// use the hard-coded colour only if not revised
		if(!getRevisions() || !pView->isShowRevisions())
			getGraphics()->setColor(pView->getColorShowPara());
        painter.drawChars(pEOP, 0, iTextLen, m_iXoffText, m_iYoffText);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


fp_ImageRun::fp_ImageRun(fl_BlockLayout* pBL,
						 UT_uint32 iOffsetFirst,
						 UT_uint32 iLen, FG_GraphicPtr && pFG,
						 pf_Frag_Object* oh) :
	fp_Run(pBL, iOffsetFirst, iLen, FPRUN_IMAGE),
	m_pFGraphic(std::move(pFG)),
	m_iPointHeight(0),
	m_pSpanAP(NULL),
	m_bImageForPrinter (false),
	m_OH(oh)
{
#if 0	// put this back later
	UT_ASSERT(pImage);
#endif

	m_pImage = m_pFGraphic->generateImage(getGraphics(), NULL, 0, 0);
	m_sCachedWidthProp = m_pFGraphic->getWidthProp();
	m_sCachedHeightProp = m_pFGraphic->getHeightProp();
	m_iGraphicTick = pBL->getDocLayout()->getGraphicTick();
	lookupProperties();
}

fp_ImageRun::~fp_ImageRun()
{
	DELETEP(m_pImage);
}

void fp_ImageRun::regenerateImage(GR_Graphics * pG)
{
	DELETEP(m_pImage);
	m_pImage = m_pFGraphic->regenerateImage(pG);
	m_bImageForPrinter = pG->queryProperties(GR_Graphics::DGP_PAPER);
	m_iGraphicTick = getBlock()->getDocLayout()->getGraphicTick();

}

void fp_ImageRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * /*pBlockAP*/,
									const PP_AttrProp * /*pSectionAP*/,
									GR_Graphics * pG)
{
	fd_Field * fd = NULL;
	UT_return_if_fail(pSpanAP);
	m_pSpanAP = pSpanAP;
	getBlock()->getField(getBlockOffset(), fd);
	_setField(fd);
	const gchar * szWidth = NULL;
	pSpanAP->getProperty("width", szWidth);
	if(szWidth == NULL)
	{
		szWidth = "0in";
	}
	const gchar * szHeight = NULL;
	pSpanAP->getProperty("height", szHeight);
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	if(szHeight == NULL)
	{
		szHeight = "0in";
	}
	UT_DEBUGMSG(("Orig szHeight = %s \n",szHeight));
	// Also get max width, height ready for generateImage.

	fl_DocSectionLayout * pDSL = getBlock()->getDocSectionLayout();
	UT_sint32 maxW = static_cast<UT_sint32>(static_cast<double>(pDSL->getActualColumnWidth()));
	UT_sint32 maxH = static_cast<UT_sint32>(static_cast<double>(pDSL->getActualColumnHeight()));
	fl_ContainerLayout * pCL = getBlock()->myContainingLayout();
	if(pCL && pCL->getContainerType() == FL_CONTAINER_FRAME)
	{
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pCL);
		maxW = pFL->getFrameWidth();
		maxH = pFL->getFrameHeight();
		if(getLine())
		{
			maxH -= getLine()->getY(); // take Y height into account.
		}
	}
	else if (pCL && pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		//
		// Don't shrink images to fit cells. Cells should expand to fit images
		// This is a compromize that makes tables sane for insanely large
		// images. The user will have to adjust images size manually
		//
		maxW = static_cast<UT_sint32>(static_cast<double>(maxW));
		maxH = static_cast<UT_sint32>(static_cast<double>(maxH));
	}
	if(pG->tdu(maxW) < 3)
	{
		maxW = pG->tlu(3);
	}
	if(pG->tdu(maxH) < 3)
	{
		maxH = pG->tlu(3);
	}
	UT_DEBUGMSG(("Image szWidth %s Image szHeight %s \n",szWidth,szHeight));
	if((pG->queryProperties(GR_Graphics::DGP_PAPER) != m_bImageForPrinter) ||
		(strcmp(m_sCachedWidthProp.c_str(),szWidth) != 0) ||
	   (strcmp(m_sCachedHeightProp.c_str(),szHeight) != 0) ||
		UT_convertToLogicalUnits(szHeight) > maxH ||
		UT_convertToLogicalUnits(szWidth) > maxW)
	{
		m_sCachedWidthProp = szWidth;
		m_sCachedHeightProp = szHeight;
		DELETEP(m_pImage);
		UT_sint32 iH = UT_convertToLogicalUnits(szHeight);
		UT_sint32 iW =  UT_convertToLogicalUnits(szWidth);
		if((iW < maxW) && (iW > 30))
		{
			maxW = iW;
			UT_DEBUGMSG(("Change Image Width to %d \n",maxW));
		}
		if((iH < maxH) && (iH > 30))
		{
			maxH = iH;
			UT_DEBUGMSG(("Change Image Height to %d \n",maxH));
		}
		m_pImage = m_pFGraphic->generateImage(pG, pSpanAP, maxW, maxH);
		if(m_pImage)
		{
			iW = pG->tlu(m_pImage->getDisplayWidth());
			iH = pG->tlu(m_pImage->getDisplayHeight());
			if(iW < maxW)
				maxW = iW;
			if(iH < maxH)
				maxH = iH;
		}
		m_sCachedWidthProp = UT_formatDimensionString(DIM_IN,static_cast<double>(maxW)/UT_LAYOUT_RESOLUTION);
		m_sCachedHeightProp = UT_formatDimensionString(DIM_IN,static_cast<double>(maxH)/UT_LAYOUT_RESOLUTION);
		PP_PropertyVector props = {
			"width", m_sCachedWidthProp.c_str(),
			"height", m_sCachedHeightProp.c_str()
		};
		if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			//
			// Change the properties in the document
			//
			getBlock()->getDocument()->changeObjectFormatNoUpdate(PTC_AddFmt, m_OH, PP_NOPROPS, props);
			//
			// update the span Attribute/Propperties with this
			//
			PT_AttrPropIndex api = getBlock()->getDocument()->getAPIFromSOH(m_OH);
			getBlock()->getDocument()->getAttrProp(api,&m_pSpanAP);
		}
		m_bImageForPrinter = pG->queryProperties(GR_Graphics::DGP_PAPER);
		markAsDirty();
		if(getLine())
		{
			getLine()->setNeedsRedraw();
		}
	}
	if (m_pImage)
	{
		_setWidth(pG->tlu(m_pImage->getDisplayWidth()));
		_setHeight(pG->tlu(m_pImage->getDisplayHeight()));
	}
	else
	{
		// If we have no image, we simply insert a square "slug"

		_setWidth(UT_convertToLogicalUnits("0.5in"));
		_setHeight(UT_convertToLogicalUnits("0.5in"));
	}

	// these asserts are no longer valid -- image can be hidden due to
	//hidden text mark up or revisions
	//UT_ASSERT(getWidth() > 0);
	//UT_ASSERT(getHeight() > 0);
	m_iImageWidth = getWidth();
	m_iImageHeight = getHeight();

	_setAscent(_getHeight());
	_setDescent(0);
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getBlockAP(pBlockAP);

	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());

	if (pFont != _getFont())
	{
		_setFont(pFont);
	}
	m_iPointHeight = pG->getFontAscent(pFont) + getGraphics()->getFontDescent(pFont);
}

bool fp_ImageRun::canBreakAfter(void) const
{
	return true;
}

bool fp_ImageRun::canBreakBefore(void) const
{
	return true;
}

bool fp_ImageRun::_letPointPass(void) const
{
	return false;
}

bool fp_ImageRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_ImageRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: This one needs fixing for multipage. Get text working first.
	//TODO: Find everything that calls this and modify them to allow y-axis.
	if (x > getWidth())
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();
	else
		pos = getBlock()->getPosition() + getBlockOffset();

	bBOL = false;
	bEOL = false;
}

void fp_ImageRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ImmageRun\n"));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(getLine());

	getLine()->getOffsets(this, xoff, yoff);
	if (iOffset == (getBlockOffset() + getLength()))
	{
		x = xoff + getWidth();
		x2 = x;
	}
	else
	{
		x = xoff;
		x2 = x;
	}
	y = yoff + getHeight() - m_iPointHeight;
	height = m_iPointHeight;
	y2 = y;
	bDirection = (getVisDirection() != UT_BIDI_LTR);
}

void fp_ImageRun::_clearScreen(bool /*bFullLineHeightRect*/ )
{
	//	UT_ASSERT(!isDirty());

	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	Fill(getGraphics(),xoff, yoff, getWidth(), iLineHeight);
	markAsDirty();
	setCleared();
}

const char * fp_ImageRun::getDataId(void) const
{
	return m_pFGraphic->getDataId();
}

void fp_ImageRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics *pG = pDA->pG;
	if(getBlock()->getDocLayout()->getGraphicTick() != m_iGraphicTick)
	{
		regenerateImage(pG);
	}
	else if(!pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		regenerateImage(pG);
		m_iGraphicTick = getBlock()->getDocLayout()->getGraphicTick()+999;
	}
	UT_sint32 xoff = 0, yoff = 0;

	if(pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{	
		getLine()->getScreenOffsets(this, xoff, yoff);
	}
	else
	{
		getLine()->getOffsets(this, xoff, yoff);
		if(_getView()->getViewMode() != VIEW_PRINT)
		{
			yoff += static_cast<fl_DocSectionLayout *>(getBlock()->getDocSectionLayout())->getTopMargin();
		}
	}

	yoff += getLine()->getAscent() - getAscent() + 1;
	// clip drawing to the page
	UT_Rect pClipRect;
	pClipRect.top = yoff;
	pClipRect.left = xoff;
	pClipRect.height = getLine()->getContainer()->getHeight();
	pClipRect.width = getLine()->getContainer()->getWidth();
    pClipRect.height -= getLine()->getY();
	//
	// SEVIOR Says don't touch this if statement unless you know how to make windows
	// and gnome-print print images. Otherwise your commit priviliges will be revoked.
	//
	const UT_Rect * pSavedRect = NULL;
	if(pG->getClipRect())
	{
		pSavedRect = pG->getClipRect();
	}
	if(pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		//
		// Take the interesction of the applied rectangle;
		if(pSavedRect != NULL)
		{
			UT_sint32 iTop,iLeft,iWidth,iHeight;
			iTop = 0;
			iLeft = 0;
			iWidth = 0;
			iHeight = 0;
			iTop =  pClipRect.top;
			if(pSavedRect->top > pClipRect.top)
			{
				iTop = pSavedRect->top;
			}
			UT_sint32 iBot = pClipRect.top + pClipRect.height;
			if((pSavedRect->top + pSavedRect->height) < (pClipRect.top + pClipRect.height))
			{
				iBot = pSavedRect->top + pSavedRect->height;
			}
			iHeight = iBot - iTop;
			if(iHeight < pG->tlu(1))
			{
				iHeight = pG->tlu(2);
			}
			iLeft = pClipRect.left;
			if(pSavedRect->left  > pClipRect.left)
			{
				iLeft = pSavedRect->left;
			}
			UT_sint32 iRight = pClipRect.left + pClipRect.width;
			if((pSavedRect->left + pSavedRect->width) < (pClipRect.left + pClipRect.width))
			{
				iRight = pSavedRect->left + pSavedRect->width;
			}
			iWidth = iRight - iLeft;
			if(iWidth < pG->tlu(1))
			{
				iWidth = pG->tlu(2);
			}
			pClipRect.left = iLeft;
			pClipRect.width = iWidth;
			pClipRect.top = iTop;
			pClipRect.height = iHeight;
			pG->setClipRect(&pClipRect);
		}
		
	}

	FV_View* pView = _getView();

	GR_Painter painter(pG);

	if (m_pImage)
	{
		// Paint the background if there is alpha in the image
		if(pG->queryProperties(GR_Graphics::DGP_SCREEN) && m_pImage->hasAlpha())
		{
			Fill(pG,xoff,yoff,getWidth(),getHeight());
		}
		// draw the image (always)
		painter.drawImage(m_pImage, xoff, yoff);

		// if we're the selection, draw some pretty selection markers
		if (pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

			UT_uint32 iSelAnchor = pView->getSelectionAnchor();
			UT_uint32 iPoint = pView->getPoint();

			UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
			UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

			UT_ASSERT(iSel1 <= iSel2);

			if (
			    /* pView->getFocus()!=AV_FOCUS_NONE && */
				(iSel1 <= iRunBase)
				&& (iSel2 > iRunBase)
				)
			{				
				UT_uint32 top = yoff;
				UT_uint32 left = xoff;
				UT_uint32 right = xoff + getWidth() - pG->tlu(1);
				UT_uint32 bottom = yoff + getHeight() - pG->tlu(1);

				UT_Rect box(left, top, right - left, bottom - top);
				pView->drawSelectionBox(box, true);
			}
		}

	}
	else
	{
		painter.fillRect(pView->getColorImage(), xoff, yoff, getWidth(), getHeight());
	}

	// unf*ck clipping rect
	pG->setClipRect(pSavedRect);
}

GR_Image * fp_ImageRun::getImage()
{
	return m_pImage;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define  _FIELD(type,desc,tag)  /*nothing*/
#define  _FIELDTYPE(type,desc)  {FPFIELDTYPE_##type, NULL, desc},

fp_FieldTypeData fp_FieldTypes[] = {

#include "fp_Fields.h"

	{FPFIELDTYPE_END, NULL, 0} };

#undef  _FIELD
#undef  _FIELDTYPE

// The way to turn macro argument into string constant
#define xstr2(x) #x
#define xstr(x) xstr2(x)
#define _FIELD(type,desc,tag)  {FPFIELDTYPE_##type, FPFIELD_##tag, NULL, xstr(tag), desc},
#define _FIELDTYPE(type,desc)  /*nothing*/

fp_FieldData fp_FieldFmts[] = {

#include "fp_Fields.h"

	{FPFIELDTYPE_END, FPFIELD_end, NULL, NULL, 0} };

#undef  xstr2
#undef  xstr
#undef  _FIELD
#undef  _FIELDTYPE

fp_FieldRun::fp_FieldRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen)
	:	fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FIELD),
		m_iFieldType(FPFIELD_start),
		m_pParameter(0),
		m_fPosition(TEXT_POSITION_NORMAL)
{
	fd_Field * fd;
	lookupProperties();
	if(!getBlock()->isContainedByTOC())
	{
		bool gotField = pBL->getField(iOffsetFirst,fd);
		if(gotField)
		{
			_setField(fd);
		}
	}
	//	UT_ASSERT(gotField);
	m_sFieldValue[0] = 0;
}

fp_FieldRun::~fp_FieldRun(void)
{
	xxx_UT_DEBUGMSG(("FieldRun deleted %x FieldType %d \n",this,getFieldType()));
	return;
}

bool fp_FieldRun::_recalcWidth()
{
	// TODO -- is this really needed ???
	// this should not be needed, since lookup properties is called
	// when formatting changes - Tomas
	//lookupProperties();

	getGraphics()->setFont(_getFont());
	UT_sint32 iNewWidth = 0;
	if(UT_UCS4_strlen(m_sFieldValue) > 0)
	{
		iNewWidth = getGraphics()->measureString(m_sFieldValue,
									 0,
									 UT_UCS4_strlen(m_sFieldValue),
									 NULL);
	}
	if (iNewWidth != getWidth())
	{
		clearScreen();
		markAsDirty();
		if(getLine())
		{
			getLine()->setNeedsRedraw();
		}
		if(getBlock())
		{
			getBlock()->setNeedsRedraw();
		}
		_setWidth(iNewWidth);

		return true;
	}

	return false;
}

bool fp_FieldRun::_setValue(const UT_UCSChar *p_new_value)
{
	if (0 != UT_UCS4_strcmp(p_new_value, m_sFieldValue))
	{
		xxx_UT_DEBUGMSG(("fp_FieldRun::_setValue: setting new value\n"));
		clearScreen();
		markAsDirty();
		if(getLine())
		{
			getLine()->setNeedsRedraw();
		}
		if(getBlock())
		{
			getBlock()->setNeedsRedraw();
		}

		markDrawBufferDirty();
		UT_uint32 iLen = UT_UCS4_strlen(p_new_value);
		iLen = UT_MIN(iLen,FPFIELD_MAX_LENGTH);

		if(iLen > 1 && XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_GUI)
		{
			UT_BidiCharType prevType, myType;

			if(getPrevRun())
				prevType = getPrevRun()->getVisDirection();
			else
				prevType = getBlock()->getDominantDirection();

			myType = prevType;
			UT_bidiReorderString(p_new_value, iLen, myType, m_sFieldValue);

			m_sFieldValue[iLen] = 0;
		}
		else
		{
			UT_UCS4_strcpy(m_sFieldValue, p_new_value);
		}

		{
			// TODO -- is this really needed???
			// should not be, since lookupProperties is called on
			// formatting changes - Tomas
			// lookupProperties();

			getGraphics()->setFont(_getFont());

			UT_sint32 iNewWidth =
				getGraphics()->measureString(m_sFieldValue,
											 0,
											 UT_UCS4_strlen(m_sFieldValue),
											 NULL);
			if (iNewWidth != getWidth())
			{
				_setWidth(iNewWidth);
				markWidthDirty();
				return true;
			}

		}
	}
	xxx_UT_DEBUGMSG(("fp_FieldRun::_setValue: value has not changed [0] %x \n", m_sFieldValue[0]));

	return false;
}

void fp_FieldRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP,
									GR_Graphics * pG)
{
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	PD_Document * pDoc = getBlock()->getDocument();
	fd_Field * fd = NULL;
	if(!getBlock()->isContainedByTOC())
	{
		getBlock()->getField(getBlockOffset() /*+1*/,fd); // Next Pos?
		_setField(fd);
	}
	else
	{
		_setField(NULL);
	}
	if(getField() != NULL)
	{
		getField()->setBlock(getBlock());
	}
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = getBlock()->getDocLayout();

	UT_RGBColor clrFG;
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true), clrFG);
	_setColorFG(clrFG);

	const char * pszFieldColor = NULL;
	pszFieldColor = PP_evalProperty("field-color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true);

	const char * pszBGColor = NULL;
	pszBGColor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true);

//
// FIXME: The "ffffff" is for backwards compatibility. If we don't exclude this
// no prexisting docs will be able to change the Highlight color in paragraphs
// with lists. I think this is a good solution for now. However it does mean
// field-color of "ffffff", pure white is actually transparent.
//
	if(pszFieldColor && strcmp(pszFieldColor,"transparent") != 0 && strcmp(pszFieldColor,"ffffff" ) != 0 && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_RGBColor r;
		UT_parseColor(pszFieldColor, r);
		_setColorHL(r);
	} 
	else if (pszBGColor && strcmp(pszFieldColor,"transparent") != 0)
	{
		UT_RGBColor r;
		UT_parseColor(pszBGColor, r);
		_setColorHL(r);
	}

	
	const gchar* pszType = NULL;
	const gchar* pszParam = NULL;

	if(pSpanAP)
	{
		pSpanAP->getAttribute("type", pszType);
		pSpanAP->getAttribute("param", pszParam);
	}
	else
	{
		pBlockAP->getAttribute("type",pszType);
		pBlockAP->getAttribute("param", pszParam);
	}

	if(pszParam)
		m_pParameter = pszParam;

	// i leave this in because it might be obscuring a larger bug
	//UT_ASSERT(pszType);
	if (!pszType) return;

	int i;
	if(pszType != NULL)
	{
		for( i = 0; fp_FieldFmts[i].m_Tag != NULL; i++ )
		{
			if (0 == strcmp(pszType, fp_FieldFmts[i].m_Tag))
			{
				m_iFieldType = fp_FieldFmts[i].m_Num;
				break;
			}
		}
		if( fp_FieldFmts[i].m_Tag == NULL )
		{
			// probably new type of field
			//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	xxx_UT_DEBUGMSG(("FieldRun: Lookup Properties  field type %d \n",m_iFieldType));
	if(m_iFieldType == FPFIELD_list_label)
	{
		_setFont(pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,pG, true));
	}
	else
	{
		_setFont(pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, pG));
	}
	xxx_UT_DEBUGMSG(("Field font is set to %s \n",_getFont()->getFamily()));
	_setAscent(pG->getFontAscent(_getFont()));
	_setDescent(pG->getFontDescent(_getFont()));
	_setHeight(pG->getFontHeight(_getFont()));

	const gchar * pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	if (0 == strcmp(pszPosition, "superscript"))
	{
		m_fPosition = TEXT_POSITION_SUPERSCRIPT;
	}
	else if (0 == strcmp(pszPosition, "subscript"))
	{
		m_fPosition = TEXT_POSITION_SUBSCRIPT;
	}
	else
	{
		m_fPosition = TEXT_POSITION_NORMAL;
	}
//
// Lookup Decoration properties for this run
//
	const gchar *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  getBlock()->getDocument(), true);
	_setLineWidth(getToplineThickness());
	_setDecorations(0);
	gchar* p;
	if (!(p = g_strdup(pszDecor)))
	{
		// TODO outofmem
	}
	UT_ASSERT(p || !pszDecor);
	gchar*	q = strtok(p, " ");

	while (q)
	{
		if (0 == strcmp(q, "underline"))
		{
			_orDecorations(TEXT_DECOR_UNDERLINE);
		}
		else if (0 == strcmp(q, "overline"))
		{
			_orDecorations(TEXT_DECOR_OVERLINE);
		}
		else if (0 == strcmp(q, "line-through"))
		{
			_orDecorations(TEXT_DECOR_LINETHROUGH);
		}
		else if (0 == strcmp(q, "topline"))
		{
			_orDecorations(TEXT_DECOR_TOPLINE);
		}
		else if (0 == strcmp(q, "bottomline"))
		{
			_orDecorations(TEXT_DECOR_BOTTOMLINE);
		}
		q = strtok(NULL, " ");
	}

	g_free(p);
}


fp_FieldsEnum fp_FieldRun::getFieldType(void) const
{
	return m_iFieldType;
}

bool fp_FieldRun::canBreakAfter(void) const
{
	return true;
}

bool fp_FieldRun::canBreakBefore(void) const
{
	return true;
}

bool fp_FieldRun::_letPointPass(void) const
{
	return true;
}

bool fp_FieldRun::isSuperscript(void) const
{
	return (m_fPosition == TEXT_POSITION_SUPERSCRIPT);
}

bool fp_FieldRun::isSubscript(void) const
{
	return (m_fPosition == TEXT_POSITION_SUBSCRIPT);
}

bool fp_FieldRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_FieldRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: Find everything that calls this and modify them to allow y-axis.
	
	// If X is left of the middle, return offset to the left,
	// otherwise the offset to the right.
	if (x < (getWidth() / 2))
		pos = getBlock()->getPosition() + getBlockOffset();
	else
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();

	bBOL = false;
	if(getNextRun() == NULL)
	{
		bEOL = true;
	}
	if(getNextRun()->getType() == FPRUN_ENDOFPARAGRAPH)
	{
		bEOL = true;
	}
}

void fp_FieldRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x,
                                  UT_sint32& y, UT_sint32& x2,
                                  UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	xxx_UT_DEBUGMSG(("findPointCoords: FieldRun offset %d \n",iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(getLine());

	// TODO is this really needed ???
	// should not be, since lookupProperties is called on
	// formatting changes - Tomas
	// lookupProperties();

	getLine()->getOffsets(this, xoff, yoff);
	xxx_UT_DEBUGMSG(("findPointCoords: FieldRun orig yoff %d \n",yoff)); 
//
// The footnote code is to handle discontinuities in offset from embedded
// footnotes in blocks.
//
	bool bFootnote = false;
	if (iOffset == (getBlockOffset() + getLength()))
	{
		xoff += getWidth();
	}
	else if(iOffset > (getBlockOffset() + getLength()))
	{
		bFootnote = true;
		xoff += getWidth();
	}

	if (!bFootnote && (m_fPosition == TEXT_POSITION_SUPERSCRIPT))
	{
		yoff -= getAscent() * 1/2;
	}
	else if (!bFootnote && (m_fPosition == TEXT_POSITION_SUBSCRIPT))
	{
		yoff += getDescent() /* * 3/2 */;
	}
	xxx_UT_DEBUGMSG(("findPointCoords: FieldRun yoff %d \n",yoff)); 
 	x = xoff;
	y = yoff;
	if(!bFootnote)
	{
		height = getHeight();
	}
	else
	{
//
// We're actually just before the next run and in the insertion point will be
// in the next run so make the insertion point reflect this.
//
		if(getNextRun() && getNextRun()->hasLayoutProperties()  )
		{
			height = getNextRun()->getHeight();
			UT_sint32 xx,xx2,yy2,hheight;
			bool bbDirection;
			getNextRun()->findPointCoords(iOffset+1,xx,y,xx2,yy2, hheight,
									   bbDirection);
			height = hheight;

		}
		else
		{
			height = getHeight();
		}
	}
	x2 = x;
	y2 = y;
	bDirection = (getVisDirection() != UT_BIDI_LTR);
}

bool fp_FieldRun::calculateValue(void)
{
	//
	// Code for the Piece Table Fields Calculation
	// Get size of the field from the following runs
	//
	//      return getField()->update();
	//        UT_ASSERT(getField());

/*  UT_sint32 count = 0;
    fp_Run* pNext = getNextRun();
	while(pNext != NULL && pNext->getField() != NULL )
	{
	    if(getField() == NULL)
		{
		        getField() = pNext->getField();
		}
	    pNext = getNextRun();
		count++;
	}
	if(count == 0)
	{
	    setWidth(0);
		_setHeight(0);
	}
	else
	{
	    pNext = getPrevRun();
		setWidth(pNext->getWidth());
		_setHeight(pNext->getHeight());
	}
	if(getField() != NULL)
	getField()->update();
*/
	return true;
}

void fp_FieldRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());

	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	Fill(getGraphics(), xoff, yoff, getWidth(), iLineHeight);
}

void fp_FieldRun::_defaultDraw(dg_DrawArgs* pDA)
{
	GR_Graphics * pG = pDA->pG;

	// TODO is this really needed
	// should not be, since lookupProperties is called on
	// formatting changes - Tomas
	// lookupProperties();
	UT_sint32 xoff = 0, yoff = 0;

	GR_Painter painter(pG);

	// need screen locations of this run

	getLine()->getScreenOffsets(this, xoff, yoff);

	UT_sint32 iYdraw =  pDA->yoff - getAscent()-1;

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		iYdraw -= getAscent() * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		iYdraw +=  getDescent(); // * 3/2
	}

	//if (pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

//
// Sevior was here
//		UT_sint32 iFillTop = iYdraw;
		UT_sint32 iFillTop = iYdraw+1;
		UT_sint32 iFillHeight = getAscent() + getDescent();

		FV_View* pView = _getView();
		UT_uint32 iSelAnchor = pView->getSelectionAnchor();
		UT_uint32 iPoint = pView->getPoint();

		UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
		UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

		UT_ASSERT(iSel1 <= iSel2);
		bool bIsInTOC = getBlock()->isContainedByTOC();
		if (
			isInSelectedTOC() || (!bIsInTOC && (
		    /* pView->getFocus()!=AV_FOCUS_NONE && */
			(iSel1 <= iRunBase)
			&& (iSel2 > iRunBase)))
			)
		{
			UT_RGBColor color(_getView()->getColorSelBackground());			
			pG->setColor(_getView()->getColorSelForeground());
			painter.fillRect(color, pDA->xoff, iFillTop, getWidth(), iFillHeight);

		}
		else
		{
			if (m_iFieldType != FPFIELD_list_label)
				Fill(getGraphics(),pDA->xoff, iFillTop, getWidth(), iFillHeight);
			pG->setColor(_getColorFG());
		}
	}

	pG->setFont(_getFont());
	xxx_UT_DEBUGMSG(("Field draw with font %s \n",_getFont()->getFamily()));

	UT_uint32 len = UT_UCS4_strlen(m_sFieldValue);
	if (len == 0)
	{
		return;
	}

	painter.drawChars(m_sFieldValue, 0, len, pDA->xoff,iYdraw, NULL);
//
// Draw underline/overline/strikethough
//
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove
	                                                 //character dirt
	xxx_UT_DEBUGMSG(("xoff infield before drawdecors %d \n",pDA->xoff));
	drawDecors( pDA->xoff, yTopOfRun,pG);

}

// BEGIN DOM work on some new fields

fp_FieldCharCountRun::fp_FieldCharCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldCharCountRun::calculateValue(void)
{
	UT_UTF8String szFieldValue;

	FV_View *pView = _getView();
	if(!pView)
	{
	    szFieldValue ="?";
	}
	else
	{
	    FV_DocCount cnt = pView->countWords(true);
	    UT_UTF8String_sprintf(szFieldValue, "%d", cnt.ch_sp);
	}
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldNonBlankCharCountRun::fp_FieldNonBlankCharCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldNonBlankCharCountRun::calculateValue(void)
{
	UT_UTF8String szFieldValue ("?");

	FV_View *pView = _getView();
	if(pView)
	{
		FV_DocCount cnt = pView->countWords(true);
	    UT_UTF8String_sprintf(szFieldValue, "%d", cnt.ch_no);
	}

	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldLineCountRun::fp_FieldLineCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldLineCountRun::calculateValue(void)
{
	FV_View *pView = _getView();
	UT_UTF8String szFieldValue ("?");

	if(pView)
	{
	    FV_DocCount cnt = pView->countWords(false);
	    UT_UTF8String_sprintf(szFieldValue, "%d", cnt.line);
	}

	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldParaCountRun::fp_FieldParaCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}


bool fp_FieldParaCountRun::calculateValue(void)
{
	FV_View *pView = _getView();
	UT_UTF8String szFieldValue ("?");
	if(pView)
	{
	    FV_DocCount cnt = pView->countWords(false);
	    UT_UTF8String_sprintf(szFieldValue, "%d", cnt.para);
	}

	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldWordCountRun::fp_FieldWordCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldWordCountRun::calculateValue(void)
{
	FV_View *pView = _getView();
	UT_UTF8String szFieldValue ("?");
	if(pView)
	{
	    FV_DocCount cnt = pView->countWords(true);
	    UT_UTF8String_sprintf(szFieldValue, "%d", cnt.word);
	}

	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

// mm/dd/yy notation
fp_FieldMMDDYYRun::fp_FieldMMDDYYRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldMMDDYYRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%m/%d/%y", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

// dd/mm/yy time
fp_FieldDDMMYYRun::fp_FieldDDMMYYRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDDMMYYRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%d/%m/%y", pTime);
	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

// Month Day, Year
fp_FieldMonthDayYearRun::fp_FieldMonthDayYearRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldMonthDayYearRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%B %d, %Y", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldMthDayYearRun::fp_FieldMthDayYearRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldMthDayYearRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%b %d, %Y", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDefaultDateRun::fp_FieldDefaultDateRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDefaultDateRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%c", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDefaultDateNoTimeRun::fp_FieldDefaultDateNoTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDefaultDateNoTimeRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%x", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldWkdayRun::fp_FieldWkdayRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldWkdayRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%A", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(g_strdup(szFieldValue)));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDOYRun::fp_FieldDOYRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDOYRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%j", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldMilTimeRun::fp_FieldMilTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldMilTimeRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%H:%M:%S", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldAMPMRun::fp_FieldAMPMRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldAMPMRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%p", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldTimeEpochRun::fp_FieldTimeEpochRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldTimeEpochRun::calculateValue(void)
{
	UT_UTF8String szFieldValue;

	time_t	tim = time(NULL);
	UT_UTF8String_sprintf(szFieldValue, "%ld", static_cast<long>(tim));
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldDateTimeCustomRun::fp_FieldDateTimeCustomRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDateTimeCustomRun::calculateValue(void)
{
	fd_Field * fld = getField();
	if (fld) {
	  const gchar * param = fld->getParameter ();

	  if (!param) // sensible fallback if no param specified
		  param = "%x %X";
	  
	  UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	  sz_ucs_FieldValue[0] = 0;
	  
	  char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	  
	  time_t	tim = time(NULL);
	  struct tm *pTime = localtime(&tim);
	  
	  strftime(szFieldValue, FPFIELD_MAX_LENGTH, param, pTime);
	  if (getField())
		  getField()->setValue(static_cast<const gchar*>(szFieldValue));
	  
	  UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);
	  
	  return _setValue(sz_ucs_FieldValue);
	}

	return false;
}

fp_FieldTimeZoneRun::fp_FieldTimeZoneRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldTimeZoneRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%Z", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildIdRun::fp_FieldBuildIdRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildIdRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_ID);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_ID));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldBuildVersionRun::fp_FieldBuildVersionRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildVersionRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_Version);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_Version));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldBuildOptionsRun::fp_FieldBuildOptionsRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildOptionsRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_Options);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_Options));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldBuildTargetRun::fp_FieldBuildTargetRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildTargetRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_Target);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_Target));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldBuildCompileDateRun::fp_FieldBuildCompileDateRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileDateRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_CompileDate);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_CompileDate));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

fp_FieldBuildCompileTimeRun::fp_FieldBuildCompileTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileTimeRun::calculateValue(void)
{
	UT_UTF8String szFieldValue(XAP_App::s_szBuild_CompileTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(XAP_App::s_szBuild_CompileTime));
	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}


// Refers to an footnote in the main body of the text.
fp_FieldFootnoteRefRun::fp_FieldFootnoteRefRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_if_fail(pp);

	const gchar * footid;
	bool bRes = pp->getAttribute("footnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldFootnoteRefRun::fp_FieldFootnoteRefRun: Missing footnote-id attribute. Probably a malformed file.\n"));
		return;
	}
	m_iPID = atol(footid);

	// see bug 9793
	_setDirection(pBL->getDominantDirection());
}


bool fp_FieldFootnoteRefRun::calculateValue(void)
{
	const PP_AttrProp * pp = getSpanAP();
	if(pp == NULL)
	{
		return false;
	}
	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("footnote-id", footid);
        
	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldFootnoteRefRun::calculateValue: Missing footnote-id attribute. Probably a malformed file.\n"));
		return false;
	}
	FV_View * pView = _getView();
	UT_uint32 iPID = atoi(footid);
        const gchar *szCitation = NULL;
        bool bHaveCitation = pp->getAttribute("text:note-citation", szCitation);
	UT_sint32 footnoteNo = bHaveCitation ? 
            atoi(szCitation) : pView->getLayout()->getFootnoteVal(iPID);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_String sFieldValue;
	FootnoteType iFootType = pView->getLayout()->getFootnoteType();
	pView->getLayout()->getStringFromFootnoteVal(sFieldValue,footnoteNo,iFootType);
	UT_UCS4_strcpy_char(sz_ucs_FieldValue, sFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

bool fp_FieldFootnoteRefRun::canBreakBefore(void) const
{
	return false;
}

fp_FieldFootnoteAnchorRun::fp_FieldFootnoteAnchorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_if_fail(pp);

	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("footnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldFootnoteAnchorRun::fp_FieldFootnoteAnchorRun: Missing footnote-id attribute. Probably a malformed file.\n"));
		return;
	}
	m_iPID = atol(footid);

	// see bug 9793
	_setDirection(pBL->getDominantDirection());
}

// Appears in the FootnoteContainer, one per footnote.
bool fp_FieldFootnoteAnchorRun::calculateValue(void)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_val_if_fail(pp, false);

	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("footnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldFootnoteAnchorRun::calculateValue: Missing footnote-id attribute. Probably a malformed file.\n"));
		return false;
	}
	UT_uint32 iPID = atoi(footid);
	FV_View * pView = _getView();
        const gchar *szCitation = NULL;
        bool bHaveCitation = pp->getAttribute("text:note-citation", szCitation);
	UT_sint32 footnoteNo = bHaveCitation ? 
            atoi(szCitation) : pView->getLayout()->getFootnoteVal(iPID);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	FootnoteType iFootType = pView->getLayout()->getFootnoteType();

	UT_String sFieldValue;
	pView->getLayout()->getStringFromFootnoteVal(sFieldValue,footnoteNo,iFootType);
	UT_UCS4_strcpy_char(sz_ucs_FieldValue, sFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}


fp_FieldEndnoteAnchorRun::fp_FieldEndnoteAnchorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_if_fail(pp);

	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("endnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldEndnoteAnchorRun::fp_FieldEndnoteAnchorRun: Missing endnote-id attribute. Probably a malformed file.\n"));
		return;
	}
	m_iPID = atoi(footid);

	// see bug 9793
	_setDirection(pBL->getDominantDirection());
}

// Appears in the EndnoteSection, one per endnote.
bool fp_FieldEndnoteAnchorRun::calculateValue(void)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_val_if_fail(pp, false);

	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("endnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldEndnoteAnchorRun::calculateValue: Missing endnote-id attribute. Probably a malformed file.\n"));
		return false;
	}
	UT_uint32 iPID = atoi(footid);
	FV_View * pView = _getView();
	UT_sint32 endnoteNo = pView->getLayout()->getEndnoteVal(iPID);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	FootnoteType iEndType = pView->getLayout()->getEndnoteType();

	UT_String sFieldValue;
	pView->getLayout()->getStringFromFootnoteVal(sFieldValue,endnoteNo,iEndType);
	UT_UCS4_strcpy_char(sz_ucs_FieldValue, sFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}


fp_FieldEndnoteRefRun::fp_FieldEndnoteRefRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_if_fail(pp);

	const gchar * footid;
	bool bRes = pp->getAttribute("endnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldEndnoteRefRun::fp_FieldEndnoteRefRun: Missing endnote-id attribute. Probably a malformed file.\n"));
		return;
	}
	m_iPID = atoi(footid);

	// see bug 9793
	_setDirection(pBL->getDominantDirection());
}

// Appears in the EndnoteSection, one per endnote.
bool fp_FieldEndnoteRefRun::calculateValue(void)
{
	const PP_AttrProp * pp = getSpanAP();
	UT_return_val_if_fail(pp, false);

	const gchar * footid = NULL;
	bool bRes = pp->getAttribute("endnote-id", footid);

	if(!bRes || !footid)
	{
		UT_DEBUGMSG(("fp_FieldEndnoteRefRun::calculateValue: Missing endnote-id attribute. Probably a malformed file.\n"));
		return false;
	}
	UT_uint32 iPID = atoi(footid);
	FV_View * pView = _getView();
	UT_sint32 endnoteNo = pView->getLayout()->getEndnoteVal(iPID);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	FootnoteType iEndType = pView->getLayout()->getEndnoteType();

	UT_String sFieldValue;
	pView->getLayout()->getStringFromFootnoteVal(sFieldValue,endnoteNo,iEndType);
	UT_UCS4_strcpy_char(sz_ucs_FieldValue, sFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

bool fp_FieldEndnoteRefRun::canBreakBefore(void) const
{
	return false;
}

fp_FieldTimeRun::fp_FieldTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldTimeRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%X", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDateRun::fp_FieldDateRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldDateRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	struct tm *pTime = localtime(&tim);

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%A %B %d, %Y", pTime);
	if (getField())
		getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldFileNameRun::fp_FieldFileNameRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldFileNameRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	PD_Document * pDoc = getBlock()->getDocument();
	UT_return_val_if_fail(pDoc, false);

	//copy in the name or some wierd char instead
	std::string name = pDoc->getFilename();
	if (name.empty()) {
		name = "*";
	}

	strncpy (szFieldValue, name.c_str(), FPFIELD_MAX_LENGTH);
	szFieldValue[FPFIELD_MAX_LENGTH] = '\0';

	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldShortFileNameRun::fp_FieldShortFileNameRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldShortFileNameRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	PD_Document * pDoc = getBlock()->getDocument();
	UT_return_val_if_fail(pDoc, false);

	//copy in the name or some wierd char instead
	const char * name = UT_go_basename_from_uri(pDoc->getFilename().c_str());
	if (!name)
		name = "*";

	strncpy (szFieldValue, name, FPFIELD_MAX_LENGTH);
	szFieldValue[FPFIELD_MAX_LENGTH] = '\0';

	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldPageNumberRun::fp_FieldPageNumberRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldPageNumberRun::calculateValue(void)
{
	UT_UTF8String szFieldValue ("?");

	if (getLine() && getLine()->getContainer() && getLine()->getContainer()->getPage())
	{
		fp_Page* pPage = getLine()->getContainer()->getPage();
		pPage->resetFieldPageNumber();
		UT_sint32 iPageNum = pPage->getFieldPageNumber();
		if (iPageNum > 0)
		{
			UT_UTF8String_sprintf(szFieldValue, "%d", iPageNum);
		}
	}

	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

fp_FieldPageReferenceRun::fp_FieldPageReferenceRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen)
	: fp_FieldRun(pBL,  iOffsetFirst, iLen)
{
}

bool fp_FieldPageReferenceRun::calculateValue(void)
{
	UT_UTF8String szFieldValue ("?");

	if(!_getParameter())
		return false;

	FV_View * pView = _getView();
	// on import the field value can be requested before the View exists
	// so we cannot assert here
	//UT_ASSERT(pView);
	if(!pView)
		return false;

	fp_Run* pRun = NULL;
	fl_BlockLayout * pBlock;
	fl_SectionLayout * pSection = pView->getLayout()->getFirstSection();
	UT_ASSERT(pSection);
	bool bFound = false;

	while (pSection)
	{
		pBlock = static_cast<fl_BlockLayout *>(pSection->getFirstLayout());

		while (pBlock)
		{
			pRun = pBlock->getFirstRun();
			while (pRun)
			{
				xxx_UT_DEBUGMSG(("pRun 0x%x, type %d\n", pRun, pRun->getType()));
				if(pRun->getType() == FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pB = static_cast<fp_BookmarkRun*>(pRun);
					if(pB->isStartOfBookmark() && !strcmp(_getParameter(),pB->getName()))
					{
						bFound = true;
						break;
					}
				}
				pRun = pRun->getNextRun();
			}
			if(bFound)
				break;

			pBlock = static_cast<fl_BlockLayout *>(pBlock->getNext());
		}
		if(bFound)
			break;
		pSection = static_cast<fl_SectionLayout *>(pSection->getNext());
	}

	if(    pRun
		&& pRun->getLine()
		&& pRun->getLine()->getContainer()
		&& pRun->getLine()->getContainer()->getPage())
	{
		fp_Page* pPage = pRun->getLine()->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		UT_sint32 iPageNum = 0;
		UT_uint32 iNumPages = pDL->countPages();
		for (UT_uint32 i=0; i<iNumPages; i++)
		{
			fp_Page* pPg = pDL->getNthPage(i);

        	if (pPg == pPage)
			{
				iPageNum = i + 1;
				break;
			}
		}
		UT_UTF8String_sprintf(szFieldValue, "%d", iPageNum);
	}
	else
	{
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		std::string Msg1;
		pSS->getValue(AP_STRING_ID_FIELD_Error, XAP_App::getApp()->getDefaultEncoding(), Msg1);

		std::string Msg2;
		pSS->getValue(AP_STRING_ID_MSG_BookmarkNotFound, XAP_App::getApp()->getDefaultEncoding(), Msg2);
		std::string format = UT_std_string_sprintf("{%s: %s}", Msg1.c_str(), Msg2.c_str());
		UT_UTF8String_sprintf(szFieldValue, format.c_str(), _getParameter());
	}

	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}


///////////////////////////////////////////////////////////////////////////

fp_FieldPageCountRun::fp_FieldPageCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldPageCountRun::calculateValue(void)
{
	UT_UTF8String szFieldValue ("?");

	if (getLine() && getLine()->getContainer() && getLine()->getContainer()->getPage())
	{

		fp_Page* pPage = getLine()->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		UT_UTF8String_sprintf(szFieldValue, "%d", pDL->countPages());
	}

	if (getField())
	  getField()->setValue(static_cast<const gchar*>(szFieldValue.utf8_str()));

	return _setValue(szFieldValue.ucs4_str().ucs4_str());
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldMailMergeRun::fp_FieldMailMergeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen)
  : fp_FieldRun(pBL, iOffsetFirst, iLen)
{
}

bool fp_FieldMailMergeRun::calculateValue(void)
{
	fd_Field * fld = getField();
	if (fld) {
	  const gchar * param = fld->getParameter ();

	  if (!param)
	    return false;

	  UT_UTF8String value ;

	  PD_Document * pDoc = getBlock()->getDocument();
	  UT_ASSERT(pDoc);	  

	  if (!pDoc->mailMergeFieldExists(param))
	  {
	    // we'll take this branch if there's no mapping, we'll display
	    // the field name instead
	    value = "<";
	    value += param;
	    value += ">";
	  }
	  else
	    {
	      value = pDoc->getMailMergeField(param);
	    }

	  fld->setValue(static_cast<const gchar*>(value.utf8_str()));

	  return _setValue(value.ucs4_str().ucs4_str());
	}

	return false;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldMetaRun::fp_FieldMetaRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen, const char * which)
  : fp_FieldRun(pBL, iOffsetFirst, iLen), m_which(which)
{
}

bool fp_FieldMetaRun::calculateValue(void)
{
	PD_Document * pDoc = getBlock()->getDocument();
	UT_ASSERT(pDoc);

	std::string value ;
	if (!pDoc->getMetaDataProp(m_which, value) || value.empty())
	  value = " ";

	if (getField())
		getField()->setValue(value.c_str());

	return _setValue(UT_UCS4String(value).ucs4_str());
}

fp_FieldMetaTitleRun::fp_FieldMetaTitleRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_TITLE)
{
}

fp_FieldMetaCreatorRun::fp_FieldMetaCreatorRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_CREATOR)
{
}

fp_FieldMetaSubjectRun::fp_FieldMetaSubjectRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_SUBJECT)
{
}

fp_FieldMetaPublisherRun::fp_FieldMetaPublisherRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_PUBLISHER)
{
}

fp_FieldMetaDateRun::fp_FieldMetaDateRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_DATE)
{
}

fp_FieldMetaDateLastChangedRun::fp_FieldMetaDateLastChangedRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_DATE_LAST_CHANGED)
{
}

fp_FieldMetaTypeRun::fp_FieldMetaTypeRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_TYPE)
{
}

fp_FieldMetaLanguageRun::fp_FieldMetaLanguageRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_LANGUAGE)
{
}

fp_FieldMetaRightsRun::fp_FieldMetaRightsRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_RIGHTS)
{
}

fp_FieldMetaKeywordsRun::fp_FieldMetaKeywordsRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_KEYWORDS)
{
}

fp_FieldMetaContributorRun::fp_FieldMetaContributorRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_CONTRIBUTOR)
{
}

fp_FieldMetaCoverageRun::fp_FieldMetaCoverageRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_COVERAGE)
{
}

fp_FieldMetaDescriptionRun::fp_FieldMetaDescriptionRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, iOffsetFirst, iLen, PD_META_KEY_DESCRIPTION)
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedColumnBreakRun::fp_ForcedColumnBreakRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FORCEDCOLUMNBREAK)
{
	lookupProperties();
}

void fp_ForcedColumnBreakRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
												const PP_AttrProp * /*pBlockAP*/,
												const PP_AttrProp * /*pSectionAP*/,
												GR_Graphics *)
{
	fd_Field * fd = NULL;

	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);

	_inheritProperties();
	_setWidth(1);
}

bool fp_ForcedColumnBreakRun::canBreakAfter(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_ForcedColumnBreakRun::canBreakBefore(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fp_ForcedColumnBreakRun::_letPointPass(void) const
{
	return false;
}

void fp_ForcedColumnBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_ForcedColumnBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ForcedColumnBreakRun\n"));
	UT_ASSERT(getBlockOffset() == iOffset || getBlockOffset()+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		if(FPRUN_TEXT == pPropRun->getType())
		{
			pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = getHeight();
			getLine()->getOffsets(this, xoff, yoff);
			x = xoff;
			y = yoff;
		}
	}
	else
	{
	    height = getHeight();
		getLine()->getOffsets(this, xoff, yoff);
		x = xoff;
		y = yoff;
	}

	x2 = x;
	y2 = y;
}

void fp_ForcedColumnBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    getLine()->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = getLine()->getMaxWidth() - getLine()->calculateWidthOfLine();
	Fill(getGraphics(),xoff,yoff,iWidth,getLine()->getHeight());
}

void fp_ForcedColumnBreakRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics * pG = pDA->pG;
    if (!(pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = _getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = getLine()->getMaxWidth();

    const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
    std::string s;
    pSS->getValueUTF8(AP_STRING_ID_BREAK_Column, s);
    UT_UCSChar *pColumnBreak;
    UT_UCS4_cloneString_char(&pColumnBreak, s.c_str());
	_drawTextLine(pDA->xoff,pDA->yoff+getLine()->getAscent(),iLineWidth,getLine()->getHeight(),pColumnBreak);
    FREEP(pColumnBreak);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedPageBreakRun::fp_ForcedPageBreakRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, iOffsetFirst, iLen, FPRUN_FORCEDPAGEBREAK)
{
	lookupProperties();
}

void fp_ForcedPageBreakRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
											  const PP_AttrProp * /*pBlockAP*/,
											  const PP_AttrProp * /*pSectionAP*/,
											  GR_Graphics *)
{
	fd_Field * fd = NULL;

	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);

	_inheritProperties();
	_setWidth(1);
}

bool fp_ForcedPageBreakRun::canBreakAfter(void) const
{
	return false;
}

bool fp_ForcedPageBreakRun::canBreakBefore(void) const
{
	return false;
}

bool fp_ForcedPageBreakRun::_letPointPass(void) const
{
	return false;
}

void fp_ForcedPageBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& /*isTOC*/)
{
	//TODO: Find everything that calls this and modify them to allow y-axis.
	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_ForcedPageBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ForcedPageBreakRun\n"));
	UT_ASSERT(getBlockOffset() == iOffset || getBlockOffset()+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		height = pPropRun->getHeight();
		if(FPRUN_TEXT == pPropRun->getType())
		{
			pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = getHeight();
			getLine()->getOffsets(this, xoff, yoff);
			x = xoff;
			y = yoff;
		}
	}
	else
	{
		height = getHeight();
		getLine()->getOffsets(this, xoff, yoff);
		x = xoff;
		y = yoff;
	}

	if (iOffset == getBlockOffset()+1)
	{
	    FV_View* pView = _getView();
	    if (pView->getShowPara())
		{
			x += getWidth();
	    }
	}

	x2 = x;
	y2 = y;
}

void fp_ForcedPageBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());
	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    getLine()->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = getLine()->getMaxWidth() - getLine()->calculateWidthOfLine();
	Fill(getGraphics(),xoff,yoff,iWidth,getLine()->getHeight());
}

void fp_ForcedPageBreakRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics * pG = pDA->pG;

    if (!(pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = _getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = getLine()->getMaxWidth();

    const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
    std::string s;
    pSS->getValueUTF8(AP_STRING_ID_BREAK_Page, s);
    UT_UCSChar *pPageBreak;
    UT_UCS4_cloneString_char(&pPageBreak, s.c_str());

	_drawTextLine(pDA->xoff,pDA->yoff+getLine()->getAscent(),iLineWidth,getLine()->getHeight(),pPageBreak);
    FREEP(pPageBreak);
}

// translates logical position in a run into visual position
// (will also translate correctly visual -> logical)
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos) const
{
    if(getVisDirection() == UT_BIDI_RTL) //rtl needs translation
    {
        return (getLength() - iLogPos - 1);
    }
    else return (iLogPos);
}

//translates a visual position in a span of length iLen to logical pos
//or vice versa
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen) const
{
    if(getVisDirection() == UT_BIDI_RTL) //rtl needs translation
    {
        return (iLen - iLogPos - 1);
    }
    else return (iLogPos);
}

//returns the logical offset of the first visual character
UT_uint32 fp_Run::getOffsetFirstVis() const
{
    if(getVisDirection() == UT_BIDI_RTL) //rtl, requires translation
    {
        return(getBlockOffset() + getLength() - 1);
    }
    else return (getBlockOffset());
}

//translates visual offset to logical one, can be also used for translation
//in the other direction
UT_uint32 fp_Run::getOffsetLog(UT_uint32 iVisOff) const
{
    if(getVisDirection() == UT_BIDI_RTL) //rtl needs translation
    {
        return(getBlockOffset() + getLength() - iVisOff + getBlockOffset() - 1);
    }
    else return (iVisOff);
}

fp_Run * fp_Run::getNextVisual()
{
	if(!getLine())
		return NULL;

	UT_uint32 iIndxVis = getLine()->getVisIndx(this);

	return getLine()->getRunAtVisPos(iIndxVis + 1);
}

fp_Run * fp_Run::getPrevVisual()
{
	if(!getLine())
		return NULL;

	UT_uint32 iIndxVis = getLine()->getVisIndx(this);

	if(!iIndxVis)
		return NULL;

	return getLine()->getRunAtVisPos(iIndxVis - 1);
}

void fp_Run::setDirection(UT_BidiCharType iDir)
{
    xxx_UT_DEBUGMSG(("fp_Run::SetDirection, getDirection() %d, iDir %d, run type %d\n", getDirection(), iDir, getType()));
	UT_BidiCharType iDirection = iDir != static_cast<UT_BidiCharType>(UT_BIDI_UNSET) ? iDir : UT_BIDI_WS;
	if(getDirection() != iDirection)
	{
		UT_BidiCharType origDirection = getDirection();
		_setDirection(iDirection);
		clearScreen();
		/*
		  if this run belongs to a line we have to notify the line that
		  that it now contains a run of this direction, if it does not belong
		  to a line this will be taken care of by the fp_Line:: member function
		  used to add the run to the line (generally, we set it here if this
		  is a run that is being typed in and it gets set in the member
		  functions when the run is loaded from a document on the disk.)
		*/

		if(getLine())
			getLine()->changeDirectionUsed(origDirection,getDirection(),true);
	}
}

// returns the direction with which the run is displayed,
UT_BidiCharType fp_Run::getVisDirection() const
{
#ifndef NO_BIDI_SUPPORT
 	FV_View * pView = _getView();
	if(pView && pView->getBidiOrder() != FV_Order_Visual)
	{
		if(pView->getBidiOrder() == FV_Order_Logical_LTR)
			return UT_BIDI_LTR;
		else
			return UT_BIDI_RTL;
	}
	else if(m_iVisDirection == static_cast<UT_BidiCharType>(UT_BIDI_UNSET))
	{
		if(m_pLine)
		{
			m_pLine->_createMapOfRuns();
			UT_ASSERT(m_iVisDirection !=  static_cast<UT_BidiCharType>(UT_BIDI_UNSET));
			return m_iVisDirection;
		}
		else if(getBlock())
			return getBlock()->getDominantDirection();
		else
		{
			bool b;
			XAP_App::getApp()->getPrefsValueBool(static_cast<const gchar*>(AP_PREF_KEY_DefaultDirectionRtl), &b);
			if(b)
				return UT_BIDI_RTL;
			else
				return UT_BIDI_LTR;
		}
	}
	else
		return m_iVisDirection;
#else
	return UT_BIDI_LTR;
#endif
}

void fp_Run::setVisDirection(UT_BidiCharType iDir)
{
    if(   iDir != m_iVisDirection
		  && m_iVisDirection !=  static_cast<UT_BidiCharType>(UT_BIDI_UNSET)
		  /*&& m_eRefreshDrawBuffer == GRSR_BufferClean*/)
	{
		// the text in the buffer is in the wrong order, schedule it
		// for refresh
		m_eRefreshDrawBuffer = GRSR_Unknown;
	}
	
	m_iVisDirection = iDir;
}

#if 0
void fp_Run::setDirectionProperty(UT_BidiCharType dir)
{
	const gchar * prop[] = {NULL, NULL, 0};
	const gchar direction[] = "dir";
	const gchar rtl[] = "rtl";
	const gchar ltr[] = "ltr";
	UT_String other;

	prop[0] = static_cast<const gchar*>(&direction);

	switch(dir)
	{
		case UT_BIDI_LTR:  prop[1] = static_cast<const gchar*>(&ltr);     break;
		case UT_BIDI_RTL:  prop[1] = static_cast<const gchar*>(&rtl);     break;
		default:
		 {
		 	// for anything other we will print the UT_BidiCharType value
		 	// this will allow us to coallesce runs of same type without
		 	// having to list here tons of possible strings
		 	// (we could do this for rtl and ltr as well, but "rtl" and "ltr"
		 	// are much more informative.)
		 	UT_String_sprintf(other,"fbt%d",static_cast<UT_uint32>(dir));
		 	prop[1] = static_cast<const gchar*>(other.c_str()); break;
		 }
	};

	UT_uint32 offset = getBlock()->getPosition() + getBlockOffset();
	getBlock()->getDocument()->changeSpanFmt(PTC_AddFmt,offset,offset + getLength(),NULL,prop);
	UT_DEBUGMSG(("fp_Run::setDirectionProperty: offset=%d, len=%d, dir=\"%s\"\n", offset,getLength(),prop[1]));
}
#endif

/*!
    The following function allows us to respond to deletion of part of
    a run in a smart way; this is just default implementation, and
    there is nothing smart about it, derrived classes should provide
    their own implementation where it makes sense (see fp_TextRun)
    
    \param offset: run offset at which deletion starts
    \param iLen:   length of the deleted section, can reach past the
                   end of the run
*/
void fp_Run::updateOnDelete(UT_uint32 offset, UT_uint32 iLenToDelete)
{
	// do not try to delete past the end of the run ...
	UT_return_if_fail(offset < m_iLen);

	UT_uint32 iLen = UT_MIN(iLenToDelete, m_iLen - offset);
	
	// do not try to delete nothing ...
	if(iLen == 0)
		return;

	setLength(m_iLen - iLen, true);
}

// house keeping
#undef FPRUN_PROPS_MINI_DUMP
