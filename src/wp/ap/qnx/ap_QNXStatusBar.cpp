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
#include "ap_QNXStatusBar.h"
#include "ut_qnxHelper.h"
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_QNXStatusBar::AP_QNXStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_pG = NULL;
}

AP_QNXStatusBar::~AP_QNXStatusBar(void)
{
	DELETEP(m_pG);
}

void AP_QNXStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.
	DELETEP(m_pG);	
	XAP_QNXApp * app;
	app = (XAP_QNXApp *)m_pFrame->getApp();
	UT_ASSERT(app);
	XAP_QNXFrame *frame = (XAP_QNXFrame *) m_pFrame;
	UT_ASSERT(frame);

	m_pG = new GR_QNXGraphics(frame->getTopLevelWindow(), m_wStatusBar, m_pFrame->getApp());
	UT_ASSERT(m_pG);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
}

PtWidget_t * AP_QNXStatusBar::createWidget(void)
{
	PhArea_t	area;
	PtArg_t 	args[10];
	int 		n;

	XAP_QNXFrame *frame = (XAP_QNXFrame *) m_pFrame;

	UT_ASSERT(!m_pG && !m_wStatusBar);

	m_wStatusBar = NULL;

	/* Create a group and then attach it to the bottom */
	UT_QNXGetWidgetArea(frame->getTopLevelWindow(), NULL, NULL, &area.size.w, &area.size.h);
	area.pos.x = 0;
	area.pos.y = area.size.h - 24;
	area.size.h = 24;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	//PtSetArg(&args[n++], Pt_ARG_FILL_COLOR, Pg_YELLOW, 0);
	PtSetArg(&args[n++], Pt_ARG_BORDER_WIDTH, 2, 0);
#define _SB_ANCHOR_     (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
                   		 Pt_TOP_ANCHORED_BOTTOM | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _SB_ANCHOR_, _SB_ANCHOR_);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_FILL, Pt_GROUP_STRETCH_FILL);
	m_wStatusBarGroup = PtCreateWidget(PtGroup, frame->getTopLevelWindow(), n, args);
	PtAddCallback(m_wStatusBarGroup, Pt_CB_RESIZE, &(_fe::resize), this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0); 
	PtSetArg(&args[n++], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1);
	void *data = this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this));
    PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); 
	m_wStatusBar = PtCreateWidget(PtRaw, m_wStatusBarGroup, n, args);


	UT_ASSERT(m_wStatusBar);
	return m_wStatusBar;
}

void AP_QNXStatusBar::show(void) {
    UT_ASSERT(m_wStatusBarGroup);
    PtRealizeWidget(m_wStatusBarGroup);
}

void AP_QNXStatusBar::hide(void) {
    UT_ASSERT(m_wStatusBarGroup);
    PtUnrealizeWidget(m_wStatusBarGroup);
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int AP_QNXStatusBar::_fe::resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	// a static function
	AP_QNXStatusBar * pQNXStatusBar = (AP_QNXStatusBar *)data;
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	if (pQNXStatusBar) {
		UT_uint32 iHeight, iWidth;

		iWidth = cbinfo->new_size.lr.x - cbinfo->new_size.ul.x; 
		iHeight = cbinfo->new_size.lr.y - cbinfo->new_size.ul.y;

		pQNXStatusBar->setHeight(iHeight);
		pQNXStatusBar->setWidth(iWidth);
	}	
	return Pt_CONTINUE;
}
	
int AP_QNXStatusBar::_fe::expose(PtWidget_t * w, PhTile_t *damage)
{
	PtArg_t args[1];
	PhRect_t rect;

	PtSuperClassDraw(PtBasic, w, damage);
	PtBasicWidgetCanvas(w, &rect);

	AP_QNXStatusBar ** ppQNXStatusBar = NULL, *pQNXStatusBar = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXStatusBar, 0);
	PtGetResources(w, 1, args);
	pQNXStatusBar = (ppQNXStatusBar) ? *ppQNXStatusBar : NULL;

	if (!pQNXStatusBar) {
		return Pt_CONTINUE;
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

        UT_DEBUGMSGr(("Clip to Rect %d,%d %d/%d ",
            rClip.left, rClip.top, rClip.width, rClip.height));
		pQNXRuler->draw(&rClip);
		//pQNXRuler->draw(NULL);
    }
#else
	pQNXStatusBar->draw();
#endif

	return Pt_CONTINUE;
}


