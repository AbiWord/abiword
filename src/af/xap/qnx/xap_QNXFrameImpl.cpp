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
#include "xav_View.h"
#include "fv_View.h"
#include "xad_Document.h"
#include "gr_Graphics.h"
#include "ie_imp.h"
#include "ap_FrameData.h"
#include "xap_Frame.h"

XAP_QNXFrameImpl::XAP_QNXFrameImpl(XAP_Frame *pFrame,XAP_QNXApp *pApp) : 
	XAP_FrameImpl(pFrame),
	m_bDoZoomUpdate(false),
	m_iZoomUpdateID(0),
	m_iAbiRepaintID(0),
	m_pQNXApp(pApp),
//	m_pQNXMenu(NULL),
//	m_pQNXPopup(NULL),
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
	return 0;
}

int XAP_QNXFrameImpl::_fe::button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();

	if (pView)
		pQNXMouse->mouseUp(pView,info);
	return 0;
}

int XAP_QNXFrameImpl::_fe::motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();	
	AV_View * pView = pFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pFrame->getMouse();

	if (pView)
		pQNXMouse->mouseMotion(pView, info);
	
	return 0;
}
	
int XAP_QNXFrameImpl::_fe::key_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	ev_QNXKeyboard * pQNXKeyboard = (ev_QNXKeyboard *) pFrame->getKeyboard();
		
	if (pView)
		pQNXKeyboard->keyPressEvent(pView, info);

	return 0;
}
	
int XAP_QNXFrameImpl::_fe::resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	//FV_View * pfView = static_cast<FV_View*>(pView);

	if (pView) {
		UT_DEBUGMSG(("Document Area Resizing to %d,%d %d,%d ",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
		pView->setWindowSize(cbinfo->new_size.lr.x - cbinfo->new_size.ul.x, 
				             cbinfo->new_size.lr.y - cbinfo->new_size.ul.y);
	}

	// Dynamic Zoom Implimentation
	//Make this as a idleadd instead!
	pFrame->updateZoom();

	return Pt_CONTINUE;
}

int XAP_QNXFrameImpl::_fe::window_resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
#if 0
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	if (m_pQNXApp) {
		UT_DEBUGMSG(("Window Resizing to %d,%d %d,%d ",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
		m_pQNXApp->setGeometry(-1, -1, 
						   cbinfo->new_size.lr.x - cbinfo->new_size.ul.x, 
				           cbinfo->new_size.lr.y - cbinfo->new_size.ul.y);
	}

	return Pt_CONTINUE;
#endif
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
/*
   	PtSuperClassDraw(PtBasic, w, damage);
*/
   	PtBasicWidgetCanvas(w, &rect);
   	PtWidgetOffset(w, &pnt);
/*
	UT_DEBUGMSG(("-----\nWidget Rect is %d,%d  %d,%d (@ %d,%d)",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, pnt.x, pnt.y));
*/
	XAP_FrameImpl *pQNXFrameImpl, **ppQNXFrameImpl = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXFrameImpl, 0);
	PtGetResources(w, 1, args);
	pQNXFrameImpl = (ppQNXFrameImpl) ? *ppQNXFrameImpl : NULL;

	UT_ASSERT(pQNXFrameImpl);
	
//XXX: Or AV_View as before??	
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
			UT_DEBUGMSG(("Multiple damage rects "));
#if defined(MULTIPLE_EXPOSE_EVENTS) 
			damage = damage->next;
#endif
		}
		while (damage) {
/*
			UT_DEBUGMSG(("Expose Rect is %d,%d  %d,%d ",
			damage->rect.ul.x, damage->rect.ul.y, damage->rect.lr.x, damage->rect.lr.y));
*/
			/* At one point in time this required some fiddling to put it in the widget co-ordinates*/
			rClip.width = (damage->rect.lr.x - damage->rect.ul.x) + 1;
			rClip.height = (damage->rect.lr.y - damage->rect.ul.y) + 1;
			rClip.left = damage->rect.ul.x - pnt.x;
			rClip.top = damage->rect.ul.y - pnt.y;

			UT_DEBUGMSG(("Adjusted Expose Rect %d,%d %d/%d ",
				rClip.left, rClip.top, rClip.width, rClip.height));
				
			//Don't bother setting the clip here, the Graphics routine does it

			//OR: Pass the draw function the clipping rectangle
			//This is preferred since this way the application
			//can optimize their drawing routines as well.
			pView->draw(&rClip);

			//OR: Completely unoptimized
			//pView->draw(NULL);
			//break;

#if defined(MULTIPLE_EXPOSE_EVENTS) 
			damage = damage->next;
#else
			break;
#endif
		}

	}
	UT_DEBUGMSG(("====="));

	return 0;
}
	
int XAP_QNXFrameImpl::_fe::vScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();

	AV_View * pView = pFrame->getCurrentView();

	if (pView)
		pView->sendVerticalScrollEvent((UT_sint32) _UL(sb->position));
	return 0;
}
	
int XAP_QNXFrameImpl::_fe::hScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;

	XAP_QNXFrameImpl * pFrameImpl = (XAP_QNXFrameImpl *)data;
	XAP_Frame	*pFrame	= pFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	
	if (pView)
		pView->sendHorizontalScrollEvent((UT_sint32) _UL(sb->position));
	return 0;
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
	
			IE_Imp::constructImporter(dr.m_pDoc,0,IE_Imp::fileTypeForSuffix(".txt"),&pImp,0);
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
return false;
}

void XAP_QNXFrameImpl::_initialize()
{

}

void XAP_QNXFrameImpl::_nullUpdate() const
{

}

void XAP_QNXFrameImpl::_setCursor(GR_Graphics::Cursor c)
{

}


UT_sint32 XAP_QNXFrameImpl::_setInputMode(const char *szName)
{

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

}

void XAP_QNXFrameImpl::_setGeometry()
{

}

void XAP_QNXFrameImpl::_rebuildMenus(void)
{

}

void XAP_QNXFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{

}

bool XAP_QNXFrameImpl::_close()
{

}

bool XAP_QNXFrameImpl::_raise()
{

}

bool XAP_QNXFrameImpl::_show()
{

}

bool XAP_QNXFrameImpl::_runModalContextMenu(AV_View *pView,const char *szMenuName,
	UT_sint32 x,UT_sint32 y)
{


}

bool XAP_QNXFrameImpl::_openURL(const char *szURL)
{


}

void XAP_QNXFrameImpl::_setFullScreen(bool changeToFullScreen)
{

}

EV_Toolbar *XAP_QNXFrameImpl::_newToolbar(XAP_App *pApp,XAP_Frame *pFrame,
	const char *szLayout,
	const char *szLanguage)
{

}

EV_Menu *XAP_QNXFrameImpl::_getMainMenu(void)
{

}

void XAP_QNXFrameImpl::_queue_resize()
{

}
