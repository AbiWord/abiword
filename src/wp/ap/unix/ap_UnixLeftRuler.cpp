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

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_UnixFrame.h"
#include "ap_UnixLeftRuler.h"
#include "gr_UnixGraphics.h"

#define DELETEP(p)		do { if (p) delete p; p = NULL; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

AP_UnixLeftRuler::AP_UnixLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_wLeftRuler = NULL;
	m_pG = NULL;
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	DELETEP(m_pG);
}

GtkWidget * AP_UnixLeftRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wLeftRuler);
	
	m_wLeftRuler = gtk_drawing_area_new();

	gtk_object_set_user_data(GTK_OBJECT(m_wLeftRuler),this);
	gtk_widget_show(m_wLeftRuler);
	gtk_widget_set_usize(m_wLeftRuler, s_iFixedWidth, -1);

	gtk_widget_set_events(GTK_WIDGET(m_wLeftRuler), (GDK_EXPOSURE_MASK |
													 GDK_BUTTON_PRESS_MASK |
													 GDK_POINTER_MOTION_MASK |
													 GDK_BUTTON_RELEASE_MASK |
													 GDK_KEY_PRESS_MASK |
													 GDK_KEY_RELEASE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_wLeftRuler), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wLeftRuler), "button_press_event",
					   GTK_SIGNAL_FUNC(_fe::button_press_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wLeftRuler), "button_release_event",
					   GTK_SIGNAL_FUNC(_fe::button_release_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wLeftRuler), "motion_notify_event",
					   GTK_SIGNAL_FUNC(_fe::motion_notify_event), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wLeftRuler), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

	return m_wLeftRuler;
}

void AP_UnixLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wLeftRuler->window)
	// is not created until the frame's top-level window is
	// shown.
	
	DELETEP(m_pG);
	AP_UnixApp * app = static_cast<AP_UnixApp *>(m_pFrame->getApp());
	AP_UnixFontManager * fontManager = app->getFontManager();
	m_pG = new UNIXGraphics(m_wLeftRuler->window, fontManager);
	UT_ASSERT(m_pG);
}

/*****************************************************************/

gint AP_UnixLeftRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_press_event\n",pUnixLeftRuler));
	return 1;
}

gint AP_UnixLeftRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_release_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] [size w %d h %d] received configure_event\n",
				 pUnixLeftRuler, e->width, e->height));

	UT_uint32 iHeight = (UT_uint32)e->height;
	if (iHeight != pUnixLeftRuler->getHeight())
	{
		pUnixLeftRuler->setHeight(iHeight);
		// TODO do we need to invalidate and redraw
	}

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pUnixLeftRuler->getWidth())
	{
		pUnixLeftRuler->setWidth(iWidth);
		// TODO do we need to invalidate and redraw
	}
	
	// TODO add code in the LeftRuler's configure_event code to send us
	// TODO a setOffsetLeftRuler() message.
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received motion_notify_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received key_press_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received delete_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pUnixLeftRuler)
		return 0;

	UT_Rect rClip;
	rClip.left = pExposeEvent->area.x;
	rClip.top = pExposeEvent->area.y;
	rClip.width = pExposeEvent->area.width;
	rClip.height = pExposeEvent->area.height;

	pUnixLeftRuler->draw(&rClip);
	return 0;
}

void AP_UnixLeftRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
