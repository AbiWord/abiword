/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2004, Martin Sevior
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


#include "fp_MathRun.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include "ut_mbtowc.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "gr_DrawArgs.h"
#include "gr_Abi_MathGraphicDevice.h"
#include "gr_Abi_RenderingContext.h"

#include <MathView/BoundingBox.hh>
#include <MathView/MathMLNamespaceContext.hh>
#include <MathView/NamespaceContext.hh>
#include <MathView/MathMLElement.hh>

fp_MathRun::fp_MathRun(fl_BlockLayout* pBL, 
					   UT_uint32 iOffsetFirst)	: 
	fp_Run(pBL,  iOffsetFirst,1, FPRUN_MATH ),
    m_iImageWidth(0),
	m_iImageHeight(0),
	m_sCachedWidthProp(""),
	m_sCachedHeightProp(""),
	m_iPointHeight(0),
	m_pSpanAP(NULL),
	m_iGraphicTick(0),
	m_pszDataID(NULL),
	m_sMathML("")
{
	m_pMathView = libxml2_MathView::create();
	m_pMathView->setMathMLNamespaceContext(
		MathMLNamespaceContext::create(m_pMathView,getMathDevice()));
	UT_DEBUGMSG(("fp_MathRun Created! \n"));
	lookupProperties(getGraphics());
}

fp_MathRun::~fp_MathRun(void)
{
}

SmartPtr<GR_Abi_MathGraphicDevice> fp_MathRun::getMathDevice(void)
{
	return getBlock()->getDocLayout()->getMathGraphicDevice();
}

GR_Abi_RenderingContext *  fp_MathRun::getAbiContext(void)
{
	return getBlock()->getDocLayout()->getAbiContext();
}

void fp_MathRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * /*pBlockAP*/,
									const PP_AttrProp * /*pSectionAP*/,
									GR_Graphics * pG)
{
	UT_DEBUGMSG(("fp_MathRun _lookupProperties span %x \n",pSpanAP));
	m_pSpanAP = pSpanAP;
	bool bFoundDataID = pSpanAP->getAttribute("dataid", m_pszDataID);
	if (bFoundDataID && m_pszDataID)
	{
		const UT_ByteBuf * pByteBuf = NULL;
		PD_Document * pDoc = getBlock()->getDocument();
		bFoundDataID = pDoc->getDataItemDataByName(const_cast<char*>(m_pszDataID), 
																 const_cast<const UT_ByteBuf **>(&pByteBuf),
																 NULL, NULL);

		UT_UCS4_mbtowc myWC;
		m_sMathML.appendBuf( *pByteBuf, myWC);
	}
	UT_return_if_fail(bFoundDataID);
	UT_return_if_fail(m_pszDataID);
	UT_DEBUGMSG(("MATH ML string is... \n %s \n",m_sMathML.utf8_str()));

// Load this into MathView

	m_pMathView->loadBuffer(m_sMathML.utf8_str());

	BoundingBox box = m_pMathView->getBoundingBox();
	UT_sint32 iWidth = getAbiContext()->toAbiLayoutUnits(box.width);
	UT_sint32 iAscent = getAbiContext()->toAbiLayoutUnits(box.height);
	UT_sint32 iDescent = getAbiContext()->toAbiLayoutUnits(box.depth);

	UT_DEBUGMSG(("Width = %d Ascent = %d Descent = %d \n",iWidth,iAscent,iDescent)); 
	const XML_Char * szWidth = NULL;
	m_pSpanAP->getProperty("width", szWidth);
	if(szWidth == NULL)
	{
		szWidth = "0in";
	}
	const XML_Char * szHeight = NULL;
	pSpanAP->getProperty("height", szHeight);
	if(pG == NULL)
	{
		pG = getGraphics();
	}
	if(szHeight == NULL)
	{
		szHeight = "0in";
	}

	// Also get max width, height ready for generateImage.

	fl_DocSectionLayout * pDSL = getBlock()->getDocSectionLayout();
	fp_Page * p = NULL;
	if(pDSL->getFirstContainer())
	{
		p = pDSL->getFirstContainer()->getPage();
	}
	else
	{
		p = pDSL->getDocLayout()->getNthPage(0);
	}
	UT_sint32 maxW = p->getWidth() - UT_convertToLogicalUnits("0.1in"); 
	UT_sint32 maxH = p->getHeight() - UT_convertToLogicalUnits("0.1in");
	maxW -= pDSL->getLeftMargin() + pDSL->getRightMargin();
	maxH -= pDSL->getTopMargin() + pDSL->getBottomMargin();

	if((strcmp(m_sCachedWidthProp.c_str(),szWidth) != 0) ||
	   (strcmp(m_sCachedHeightProp.c_str(),szHeight) != 0) ||
		UT_convertToLogicalUnits(szHeight) > maxH ||
		UT_convertToLogicalUnits(szWidth) > maxW)
	{
		m_sCachedWidthProp = szWidth;
		m_sCachedHeightProp = szHeight;
	}
	markAsDirty();
	if(getLine())
	{
		getLine()->setNeedsRedraw();
	}
	m_iImageWidth = getWidth();
	m_iImageHeight = getHeight();

	_setAscent(iAscent);
	_setDescent(iDescent);
	_setWidth(iWidth);
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getBlockAP(pBlockAP);

	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	GR_Font * pFont = const_cast<GR_Font *>(pLayout->findFont(pSpanAP,pBlockAP,pSectionAP));

	if (pFont != _getFont())
	{
		_setFont(pFont);
	}
	m_iPointHeight = pG->getFontAscent(pFont) + getGraphics()->getFontDescent(pFont);
}


void fp_MathRun::_drawResizeBox(UT_Rect box)
{
	GR_Graphics * pG = getGraphics();
	UT_sint32 left = box.left;
	UT_sint32 top = box.top;
	UT_sint32 right = box.left + box.width - pG->tlu(1);
	UT_sint32 bottom = box.top + box.height - pG->tlu(1);

	GR_Painter painter(pG);
	
	pG->setLineProperties(pG->tluD(1.0),
								 GR_Graphics::JOIN_MITER,
								 GR_Graphics::CAP_BUTT,
								 GR_Graphics::LINE_SOLID);	
	
	// draw some really fancy box here
	pG->setColor(UT_RGBColor(98,129,131));
	painter.drawLine(left, top, right, top);
	painter.drawLine(left, top, left, bottom);
	pG->setColor(UT_RGBColor(230,234,238));
	painter.drawLine(box.left+pG->tlu(1), box.top + pG->tlu(1), right - pG->tlu(1), top+pG->tlu(1));
	painter.drawLine(box.left+pG->tlu(1), box.top + pG->tlu(1), left + pG->tlu(1), bottom - pG->tlu(1));
	pG->setColor(UT_RGBColor(98,129,131));
	painter.drawLine(right - pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));
	painter.drawLine(left + pG->tlu(1), bottom - pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));
	pG->setColor(UT_RGBColor(49,85,82));
	painter.drawLine(right, top, right, bottom);
	painter.drawLine(left, bottom, right, bottom);
	painter.fillRect(UT_RGBColor(156,178,180),box.left + pG->tlu(2), box.top + pG->tlu(2), box.width - pG->tlu(4), box.height - pG->tlu(4));
}

bool fp_MathRun::canBreakAfter(void) const
{
	return true;
}

bool fp_MathRun::canBreakBefore(void) const
{
	return true;
}

bool fp_MathRun::_letPointPass(void) const
{
	return false;
}

bool fp_MathRun::hasLayoutProperties(void) const
{
	return true;
}

bool fp_MathRun::isSuperscript(void) const
{
	return false;
}


bool fp_MathRun::isSubscript(void) const
{
	return false;
}

void fp_MathRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC)
{
	if (x > getWidth())
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();
	else
		pos = getBlock()->getPosition() + getBlockOffset();

	bBOL = false;
	bEOL = false;
}

void fp_MathRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
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

void fp_MathRun::_clearScreen(bool  bFullLineHeightRect )
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

const char * fp_MathRun::getDataID(void) const
{
	return m_pszDataID;
}

void fp_MathRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics *pG = pDA->pG;
	UT_DEBUGMSG(("Draw with class %x \n",pG));
	UT_DEBUGMSG(("Contents of fp MathRun \n %s \n",m_sMathML.utf8_str()));
}


