/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#if 0
// evil ugly hack
static int style_changed (GtkWidget * w, GdkEventClient * event,
						  AP_CocoaStatusBar * bar)
{
	static GdkAtom atom_rcfiles = GDK_NONE;
	g_return_val_if_fail (w != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	if (!atom_rcfiles)
		atom_rcfiles = gdk_atom_intern ("_GTK_READ_RCFILES", FALSE);
	if (event->message_type != atom_rcfiles)
		return FALSE;
	bar->_style_changed();
	return FALSE;
}
#endif

AP_CocoaStatusBar::AP_CocoaStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = nil;
	m_pG = NULL;

#if 0
	GtkWidget * toplevel = (static_cast<XAP_CocoaFrame *> (m_pFrame))->getTopLevelWindow();
	gtk_signal_connect_after (GTK_OBJECT(toplevel),
							  "client_event",
							  GTK_SIGNAL_FUNC(style_changed),
							  (gpointer)this);
#endif
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
	XAP_CocoaApp * app = static_cast<XAP_CocoaApp *>(m_pFrame->getApp());
	XAP_CocoaFontManager * fontManager = app->getFontManager();
	GR_CocoaGraphics * pG = new GR_CocoaGraphics(m_wStatusBar, fontManager, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

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

NSControl * AP_CocoaStatusBar::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wStatusBar);
	
	m_wStatusBar = [static_cast<XAP_CocoaFrame *>(m_pFrame)->_getController() getStatusBar];
	
	return m_wStatusBar;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
#if 0
gint AP_CocoaStatusBar::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_CocoaStatusBar * pCocoaStatusBar = (AP_CocoaStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	pCocoaStatusBar->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pCocoaStatusBar->getWidth())
		pCocoaStatusBar->setWidth(iWidth);
	
	return 1;
}
	
gint AP_CocoaStatusBar::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_CocoaStatusBar * pCocoaStatusBar = (AP_CocoaStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("CocoaStatusBar: [p %p] received delete_event\n",pCocoaStatusBar));
	return 1;
}
	
gint AP_CocoaStatusBar::_fe::expose(GtkWidget * w, GdkEventExpose * /*pExposeEvent*/)
{
	// a static function
	AP_CocoaStatusBar * pCocoaStatusBar = (AP_CocoaStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pCocoaStatusBar)
		return 0;

	pCocoaStatusBar->draw();
	return 0;
}

void AP_CocoaStatusBar::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
#endif

void AP_CocoaStatusBar::show(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	gtk_widget_show (m_wStatusBar);
}

void AP_CocoaStatusBar::hide(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	gtk_widget_hide (m_wStatusBar);
//	m_pFrame->queue_resize();
}
