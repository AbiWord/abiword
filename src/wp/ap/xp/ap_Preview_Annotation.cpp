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
/*AP_Preview_Annotation::AP_Preview_Annotation() : XAP_Preview(),
m_pFont(NULL),
m_iAscent(0),
m_iDescent(0),
m_iHeight(0)
{
	UT_setColor(m_clrBackground,255,247,177);
	m_width					= PREVIEW_DEFAULT_WIDTH;
	m_height				= PREVIEW_DEFAULT_HEIGHT;
	m_pTitle				= "n/a";
	m_pAuthor				= "n/a";
	m_pDescription			= "n/a";
}*/

// MARTIN
AP_Preview_Annotation::AP_Preview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id): XAP_Dialog_Modeless(pDlgFactory,id)
{
	UT_setColor(m_clrBackground,255,247,177);
	m_width					= PREVIEW_DEFAULT_WIDTH;
	m_height				= PREVIEW_DEFAULT_HEIGHT;
	m_left = 0;
	m_top = 0;
	m_pTitle				= "n/a";
	m_pAuthor				= "n/a";
	m_pDescription			= "n/a";
}


/*AP_Preview_Annotation::AP_Preview_Annotation(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top)
{
	UT_DEBUGMSG(("AP_Preview_Annotation: WARNING Preview annotation not implemented for this platform\n"));
}*/

AP_Preview_Annotation::~AP_Preview_Annotation()
{
}

// Setters and getters
void AP_Preview_Annotation::setTitle(const gchar * pTitle)
{
	UT_return_if_fail(pTitle);
	m_pTitle = g_strdup(pTitle);
}
void AP_Preview_Annotation::setAuthor(const gchar * pAuthor)
{
	UT_return_if_fail(pAuthor);
	m_pAuthor = g_strdup(pAuthor);
}
void AP_Preview_Annotation::setDescription(const gchar * pDescription)
{
	UT_return_if_fail(pDescription);
	m_pDescription = g_strdup(pDescription);
}
void AP_Preview_Annotation::setAnnotationID(UT_uint32 aID)
{
	m_iAID = aID;
}
UT_uint32 AP_Preview_Annotation::getAnnotationID()
{
	return m_iAID;
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

// Finally draw the characters in the preview.
void AP_Preview_Annotation::draw(void)
{
	UT_UCS4_cloneString_char (&m_drawString,
							  g_strconcat(m_pTitle, " (", m_pAuthor, "):", m_pDescription, NULL)
							  );
	
	//
	// Get text decorations.
	//
	bool isUnder,isOver,isStrike;
	
	/*const gchar * pDecor = getVal("text-decoration");
	if(pDecor)
	{
		isUnder = (NULL != strstr(pDecor,));
		isOver = (NULL != strstr(pDecor,));
		isStrike = (NULL != strstr(pDecor,));
	}
	else
	{*/
		isUnder = false;	// "underline"
		isOver = false;		// "overline"
		isStrike = false;	// "line-through"
	//}
	
	//
	// Do foreground and background colors.
	//
	UT_RGBColor FGcolor(0,0,0);
	/*const char * pszFGColor = getVal("color");
	if(pszFGColor)
		UT_parseColor(getVal("color"),FGcolor);*/
	UT_RGBColor BGcolor(m_clrBackground);
	/*const char * pszBGColor = getVal("bgcolor");
	if(pszBGColor && strcmp(pszBGColor,"transparent") != 0)
		UT_parseColor(getVal("bgcolor"),BGcolor);*/
	//
	// Get the font and bold/italic- ness
	//
	//GR_Font * pFont;
	/*const char* pszFamily	= getVal();
	const char* pszStyle	= getVal();
	const char* pszVariant	= getVal();
	const char* pszWeight	= getVal();
	const char* pszStretch	= getVal();
	const char* pszSize		= getVal();*/
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
	UT_sint32 iWinWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iWinHeight = m_gc->tlu(getWindowHeight());
	UT_sint32 iTop = (iWinHeight - m_iHeight)/2;
	UT_sint32 len = UT_UCS4_strlen(m_drawString);
	UT_sint32 twidth = m_gc->measureString(m_drawString,0,len,NULL);
	UT_sint32 iLeft = (iWinWidth - twidth)/2;
	//
	// Fill the background color
	//
	GR_Painter painter(m_gc);
	
	//if(pszBGColor)
		//MAYBEpainter.fillRect(BGcolor,iLeft,iTop,twidth,m_iHeight);
	//
	// Do the draw chars at last!
	//
	m_gc->setColor(FGcolor);
	painter.drawChars(m_drawString, 0, len, iLeft, iTop);
	
	//
	// Do the decorations
	//
	if(isUnder)
	{
		UT_sint32 iDrop = iTop + m_iAscent + m_iDescent/3;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isOver)
	{
		UT_sint32 iDrop = iTop + m_gc->tlu(1) + (UT_MAX(m_gc->tlu(10),m_iAscent) - m_gc->tlu(10))/8;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isStrike)
	{
		UT_sint32 iDrop = iTop + m_iAscent * 2 /3;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	
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
	notifyActiveFrame(getActiveFrame());
}

/*!
 * Set the left and top positions of the popup. These are the mouse
 * coordinates in device units.
 */
void  AP_Preview_Annotation::setXY(UT_sint32 x, UT_sint32 y)
{
  m_left = x - PREVIEW_DEFAULT_WIDTH/2;
  m_top = y;
  if(m_top < 0)
    m_top = 0;
  if(m_left < 0)
    m_left = 0;
}
