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
#include "ap_QNXLeftRuler.h"
#include "gr_QNXGraphics.h"
#include <stdio.h>

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

AP_QNXLeftRuler::AP_QNXLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_wLeftRuler = NULL;
	m_pG = NULL;
}

AP_QNXLeftRuler::~AP_QNXLeftRuler(void)
{
	DELETEP(m_pG);
}

PtWidget_t * AP_QNXLeftRuler::createWidget(void)
{
	PtArg_t args[10];
	PhArea_t area;
	void    *data = this;
	int n = 0;
	UT_ASSERT(!m_pG && !m_wLeftRuler);

	XAP_QNXFrame *pQNXFrame = (XAP_QNXFrame *)m_pFrame;
	UT_ASSERT(m_pFrame);

	area.pos.x = 0;
	area.pos.y = pQNXFrame->m_AvailableArea.pos.y;
	area.size.w = s_iFixedWidth;
	area.size.h = pQNXFrame->m_AvailableArea.size.h;
	pQNXFrame->m_AvailableArea.pos.x += area.size.w + 3;
	pQNXFrame->m_AvailableArea.size.w -= area.size.w + 3;
	PtSetArg(&args[n], Pt_ARG_AREA, &area, 0); n++;
	UT_DEBUGMSG(("LR: Offset %d,%d Size %d/%d ",
                area.pos.x, area.pos.y, area.size.w, area.size.h));
	PtSetArg(&args[n], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); n++;
#define _LR_ANCHOR_     (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_LEFT | \
                         Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n], Pt_ARG_ANCHOR_FLAGS, _LR_ANCHOR_, _LR_ANCHOR_); n++;
#define _LR_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
	PtSetArg(&args[n], Pt_ARG_GROUP_FLAGS, _LR_STRETCH_, _LR_STRETCH_); n++;
	PtSetArg(&args[n], Pt_ARG_BORDER_WIDTH, 2, 0); n++;
	PtSetArg(&args[n], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED); n++;
	m_wLeftRulerGroup = PtCreateWidget(PtGroup, pQNXFrame->getTopLevelWindow(), n, args);
	PtAddCallback(m_wLeftRulerGroup, Pt_CB_RESIZE, &(_fe::resize), this);

	n = 0;
	PtSetArg(&args[n], Pt_ARG_DIM, &area.size, 0); n++;
	PtSetArg(&args[n], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); n++;
	PtSetArg(&args[n], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1); n++;
	PtSetArg(&args[n], Pt_ARG_USER_DATA, &data, sizeof(this)); n++;
	PtSetArg(&args[n], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); n++;
	m_wLeftRuler = PtCreateWidget(PtRaw, m_wLeftRulerGroup, n, args);
	PtAddEventHandler(m_wLeftRuler, Ph_EV_PTR_MOTION_BUTTON /* Ph_EV_PTR_MOTION */, 
								  _fe::motion_notify_event, this);
	PtAddEventHandler(m_wLeftRuler, Ph_EV_BUT_PRESS, _fe::button_press_event, this);
	PtAddEventHandler(m_wLeftRuler, Ph_EV_BUT_RELEASE, _fe::button_release_event, this);


	return m_wLeftRulerGroup;
}

void AP_QNXLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wLeftRuler->window)
	// is not created until the frame's top-level window is
	// shown.
	DELETEP(m_pG);
	GR_QNXGraphics * pG = new GR_QNXGraphics(((XAP_QNXFrame *)m_pFrame)->getTopLevelWindow(),
                                           m_wLeftRuler, m_pFrame->getApp());
	m_pG = pG;
	pG->init3dColors();
}

/*****************************************************************/
#if 0
int AP_QNXLeftRuler::_fe::key_press_event(GtkWidget* w, GdkEventKey* /* e */)
{
	// a static function
	AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_DEBUGMSG(("QNXLeftRuler: [p %p] received key_press_event\n",pQNXLeftRuler));
	return 1;
}
	
int AP_QNXLeftRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	// AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)gtk_object_get_user_data(GTK_OBJECT(w));
	// UT_DEBUGMSG(("QNXLeftRuler: [p %p] received delete_event\n",pQNXLeftRuler));
	return 1;
}
#endif

/* Both the left and the top ruler hve this same function */
static int get_stuff(PtCallbackInfo_t *info, EV_EditModifierState *ems, 
									EV_EditMouseButton *emb, int *x, int *y) {
	PhPointerEvent_t *ptrevent;
    PhRect_t         *rect;

	ptrevent = (PhPointerEvent_t *)PhGetData(info->event);
    rect = PhGetRects(info->event);

	if (x) {
		*x = rect->ul.x;
	}
	if (y) {
		*y = rect->ul.y;
	}
	if (ems) {
		if (ptrevent->key_mods & Pk_KM_Shift)
			*ems |= EV_EMS_SHIFT;
		if (ptrevent->key_mods & Pk_KM_Ctrl)
			*ems |= EV_EMS_CONTROL;
		if (ptrevent->key_mods & Pk_KM_Alt)
			*ems |= EV_EMS_ALT;
	}

	if (emb) {
  		//PHOTON refers to buttons 1,2,3 as the mouse buttons
		//from right to left (biased against right handers!)
		if (ptrevent->buttons & Ph_BUTTON_3)
 			*emb = EV_EMB_BUTTON1;
		else if (ptrevent->buttons & Ph_BUTTON_2)
			*emb = EV_EMB_BUTTON2;
		else if (ptrevent->buttons & Ph_BUTTON_1)
			*emb = EV_EMB_BUTTON3;
	}

	return 0;
}

int AP_QNXLeftRuler::_fe::button_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)data;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	int 				mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, &emb, &mx, &my);

	UT_DEBUGMSG(("LR: Pressing the mouse %x %x %d,%d ", ems, emb, mx, my));
	pQNXLeftRuler->mousePress(ems, emb, mx, my);

	return Pt_CONTINUE;
}

int AP_QNXLeftRuler::_fe::button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)data;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	int 				mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, &emb, &mx, &my);

	if (info->event->subtype == Ph_EV_RELEASE_REAL) {
		UT_DEBUGMSG(("LR: Mouse Real Release! (%d,%d)", mx, my));
	}
	else if (info->event->subtype == Ph_EV_RELEASE_PHANTOM) {
		UT_DEBUGMSG(("LR: Mouse Phantom Release! (%d,%d)", mx, my));
		UT_DEBUGMSG(("LR: Skipping "));
		return Pt_CONTINUE;
	}
	else if (info->event->subtype == Ph_EV_RELEASE_ENDCLICK) {
		UT_DEBUGMSG(("LR: Mouse Endclick Release! (%d,%d)", mx, my));
		UT_DEBUGMSG(("LR: Skipping "));
		return Pt_CONTINUE;
	}
	else {
		UT_DEBUGMSG(("LR: Unknown release type 0x%x (%d,%d)",info->event->subtype, mx, my));
		UT_DEBUGMSG(("LR: Skipping "));
		return Pt_CONTINUE;
	}

	pQNXLeftRuler->mouseRelease(ems, emb, mx, my);

	return Pt_CONTINUE;
}
	
int AP_QNXLeftRuler::_fe::motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)data;

	EV_EditModifierState ems = 0;
	int 				 mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, NULL, &mx, &my);

	pQNXLeftRuler->mouseMotion(ems, mx, my);
	PgFlush();

	return Pt_CONTINUE;
}
		
int AP_QNXLeftRuler::_fe::expose(PtWidget_t * w, PhTile_t * damage)
{
	PtArg_t args[1];
	PhRect_t rect;

	//This will draw the basic widget
	PtSuperClassDraw(PtBasic, w, damage);
	PtBasicWidgetCanvas(w, &rect);

	AP_QNXLeftRuler ** ppQNXRuler = NULL, *pQNXRuler = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXRuler, 0);
	PtGetResources(w, 1, args);
	pQNXRuler = (ppQNXRuler) ? *ppQNXRuler : NULL;

	if (!pQNXRuler)
		return 0;

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

		/*
        printf("Clip to Rect %d,%d %d/%d \n",
            rClip.left, rClip.top, rClip.width, rClip.height);
        pQNXRuler->draw(&rClip);
		*/
		pQNXRuler->draw(NULL);
    }
#else
		pQNXRuler->draw(NULL);
#endif

	return 0;
}
	
int AP_QNXLeftRuler::_fe::resize(PtWidget_t* w, void *data,  PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	// a static function
	AP_QNXLeftRuler * pQNXLeftRuler = (AP_QNXLeftRuler *)data;

	if (pQNXLeftRuler) {
		UT_uint32 iHeight, iWidth, *piBWidth;

		//TODO: We should probably just measure the proper widget.

		//Do this since this size is the size of the group not the widget
		PtGetResource(w, Pt_ARG_BORDER_WIDTH, &piBWidth, sizeof(piBWidth)); 

		iWidth = cbinfo->new_size.lr.x - cbinfo->new_size.ul.x - (2 * *piBWidth); 
		iHeight = cbinfo->new_size.lr.y - cbinfo->new_size.ul.y - (2 * *piBWidth);

		UT_DEBUGMSG(("LR: Resize to %d,%d %d,%d [%dx%d] Border %d ",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y,
			iWidth, iHeight, *piBWidth));
	
		if (iHeight != pQNXLeftRuler->getHeight()) {
			pQNXLeftRuler->setHeight(iHeight);
		}
	
		if (iWidth != pQNXLeftRuler->getWidth()) {
			pQNXLeftRuler->setWidth(iWidth);
		}
	}

	return Pt_CONTINUE;
}

