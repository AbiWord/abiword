/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301 USA.
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
	m_Offset(0),
	m_clrBackground(255, 247, 177),
	m_sTitle("n/a"),
	m_sAuthor("n/a"),
	m_sDescription("n/a")
{
	m_gc = NULL;
}

AP_Preview_Annotation::~AP_Preview_Annotation()
{
}

// Setters and getters
void AP_Preview_Annotation::setTitle(const gchar * pTitle)
{
	UT_return_if_fail(pTitle);
	m_sTitle = pTitle;
}

void AP_Preview_Annotation::setAuthor(const gchar * pAuthor)
{
	UT_return_if_fail(pAuthor);
	m_sAuthor = pAuthor;
}

void AP_Preview_Annotation::setDescription(const gchar * pDescription)
{
	UT_return_if_fail(pDescription);
	m_sDescription = pDescription;
}

void AP_Preview_Annotation::setAnnotationID(UT_uint32 aID)
{
	m_iAID = aID;
}

void AP_Preview_Annotation::_createAnnotationPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height)
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
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	GR_Graphics * pG = NULL;
	UT_return_if_fail(pView);
	pG = pView->getGraphics();

	UT_return_if_fail(pG);
	GR_Font * pFont = pG->findFont("Times New Roman", "normal",
				       "normal", "normal",
				       "normal", "12pt",
				       NULL);
	UT_return_if_fail(pFont);
	
	double rat = 100./static_cast<double>(pG->getZoomPercentage());
	UT_sint32 iHeight = pG->getFontAscent(pFont) + pG->tlu(7);
	iHeight = static_cast<UT_sint32>(static_cast<double>(iHeight));
	m_drawString = m_sDescription;
	UT_sint32 len = m_drawString.size();
	pG->setFont(pFont);
	UT_sint32 iwidth = pG->measureString(m_drawString.ucs4_str(),0,len,NULL) + pG->tlu(6);
	iwidth = static_cast<UT_sint32>(static_cast<double>(iwidth));
	m_width = static_cast<UT_sint32>(static_cast<double>(pG->tdu(iwidth))*rat);
	m_height = static_cast<UT_sint32>(static_cast<double>(pG->tdu(iHeight))*rat);
	if(pG->tdu(pView->getWindowWidth()) < m_width)
		m_width = pG->tdu(pView->getWindowWidth());
	UT_DEBUGMSG(("SetSize from Annotation width %d rat %f \n",m_width,rat));
}

void AP_Preview_Annotation::drawImmediate(const UT_Rect* clip)
{
	UT_UNUSED(clip);
	m_drawString = m_sDescription;
	UT_return_if_fail(m_gc);
	UT_RGBColor FGcolor(0,0,0);
	UT_RGBColor BGcolor(m_clrBackground);
	
	m_pFont = m_gc->findFont("Times New Roman", "normal",
							 "normal", "normal",
							 "normal", "12pt",
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
	
	clearScreen();

	//
	// Calculate the draw coordinates position
	//
	UT_sint32 iTop = m_gc->tlu(1);
	UT_sint32 len = m_drawString.size();
	UT_sint32 iLeft = m_gc->tlu(2);

	//
	// Fill the background color
	//
	GR_Painter painter(m_gc);
	
	//
	// Do the draw chars at last!
	//
	m_gc->setColor(FGcolor);
	painter.drawChars(m_drawString.ucs4_str(), 0, len, iLeft, iTop);
	
	// bad hardcoded color, but this will probably [ <-this assumption is the bad thing :) ] never be different anyway
	m_gc->setColor(UT_RGBColor(0,0,0));
	painter.drawLine(0, 0, m_gc->tlu(getWindowWidth()), 0);
	painter.drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), 0, m_gc->tlu(getWindowWidth()) - m_gc->tlu(1),
					 m_gc->tlu(getWindowHeight()));
	painter.drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0,
					 m_gc->tlu(getWindowHeight()) - m_gc->tlu(1));
	painter.drawLine(0, m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0, 0);
}

void AP_Preview_Annotation::clearScreen(void)
{
	UT_sint32 iWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu(getWindowHeight());
	
	GR_Painter painter(m_gc);
	
	// clear the whole drawing area, except for the border
	painter.fillRect(m_clrBackground, 0 + m_gc->tlu(1), 0 + m_gc->tlu(1), iWidth - m_gc->tlu(2), iHeight - m_gc->tlu(2));
}

void AP_Preview_Annotation::setActiveFrame(XAP_Frame *pFrame)
{
	UT_DEBUGMSG(("AP_Preview_Annotation: setActiveFrame\n"));
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
	UT_DEBUGMSG(("AP_Preview_Annotation: setXY top %d, left %d\n", m_top, m_left));
}

/*!
 * In Gtk Apps it is really hard to translate the position of the 
 * popup window to the actual position of the run. In insead use the
 * location of the mouse when it crosses the run.
 * If induces a vertical uncertainty in here the popup appears.
 * Setting this offset corrects for this.
 */
void AP_Preview_Annotation::setOffset(UT_sint32 offset)
{
	m_Offset = offset;
}
