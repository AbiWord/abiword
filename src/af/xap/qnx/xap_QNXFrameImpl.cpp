/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Johan Björk 
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

#include <Pt.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_sleep.h"
#include "xap_ViewListener.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "ev_QNXKeyboard.h"
#include "ev_QNXMouse.h"
#include "ev_QNXToolbar.h"
#include "ev_EditMethod.h"
#include "gr_QNXGraphics.h"
#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "xad_Document.h"
#include "gr_Graphics.h"
#include "ie_imp.h"
#include "ap_FrameData.h"
#include "xap_Frame.h"
#include "ut_qnxHelper.h"
#include "xap_Prefs.h"

XAP_QNXFrameImpl::XAP_QNXFrameImpl(XAP_Frame *pFrame,XAP_QNXApp *pApp) : 
	XAP_FrameImpl(pFrame),
	m_bDoZoomUpdate(false),
	m_pZoomUpdateID(0),
	m_iAbiRepaintID(0),
	m_pQNXApp(pApp),
	m_pQNXMenu(NULL),
	m_pQNXPopup(NULL),
	m_dialogFactory(pFrame,static_cast<XAP_App *>(pApp))
{
}

XAP_QNXFrameImpl::~XAP_QNXFrameImpl()
{
}

int XAP_QNXFrameImpl::_fe::focus_in_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) 
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame *pFrame = pFrameImpl->getFrame();

	UT_ASSERT(pFrame);
        if (pFrame->getCurrentView())
            pFrame->getCurrentView()->focusChange(AV_FOCUS_HERE/*AV_FOCUS_NEARBY*/);
					
	return Pt_CONTINUE;
}

int XAP_QNXFrameImpl::_fe::focus_out_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) 
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	UT_ASSERT(pFrame);

	if(pFrame->getCurrentView())	
		pFrame->getCurrentView() ->focusChange(AV_FOCUS_NONE);
	return Pt_CONTINUE;
}


int XAP_QNXFrameImpl::_fe::button_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();


	if (pView)
		pQNXMouse->mouseClick(pView,info);
	return Pt_END;
}

int XAP_QNXFrameImpl::_fe::button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();

	if (pView)
		pQNXMouse->mouseUp(pView,info);
	return Pt_END;
}

int XAP_QNXFrameImpl::_fe::motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();	
	AV_View * pView = pFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();

	//Photon compresses events, ie, we will get called with the last collected motion event. (unix/gtk needs special handling for that)

	if (pView)
		pQNXMouse->mouseMotion(pView, info);
	
	return Pt_END;
}
	
int XAP_QNXFrameImpl::_fe::key_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	ev_QNXKeyboard * pQNXKeyboard = (ev_QNXKeyboard *) pFrame->getKeyboard();
	EV_QNXMouse	*pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();
	PhKeyEvent_t *kev = (PhKeyEvent_t *)PhGetData(info->event);
		
	if (pView) {
		if((kev->key_cap == Pk_Up || kev->key_cap == Pk_Down )&& kev->key_scan == 0 && (kev->key_flags & Pk_KF_Scan_Valid)) //wheelmouse UP
			pQNXMouse->mouseScroll(pView,info);

		pQNXKeyboard->keyPressEvent(pView, info);
	}
	info->event->processing_flags |= Ph_NOT_CUAKEY;
	return Pt_CONTINUE;
}
	
int XAP_QNXFrameImpl::_fe::resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	//FV_View * pfView = static_cast<FV_View*>(pView);

		UT_DEBUGMSG(("Document Area Resizing to %d,%d %d,%d",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
		pFrameImpl->m_iNewWidth = cbinfo->new_dim.w;
		pFrameImpl->m_iNewHeight = cbinfo->new_dim.h;
		// Dynamic Zoom Implimentation

	if(!pFrameImpl->m_bDoZoomUpdate && (pFrameImpl->m_pZoomUpdateID == 0))
		{
		UT_DEBUGMSG(("Starting do_ZoomUpdate workproc."));
		pFrameImpl->m_pZoomUpdateID = PtAppAddWorkProc(NULL,do_ZoomUpdate,pFrameImpl);	

		}
	return Pt_CONTINUE;
}

int XAP_QNXFrameImpl::_fe::do_ZoomUpdate(void * /*XAP_QNXFrameImpl * */ p)
{
	XAP_QNXFrameImpl * pQNXFrameImpl = static_cast<XAP_QNXFrameImpl *>(p);
	XAP_Frame* pFrame = pQNXFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	if(!pView || pFrame->isFrameLocked() ||
	(pQNXFrameImpl->m_bDoZoomUpdate && (pView->getGraphics()->tdu(pView->getWindowWidth()) == pQNXFrameImpl->m_iNewWidth) && (pView->getGraphics()->tdu(pView->getWindowHeight()) == pQNXFrameImpl->m_iNewHeight)))
	{
		pQNXFrameImpl->m_pZoomUpdateID = 0;
		pQNXFrameImpl->m_bDoZoomUpdate = false;
		return Pt_END;
	}
	pQNXFrameImpl->m_bDoZoomUpdate = true;
	UT_sint32 iNewWidth = 0;
	UT_sint32 iNewHeight = 0;
	do
	{
		AV_View * pView = pFrame->getCurrentView();
		if(!pView)
		{
			pQNXFrameImpl->m_pZoomUpdateID = 0;
			pQNXFrameImpl->m_bDoZoomUpdate = false;
			return Pt_END;
		}
		while(pView->isLayoutFilling())
		{
//
// Comeback when it's finished.
//
			return Pt_CONTINUE;
		}
		iNewWidth = pQNXFrameImpl->m_iNewWidth;
		iNewHeight = pQNXFrameImpl->m_iNewHeight;
		pView = pFrame->getCurrentView();
		if(pView)
		{
			/* Not needed for QuickZoom 
			pQNXFrameImpl->_startViewAutoUpdater(); 
			*/
			pView->setWindowSize(iNewWidth, iNewHeight);
			pFrame->quickZoom();
//			PtFlush();
		}
		else
		{
			pQNXFrameImpl->m_pZoomUpdateID = 0;
			pQNXFrameImpl->m_bDoZoomUpdate = false;
			return Pt_END; 
		}
	}
	while((iNewWidth != pQNXFrameImpl->m_iNewWidth) || (iNewHeight != pQNXFrameImpl->m_iNewHeight));

	pQNXFrameImpl->m_pZoomUpdateID = 0;
	pQNXFrameImpl->m_bDoZoomUpdate = false;
	return Pt_END;

}


int XAP_QNXFrameImpl::_fe::window_resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	if(pFrame) {
		XAP_App	*pApp	=pFrame->getApp();
	if (pApp) {
		UT_DEBUGMSG(("Window Resizing to %d,%d %d,%d ",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
		pApp->setGeometry(-1, -1, 
						   cbinfo->new_dim.w, 
				           cbinfo->new_dim.h);
	}
	}

return Pt_CONTINUE;
}
		
int XAP_QNXFrameImpl::_fe::window_delete(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	PhWindowEvent_t *winevent = (PhWindowEvent_t *)info->cbdata;

	if(!winevent || winevent->event_f != Ph_WM_CLOSE) {
		return Pt_CONTINUE;
	}
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
	UT_ASSERT(pEM);

	if (pEM)
	{
		if (pEM->Fn(pFrame->getCurrentView(),NULL))
		{
			//Destroy this window.
			PtDestroyWidget(w);
		}
	}
		
	//Do not destroy this window.
	return Pt_CONTINUE;
}
	
int XAP_QNXFrameImpl::_fe::expose(PtWidget_t * w, PhTile_t * damage)
{
	PtArg_t args[1];
	UT_Rect rClip;
	PhRect_t rect;
 	PhPoint_t pnt;


 	PtCalcCanvas(w, &rect);
 	PtWidgetOffset(w, &pnt);

	XAP_FrameImpl *pQNXFrameImpl;
	PtGetResource(w,Pt_ARG_POINTER, &pQNXFrameImpl,0);

	UT_ASSERT(pQNXFrameImpl);

	
	FV_View * pView = (FV_View *) pQNXFrameImpl->getFrame()->getCurrentView();
	if (pView) {
		/*
   		 The first damage rect spans all of the damage areas, so
   		 do it piecewise if we can over the areas that need it.

		 The damage rectangles are provided in the windows'
		 co-ordinates, so make sure to translate them to the
		 widgets co-ordinates when passing to our draw() functions
		*/
/*
 When Abi's Draw routine gets to be faster about not going through
 and calculating areas which don't need to be done, then we can
 actually take advantage of the multiple clip areas.  For now though
 we just do it based on the first clip.
#define MULTIPLE_EXPOSE_EVENTS
*/
		if (damage->next) {
#if defined(MULTIPLE_EXPOSE_EVENTS) 
			damage = damage->next;
#endif
		}
		while (damage) {
			/* At one point in time this required some fiddling to put it in the widget co-ordinates*/
			GR_Graphics * pGr = pView->getGraphics ();
			rClip.width = pGr->tlu(damage->rect.lr.x - damage->rect.ul.x);
			rClip.height = pGr->tlu(damage->rect.lr.y - damage->rect.ul.y);
			rClip.left = pGr->tlu((damage->rect.ul.x - pnt.x) > 0 ? damage->rect.ul.x - pnt.x : 0 );
			rClip.top = pGr->tlu((damage->rect.ul.y - pnt.y) > 0 ? damage->rect.ul.y - pnt.y : 0);

			//OR: Pass the draw function the clipping rectangle
			//This is preferred since this way the application
			//can optimize their drawing routines as well.

			pView->draw(&rClip);

#if defined(MULTIPLE_EXPOSE_EVENTS) 
			damage = damage->next;
#else
			break;
#endif
		}

	}
	return Pt_END;
}
	
int XAP_QNXFrameImpl::_fe::vScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();

	AV_View * pView = pFrame->getCurrentView();
	
	if (pView)
		pView->sendVerticalScrollEvent((UT_sint32)sb->position);
	return Pt_END;
}
	
int XAP_QNXFrameImpl::_fe::hScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	
	if (pView)
		pView->sendHorizontalScrollEvent((UT_sint32) sb->position);
	return Pt_END;
}

//QNX DnD
static PtDndFetch_t acceptdata[] = {
	{"text","plain",Ph_TRANSPORT_INLINE,},
};

enum {
PLAIN_TEXT = 0,
};

int XAP_QNXFrameImpl::_fe::dnd(PtWidget_t *w,void *data,PtCallbackInfo_t *info)
{

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	XAP_App * pApp = pFrame->getApp();
PtDndCallbackInfo_t *dc = (PtDndCallbackInfo_t *)info->cbdata;
IE_Imp *pImp = 0;


	if(info->reason_subtype == Ph_EV_DND_ENTER)
	{
		PtDndSelect(w,acceptdata,1,0,0,info);
		return Pt_CONTINUE;
	}


if(info->reason_subtype == Ph_EV_DND_DROP)
{
	switch(dc->fetch_index)
	{
		case PLAIN_TEXT:
			AP_FrameData *pFrameData = (AP_FrameData*)pFrame->getFrameData();
			FL_DocLayout *pDocLy = pFrameData->m_pDocLayout;
			FV_View *pView = pDocLy->getView();
			PD_DocumentRange dr(pView->getDocument(),pView->getPoint(),pView->getPoint());
	
			IE_Imp::constructImporter(dr.m_pDoc,IE_Imp::fileTypeForSuffix(".txt"),&pImp,0);
			if(pImp)
			{
				const char * szEncoding = pApp->getDefaultEncoding();
				pImp->pasteFromBuffer(&dr,(unsigned char *)dc->data,strlen((const char*)dc->data),szEncoding);
				delete pImp;
				pView->_generalUpdate();
			}
				break;
	
		}
}

return Pt_CONTINUE;
}	

bool XAP_QNXFrameImpl::_updateTitle()
{
	if (!XAP_FrameImpl::_updateTitle() || (m_wTopLevelWindow == NULL) || (m_iFrameMode != XAP_NormalFrame))
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pQNXApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getFrame()->getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);

	PtArg_t args[1];

	PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, buf, 0); 
	PtSetResources(getTopLevelWindow(), 1, args);
	
	return true;
}

void XAP_QNXFrameImpl::_initialize()
{

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_QNXKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_QNXMouse(pEEM);
	UT_ASSERT(m_pMouse);

}

void XAP_QNXFrameImpl::_nullUpdate() const
{
      #define EVENT_SIZE sizeof(PhEvent_t) + 1000
	PhDrawContext_t *dc;
	UT_uint32 i =0;
	dc = PhDCGetCurrent();


	if(dc->type == Ph_DRAW_TO_PRINT_CONTEXT) {
		return;
	}

	PhEvent_t *event = (PhEvent_t*)g_try_malloc(EVENT_SIZE);
        if (!event) return;

	while(i < 5)
	{
		switch(PhEventPeek(event, EVENT_SIZE))
		{
		case Ph_EVENT_MSG:
			PtEventHandler( event );
			break;
		case -1:
			perror( "PhEventPeek failed" );
			break;
		}
		i++;
	}

	g_free(event);
}

void XAP_QNXFrameImpl::_setCursor(GR_Graphics::Cursor c)
{
unsigned short cursor_number=0;

if(getTopLevelWindow() == NULL || (m_iFrameMode != XAP_NormalFrame))
	return;
	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_Graphics::GR_CURSOR_DEFAULT:
		cursor_number = Ph_CURSOR_POINTER;
		break;

	case GR_Graphics::GR_CURSOR_IBEAM:
		cursor_number = Ph_CURSOR_POINTER; //XXX: Wtf is IBEAM ?
		break;

	case GR_Graphics::GR_CURSOR_RIGHTARROW:
		cursor_number = Ph_CURSOR_DRAG_RIGHT;
		break;

	case GR_Graphics::GR_CURSOR_LEFTARROW:
		cursor_number = Ph_CURSOR_DRAG_LEFT; //GDK_ARROW;
		break;

	case GR_Graphics::GR_Graphics::GR_CURSOR_IMAGE:
		cursor_number = Ph_CURSOR_POINTER; //XXX: ???
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NW:
		cursor_number = Ph_CURSOR_DRAG_TL;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_N:
		cursor_number = Ph_CURSOR_DRAG_TOP;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NE:
		cursor_number = Ph_CURSOR_DRAG_TR;
		break;

	case GR_Graphics::GR_Graphics::GR_CURSOR_IMAGESIZE_E:
		cursor_number = Ph_CURSOR_DRAG_RIGHT;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SE:
		cursor_number = Ph_CURSOR_DRAG_BR;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_S:
		cursor_number = Ph_CURSOR_DRAG_BOTTOM;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SW:
		cursor_number = Ph_CURSOR_DRAG_BL;
		break;

	case GR_Graphics::GR_Graphics::GR_CURSOR_IMAGESIZE_W:
		cursor_number = Ph_CURSOR_DRAG_LEFT;
		break;

	case GR_Graphics::GR_CURSOR_LEFTRIGHT:
		cursor_number=Ph_CURSOR_DRAG_HORIZONTAL;
		break;
	case GR_Graphics::GR_CURSOR_UPDOWN:
		cursor_number=Ph_CURSOR_DRAG_VERTICAL;
		break;
	case GR_Graphics::GR_CURSOR_EXCHANGE:
		cursor_number=Ph_CURSOR_POINTER;
		break;
	case GR_Graphics::GR_CURSOR_GRAB:
		cursor_number=Ph_CURSOR_MOVE;
		break;
	case GR_Graphics::GR_CURSOR_LINK:
		cursor_number=Ph_CURSOR_FINGER;
		break;
	case GR_Graphics::GR_CURSOR_WAIT:
		cursor_number= Ph_CURSOR_WAIT;
		break;
	case GR_Graphics::GR_CURSOR_CROSSHAIR:
		cursor_number = Ph_CURSOR_CROSSHAIR;
		break;
	}

	PtSetResource(getTopLevelWindow(),Pt_ARG_CURSOR_TYPE,cursor_number,0);
}


UT_sint32 XAP_QNXFrameImpl::_setInputMode(const char *szName)
{
	XAP_Frame*	pFrame = getFrame();
	UT_sint32 result = XAP_App::getApp()->setInputMode(szName);

	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;

}

PtWidget_t *XAP_QNXFrameImpl::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

XAP_DialogFactory *XAP_QNXFrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_QNXFrameImpl::createTopLevelWindow(void)
{
	bool bResult;
	PtArg_t args[15];
	int 	n;
	UT_uint32	w, h;
	PhArea_t area;
	PhDim_t minsize = {200,200};

#define INIT_WIDTH 500
#define INIT_HEIGHT 400
	/*** Create the main window ***/
	//ndim.w = m_geometry.width; ndim.h = m_geometry.height;
	area.pos.x = 0; area.pos.y = 0;
	area.size.w = INIT_WIDTH; area.size.h = INIT_HEIGHT;

	//If it is available then get the default geometry
	if(m_pQNXApp && m_pQNXApp->getGeometry(NULL, NULL, &w, &h) == true) {
		area.size.w = w;
		area.size.h = h;
	} 

	m_AvailableArea = area;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, m_pQNXApp->getApplicationTitleForTitleBar(), 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, Ph_WM_HELP, Ph_WM_CLOSE|Ph_WM_HELP);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_NOTIFY_FLAGS, Ph_WM_CLOSE, Ph_WM_CLOSE);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS,Pt_TRUE,Ph_WM_RENDER_HELP);
	PtSetArg(&args[n++], Pt_ARG_FLAGS,Pt_TRUE,Pt_CALLBACKS_ACTIVE);
	PtSetArg(&args[n++], Pt_ARG_CURSOR_OVERRIDE,Pt_TRUE,0);
	PtSetArg(&args[n++], Pt_ARG_MINIMUM_DIM,&minsize,0);

	PtSetParentWidget(NULL);
	m_wTopLevelWindow = PtCreateWidget(PtWindow, NULL /* Use last widget? */, n, args);
	if (!m_wTopLevelWindow) {
		fprintf(stderr, "Can't create top level window \n");
		exit(1);
	}
	//PtAddEventHandler(m_wTopLevelWindow, Ph_EV_KEY, _fe::key_press_event, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_GOT_FOCUS, _fe::focus_in_event, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_LOST_FOCUS, _fe::focus_out_event, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_RESIZE, _fe::window_resize, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_WINDOW, _fe::window_delete, this);


	/* TODO: Menu and the Toolbars all go into the same Toolbar "group" */
	n = 0;
#define _A_TBGRP (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | Pt_TOP_ANCHORED_TOP)
    PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _A_TBGRP, _A_TBGRP);
    PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, 0, Pt_RESIZE_X_BITS);
    PtSetArg(&args[n++], Pt_ARG_WIDTH, area.size.w, 0); 
		m_wTBGroup = PtCreateWidget(PtToolbarGroup, m_wTopLevelWindow, n, args);

	/*** Create the menu bars ***/
	m_pQNXMenu = new EV_QNXMenuBar(m_pQNXApp, getFrame(), m_szMenuLayoutName, m_szMenuLabelSetName);
	UT_ASSERT(m_pQNXMenu);
	bResult = m_pQNXMenu->synthesizeMenuBar();
	UT_ASSERT(bResult);

	/*** Create the tool bars ***/
	_createToolbars();

	/*** Figure out the height to adjust by ***/
	unsigned short tbheight;	
	UT_QNXGetWidgetArea(m_wTBGroup, NULL, NULL, NULL, &tbheight);
#define PHOTON_TOOLBAR_EXTENT_WEIRDNESS
#if defined(PHOTON_TOOLBAR_EXTENT_WEIRDNESS)
	tbheight += 4; 	
#endif
	m_AvailableArea.pos.y += tbheight;
	m_AvailableArea.size.h -= tbheight;

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	
	/*** Add the document view ***/	
	m_wSunkenBox = _createDocumentWindow();

	//Add status bars
	m_wStatusBar = _createStatusBarWindow();
	
	//Set the icon for the window
	_setWindowIcon();

	// Set geometry 
	int x,y;
	UT_uint32 width,height;
	UT_uint32 f;

	m_pQNXApp->getWinGeometry(&x,&y,&width,&height,&f);
	
	//Get fall-back defaults from pref
	UT_uint32 pref_flags, pref_width, pref_height;
	UT_sint32 pref_x, pref_y;
	pref_flags = pref_width = pref_height = pref_x = pref_y = 0;
	m_pQNXApp->getPrefs()->getGeometry(&pref_x, &pref_y, &pref_width, &pref_height, &pref_flags);
	if (!(f & XAP_QNXApp::GEOMETRY_FLAG_SIZE)
	&& (pref_flags & PREF_FLAG_GEOMETRY_SIZE))
	{
		width = pref_width;
		height = pref_height;
		f |= XAP_QNXApp::GEOMETRY_FLAG_SIZE;
	}
	if (!(f & XAP_QNXApp::GEOMETRY_FLAG_POS)
		&& (pref_flags & PREF_FLAG_GEOMETRY_POS))
	{
		x = pref_x;
		y = pref_y;
	f |= XAP_QNXApp::GEOMETRY_FLAG_POS;
	}
// Set the size if requested.
	if(f & XAP_QNXApp::GEOMETRY_FLAG_SIZE)
	{
			PhDim_t dim;
			dim.w = width;
			dim.h = height;
		
			PtSetResource(m_wTopLevelWindow,Pt_ARG_DIM,&dim,0);		
	}
	//Only set the pos on the first window.
	if(m_pQNXApp->getFrameCount() <= 1)
	{
		if( f & XAP_QNXApp::GEOMETRY_FLAG_POS)
		{
			PhPoint_t pos;
			pos.x=x;
			pos.y=y;
			PtSetResource(m_wTopLevelWindow,Pt_ARG_POS,&pos,0);
		}
	}
	//Remember the settings for next time
	m_pQNXApp->getPrefs()->setGeometry(x,y,width,height,f);


	PgSetDrawBufferSize(16*1024);
return;
}

void XAP_QNXFrameImpl::_setGeometry()
{

}

void XAP_QNXFrameImpl::_rebuildMenus(void)
{
}

void XAP_QNXFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	XAP_Frame*	pFrame = getFrame();
	
	pFrame->refillToolbarsInFrameData();
	pFrame->repopulateCombos();
}

bool XAP_QNXFrameImpl::_close()
{
	PtDestroyWidget(getTopLevelWindow());
	return true;
}

bool XAP_QNXFrameImpl::_raise()
{
	if (getTopLevelWindow())
		PtWindowToFront(getTopLevelWindow());
	return true;
}

bool XAP_QNXFrameImpl::_show()
{
	_raise();
	return true;
}

bool XAP_QNXFrameImpl::_runModalContextMenu(AV_View *pView,const char *szMenuName,
	UT_sint32 x,UT_sint32 y)
{
	GR_Graphics * pGr = pView->getGraphics ();
	x = pGr->tdu(x);
	y = pGr->tdu(y);
	bool bResult = true;
	UT_ASSERT(!m_pQNXPopup);

	setPopupDone(0);	
	m_pQNXPopup = new EV_QNXMenuPopup(m_pQNXApp, getFrame(), 
									  szMenuName, 
									  m_szMenuLabelSetName);
	if (m_pQNXPopup && m_pQNXPopup->synthesizeMenuPopup())
	{
		PtArg_t	arg;
		PhPoint_t pos;

		PtWidget_t *menu = m_pQNXPopup->getMenuHandle();

		memset(&pos, 0, sizeof(pos));
		PtGetAbsPosition(m_wSunkenBox, &pos.x, &pos.y);
		pos.x += x; pos.y += y;

		PtSetArg(&arg, Pt_ARG_POS, &pos, 0);
		PtSetResources(menu, 1, &arg);
		PtRealizeWidget(menu);

		//Really we need to run this synchronously ... or at least
		//be able to provide some sort of handler that blocks the
		//window and then unblocks it when we are finished with 
		//the menu. This is why we do the "getPopupDone" test, set by the menu
		int level;
		level = PtModalStart();
		while (getPopupDone() == 0) {
			PtProcessEvent();
		}	
		PtModalEnd(MODAL_END_ARG(level));
	}
	DELETEP(m_pQNXPopup);

	return bResult;
}

void XAP_QNXFrameImpl::_setFullScreen(bool changeToFullScreen)
{
PtWidget_t *app = getTopLevelWindow();
static PhArea_t pharea;
static unsigned short phflags;

PhArea_t *area,area2;
unsigned short *flags;
PgDisplaySettings_t disp;

if(changeToFullScreen == false)
{
	PtSetResource(app,Pt_ARG_AREA,&pharea,0);	
	PtSetResource(app,Pt_ARG_WINDOW_RENDER_FLAGS,Pt_TRUE,phflags);
}
else
{
	PtGetResource(app,Pt_ARG_WINDOW_RENDER_FLAGS,&flags,0);
	phflags = *flags;
	PtSetResource(app,Pt_ARG_WINDOW_RENDER_FLAGS,Pt_FALSE,Pt_TRUE);
	PtGetResource(app,Pt_ARG_AREA,&area,0);

	area2.pos.x = area2.pos.y=0;
	PgGetVideoMode(&disp);
	area2.size.w=disp.xres;
	area2.size.h=disp.yres;
	pharea = *area;
	PtSetResource(app,Pt_ARG_AREA,&area2,0);
}
}

EV_Toolbar *XAP_QNXFrameImpl::_newToolbar(XAP_App *pApp,XAP_Frame *pFrame,
	const char *szLayout,
	const char *szLanguage)
{
	return (new EV_QNXToolbar((XAP_QNXApp *)(pApp), 
							  pFrame, szLayout, szLanguage));
}

EV_Menu *XAP_QNXFrameImpl::_getMainMenu(void)
{
	return m_pQNXMenu;
}


void XAP_QNXFrameImpl::_queue_resize()
{

}
PtWidget_t * XAP_QNXFrameImpl::getTBGroupWidget(void) const
{
	return m_wTBGroup;
}


GR_Graphics * XAP_QNXFrameImpl::getGraphics()
{
return(((AP_FrameData *)getFrame()->getFrameData())->m_pG);
}

