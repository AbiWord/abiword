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
class TopRulerDrawView: public be_GRDrawView {
public:
 	TopRulerDrawView(AP_BeOSTopRuler *pRuler, AV_View *pView, 
			 BRect frame, const char *name,
                         uint32 resizeMask, uint32 flags);
	virtual void FrameResized(float new_width, float new_height);
	virtual void Draw(BRect invalid);

	AP_BeOSTopRuler *m_pAPRuler;
};

class RulerFilter: public BMessageFilter {
        public:
                RulerFilter(TopRulerDrawView *pRuler);
                filter_result Filter(BMessage *message, BHandler **target);
        private:
                TopRulerDrawView  *m_pRuler;
};

RulerFilter::RulerFilter(TopRulerDrawView *pRuler)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) {
        m_pRuler = pRuler;
}

filter_result RulerFilter::Filter(BMessage *msg, BHandler **target) {
        switch(msg->what) {
        case B_MOUSE_DOWN:
        case B_MOUSE_UP:
        case B_MOUSE_MOVED:
                break;
        default:
                return(B_DISPATCH_MESSAGE);
        }

	if (!m_pRuler)
		return(B_DISPATCH_MESSAGE);
        AP_BeOSTopRuler * pBeOSTopRuler = m_pRuler->m_pAPRuler;
	if (!pBeOSTopRuler)
		return(B_DISPATCH_MESSAGE);

  	BPoint  pt;
        int32   clicks, mod, buttons;
        EV_EditModifierState ems = 0;
        EV_EditMouseButton emb = 0;

        msg->FindInt32("clicks", &clicks);
        msg->FindInt32("buttons", &buttons);
        msg->FindInt32("modifiers", &mod);
        msg->FindPoint("where", &pt);

        if (mod & B_SHIFT_KEY)
                ems |= EV_EMS_SHIFT;
        if (mod & B_CONTROL_KEY)
                ems |= EV_EMS_CONTROL;
        if (mod & B_OPTION_KEY)
                ems |= EV_EMS_ALT;

        if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON1;
        else if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON2;
        else if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON3;           

	
	printf("Mouse Position: "); pt.PrintToStream();

#if SEND_MOUSE_EVENT
	if (msg->what == B_MOUSE_DOWN)
		pBeOSTopRuler->mousePress(ems, emb,(long)pt.x,(long)pt.y);
	else if (msg->what == B_MOUSE_UP)
	        pBeOSTopRuler->mouseRelease(ems, emb, (long)pt.x, (long)pt.y);
	else if (msg->what == B_MOUSE_MOVED && emb)
 		pBeOSTopRuler->mouseMotion(ems, (long)pt.x, (long)pt.y);   

        return(B_DISPATCH_MESSAGE);
#endif
        return(B_SKIP_MESSAGE);
}                                             

TopRulerDrawView::TopRulerDrawView(AP_BeOSTopRuler *pRuler, AV_View *pView, 
				   BRect frame, const char *name,
                                   uint32 resizeMask, uint32 flags) 
		: be_GRDrawView(pView, frame, name, resizeMask, flags) {
	m_pAPRuler = pRuler;
 	//Window()->Lock();
        AddFilter(new RulerFilter(this));
        //Window()->Unlock(); 
}

void TopRulerDrawView::FrameResized(float new_width, float new_height) {
	m_pAPRuler->setHeight((int)new_height);
        m_pAPRuler->setWidth((int)new_width);
	//Then call the inherited version (only on grow?)
	//be_GRDrawView::FrameResized(new_width, new_height);
	m_pAPRuler->draw(NULL);
}

void TopRulerDrawView::Draw(BRect invalid) {
	UT_Rect rect(invalid.left,invalid.top, 
		     invalid.Width(), invalid.Height());
	m_pAPRuler->draw(&rect);
	//m_pAPRuler->draw(NULL);
}

/*****************************************************************/

AP_BeOSTopRuler::AP_BeOSTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_wTopRuler = NULL;
}

AP_BeOSTopRuler::~AP_BeOSTopRuler(void)
{
}

void AP_BeOSTopRuler::createWidget(BRect r)
{
	printf("TOPRULER: create widget \n");
	/*
	m_wTopRuler = new be_GRDrawView(NULL, r, "TopRuler", 
					B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
					B_WILL_DRAW);
	*/
	m_wTopRuler = new TopRulerDrawView(this, NULL, r, "TopRuler", 
					B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
					B_WILL_DRAW);

	//Attach the widget to the window ...
	be_Window *pBWin = (be_Window*)((XAP_BeOSFrame *)m_pFrame)->getTopLevelWindow();
	pBWin->AddChild(m_wTopRuler);
	printf("Setting height/width to %d/%d \n", r.Height(), r.Width());
 	setHeight(r.Height());
        setWidth(r.Width());
}

void AP_BeOSTopRuler::setView(AV_View * pView) {
	printf("TOPRULER: SetView \n");
	AP_TopRuler::setView(pView);

	if (m_wTopRuler && pView) {
		m_wTopRuler->SetView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

		DELETEP(m_pG);	
		m_pG = new GR_BEOSGraphics(m_wTopRuler);
		UT_ASSERT(m_pG);
	}
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
