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
#include "ap_QNXTopRuler.h"
#include <stdio.h>

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/
#if 0
static void s_getWidgetRelativeMouseCoordinates(AP_QNXTopRuler * pQNXTopRuler,
												int * prx, int * pry)
{
	// TODO there is what appears to be a bug in GTK where
	// TODO mouse coordinates that we receive (motion and
	// TODO release) when we have a grab are relative to
	// TODO whatever window the mouse is over ***AND NOT***
	// TODO relative to our window.  the following ***HACK***
	// TODO is used to map the mouse coordinates relative to
	// TODO our widget.

	// root (absolute) coordinates
	int rx, ry;
	GdkModifierType mask;
	gdk_window_get_pointer((GdkWindow *) pQNXTopRuler->getRootWindow(), &rx, &ry, &mask);

	// local (ruler widget) coordinates
	int wx, wy;
	pQNXTopRuler->getWidgetPosition(&wx, &wy);

	// subtract one from the other to catch all coordinates
	// relative to the widget's 0,
	*prx = rx - wx;
	*pry = ry - wy;
	return;
}
#endif
/*****************************************************************/

AP_QNXTopRuler::AP_QNXTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;
}

AP_QNXTopRuler::~AP_QNXTopRuler(void)
{
	DELETEP(m_pG);
}

PtWidget_t * AP_QNXTopRuler::createWidget(void)
{
	PtArg_t args[10];
	PhArea_t area;
	void 	*data = this;
	int n = 0;
	UT_ASSERT(!m_pG && !m_wTopRuler);

	XAP_QNXFrame *pQNXFrame = (XAP_QNXFrame *)m_pFrame;
	m_rootWindow = pQNXFrame->getTopLevelWindow();

	area.pos.x = 0;
	area.pos.y = pQNXFrame->m_AvailableArea.pos.y;
	area.size.w = pQNXFrame->m_AvailableArea.size.w;
	area.size.h = s_iFixedHeight;
	pQNXFrame->m_AvailableArea.pos.y += area.size.h + 3;
	pQNXFrame->m_AvailableArea.size.h -= area.size.h + 3;
	PtSetArg(&args[n], Pt_ARG_AREA, &area, 0); n++;
	UT_DEBUGMSG(("TR: Offset %d,%d Size %d/%d \n",
				area.pos.x, area.pos.y, area.size.w, area.size.h));
	PtSetArg(&args[n], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); n++;
#define _TR_ANCHOR_     (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
                         Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_TOP)
      PtSetArg(&args[n], Pt_ARG_ANCHOR_FLAGS, _TR_ANCHOR_, _TR_ANCHOR_); n++;
#define _TR_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
        PtSetArg(&args[n], Pt_ARG_GROUP_FLAGS, _TR_STRETCH_, _TR_STRETCH_); n++;
        PtSetArg(&args[n], Pt_ARG_BORDER_WIDTH, 2, 2); n++;
        PtSetArg(&args[n], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED); n++;
	PtWidget_t *cont = PtCreateWidget(PtGroup, m_rootWindow, n, args);

	n = 0;
	PtSetArg(&args[n], Pt_ARG_DIM, &area.size, 0); n++;
	PtSetArg(&args[n], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); n++;
	PtSetArg(&args[n], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1); n++;
	PtSetArg(&args[n], Pt_ARG_USER_DATA, &data, sizeof(this)); n++;
 	PtSetArg(&args[n], Pt_ARG_FLAGS, Pt_GETS_FOCUS, Pt_GETS_FOCUS); n++;
	m_wTopRuler = PtCreateWidget(PtRaw, cont, n, args);

/*
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
*/
	return m_wTopRuler;
}

void AP_QNXTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	GR_QNXGraphics * pG = new GR_QNXGraphics(((XAP_QNXFrame *)m_pFrame)->getTopLevelWindow(), 
											 m_wTopRuler);
	m_pG = pG;
	UT_ASSERT(m_pG);
	pG->init3dColors();
}

void AP_QNXTopRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(x && y);
	PtArg_t  args[1];
	PhArea_t *area;

    PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
    PtGetResources(m_wTopRuler, 1, args);
	if (x)
		*x = area->pos.x;
	if (y)
		*y = area->pos.y;
}

void * AP_QNXTopRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	if (m_pFrame)
		return (m_rootWindow = ((XAP_QNXFrame *)m_pFrame)->getTopLevelWindow()) ;

	return NULL;
}

		
/*****************************************************************/
#if 0
int AP_QNXTopRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

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

	pQNXTopRuler->mousePress(ems, emb, (long) e->x, (long) e->y);
	return 1;
}

int AP_QNXTopRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

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
	int xrel, yrel;
	s_getWidgetRelativeMouseCoordinates(pQNXTopRuler,&xrel,&yrel);

	pQNXTopRuler->mouseRelease(ems, emb, xrel, yrel);

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
int AP_QNXTopRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));

	EV_EditModifierState ems;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	// Map the mouse into coordinates relative to our window.
	int xrel, yrel;
	s_getWidgetRelativeMouseCoordinates(pQNXTopRuler,&xrel,&yrel);

	pQNXTopRuler->mouseMotion(ems, xrel, yrel);
	return 1;

}
	
int AP_QNXTopRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("QNXTopRuler: [p %p] received key_press_event\n",pQNXTopRuler));
	return 1;
}
	
int AP_QNXTopRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("QNXTopRuler: [p %p] received delete_event\n",pQNXTopRuler));
	return 1;
}
#endif
	
int AP_QNXTopRuler::_fe::expose(PtWidget_t * w, PhTile_t *damage)
{
	PtArg_t args[1];
	UT_Rect rClip;
	PhRect_t rect;

	PtSuperClassDraw(PtBasic, w, damage);
	PtBasicWidgetCanvas(w, &rect);

	AP_QNXTopRuler ** ppQNXRuler = NULL, *pQNXRuler = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXRuler, 0);
	PtGetResources(w, 1, args);
	pQNXRuler = (ppQNXRuler) ? *ppQNXRuler : NULL;

	if (!pQNXRuler) {
		return 0;
	}

#if 0
    if (damage->next) {
        damage = damage->next;
    }
    while (damage) {
        rClip.left = damage->rect.ul.x;
        rClip.top = damage->rect.ul.y;
        rClip.width = damage->rect.lr.x - damage->rect.ul.x;
        rClip.height = damage->rect.lr.y - damage->rect.ul.y;
        damage = damage->next;

        printf("Clip to Rect %d,%d %d/%d \n",
            rClip.left, rClip.top, rClip.width, rClip.height);
		pQNXRuler->draw(&rClip);
		//pQNXRuler->draw(NULL);
    }
#else
	pQNXRuler->draw(NULL);
#endif

	return 0;
}

#if 0
void AP_QNXTopRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
#endif
