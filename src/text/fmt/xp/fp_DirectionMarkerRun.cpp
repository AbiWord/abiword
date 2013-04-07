/* AbiWord
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "fp_DirectionMarkerRun.h"
#include "fl_BlockLayout.h"
#include "fp_TextRun.h"
#include "fp_Line.h"
#include "fv_View.h"

#include "gr_DrawArgs.h"
#include "gr_Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "gr_Painter.h"

fp_DirectionMarkerRun::fp_DirectionMarkerRun(fl_BlockLayout* pBL,
					  UT_uint32 iOffsetFirst,
					  UT_UCS4Char cMarker):
		fp_Run(pBL, iOffsetFirst, 1, FPRUN_DIRECTIONMARKER)
{
	m_iMarker = cMarker;
	
	_setDirty(true);
	_setDirection(UT_bidiGetCharType(m_iMarker));
	lookupProperties();
}


bool fp_DirectionMarkerRun::_recalcWidth(void)
{
	UT_sint32 iOldWidth = getWidth();
	FV_View* pView = _getView();

	if (pView && pView->getShowPara())
	{
		if(static_cast<UT_sint32>(m_iDrawWidth) != iOldWidth)
		{
			_setWidth(m_iDrawWidth);
			return true;
		}
		
		xxx_UT_DEBUGMSG(("fp_DirectionMarkerRun::lookupProperties: width %d\n", getWidth()));
	}
	else
	{
		if(iOldWidth != 0)
		{
			_setWidth(0);
			return true;
		}
	}
	
	return false;
}

void fp_DirectionMarkerRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG)
{
	//UT_DEBUGMSG(("fp_DirectionMarkerRun::lookupProperties\n"));
	_inheritProperties();
	if(pG == NULL)
	{
		pG = getGraphics();
	}
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

	// Find drawing width
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

		const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);
		pG->setFont(pFont);
	}

	UT_UCS4Char cM = m_iMarker == UCS_LRM ? (UT_UCS4Char)'>' : (UT_UCS4Char)'<';
	m_iDrawWidth  = pG->measureString(&cM, 0, 1, NULL);
	xxx_UT_DEBUGMSG(("fp_DirectionMarkerRun::lookupProperties: width %d\n", getWidth()));
}

bool fp_DirectionMarkerRun::canBreakAfter(void) const
{
	return true;
}

bool fp_DirectionMarkerRun::canBreakBefore(void) const
{
	return true;
}

bool fp_DirectionMarkerRun::_letPointPass(void) const
{
	return false;
}

bool fp_DirectionMarkerRun::_deleteFollowingIfAtInsPoint() const
{
	// we will only allow deletion if visible on screen
	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return true;
    }

	return false;
}

void fp_DirectionMarkerRun::mapXYToPosition(UT_sint32 x,
											UT_sint32 /*y*/,
											PT_DocPosition& pos,
											bool& bBOL,
											bool& bEOL,
											bool & /*isTOC*/)
{
	if (x > getWidth())
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();
	else
		pos = getBlock()->getPosition() + getBlockOffset();

	bBOL = false;
	bEOL = false;
}

void fp_DirectionMarkerRun::findPointCoords(UT_uint32 iOffset,
										   UT_sint32& x, UT_sint32& y,
										   UT_sint32& x2, UT_sint32& y2,
										   UT_sint32& height,
										   bool& bDirection)
{
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
	bDirection = (getVisDirection() != UT_BIDI_LTR);
}

void fp_DirectionMarkerRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_return_if_fail(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

	GR_Painter painter(getGraphics());

	if(getWidth())
	{
		UT_sint32 xoff = 0, yoff = 0;
		getLine()->getScreenOffsets(this, xoff, yoff);

		if(getVisDirection() == UT_BIDI_RTL)
		{
			xoff -= m_iDrawWidth;
		}
		painter.fillRect(_getColorPG(), xoff, yoff+1, m_iDrawWidth, getLine()->getHeight()+1);
	}
}

/*!
  Draw Direction marker graphical representation
  \param pDA Draw arguments
  Draws the < or > character in show paragraphs mode.
*/
void fp_DirectionMarkerRun::_draw(dg_DrawArgs* pDA)
{
	// if showPara is turned off we will not draw anything at all; however,
	// we will ensure that the width is set to 0, and if it is currently not
	// we will get our line to redo its layout and redraw.
	FV_View* pView = _getView();
    if(!pView || !pView->getShowPara())
    {
    	return;
    }

	GR_Painter painter(getGraphics());

	UT_ASSERT_HARMLESS(pDA->pG == getGraphics());

	UT_uint32 iRunBase = getBlock()->getPosition() + getBlockOffset();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT_HARMLESS(iSel1 <= iSel2);

	bool bIsSelected = false;
	if (/* pView->getFocus()!=AV_FOCUS_NONE && */	(iSel1 <= iRunBase) && (iSel2 > iRunBase))
		bIsSelected = true;

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

		const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);
		getGraphics()->setFont(pFont);
		iAscent = getGraphics()->getFontAscent();
	}

	// if we currently have a 0 width, i.e., we draw in response to the
	// showPara being turned on, then we obtain the new width, and then
	// tell the line to redo its layout and redraw instead of drawing ourselves
	UT_UCS4Char cM = m_iMarker == UCS_LRM ? (UT_UCS4Char)'>' : (UT_UCS4Char)'<';
	m_iDrawWidth  = getGraphics()->measureString(&cM, 0, 1, NULL);

	_setHeight(getGraphics()->getFontHeight());
	m_iXoffText = pDA->xoff;

	m_iYoffText = pDA->yoff - iAscent;
	xxx_UT_DEBUGMSG(("fp_DirectionMarkerRun::draw: width %d\n", m_iDrawWidth));

	if (bIsSelected)
	{
		painter.fillRect(_getView()->getColorSelBackground(),
						  m_iXoffText,
						  m_iYoffText,
						  m_iDrawWidth,
						  getLine()->getHeight());
	}
	else
	{
		painter.fillRect(_getColorPG(),
						  m_iXoffText,
						  m_iYoffText,
						  m_iDrawWidth,
						  getLine()->getHeight());
	}
	if (pView->getShowPara())
	{
		// Draw symbol
		// use the hard-coded colour only if not revised
		if(!getRevisions())
			getGraphics()->setColor(pView->getColorShowPara());
        painter.drawChars(&cM, 0, 1, m_iXoffText, m_iYoffText);
	}
}
