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
#include "ap_CocoaFrame.h"

#include "gr_CocoaGraphics.h"
#include "ap_CocoaTopRuler.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

#if 0
static void s_getWidgetRelativeMouseCoordinates(AP_CocoaTopRuler * pCocoaTopRuler,
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
	gdk_window_get_pointer((GdkWindow *) pCocoaTopRuler->getRootWindow(), &rx, &ry, &mask);

	// local (ruler widget) coordinates
	gint wx, wy;
	pCocoaTopRuler->getWidgetPosition(&wx, &wy);

	// subtract one from the other to catch all coordinates
	// relative to the widget's 0,
	*prx = rx - wx;
	*pry = ry - wy;
	return;
}

/*****************************************************************/

//evil ugly hack
static int ruler_style_changed (GtkWidget * w, GdkEventClient * event,
								AP_CocoaTopRuler * ruler)
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

#endif


AP_CocoaTopRuler::AP_CocoaTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;

	m_wTopRuler = [(AP_CocoaFrameController *)(static_cast<XAP_CocoaFrameHelper *>(m_pFrame->getFrameHelper())->_getController()) getHRuler];

#if 0
	// change ruler color on theme change
	NSWindow * toplevel = (static_cast<XAP_CocoaFrame *> (m_pFrame))->getTopLevelWindow();
	g_signal_connect_after (G_OBJECT(toplevel),
							  "client_event",
							  G_CALLBACK(ruler_style_changed),
							  (gpointer)this);
#endif
}

AP_CocoaTopRuler::~AP_CocoaTopRuler(void)
{
	DELETEP(m_pG);
}

#if 0
void AP_CocoaTopRuler::_ruler_style_changed (void)
{
	_refreshView();
}
#endif

XAP_CocoaNSView * AP_CocoaTopRuler::createWidget(void)
{
	//UT_DEBUGMSG(("AP_CocoaTopRuler::createWidget - [w=%p] [this=%p]\n", m_wTopRuler,this));

#if 0
	g_object_set_user_data(G_OBJECT(m_wTopRuler),this);
	gtk_widget_show(m_wTopRuler);
	gtk_widget_set_usize(m_wTopRuler, -1, s_iFixedHeight);

	gtk_widget_set_events(GTK_WIDGET(m_wTopRuler), (GDK_EXPOSURE_MASK |
													GDK_BUTTON_PRESS_MASK |
													GDK_POINTER_MOTION_MASK |
													GDK_BUTTON_RELEASE_MASK |
													GDK_KEY_PRESS_MASK |
													GDK_KEY_RELEASE_MASK));

	g_signal_connect(G_OBJECT(m_wTopRuler), "expose_event",
					   G_CALLBACK(_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_wTopRuler), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_wTopRuler), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_wTopRuler), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);
  
	g_signal_connect(G_OBJECT(m_wTopRuler), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);
#endif

	return m_wTopRuler;
}

void AP_CocoaTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);

	GR_CocoaGraphics * pG = new GR_CocoaGraphics(m_wTopRuler, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);
	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);

//	GtkWidget * ruler = gtk_hruler_new ();
// TODO	pG->init3dColors(get_ensured_style(ruler));
}

void AP_CocoaTopRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(x && y);
	
	NSRect theBounds = [m_wTopRuler bounds];
	*x = (int)theBounds.size.width;
	*y = (int)theBounds.size.height;
}

NSWindow * AP_CocoaTopRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = [m_wTopRuler window];
	return m_rootWindow;
}


bool AP_CocoaTopRuler::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)param;
	if (!pCocoaTopRuler)
		return false;

	UT_Rect rClip;
	rClip.left = (UT_sint32)aRect->origin.x;
	rClip.top = (UT_sint32)aRect->origin.y;
	rClip.width = (UT_sint32)aRect->size.width;
	rClip.height = (UT_sint32)aRect->size.height;
	xxx_UT_DEBUGMSG(("Cocoa in topruler expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	if(pG != NULL)
	{
//		pCocoaTopRuler->getGraphics()->doRepaint(&rClip);
		pCocoaTopRuler->draw(&rClip);
	}
	else {
		return false;
	}
	return true;
}
		
/*****************************************************************/

#if 0
gint AP_CocoaTopRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));

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


	if (1 == e->button )
		emb = EV_EMB_BUTTON1;
	else if (2 == e->button )
		emb = EV_EMB_BUTTON2;
	else if (3 == e->button)
		emb = EV_EMB_BUTTON3;

	UT_DEBUGMSG(("SEVIOR: e->button = %x \n",e->button));
	pCocoaTopRuler->mousePress(ems, emb, (UT_uint32) e->x, (UT_uint32) e->y);
	return 1;
}

gint AP_CocoaTopRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));

	EV_EditModifierState ems;
	EV_EditMouseButton emb = 0;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (1 == e->button )
		emb = EV_EMB_BUTTON1;
	else if (2 == e->button )
		emb = EV_EMB_BUTTON2;
	else if (3 == e->button)
		emb = EV_EMB_BUTTON3;

	// Map the mouse into coordinates relative to our window.
	gint xrel, yrel;
	s_getWidgetRelativeMouseCoordinates(pCocoaTopRuler,&xrel,&yrel);

	pCocoaTopRuler->mouseRelease(ems, emb, xrel, yrel);

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
gint AP_CocoaTopRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));

	//UT_DEBUGMSG(("CocoaTopRuler: [p %p] [size w %d h %d] received configure_event\n",
	//			 pCocoaTopRuler, e->width, e->height));

	UT_uint32 iHeight = (UT_uint32)e->height;
	if (iHeight != pCocoaTopRuler->getHeight())
		pCocoaTopRuler->setHeight(iHeight);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != pCocoaTopRuler->getWidth())
		pCocoaTopRuler->setWidth(iWidth);
	
	return 1;
}

	
gint AP_CocoaTopRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));

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
	s_getWidgetRelativeMouseCoordinates(pCocoaTopRuler,&xrel,&yrel);

	pCocoaTopRuler->mouseMotion(ems, xrel, yrel);
	pCocoaTopRuler->isMouseOverTab((UT_uint32) e->x,(UT_uint32)e->y);

	return 1;

}
	
gint AP_CocoaTopRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));
	UT_DEBUGMSG(("CocoaTopRuler: [p %p] received key_press_event\n",pCocoaTopRuler));
	return 1;
}
	
gint AP_CocoaTopRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)g_object_get_user_data(G_OBJECT(w));
	// UT_DEBUGMSG(("CocoaTopRuler: [p %p] received delete_event\n",pCocoaTopRuler));
	return 1;
}


void AP_CocoaTopRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}

#endif
