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

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"

#include "ap_Prefs.h"

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
		m_iY(0),
		m_iHeight(0),
		m_iHeightLayoutUnits(0),
		m_iWidth(0),
		m_iWidthLayoutUnits(0),
		m_iOffsetFirst(iOffsetFirst),
		m_iLen(iLen),
		m_iAscent(0),
		m_iDescent(0),
		m_iAscentLayoutUnits(0),
		m_iDescentLayoutUnits(0),
		m_pG(pG),
		m_bDirty(true),	// a run which has just been created is not onscreen, therefore it is dirty
		m_pField(0),
		m_pScreenFont(0),
		m_pLayoutFont(0)
{
        // set the default background color and the paper color of the 
	    // section owning the run.
	getHighlightColor();
	getPageColor();
	
#ifdef BIDI_ENABLED
	m_iDirection = -1; //by default all runs are whitespace
#endif
	m_fDecorations = 0;
	m_iLineWidth = 0;
}


fp_Run::~fp_Run()
{
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
bool fp_Run::findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 /* iMaxLeftWidth */,
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
		//UT_DEBUGMSG(("fp_Run::_inheritProperties: from prev run\n"));
		m_iAscent = pRun->getAscent();
		m_iDescent = pRun->getDescent();
		m_iHeight = pRun->getHeight();
		m_iAscentLayoutUnits = pRun->getAscentInLayoutUnits();
		m_iDescentLayoutUnits = pRun->getDescentInLayoutUnits();
		m_iHeightLayoutUnits = pRun->getHeightInLayoutUnits();
	}
	else
	{
		// look for fonts in this DocLayout's font cache
		//UT_DEBUGMSG(("fp_Run::_inheritProperties: from current font\n"));
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
		m_pBL->getSpanAttrProp(m_iOffsetFirst,true,&pSpanAP);
		m_pBL->getAttrProp(&pBlockAP);

		FL_DocLayout * pLayout = m_pBL->getDocLayout();

		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
		if (pFont != m_pScreenFont)
		  {
		    m_pScreenFont = pFont;
		    m_iAscent = m_pG->getFontAscent(pFont);	
		    m_iDescent = m_pG->getFontDescent(pFont);
		    m_iHeight = m_pG->getFontHeight(pFont);
		  }

		pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
		if (pFont != m_pLayoutFont)
		  {
		    m_pLayoutFont = pFont;
		    m_iAscentLayoutUnits = m_pG->getFontAscent(pFont);	
		    m_iDescentLayoutUnits = m_pG->getFontDescent(pFont);
		    m_iHeightLayoutUnits = m_pG->getFontHeight(pFont);
		  }
	}
}

/*!
 * This method returns the Hight Light color as defined for this page. It examines
 * the bgcolor property and uses that over the page color if its defined as not
 * transperent. It sets the m_colorHL member variable.
 */
UT_RGBColor * fp_Run::getHighlightColor(void)
{

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; 
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	xxx_UT_DEBUGMSG(("SEVIOR: Doing Lookupprops for block %x run %x  offset =%d \n ",m_pBL,this,m_iOffsetFirst));
//	UT_ASSERT(pSpanAP); // Sevior put this back to track down interesting 
	// Section change bug.
	m_pBL->getAttrProp(&pBlockAP);

	const char * pszBGcolor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP, m_pBL->getDocument(), true);
	static UT_RGBColor sClr;
	UT_RGBColor * pClr = NULL;

//
// FIXME: The "ffffff" is for backwards compatibility. If we don't exclude this 
// no prexisting docs will be able to change the Highlight color in paragraphs 
// with lists. I think this is a good solution for now. However it does mean
// field-color of "ffffff", ie pure white is actually transparent.
//
	if(pszBGcolor && UT_strcmp(pszBGcolor,"transparent")!= 0  && UT_strcmp(pszBGcolor,"ffffff") != 0)
	{
		UT_parseColor (pszBGcolor, m_colorHL);
		return &m_colorHL;
	}
	else
	{
		fp_Line * pLine = getLine();
		fp_Page * pPage = NULL;		
		if(pLine != NULL)
			pPage = pLine->getContainer()->getPage();
		if(pPage != NULL)
			pClr = pPage->getOwningSection()->getPaperColor();
		else
			pClr = m_pBL->getDocSectionLayout()->getPaperColor();
		UT_setColor (m_colorHL, pClr->m_red, pClr->m_grn, pClr->m_blu);
		return pClr;
	}
}


/*!
 * This method returns the page color and sets the page color member variable 
 * as defined for this page. It is used for clearscreen() methods and so does 
 * not examine the bgcolor property.
 */
UT_RGBColor * fp_Run::getPageColor(void)
{
	fp_Line * pLine = getLine();
	fp_Page * pPage = NULL;
	UT_RGBColor * pClr = NULL;
	if(pLine != NULL)
		pPage = pLine->getContainer()->getPage();
	if(pPage != NULL)
		pClr = pPage->getOwningSection()->getPaperColor();
	else
		pClr = m_pBL->getDocSectionLayout()->getPaperColor();
	UT_setColor (m_colorPG, pClr->m_red, pClr->m_grn, pClr->m_blu);
	return pClr;
}

/*!
 * This method updates the Highlight and Page color underneath a run. 
 * It would typically be called after a change in the SectionLevel properties or
 * if a line cotaining a run is moved from one color page to another.
 */
void fp_Run::updateBackgroundColor(void)
{
	getHighlightColor();
	getPageColor();
}

void fp_Run::insertIntoRunListBeforeThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.m_pNext = this;
	if (m_pPrev)
	{
		m_pPrev->m_pNext = &newRun;
	}
	newRun.m_pPrev = m_pPrev;
	m_pPrev = &newRun;
}

void fp_Run::insertIntoRunListAfterThis(fp_Run& newRun)
{
	newRun.unlinkFromRunList();
	newRun.m_pPrev = this;
	if (m_pNext)
	{
		m_pNext->m_pPrev = &newRun;
	}
	newRun.m_pNext = m_pNext;
	m_pNext = &newRun;
}

void fp_Run::unlinkFromRunList()
{
	if (m_pPrev)
	{
		m_pPrev->m_pNext = m_pNext;
	}
	if (m_pNext)
	{
		m_pNext->m_pPrev = m_pPrev;
		m_pNext = 0;
	}
	m_pPrev = 0;
}

void	fp_Run::setX(UT_sint32 iX)
{
	if (iX == m_iX)
	{
		return;
	}

	clearScreen();
	
	m_iX = iX;
}

void	fp_Run::setY(UT_sint32 iY)
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
	updateBackgroundColor();
}


void fp_Run::setBlock(fl_BlockLayout * pBL)
{
	m_pBL = pBL;
}

void fp_Run::setNext(fp_Run* p)
{
	m_pNext = p;
}

void fp_Run::setPrev(fp_Run* p)
{
	m_pPrev = p;
}

bool fp_Run::isLastRunOnLine(void) const
{
	return (m_pLine->getLastRun() == this);
}

bool fp_Run::isFirstRunOnLine(void) const
{
	return (m_pLine->getFirstRun() == this);
}

#ifdef BIDI_ENABLED
bool fp_Run::isLastVisRunOnLine(void) const
{
	return (m_pLine->getLastVisRun() == this);
}

bool fp_Run::isFirstVisRunOnLine(void) const
{
	return (m_pLine->getFirstVisRun() == this);
}
#endif

bool fp_Run::isOnlyRunOnLine(void) const
{
	if (m_pLine->countRuns() == 1)
	{
		UT_ASSERT(isFirstRunOnLine());
		UT_ASSERT(isLastRunOnLine());

		return true;
	}

	return false;
}

void fp_Run::setLength(UT_uint32 iLen)
{
	if (iLen == m_iLen)
	{
		return;
	}

	clearScreen();
	
	m_iLen = iLen;
}

void fp_Run::setBlockOffset(UT_uint32 offset)
{
	m_iOffsetFirst = offset;
}

void fp_Run::clearScreen(bool bFullLineHeightRect)
{

	if (m_bDirty)
	{
		// no need to clear if we've already done so.
		return;
	}

	if (!m_pLine)
	{
		// nothing to clear if this run is not currently on a line
		return;
	}
	
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	_clearScreen(bFullLineHeightRect);
	
	// make sure we only get erased once
	m_bDirty = true;
}

void fp_Run::draw(dg_DrawArgs* pDA)
{

	if (pDA->bDirtyRunsOnly)
	{
		if (!m_bDirty)
		{
			return;
		}
	}
	
	_draw(pDA);

	m_bDirty = false;
}

bool fp_Run::canContainPoint(void) const
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
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);

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
underline or overline could shift position depending on the text size -
particularly for subscripts and superscripts. We can't work out where to put
the lines until the end of the lined span. This info is saved in the fp_Run
class. If a underline or overline is pending (because the next run continues
 the underline or overline), mark the next run as dirty to make sure it is
drawn.
     */
	
	if( (m_fDecorations & (TEXT_DECOR_UNDERLINE | TEXT_DECOR_OVERLINE |
			TEXT_DECOR_LINETHROUGH | TEXT_DECOR_TOPLINE | TEXT_DECOR_BOTTOMLINE)) == 0)
	{
		return;
	}

	const UT_sint32 old_LineWidth = m_iLineWidth;
	UT_sint32 cur_linewidth = 1+ (UT_MAX(10,m_iAscent)-10)/8;
	UT_sint32 iDrop = 0;
	fp_Run* P_Run = getPrev();
	fp_Run* N_Run = getNext();
	const bool b_Underline = isUnderline();
	const bool b_Overline = isOverline();
	const bool b_Strikethrough = isStrikethrough();
	const bool b_Topline = isTopline();
	const bool b_Bottomline = isBottomline();
	const bool b_Firstrun = (P_Run == NULL) || (getLine()->getFirstRun()== this);
	const bool b_Lastrun = (N_Run == NULL) || (getLine()->getLastRun()== this);

	/*
	  If the previous run is NULL or if this is the first run of a line,
we are on the first run of the line so set the linethickness, start of the line span and the overline and underline positions from the current measurements.
	*/
	if(P_Run == NULL || b_Firstrun )
	{
		setLinethickness(cur_linewidth);
		if(b_Underline)
		{
			iDrop = yoff + m_iAscent + m_iDescent/3;
			setUnderlineXoff( xoff);
			setMaxUnderline(iDrop);
		}
		if(b_Overline)
		{
			iDrop = yoff + 1 + (UT_MAX(10,m_iAscent) - 10)/8;
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
			  iDrop = yoff + m_iAscent + m_iDescent/3;
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
		     iDrop = yoff + 1 + (UT_MAX(10,m_iAscent) - 10)/8;
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
	m_pG->setLineWidth(m_iLineWidth);
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
			m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
		if ( b_Overline)
		{
			iDrop = UT_MIN( getMinOverline(), iDrop);
			UT_sint32 totx = getOverlineXoff();
			m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
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
				 m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
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
				m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
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
		m_pG->drawLine(xoff, iDrop, xoff+getWidth(), iDrop);
	}
	/*
	   Restore the previous line width.
	*/
	m_iLineWidth = old_LineWidth;
	m_pG->setLineWidth(m_iLineWidth);
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

	PD_Document * pDoc = m_pBL->getDocument();
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);
	// This gives the baseline of the selection.
	// need to clear full height of line, in case we had a selection
	UT_sint32 xxoff=0 ,ybase =0;
	m_pLine->getScreenOffsets(this, xxoff, ybase);

	if ( b_Topline)
	{
		m_pG->fillRect(clrFG, xoff, ybase, getWidth(), ithick);
	}
	/*
	  We always draw bottomline right at the bottom so there is no ambiguity
	*/
	if ( b_Bottomline)
	{
		m_pG->fillRect(clrFG, xoff, ybase+getLine()->getHeight()-ithick, getWidth(), ithick);
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
	return m_pG->convertDimension("0.8pt");
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

    GR_Font *pFont = m_pG->getGUIFont();
    m_pG->setFont(pFont);

    UT_uint32 iTextLen = UT_UCS_strlen(pText);
    UT_uint32 iTextWidth = m_pG->measureString(pText,0,iTextLen,NULL);
    UT_uint32 iTextHeight = m_pG->getFontHeight(pFont);

    UT_uint32 xoffText = xoff + (iWidth - iTextWidth) / 2;
    UT_uint32 yoffText = yoff - m_pG->getFontAscent(pFont) * 2 / 3;

    m_pG->drawLine(xoff,yoff,xoff + iWidth,yoff);

    if((iTextWidth < iWidth) && (iTextHeight < iHeight)){
        m_pG->fillRect(m_colorHL,xoffText,yoffText,iTextWidth,iTextHeight);
        m_pG->drawChars(pText,0,iTextLen,xoffText,yoffText);
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_TabRun::fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TAB)
{
	lookupProperties();
}

void fp_TabRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);
	m_pBL->getField(m_iOffsetFirst,m_pField);


	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, m_pBL->getDocument(), true), m_colorFG);
	
	
	getHighlightColor(); // Highlight color
	getPageColor(); // update Page Color member variable.

	if (pFont != m_pScreenFont)
	{
	    m_pScreenFont = pFont;
	    m_iAscent = m_pG->getFontAscent(pFont);	
	    m_iDescent = m_pG->getFontDescent(pFont);
	    m_iHeight = m_pG->getFontHeight(pFont);
	}

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);

	if (pFont != m_pLayoutFont)
	{
	    m_pLayoutFont = pFont;
	    m_iAscentLayoutUnits = m_pG->getFontAscent(pFont);	
	    m_iDescentLayoutUnits = m_pG->getFontDescent(pFont);
	    m_iHeightLayoutUnits = m_pG->getFontHeight(pFont);
	}
#ifdef BIDI_ENABLED
	m_iDirection = -1;
#endif
//
// Lookup Decoration properties for this run
//
	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  m_pBL->getDocument(), true);
	m_iLineWidth = getToplineThickness();
	m_fDecorations = 0;
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
			m_fDecorations |= TEXT_DECOR_UNDERLINE;
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			m_fDecorations |= TEXT_DECOR_OVERLINE;
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
			m_fDecorations |= TEXT_DECOR_LINETHROUGH;
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			m_fDecorations |= TEXT_DECOR_TOPLINE;
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			m_fDecorations |= TEXT_DECOR_BOTTOMLINE;
		}
		q = strtok(NULL, " ");
	}
	free(p);

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

void fp_TabRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	// If X is left of the middle, return offset to the left,
	// otherwise the offset to the right.
	if (x < (getWidth() / 2))
		pos = m_pBL->getPosition() + m_iOffsetFirst;
	else
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
		
	bBOL = false;
	bEOL = false;
}

void fp_TabRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: TabRun\n"));
	UT_sint32 xoff;
	UT_sint32 yoff;
#ifdef BIDI_ENABLED
	UT_sint32 xoff2;
	UT_sint32 yoff2;
#endif
	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
#ifdef BIDI_ENABLED
	fp_Run * pRun = 0;
	UT_sint32 iNextDir;

	if (iOffset == (m_iOffsetFirst + m_iLen))  //#TF is this the right-most logical element of the run?
	{
	    pRun = getNext();
	    if(pRun)
	    {
	        pRun->getLine()->getOffsets(pRun, xoff2, yoff2);
	        iNextDir = pRun->getVisDirection();
	    }
	}
	
	UT_sint32 iDirection = getVisDirection();

	if(iDirection == 1)
	    x = xoff - 1;      //necessary to draw the the caret properly -- I am not sure why though #TF
	else
	    x = xoff + getWidth() + 1;
	
	
	if(pRun && (iNextDir != iDirection)) //if this run precedes run of different direction, we have to split the caret
	{
	    x2 = (iNextDir == 0) ?  xoff + pRun->getWidth() + 1 : xoff2 - 1;
	    y2 = yoff2;
	}
	else
	{
	    x2 = x;
	    y2 = yoff;
	}
	bDirection = (iDirection != 0);
#else
	if (iOffset == m_iOffsetFirst)
	{
		x = xoff;
	}
	else
	{
		UT_ASSERT(iOffset == (m_iOffsetFirst + m_iLen));
		
		x = xoff + getWidth();
	}
#endif	
	y = yoff;
	height = m_iHeight;
}

void fp_TabRun::setWidth(UT_sint32 iWidth)
{
	clearScreen();
	m_iWidth = iWidth;
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
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);

	m_pG->fillRect(m_colorPG,xoff, yoff, m_iWidth, m_pLine->getHeight());
}

void fp_TabRun::_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight)
{
    if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

#define NPOINTS 6

    UT_Point points[NPOINTS];

    UT_sint32 cur_linewidth = 1 + (UT_MAX(10,m_iAscent) - 10) / 8;
    UT_uint32 iyAxis = iTop + m_pLine->getAscent() * 2 / 3;
    UT_uint32 iMaxWidth = UT_MIN(iWidth / 10 * 6, (UT_uint32) cur_linewidth * 9);
    UT_uint32 ixGap = (iWidth - iMaxWidth) / 2;

    points[0].x = iLeft + ixGap + iMaxWidth - cur_linewidth * 4;
    points[0].y = iyAxis - cur_linewidth * 2;

    points[1].x = points[0].x + cur_linewidth;
    points[1].y = points[0].y;

    points[2].x = iLeft + iWidth - ixGap;
    points[2].y = iyAxis;

    points[3].x = points[1].x;
    points[3].y = iyAxis + cur_linewidth * 2;

    points[4].x = points[0].x;
    points[4].y = points[3].y;

    points[5].x = points[0].x;
    points[5].y = points[0].y;

    UT_RGBColor clrShowPara(127,127,127);
    m_pG->polygon(clrShowPara,points,NPOINTS);
    m_pG->fillRect(clrShowPara,iLeft + ixGap,iyAxis - cur_linewidth / 2,iMaxWidth - cur_linewidth * 4,cur_linewidth);
}

void fp_TabRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

	UT_RGBColor clrSelBackground(192, 192, 192);
	UT_RGBColor clrNormalBackground(m_colorHL.m_red,m_colorHL.m_grn,m_colorHL.m_blu);
	// need to draw to the full height of line to join with line above.
	UT_sint32 xoff= 0, yoff=0;
	getLine()->getScreenOffsets(this, xoff, yoff);

	UT_sint32 iFillHeight = m_pLine->getHeight();
	UT_sint32 iFillTop = pDA->yoff - m_pLine->getAscent();
		
	FV_View* pView = m_pBL->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
	UT_ASSERT(iSel1 <= iSel2);
	
#ifdef BIDI_ENABLED
	UT_uint32 iRunBase = m_pBL->getPosition() + getOffsetFirstVis(); //m_iOffsetFirst;
#else
	UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;
#endif

	UT_RGBColor clrFG;
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	PD_Document * pDoc = m_pBL->getDocument();
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);
	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP, pSectionAP, pDoc, true), clrFG);
	if (m_leader != FL_LEADER_NONE)
	{
		UT_UCSChar tmp[151];
        	unsigned short wid[151];
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

		m_pG->setFont(m_pScreenFont);
		m_pG->measureString(tmp, 1, 150, wid);
		// one would think that one could measure
		// one character and divide the needed
		// width by that; would one be so wrong?
		// we're not dealing with different letters
		// here, after all.

		i = 1; 
		cumWidth = 0;
		while (cumWidth < m_iWidth && i < 151)
			cumWidth += wid[i++];

		i = (i>=3) ? i - 2 : 1;
		m_pG->setColor(clrFG);
		m_pG->drawChars(tmp, 1, i, pDA->xoff, iFillTop);
	}
	else
	if (
		pView->getFocus()!=AV_FOCUS_NONE &&
		(iSel1 <= iRunBase)
		&& (iSel2 > iRunBase)
		)
	{
		m_pG->fillRect(clrSelBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
        if(pView->getShowPara()){
            _drawArrow(pDA->xoff, iFillTop, m_iWidth, iFillHeight);
        }
	}
	else
	{
		m_pG->fillRect(clrNormalBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
        if(pView->getShowPara()){
            _drawArrow(pDA->xoff, iFillTop, m_iWidth, iFillHeight);
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
		m_pG->fillRect(clrFG, pDA->xoff+getWidth()-ithick, iFillTop, ithick, iFillHeight);
	}
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedLineBreakRun::fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDLINEBREAK)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun constructor\n"));
	lookupProperties();
}

void fp_ForcedLineBreakRun::lookupProperties(void)
{
	//UT_DEBUGMSG(("fp_ForcedLineBreakRun::lookupProperties\n"));
	m_pBL->getField(m_iOffsetFirst,m_pField);

	_inheritProperties();
	m_iWidth = 16;
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
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = false;
	bEOL = false;
}

void fp_ForcedLineBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_ASSERT(m_iOffsetFirst == iOffset || m_iOffsetFirst+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		height = pPropRun->getHeight();
	}
	else
	{
		height = m_iHeight;
	}

	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;

	if (iOffset == m_iOffsetFirst+1)
	{
	    FV_View* pView = m_pBL->getDocLayout()->getView();
	    if (pView->getShowPara())
		{
			x += m_iWidth;
	    }
	}

	x2 = x;
	y2 = y;
	//UT_DEBUGMSG(("fintPointCoords: ForcedLineBreakRun: x=%d, y=%d, h=%d\n", x,y,height));	
}

void fp_ForcedLineBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedLineBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldStartRun::fp_FieldStartRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELDSTARTRUN)
{
	lookupProperties();
}

void fp_FieldStartRun::lookupProperties(void)
{
	m_pBL->getField(m_iOffsetFirst,m_pField);
	m_iWidth = 0;
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
	
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = false;
	bEOL = false;
}


void fp_FieldStartRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldStartRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldStartRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_FieldEndRun::fp_FieldEndRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELDENDRUN)
{
	lookupProperties();
}

void fp_FieldEndRun::lookupProperties(void)
{
	m_pBL->getField(m_iOffsetFirst,m_pField);
	m_iWidth = 0;
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
	
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = false;
	bEOL = false;
}

void fp_FieldEndRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_FieldEndRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_FieldEndRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_EndOfParagraphRun::fp_EndOfParagraphRun(fl_BlockLayout* pBL, 
										   GR_Graphics* pG, UT_uint32 iOffsetFirst, 
										   UT_uint32 iLen)
	: fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_ENDOFPARAGRAPH)
{
	
	m_iLen = 1;
	m_bDirty = true;
#ifdef BIDI_ENABLED
	UT_ASSERT((pBL));
	m_iDirection = pBL->getDominantDirection();
#endif
	lookupProperties();
}

void fp_EndOfParagraphRun::lookupProperties(void)
{
	//UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties\n"));
	_inheritProperties();

	FV_View* pView = m_pBL->getDocLayout()->getView();
	if (pView && pView->getShowPara())
	{
		// Find width of Pilcrow
		UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
		UT_uint32 iTextLen = UT_UCS_strlen(pEOP);

		fp_Run* pPropRun = _findPrevPropertyRun();
		if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
			m_pG->setFont(pTextRun->getFont());
		}
		else
		{
			const PP_AttrProp * pSpanAP = NULL;
			const PP_AttrProp * pBlockAP = NULL;
			const PP_AttrProp * pSectionAP = NULL;
			m_pBL->getSpanAttrProp(m_iOffsetFirst,true,&pSpanAP);
			m_pBL->getAttrProp(&pBlockAP);
			// look for fonts in this DocLayout's font cache
			FL_DocLayout * pLayout = m_pBL->getDocLayout();

			GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, 
											   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
			m_pG->setFont(pFont);
		}
		m_iWidth  = m_pG->measureString(pEOP, 0, iTextLen, NULL);
		xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::lookupProperties: width %d\n", m_iWidth));
	}
	else
	{
		// FIXME:jskov This should probably be the width of the
		// document to the right of the pilcrow, see Paul's suggested
		// selection behaviors. Doesn't matter until we get selection
		// support though (which requires PT changes).
		m_iWidth = 8;
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
	pos = m_pBL->getPosition() + m_iOffsetFirst;
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
	//	UT_ASSERT(m_iOffsetFirst == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	height = m_iHeight;
	m_pLine->getOffsets(this, xoff, yoff);
	x = x2 = xoff;
	y = y2 = yoff;

	if (pPropRun)
	{
		//UT_DEBUGMSG(("fp_EndOfParagraphRun::findPointCoords: prop. run type=%d\n", pPropRun->getType()));
		height = pPropRun->getHeight();
		// If property Run is on the same line, get y location from
		// it (to reflect proper ascent).
		if (pPropRun->getLine() == m_pLine)
		{
#ifdef BIDI_ENABLED
			pPropRun->findPointCoords(iOffset, x, y, x2, y2, height, bDirection);
#else		
			m_pLine->getOffsets(pPropRun, xoff, y);
#endif
		}
	}
	//UT_DEBUGMSG(("fintPointCoords: EndOfParagraphRun 0x%x: x,y,x2,y2=[%d,%d,%d,%d]\n", this,x,y,x2,y2));
	
}

void fp_EndOfParagraphRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	FV_View* pView = m_pBL->getDocLayout()->getView();
		
	UT_sint32 xoff = 0, yoff = 0;
	m_pLine->getScreenOffsets(this, xoff, yoff);
	m_pG->fillRect(m_colorPG, xoff, yoff, m_iWidth, m_pLine->getHeight());
	UT_DEBUGMSG(("SEVIOR: Doing clear screen in End of Pargraph run \n"));
//	m_pG->fillRect(m_colorPG, m_iXoffText, m_iYoffText, m_iWidth, m_pLine->getHeight());
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
	UT_ASSERT(pDA->pG == m_pG);

	UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;

	FV_View* pView = m_pBL->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
	UT_ASSERT(iSel1 <= iSel2);
	
	bool bIsSelected = false;
	if (pView->getFocus()!=AV_FOCUS_NONE &&	(iSel1 <= iRunBase) && (iSel2 > iRunBase))
		bIsSelected = true;

	/*
	  TODO this should not be hard-coded.  We should calculate an
	  appropriate selection background color based on the color
	  of the foreground text, probably.
	*/
	UT_RGBColor clrSelBackground(192, 192, 192);
	UT_RGBColor clrShowPara(127,127,127);

	UT_UCSChar pEOP[] = { UCS_PILCROW, 0 };
	UT_uint32 iTextLen = UT_UCS_strlen(pEOP);
	UT_sint32 iAscent;

	fp_Run* pPropRun = _findPrevPropertyRun();
	if (pPropRun && (FPRUN_TEXT == pPropRun->getType()))
	{
		fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPropRun);
		m_pG->setFont(pTextRun->getFont());
		iAscent = pTextRun->getAscent();
	}
	else
	{
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;
		m_pBL->getSpanAttrProp(m_iOffsetFirst,true,&pSpanAP);
		m_pBL->getAttrProp(&pBlockAP);
		// look for fonts in this DocLayout's font cache
		FL_DocLayout * pLayout = m_pBL->getDocLayout();

		GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, 
										   FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
		m_pG->setFont(pFont);
		iAscent = m_pG->getFontAscent();
	}

	m_iWidth  = m_pG->measureString(pEOP, 0, iTextLen, NULL);
	m_iHeight = m_pG->getFontHeight();
	m_iXoffText = pDA->xoff;
	m_iYoffText = pDA->yoff - iAscent; 
	xxx_UT_DEBUGMSG(("fp_EndOfParagraphRun::draw: width %d\n", m_iWidth));

	if (bIsSelected)
	{
		m_pG->fillRect(clrSelBackground, m_iXoffText, m_iYoffText, m_iWidth, m_pLine->getHeight());
		UT_setColor(clrShowPara, 80, 80, 80);
	}
	else
	{
		m_pG->fillRect(m_colorPG, m_iXoffText, m_iYoffText, m_iWidth, m_pLine->getHeight());
	}
	if (pView->getShowPara())
	{
		// Draw pilcrow
		m_pG->setColor(clrShowPara);
        m_pG->drawChars(pEOP, 0, iTextLen, m_iXoffText, m_iYoffText);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


fp_ImageRun::fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, GR_Image* pImage) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_IMAGE)
{
#if 0	// put this back later
	UT_ASSERT(pImage);
#endif
	
	m_pImage = pImage;
	
	lookupProperties();
}

fp_ImageRun::~fp_ImageRun()
{
	if (m_pImage)
	{
		delete m_pImage;
	}
}

void fp_ImageRun::lookupProperties(void)
{
	m_pBL->getField(m_iOffsetFirst,m_pField);
	if (m_pImage)
	{
		m_iWidth = m_pImage->getDisplayWidth();
		m_iHeight = m_pImage->getDisplayHeight();
		m_iWidthLayoutUnits = m_pImage->getLayoutWidth();
		m_iHeightLayoutUnits = m_pImage->getLayoutHeight();
	}
	else
	{
		// If we have no image, we simply insert a square "slug"
			
		m_iWidth = m_pG->convertDimension("0.5in");
		m_iHeight = m_pG->convertDimension("0.5in");
		m_iWidthLayoutUnits = UT_convertToLayoutUnits("0.5in");
		m_iHeightLayoutUnits = UT_convertToLayoutUnits("0.5in");
	}
		
	UT_ASSERT(m_iWidth > 0);
	UT_ASSERT(m_iHeight > 0);
	m_iImageWidth = m_iWidth;
	m_iImageWidthLayoutUnits = m_iWidthLayoutUnits;
	m_iImageHeight = m_iHeight;
	m_iImageHeightLayoutUnits = m_iHeightLayoutUnits;
//
// This code deals with too big images
//
//
	if(getLine() != NULL)
	{
		if(getLine()->getMaxWidth() - 1 < m_iWidth)
		{
			double dw = (double) getLine()->getMaxWidth();
			double dwL = getLine()->getMaxWidthInLayoutUnits();
			double dnwL = dwL - dwL/dw + 0.5; 
			m_iWidth = getLine()->getMaxWidth() -1;
			m_iWidthLayoutUnits = (UT_sint32) dnwL;
		}
		if(getLine()->getContainer() != NULL && 
		   getLine()->getContainer()->getMaxHeight() - 1 < m_iHeight)
		{
			double dh = (double) getLine()->getContainer()->getMaxHeight();
			double dhL = (double) getLine()->getContainer()->getMaxHeightInLayoutUnits();
			double dnhL = dhL - dhL/dh + 0.5; 
			m_iHeight = getLine()->getContainer()->getMaxHeight() -1;
			m_iHeightLayoutUnits = (UT_sint32) dnhL;
		}
	}
	m_iAscent = m_iHeight;
	m_iDescent = 0;
	m_iAscentLayoutUnits = m_iHeightLayoutUnits;
	m_iDescentLayoutUnits = 0;
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

void fp_ImageRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	if (x > m_iWidth)
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
	else
		pos = m_pBL->getPosition() + m_iOffsetFirst;

	bBOL = false;
	bEOL = false;
}

void fp_ImageRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ImmageRun\n"));	
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	if (iOffset == (m_iOffsetFirst + m_iLen))
	{
		x = xoff + m_iWidth;
#ifdef BIDI_ENABLED
		x2 = x;
#endif
	}
	else
	{
		x = xoff;
	}
	y = yoff;
	height = m_iHeight;
#ifdef BIDI_ENABLED
	y2 = y;
	bDirection = (getVisDirection() != 0);
#endif
}

void fp_ImageRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = m_pLine->getHeight();

	m_pG->fillRect(m_colorPG,xoff, yoff, m_iWidth, iLineHeight);
}

void fp_ImageRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

	UT_sint32 xoff = 0, yoff = 0;
	m_pLine->getScreenOffsets(this, xoff, yoff);

	yoff += m_pLine->getAscent() - m_iAscent;
	
	// clip drawing to the page
	UT_Rect pClipRect;
	pClipRect.top = yoff;
	pClipRect.left = xoff;
	pClipRect.height = m_pLine->getContainer()->getHeight();
	pClipRect.width = m_pLine->getContainer()->getWidth();
	m_pG->setClipRect(&pClipRect);

	if (m_pImage)
	{
		// draw the image (always)
		m_pG->drawImage(m_pImage, xoff, yoff);

		// if we're the selection, draw a pretty box
		if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;

			FV_View* pView = m_pBL->getDocLayout()->getView();
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
				UT_Point pts[5];

				UT_uint32 top = yoff;
				UT_uint32 left = xoff;
				UT_uint32 right = xoff + m_iWidth - 1;
				UT_uint32 bottom = yoff + m_iHeight - 1;
									
				pts[0].x = left; 	pts[0].y = top;
				pts[1].x = right;	pts[1].y = top;
				pts[2].x = right;	pts[2].y = bottom;
				pts[3].x = left; 	pts[3].y = bottom;
				pts[4].x = left;	pts[4].y = top;
				
				// TODO : remove the hard-coded (but pretty) blue color

				UT_RGBColor clr(0, 0, 255);
				m_pG->setColor(clr);
				m_pG->polyLine(pts, 5);
				
			}
		}
		
	}
	else
	{
		UT_RGBColor clr(0, 0, 255);
		m_pG->fillRect(clr, xoff, yoff, m_iWidth, m_iHeight);
	}

	// unf*ck clipping rect
	m_pG->setClipRect(NULL);
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
		m_pFontLayout(0),
		m_iFieldType(FPFIELD_start)
{
	bool gotField = pBL->getField(iOffsetFirst,m_pField);
	UT_ASSERT(gotField);
	m_sFieldValue[0] = 0;
}

bool fp_FieldRun::_setValue(UT_UCSChar *p_new_value)
{

	if (0 != UT_UCS_strcmp(p_new_value, m_sFieldValue))
	{
		clearScreen();

		UT_UCS_strcpy(m_sFieldValue, p_new_value);

		{
			unsigned short aCharWidths[FPFIELD_MAX_LENGTH];
			lookupProperties();
			m_pG->setFont(m_pFont);
			UT_sint32 iNewWidth = m_pG->measureString(m_sFieldValue, 0, UT_UCS_strlen(m_sFieldValue), aCharWidths);
			if (iNewWidth != m_iWidth)
			{
				clearScreen();
				m_iWidth = iNewWidth;

				m_pG->setFont(m_pFontLayout);
				m_iWidthLayoutUnits = m_pG->measureString(m_sFieldValue, 0, UT_UCS_strlen(m_sFieldValue), aCharWidths);

				return true;
			}

			return false;
		}
	}

	return false;
}

void fp_FieldRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	//	UT_DEBUGMSG(("SEVIOR: Doing Lookupprops for block %x run %x  offset =%d \n ",m_pBL,this,m_iOffsetFirst));
	UT_ASSERT(pSpanAP);
	PD_Document * pDoc = m_pBL->getDocument();
	m_pBL->getAttrProp(&pBlockAP);
	m_pBL->getField(m_iOffsetFirst+1,m_pField); // Next Pos?
	if(m_pField != NULL)
	{
		m_pField->setBlock(m_pBL);
	}
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	if(m_iFieldType == FPFIELD_list_label)
	{
		m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION,true);
		m_pFontLayout = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION,true);
	}
	else
	{
		m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
		m_pFontLayout = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
	}

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, m_pBL->getDocument(), true), m_colorFG);
	
	getHighlightColor(); 
	getPageColor();
	
	const char * pszFieldColor = NULL;
	pszFieldColor = PP_evalProperty("field-color",pSpanAP,pBlockAP,pSectionAP, m_pBL->getDocument(), true);

//
// FIXME: The "ffffff" is for backwards compatibility. If we don't exclude this 
// no prexisting docs will be able to change the Highlight color in paragraphs 
// with lists. I think this is a good solution for now. However it does mean
// field-color of "ffffff", pure white is actually transparent.
//
	if(pszFieldColor && UT_strcmp(pszFieldColor,"transparent") != 0 && UT_strcmp(pszFieldColor,"ffffff" ) != 0 )
		UT_parseColor(pszFieldColor, m_colorHL); 

	m_iAscent = m_pG->getFontAscent(m_pFont);	
	m_iDescent = m_pG->getFontDescent(m_pFont);
	m_iHeight = m_pG->getFontHeight(m_pFont);

	m_iAscentLayoutUnits = m_pG->getFontAscent(m_pFontLayout);	
	m_iDescentLayoutUnits = m_pG->getFontDescent(m_pFontLayout);
	m_iHeightLayoutUnits = m_pG->getFontHeight(m_pFontLayout);

	//m_pG->setFont(m_pFont); DOM!!

	const XML_Char* pszType = NULL;

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

	pSpanAP->getAttribute("type", pszType);

	// i leave this in because it might be obscuring a larger bug
	//UT_ASSERT(pszType);
	if (!pszType) return;

#ifdef BIDI_ENABLED
	//#TF need to retrieve the direction of this run
	//PD_Document * pDoc = m_pBL->getDocument();
	const XML_Char * pszDirection = PP_evalProperty("dir",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	if(!UT_stricmp(pszDirection, "rtl"))
	{
		m_iDirection = 1;
	}
	else
	{
		m_iDirection = 0;
	}
#endif

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
	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,  m_pBL->getDocument(), true);
	m_iLineWidth = getToplineThickness();
	m_fDecorations = 0;
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
			m_fDecorations |= TEXT_DECOR_UNDERLINE;
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			m_fDecorations |= TEXT_DECOR_OVERLINE;
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
			m_fDecorations |= TEXT_DECOR_LINETHROUGH;
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			m_fDecorations |= TEXT_DECOR_TOPLINE;
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			m_fDecorations |= TEXT_DECOR_BOTTOMLINE;
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

void fp_FieldRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	// If X is left of the middle, return offset to the left,
	// otherwise the offset to the right.
	if (x < (getWidth() / 2))
		pos = m_pBL->getPosition() + m_iOffsetFirst;
	else
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
		
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

	UT_ASSERT(m_pLine);

	lookupProperties();
	
	m_pLine->getOffsets(this, xoff, yoff);

	if (iOffset == (m_iOffsetFirst + m_iLen))
	{
		xoff += m_iWidth;
	}
	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += m_iDescent /* * 3/2 */;
	}
 	x = xoff;
	y = yoff;
	height = m_iHeight;
#ifdef BIDI_ENABLED
	x2 = x;
	y2 = y;
	bDirection = (getVisDirection() != 0);
#endif
}

bool fp_FieldRun::calculateValue(void)
{
	//
	// Code for the Piece Table Fields Calculation
	// Get size of the field from the following runs
	//
	//      return m_pField->update();
	//        UT_ASSERT(m_pField);

  /*        UT_sint32 count = 0;
        fp_Run* pNext = getNext();
	while(pNext != NULL && pNext->getField() != NULL )
	{
	        if(m_pField == NULL)
		{
		        m_pField = pNext->getField();
		}
	        pNext = getNext();
		count++;
	}
	if( count == 0)
	{
	        m_iWidth = 0;
		m_iHeight = 0;
		m_iWidthLayoutUnits = 0;
		m_iHeightLayoutUnits = 0;
	}
	else
        {
	        pNext = getPrev();
	        m_iWidth = pNext->getWidth();
		m_iHeight = pNext->getHeight();
		m_iWidthLayoutUnits = pNext->getWidthInLayoutUnits();
		m_iHeightLayoutUnits = pNext->getHeightInLayoutUnits();
	}
	if(m_pField != NULL)
	m_pField->update();
*/
	return true;
}

void fp_FieldRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);

	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = m_pLine->getHeight();
//
// Sevior was here
//	m_pG->fillRect(m_colorPG, xoff, yoff-1, m_iWidth, iLineHeight);
	m_pG->fillRect(m_colorPG, xoff, yoff, m_iWidth, iLineHeight);
}

void fp_FieldRun::_defaultDraw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

	lookupProperties();
	UT_sint32 xoff = 0, yoff = 0;
	
	// need screen locations of this run

	m_pLine->getScreenOffsets(this, xoff, yoff);

	UT_sint32 iYdraw =  pDA->yoff - getAscent()-1;
	
	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		iYdraw -= getAscent() * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		iYdraw +=  getDescent(); // * 3/2
	}

	//if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;

//
// Sevior was here		
//		UT_sint32 iFillTop = iYdraw;
		UT_sint32 iFillTop = iYdraw+1;
		UT_sint32 iFillHeight = getAscent() + getDescent();
		
		FV_View* pView = m_pBL->getDocLayout()->getView();
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
		  /*
		    TODO: we might want special colors for fields.  We might 
		    also want the colors to be calculated on the fly instead of
		    hard-coded.  See comment above in fp_TextRun::_draw*.
		  */
		        UT_RGBColor clrSelBackground(112, 112, 112);
				m_pG->fillRect(clrSelBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);

		}
		else
		{
			getHighlightColor();
			m_pG->fillRect(m_colorHL, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
		}
	}
	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);
	
	m_pG->drawChars(m_sFieldValue, 0, UT_UCS_strlen(m_sFieldValue), pDA->xoff,iYdraw);
//
// Draw underline/overline/strikethough
//	
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove 
	                                                 //character dirt
	drawDecors( xoff, yTopOfRun);

}

// BEGIN DOM work on some new fields

static FV_View *
_getViewFromBlk(fl_BlockLayout* pBlock)
{
	FV_View *pView    = pBlock->getView();	
	return pView;
}

fp_FieldCharCountRun::fp_FieldCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldCharCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	szFieldValue[0] = 0;

	FV_View *pView = _getViewFromBlk(m_pBL);
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();	    
	    sprintf(szFieldValue, "%d", cnt.ch_sp);
	}
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	FV_View *pView = _getViewFromBlk(m_pBL);
	if(!pView)
	{
		strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();	    
	    sprintf(szFieldValue, "%d", cnt.ch_no);
	}

	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	FV_View *pView = _getViewFromBlk(m_pBL);
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();	    
	    sprintf(szFieldValue, "%d", cnt.line);
	}

	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	FV_View *pView = _getViewFromBlk(m_pBL);
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();	    
	    sprintf(szFieldValue, "%d", cnt.para);
	}

	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	FV_View *pView = _getViewFromBlk(m_pBL);
	if(!pView)
	{
	    strcpy(szFieldValue, "?");
	}
	else
	{
	    FV_DocCount cnt = pView->countWords();	    
	    sprintf(szFieldValue, "%d", cnt.word);
	}

	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
	  m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) UT_strdup(szFieldValue));
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildIdRun::fp_FieldBuildIdRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildIdRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_ID);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_ID);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildVersionRun::fp_FieldBuildVersionRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildVersionRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Version);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_Version);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildOptionsRun::fp_FieldBuildOptionsRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildOptionsRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Options);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_Options);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildTargetRun::fp_FieldBuildTargetRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildTargetRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_Target);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_Target);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildCompileDateRun::fp_FieldBuildCompileDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileDateRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_CompileDate);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_CompileDate);
	return _setValue(sz_ucs_FieldValue);
}

fp_FieldBuildCompileTimeRun::fp_FieldBuildCompileTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldBuildCompileTimeRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	UT_UCS_strcpy_char(sz_ucs_FieldValue, XAP_App::s_szBuild_CompileTime);
	if (m_pField)
		m_pField->setValue((XML_Char*) XAP_App::s_szBuild_CompileTime);
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
		if (someid && UT_strcmp(someid, endid)==0)
			break;
		pBL = pBL->getNext(); 

		// TODO HACK until we stop propagating endnote-ids.
		if (!previd || (previd && someid && UT_strcmp(someid, previd) != 0))
			endnoteNo++;

		if (previd != NULL)
			free(previd);
		previd = UT_strdup(someid);
	}
	if (previd != NULL)
		free(previd);

	return endnoteNo;
}

// Refers to an endnote in the main body of the text.
fp_FieldEndnoteRefRun::fp_FieldEndnoteRefRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldEndnoteRefRun::calculateValue(void)
{
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);

	// What we want to do now is to search in the containing
	// sectionLayout's EndnoteSection for all of the endnotes which
	// are numbered before us.  This assumes, of course, that we want
	// numeric endnotes.  I'm not sure how to do otherwise at the
	// moment.  Probably a paragraph property or something.  or maybe
	// we put it directly on the endnote anchor. -PL

	// Clearly this should be cached or something, later.  It's updated
	// far, far too often.

	fl_DocSectionLayout * pEndSL = getBlock()->
		getDocSectionLayout()->getEndnote();

	// Hmm, this is kind of messy; we can be called before
	// the endnote section has been added.
	if (pEndSL == NULL)
	{
		UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
		UT_UCS_strcpy_char(sz_ucs_FieldValue, "?");
		return _setValue(sz_ucs_FieldValue);
	}
	UT_ASSERT(pEndSL->getType() == FL_SECTION_ENDNOTE);

	// Now, count out how many paragraphs have special endnote-id tags
	// until we reach the desired paragraph.  (para == block)
	
	fl_BlockLayout * pBL = pEndSL->getFirstBlock();
	int endnoteNo = countEndnotesBefore(pBL, endid);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	// How do we superscript the endnote?
	snprintf(szFieldValue, FPFIELD_MAX_LENGTH, "[%d]", endnoteNo);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldEndnoteAnchorRun::fp_FieldEndnoteAnchorRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

// Appears in the EndnoteSection, one per endnote.
bool fp_FieldEndnoteAnchorRun::calculateValue(void)
{
	const PP_AttrProp * pp = getAP();
	const XML_Char * endid;
	bool bRes = pp->getAttribute("endnote-id", endid);

	UT_ASSERT(bRes);

	// What we want to do now is to search in the containing
	// sectionLayout's EndnoteSection for all of the endnotes which
	// are numbered before us.  This assumes, of course, that we want
	// numeric endnotes.  I'm not sure how to do otherwise at the
	// moment.  Probably a paragraph property or something.  or maybe
	// we put it directly on the endnote anchor. -PL

	// Clearly this should be cached or something, later.  It's updated
	// far, far too often.

	fl_DocSectionLayout * pEndSL = getBlock()->
		getDocSectionLayout();
	UT_ASSERT((pEndSL->getType() == FL_SECTION_ENDNOTE));

	// Now, count out how many paragraphs have special endnote-id tags
	// until we reach the desired paragraph.  (para == block)

	// should this actually be refactored?

	fl_BlockLayout * pBL = pEndSL->getFirstBlock();
	int endnoteNo = countEndnotesBefore(pBL, endid);

	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	char szFieldValue[FPFIELD_MAX_LENGTH + 1];

	snprintf(szFieldValue, FPFIELD_MAX_LENGTH, "[%d] ", endnoteNo);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%I:%M:%S %p", pTime);
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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
	if (m_pField)
		m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	PD_Document * pDoc = m_pBL->getDocument();
	UT_ASSERT(pDoc);

	//copy in the name or some wierd char instead
	const char * name = pDoc->getFileName();
	if (!name)
		name = "*";

	strcpy (szFieldValue, name);

	if (m_pField)
	  m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

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

	if (m_pLine && m_pLine->getContainer() && m_pLine->getContainer()->getPage())
	{
		fp_Page* pPage = m_pLine->getContainer()->getPage();
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
	if (m_pField)
	  m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}

fp_FieldPageCountRun::fp_FieldPageCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

bool fp_FieldPageCountRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	char szFieldValue[FPFIELD_MAX_LENGTH + 1];
	
	if (m_pLine && m_pLine->getContainer() && m_pLine->getContainer()->getPage())
	{

		fp_Page* pPage = m_pLine->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		sprintf(szFieldValue, "%d", pDL->countPages());
	}
	else
	{
		strcpy(szFieldValue, "?");
	}
	if (m_pField)
	  m_pField->setValue((XML_Char*) szFieldValue);

	UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);

	return _setValue(sz_ucs_FieldValue);
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedColumnBreakRun::fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDCOLUMNBREAK)
{
	lookupProperties();
}

void fp_ForcedColumnBreakRun::lookupProperties(void)
{
	m_pBL->getField(m_iOffsetFirst,m_pField);

	_inheritProperties();
	m_iWidth = 1;
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
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = false;
	bEOL = false;
}

void fp_ForcedColumnBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ForcedColumnBreakRun\n"));
	UT_ASSERT(m_iOffsetFirst == iOffset || m_iOffsetFirst+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		height = pPropRun->getHeight();
	}
	else
	{
	    height = m_iHeight;
	}

	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;

#ifdef BIDI_ENABLED
	x2 = x;
	y2 = y;
#endif
}

void fp_ForcedColumnBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    m_pLine->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = m_pLine->getMaxWidth() - m_pLine->calculateWidthOfLine();
    m_pG->fillRect(m_colorPG,xoff,yoff,iWidth,m_pLine->getHeight());
}

void fp_ForcedColumnBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

    if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = m_pBL->getDocLayout()->getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = m_pLine->getMaxWidth();

    UT_UCSChar *pColumnBreak;
    UT_UCS_cloneString_char(&pColumnBreak,"Column Break");
	_drawTextLine(pDA->xoff,pDA->yoff+m_pLine->getAscent(),iLineWidth,m_pLine->getHeight(),pColumnBreak);
    FREEP(pColumnBreak);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fp_ForcedPageBreakRun::fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDPAGEBREAK)
{
	lookupProperties();
}

void fp_ForcedPageBreakRun::lookupProperties(void)
{
	m_pBL->getField(m_iOffsetFirst,m_pField);

	_inheritProperties();
	m_iWidth = 1;
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
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = false;
	bEOL = false;
}

void fp_ForcedPageBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ForcedPageBreakRun\n"));	
	UT_ASSERT(m_iOffsetFirst == iOffset || m_iOffsetFirst+1 == iOffset);

	UT_sint32 xoff, yoff;

	fp_Run* pPropRun = _findPrevPropertyRun();

	if (pPropRun)
	{
		height = pPropRun->getHeight();
	}
	else
	{
		height = m_iHeight;
	}

	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;

	if (iOffset == m_iOffsetFirst+1)
	{
	    FV_View* pView = m_pBL->getDocLayout()->getView();
	    if (pView->getShowPara())
		{
			x += m_iWidth;
	    }
	}

#ifdef BIDI_ENABLED
	x2 = x;
	y2 = y;
#endif
}

void fp_ForcedPageBreakRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

    UT_sint32 xoff = 0, yoff = 0;
    m_pLine->getScreenOffsets(this, xoff, yoff);
    UT_sint32 iWidth  = m_pLine->getMaxWidth() - m_pLine->calculateWidthOfLine();
    m_pG->fillRect(m_colorPG,xoff,yoff,iWidth,m_pLine->getHeight());
}

void fp_ForcedPageBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

    if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))){
        return;
    }

    FV_View* pView = m_pBL->getDocLayout()->getView();
    UT_ASSERT(pView);
    if(!pView->getShowPara()){
        return;
    }

    UT_sint32 iLineWidth  = m_pLine->getMaxWidth();

    UT_UCSChar *pPageBreak;
    UT_UCS_cloneString_char(&pPageBreak,"Page Break");

	_drawTextLine(pDA->xoff,pDA->yoff+m_pLine->getAscent(),iLineWidth,m_pLine->getHeight(),pPageBreak);
    FREEP(pPageBreak);
}


#ifdef BIDI_ENABLED
////////////////////////////////////////////////////////////////////////
// the following functions are used in the BiDi implementation #TF

// translates logical position in a run into visual position
// (will also translate correctly visual -> logical)
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos)
{
    if(getVisDirection() == 1) //rtl needs translation
    {
        return (m_iLen - iLogPos - 1);
    }
    else return (iLogPos);
}

//translates a visual position in a span of length iLen to logical pos
//or vice versa
UT_uint32 fp_Run::getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen)
{
    if(getVisDirection() == 1) //rtl needs translation
    {
        return (iLen - iLogPos - 1);
    }
    else return (iLogPos);
}

//returns the logical offset of the first visual character
UT_uint32 fp_Run::getOffsetFirstVis()
{
    if(getVisDirection() == 1) //rtl, requires translation
    {
        return(m_iOffsetFirst + m_iLen - 1);
    }
    else return (m_iOffsetFirst);
}

//translates visual offset to logical one, can be also used for translation
//in the other direction
UT_uint32 fp_Run::getOffsetLog(UT_uint32 iVisOff)
{
    if(getVisDirection() == 1) //rtl needs translation
    {
        return(m_iOffsetFirst + m_iLen - iVisOff + m_iOffsetFirst - 1);
    }
    else return (iVisOff);
}

void fp_Run::setDirection(UT_sint32 iDir)
{
	//this function should be called with -1,0,1; if it is called
	//with -2, which is used in the derived classes that can handle
	//Unicode as an indication that direction should be worked out
	//from Unicode, we will treat this a whitespace

	UT_sint32 iDirection = iDir != -2 ? iDir : -1;
	if(m_iDirection != iDirection)
	{
		m_iDirection = iDirection;
		clearScreen();
		/*
		  if this run belongs to a line we have to notify the line that
		  that it now contains a run of this direction, if it does not belong
		  to a line this will be taken care of by the fp_Line:: member function
		  used to add the run to the line (generally, we set it here if this
		  is a run that is being typed in and it gets set in the member
		  functions when the run is loaded from a document on the disk.)
		*/
	
		if(m_pLine)
			m_pLine->addDirectionUsed(m_iDirection);

	}
}

// returns the direction with which the run is displayed, converting the direction of white
// space to either ltr or rtl depending on context
UT_sint32 fp_Run::getVisDirection()
{
    if(m_iDirection == 2) //direction not set, yet, use dominant direction;
    {
        return (m_pBL->getDominantDirection());
    }

    if(m_iDirection != -1)
    {
        //UT_DEBUGMSG(("getVisDirection (non-white): direction = %d, visDirection = %d\n", m_iDirection, m_iDirection));
        return(m_iDirection);
    }

    UT_sint32 iBlDirection = (UT_sint32) m_pBL->getDominantDirection();
    fp_Run * r = getPrev();
    UT_sint32 prevDir;

    // find first non-white run before this one
    while (r)
    {
        prevDir = r->getDirection();
        r = r->getPrev();

        if(prevDir == iBlDirection)
        {
            //UT_DEBUGMSG(("getVisDirection (prev): direction = %d, visDirection = %d\n", m_iDirection, iBlDirection));
            return(iBlDirection);
        }
        if(prevDir != -1) break; //run of foreign direction
    }

    // if there are no non-white runs, return direction of the block
    if(r == 0)
    {
        //UT_DEBUGMSG(("getVisDirection (non-white start): direction = %d, visDirection = %d\n", m_iDirection, iBlDirection));
        return(iBlDirection);
    }

    // now we know that the preceding run is of foreing direction, in which case
    // the direction of white space depends on the first non-white run that follows it
    UT_sint32 nextDir;
    r = getNext();

    while(r)
    {
        nextDir = r->getDirection();

        if(nextDir != -1)
        {
            //UT_DEBUGMSG(("getVisDirection (next): direction = %d, visDirection = %d\n", m_iDirection, nextDir));
            //last run pefore end of paragraph mark will have direction of the run
            //that precedes it, otherwise the direction of the next run
            return(r->getType() == FPRUN_ENDOFPARAGRAPH) ? prevDir : nextDir;
        }
        r = r->getNext();

    }

    // if get this far, this white space run is the last one on the line
    // and will have the direciton of the previous run
    //UT_DEBUGMSG(("getVisDirection (last on line): direction = %d, visDirection = %d\n", m_iDirection, prevDir));
    return(prevDir);
}

/*
bool fp_Run::setUnicodeDirection()
{
	//bool bSetProperty = (m_iDirection == 2) ? true : false; //do we need to call View->setCharFormat ?
	
	m_iDirection = -1; //all runs that rely on this implementatio will be treated as whitespace
	
	//if this run did not have its direction set so far we have to call View->setCharFormat
	//if(bSetProperty)
	//    setDirectionProperty(-1);
	
	return (false);
}
*/
void fp_Run::setDirectionProperty(UT_sint32 dir)
{
	//FV_View * pView = getBlock()->getDocLayout()->getView();
	//UT_ASSERT(pView);
	const XML_Char * prop[] = {NULL, NULL, 0};
	const XML_Char direction[] = "dir";
	const XML_Char rtl[] = "rtl";
	const XML_Char ltr[] = "ltr";
	const XML_Char neutral[] = "ntrl";
	
	prop[0] = (XML_Char*) &direction;
	
	switch(dir)
	{
	case 0:  prop[1] = (XML_Char*) &ltr;     break;
	case 1:  prop[1] = (XML_Char*) &rtl;     break;
	case -1: prop[1] = (XML_Char*) &neutral; break;
	default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	};
	
	//pView->setCharFormat(prop);
	UT_uint32 offset = m_pBL->getPosition() + m_iOffsetFirst;
	getBlock()->getDocument()->changeSpanFmt(PTC_AddFmt,offset,offset + m_iLen,NULL,prop);
	//UT_DEBUGMSG(("Run::setDirectionProperty: offset=%d, len=%d\n", offset,m_iLen));
}

#endif //BIDI_ENABLED
