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
#include "ut_dialogHelper.h"

#include "xap_Frame.h"
#include "xap_UnixFrame.h"

#include "gr_UnixGraphics.h"
#include "ap_UnixTopRuler.h"

#define DELETEP(p)		do { if (p) delete p; p = NULL; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static void s_getWidgetRelativeMouseCoordinates(AP_UnixTopRuler * pUnixTopRuler,
												gint * prx, gint * pry)
{
	// TODO there is what appears to be a bug in GTK where
	// TODO mouse coordinates that we receive (motion and
	// TODO release) when we have a grab are relative to
	// TODO whatever window the mouse is over ***AND NOT***
	// TODO relative to our window.  the following ***HACK***
	// TODO is used to map the mouse coordinates relative to
	// TODO our widget.

	// root (absolute) coordinates
	gint rx, ry;
	GdkModifierType mask;
	gdk_window_get_pointer((GdkWindow *) pUnixTopRuler->getRootWindow(), &rx, &ry, &mask);

	// local (ruler widget) coordinates
	gint wx, wy;
	pUnixTopRuler->getWidgetPosition(&wx, &wy);

	// subtract one from the other to catch all coordinates
	// relative to the widget's 0,
	*prx = rx - wx;
	*pry = ry - wy;
	return;
}

/*****************************************************************/

AP_UnixTopRuler::AP_UnixTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;

	// Initialize ruler colors to match the style of the GTK Window
	// representing pFrame
	GtkStyle * style = gtk_widget_get_default_style();
	UT_ASSERT(style);

	UT_setColor(m_clrBackground, style->bg[GTK_STATE_NORMAL].red, style->bg[GTK_STATE_NORMAL].green, style->bg[GTK_STATE_NORMAL].blue);
}

AP_UnixTopRuler::~AP_UnixTopRuler(void)
{
	DELETEP(m_pG);
}

GtkWidget * AP_UnixTopRuler::createWidget(void)
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

void AP_UnixTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	XAP_UnixApp * app = static_cast<XAP_UnixApp *>(m_pFrame->getApp());
	XAP_UnixFontManager * fontManager = app->getFontManager();
	m_pG = new GR_UnixGraphics(m_wTopRuler->window, fontManager);
	UT_ASSERT(m_pG);
}

void AP_UnixTopRuler::getWidgetPosition(gint * x, gint * y)
{
	UT_ASSERT(x && y);
	
	gdk_window_get_position(m_wTopRuler->window, x, y);
}

GdkWindowPrivate * AP_UnixTopRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = ::getRootWindow(m_wTopRuler);
	return m_rootWindow;
}

		
/*****************************************************************/

gint AP_UnixTopRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	// grab the mouse for the duration of the drag.
	gtk_grab_add(w);
	
	EV_EditModifierState ems;
	EV_EditMouseButton emb = 0;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;

	pUnixTopRuler->mousePress(ems, emb, (long) e->x, (long) e->y);
	return 1;
}

gint AP_UnixTopRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	EV_EditModifierState ems;
	EV_EditMouseButton emb = 0;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;

	// Map the mouse into coordinates relative to our window.
	gint xrel, yrel;
	s_getWidgetRelativeMouseCoordinates(pUnixTopRuler,&xrel,&yrel);

	pUnixTopRuler->mouseRelease(ems, emb, xrel, yrel);

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
gint AP_UnixTopRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	UT_uint32 iHeight = (UT_uint32)e->height;
	if (iHeight != pUnixTopRuler->getHeight())
		pUnixTopRuler->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pUnixTopRuler->getWidth())
		pUnixTopRuler->setWidth(iWidth);
	
	return 1;
}

	
gint AP_UnixTopRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	EV_EditModifierState ems;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	// Map the mouse into coordinates relative to our window.
	gint xrel, yrel;
	s_getWidgetRelativeMouseCoordinates(pUnixTopRuler,&xrel,&yrel);

	pUnixTopRuler->mouseMotion(ems, xrel, yrel);
	return 1;

}
	
gint AP_UnixTopRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("UnixTopRuler: [p %p] received key_press_event\n",pUnixTopRuler));
	return 1;
}
	
gint AP_UnixTopRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("UnixTopRuler: [p %p] received delete_event\n",pUnixTopRuler));
	return 1;
}
	
gint AP_UnixTopRuler::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = (AP_UnixTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (!pUnixTopRuler)
		return 0;

	UT_Rect rClip;
	rClip.left = pExposeEvent->area.x;
	rClip.top = pExposeEvent->area.y;
	rClip.width = pExposeEvent->area.width;
	rClip.height = pExposeEvent->area.height;

	pUnixTopRuler->draw(&rClip);
	return 0;
}

void AP_UnixTopRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
