/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"
#include "gr_CocoaGraphics.h"
#include "ap_CocoaStatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_CocoaStatusBar::AP_CocoaStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = nil;
	m_pG = NULL;

	/* fetch the widget from the controller */
	m_wStatusBar = [static_cast<XAP_CocoaFrame *>(m_pFrame)->_getController() getStatusBar];
}

AP_CocoaStatusBar::~AP_CocoaStatusBar(void)
{
	DELETEP(m_pG);
}

#if 0
void AP_CocoaStatusBar::_style_changed(void)
{
	setView(m_pView);
}
#endif

void AP_CocoaStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	GR_CocoaGraphics * pG = new GR_CocoaGraphics(m_wStatusBar, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);
	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);
	
#if 0
	GtkStyle * style = gtk_widget_get_style((static_cast<XAP_CocoaFrame *> (m_pFrame))->getTopLevelWindow());
	UT_ASSERT(style);
	pG->init3dColors(style);
#endif

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
}

XAP_CocoaNSView * AP_CocoaStatusBar::createWidget(void)
{
	// TODO remove that method. uneeded.
	return m_wStatusBar;
}


bool AP_CocoaStatusBar::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pGR, void *param)
{
	// a static function
	UT_DEBUGMSG (("AP_CocoaStatusBar::_graphicsUpdateCB()\n"));
	AP_CocoaStatusBar * pCocoaStatusBar = (AP_CocoaStatusBar *)param;
	if (!pCocoaStatusBar)
		return false;

	pCocoaStatusBar->draw();
	return true;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
#if 0
gint AP_CocoaStatusBar::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_CocoaStatusBar * pCocoaStatusBar = (AP_CocoaStatusBar *)g_object_get_user_data(G_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	pCocoaStatusBar->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pCocoaStatusBar->getWidth())
		pCocoaStatusBar->setWidth(iWidth);
	
	return 1;
}
#endif

void AP_CocoaStatusBar::show(void)
{
	if ([m_wStatusBar superview] == nil) {
		[m_superView addSubview:m_wStatusBar];
		UT_ASSERT ([m_wStatusBar retainCount] > 1);
		[m_wStatusBar autorelease];		// at this time it should have already been retained.
	}
}

void AP_CocoaStatusBar::hide(void)
{
	if ([m_wStatusBar superview] != nil) {
		m_superView = [m_wStatusBar superview];
		UT_ASSERT (m_superView);
		[m_wStatusBar retain];
		[m_wStatusBar removeFromSuperview];
	}
	// TODO Check about resizing / layout changes
}
