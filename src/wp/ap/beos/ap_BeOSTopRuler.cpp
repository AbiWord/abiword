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
#include "ap_BeOSTopRuler.h"
#include "gr_BeOSGraphics.h"

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

AP_BeOSTopRuler::AP_BeOSTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_wTopRuler = NULL;
}

AP_BeOSTopRuler::~AP_BeOSTopRuler(void)
{
}

/*
GtkWidget * AP_BeOSTopRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wTopRuler);
	
	m_wTopRuler = gtk_drawing_area_new();

	gtk_object_set_user_data(GTK_OBJECT(m_wTopRuler),this);
	gtk_widget_show(m_wTopRuler);
	gtk_widget_set_usize(m_wTopRuler, -1, s_iFixedHeight);

	gtk_widget_set_events(GTK_WIDGET(m_wTopRuler), (GDK_EXPOSURE_MASK |
													GDK_BUTTON_PRESS_MASK |
													GDK_POINTER_MOTION_MASK |
													GDK_BUTTON_RELEASE_MASK |
													GDK_KEY_PRESS_MASK |
													GDK_KEY_RELEASE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_wTopRuler), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wTopRuler), "button_press_event",
					   GTK_SIGNAL_FUNC(_fe::button_press_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wTopRuler), "button_release_event",
					   GTK_SIGNAL_FUNC(_fe::button_release_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wTopRuler), "motion_notify_event",
					   GTK_SIGNAL_FUNC(_fe::motion_notify_event), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wTopRuler), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

	return m_wTopRuler;
}
*/

void AP_BeOSTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

/*
	DELETEP(m_pG);	
	AP_BeOSApp * app = static_cast<AP_BeOSApp *>(m_pFrame->getApp());
	AP_BeOSFontManager * fontManager = app->getFontManager();
	m_pG = new BEOSGraphics(m_wTopRuler->window, fontManager);
	UT_ASSERT(m_pG);
*/
}

/*****************************************************************/
#if 0
gint AP_BeOSTopRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("BeOSTopRuler: [p %p] received button_press_event\n",pBeOSTopRuler));
	return 1;
}

gint AP_BeOSTopRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("BeOSTopRuler: [p %p] received button_release_event\n",pBeOSTopRuler));
	return 1;
}
	
gint AP_BeOSTopRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	// UT_DEBUGMSG(("BeOSTopRuler: [p %p] [size w %d h %d] received configure_event\n",
	//				 pBeOSTopRuler, e->width, e->height));

	UT_uint32 iHeight = (UT_uint32)e->height;
	if (iHeight != pBeOSTopRuler->getHeight())
		pBeOSTopRuler->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pBeOSTopRuler->getWidth())
		pBeOSTopRuler->setWidth(iWidth);
	
	return 1;
}
	
gint AP_BeOSTopRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	// AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("BeOSTopRuler: [p %p] received motion_notify_event\n",pBeOSTopRuler));
	return 1;
}
	
gint AP_BeOSTopRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	// a static function
	AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("BeOSTopRuler: [p %p] received key_press_event\n",pBeOSTopRuler));
	return 1;
}
	
gint AP_BeOSTopRuler::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("BeOSTopRuler: [p %p] received delete_event\n",pBeOSTopRuler));
	return 1;
}
	
gint AP_BeOSTopRuler::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_BeOSTopRuler * pBeOSTopRuler = (AP_BeOSTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pBeOSTopRuler)
		return 0;

	UT_Rect rClip;
	rClip.left = pExposeEvent->area.x;
	rClip.top = pExposeEvent->area.y;
	rClip.width = pExposeEvent->area.width;
	rClip.height = pExposeEvent->area.height;

	pBeOSTopRuler->draw(&rClip);
	return 0;
}

void AP_BeOSTopRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
#endif
