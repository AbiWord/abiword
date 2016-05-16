/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2007 Martin Sevior
 * Copyright (C) 2011 Ben Martin
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


#include "fp_Run.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fv_View.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "gr_DrawArgs.h"

#include "pd_DocumentRDF.h"

#include <string.h>

fp_RDFAnchorRun::fp_RDFAnchorRun( fl_BlockLayout* pBL,
                                  UT_uint32 iOffsetFirst, 
                                  UT_uint32 /*iLen*/ )
    : fp_HyperlinkRun(pBL,iOffsetFirst,1)
    , m_iPID(0)
    , m_sValue("")
    , m_iRealWidth(0)
{
    UT_ASSERT(pBL);
	_setLength(1);
	_setDirty(false);
	_setWidth(0);
	_setRecalcWidth(true);
	
	UT_ASSERT((pBL));
	_setDirection(UT_BIDI_WS);

    _setTargetFromAPAttribute( "AnnotationX" );
    if( m_pTarget )
    {
//        m_iPID = atoi(m_pTarget);
    }

    // _setTarget( "fake target" );
    // m_bIsStart = true;
    
	const PP_AttrProp * pAP = NULL;
	getSpanAP(pAP);

    RDFAnchor a( pAP );
    _setTarget( a.getID().c_str() );
    m_bIsStart = !a.isEnd();
    if( m_bIsStart )
    {
        _setHyperlink(this);
    }

	lookupProperties();

}




fp_RDFAnchorRun::~fp_RDFAnchorRun()
{
	DELETEPV(m_pTarget);
}


void fp_RDFAnchorRun::_draw(dg_DrawArgs* pDA)
{
//    UT_DEBUGMSG(("_draw() showan:%d isStart:%d\n", displayAnnotations(), m_bIsStart ));

	if(!displayAnnotations()) {
		return;
	}
	if(!m_bIsStart) {
		return;
	}

	GR_Graphics * pG = pDA->pG;

	UT_sint32 xoff = 0, yoff = 0;
	GR_Painter painter(pG);

	// need screen locations of this run

	getLine()->getScreenOffsets(this, xoff, yoff);

	UT_sint32 iYdraw =  pDA->yoff - getAscent()-1;

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();
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
	    pG->setColor(_getView()->getColorAnnotation(this));
	    painter.fillRect(color, pDA->xoff, iFillTop, getWidth(), iFillHeight);

	}
	else
        {
	    Fill(getGraphics(),pDA->xoff, iFillTop, getWidth(), iFillHeight);
	    pG->setColor(_getColorFG());
	}
	pG->setFont(_getFont());
	pG->setColor(_getView()->getColorAnnotation(this));
//	UT_DEBUGMSG(("Drawing string m_sValue %s \n",m_sValue.utf8_str()));
	painter.drawChars(m_sValue.ucs4_str().ucs4_str(), 0,m_sValue.ucs4_str().size(), pDA->xoff,iYdraw, NULL);
//
// Draw underline/overline/strikethough
//
	UT_sint32 yTopOfRun = pDA->yoff - getAscent()-1; // Hack to remove
	                                                 //character dirt
	drawDecors( xoff, yTopOfRun,pG);

}

void fp_RDFAnchorRun::_lookupProperties( const PP_AttrProp * pSpanAP,
                                         const PP_AttrProp * pBlockAP,
                                         const PP_AttrProp * pSectionAP,
                                         GR_Graphics * pG )
{

	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,pG);
	if(pFont == NULL)
	{
	    pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,getGraphics());
	    UT_ASSERT_HARMLESS(pFont);
	}

	if (pFont != _getFont())
	{
	    _setFont(pFont);
	    _setAscent(getGraphics()->getFontAscent(pFont));
	    _setDescent(getGraphics()->getFontDescent(pFont));
	    _setHeight(getGraphics()->getFontHeight(pFont));
	}
}



void fp_RDFAnchorRun::_clearScreen(bool /*bFullLineHeightRect*/)
{
    if(getWidth() == 0)
        return;

	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	Fill(getGraphics(), xoff, yoff, getWidth(), iLineHeight);
}

void fp_RDFAnchorRun::_setWidth(UT_sint32 iWidth)
{
  UT_DEBUGMSG(("Annotation width set to %d \n",iWidth));
  fp_Run::_setWidth(iWidth);
}

void fp_RDFAnchorRun::recalcValue(void)
{
    _recalcWidth();
    if(!displayAnnotations())
    {
        m_iRealWidth = calcWidth();
    }
}

bool fp_RDFAnchorRun::_recalcWidth(void)
{
    if(!displayAnnotations())
    {
        if(getWidth() == 0)
	    return false;
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
	_setWidth(0);
	return true;
    }
    if(!m_bIsStart)
    {
	_setWidth(0);
	return false;
    }
    UT_sint32 iNewWidth = calcWidth();
    m_iRealWidth = iNewWidth;

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

UT_sint32 fp_RDFAnchorRun::calcWidth(void)
{
    UT_sint32 iNewWidth = 0;
    _setValue();
    getGraphics()->setFont(_getFont());
    if(m_sValue.size() > 0)
    {
        iNewWidth = getGraphics()->measureString(m_sValue.ucs4_str().ucs4_str(),
                                                 0,
                                                 m_sValue.ucs4_str().size(),
                                                 NULL);
    }
//    UT_ASSERT(iNewWidth > 0);
    return iNewWidth;
}

const char * fp_RDFAnchorRun::getValue(void)
{
  return m_sValue.utf8_str();
}


bool fp_RDFAnchorRun::canBreakAfter(void) const
{
	return true;
}

bool fp_RDFAnchorRun::canBreakBefore(void) const
{
	return true;
}

bool fp_RDFAnchorRun::_letPointPass(void) const
{
	return true;
}

bool fp_RDFAnchorRun::_canContainPoint(void) const
{
	return false;
}

bool fp_RDFAnchorRun::_setValue(void)
{
  //  UT_uint32 pos = getBlock()->getDocLayout()->getAnnotationVal(getPID()) + 1;

  const PP_AttrProp * pAP = NULL;
  getSpanAP(pAP);
  RDFAnchor a( pAP );
  
  UT_String tmp;
//  if(getBlock()->getDocLayout()->displayRDFAnchors())
//      UT_String_sprintf( tmp, "x%s (%d)", a.getID().c_str(), pos );
//  else
//  UT_String_sprintf( tmp, "" );
  m_sValue = tmp.c_str();
  return true;
}

std::string fp_RDFAnchorRun::getXMLID()
{
    const PP_AttrProp * pAP = NULL;
    getSpanAP(pAP);
    RDFAnchor a( pAP );
    return a.getID();
}

