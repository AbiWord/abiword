/* AbiSource Application Framework
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
#include "ev_QNXKeyboard.h"
#include "ev_QNXMouse.h"
#include "ev_QNXMenu.h"
#include "ev_QNXToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ut_qnxHelper.h"

/* This is required for dynamic zoom implimentation */
#include "fv_View.h"

//THIS IS BAD!!!
#include "ap_FrameData.h"

/*****************************************************************/

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/
int XAP_QNXFrame::_fe::focus_in_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) 
{
	XAP_QNXFrame * pFrame = (XAP_QNXFrame *)data;
	UT_ASSERT(pFrame);
	/*
 	pFrame->getCurrentView()->focusChange( gtk_grab_get_current() == NULL || 
											 gtk_grab_get_current() == w ? AV_FOCUS_HERE 
																		 : AV_FOCUS_NEARBY);
	*/
	//I don't understand the AV_FOCUS_HERE vs NEARBY
	pFrame->getCurrentView()->focusChange(AV_FOCUS_HERE/*AV_FOCUS_NEARBY*/);
	return Pt_CONTINUE;
}

int XAP_QNXFrame::_fe::focus_out_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) 
{
	XAP_QNXFrame * pFrame = (XAP_QNXFrame *)data;
	UT_ASSERT(pFrame);
	pFrame->getCurrentView()->focusChange(AV_FOCUS_NONE);
	return Pt_CONTINUE;
}


int XAP_QNXFrame::_fe::button_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pQNXFrame->getMouse();


	if (pView)
		pQNXMouse->mouseClick(pView,info);
	return 0;
}

int XAP_QNXFrame::_fe::button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pQNXFrame->getMouse();

	if (pView)
		pQNXMouse->mouseUp(pView,info);
	return 0;
}

int XAP_QNXFrame::_fe::motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	EV_QNXMouse * pQNXMouse = (EV_QNXMouse *) pQNXFrame->getMouse();

	if (pView)
		pQNXMouse->mouseMotion(pView, info);
	
	return 0;
}
	
int XAP_QNXFrame::_fe::key_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info)
{
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	ev_QNXKeyboard * pQNXKeyboard = (ev_QNXKeyboard *) pQNXFrame->getKeyboard();
		
	if (pView)
		pQNXKeyboard->keyPressEvent(pView, info);

	return 0;
}
	
int XAP_QNXFrame::_fe::resize(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	FV_View * pfView = static_cast<FV_View*>(pView);

	if (pView) {
		UT_DEBUGMSG(("Resizing to %d,%d %d,%d \n",
			cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
			cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
		pView->setWindowSize(cbinfo->new_size.lr.x - cbinfo->new_size.ul.x, 
				             cbinfo->new_size.lr.y - cbinfo->new_size.ul.y);
	}

	// Dynamic Zoom Implimentation
	pQNXFrame->updateZoom();

	return Pt_CONTINUE;
}
		
#if 0

int XAP_QNXFrame::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *) gtk_object_get_user_data(GTK_OBJECT(w));
	XAP_App * pApp = pQNXFrame->getApp();
	UT_ASSERT(pApp);

	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindow");
	UT_ASSERT(pEM);
	
	if (pEM)
	{
		if ((*pEM->getFn())(pQNXFrame->getCurrentView(),NULL))
		{
			// returning FALSE means destroy the window, continue along the
			// chain of Gtk destroy events
			return FALSE;
		}
	}
		
	// returning TRUE means do NOT destroy the window; halt the message
	// chain so it doesn't see destroy
	return TRUE;
}
#endif
	
int XAP_QNXFrame::_fe::expose(PtWidget_t * w, PhTile_t * damage)
{
	PtArg_t args[1];
	UT_Rect rClip;

   PhRect_t rect;
   PhPoint_t pnt;
   PtSuperClassDraw(PtBasic, w, damage);
   PtBasicWidgetCanvas(w, &rect);
   PtWidgetOffset(w, &pnt);
/*
	UT_DEBUGMSG(("-----\nWidget Rect is %d,%d  %d,%d (@ %d,%d)",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, pnt.x, pnt.y));
*/
	XAP_QNXFrame *pQNXFrame, **ppQNXFrame = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXFrame, 0);
	PtGetResources(w, 1, args);
	pQNXFrame = (ppQNXFrame) ? *ppQNXFrame : NULL;

	UT_ASSERT(pQNXFrame);
	
	AV_View * pView = pQNXFrame->getCurrentView();
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
	
int XAP_QNXFrame::_fe::vScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();

	if (pView)
		pView->sendVerticalScrollEvent((UT_sint32) sb->position);
	return 0;
}
	
int XAP_QNXFrame::_fe::hScrollChanged(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtScrollbarCallback_t *sb = (PtScrollbarCallback_t *)info->cbdata;
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)data;
	AV_View * pView = pQNXFrame->getCurrentView();
	
	if (pView)
		pView->sendHorizontalScrollEvent((UT_sint32) sb->position);
	return 0;
}
	
/*****************************************************************/

/*
XAP_QNXFrame::XAP_QNXFrame(XAP_QNXApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
*/
XAP_QNXFrame::XAP_QNXFrame(XAP_QNXApp * app)
	: XAP_Frame((XAP_App *)(app)),
	  m_dialogFactory(this, (XAP_App *)(app))
{
	m_pQNXApp = app;
	m_pQNXMenu = NULL;
	m_pQNXPopup = NULL;
	m_pView = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

/*
XAP_QNXFrame::XAP_QNXFrame(XAP_QNXFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this, static_cast<XAP_App *>(f->m_pQNXApp))
*/
XAP_QNXFrame::XAP_QNXFrame(XAP_QNXFrame * f)
	: XAP_Frame((XAP_Frame *)(f)),
	  m_dialogFactory(this, (XAP_App *)(f->m_pQNXApp))
{
	m_pQNXApp = f->m_pQNXApp;
	m_pQNXMenu = NULL;
	m_pQNXPopup = NULL;
	m_pView = NULL;
}

XAP_QNXFrame::~XAP_QNXFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pQNXMenu);
	DELETEP(m_pQNXPopup);
}

bool XAP_QNXFrame::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
								  const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
								  const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
								  const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
								  const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue,
									szMenuLayoutKey, szMenuLayoutDefaultValue,
									szMenuLabelSetKey, szMenuLabelSetDefaultValue,
									szToolbarLayoutsKey, szToolbarLayoutsDefaultValue,
									szToolbarLabelSetKey, szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_QNXKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_QNXMouse(pEEM);
	UT_ASSERT(m_pMouse);

	return true;
}

UT_sint32 XAP_QNXFrame::setInputMode(const char * szName)
{
	UT_sint32 result = XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

PtWidget_t * XAP_QNXFrame::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

PtWidget_t * XAP_QNXFrame::getVBoxWidget(void) const
{
	return m_wVBox;
}

PtWidget_t * XAP_QNXFrame::getTBGroupWidget(void) const
{
	return m_wTBGroup;
}

GR_Graphics * XAP_QNXFrame::getGraphics() {
	return(((AP_FrameData*)m_pData)->m_pG);
}


XAP_DialogFactory * XAP_QNXFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

/*
 This is all based on the GTK version from unix
*/
void XAP_QNXFrame::_createTopLevelWindow(void)
{
	bool bResult;
	PtArg_t args[10];
	int 	n;
	PhArea_t area;

#define INIT_WIDTH 500
#define INIT_HEIGHT 400
	/*** Create the main window ***/
	//ndim.w = m_geometry.width; ndim.h = m_geometry.height;
	void *data = this;
	area.pos.x = 0; area.pos.y = 0;
	area.size.w = INIT_WIDTH; area.size.h = INIT_HEIGHT;
	m_AvailableArea = area;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, m_pQNXApp->getApplicationTitleForTitleBar(), 0);
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this));
	/*
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, Ph_WM_FFRONT);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_STATE, 0, Ph_WM_STATE_ISFRONT);
	*/
	//m_wTopLevelWindow = PtAppInit(NULL, NULL, NULL, 2, args);
	PtSetParentWidget(NULL);
	m_wTopLevelWindow = PtCreateWidget(PtWindow, NULL /* Use last widget? */, n, args);
	if (!m_wTopLevelWindow) {
		fprintf(stderr, "Can't create top level window \n");
		exit(1);
	}
	//PtAddEventHandler(m_wTopLevelWindow, Ph_EV_KEY, _fe::key_press_event, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_GOT_FOCUS, _fe::focus_in_event, this);
	PtAddCallback(m_wTopLevelWindow, Pt_CB_LOST_FOCUS, _fe::focus_out_event, this);

	/* TODO: Menu and the Toolbars all go into the same Toolbar "group" */
#if 0
	n = 0;
#define _MNU_GRP_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | Pt_TOP_ANCHORED_TOP)
    PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _MNU_GRP_ANCHOR_, _MNU_GRP_ANCHOR_);
    PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, 0, Pt_RESIZE_X_BITS);
    PtSetArg(&args[n++], Pt_ARG_WIDTH, area.size.w, 0); 
	m_wTBGroup = PtCreateWidget(PtToolbarGroup, m_wTopLevelWindow, n, args);
#endif

	/*** Create the menu bars ***/
	UT_DEBUGMSG(("QNXFrame: creating menu bars \n"));
	m_pQNXMenu = new EV_QNXMenuBar(m_pQNXApp, this, m_szMenuLayoutName, m_szMenuLabelSetName);
	UT_ASSERT(m_pQNXMenu);
	bResult = m_pQNXMenu->synthesizeMenuBar();
	UT_ASSERT(bResult);

#if 1
	m_AvailableArea.pos.y += 30 + 3;
	m_AvailableArea.size.h -= 30 + 3;
#else
	m_AvailableArea.pos.y += m_pQNXMenu->getMenuHeight() + 3;
	m_AvailableArea.size.h -= m_pQNXMenu->getMenuHeight() + 3;
#endif
	
	/*** Create the tool bars ***/
	_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	
	/* Peel off some space at the bottom for the status bar */
	m_AvailableArea.size.h -= 24;

	/*** Add the document view ***/	
	m_wSunkenBox = _createDocumentWindow();

	//Add status bars
	m_wStatusBar = _createStatusBarWindow();
	
	//Set the icon for the window
	_setWindowIcon();

	//Center the window (or put it below existing ones?)
	UT_QNXCenterWindow(NULL, m_wTopLevelWindow);
}

bool XAP_QNXFrame::close()
{
	PtDestroyWidget(getTopLevelWindow());
	return true;
}

bool XAP_QNXFrame::raise()
{
	if (getTopLevelWindow())
		PtWindowToFront(getTopLevelWindow());
	return true;
}

bool XAP_QNXFrame::show()
{
	raise();
	return true;
}

bool XAP_QNXFrame::openURL(const char * szURL)
{
	char execstring[4096];

	//TODO: I should use spawn here
	snprintf(execstring, 4096, "voyager -u %s &", szURL);
	system(execstring);

	return true;
}

bool XAP_QNXFrame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pQNXApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);

	PtArg_t args[1];

	PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, buf, 0); 
	PtSetResources(getTopLevelWindow(), 1, args);
	
	return true;
}

/*****************************************************************/
bool XAP_QNXFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{

	bool bResult = true;
	UT_ASSERT(!m_pQNXPopup);

	setPopupDone(0);	
	m_pQNXPopup = new EV_QNXMenuPopup(m_pQNXApp, this, 
									  szMenuName, 
									  m_szMenuLabelSetName);
	if (m_pQNXPopup && m_pQNXPopup->synthesizeMenuPopup())
	{
		PtArg_t	arg;
		PhPoint_t pos;

		PtWidget_t *menu = m_pQNXPopup->getMenuHandle();
		PtRealizeWidget(menu);

		memset(&pos, 0, sizeof(pos));
		PtGetAbsPosition(m_wSunkenBox, &pos.x, &pos.y);
		pos.x += x; pos.y += y;

		PtSetArg(&arg, Pt_ARG_POS, &pos, 0);
		PtSetResources(menu, 1, &arg);

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

void XAP_QNXFrame::setTimeOfLastEvent(unsigned int eventTime)
{
//	m_pQNXApp->setTimeOfLastEvent(eventTime);
}

EV_Toolbar * XAP_QNXFrame::_newToolbar(XAP_App *app, XAP_Frame *frame,
					const char *szLayout,
					const char *szLanguage)
{
	return (new EV_QNXToolbar((XAP_QNXApp *)(app), 
							  (XAP_QNXFrame *)(frame), szLayout, szLanguage));
}
