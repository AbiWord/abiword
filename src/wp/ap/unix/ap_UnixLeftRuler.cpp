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
#include "xap_UnixDialogHelper.h"
#include "fv_View.h"
#include "ut_sleep.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static void s_getWidgetRelativeMouseCoordinates(AP_UnixLeftRuler * pUnixLeftRuler,
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
	gdk_window_get_pointer((GdkWindow *) pUnixLeftRuler->getRootWindow(), &rx, &ry, &mask);

	// local (ruler widget) coordinates
	gint wx, wy;
	pUnixLeftRuler->getWidgetPosition(&wx, &wy);

	// subtract one from the other to catch all coordinates
	// relative to the widget's 0,
	*prx = rx - wx;
	*pry = ry - wy;
	return;
}

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
	m_rootWindow = NULL;
	m_wLeftRuler = NULL;
	m_pG = NULL;
	m_iBackgroundRedrawID = 0;
    // change ruler color on theme change
	GtkWidget * toplevel = (static_cast<XAP_UnixFrame *> (m_pFrame))->getTopLevelWindow();
	g_signal_connect_after (G_OBJECT(toplevel),
							  "client_event",
							  G_CALLBACK(ruler_style_changed),
							  (gpointer)this);
	m_wVruler = NULL;
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	if(m_iBackgroundRedrawID != 0)
		gtk_timeout_remove(m_iBackgroundRedrawID);
	while(m_pG->isSpawnedRedraw())
	{
		UT_usleep(100);
	}
	DELETEP(m_pG);
	m_wVruler = NULL;
}

void AP_UnixLeftRuler::_ruler_style_changed (void)
{
	_refreshView();
}

GtkWidget * AP_UnixLeftRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wLeftRuler);
	
	m_wLeftRuler = createDrawingArea ();

	gtk_object_set_user_data(GTK_OBJECT(m_wLeftRuler),this);
	gtk_widget_show(m_wLeftRuler);
	gtk_widget_set_usize(m_wLeftRuler, s_iFixedWidth, -1);

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
	if( m_iBackgroundRedrawID == 0)
	{
//
// Start background repaint
//
		m_iBackgroundRedrawID = gtk_timeout_add(200,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
	else
    {
		gtk_timeout_remove(m_iBackgroundRedrawID);
		m_iBackgroundRedrawID = gtk_timeout_add(200,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
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

	m_wVruler = gtk_vruler_new ();
	pG->init3dColors(get_ensured_style (m_wVruler));
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
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	xxx_UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_press_event\n",pUnixLeftRuler));

	FV_View * pView = (FV_View *) pUnixLeftRuler->m_pFrame->getCurrentView();
	if(pView && pView->getPoint()==0)
	{
//
// Abort
//
		return 0;
	}

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

	pUnixLeftRuler->mousePress(ems, emb, (long) e->x, (long) e->y);

	return 1;
}

gint AP_UnixLeftRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	xxx_UT_DEBUGMSG(("UnixLeftRuler: [p %p] received button_release_event\n",pUnixLeftRuler));
	EV_EditModifierState ems;
	EV_EditMouseButton emb = 0;

	FV_View * pView = (FV_View *) pUnixLeftRuler->m_pFrame->getCurrentView();
	if(pView && pView->getPoint()==0)
	{
//
// Abort
//
		return 0;
	}

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
	s_getWidgetRelativeMouseCoordinates(pUnixLeftRuler,&xrel,&yrel);

	pUnixLeftRuler->mouseRelease(ems, emb, xrel, yrel);

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
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
	
gint AP_UnixLeftRuler::_fe::motion_notify_event(GtkWidget* w , GdkEventMotion* e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("UnixLeftRuler: [p %p] received motion_notify_event\n",pUnixLeftRuler));

	FV_View * pView = (FV_View *) pUnixLeftRuler->m_pFrame->getCurrentView();
	if(pView && pView->getPoint()==0)
	{
//
// Abort
//
		return 0;
	}

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
	s_getWidgetRelativeMouseCoordinates(pUnixLeftRuler,&xrel,&yrel);

	pUnixLeftRuler->mouseMotion(ems, xrel, yrel);
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
// 	AP_UnixLeftRuler * pUnixLeftRuler = (AP_UnixLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	xxx_UT_DEBUGMSG(("UnixLeftRuler: [p %p] received key_press_event\n",pUnixLeftRuler));
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
	xxx_UT_DEBUGMSG(("gtk in leftruler expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	GR_Graphics * pG = pUnixLeftRuler->getGraphics();
	if(pG != NULL)
	{
		pUnixLeftRuler->getGraphics()->doRepaint(&rClip);
//		pUnixLeftRuler->draw(&rClip);
	}
	else
	{
		UT_DEBUGMSG(("No graphics Context. Doing fallback. \n"));
//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		FV_View * pView = (FV_View *) pUnixLeftRuler->m_pFrame->getCurrentView();
		if(pView && pView->getPoint()==0)
		{
//
// Abort
//
			return 0;
		}
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
	FV_View * pView = (FV_View *) pR->m_pFrame->getCurrentView();
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
