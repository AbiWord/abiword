/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_UnixFrame.h"
#include "gr_UnixGraphics.h"
#include "ap_UnixStatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_UnixStatusBar::AP_UnixStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_pG = NULL;
}

AP_UnixStatusBar::~AP_UnixStatusBar(void)
{
	DELETEP(m_pG);
}

void AP_UnixStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	XAP_UnixApp * app = static_cast<XAP_UnixApp *>(m_pFrame->getApp());
	XAP_UnixFontManager * fontManager = app->getFontManager();
	GR_UnixGraphics * pG = new GR_UnixGraphics(m_wStatusBar->window, fontManager);
	m_pG = pG;
	UT_ASSERT(m_pG);

	GtkStyle * style = gtk_widget_get_style((static_cast<XAP_UnixFrame *> (m_pFrame))->getTopLevelWindow());
	UT_ASSERT(style);
	pG->init3dColors(style);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
}

GtkWidget * AP_UnixStatusBar::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wStatusBar);
	
	m_wStatusBar = gtk_drawing_area_new();

	gtk_object_set_user_data(GTK_OBJECT(m_wStatusBar),this);
	gtk_widget_show(m_wStatusBar);
	gtk_widget_set_usize(m_wStatusBar, -1, s_iFixedHeight);

	gtk_widget_set_events(GTK_WIDGET(m_wStatusBar), (GDK_EXPOSURE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_wStatusBar), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wStatusBar), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

	return m_wStatusBar;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

gint AP_UnixStatusBar::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_UnixStatusBar * pUnixStatusBar = (AP_UnixStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	pUnixStatusBar->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pUnixStatusBar->getWidth())
		pUnixStatusBar->setWidth(iWidth);
	
	return 1;
}
	
gint AP_UnixStatusBar::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_UnixStatusBar * pUnixStatusBar = (AP_UnixStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("UnixStatusBar: [p %p] received delete_event\n",pUnixStatusBar));
	return 1;
}
	
gint AP_UnixStatusBar::_fe::expose(GtkWidget * w, GdkEventExpose * /*pExposeEvent*/)
{
	// a static function
	AP_UnixStatusBar * pUnixStatusBar = (AP_UnixStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pUnixStatusBar)
		return 0;

	pUnixStatusBar->draw();
	return 0;
}

void AP_UnixStatusBar::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
