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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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
#include "fv_View.h"
#include "pp_AttrProp.h"
#include "fd_Field.h"
#include "po_Bookmark.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_FootnoteLayout.h"

#include "ap_Prefs.h"
#include "xap_Frame.h"

#include "ut_sleep.h"

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
			   GR_Graphics* pG,
			   UT_uint32 iOffsetFirst,
			   UT_uint32 iLen,
			   FP_RUN_TYPE iType)
	:	m_iType (iType),
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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_iHeightLayoutUnits(0),
		m_iWidthLayoutUnits(0),
		m_iAscentLayoutUnits(0),
		m_iDescentLayoutUnits(0),
#endif
		m_iOffsetFirst(iOffsetFirst),
		m_iLen(iLen),
		m_pG(pG),
		m_bDirty(true),	// a run which has just been created is not onscreen, therefore it is dirty
		m_pField(0),
		m_iDirection(FRIBIDI_TYPE_WS), //by default all runs are whitespace
		m_iVisDirection(FRIBIDI_TYPE_UNSET),
		m_bRefreshDrawBuffer(true),
		m_pColorHL(255,255,255,true), // set highlight colour to transparent
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_pScreenFont(0),
		m_pLayoutFont(0),
#elif defined(WITH_PANGO)
		m_pPangoFont(0),
#else
		m_pFont(0),
#endif
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
		m_eHidden(FP_VISIBLE)
{
        // set the default background color and the paper color of the
	    // section owning the run.

	// will do this in ::lookupProperties
	//updateHighlightColor();
	//updatePageColor();
}

fp_Run::~fp_Run()
{
  // no impl.

// Zero these to trap mem errors.
#if 1
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pBL = NULL;
	m_pLine = NULL;
#endif
}

void fp_Run::lookupProperties()
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	bool bDelete;

	m_pBL->getAttrProp(&pBlockAP);

	PD_Document * pDoc = m_pBL->getDocument();

	// examining the m_pRevisions contents is too involved, it is
	// faster to delete it and create a new instance if needed
	if(m_pRevisions)
	{
		delete m_pRevisions;
		m_pRevisions = NULL;
	}

	// NB this call will recreate m_pRevisions for us
	getSpanAP(pSpanAP,bDelete);


	//evaluate the "display" property

	const XML_Char *pszDisplay = PP_evalProperty("display",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	if(pszDisplay && !UT_strcmp(pszDisplay,"none"))
	{
		setVisibility(FP_HIDDEN_TEXT);
	}
	   else
	{
		setVisibility(FP_VISIBLE);
	}

	// here we handle background colour -- we parse the property into
	// m_pColorHL and then call updateHighlightColor() to overlay any
	// colour from higher layout elements
	const char * pszBGcolor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	_setColorHL(pszBGcolor);

	updateHighlightColor();
	updatePageColor();

	_lookupProperties(pSpanAP, pBlockAP, pSectionAP);

	if(m_pRevisions)
	{
		FV_View* pView = _getView();
		UT_return_if_fail(pView);
		UT_uint32 iId  = pView->getRevisionLevel();

		if(iId && !m_pRevisions->isVisible(iId))
		{
			if(isHidden() == FP_HIDDEN_TEXT)
			{
				setVisibility(FP_HIDDEN_REVISION_AND_TEXT);
			}
			else
			{
				setVisibility(FP_HIDDEN_REVISION);
			}
		}
	}

	//if we are responsible for deleting pSpanAP, then just do so
	if(bDelete)
		delete pSpanAP;

}

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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
bool fp_Run::findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 /* iMaxLeftWidth */,
												   fp_RunSplitInfo& /* si */,
												   bool /* bForce */)
{
	return false;
}
#else
bool fp_Run::findMaxLeftFitSplitPoint(UT_sint32 /* iMaxLeftWidth */,
									  fp_RunSplitInfo& /* si */,
									  bool /* bForce */)
{
	return false;
}
#endif

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
	fp_Run* pRun = getPrev();
	while (pRun && !pRun->hasLayoutProperties())
	    pRun = pRun->getPrev();

	return pRun;
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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		_setAscentLayoutUnits(pRun->getAscentInLayoutUnits());
		_setDescentLayoutUnits(pRun->getDescentInLayoutUnits());
		_setHeightLayoutUnits(pRun->getHeightInLayoutUnits());
#endif
	}
	else
	{
		// look for fonts in this DocLayout's font cache
		//UT_DEBUGMSG(("fp_Run::_inheritProperties: from current font\n"));
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?

		//m_pBL->getSpanAttrProp(getBlockOffset(),true,&pSpanAP);
		bool bDeleteSpanAP;
		getSpanAP(pSpanAP, bDeleteSpanAP);
		getBlock()->getAttrProp(&pBlockAP);

		FL_DocLayout * pLayout = getBlock()->getDocLayout();

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);

		if (pFont != _getScreenFont())
		{
			_setScreenFont(pFont);
		    _setAscent(getGR()->getFontAscent(pFont));
			_setDescent(getGR()->getFontDescent(pFont));
		    _setHeight(getGR()->getFontHeight(pFont));
		}

		pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
		if (pFont != _getLayoutFont())
		{
		    _setLayoutFont(pFont);
		    _setAscentLayoutUnits(getGR()->getFontAscent(pFont));
		    _setDescentLayoutUnits(getGR()->getFontDescent(pFont));
		    _setHeightLayoutUnits(getGR()->getFontHeight(pFont));
		}
#elif defined (WITH_PANGO)
		PangoFont* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);

		if (pFont != _getPangoFont())
		{
			_setPangoFont(pFont);
		    _setAscent(getGR()->getFontAscent(pFont));
		    _setDescent(getGR()->getFontDescent(pFont));
		    _setHeight(getGR()->getFontHeight(pFont));
		}
#else
		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);

		if (pFont != _getFont())
		{
			_setFont(pFont);
		    _setAscent(getGR()->getFontAscent(pFont));
			_setDescent(getGR()->getFontDescent(pFont));
		    _setHeight(getGR()->getFontHeight(pFont));
		}
#endif
		if(bDeleteSpanAP)
			delete pSpanAP;
	}
}

/*!
 * This method returns the High Light color as defined for this page. It examines
 * the bgcolor property and uses that over the page color if its defined as not
 * transperent. It sets the m_pColorHL member variable.
 */
bool fp_Run::updateHighlightColor(void)
{
	if(m_pColorHL.isTransparent())
	{
		UT_RGBColor oldColor = m_pColorHL;

		fp_Page * pPage = NULL;
		fp_Line * pLine = getLine();
		if(pLine != NULL)
		{
			fp_Container * pCon = pLine->getContainer();
			if(pCon)
			{
				// Check if we are in a table cell which has a NON-transparent background
				// if it is, then use the cell backgound color
				if(pCon->getContainerType() == FP_CONTAINER_CELL && (((fp_CellContainer *) pCon)->getBgStyle() != FS_OFF))
				{
					_setColorHL(((fp_CellContainer *) pCon)->getBgColor());
					return m_pColorHL != oldColor;
				}
				pPage = pCon->getPage();
			}
		}
		if (pPage != NULL)
		{
			if (getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
				_setColorHL(*pPage->getOwningSection()->getPaperColor());
			else
			{
				UT_setColor (m_pColorHL, 255, 255, 255);
				return m_pColorHL != oldColor;
			}
		}
		else if(getBlock()->isHdrFtr())
		{
			UT_setColor (m_pColorHL, 255, 255, 255);
			return m_pColorHL != oldColor;
		}
		else
			_setColorHL(*getBlock()->getDocSectionLayout()->getPaperColor());

		return m_pColorHL != oldColor;
	}

	return false;
}

/*!
 * This method updates the page color member variable as defined for
 * this page. It is used for clearscreen() methods and so does not
 * examine the bgcolor property.
 */
bool fp_Run::updatePageColor(void)
{
	UT_RGBColor oldColor = m_pColorPG;

	UT_RGBColor * pClr = NULL;
	fp_Page * pPage = NULL;
	fp_Line * pLine = getLine();

	if(pLine != NULL)
	{
		fp_Container * pCon = pLine->getContainer();
		if(pCon)
		{

			// Check if we are in a table cell which has a NON-transparent background
			// if it is, then use the cell backgound color
			if(pCon->getContainerType() == FP_CONTAINER_CELL && (((fp_CellContainer *) pCon)->getBgStyle() != FS_OFF))
			{
				m_pColorPG = ((fp_CellContainer *) pCon)->getBgColor();
				return m_pColorPG != oldColor;
			}
			pPage = pCon->getPage();
		}
	}
	if (pPage != NULL)
	{
		pClr = pPage->getOwningSection()->getPaperColor();
	}
	else if(getBlock()->isHdrFtr())
	{
		UT_setColor (m_pColorPG, 255, 255, 255);
		return m_pColorPG != oldColor;
	}
	else
		pClr = getBlock()->getDocSectionLayout()->getPaperColor();
	UT_setColor (m_pColorPG, pClr->m_red, pClr->m_grn, pClr->m_blu);

	return m_pColorPG != oldColor;
}

/*!
 * This method updates the Highlight and Page color underneath a run.
 * It would typically be called after a change in the SectionLevel properties or
 * if a line cotaining a run is moved from one color page to another.
 */
bool fp_Run::updateBackgroundColor(void)
{
	bool h = updateHighlightColor();
	bool p = updatePageColor();
	return h || p;
}

void fp_Run::insertIntoRunListBeforeThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.setNext(this);
	if (m_pPrev)
	{
		m_pPrev->setNext(&newRun);
		if(newRun.getType()!= FPRUN_HYPERLINK)
		newRun.setHyperlink( m_pPrev->getHyperlink());
	}
	newRun.setPrev(m_pPrev);
	setPrev(&newRun);

}

void fp_Run::insertIntoRunListAfterThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.setPrev(this);
	if(newRun.getType()!= FPRUN_HYPERLINK)
		newRun.setHyperlink(m_pHyperlink);
	if (m_pNext)
	{
		m_pNext->setPrev(&newRun);
	}
	newRun.setNext(m_pNext);
	setNext(&newRun);
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
			fp_Run * pRun = getNext();

			while(pRun && pRun->getHyperlink() == pH)
			{
				pRun->setHyperlink(NULL);
				pRun = pRun->getNext();
			}
		}
	}

	if (m_pPrev)
	{
		m_pPrev->setNext(m_pNext);
	}
	if (m_pNext)
	{
		m_pNext->setPrev(m_pPrev);
		setNext(0);
	}
	setPrev(0);
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
    \param pSpan : location to store the PP_AttrProp
    \param bDeleteAfter : indicates whether the caller should free the
    pointer when no longer needed
*/
void fp_Run::getSpanAP(const PP_AttrProp * &pSpanAP, bool &bDeleteAfter)
{
	PP_AttrProp * pMySpanAP;

	getBlock()->getSpanAttrProp(getBlockOffset(),false,&pSpanAP);

	/**************************************************************************
	 * revision handling
	 * -----------------
	 *
	 * if there is a revision attribute in our span AP we have to
	 * superimpose any props contained in that attribute over the span props
	 *
	 */

	bDeleteAfter = false;

	const XML_Char* pRevision = NULL;
	if(pSpanAP && pSpanAP->getAttribute("revision", pRevision))
	{
		if(!m_pRevisions)
			m_pRevisions = new PP_RevisionAttr(pRevision);

		//next step is to parse any properties associated with this
		//revision

		const PP_Revision * pRev = m_pRevisions->getLastRevision();

		if( pRev &&
		  ((pRev->getType() == PP_REVISION_FMT_CHANGE)
		 ||(pRev->getType() == PP_REVISION_ADDITION_AND_FMT)))
		{
			// create copy of span AP and then set all props contained
			// in our revision;
			pMySpanAP = new PP_AttrProp;

			(*pMySpanAP) = *pSpanAP;
			(*pMySpanAP) = *pRev;

			pSpanAP = pMySpanAP;
			bDeleteAfter = true;
		}
	}
}

// the parameter eClearScreen has a default value AUTO
// we need this extra parameter be able to specify false when calling from
// inside of the first pass of fp_Line::layout(), which sets
// only a temporary value of iX which is then adjusted in the
// second pass, without this the run will redraw twice, once always unnecessarily
// and most of the time both times unnecessarily
void	fp_Run::setX(UT_sint32 iX, FPRUN_CLEAR_SCREEN eClearScreen)
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
	clearScreen();

	m_pLine = pLine;
	if(pLine != NULL)
		updateBackgroundColor();
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

void fp_Run::setNext(fp_Run* p, bool bRefresh)
{
	if(p != m_pNext)
	{
		m_bRefreshDrawBuffer |= bRefresh;
		m_bRecalcWidth |= bRefresh;
		// because we support 2-char ligatures, the change of next
		// can also influence the run ahead of us
		// we will just mark it
		if(m_pPrev && bRefresh)
		{
			m_pPrev->markDrawBufferDirty();
			m_pPrev->markWidthDirty();
		}

		m_pNext = p;
	}
}

void fp_Run::setPrev(fp_Run* p, bool bRefresh)
{
	if(p != m_pPrev)
	{
		m_bRefreshDrawBuffer |= bRefresh;
		m_bRecalcWidth |= bRefresh;
		// because we support 2-char ligatures, the change of prev
		// can also influence the run that follows us
		// we will just mark it
		if(m_pNext && bRefresh)
		{
			m_pNext->markDrawBufferDirty();
			m_pNext->markWidthDirty();
		}

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

void fp_Run::setLength(UT_uint32 iLen)
{
	if (iLen == getLength())
	{
		return;
	}
    m_bRecalcWidth = true;
	clearScreen();

	m_iLen = iLen;
	m_bRefreshDrawBuffer = true;
}

void fp_Run::setBlockOffset(UT_uint32 offset)
{
	m_iOffsetFirst = offset;
}

void fp_Run::clearScreen(bool bFullLineHeightRect)
{
	if(!getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		return;
	}

	if (isDirty())
	{
		// no need to clear if we've already done so.
		return;
	}

	if (!getLine())
	{
		// nothing to clear if this run is not currently on a line
		return;
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Doing clear screen in run \n"));
	if(getLine()->getContainer() != NULL)
	{
		if(getLine()->getContainer()->getPage() != 0)
		{

			_clearScreen(bFullLineHeightRect);
#if 0
			// this clears any posible underlining from revisions,
			// hyperlinks, squiggles, etc.
			UT_sint32 xoff = 0, yoff = 0;
			m_pLine->getScreenOffsets(this, xoff, yoff);
			UT_uint32 iWidth = getDrawingWidth();

			m_pG->fillRect(m_colorPG,xoff, yoff + _UL(4), iWidth, _UL(3));
#endif
			// make sure we only get erased once
			_setDirty(true);
		}
	}
}

static UT_RGBColor s_fgColor;

const UT_RGBColor fp_Run::getFGColor(void) const
{
	// revision colours
	if(m_pRevisions)
	{
		UT_uint32 iId = m_pRevisions->getLastRevision()->getId();
		s_fgColor = _getView()->getColorRevisions(iId-1);
	}
	else if(m_pHyperlink && getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		s_fgColor = _getView()->getColorHyperLink();
	}
	else
		return _getColorFG();

	return s_fgColor;
}

const UT_RGBColor fp_Run::getPageColor(void)
{
	updatePageColor(); // TODO -- why ??? Tomas
	return _getColorPG();
}

void fp_Run::draw(dg_DrawArgs* pDA)
{
	if (pDA->bDirtyRunsOnly && !m_bDirty)
		return;

	if (getLine())
		getLine()->setScreenCleared(false);

//	UT_usleep(100000); // 0.1 seconds useful for debugging
	xxx_UT_DEBUGMSG(("SEVIOR: draw this %x \n"));

	// shortcircuit drawing if we're way off base.
#ifdef USE_LAYOUT_UNITS
	long imax = (1 << 15) - 1;
#else
	// lets assume 5000 pixel screen
	long imax = _UL(5000);
#endif
	if (((pDA->yoff < -imax) || (pDA->yoff > imax)) && getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	     return;

	getGR()->setColor(getFGColor());

	_draw(pDA);

	if(m_pRevisions)
	{
		const PP_Revision * r = m_pRevisions->getLastRevision();
		PP_RevisionType r_type = r->getType();

		getGR()->setColor(getFGColor());

		UT_uint32 iWidth = getDrawingWidth();

		if(r_type == PP_REVISION_ADDITION)
		{
			getGR()->fillRect(s_fgColor,pDA->xoff, pDA->yoff, iWidth, 1);
			getGR()->fillRect(s_fgColor,pDA->xoff, pDA->yoff + 2, iWidth, 1);
		}
		else if(r_type == PP_REVISION_FMT_CHANGE)
		{
			// draw a thick line underneath
			getGR()->fillRect(s_fgColor,pDA->xoff, pDA->yoff, iWidth, 2);
		}
		else
		{
			// draw a strike-through line
			getGR()->fillRect(s_fgColor,pDA->xoff, pDA->yoff - m_iHeight/3, iWidth, 2);
		}

	}

	if(m_pHyperlink && getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// have to set the colour again, since fp_TextRun::_draw can set it to red
		// for drawing sguiggles
		getGR()->setColor(_getView()->getColorHyperLink());
		getGR()->setLineProperties(1.0,
								GR_Graphics::JOIN_MITER,
								GR_Graphics::CAP_BUTT,
								GR_Graphics::LINE_SOLID);

		getGR()->drawLine(pDA->xoff, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff);
	}

	if(m_eHidden == FP_HIDDEN_TEXT || m_eHidden == FP_HIDDEN_REVISION_AND_TEXT)
	{
		getGR()->setColor(getFGColor());
		getGR()->setLineProperties(1.0,
								GR_Graphics::JOIN_MITER,
								GR_Graphics::CAP_BUTT,
								GR_Graphics::LINE_DOTTED);

		getGR()->drawLine(pDA->xoff, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff);

	}
	_setDirty(false);
}

bool fp_Run::canContainPoint(void) const
{
	FV_View* pView = _getView();
	bool bShowHidden = pView->getShowPara();


	// if this tab is to be hidden, we must treated as if its
	// width was 0
	bool bHidden = ((m_eHidden == FP_HIDDEN_TEXT && !bShowHidden)
	              || m_eHidden == FP_HIDDEN_REVISION
		          || m_eHidden == FP_HIDDEN_REVISION_AND_TEXT);

	if(bHidden)
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
	return true;
}

void fp_Run::fetchCharWidths(fl_CharWidths * /* pgbCharWidths */)
{
	// do nothing.  subclasses may override this.
}

bool fp_Run::recalcWidth(void)
{
	// do nothing.  subclasses may override this.
	return false;
}

const PP_AttrProp* fp_Run::getAP(void) const
{
	const PP_AttrProp * pSpanAP = NULL;

	getBlock()->getSpanAttrProp(getBlockOffset(),false,&pSpanAP);

	return pSpanAP;
}


void fp_Run::drawDecors(UT_sint32 xoff, UT_sint32 yoff)
{

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

	const UT_sint32 old_LineWidth = m_iLineWidth;
	UT_sint32 cur_linewidth = 1+ (UT_MAX(10,getAscent())-10)/8;
//
// Line thickness is too thick.
//
	cur_linewidth = UT_MAX(1,cur_linewidth/2);
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
			iDrop = yoff + getAscent() + getDescent()/3;
			setUnderlineXoff( xoff);
			setMaxUnderline(iDrop);
		}
		if(b_Overline)
		{
			iDrop = yoff + 1 + (UT_MAX(10,getAscent()) - 10)/8;
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
			  iDrop = yoff + getAscent() + m_iDescent/3;
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
		     iDrop = yoff + 1 + (UT_MAX(10,getAscent()) - 10)/8;
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
	getGR()->setLineWidth(m_iLineWidth);
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
			getGR()->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
		if ( b_Overline)
		{
			iDrop = UT_MIN( getMinOverline(), iDrop);
			UT_sint32 totx = getOverlineXoff();
			getGR()->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
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
		     if(!N_Run->isUnderline())
		     {
				 iDrop = UT_MAX( getMaxUnderline(), iDrop);
				 UT_sint32 totx = getUnderlineXoff();
				 getGR()->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		     }
		     else
		     {
		          N_Run->markAsDirty();
		     }
		}
		if ( b_Overline )
		{
			if(!N_Run->isOverline())
			{
				iDrop = UT_MIN( getMinOverline(), iDrop);
				UT_sint32 totx = getOverlineXoff();
				getGR()->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
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
		getGR()->drawLine(xoff, iDrop, xoff+getWidth(), iDrop);
	}
	/*
	   Restore the previous line width.
	*/
	_setLineWidth(old_LineWidth);
	getGR()->setLineWidth(_getLineWidth());
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
	getBlock()->getSpanAttrProp(getBlockOffset(),false,&pSpanAP);
	getBlock()->getAttrProp(&pBlockAP);
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);

	// This gives the baseline of the selection.
	// need to clear full height of line, in case we had a selection
//	UT_sint32 xxoff=0 ,ybase =0;
//	getLine()->getScreenOffsets(this, xxoff, ybase);


	if ( b_Topline)
	{
		UT_sint32 ybase = yoff + getAscent() - getLine()->getAscent() + 1;
		getGR()->fillRect(clrFG, xoff, ybase, getWidth(), ithick);
	}
	/*
	  We always draw bottomline right at the bottom so there is no ambiguity
	*/
	if ( b_Bottomline)
	{
		getGR()->fillRect(clrFG, xoff, yoff+getLine()->getHeight()-ithick+1, getWidth(), ithick);
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

UT_sint32 fp_Run::getUnderlineXoff(void)
{
	return m_iUnderlineXoff;
}

void fp_Run::setOverlineXoff(UT_sint32 xoff)
{
	m_iOverlineXoff = xoff;
}

UT_sint32 fp_Run::getOverlineXoff(void)
{
	return m_iOverlineXoff;
}

void fp_Run::setMaxUnderline(UT_sint32 maxh)
{
	m_imaxUnderline = maxh;
}

UT_sint32 fp_Run::getMaxUnderline(void)
{
	return m_imaxUnderline;
}

void fp_Run::setMinOverline(UT_sint32 minh)
{
	m_iminOverline = minh;
}

UT_sint32 fp_Run::getMinOverline(void)
{
	return m_iminOverline;
}

UT_sint32 fp_Run::getLinethickness( void)
{
	return m_iLinethickness;
}

UT_sint32 fp_Run::getToplineThickness(void)
{
	return getGR()->convertDimension("0.8pt");
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

    GR_Font *pFont = getGR()->getGUIFont();
    getGR()->setFont(pFont);

    UT_uint32 iTextLen = UT_UCS4_strlen(pText);
    UT_uint32 iTextWidth = getGR()->measureString(pText,0,iTextLen,NULL);
    UT_uint32 iTextHeight = getGR()->getFontHeight(pFont);

    UT_uint32 xoffText = xoff + (iWidth - iTextWidth) / 2;
    UT_uint32 yoffText = yoff - getGR()->getFontAscent(pFont) * 2 / 3;

    getGR()->drawLine(xoff,yoff,xoff + iWidth,yoff);

    if((iTextWidth < iWidth) && (iTextHeight < iHeight)){
        getGR()->fillRect(_getColorHL(),xoffText,yoffText,iTextWidth,iTextHeight);
        getGR()->drawChars(pText,0,iTextLen,xoffText,yoffText);
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_TabRun::fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TAB)
{
	lookupProperties();
}

void fp_TabRun::_lookupProperties(const PP_AttrProp * pSpanAP,
								  const PP_AttrProp * pBlockAP,
								  const PP_AttrProp * pSectionAP)
{
	bool bChanged = false;

	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);

	UT_RGBColor clrFG;
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true), clrFG);
	bChanged |= (clrFG != _getColorFG());
	_setColorFG(clrFG);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);

	if (pFont != _getScreenFont())
	{
	    _setScreenFont(pFont);
	    _setAscent(getGR()->getFontAscent(pFont));
	    _setDescent(getGR()->getFontDescent(pFont));
	    _setHeight(getGR()->getFontHeight(pFont));
		bChanged = true;
	}

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);

	if (pFont != _getLayoutFont())
	{
	    _setLayoutFont(pFont);
	    _setAscentLayoutUnits(getGR()->getFontAscent(pFont));
	    _setDescentLayoutUnits(getGR()->getFontDescent(pFont));
	    _setHeightLayoutUnits(getGR()->getFontHeight(pFont));
		bChanged = true;
	}
#elif defined (WITH_PANGO)
	PangoFont* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);

	if (pFont != _getPangoFont())
	{
	    _setPangoFont(pFont);
	    _setAscent(getGR()->getFontAscent(pFont));
	    _setDescent(getGR()->getFontDescent(pFont));
	    _setHeight(getGR()->getFontHeight(pFont));
		bChanged = true;
	}
#else
	GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);

	if (pFont != _getFont())
	{
	    _setFont(pFont);
	    _setAscent(getGR()->getFontAscent(pFont));
	    _setDescent(getGR()->getFontDescent(pFont));
	    _setHeight(getGR()->getFontHeight(pFont));
		bChanged = true;
	}
#endif

	if(getDirection() != FRIBIDI_TYPE_WS)
	{
		_setDirection(FRIBIDI_TYPE_WS);
		bChanged = true;
		//setDirectionProperty(FRIBIDI_TYPE_WS);
	}
//
// Lookup Decoration properties for this run
//
	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  getBlock()->getDocument(), true);
	_setLineWidth(getToplineThickness());

	UT_uint32 oldDecors = _getDecorations();
	_setDecorations(0);
	XML_Char* p;
	if (!UT_cloneString((char *&)p, pszDecor))
	{
		// TODO outofmem
	}
	UT_ASSERT(p || !pszDecor);
	XML_Char*	q = strtok(p, " ");

	while (q)
	{
		if (0 == UT_strcmp(q, "underline"))
		{
			_orDecorations(TEXT_DECOR_UNDERLINE);
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			_orDecorations(TEXT_DECOR_OVERLINE);
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
			_orDecorations(TEXT_DECOR_LINETHROUGH);
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			_orDecorations(TEXT_DECOR_TOPLINE);
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			_orDecorations(TEXT_DECOR_BOTTOMLINE);
		}
		q = strtok(NULL, " ");
	}
	free(p);

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

bool fp_TabRun::letPointPass(void) const
{
	return true;
}

bool fp_TabRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_TabRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
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

	getLine()->getOffsets(this, xoff, yoff);

	fp_Run * pRun = 0;
	UT_sint32 iNextDir = getVisDirection();

	if (iOffset == (getBlockOffset() + getLength()))  //#TF is this the right-most logical element of the run?
	{
	    pRun = getNext();
	    if(pRun)
	    {
	        pRun->getLine()->getOffsets(pRun, xoff2, yoff2);
	        iNextDir = pRun->getVisDirection();
	    }
	}

	UT_sint32 iDirection = getVisDirection();

    x = xoff;

	if(iDirection == FRIBIDI_TYPE_LTR)
	{
		xxx_UT_DEBUGMSG(("iOffset %d, getBlockOffset() %d, getLength() %d\n", iOffset,getBlockOffset(),getLength()));
		if(iOffset != getBlockOffset())
		{
			UT_ASSERT(iOffset == (getBlockOffset() + getLength()));
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
	    x2 = (iNextDir == FRIBIDI_TYPE_LTR) ?  xoff + pRun->getWidth() : xoff2;
	    y2 = yoff2;
	}
	else
	{
	    x2 = x;
	    y2 = yoff;
	}

	bDirection = (iDirection != FRIBIDI_TYPE_LTR);
	y = yoff;
	height = getHeight();
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
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	getGR()->fillRect(_getColorPG(),xoff, yoff, getWidth(), getLine()->getHeight());
}

void fp_TabRun::_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight)
{
    if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

#define NPOINTS 6

    UT_Point points[NPOINTS];

    UT_sint32 cur_linewidth = 1 + (UT_MAX(10,getAscent()) - 10) / 8;
    UT_uint32 iyAxis = iTop + getLine()->getAscent() * 2 / 3;
    UT_uint32 iMaxWidth = UT_MIN(iWidth / 10 * 6, (UT_uint32) cur_linewidth * 9);
    UT_uint32 ixGap = (iWidth - iMaxWidth) / 2;

	//UT_DEBUGMSG(("iLeft %d, iWidth %d, visDir \"%s\"\n", iLeft,iWidth, getVisDirection() == FRIBIDI_TYPE_LTR ? "ltr":"rtl"));
	if(getVisDirection() == FRIBIDI_TYPE_LTR)
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

    UT_RGBColor clrShowPara = _getView()->getColorShowPara();
    getGR()->polygon(clrShowPara,points,NPOINTS);

    xxx_UT_DEBUGMSG(("fp_TabRun::_drawArrow: iLeft %d, iyAxis %d, cur_linewidth %d, iMaxWidth %d\n",
    			iLeft, iyAxis, cur_linewidth, iMaxWidth));

    // only draw the rectangle if iMaxWidth - cur_linewidth * 4 > 0, otherwise
    // we get the rect running pass the end of the line and off the screen
    if((UT_sint32)(iMaxWidth - cur_linewidth * 4) > 0)
	    if(getVisDirection() == FRIBIDI_TYPE_LTR)
		{
		    getGR()->fillRect(clrShowPara,iLeft + ixGap,iyAxis - cur_linewidth / 2,iMaxWidth - cur_linewidth * 4,cur_linewidth);
		}
		else
		{
	    	getGR()->fillRect(clrShowPara,iLeft + ixGap + cur_linewidth * 4,iyAxis - cur_linewidth / 2,iMaxWidth - cur_linewidth * 4,cur_linewidth);
		}
#undef NPOINTS
}

void fp_TabRun::_draw(dg_DrawArgs* pDA)
{
	xxx_UT_DEBUGMSG(("fp_TabRun::_draw (0x%x)\n",this));
	UT_ASSERT(pDA->pG == getGR());

	UT_RGBColor clrNormalBackground(_getColorHL());
	// need to draw to the full height of line to join with line above.
	UT_sint32 xoff= 0, yoff=0, DA_xoff = pDA->xoff;

	getLine()->getScreenOffsets(this, xoff, yoff);

	UT_sint32 iFillHeight = getLine()->getHeight();
	UT_sint32 iFillTop = pDA->yoff - getLine()->getAscent();

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
	getBlock()->getSpanAttrProp(getBlockOffset(),false,&pSpanAP);
	getBlock()->getAttrProp(&pBlockAP);
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);
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

		
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		getGR()->setFont(_getScreenFont());
#else
		getGR()->setFont(_getFont());
#endif
		getGR()->measureString(tmp, 1, 150, wid);
		// one would think that one could measure
		// one character and divide the needed
		// width by that; would one be so wrong?
		// we're not dealing with different letters
		// here, after all.

		i = 1;
		cumWidth = 0;
		while (cumWidth < getWidth() && i < 151)
			cumWidth += wid[i++];

		i = (i>=3) ? i - 2 : 1;
		getGR()->setColor(clrFG);
		getGR()->drawChars(tmp, 1, i, /*pDA->xoff*/DA_xoff, iFillTop);
	}
	else
	if (
		pView->getFocus()!=AV_FOCUS_NONE &&
		(iSel1 <= iRunBase)
		&& (iSel2 > iRunBase)
		)
	{
		getGR()->fillRect(_getView()->getColorSelBackground(), /*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
        if(pView->getShowPara()){
            _drawArrow(/*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
        }
	}
	else
	{
		getGR()->fillRect(clrNormalBackground, /*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
        if(pView->getShowPara()){
            _drawArrow(/*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
        }
	}
//
// Draw underline/overline/strikethough
//
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove
	                                                 //character dirt
	drawDecors( xoff, yTopOfRun);
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
		UT_sint32 ithick =  getToplineThickness();
		getGR()->fillRect(clrFG, /*pDA->xoff*/DA_xoff+getWidth()-ithick, iFillTop, ithick, iFillHeight);
	}
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedLineBreakRun::fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDLINEBREAK)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun constructor\n"));
	lookupProperties();
}

void fp_ForcedLineBreakRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun::lookupProperties\n"));
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(),fd);
	_setField(fd);

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
			getGR()->setFont(pTextRun->getFont());
		}
		else
		{
			// look for fonts in this DocLayout's font cache
			FL_DocLayout * pLayout = getBlock()->getDocLayout();

			GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,
											   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
			getGR()->setFont(pFont);
		}
		_setWidth(getGR()->measureString(pEOP, 0, iTextLen, NULL));
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

bool fp_ForcedLineBreakRun::letPointPass(void) const
{
	return false;
}

void fp_ForcedLineBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun::mapXYToPosition\n"));
	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_ForcedLineBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
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
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedLineBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());

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

	UT_ASSERT(pDA->pG == getGR());

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (pView->getFocus()!=AV_FOCUS_NONE &&	(iSel1 <= iRunBase) && (iSel2 > iRunBase))
		bIsSelected = true;

	UT_RGBColor clrShowPara = pView->getColorShowPara();

	//UT_UCSChar pEOP[] = { UCS_LINESEP, 0 };
	UT_UCSChar pEOP[] = { '^', 'l', 0 };
	UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);
	UT_sint32 iAscent;

	fp_Run* pPropRun = _findPrevPropertyRun();
	if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
    {
		fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
		getGR()->setFont(pTextRun->getFont());
		iAscent = pTextRun->getAscent();
    }
	else
    {
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;
		getBlock()->getSpanAttrProp(getBlockOffset(),true,&pSpanAP);
		getBlock()->getAttrProp(&pBlockAP);
		// look for fonts in this DocLayout's font cache
		FL_DocLayout * pLayout = getBlock()->getDocLayout();

		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,
										   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
		getGR()->setFont(pFont);
		iAscent = getGR()->getFontAscent();
    }

	// if we currently have a 0 width, i.e., we draw in response to the
	// showPara being turned on, then we obtain the new width, and then
	// tell the line to redo its layout and redraw instead of drawing ourselves
	//	bool bWidthChange = false;
	//	if(!getWidth())
	//		bWidthChange = true;

	_setWidth(getGR()->measureString(pEOP, 0, iTextLen, NULL));
	// 	if(bWidthChange)
	//	{
	//		getLine()->layout();
	//		getLine()->redrawUpdate();
	//		return;
	//	}

	_setHeight(getGR()->getFontHeight());
	iXoffText = pDA->xoff;

	if(getBlock()->getDominantDirection() == FRIBIDI_TYPE_RTL)
    {
		iXoffText -= getWidth();
    }

	iYoffText = pDA->yoff - iAscent;
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::draw: width %d\n", getWidth()));

	if (bIsSelected)
    {

		getGR()->fillRect(_getView()->getColorSelBackground(), iXoffText, iYoffText, getWidth(), getLine()->getHeight());
    }
	else
    {
		getGR()->fillRect(_getColorPG(), iXoffText, iYoffText, getWidth(), getLine()->getHeight());
    }
	if (pView->getShowPara())
    {
		// Draw pilcrow
		getGR()->setColor(clrShowPara);
		getGR()->drawChars(pEOP, 0, iTextLen, iXoffText, iYoffText);
    }

}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldStartRun::fp_FieldStartRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELDSTARTRUN)
{
	lookupProperties();
}

void fp_FieldStartRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
										 const PP_AttrProp * /*pBlockAP*/,
										 const PP_AttrProp * /*pSectionAP*/)
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

bool fp_FieldStartRun::letPointPass(void) const
{
	return true;
}

void fp_FieldStartRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}


void fp_FieldStartRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldStartRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldStartRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldEndRun::fp_FieldEndRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELDENDRUN)
{
	lookupProperties();
}

void fp_FieldEndRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/)
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

bool fp_FieldEndRun::letPointPass(void) const
{
	return true;
}

void fp_FieldEndRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	pos = getBlock()->getPosition() + getBlockOffset();
	bBOL = false;
	bEOL = false;
}

void fp_FieldEndRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldEndRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldEndRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



fp_BookmarkRun::fp_BookmarkRun( fl_BlockLayout* pBL,
								GR_Graphics* pG,
								UT_uint32 iOffsetFirst,
								UT_uint32 /*iLen*/)
	: fp_Run(pBL, pG, iOffsetFirst, 1, FPRUN_BOOKMARK)
{
	m_pBookmark = getBlock()->getBookmark(iOffsetFirst);
	UT_ASSERT(m_pBookmark);

	_setLength(1);
	_setDirty(true);

	UT_ASSERT((pBL));
	_setDirection(FRIBIDI_TYPE_WS);

	m_bIsStart = (po_Bookmark::POBOOKMARK_START == m_pBookmark->getBookmarkType());

	// have to cache the name, since we will need to use it for a while
	// after the associated PT fragment has been deleted.
	UT_XML_strncpy(m_pName, BOOKMARK_NAME_SIZE, m_pBookmark->getName());
	m_pName[BOOKMARK_NAME_SIZE] = 0;
}

bool fp_BookmarkRun::isComrade(fp_BookmarkRun *pBR) const
{
	UT_ASSERT(m_pName && *m_pName && pBR->m_pName && *pBR->m_pName);
	return (0 == UT_XML_strcmp(m_pName, pBR->m_pName));
}

void fp_BookmarkRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/)
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

bool fp_BookmarkRun::letPointPass(void) const
{
	return true;
}

void fp_BookmarkRun::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	fp_Run *pRun = getNext();
	UT_ASSERT(pRun);
	pRun->mapXYToPosition(x, y, pos, bBOL, bEOL);
}

void fp_BookmarkRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y,  UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	fp_Run * pRun = getNext();
	UT_ASSERT(pRun);

	pRun->findPointCoords(iOffset, x, y,  x2, y2, height, bDirection);
}

void fp_BookmarkRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

   	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return;
    }


	UT_sint32 xoff = 0, yoff = 0;
	getLine()->getScreenOffsets(this, xoff, yoff);

	if(m_bIsStart)
		getGR()->fillRect(_getColorPG(), xoff, yoff, 4, 8);
	else
		getGR()->fillRect(_getColorPG(), xoff - 4, yoff, 4, 8);

}

void fp_BookmarkRun::_draw(dg_DrawArgs* pDA)
{
    if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

   	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return;
    }

	UT_ASSERT(pDA->pG == getGR());

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (pView->getFocus()!=AV_FOCUS_NONE &&	(iSel1 <= iRunBase) && (iSel2 > iRunBase))
		bIsSelected = true;

	getGR()->setColor(_getView()->getColorShowPara());


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


    UT_RGBColor clrShowPara = _getView()->getColorShowPara();
    getGR()->polygon(clrShowPara,points,NPOINTS);
    #undef NPOINTS

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_HyperlinkRun::fp_HyperlinkRun( fl_BlockLayout* pBL,
								GR_Graphics* pG,
								UT_uint32 iOffsetFirst,
								UT_uint32 /*iLen*/)
	: fp_Run(pBL, pG, iOffsetFirst, 1, FPRUN_HYPERLINK)
{
	_setLength(1);
	_setDirty(false);

	UT_ASSERT((pBL));
	_setDirection(FRIBIDI_TYPE_WS);

	const PP_AttrProp * pAP = NULL;

	getBlock()->getSpanAttrProp(getBlockOffset(),false,&pAP);
	const XML_Char * pTarget;
	const XML_Char * pName;
	bool bFound = false;
	UT_uint32 k = 0;

	while(pAP->getNthAttribute(k++, pName, pTarget))
	{
		bFound = (0 == UT_XML_strnicmp(pName,"xlink:href",10));
		if(bFound)
			break;
	}

	// we have got to keep a local copy, since the pointer we get
	// is to a potentially volatile location
	if(bFound)
	{
		UT_uint32 iTargetLen = UT_XML_strlen(pTarget);
		m_pTarget = new XML_Char [iTargetLen + 1];
		UT_XML_strncpy(m_pTarget, iTargetLen + 1, pTarget);
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


fp_HyperlinkRun::~fp_HyperlinkRun()
{
	if(m_pTarget)
		delete [] m_pTarget;
}

void fp_HyperlinkRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
									   const PP_AttrProp * /*pBlockAP*/,
									   const PP_AttrProp * /*pSectionAP*/)
{
}

bool fp_HyperlinkRun::canBreakAfter(void) const
{
	return false;
}

bool fp_HyperlinkRun::canBreakBefore(void) const
{
	return false;
}

bool fp_HyperlinkRun::letPointPass(void) const
{
	return true;
}

void fp_HyperlinkRun::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	fp_Run *pRun = getNext();
	UT_ASSERT(pRun);
	pRun->mapXYToPosition(x, y, pos, bBOL, bEOL);
}

void fp_HyperlinkRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y,  UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	fp_Run * pRun = getNext();
	UT_ASSERT(pRun);

	pRun->findPointCoords(iOffset, x, y,  x2, y2, height, bDirection);
}

void fp_HyperlinkRun::_clearScreen(bool /* bFullLineHeightRect */)
{
}

void fp_HyperlinkRun::_draw(dg_DrawArgs* /*pDA*/)
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
fp_EndOfParagraphRun::fp_EndOfParagraphRun(fl_BlockLayout* pBL,
										   GR_Graphics* pG, UT_uint32 iOffsetFirst,
										   UT_uint32 iLen)
	: fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_ENDOFPARAGRAPH)
{

	_setLength(1);
	_setDirty(true);

	UT_ASSERT((pBL));
	_setDirection(pBL->getDominantDirection());
	lookupProperties();
}


bool fp_EndOfParagraphRun::recalcWidth(void)
{
	return false;
}

void fp_EndOfParagraphRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP)
{
	//UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties\n"));
	_inheritProperties();

	const XML_Char* pRevision = NULL;

	if(pBlockAP && pBlockAP->getAttribute("revision", pRevision))
	{
		// we will not in fact be doing anything with the actual
		// properties and attributes contained in the revision
		// we just need its representation so the base class can
		// handle us properly
		PP_RevisionAttr * pRev = getRevisions();

		if(pRev)
			delete pRev;

		_setRevisions(new PP_RevisionAttr(pRevision));
	}

	FV_View* pView = _getView();
	if (pView && pView->getShowPara())
	{
		// Find width of Pilcrow
		UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
		UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);

		fp_Run* pPropRun = _findPrevPropertyRun();
		if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
			getGR()->setFont(pTextRun->getFont());
		}
		else
		{
			// look for fonts in this DocLayout's font cache
			FL_DocLayout * pLayout = getBlock()->getDocLayout();

			GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,
											   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
			getGR()->setFont(pFont);
		}
		m_iDrawWidth  = getGR()->measureString(pEOP, 0, iTextLen, NULL);
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

bool fp_EndOfParagraphRun::letPointPass(void) const
{
	return false;
}

void fp_EndOfParagraphRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
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

	if (pPropRun)
	{
		height = pPropRun->getHeight();
		// If property Run is on the same line, get y location from
		// it (to reflect proper ascent).
		if (pPropRun->getLine() == getLine())
		{
			if(FPRUN_TEXT == pPropRun->getType())
			{
				pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
				return;
			}
		}
	}

	getLine()->getOffsets(this, x, y);
	x2 = x;
	y2 = y;
}

void fp_EndOfParagraphRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	getLine()->getScreenOffsets(this, xoff, yoff);

	if(getBlock()->getDominantDirection() == FRIBIDI_TYPE_RTL)
	{
		xoff -= m_iDrawWidth;
	}
	getGR()->fillRect(_getColorPG(), xoff, yoff+1, m_iDrawWidth, getLine()->getHeight()+1);
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

	UT_ASSERT(pDA->pG == getGR());

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (pView->getFocus()!=AV_FOCUS_NONE &&	(iSel1 <= iRunBase) && (iSel2 > iRunBase))
		bIsSelected = true;

	UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
	UT_uint32 iTextLen = UT_UCS4_strlen(pEOP);
	UT_sint32 iAscent;

	fp_Run* pPropRun = _findPrevPropertyRun();
	if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
	{
		fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
		getGR()->setFont(pTextRun->getFont());
		iAscent = pTextRun->getAscent();
	}
	else
	{
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;
		getBlock()->getSpanAttrProp(getBlockOffset(),true,&pSpanAP);
		getBlock()->getAttrProp(&pBlockAP);
		// look for fonts in this DocLayout's font cache
		FL_DocLayout * pLayout = getBlock()->getDocLayout();

		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,
										   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
		getGR()->setFont(pFont);
		iAscent = getGR()->getFontAscent();
	}

	// if we currently have a 0 width, i.e., we draw in response to the
	// showPara being turned on, then we obtain the new width, and then
	// tell the line to redo its layout and redraw instead of drawing ourselves
//	bool bWidthChange = false;
//	if(!m_iDrawWidth)
//		bWidthChange = true;

	m_iDrawWidth  = getGR()->measureString(pEOP, 0, iTextLen, NULL);
// 	if(bWidthChange)
//	{
//		getLine()->layout();
//		getLine()->redrawUpdate();
//		return;
//	}

	_setHeight(getGR()->getFontHeight());
	m_iXoffText = pDA->xoff;

	if(getBlock()->getDominantDirection() == FRIBIDI_TYPE_RTL)
	{
		m_iXoffText -= m_iDrawWidth;
	}

	m_iYoffText = pDA->yoff - iAscent;
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::draw: width %d\n", m_iDrawWidth));

	if (bIsSelected)
	{
		getGR()->fillRect(_getView()->getColorSelBackground(), m_iXoffText, m_iYoffText, m_iDrawWidth, getLine()->getHeight());
	}
	else
	{
		getGR()->fillRect(_getColorPG(), m_iXoffText, m_iYoffText, m_iDrawWidth, getLine()->getHeight());
	}
	if (pView->getShowPara())
	{
		// Draw pilcrow
		// use the hard-coded colour only if not revised
		if(!getRevisions())
			getGR()->setColor(pView->getColorShowPara());
        getGR()->drawChars(pEOP, 0, iTextLen, m_iXoffText, m_iYoffText);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


fp_ImageRun::fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, FG_Graphic * pFG) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_IMAGE)
{
#if 0	// put this back later
	UT_ASSERT(pImage);
#endif

	m_pFGraphic = pFG;
	m_pImage = pFG->generateImage(pG);
	m_WidthProp = pFG->getWidthProp();
	m_HeightProp = pFG->getHeightProp();
	lookupProperties();
}

fp_ImageRun::~fp_ImageRun()
{
	if (m_pImage)
	{
		delete m_pImage;
	}
	if(m_pFGraphic)
	{
		delete m_pFGraphic;
	}
}

void fp_ImageRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * /*pBlockAP*/,
									const PP_AttrProp * /*pSectionAP*/)
{
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset(), fd);
	_setField(fd);
	const XML_Char * szWidth = NULL;
	pSpanAP->getProperty("width", szWidth);
	if(szWidth == NULL)
	{
		szWidth = "0in";
	}
	const XML_Char * szHeight = NULL;
	pSpanAP->getProperty("height", szHeight);
	if(szHeight == NULL)
	{
		szHeight = "0in";
	}

	if((strcmp(m_WidthProp.c_str(),szWidth) != 0) ||
	   (strcmp(m_HeightProp.c_str(),szHeight) != 0))
	{
		m_WidthProp = szWidth;
		m_HeightProp = szHeight;
		DELETEP(m_pImage);
		m_pImage = m_pFGraphic->generateImage(getGR(),pSpanAP);
		markAsDirty();
		if(getLine())
		{
			getLine()->setNeedsRedraw();
		}
	}
	if (m_pImage)
	{
		_setWidth(m_pImage->getDisplayWidth());
		_setHeight(m_pImage->getDisplayHeight());
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		_setWidthLayoutUnits(m_pImage->getLayoutWidth());
		_setHeightLayoutUnits(m_pImage->getLayoutHeight());
#endif
	}
	else
	{
		// If we have no image, we simply insert a square "slug"

		_setWidth(getGR()->convertDimension("0.5in"));
		_setHeight(getGR()->convertDimension("0.5in"));
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		_setWidthLayoutUnits(UT_convertToLayoutUnits("0.5in"));
		_setHeightLayoutUnits(UT_convertToLayoutUnits("0.5in"));
#endif
	}

	UT_ASSERT(getWidth() > 0);
	UT_ASSERT(getHeight() > 0);
	m_iImageWidth = getWidth();
	m_iImageHeight = getHeight();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	m_iImageWidthLayoutUnits = getWidthInLayoutUnits();
	m_iImageHeightLayoutUnits = getHeightInLayoutUnits();
#endif

//
// This code deals with too big images
//
//
	if(getLine() != NULL)
	{
		if(getLine()->getMaxWidth() - 1 < getWidth())
		{
			double dw = (double) getLine()->getMaxWidth();
			double rat = (dw - 1.0)/dw;
			_setWidth(getLine()->getMaxWidth()-1);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			double dwL = (double) getLine()->getMaxWidthInLayoutUnits();
			double dnwL = dwL - dwL*rat;
			_setWidthLayoutUnits((UT_sint32) dnwL);
#endif
		}
		if(getLine()->getContainer() != NULL &&
		   static_cast<fp_VerticalContainer *>(getLine()->getContainer())->getMaxHeight() - 1 < getHeight())
		{
			double dh = (double) static_cast<fp_VerticalContainer *>(getLine()->getContainer())->getMaxHeight();
			_setHeight(static_cast<fp_VerticalContainer *>(getLine()->getContainer())->getMaxHeight() -1);
			double rat = (dh - 1.0)/dh;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			double dhL = (double) static_cast<fp_VerticalContainer *>(getLine()->getContainer())->getMaxHeightInLayoutUnits();
			double dnhL = dhL - dhL*rat;
			_setHeightLayoutUnits((UT_sint32) dnhL);
#endif
		}
	}
	_setAscent(getHeight());
	_setDescent(0);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	_setAscentLayoutUnits(getHeightInLayoutUnits());
	_setDescentLayoutUnits(0);
#endif
}

bool fp_ImageRun::canBreakAfter(void) const
{
	return true;
}

bool fp_ImageRun::canBreakBefore(void) const
{
	return true;
}

bool fp_ImageRun::letPointPass(void) const
{
	return false;
}

bool fp_ImageRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_ImageRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
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
	}
	y = yoff;
	height = getHeight();
	y2 = y;
	bDirection = (getVisDirection() != FRIBIDI_TYPE_LTR);
}

void fp_ImageRun::_clearScreen(bool  bFullLineHeightRect )
{
	UT_ASSERT(!isDirty());

	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();

	getGR()->fillRect(_getColorPG(),xoff, yoff, getWidth(), iLineHeight);
}

const char * fp_ImageRun::getDataId(void) const
{
	return m_pFGraphic->getDataId();
}

void fp_ImageRun::_drawResizeBox(UT_Rect box)
{
	UT_sint32 left = box.left;
	UT_sint32 top = box.top;
	UT_sint32 right = box.left + box.width;
	UT_sint32 bottom = box.top + box.height;
	
	// draw some really fancy box here
	getGR()->setColor(UT_RGBColor(98,129,131));
	getGR()->drawLine(left, box.top, right-1, box.top);
	getGR()->drawLine(left, box.top, left, bottom-1);
	getGR()->setColor(UT_RGBColor(230,234,238));
	getGR()->drawLine(box.left+1, box.top+1, right - 2, top+1);
	getGR()->drawLine(box.left+1, box.top+1, left+1, bottom - 2);
	getGR()->setColor(UT_RGBColor(98,129,131));
	getGR()->drawLine(right-2, top+1, right-2, bottom-2);
	getGR()->drawLine(left+1, bottom-2, right, bottom-2);
	getGR()->setColor(UT_RGBColor(49,85,82));
	getGR()->drawLine(right-1, top, right-1, bottom-1);
	getGR()->drawLine(left, bottom-1, right, bottom-1);
	getGR()->fillRect(UT_RGBColor(156,178,180),box.left + 2, box.top + 2, box.width - 4, box.height - 4);
}

void fp_ImageRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());

	UT_sint32 xoff = 0, yoff = 0;
	getLine()->getScreenOffsets(this, xoff, yoff);

//
// Sevior's infamous + 1....
	yoff += getLine()->getAscent() - getAscent() + 1;

	// clip drawing to the page
	UT_Rect pClipRect;
	pClipRect.top = yoff;
	pClipRect.left = xoff;
	pClipRect.height = getLine()->getContainer()->getHeight();
	pClipRect.width = getLine()->getContainer()->getWidth();
	//
	// SEVIOR Says don't touch this if statement unless you know how to make windows
	// and gnome-print print images. Otherwise your commit priviliges will be revoked.
	//
	if(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
	    getGR()->setClipRect(&pClipRect);
	}

	FV_View* pView = _getView();
	if (m_pImage)
	{
		// draw the image (always)
		xxx_UT_DEBUGMSG(("SEVIOR: Drawing image now \n"));
		getGR()->drawImage(m_pImage, xoff, yoff);

		// if we're the selection, draw some pretty selection markers
		if (getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

			UT_uint32 iSelAnchor = pView->getSelectionAnchor();
			UT_uint32 iPoint = pView->getPoint();

			UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
			UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

			UT_ASSERT(iSel1 <= iSel2);

			if (
				pView->getFocus()!=AV_FOCUS_NONE &&
				(iSel1 <= iRunBase)
				&& (iSel2 > iRunBase)
				)
			{
				UT_uint32 top = yoff;
				UT_uint32 left = xoff;
				UT_uint32 right = xoff + getWidth() - 1;
				UT_uint32 bottom = yoff + getHeight() - 1;

				UT_sint32 boxSize = pView->getImageSelInfo();
			
				// now, draw the resize boxes around the image
	
				_drawResizeBox(UT_Rect(left, top, boxSize, boxSize));
				_drawResizeBox(UT_Rect(left + (right - left)/2 - boxSize/2, top, boxSize, boxSize)); 				// North
				_drawResizeBox(UT_Rect(right-boxSize+1, top, boxSize, boxSize)); 									// North East
				_drawResizeBox(UT_Rect(right-boxSize+1, top + ((bottom - top) / 2) - boxSize/2, boxSize, boxSize)); // East
				_drawResizeBox(UT_Rect(right-boxSize+1, bottom - boxSize + 1, boxSize, boxSize)); 					// South East
				_drawResizeBox(UT_Rect(left + (right - left)/2 - boxSize/2, bottom - boxSize + 1, boxSize, boxSize));// South
				_drawResizeBox(UT_Rect(left, bottom - boxSize + 1, boxSize, boxSize)); 								// South West
				_drawResizeBox(UT_Rect(left, top + ((bottom - top) / 2) - boxSize/2, boxSize, boxSize)); 			// West
			}
		}

	}
	else
	{
		getGR()->fillRect(pView->getColorImage(), xoff, yoff, getWidth(), getHeight());
	}

	// unf*ck clipping rect
	getGR()->setClipRect(NULL);
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

fp_FieldRun::fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen)
	:	fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELD),
		m_pFont(0),
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_pFontLayout(0),
#endif
		m_iFieldType(FPFIELD_start),
		m_pParameter(0)
{
	fd_Field * fd;
	lookupProperties();
	bool gotField = pBL->getField(iOffsetFirst,fd);
	_setField(fd);
	UT_ASSERT(gotField);
	m_sFieldValue[0] = 0;
}

bool fp_FieldRun::recalcWidth()
{
	UT_GrowBufElement aCharWidths[FPFIELD_MAX_LENGTH];
	// TODO -- is this really needed ???
	// this should not be needed, since lookup properties is called
	// when formatting changes - Tomas
	//lookupProperties();

#ifndef WITH_PANGO
	getGR()->setFont(m_pFont);
#else
	getGR()->setFont(_getPangoFont());
#endif

	UT_sint32 iNewWidth = getGR()->measureString(m_sFieldValue, 0, UT_UCS4_strlen(m_sFieldValue), aCharWidths);
	xxx_UT_DEBUGMSG(("fp_FieldRun::recalcWidth: old width %d, new width %d\n", getWidth(), iNewWidth));
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

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		getGR()->setFont(m_pFontLayout);
		_setWidthLayoutUnits(getGR()->measureString(m_sFieldValue, 0, UT_UCS4_strlen(m_sFieldValue), aCharWidths));
#endif
		return true;
	}

	return false;
}

bool fp_FieldRun::_setValue(UT_UCSChar *p_new_value)
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

		if(iLen > 1 && !XAP_App::getApp()->theOSHasBidiSupport())
		{
			FriBidiChar * fVisStr = new FriBidiChar[iLen+1];
			FriBidiChar * fLogStr = new FriBidiChar[iLen+1];
			UT_ASSERT(fVisStr && fLogStr);

			for(UT_uint32 i = 0; i < iLen; i++)
				fLogStr[i] = p_new_value[i];

			FriBidiCharType prevType/*, nextType*/, myType;
#if 0
			if(getNext())
				nextType = getNext()->getVisDirection();
			else
				nextType = getBlock()->getDominantDirection();
#endif

			if(getPrev())
				prevType = getPrev()->getVisDirection();
			else
				prevType = getBlock()->getDominantDirection();

			myType = prevType;

			fribidi_log2vis(/* input */
		     fLogStr,
		     iLen,
		     &myType,
		     /* output */
		     fVisStr,
		     NULL,
		     NULL,
		     NULL
		     );

			for(UT_uint32 j = 0; j < iLen; j++)
				m_sFieldValue[j] = (UT_UCSChar)fVisStr[j];

			m_sFieldValue[iLen] = 0;

			delete [] fLogStr;
			delete [] fVisStr;
		}
		else
		{
			UT_UCS4_strcpy(m_sFieldValue, p_new_value);
		}

		{
			UT_GrowBufElement aCharWidths[FPFIELD_MAX_LENGTH];

			// TODO -- is this really needed???
			// should not be, since lookupProperties is called on
			// formatting changes - Tomas
			// lookupProperties();

#ifndef WITH_PANGO
			getGR()->setFont(m_pFont);
#else
			getGR()->setFont(m_pPangoFont);
#endif
			UT_sint32 iNewWidth = getGR()->measureString(m_sFieldValue, 0, UT_UCS4_strlen(m_sFieldValue), aCharWidths);
			if (iNewWidth != getWidth())
			{
				_setWidth(iNewWidth);
				markWidthDirty();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				getGR()->setFont(m_pFontLayout);
				_setWidthLayoutUnits(getGR()->measureString(m_sFieldValue, 0, UT_UCS4_strlen(m_sFieldValue), aCharWidths));
#endif
				return true;
			}

		}
	}
	xxx_UT_DEBUGMSG(("fp_FieldRun::_setValue: value has not changed\n"));

	return false;
}

void fp_FieldRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP)
{
	UT_ASSERT(pSpanAP);
	PD_Document * pDoc = getBlock()->getDocument();
	fd_Field * fd = NULL;
	getBlock()->getField(getBlockOffset()+1,fd); // Next Pos?
	_setField(fd);
	if(getField() != NULL)
	{
		getField()->setBlock(getBlock());
	}
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	if(m_iFieldType == FPFIELD_list_label)
	{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION,true);
		m_pFontLayout = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION,true);
#else
		m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, true);
#endif
	}
	else
	{
		m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_pFontLayout = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
#endif
	}

	UT_RGBColor clrFG;
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true), clrFG);
	_setColorFG(clrFG);

	const char * pszFieldColor = NULL;
	pszFieldColor = PP_evalProperty("field-color",pSpanAP,pBlockAP,pSectionAP, getBlock()->getDocument(), true);

//
// FIXME: The "ffffff" is for backwards compatibility. If we don't exclude this
// no prexisting docs will be able to change the Highlight color in paragraphs
// with lists. I think this is a good solution for now. However it does mean
// field-color of "ffffff", pure white is actually transparent.
//
	if(pszFieldColor && UT_strcmp(pszFieldColor,"transparent") != 0 && UT_strcmp(pszFieldColor,"ffffff" ) != 0 )
	{
		UT_RGBColor r;
		UT_parseColor(pszFieldColor, r);
		_setColorHL(r);
	}

	_setAscent(getGR()->getFontAscent(m_pFont));
	_setDescent(getGR()->getFontDescent(m_pFont));
	_setHeight(getGR()->getFontHeight(m_pFont));
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	_setAscentLayoutUnits(getGR()->getFontAscent(m_pFontLayout));
	_setDescentLayoutUnits(getGR()->getFontDescent(m_pFontLayout));
	_setHeightLayoutUnits(getGR()->getFontHeight(m_pFontLayout));
#endif
//	getGR()->setFont(m_pFont);  Why??? DOM!!

	const XML_Char* pszType = NULL;
	const XML_Char* pszParam = NULL;


	const XML_Char * pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	if (0 == UT_strcmp(pszPosition, "superscript"))
	{
		m_fPosition = TEXT_POSITION_SUPERSCRIPT;
	}
	else if (0 == UT_strcmp(pszPosition, "subscript"))
	{
		m_fPosition = TEXT_POSITION_SUBSCRIPT;
	}
	else
	{
		m_fPosition = TEXT_POSITION_NORMAL;
	}

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
	for( i = 0; fp_FieldFmts[i].m_Tag != NULL; i++ )
	{
		if (0 == UT_stricmp(pszType, fp_FieldFmts[i].m_Tag))
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
//
// Lookup Decoration properties for this run
//
	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  getBlock()->getDocument(), true);
	_setLineWidth(getToplineThickness());
	_setDecorations(0);
	XML_Char* p;
	if (!UT_cloneString((char *&)p, pszDecor))
	{
		// TODO outofmem
	}
	UT_ASSERT(p || !pszDecor);
	XML_Char*	q = strtok(p, " ");

	while (q)
	{
		if (0 == UT_strcmp(q, "underline"))
		{
			_orDecorations(TEXT_DECOR_UNDERLINE);
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			_orDecorations(TEXT_DECOR_OVERLINE);
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
			_orDecorations(TEXT_DECOR_LINETHROUGH);
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			_orDecorations(TEXT_DECOR_TOPLINE);
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			_orDecorations(TEXT_DECOR_BOTTOMLINE);
		}
		q = strtok(NULL, " ");
	}
	free(p);

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

bool fp_FieldRun::letPointPass(void) const
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

void fp_FieldRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	// If X is left of the middle, return offset to the left,
	// otherwise the offset to the right.
	if (x < (getWidth() / 2))
		pos = getBlock()->getPosition() + getBlockOffset();
	else
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();

	bBOL = false;
	bEOL = false;
}

void fp_FieldRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x,
                                  UT_sint32& y, UT_sint32& x2,
                                  UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: FieldRun\n"));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(getLine());

	// TODO is this really needed ???
	// should not be, since lookupProperties is called on
	// formatting changes - Tomas
	// lookupProperties();

	getLine()->getOffsets(this, xoff, yoff);

	if (iOffset == (getBlockOffset() + getLength()))
	{
		xoff += getWidth();
	}
	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= getAscent() * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += getDescent() /* * 3/2 */;
	}
 	x = xoff;
	y = yoff;
	height = getHeight();
	x2 = x;
	y2 = y;
	bDirection = (getVisDirection() != FRIBIDI_TYPE_LTR);
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
    fp_Run* pNext = getNext();
	while(pNext != NULL && pNext->getField() != NULL )
	{
	    if(getField() == NULL)
		{
		        getField() = pNext->getField();
		}
	    pNext = getNext();
		count++;
	}
	if(count == 0)
	{
	    setWidth(0);
		_setHeight(0);
		_setWidthLayoutUnits(0);
		_setHeightLayoutUnits(0);
	}
	else
	{
	    pNext = getPrev();
		setWidth(pNext->getWidth());
		_setHeight(pNext->getHeight());
		_setWidthLayoutUnits(pNext->getWidthInLayoutUnits());
		_setHeightLayoutUnits(pNext->getHeightInLayoutUnits());
	}
	if(getField() != NULL)
	getField()->update();
*/
	return true;
}

void fp_FieldRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!isDirty());

	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	getGR()->fillRect(_getColorPG(), xoff, yoff, getWidth(), iLineHeight);
}

void fp_FieldRun::_defaultDraw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());

	// TODO is this really needed
	// should not be, since lookupProperties is called on
	// formatting changes - Tomas
	// lookupProperties();
	UT_sint32 xoff = 0, yoff = 0;

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

	//if (getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
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

		if (
			pView->getFocus()!=AV_FOCUS_NONE &&
			(iSel1 <= iRunBase)
			&& (iSel2 > iRunBase)
			)
		{
			UT_RGBColor color = _getView()->getColorSelBackground();
			color -= _getView()->getColorFieldOffset();
			getGR()->fillRect(color, pDA->xoff, iFillTop, getWidth(), iFillHeight);

		}
		else
		{
			updateHighlightColor();
			getGR()->fillRect(_getColorHL(), pDA->xoff, iFillTop, getWidth(), iFillHeight);
		}
	}

	getGR()->setFont(m_pFont);
	getGR()->setColor(_getColorFG());

	getGR()->drawChars(m_sFieldValue, 0, UT_UCS4_strlen(m_sFieldValue), pDA->xoff,iYdraw);
//
// Draw underline/overline/strikethough
//
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove
	                                                 //character dirt
	drawDecors( xoff, yTopOfRun);

}

// BEGIN DOM work on some new fields

fp_FieldCharCountRun::fp_FieldCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldCharCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_String szFieldValue;

	FV_View *pView = _getView();
	if(!pView)
	{
	    szFieldValue ="?";
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();
	    UT_String_sprintf(szFieldValue, "%d", cnt.ch_sp);
	}
	if (getField())
		getField()->setValue((XML_Char*) szFieldValue.c_str());

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldNonBlankCharCountRun::fp_FieldNonBlankCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldNonBlankCharCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	szFieldValue[0] = 0;

	FV_View *pView = _getView();
	if(!pView)
	{
		strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();
	    sprintf(szFieldValue, "%d", cnt.ch_no);
	}

	if (getField())
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldLineCountRun::fp_FieldLineCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldLineCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	szFieldValue[0] = 0;

	FV_View *pView = _getView();
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();
	    sprintf(szFieldValue, "%d", cnt.line);
	}

	if (getField())
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldParaCountRun::fp_FieldParaCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}


bool fp_FieldParaCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	szFieldValue[0] = 0;

	FV_View *pView = _getView();
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();
	    sprintf(szFieldValue, "%d", cnt.para);
	}

	if (getField())
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldWordCountRun::fp_FieldWordCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldWordCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	szFieldValue[0] = 0;

	FV_View *pView = _getView();
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();
	    sprintf(szFieldValue, "%d", cnt.word);
	}

	if (getField())
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

// mm/dd/yy notation
fp_FieldMMDDYYRun::fp_FieldMMDDYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

// dd/mm/yy time
fp_FieldDDMMYYRun::fp_FieldDDMMYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
	  getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

// Month Day, Year
fp_FieldMonthDayYearRun::fp_FieldMonthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldMthDayYearRun::fp_FieldMthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDefaultDateRun::fp_FieldDefaultDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDefaultDateNoTimeRun::fp_FieldDefaultDateNoTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldWkdayRun::fp_FieldWkdayRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) UT_strdup(szFieldValue));

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDOYRun::fp_FieldDOYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldMilTimeRun::fp_FieldMilTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldAMPMRun::fp_FieldAMPMRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldTimeEpochRun::fp_FieldTimeEpochRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldTimeEpochRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	time_t	tim = time(NULL);
	sprintf(szFieldValue, "%ld", (long)tim);
	if (getField())
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldTimeZoneRun::fp_FieldTimeZoneRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildIdRun::fp_FieldBuildIdRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildIdRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_ID);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_ID);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildVersionRun::fp_FieldBuildVersionRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildVersionRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Version);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_Version);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildOptionsRun::fp_FieldBuildOptionsRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildOptionsRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Options);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_Options);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildTargetRun::fp_FieldBuildTargetRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildTargetRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Target);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_Target);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildCompileDateRun::fp_FieldBuildCompileDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileDateRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_CompileDate);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_CompileDate);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildCompileTimeRun::fp_FieldBuildCompileTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileTimeRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_CompileTime);
	if (getField())
		getField()->setValue((XML_Char*) XAP_App::s_szBuild_CompileTime);
	return _setValue(sz_ucs_FieldValue);
}

// END OF DOM NEW FIELDS

static int countEndnotesBefore(fl_BlockLayout * pBL, const XML_Char * endid)
{
	int endnoteNo = 1;
	const XML_Char * someid;
	XML_Char * previd = NULL;
	while (pBL != NULL)
	{
		const PP_AttrProp *pp; bool bRes = pBL->getAttrProp(&pp);
		if (!bRes) return -1;
		pp->getAttribute("endnote-id", someid);
		xxx_UT_DEBUGMSG(("countEndnotesBefore: endid [%s], someid [%s]\n",endid,someid));
		if (someid && UT_strcmp(someid, endid)==0)
			break;
		pBL = (fl_BlockLayout *) pBL->getNext();

		// HACK until we stop propagating endnote-ids.
		// actually, we do want to propagate the endnote ids, it allows
		// us to create multiparagraph endnotes and delete everyting
		// when the reference is deleted
		if (!previd || (previd && someid && UT_strcmp(someid, previd) != 0))
		{
			endnoteNo++;

			if (previd != NULL)
				free(previd);
			previd = UT_strdup(someid);
		}
	}
	if (previd != NULL)
		free(previd);

	return endnoteNo;
}

// Refers to an footnote in the main body of the text.
fp_FieldFootnoteRefRun::fp_FieldFootnoteRefRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
#if 0
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);
	m_iPID = atol(endid);
#endif
}


bool fp_FieldFootnoteRefRun::calculateValue(void)
{
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);

	// now we need to find the block's footnote section.
	fl_FootnoteLayout * pFootL = static_cast<fl_FootnoteLayout *>(getBlock()->getNext());

	// Hmm, this is kind of messy; we can be called before
	// the endnote section has been added.
	if (pFootL == NULL)
	{
		UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
		UT_UCS4_strcpy_char(sz_ucs_FieldValue, "?");
		return _setValue(sz_ucs_FieldValue);
	}
	UT_ASSERT(pFootL->getType() == FL_SECTION_FOOTNOTE);

	// Now, count out how many paragraphs have special endnote-id tags
	// until we reach the desired paragraph.  (para == block)

	fl_BlockLayout * pBL = (fl_BlockLayout *) pFootL->getFirstLayout();
	int endnoteNo = countEndnotesBefore(pBL, endid);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_String szFieldValue;

	// How do we superscript the endnote?
	szFieldValue = UT_String_sprintf ("[%d]", endnoteNo);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldFootnoteAnchorRun::fp_FieldFootnoteAnchorRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
#if 0
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);
	m_iPID = atol(endid);
#endif
}

// Appears in the FootnoteContainer, one per footnote.
bool fp_FieldFootnoteAnchorRun::calculateValue(void)
{
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);

	fl_SectionLayout * pFootSL = getBlock()->getSectionLayout();
	//UT_ASSERT((pFootSL->getType() == FL_SECTION_ENDNOTE));

	// this can happen when we delete last endnote
	if(pFootSL->getType() != FL_SECTION_FOOTNOTE)
		return false;
	// Now, count out how many paragraphs have special endnote-id tags
	// until we reach the desired paragraph.  (para == block)

	// should this actually be refactored?

	fl_BlockLayout * pBL = (fl_BlockLayout *) pFootSL->getFirstLayout();
	int endnoteNo = countEndnotesBefore(pBL, endid);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_String szFieldValue;

	szFieldValue = UT_String_sprintf ("[%d] ", endnoteNo);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

// Appears in the EndnoteSection, one per endnote.
bool fp_FieldEndnoteAnchorRun::calculateValue(void)
{
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);

	fl_DocSectionLayout * pEndSL = getBlock()->
		getDocSectionLayout();
	//UT_ASSERT((pEndSL->getType() == FL_SECTION_ENDNOTE));

	// this can happen when we delete last endnote
	if(pEndSL->getType() != FL_SECTION_ENDNOTE)
		return false;
	// Now, count out how many paragraphs have special endnote-id tags
	// until we reach the desired paragraph.  (para == block)

	// should this actually be refactored?

	fl_BlockLayout * pBL = (fl_BlockLayout *) pEndSL->getFirstLayout();
	int endnoteNo = countEndnotesBefore(pBL, endid);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	UT_String szFieldValue;

	szFieldValue = UT_String_sprintf ("[%d] ", endnoteNo);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue.c_str());

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldTimeRun::fp_FieldTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldDateRun::fp_FieldDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
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
		getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldFileNameRun::fp_FieldFileNameRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldFileNameRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	PD_Document * pDoc = getBlock()->getDocument();
	UT_ASSERT(pDoc);

	//copy in the name or some wierd char instead
	const char * name = pDoc->getFileName();
	if (!name)
		name = "*";

	strcpy (szFieldValue, name);

	if (getField())
	  getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldPageNumberRun::fp_FieldPageNumberRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldPageNumberRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	if (getLine() && getLine()->getContainer() && getLine()->getContainer()->getPage())
	{
		fp_Page* pPage = getLine()->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();
		fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *)  pPage->getOwningSection();
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
		while(pDSL && !pDSL->arePageNumbersRestarted())
		{
			pDSL =  pDSL->getPrevDocSection();
		}
		UT_sint32 icnt = 0;
		fp_Page * pFirstPage = NULL;
		if(pDSL && pDSL->arePageNumbersRestarted())
		{
			fp_Container * pCon = pDSL->getFirstContainer();
			if(pCon)
			{
				bool bFound = false;
				pFirstPage = pCon->getPage();
				while(pFirstPage && !bFound)
				{
					if(pDSL == (fl_DocSectionLayout *) pFirstPage->getOwningSection())
					{
						bFound = true;
						break;
					}
					pFirstPage = pFirstPage->getNext();
				}
				if(bFound)
				{
					while(pFirstPage && (pFirstPage != pPage))
					{
						icnt++;
						pFirstPage = pFirstPage->getNext();
					}
					UT_ASSERT(pFirstPage);
					iPageNum = pDSL->getRestartedPageNumber() + icnt;
				}
			}
		}
#if 0
		// FIXME:jskov Cannot assert here since the field might get
        // updated while the page is still being populated (and thus not in
		// the doc's page list).  Surely the field should not get updated
        // until the page is fully populated?
		UT_ASSERT(iPageNum > 0);
#endif

		sprintf(szFieldValue, "%d", iPageNum);
	}
	else
	{
		strcpy(szFieldValue, "?");
	}
	if (getField())
	  getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

fp_FieldPageReferenceRun::fp_FieldPageReferenceRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen)
	: fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldPageReferenceRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
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
		pBlock = (fl_BlockLayout *) pSection->getFirstLayout();

		while (pBlock)
		{
			pRun = pBlock->getFirstRun();
			while (pRun)
			{
				xxx_UT_DEBUGMSG(("pRun 0x%x, type %d\n", pRun, pRun->getType()));
				if(pRun->getType() == FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pB = static_cast<fp_BookmarkRun*>(pRun);
					if(pB->isStartOfBookmark() && !UT_strcmp(_getParameter(),pB->getName()))
					{
						bFound = true;
						break;
					}
				}
				pRun = pRun->getNext();
			}
			if(bFound)
				break;

			pBlock = (fl_BlockLayout *) pBlock->getNext();
		}
		if(bFound)
			break;
		pSection = (fl_SectionLayout *) pSection->getNext();
	}

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

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
		sprintf(szFieldValue, "%d", iPageNum);
	}
	else
	{
	  static XAP_App * pApp = XAP_App::getApp();
		// did not find the bookmark, set the field to an error value
		XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
		UT_ASSERT((pFrame));

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_FIELD_Error, pApp->getDefaultEncoding()).c_str();
		const char *pMsg2 = pSS->getValue(AP_STRING_ID_MSG_BookmarkNotFound, pApp->getDefaultEncoding()).c_str();
		char * format = new char[strlen(pMsg1) + strlen(pMsg2) + 5];

		sprintf(format, "{%s: %s}", pMsg1, pMsg2);
		sprintf(szFieldValue, format, _getParameter());

		delete [] format;
	}

	if (getField())
	  getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}


///////////////////////////////////////////////////////////////////////////

fp_FieldPageCountRun::fp_FieldPageCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldPageCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	if (getLine() && getLine()->getContainer() && getLine()->getContainer()->getPage())
	{

		fp_Page* pPage = getLine()->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		sprintf(szFieldValue, "%d", pDL->countPages());
	}
	else
	{
		strcpy(szFieldValue, "?");
	}
	if (getField())
	  getField()->setValue((XML_Char*) szFieldValue);

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldMailMergeRun::fp_FieldMailMergeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen)
  : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldMailMergeRun::calculateValue(void)
{
	fd_Field * fld = getField();
	if (fld) {
	  const XML_Char * param = fld->getParameter ();

	  if (!param)
	    return false;

	  UT_UTF8String value ;

	  UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	  sz_ucs_FieldValue[0] = 0;
	  
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

	  fld->setValue((XML_Char*) value.utf8_str());
	  UT_UCS4_strcpy(sz_ucs_FieldValue, value.ucs4_str().ucs4_str());

	  return _setValue(sz_ucs_FieldValue);
	}

	return false;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldMetaRun::fp_FieldMetaRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, const char * which)
  : fp_FieldRun(pBL, pG, iOffsetFirst, iLen), m_which(which)
{
}

bool fp_FieldMetaRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;

	PD_Document * pDoc = getBlock()->getDocument();
	UT_ASSERT(pDoc);

	UT_String value ;
	if (!pDoc->getMetaDataProp(m_which, value) || !value.size())
	  value = " ";

	if (getField())
		getField()->setValue((XML_Char*) value.c_str());

	UT_UCS4_strcpy_char(sz_ucs_FieldValue, value.c_str());

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldMetaTitleRun::fp_FieldMetaTitleRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_TITLE)
{
}

fp_FieldMetaCreatorRun::fp_FieldMetaCreatorRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_CREATOR)
{
}

fp_FieldMetaSubjectRun::fp_FieldMetaSubjectRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_SUBJECT)
{
}

fp_FieldMetaPublisherRun::fp_FieldMetaPublisherRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_PUBLISHER)
{
}

fp_FieldMetaDateRun::fp_FieldMetaDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_DATE)
{
}

fp_FieldMetaTypeRun::fp_FieldMetaTypeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_TYPE)
{
}

fp_FieldMetaLanguageRun::fp_FieldMetaLanguageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_LANGUAGE)
{
}

fp_FieldMetaRightsRun::fp_FieldMetaRightsRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_RIGHTS)
{
}

fp_FieldMetaKeywordsRun::fp_FieldMetaKeywordsRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_KEYWORDS)
{
}

fp_FieldMetaContributorRun::fp_FieldMetaContributorRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_CONTRIBUTOR)
{
}

fp_FieldMetaCoverageRun::fp_FieldMetaCoverageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_COVERAGE)
{
}

fp_FieldMetaDescriptionRun::fp_FieldMetaDescriptionRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldMetaRun(pBL, pG, iOffsetFirst, iLen, PD_META_KEY_DESCRIPTION)
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedColumnBreakRun::fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDCOLUMNBREAK)
{
	lookupProperties();
}

void fp_ForcedColumnBreakRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
												const PP_AttrProp * /*pBlockAP*/,
												const PP_AttrProp * /*pSectionAP*/)
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

bool fp_ForcedColumnBreakRun::letPointPass(void) const
{
	return false;
}

void fp_ForcedColumnBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
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
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    getLine()->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = getLine()->getMaxWidth() - getLine()->calculateWidthOfLine();
    getGR()->fillRect(_getColorPG(),xoff,yoff,iWidth,getLine()->getHeight());
}

void fp_ForcedColumnBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());

    if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = _getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = getLine()->getMaxWidth();

    UT_UCSChar *pColumnBreak;
    UT_UCS4_cloneString_char(&pColumnBreak,"Column Break");
	_drawTextLine(pDA->xoff,pDA->yoff+getLine()->getAscent(),iLineWidth,getLine()->getHeight(),pColumnBreak);
    FREEP(pColumnBreak);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedPageBreakRun::fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDPAGEBREAK)
{
	lookupProperties();
}

void fp_ForcedPageBreakRun::_lookupProperties(const PP_AttrProp * /*pSpanAP*/,
											  const PP_AttrProp * /*pBlockAP*/,
											  const PP_AttrProp * /*pSectionAP*/)
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

bool fp_ForcedPageBreakRun::letPointPass(void) const
{
	return false;
}

void fp_ForcedPageBreakRun::mapXYToPosition(UT_sint32 /* x */, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
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
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    getLine()->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = getLine()->getMaxWidth() - getLine()->calculateWidthOfLine();
    getGR()->fillRect(_getColorPG(),xoff,yoff,iWidth,getLine()->getHeight());
}

void fp_ForcedPageBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == getGR());

    if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = _getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = getLine()->getMaxWidth();

    UT_UCSChar *pPageBreak;
    UT_UCS4_cloneString_char(&pPageBreak,"Page Break");

	_drawTextLine(pDA->xoff,pDA->yoff+getLine()->getAscent(),iLineWidth,getLine()->getHeight(),pPageBreak);
    FREEP(pPageBreak);
}

// translates logical position in a run into visual position
// (will also translate correctly visual -> logical)
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos)
{
    if(getVisDirection() == FRIBIDI_TYPE_RTL) //rtl needs translation
    {
        return (getLength() - iLogPos - 1);
    }
    else return (iLogPos);
}

//translates a visual position in a span of length iLen to logical pos
//or vice versa
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen)
{
    if(getVisDirection() == FRIBIDI_TYPE_RTL) //rtl needs translation
    {
        return (iLen - iLogPos - 1);
    }
    else return (iLogPos);
}

//returns the logical offset of the first visual character
UT_uint32 fp_Run::getOffsetFirstVis()
{
    if(getVisDirection() == FRIBIDI_TYPE_RTL) //rtl, requires translation
    {
        return(getBlockOffset() + getLength() - 1);
    }
    else return (getBlockOffset());
}

//translates visual offset to logical one, can be also used for translation
//in the other direction
UT_uint32 fp_Run::getOffsetLog(UT_uint32 iVisOff)
{
    if(getVisDirection() == FRIBIDI_TYPE_RTL) //rtl needs translation
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

void fp_Run::setDirection(FriBidiCharType iDir)
{
    xxx_UT_DEBUGMSG(("fp_Run::SetDirection, getDirection() %d, iDir %d, run type %d\n", getDirection(), iDir, getType()));
	FriBidiCharType iDirection = iDir != FRIBIDI_TYPE_UNSET ? iDir : FRIBIDI_TYPE_WS;
	if(getDirection() != iDirection)
	{
		FriBidiCharType origDirection = getDirection();
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
FriBidiCharType fp_Run::getVisDirection()
{
	if(m_iVisDirection == FRIBIDI_TYPE_UNSET)
	{
		if(getBlock())
			return getBlock()->getDominantDirection();
		else
		{
			bool b;
			XAP_App::getApp()->getPrefsValueBool((XML_Char*)AP_PREF_KEY_DefaultDirectionRtl, &b);
			if(b)
				return FRIBIDI_TYPE_RTL;
			else
				return FRIBIDI_TYPE_LTR;
		}
	}
	else
		return m_iVisDirection;
}

void fp_Run::setVisDirection(FriBidiCharType iDir)
{
    if(iDir != getVisDirection())
		m_bRefreshDrawBuffer = true;
	m_iVisDirection = iDir;
}

#if 0
void fp_Run::setDirectionProperty(FriBidiCharType dir)
{
	const XML_Char * prop[] = {NULL, NULL, 0};
	const XML_Char direction[] = "dir";
	const XML_Char rtl[] = "rtl";
	const XML_Char ltr[] = "ltr";
	UT_String other;

	prop[0] = (XML_Char*) &direction;

	switch(dir)
	{
		case FRIBIDI_TYPE_LTR:  prop[1] = (XML_Char*) &ltr;     break;
		case FRIBIDI_TYPE_RTL:  prop[1] = (XML_Char*) &rtl;     break;
		default:
		 {
		 	// for anything other we will print the FriBidiCharType value
		 	// this will allow us to coallesce runs of same type without
		 	// having to list here tons of possible strings
		 	// (we could do this for rtl and ltr as well, but "rtl" and "ltr"
		 	// are much more informative.)
		 	UT_String_sprintf(other,"fbt%d",(UT_uint32)dir);
		 	prop[1] = (XML_Char*) other.c_str(); break;
		 }
	};

	UT_uint32 offset = getBlock()->getPosition() + getBlockOffset();
	getBlock()->getDocument()->changeSpanFmt(PTC_AddFmt,offset,offset + getLength(),NULL,prop);
	UT_DEBUGMSG(("fp_Run::setDirectionProperty: offset=%d, len=%d, dir=\"%s\"\n", offset,getLength(),prop[1]));
}
#endif
