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
#import "ev_CocoaMouse.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"
#import "ap_FrameData.h"
#include "ap_CocoaFrame.h"

#include "gr_CocoaGraphics.h"
#include "ap_CocoaTopRuler.h"

#import "ap_CocoaFrameImpl.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

@interface AP_CocoaTopRulerDelegate : NSObject <XAP_MouseEventDelegate>
{
}
@end


AP_CocoaTopRuler::AP_CocoaTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;

	m_wTopRuler = [(AP_CocoaFrameController *)(static_cast<XAP_CocoaFrameImpl *>(m_pFrame->getFrameImpl())->_getController()) getHRuler];

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
	[m_wTopRuler setEventDelegate:[[[AP_CocoaTopRulerDelegate alloc] init] autorelease]];
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

@implementation AP_CocoaTopRulerDelegate

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)pFrameData->m_pTopRuler;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pt.y = [sender bounds].size.height - pt.y;
	pCocoaTopRuler->mousePress(ems, emb, (UT_uint32) pt.x, (UT_uint32) pt.y);
}


- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)pFrameData->m_pTopRuler;

	EV_EditModifierState ems = 0;
	
	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pt.y = [sender bounds].size.height - pt.y;
	pCocoaTopRuler->mouseMotion(ems, (UT_sint32)pt.x, (UT_sint32)pt.y);
	pCocoaTopRuler->isMouseOverTab((UT_uint32) pt.x,(UT_uint32)pt.y);
}


- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)pFrameData->m_pTopRuler;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pt.y = [sender bounds].size.height - pt.y;
	pCocoaTopRuler->mouseRelease(ems, emb, (UT_sint32)pt.x, (UT_sint32)pt.y);
}
@end
