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
#include "ut_dialogHelper.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

// evil ugly hack
static int ruler_style_changed (GtkWidget * w, GdkEventClient * event,
								AP_UnixLeftRuler * ruler)
{
	static GdkAtom atom_rcfiles = GDK_NONE;
	g_return_val_if_fail (w != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	if (!atom_rcfiles)
		atom_rcfiles = gdk_atom_intern ("_GTK_READ_RCFILES", FALSE);
	if (event->message_type != atom_rcfiles)
		return FALSE;
	ruler->_ruler_style_changed();
	return FALSE;
}

AP_UnixLeftRuler::AP_UnixLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_wLeftRuler = NULL;
	m_ruler = gtk_vruler_new ();
	m_pG = NULL;
	
    // change ruler color on theme change
	GtkWidget * toplevel = (static_cast<XAP_UnixFrame *> (m_pFrame))->getTopLevelWindow();
	gtk_signal_connect_after (GTK_OBJECT(toplevel),
							  "client_event",
							  GTK_SIGNAL_FUNC(ruler_style_changed),
							  (gpointer)this);
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	DELETEP(m_pG);
	gtk_widget_destroy (m_ruler);
}

void AP_UnixLeftRuler::_ruler_style_changed (void)
{
	if (m_ruler)
		gtk_widget_destroy (m_ruler);

	m_ruler = gtk_vruler_new ();
	setView(m_pView);
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

	XAP_UnixApp * app = static_cast<XAP_UnixApp *>(m_pFrame->getApp());
	XAP_UnixFontManager * fontManager = app->getFontManager();
	GR_UnixGraphics * pG = new GR_UnixGraphics(m_wLeftRuler->window, fontManager, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

	pG->init3dColors(get_ensured_style (m_ruler));
}

/*****************************************************************/

gint AP_UnixLeftRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * /* e */)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_press_event\n",pUnixLeftRuler));
	return 1;
}

gint AP_UnixLeftRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * /* e */)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_release_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	// UT_DEBUGMSG(("UnixLeftRuler: [p %p] [size w %d h %d] received configure_event\n",
	//			 pUnixLeftRuler, e->width, e->height));

	UT_uint32 iHeight = (UT_uint32)e->height;
	if (iHeight != pUnixLeftRuler->getHeight())
		pUnixLeftRuler->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pUnixLeftRuler->getWidth())
		pUnixLeftRuler->setWidth(iWidth);
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::motion_notify_event(GtkWidget* /* w */, GdkEventMotion* /* e */)
{
	// a static function
	// AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("UnixLeftRuler: [p %p] received motion_notify_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixLeftRuler: [p %p] received key_press_event\n",pUnixLeftRuler));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("UnixLeftRuler: [p %p] received delete_event\n",pUnixLeftRuler));
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
