#include <Pt.h>
#include "ap_QNXFrameImpl.h"
#include "ap_QNXApp.h"
#include "ev_QNXToolbar.h"
#include "ap_FrameData.h"
#include "ap_QNXTopRuler.h"
#include "ap_QNXLeftRuler.h"
#include "xap_QNXApp.h"
#include "ap_QNXStatusBar.h"
#include "gr_QNXGraphics.h"
#include "ut_debugmsg.h"



AP_QNXFrameImpl::AP_QNXFrameImpl(AP_QNXFrame *pQNXFrame,XAP_QNXApp *pQNXApp) :
	XAP_QNXFrameImpl(static_cast<XAP_Frame *>(pQNXFrame),static_cast<AP_App *>(pQNXApp))
{
}

XAP_FrameImpl * AP_QNXFrameImpl::createInstance(XAP_Frame *pFrame,XAP_App *pApp)
{
	XAP_FrameImpl *pFrameImpl = new AP_QNXFrameImpl(static_cast<AP_QNXFrame *>(pFrame),static_cast<XAP_QNXApp *>(pApp));
	return pFrameImpl;
}

void AP_QNXFrameImpl::_bindToolbars(AV_View *pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.		
		EV_QNXToolbar * pQNXToolbar = (EV_QNXToolbar *)m_vecToolbars.getNthItem(k);
		pQNXToolbar->bindListenerToView(pView);
	}	
}

UT_RGBColor AP_QNXFrameImpl::getColorSelBackground () const

{
return UT_RGBColor(125,125,125);
}
void AP_QNXFrameImpl::_showOrHideToolbars()
{
	XAP_Frame *pFrame = getFrame();
	AP_FrameData *pData = static_cast<AP_FrameData *>(pFrame->getFrameData());	
  bool *bShowBar = pData->m_bShowBar;

    for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
    {
        // TODO: The two next lines are here to bind the EV_Toolbar to the
        // AP_FrameData, but their correct place are next to the toolbar creation (JCA)
        EV_QNXToolbar * pQNXToolbar = static_cast<EV_QNXToolbar *> (m_vecToolbars.getNthItem(i));
        pData->m_pToolbar[i] = pQNXToolbar;
	  static_cast<AP_QNXFrame *>(pFrame)->toggleBar(i, bShowBar[i]);
    }
}

void AP_QNXFrameImpl::_refillToolbarsInFrameData()
{
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_QNXToolbar * pQNXToolbar = static_cast<EV_QNXToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*>(getFrame()->getFrameData())->m_pToolbar[i] = pQNXToolbar;
	}
}

void AP_QNXFrameImpl::_showOrHideStatusbar()
{
	XAP_Frame *pFrame = getFrame();
	AP_FrameData *pData = static_cast<AP_FrameData *>(pFrame->getFrameData());
  bool bShowStatusBar = pData->m_bShowStatusBar;
 	static_cast<AP_QNXFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
}

PtWidget_t * AP_QNXFrameImpl::_createDocumentWindow()
{
	PhArea_t area, savedarea;
	XAP_Frame *pFrame = getFrame();
	AP_FrameData *pData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	PtArg_t args[10];
	int n;
	bool bShowRulers = pData->m_bShowRuler; 


#define SCROLLBAR_WIDTHHEIGHT 20
	// Strip the scrollbarwidth off the right and bottom
	// so that the scrollbars overlap the rulers
	savedarea = m_AvailableArea;
#if !defined(SCROLL_SMALLER_THAN_RULER) 
	m_AvailableArea.size.h -= SCROLLBAR_WIDTHHEIGHT; 
	m_AvailableArea.size.w -= SCROLLBAR_WIDTHHEIGHT; 
#endif

	AP_QNXTopRuler * pQNXTopRuler = new AP_QNXTopRuler(getFrame());
	UT_ASSERT(pQNXTopRuler);
	m_topRuler = pQNXTopRuler->createWidget();

	((AP_FrameData*)pData)->m_pTopRuler = pQNXTopRuler;

		// create the left ruler
		AP_QNXLeftRuler * pQNXLeftRuler = new AP_QNXLeftRuler(getFrame());
		UT_ASSERT(pQNXLeftRuler);
		m_leftRuler = pQNXLeftRuler->createWidget();
		((AP_FrameData*)pData)->m_pLeftRuler = pQNXLeftRuler;
		// get the width from the left ruler and stuff it into the top ruler.
		pQNXTopRuler->setOffsetLeftRuler(pQNXTopRuler->getGraphics()->tdu(pQNXLeftRuler->getWidth()));

		// create the scrollbars horizontal then vertical

#if defined(SCROLL_SMALLER_THAN_RULER) 
	area.size.w = SCROLLBAR_WIDTHHEIGHT;
	area.size.h = m_AvailableArea.size.h - area.size.w;
	area.pos.y = m_AvailableArea.pos.y;
	area.pos.x = m_AvailableArea.pos.x + m_AvailableArea.size.w - area.size.w;
	m_AvailableArea.size.w -= area.size.w;
#else
	area.size.w = SCROLLBAR_WIDTHHEIGHT;
	area.size.h = savedarea.size.h - area.size.w;
	area.pos.y = savedarea.pos.y;
	area.pos.x = savedarea.pos.x + savedarea.size.w - area.size.w;
#endif

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
#define _VS_ANCHOR_ (Pt_LEFT_ANCHORED_RIGHT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _VS_ANCHOR_, _VS_ANCHOR_); 
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_FLAGS, Pt_SCROLLBAR_FOCUSED, 
									 		     Pt_SCROLLBAR_FOCUSED ); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	PtSetArg(&args[n++], Pt_ARG_ORIENTATION, Pt_VERTICAL, 0); 
	m_vScroll = PtCreateWidget(PtScrollbar, getTopLevelWindow(), n, args);
	PtAddCallback(m_vScroll, Pt_CB_SCROLL_MOVE, _fe::vScrollChanged, this);

#if defined(SCROLL_SMALLER_THAN_RULER) 
	area.size.h = SCROLLBAR_WIDTHHEIGHT;
	area.size.w = m_AvailableArea.size.w;
	area.pos.y = m_AvailableArea.pos.y + m_AvailableArea.size.h - area.size.h;
	area.pos.x = m_AvailableArea.pos.x;
	m_AvailableArea.size.h -= area.size.h;
#else
	area.size.h = SCROLLBAR_WIDTHHEIGHT;
	area.size.w = savedarea.size.w - SCROLLBAR_WIDTHHEIGHT;
	area.pos.y = savedarea.pos.y + savedarea.size.h - area.size.h;
	area.pos.x = savedarea.pos.x;
#endif

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
#define _HS_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_BOTTOM | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _HS_ANCHOR_, _HS_ANCHOR_); 
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_FLAGS, Pt_SCROLLBAR_FOCUSED | 1 /*Horizontal*/,
									 			 Pt_SCROLLBAR_FOCUSED | 1 /*Horizontal*/); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_FALSE, Pt_GETS_FOCUS); 
	PtSetArg(&args[n++], Pt_ARG_ORIENTATION, 1 /*Horizontal*/, 0); 
	m_hScroll = PtCreateWidget(PtScrollbar, getTopLevelWindow(), n, args);
	PtAddCallback(m_hScroll, Pt_CB_SCROLL_MOVE, _fe::hScrollChanged, this);

	// create a drawing area in the for our document window.

	area.pos.x = m_AvailableArea.pos.x;
	area.pos.y = m_AvailableArea.pos.y;
	area.size.w = m_AvailableArea.size.w; 
	area.size.h = m_AvailableArea.size.h;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
#define _DA_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _DA_ANCHOR_, _DA_ANCHOR_);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS,Pt_TRUE,Pt_GROUP_STRETCH_VERTICAL|Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0); 
	PtSetArg(&args[n++], Pt_ARG_CURSOR_OVERRIDE,Pt_TRUE,0);
	m_dAreaGroup = PtCreateWidget(PtGroup, getTopLevelWindow(), n, args);
	PtAddCallback(m_dAreaGroup, Pt_CB_RESIZE,_fe::resize, this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0); 
	PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_GETS_FOCUS| Pt_CALLBACKS_ACTIVE, Pt_GETS_FOCUS|Pt_CALLBACKS_ACTIVE); 
	m_dArea = PtCreateWidget(PtRaw, m_dAreaGroup, n, args); 

	PtAddEventHandler(m_dArea, Ph_EV_KEY, _fe::key_press_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_PTR_MOTION_BUTTON, _fe::motion_notify_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_BUT_PRESS, _fe::button_press_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_BUT_RELEASE, _fe::button_release_event, this);
	//QNX DND Code
	PtAddCallback(m_dArea,Pt_CB_DND,_fe::dnd,this);

	if(bShowRulers) {
		int *width,*height;
		PtSetResource(m_topRuler,Pt_ARG_FLAGS,Pt_FALSE,Pt_DELAY_REALIZE); //We simply deactivate this if we want to show the rulers on startup.
		PtSetResource(m_leftRuler,Pt_ARG_FLAGS,Pt_FALSE,Pt_DELAY_REALIZE); 
		PtExtentWidgetFamily(m_leftRuler);
		PtExtentWidgetFamily(m_topRuler);
		PtGetResource(m_leftRuler,Pt_ARG_WIDTH,&width,0);
		PtGetResource(m_topRuler,Pt_ARG_HEIGHT,&height,0);
		_reflowLayout(0,0,-(*height),0);
		_reflowLayout(0,0,0,-(*width));
	}

	return(m_dAreaGroup);
}

void AP_QNXFrameImpl::_setWindowIcon()
{
//Icon is bound into binary, can't set from code.
}

void AP_QNXFrameImpl::_createWindow()
{
	createTopLevelWindow();
	PtRealizeWidget(getTopLevelWindow());
	if(getFrame()->getFrameMode() == XAP_NormalFrame)
	{
		// needs to be shown so that the following functions work
		// TODO: get rid of cursed flicker caused by initially
		// TODO: showing these and then hiding them (esp.
		// TODO: noticable in the gnome build with a toolbar disabled)
		_showOrHideToolbars();
		_showOrHideStatusbar();
	}
	PtGiveFocus(m_dArea,NULL);
}

PtWidget_t * AP_QNXFrameImpl::_createStatusBarWindow()
{
	AP_QNXStatusBar * pQNXStatusBar = new AP_QNXStatusBar(getFrame());
	UT_ASSERT(pQNXStatusBar);
	AP_FrameData *pData = static_cast<AP_FrameData*>(getFrame()->getFrameData());

	((AP_FrameData *)pData)->m_pStatusBar = pQNXStatusBar;
	
	//This should probably be held in XP land
	PtWidget_t *w = pQNXStatusBar->createWidget();

	return w;
}

void AP_QNXFrameImpl::_setScrollRange(apufi_ScrollType scrollType,int iValue,float fUpperLimit,float fSize)
{

}

void AP_QNXFrameImpl::setDocumentFocus() {
PtContainerGiveFocus(m_dArea,NULL);
}

void AP_QNXFrameImpl::_reflowLayout(int loweradj, int upperadj, int topruleradj, int leftruleradj) {
	PhArea_t 	newarea, *oldarea;

	/*
     loweradj < 0 means we are enabling and > 0 means disabling
	*/
	if(loweradj != 0) { 	
		PtGetResource(m_hScroll, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.pos.y += loweradj;
		PtSetResource(m_hScroll, Pt_ARG_AREA, &newarea, 0);

		PtGetResource(m_vScroll, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.size.h += loweradj;
		PtSetResource(m_vScroll, Pt_ARG_AREA, &newarea, 0);
		if(m_leftRuler){
			PtGetResource(m_leftRuler, Pt_ARG_AREA, &oldarea, 0);
			newarea = *oldarea;
			newarea.size.h += loweradj;
			PtSetResource(m_leftRuler, Pt_ARG_AREA, &newarea, 0);
		}

		PtGetResource(m_dAreaGroup, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.size.h += loweradj;
		PtSetResource(m_dAreaGroup, Pt_ARG_AREA, &newarea, 0);
	} 

	/*
	 upperadj < 0 means we are enabling an > 0 means disabling
	*/
	if(upperadj != 0) {
		PtGetResource(m_vScroll, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.pos.y -= upperadj;
		newarea.size.h += upperadj;
		PtSetResource(m_vScroll, Pt_ARG_AREA, &newarea, 0);
		if(m_topRuler) {
			PtGetResource(m_topRuler, Pt_ARG_AREA, &oldarea, 0);
			newarea = *oldarea;
			newarea.pos.y -= upperadj;
			PtSetResource(m_topRuler, Pt_ARG_AREA, &newarea, 0);
		}
	}

	if(topruleradj != 0 || upperadj != 0) {
		if(m_leftRuler) {
			PtGetResource(m_leftRuler, Pt_ARG_AREA, &oldarea, 0);
			newarea = *oldarea;
			newarea.pos.y -= upperadj;
			newarea.size.h += upperadj;
			PtSetResource(m_leftRuler, Pt_ARG_AREA, &newarea, 0);
		}

		PtGetResource(m_dAreaGroup, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.pos.y -= upperadj + topruleradj;
		newarea.size.h += upperadj + topruleradj;
		PtSetResource(m_dAreaGroup, Pt_ARG_AREA, &newarea, 0);
	}

	if(leftruleradj != 0) {
		PtGetResource(m_dAreaGroup, Pt_ARG_AREA, &oldarea, 0);
		newarea = *oldarea;
		newarea.pos.x -= leftruleradj;
		newarea.size.w += leftruleradj;
		PtSetResource(m_dAreaGroup, Pt_ARG_AREA, &newarea, 0);
	}
}
