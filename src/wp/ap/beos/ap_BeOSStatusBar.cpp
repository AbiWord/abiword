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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_BeOSFrame.h"
#include "gr_BeOSGraphics.h"
#include "ap_BeOSStatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_BeOSStatusBar::AP_BeOSStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
//	m_wStatusBar = NULL;
	m_pG = NULL;
				 //	R, G, B
	UT_setColor(m_clrForeground, 255, 0, 0);
	UT_setColor(m_clrBackground, 0, 0, 255);
}

AP_BeOSStatusBar::~AP_BeOSStatusBar(void)
{
	DELETEP(m_pG);
}

void AP_BeOSStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.
#if 0
	DELETEP(m_pG);	
	XAP_BeOSApp * app = static_cast<XAP_BeOSApp *>(m_pFrame->getApp());
	XAP_BeOSFontManager * fontManager = app->getFontManager();
	m_pG = new GR_BeOSGraphics(m_wStatusBar->window, fontManager);
	UT_ASSERT(m_pG);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
#endif
}

#if 0
GtkWidget * AP_BeOSStatusBar::createWidget(void)
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

gint AP_BeOSStatusBar::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_BeOSStatusBar * pBeOSStatusBar = (AP_BeOSStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	pBeOSStatusBar->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pBeOSStatusBar->getWidth())
		pBeOSStatusBar->setWidth(iWidth);
	
	return 1;
}
	
gint AP_BeOSStatusBar::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_BeOSStatusBar * pBeOSStatusBar = (AP_BeOSStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("BeOSStatusBar: [p %p] received delete_event\n",pBeOSStatusBar));
	return 1;
}
	
gint AP_BeOSStatusBar::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_BeOSStatusBar * pBeOSStatusBar = (AP_BeOSStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pBeOSStatusBar)
		return 0;

	pBeOSStatusBar->draw();
	return 0;
}

void AP_BeOSStatusBar::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}

#endif
