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
#include "xap_QNXFrameImpl.h"
#include "gr_QNXGraphics.h"
#include "ap_QNXTopRuler.h"
#include "fv_View.h"
#include <stdio.h>

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)


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
	int n = 0;
	UT_ASSERT(!m_pG && !m_wTopRuler);

	XAP_QNXFrameImpl *pQNXFrameImpl = (XAP_QNXFrameImpl *)m_pFrame->getFrameImpl();
	m_rootWindow = pQNXFrameImpl->getTopLevelWindow();

	area.pos.x = 0;
	area.pos.y = pQNXFrameImpl->m_AvailableArea.pos.y;
	area.size.w = pQNXFrameImpl->m_AvailableArea.size.w;
	area.size.h = s_iFixedHeight;

	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
#define _TR_ANCHOR_     (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
                         Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_TOP)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _TR_ANCHOR_, _TR_ANCHOR_);
#define _TR_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, _TR_STRETCH_, _TR_STRETCH_); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED|Pt_DELAY_REALIZE, Pt_HIGHLIGHTED|Pt_DELAY_REALIZE);
	m_wTopRulerGroup = PtCreateWidget(PtGroup, m_rootWindow, n, args);
	PtAddCallback(m_wTopRulerGroup, Pt_CB_RESIZE, _fe::resize, this);


	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1);
	PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0); 
   PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_FALSE, Pt_GETS_FOCUS); 
	m_wTopRuler = PtCreateWidget(PtRaw, m_wTopRulerGroup, n, args);
	PtAddEventHandler(m_wTopRuler, Ph_EV_PTR_MOTION_BUTTON /* Ph_EV_PTR_MOTION */, 
								  _fe::motion_notify_event, this);
	PtAddEventHandler(m_wTopRuler, Ph_EV_BUT_PRESS, _fe::button_press_event, this);
	PtAddEventHandler(m_wTopRuler, Ph_EV_BUT_RELEASE, _fe::button_release_event, this);

	DELETEP(m_pG);	
	//GR_QNXGraphics * pG = new GR_QNXGraphics(((XAP_QNXFrameImpl *)m_pFrame->getFrameImpl())->getTopLevelWindow(), m_wTopRuler,  ((XAP_QNXFrameImpl *)m_pFrame->getFrameImpl())->getFrame()->getApp());

	GR_QNXAllocInfo ai(((XAP_QNXFrameImpl *)m_pFrame->getFrameImpl())->getTopLevelWindow(),
					   m_wTopRuler,
					   ((XAP_QNXFrameImpl *)m_pFrame->getFrameImpl())->getFrame()->getApp());
	GR_QNXGraphics *pG = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	m_pG = pG;
	UT_ASSERT(m_pG);
	pG->init3dColors();

	return m_wTopRulerGroup;
}

void AP_QNXTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);
}

void AP_QNXTopRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(0);
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

	//XXX:
return NULL;
}

		
/*****************************************************************/
static int get_stuff(PtCallbackInfo_t *info, EV_EditModifierState *ems, 
									EV_EditMouseButton *emb, int *x, int *y) {
	PhPointerEvent_t *ptrevent;
    PhRect_t         *rect;

	ptrevent = (PhPointerEvent_t *)PhGetData(info->event);
    rect = PhGetRects(info->event);

	if (x) {
		*x =rect->ul.x;
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

int AP_QNXTopRuler::_fe::button_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)data;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	int 				mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, &emb, &mx, &my);

	UT_DEBUGMSG(("TR: Pressing the mouse %x %x %d,%d ",
				ems, emb, mx, my));
	pQNXTopRuler->mousePress(ems, emb, pQNXTopRuler->getGraphics()->tlu(mx), pQNXTopRuler->getGraphics()->tlu(my));

	return Pt_CONTINUE;
}

int AP_QNXTopRuler::_fe::button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)data;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	int 				mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, &emb, &mx, &my);

	if (info->event->subtype == Ph_EV_RELEASE_REAL) {
		UT_DEBUGMSG(("TR: Mouse Real Release! (%d,%d)", mx, my));
	}
	else if (info->event->subtype == Ph_EV_RELEASE_PHANTOM) {
		UT_DEBUGMSG(("TR: Mouse Phantom Release! (%d,%d)", mx, my));
		UT_DEBUGMSG(("TR: Skipping "));
		return Pt_CONTINUE;
	}
	else if (info->event->subtype == Ph_EV_RELEASE_ENDCLICK) {
		UT_DEBUGMSG(("TR: Mouse Endclick Release! (%d,%d)", mx, my));
		UT_DEBUGMSG(("TR: Skipping "));
		return Pt_CONTINUE;
	}
	else {
		UT_DEBUGMSG(("TR: Unknown release type 0x%x (%d,%d)",info->event->subtype, mx, my));
		UT_DEBUGMSG(("TR: Skipping "));
		return Pt_CONTINUE;
	}

	pQNXTopRuler->mouseRelease(ems, emb,  pQNXTopRuler->getGraphics()->tlu(mx),  pQNXTopRuler->getGraphics()->tlu(my));

	return Pt_CONTINUE;
}
	
int AP_QNXTopRuler::_fe::motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{

	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)data;

	EV_EditModifierState ems = 0;
	int 				 mx, my;
	
	mx = my = 0;
	get_stuff(info, &ems, NULL, &mx, &my);

	pQNXTopRuler->mouseMotion(ems,  pQNXTopRuler->getGraphics()->tlu(mx),  pQNXTopRuler->getGraphics()->tlu(my));

	return Pt_CONTINUE;
}
	
int AP_QNXTopRuler::_fe::resize(PtWidget_t* w, void *data,  PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	// a static function
	AP_QNXTopRuler * pQNXTopRuler = (AP_QNXTopRuler *)data;

	if (pQNXTopRuler) {
		UT_uint32 iHeight, iWidth, *piBWidth;

		//TODO: We should probably just measure the proper widget.

		//Do this since this size is the size of the group not the widget
		PtGetResource(w, Pt_ARG_BORDER_WIDTH, &piBWidth, sizeof(piBWidth)); 

		iWidth = cbinfo->new_size.lr.x - cbinfo->new_size.ul.x - (2 * *piBWidth); 
		iHeight = cbinfo->new_size.lr.y - cbinfo->new_size.ul.y - (2 * *piBWidth);

		UT_DEBUGMSG(("TR: Resize to %d,%d %d,%d [%dx%d] Border %d ",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y,
			iWidth, iHeight, *piBWidth));
	
		if (iHeight != pQNXTopRuler->getHeight()) {
			pQNXTopRuler->setHeight(iHeight);
		}
	
		if (iWidth != pQNXTopRuler->getWidth()) {
			pQNXTopRuler->setWidth(iWidth);
		}
	}

	return Pt_CONTINUE;
}

int AP_QNXTopRuler::_fe::expose(PtWidget_t * w, PhTile_t *damage)
{
	PhRect_t rect;

	PtCalcCanvas(w, &rect);

	AP_QNXTopRuler *pQNXRuler = NULL;
	PtGetResource(w, Pt_ARG_POINTER,&pQNXRuler,0);
	FV_View * pView = (FV_View *) pQNXRuler->getView();

	if (!pQNXRuler || !pView) {
		return 0;
	}
			// HACK for not updating Ruler for incremental Loading
			if(pView && (pView->getPoint() == 0))
			{
				return 0;
			}
				UT_Rect rClip;
				PhPoint_t shift;
				PtWidgetOffset(w, &shift);

				GR_Graphics * pGr = pQNXRuler->getGraphics();
				rClip.width = pGr->tlu(damage->rect.lr.x - damage->rect.ul.x);
				rClip.height = pGr->tlu(damage->rect.lr.y - damage->rect.ul.y);
				rClip.left = pGr->tlu((damage->rect.ul.x - shift.x) > 0 ? damage->rect.ul.x - shift.x : 0 );
				rClip.top = pGr->tlu((damage->rect.ul.y - shift.y) > 0 ? damage->rect.ul.y - shift.y : 0);

				pQNXRuler->draw(&rClip);
	return 0;
}

bool AP_QNXTopRuler::notify(AV_View * pView, const AV_ChangeMask mask)
{


	// if the column containing the caret has changed or any
	// properties on the section (like the number of columns
	// or the margins) or on the block (like the paragraph
	// indents), then we redraw the ruler.
//#bidi
	if (mask & (AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK | AV_CHG_HDRFTR ))
	{
/*		UT_Rect pClipRect;
		pClipRect.top = 0;
		pClipRect.left = UT_MAX(m_iLeftRulerWidth, s_iFixedWidth);
		FV_View * pView = static_cast<FV_View *>(m_pView);
		if(pView->getViewMode() != VIEW_PRINT)
		{
			pClipRect.left = 0;
		}

		pClipRect.height = m_iHeight;
		pClipRect.width = m_iWidth;
		draw(&pClipRect);*/
		PtDamageWidget(m_wTopRuler);

	}

	return true;
}

