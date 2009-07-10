/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include "xap_UnixFrameImpl.h"
#include "ap_UnixLeftRuler.h"
#include "gr_UnixCairoGraphics.h"
#include "xap_UnixDialogHelper.h"
#include "fv_View.h"
#include "ut_sleep.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static void
ruler_style_changed (GtkWidget 			* /*w*/, 
					 GtkStyle 			* /*previous_style*/,
					 AP_UnixLeftRuler 	*ruler)
{
	ruler->_ruler_style_changed();
}

AP_UnixLeftRuler::AP_UnixLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wLeftRuler = NULL;
	m_pG = NULL;
	m_iBackgroundRedrawID = 999999;
    // change ruler color on theme change
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_iBackgroundRedrawID = g_signal_connect_after (G_OBJECT(toplevel),
							  "style-set",
							  G_CALLBACK(ruler_style_changed),
							  static_cast<gpointer>(this));
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	if (toplevel && g_signal_handler_is_connected(G_OBJECT(toplevel), m_iBackgroundRedrawID)) {
		g_signal_handler_disconnect(G_OBJECT(toplevel),m_iBackgroundRedrawID);
	}
	while(m_pG && m_pG->isSpawnedRedraw())
	{
		UT_usleep(100);
	}
	DELETEP(m_pG);
}

void AP_UnixLeftRuler::_ruler_style_changed (void)
{
	_refreshView();
}

GtkWidget * AP_UnixLeftRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wLeftRuler);
	
	m_wLeftRuler = createDrawingArea ();

	g_object_set_data(G_OBJECT(m_wLeftRuler), "user_data", this);
	gtk_widget_show(m_wLeftRuler);
	gtk_widget_set_size_request(m_wLeftRuler, s_iFixedWidth, -1);

	gtk_widget_set_events(GTK_WIDGET(m_wLeftRuler), (GDK_EXPOSURE_MASK |
													 GDK_BUTTON_PRESS_MASK |
													 GDK_POINTER_MOTION_MASK |
													 GDK_BUTTON_RELEASE_MASK |
													 GDK_KEY_PRESS_MASK |
													 GDK_KEY_RELEASE_MASK));

	g_signal_connect(G_OBJECT(m_wLeftRuler), "expose_event",
					   G_CALLBACK(_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_wLeftRuler), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_wLeftRuler), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_wLeftRuler), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);
  
	g_signal_connect(G_OBJECT(m_wLeftRuler), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);

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

	GR_UnixCairoAllocInfo ai(m_wLeftRuler->window);
	m_pG = XAP_App::getApp()->newGraphics(ai);

	UT_ASSERT(m_pG);
	m_pG->setZoomPercentage(pView->getGraphics()->getZoomPercentage());

	GtkWidget * ruler = gtk_vruler_new ();
	((GR_UnixCairoGraphics*)m_pG)->init3dColors(get_ensured_style (ruler));
}

void AP_UnixLeftRuler::getWidgetPosition(gint * x, gint * y)
{
	UT_ASSERT(x && y);
	
	gdk_window_get_position(m_wLeftRuler->window, x, y);
}

GdkWindow * AP_UnixLeftRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = ::getRootWindow(m_wLeftRuler);
	return m_rootWindow;
}

/*****************************************************************/

gint AP_UnixLeftRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

	// grab the mouse for the duration of the drag.
	gtk_grab_add(w);
	
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	
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

	pUnixLeftRuler->mousePress(ems, emb, 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)), 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));

	return 1;
}

gint AP_UnixLeftRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

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

	pUnixLeftRuler->mouseRelease(ems, emb, 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)),
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	// nb: we'd convert here, but we can't: have no graphics class!
	pUnixLeftRuler->setHeight(e->height);
	pUnixLeftRuler->setWidth(e->width);
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::motion_notify_event(GtkWidget* w , GdkEventMotion* e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

	EV_EditModifierState ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	pUnixLeftRuler->mouseMotion(ems, 
				    pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)),
				    pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::key_press_event(GtkWidget* /*w*/, GdkEventKey* /* e */)
{
	// a static function
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
	if (!pUnixLeftRuler)
		return 0;

	GR_Graphics * pG = pUnixLeftRuler->getGraphics();
	if(pG != NULL)
	{
		UT_Rect rClip;
		rClip.left = pG->tlu(pExposeEvent->area.x);
		rClip.top = pG->tlu(pExposeEvent->area.y);
		rClip.width = pG->tlu(pExposeEvent->area.width);
		rClip.height = pG->tlu(pExposeEvent->area.height);

		pUnixLeftRuler->draw(&rClip);
	}
	return 0;
}


/*!
 * Background abi repaint function.
\param XAP_UnixFrame * p pointer to the Frame that initiated this background
       repainter.
 */
gint AP_UnixLeftRuler::_fe::abi_expose_repaint( gpointer p)
{
//
// Grab our pointer so we can do useful stuff.
//
	UT_Rect localCopy;
	AP_UnixLeftRuler * pR = static_cast<AP_UnixLeftRuler *>(p);
	GR_Graphics * pG = pR->getGraphics();
	if(pG == NULL || pG->isDontRedraw())
	{
//
// Come back later
//
		return TRUE;
	}
	FV_View * pView = static_cast<FV_View *>(pR->m_pFrame->getCurrentView());
	if(pView && pView->getPoint()==0)
	{
//
// Come back later
//
		return TRUE;
	}
		
	pG->setSpawnedRedraw(true);
	if(pG->isExposePending())
	{
		while(pG->isExposedAreaAccessed())
		{
			UT_usleep(10); // 10 microseconds
		}
		pG->setExposedAreaAccessed(true);
		localCopy.set(pG->getPendingRect()->left,pG->getPendingRect()->top,
			      pG->getPendingRect()->width,pG->getPendingRect()->height);
//
// Clear out this set of expose info
//
		pG->setExposePending(false);
		pG->setExposedAreaAccessed(false);
		xxx_UT_DEBUGMSG(("Painting area on Left ruler:  left=%d, top=%d, width=%d, height=%d\n", localCopy.left, localCopy.top, localCopy.width, localCopy.height));
		xxx_UT_DEBUGMSG(("SEVIOR: Repaint now \n"));
		pR->draw(&localCopy);
	}
//
// OK we've finshed. Wait for the next signal
//
	pG->setSpawnedRedraw(false);
	return TRUE;
}

void AP_UnixLeftRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
