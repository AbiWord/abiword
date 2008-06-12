/* AbiSource Application Framework
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "gr_Painter.h"
#include "ap_Preview_Annotation.h"
#include "fv_View.h"
#include "xap_Frame.h"

// RIVERA
AP_Preview_Annotation::AP_Preview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id),
	m_width(PREVIEW_DEFAULT_WIDTH),
	m_height(PREVIEW_DEFAULT_HEIGHT),
	m_left(0),
	m_top(0),
	m_clrBackground(255, 247, 177),
	m_pTitle("n/a"),
	m_pAuthor("n/a"),
	m_pDescription("n/a")
{
}

AP_Preview_Annotation::~AP_Preview_Annotation()
{
}

// Setters and getters
void AP_Preview_Annotation::setTitle(const gchar * pTitle)
{
	UT_return_if_fail(pTitle);
	m_pTitle = pTitle;
}
void AP_Preview_Annotation::setAuthor(const gchar * pAuthor)
{
	UT_return_if_fail(pAuthor);
	m_pAuthor = pAuthor;
}
void AP_Preview_Annotation::setDescription(const gchar * pDescription)
{
	UT_return_if_fail(pDescription);
	m_pDescription = pDescription;
}
void AP_Preview_Annotation::setAnnotationID(UT_uint32 aID)
{
	m_iAID = aID;
}
UT_uint32 AP_Preview_Annotation::getAnnotationID()
{
	return m_iAID;
}

void AP_Preview_Annotation::_createAnnotationPreviewFromGC(GR_ScreenGraphics * gc, UT_uint32 width, UT_uint32 height)
{
	UT_ASSERT(gc);
	m_gc = gc;
	setWindowSize(width, height);
	m_width = gc->tlu(width);
	m_height = gc->tlu(height);
	UT_DEBUGMSG(("AP_Preview_Annotation: Annotation preview created!\n"));
}

/*!
 * This method sets the height and width of the preview from
 * the size of the comment in the annotation.
 */
void AP_Preview_Annotation::setSizeFromAnnotation(void)
{
	//
	// Get the font and features
	//
	
	// "font-family"
	const char * pszFamily = "Times New Roman";
	
	// "font-style"
	const char * pszStyle = "normal";
	
	// "font-variant"
	const char * pszVariant = "normal";
	
	// "font-stretch"
	const char * pszStretch = "normal";
	
	// "font-size"
	const char * pszSize="12pt";
	
	// "font-weight"
	const char * pszWeight = "normal";

	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	UT_return_if_fail(pView);
	GR_ScreenGraphics * pVGraphics = dynamic_cast<GR_ScreenGraphics *>(pView->getGraphics());
	UT_return_if_fail(pVGraphics);
	GR_Font * pFont = pVGraphics->findFont(pszFamily, pszStyle,
							 pszVariant, pszWeight,
							 pszStretch, pszSize,
							 NULL);
	
	UT_return_if_fail(pFont);
	
	UT_sint32 iHeight = pVGraphics->getFontAscent(pFont) + pVGraphics->tlu(7);
	double rat = 100./static_cast<double>(pVGraphics->getZoomPercentage());
	iHeight = static_cast<UT_sint32>(static_cast<double>(iHeight));
	m_drawString = m_pDescription;
	UT_sint32 len = m_drawString.size();
	UT_sint32 iwidth = pVGraphics->measureString(m_drawString.ucs4_str(),0,len,NULL) + pVGraphics->tlu(6);
	iwidth = static_cast<UT_sint32>(static_cast<double>(iwidth));
	m_width = static_cast<UT_sint32>(static_cast<double>(pVGraphics->tdu(iwidth))*rat);
	m_height = static_cast<UT_sint32>(static_cast<double>(pVGraphics->tdu(iHeight))*rat);
	if(pVGraphics->tdu(pView->getWindowWidth()) < m_width)
	  m_width = pVGraphics->tdu(pView->getWindowWidth());
}
// Finally draw the characters in the preview.
void AP_Preview_Annotation::draw(void)
{
        m_drawString = m_pDescription;
	//
	// Text decorations.
	//
	bool isUnder,isOver,isStrike;
	
	isUnder = false;	// "underline"
	isOver = false;		// "overline"
	isStrike = false;	// "line-through"
	
	//
	// Foreground and background colors.
	//
	UT_RGBColor FGcolor(0,0,0);
	UT_RGBColor BGcolor(m_clrBackground);
	
	//
	// Get the font and features
	//
	
	// "font-family"
	const char * pszFamily = "Times New Roman";
	
	// "font-style"
	const char * pszStyle = "normal";
	
	// "font-variant"
	const char * pszVariant = "normal";
	
	// "font-stretch"
	const char * pszStretch = "normal";
	
	// "font-size"
	const char * pszSize="12pt";
	
	// "font-weight"
	const char * pszWeight = "normal";
	
	m_pFont = m_gc->findFont(pszFamily, pszStyle,
							 pszVariant, pszWeight,
							 pszStretch, pszSize,
							 NULL);
	
	UT_ASSERT_HARMLESS(m_pFont);
	if(!m_pFont)
	{
		clearScreen();
		return;
	}
	
	m_gc->setFont(m_pFont);		
	
	m_iAscent = m_gc->getFontAscent(m_pFont);
	m_iDescent = m_gc->getFontDescent(m_pFont);
	m_iHeight = m_gc->getFontHeight(m_pFont);
	
	//
	// Clear the screen!
	//
	clearScreen();
	//
	// Calculate the draw coordinates position
	//
	UT_sint32 iTop = m_gc->tlu(1);
	UT_sint32 len = m_drawString.size();
	UT_sint32 twidth = m_gc->measureString(m_drawString.ucs4_str(),0,len,NULL);
	UT_sint32 iLeft = m_gc->tlu(2);

	//
	// Fill the background color
	//
	GR_Painter painter(m_gc);
	
	//
	// Do the draw chars at last!
	//
	m_gc->setColor(FGcolor);
	m_gc->drawChars(m_drawString.ucs4_str(), 0, len, iLeft, iTop);
	//
	// Do the decorations
	//
	if(isUnder)
	{
		UT_sint32 iDrop = iTop + m_iAscent + m_iDescent/3;
		m_gc->drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isOver)
	{
		UT_sint32 iDrop = iTop + m_gc->tlu(1) + (UT_MAX(m_gc->tlu(10),m_iAscent) - m_gc->tlu(10))/8;
		m_gc->drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isStrike)
	{
		UT_sint32 iDrop = iTop + m_iAscent * 2 /3;
		m_gc->drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	
	// bad hardcoded color, but this will probably [ <-this assumption is the bad thing :) ] never be different anyway
	m_gc->setColor(UT_RGBColor(0,0,0));
	m_gc->drawLine(0, 0, m_gc->tlu(getWindowWidth()), 0);
	m_gc->drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), 0, m_gc->tlu(getWindowWidth()) - m_gc->tlu(1),
					 m_gc->tlu(getWindowHeight()));
	m_gc->drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0,
					 m_gc->tlu(getWindowHeight()) - m_gc->tlu(1));
	m_gc->drawLine(0, m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0, 0);
	
	// somehow this gets ignored...
	//activate();
}

/*void AP_Preview_Annotation::activate(void)
{
	UT_DEBUGMSG(("AP_Preview_Annotation: no activate implementation\n"));
}*/

/*void AP_Preview_Annotation::_bringToTop(void)
{
	UT_DEBUGMSG(("AP_Preview_Annotation: no _bringToTop\n"));
}*/

void AP_Preview_Annotation::clearScreen(void)
{
	UT_sint32 iWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu(getWindowHeight());
	
	GR_Painter painter(m_gc);
	
	// clear the whole drawing area, except for the border
	// Why i ihave to upcast manually here is beyond me -Rob.
	GR_Graphics *gc = m_gc;
	gc->fillRect(m_clrBackground, 0 + m_gc->tlu(1), 0 + m_gc->tlu(1), iWidth - m_gc->tlu(2), iHeight - m_gc->tlu(2));
}

void AP_Preview_Annotation::setActiveFrame(XAP_Frame *pFrame)
{
	UT_DEBUGMSG(("AP_Preview_Annotation: setActiveFrame\n"));
	//notifyActiveFrame(getActiveFrame());
	notifyActiveFrame(pFrame);
}

/*!
 * Set the left and top positions of the popup. These are the mouse
 * coordinates in device units.c - This is ignored by the unix FE
 */
void  AP_Preview_Annotation::setXY(UT_sint32 x, UT_sint32 y)
{
  m_left = x - m_width/2;
  m_top = y;
  if(m_top < 0)
    m_top = 0;
  if(m_left < 0)
    m_left = 0;
  UT_DEBUGMSG(("AP_Preview_Annotation: setXY top %d, left %d\n",m_top,m_left));
}
