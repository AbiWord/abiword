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
#include "xap_QNXFrame.h"
#include "gr_QNXGraphics.h"
#include "ap_QNXStatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_QNXStatusBar::AP_QNXStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_pG = NULL;
}

AP_QNXStatusBar::~AP_QNXStatusBar(void)
{
	DELETEP(m_pG);
}

void AP_QNXStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.
#if 0
	DELETEP(m_pG);	
	XAP_QNXApp * app = static_cast<XAP_QNXApp *>(m_pFrame->getApp());
	XAP_QNXFontManager * fontManager = app->getFontManager();
	GR_QNXGraphics * pG = new GR_QNXGraphics(m_wStatusBar->window, fontManager);
	m_pG = pG;
	UT_ASSERT(m_pG);

	GtkStyle * style = gtk_widget_get_style((static_cast<XAP_QNXFrame *> (m_pFrame))->getTopLevelWindow());
	UT_ASSERT(style);
	pG->init3dColors(style);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
#endif
}

void * AP_QNXStatusBar::createWidget(void)
{
#if 0
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
#endif 
	return(NULL);
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
#if 0
int AP_QNXStatusBar::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_QNXStatusBar * pQNXStatusBar = (AP_QNXStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	pQNXStatusBar->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pQNXStatusBar->getWidth())
		pQNXStatusBar->setWidth(iWidth);
	
	return 1;
}
	
int AP_QNXStatusBar::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_QNXStatusBar * pQNXStatusBar = (AP_QNXStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("QNXStatusBar: [p %p] received delete_event\n",pQNXStatusBar));
	return 1;
}
	
int AP_QNXStatusBar::_fe::expose(GtkWidget * w, GdkEventExpose * /*pExposeEvent*/)
{
	// a static function
	AP_QNXStatusBar * pQNXStatusBar = (AP_QNXStatusBar *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pQNXStatusBar)
		return 0;

	pQNXStatusBar->draw();
	return 0;
}

void AP_QNXStatusBar::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
#endif
