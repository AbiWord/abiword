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
#include <string.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"
#include "ev_BeOSMenu.h"
#include "ev_BeOSToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"


#include "ap_FrameData.h"
#include "gr_BeOSGraphics.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*********************************************************/
TFScrollBar::TFScrollBar(XAP_BeOSFrame *pFrame, BRect frame, const char *name,
                         BView *target, float min, float max, 
			 orientation direction)
                        :BScrollBar(frame, name, NULL, min, max, direction) {
        m_pBeOSFrame = pFrame;
}

/*
 This is triggered when the user changes the value of the
 scroll bar.  When this function is called, the value has
 already changed. (Just to be sure we shouldn't send an
 event if the newValue matches the old value).
*/
void TFScrollBar::ValueChanged(float newValue) {
        AV_View * pView = m_pBeOSFrame->getCurrentView();
		if (!pView) {
			return;
		}
        
	float minRange , maxRange;
	GetRange(&minRange , &maxRange);

	if (Orientation() == B_VERTICAL &&
			(UT_sint32)newValue != pView->getYScrollOffset()) {
				pView->sendVerticalScrollEvent((UT_sint32) newValue, (UT_sint32)maxRange);
        }
        else if (Orientation() == B_HORIZONTAL &&
				 (UT_sint32)newValue != pView->getXScrollOffset()) {
				pView->sendHorizontalScrollEvent((UT_sint32) newValue, (UT_sint32)maxRange);
        }
}


/*********************************************************/ 

XAP_BeOSFrame::XAP_BeOSFrame(XAP_BeOSApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
	  m_dialogFactory(this, static_cast<XAP_App *>(app))
{
	m_pBeOSApp = app;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	m_pBeOSMenu = NULL;
	m_pView = NULL;
	m_pBeWin = NULL;
	m_pBeDocView = NULL;
	m_pBeOSPopup = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_BeOSFrame::XAP_BeOSFrame(XAP_BeOSFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this, static_cast<XAP_App *>(f->m_pBeOSApp))
{
	m_pBeOSApp = f->m_pBeOSApp;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	m_pBeOSMenu = NULL;
	m_pView = NULL;
	m_pBeWin = NULL;
	m_pBeDocView = NULL;
}

XAP_BeOSFrame::~XAP_BeOSFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pBeOSMenu);
}

bool XAP_BeOSFrame::initialize(const char * szKeyBindingsKey, 
				  const char * szKeyBindingsDefaultValue,
				  const char * szMenuLayoutKey, 
				  const char * szMenuLayoutDefaultValue,
				  const char * szMenuLabelSetKey, 
				  const char * szMenuLabelSetDefaultValue,
				  const char * szToolbarLayoutsKey, 
				  const char * szToolbarLayoutsDefaultValue,
				  const char * szToolbarLabelSetKey, 
				  const char * szToolbarLabelSetDefaultValue) {
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, 
					szKeyBindingsDefaultValue,
					szMenuLayoutKey, 
					szMenuLayoutDefaultValue,
					szMenuLabelSetKey, 
					szMenuLabelSetDefaultValue,
					szToolbarLayoutsKey, 
					szToolbarLayoutsDefaultValue,
					szToolbarLabelSetKey, 
					szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	//These are actually "attached" in the ap_Frame code
	//since they require that all the beos classes be
	//instantiated.
	m_pKeyboard = new ev_BeOSKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new ev_BeOSMouse(pEEM);
	UT_ASSERT(m_pMouse);

	return true;
}

BWindow * XAP_BeOSFrame::getTopLevelWindow(void) const
{
	return(m_pBeWin);
}

be_DocView * XAP_BeOSFrame::getBeDocView(void) const
{
	return(m_pBeDocView);
}

void XAP_BeOSFrame::setBeDocView(be_DocView *view)
{
	m_pBeDocView = view;
}
UT_sint32 XAP_BeOSFrame::setInputMode(const char * szName)
{
	UT_sint32 result = XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// If it actually changed the mode, update the maps
		EV_EditEventMapper * pEEM = getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}
	return result;
}


XAP_DialogFactory * XAP_BeOSFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_BeOSFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	bool bResult;

	m_pBeWin = new be_Window(m_pBeOSApp, this, 
			      	BRect(50, 50, 200+50, 300+50),
			      	"Alpha-AbiWord", 
			      	B_DOCUMENT_WINDOW_LOOK,
			      	B_NORMAL_WINDOW_FEEL, 0);
	UT_ASSERT(m_pBeWin);
	m_pBeWin->_createWindow(m_szMenuLayoutName, m_szMenuLabelSetName);
	// we let our caller decide when to show m_wTopLevelWindow.
	return;
}

bool XAP_BeOSFrame::close()
{
	m_pBeWin->Lock();
	m_pBeWin->Close();
	m_pBeWin->Unlock();
	return true;
}

bool XAP_BeOSFrame::raise()
{
	
	m_pBeWin->Lock();
	m_pBeWin->Show();
	m_pBeWin->Unlock();
	return true;
}

bool XAP_BeOSFrame::show()
{
	m_pBeWin->Lock();
	m_pBeWin->Show();
	m_pBeWin->Unlock();
	return true;
}

bool XAP_BeOSFrame::openURL(const char * szURL)
{
	char *url;
	url = (char*)UT_strdup(szURL);
	be_roster->Launch("text/html", 1, &url);
	free(url);
	return true;
}

bool XAP_BeOSFrame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pBeOSApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);
			
	m_pBeWin->Lock();
	m_pBeWin->SetTitle(buf);
	m_pBeWin->Unlock();

	return true;
}

UT_Vector * XAP_BeOSFrame::VecToolbarLayoutNames() {
	return(&m_vecToolbarLayoutNames); 
}

const char * XAP_BeOSFrame::ToolbarLabelSetName() { 
	return(m_szToolbarLabelSetName); 
};     

GR_Graphics * XAP_BeOSFrame::Graphics() {
	return(((AP_FrameData*)m_pData)->m_pG);
}

void XAP_BeOSFrame::setScrollBars(TFScrollBar *h, TFScrollBar *v) {
	m_hScroll = h;
	m_vScroll = v;
}	



/*********************************************************
 Local Window/View Class Stuff 
*********************************************************/

be_Window::be_Window(XAP_BeOSApp *theApp, XAP_BeOSFrame *theFrame,
 	  	BRect r, char *name, 
		window_look look, window_feel feel, 
 		uint32 flags, uint32 workspace) 
       : BWindow(r, name, look, feel, flags, workspace) {
	
	m_pBeOSFrame = theFrame;
	m_pBeOSApp = theApp;
}

#if 0
void be_Window::MessageReceived(BMessage *msg) {
	switch(msg->what) {
	case 'tmer':
		printf("Received a timer message \n");
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

#endif

bool be_Window::QuitRequested(void) {
	//I need to tell the framework here ...
	XAP_App *pApp = m_pBeOSApp;
        AV_View *pView = m_pBeOSFrame->getCurrentView();
	UT_ASSERT(pApp);
	UT_ASSERT(pView);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	// make sure it's bound to something
	EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindow");
	UT_ASSERT(pEM);                                         

	if (pEM) {
		(*pEM->getFn())(pView,NULL);
	}

	// let the window be destroyed
	return(false);
}

void be_Window::MessageReceived(BMessage *pMsg)
{
	float smallStep, bigStep;
	float deltaX, deltaY;
	
	switch(pMsg->what)
	{
		case 'inme': // Inme is a "Create Menu" message sent when the view is established.
			m_pBeOSMenu->synthesize();
			break;
#if (B_BEOS_VERSION > B_BEOS_VERSION_4_5)		
		case B_MOUSE_WHEEL_CHANGED:
		
			if( pMsg->FindFloat("be:wheel_delta_y" , &deltaY) == B_OK)
			{		
			m_vScroll->GetSteps(&smallStep, &bigStep);
			m_vScroll->SetValue(m_vScroll->Value() + smallStep * deltaY);
			}
			
			if( pMsg->FindFloat("be:wheel_delta_x" , &deltaX) == B_OK)
			{
			m_hScroll->GetSteps(&smallStep, &bigStep);
			m_hScroll->SetValue(m_hScroll->Value() + smallStep * deltaX);
			}

			break;
#else
#warning "You should really upgrade your BeOS version to get wheel support"
#endif
	}
	
	BWindow::MessageReceived(pMsg);
}

bool be_Window::_createWindow(const char *szMenuLayoutName,
			   const char *szMenuLabelSetName) {
	//BRect r;

	m_winRectAvailable = Bounds();
	//printf("Initial Available Rect: "); m_winRectAvailable.PrintToStream();
	//Add the menu
	
	m_pBeOSMenu = new EV_BeOSMenu(m_pBeOSApp, m_pBeOSFrame,
				      szMenuLayoutName, szMenuLabelSetName);
	
	m_pBeOSMenu->synthesizeMenuBar();
		
	//printf("After Adding Menu Available Rect: "); m_winRectAvailable.PrintToStream();
	
	//Add the toolbars
	UT_ASSERT(m_pBeOSFrame);
	
	m_pBeOSFrame->_createToolbars();

	//printf("After Adding Toolbars: "); m_winRectAvailable.PrintToStream();


	// Let the app-specific frame code create the contents of
        // the child area of the window (between the toolbars and
        // the status bar).

        _createDocumentWindow();

        // Let the app-specific frame code create the status bar
        // if it wants to.  we will put it below the document
        // window (a peer with toolbars and the overall sunkenbox)
        // so that it will appear outside of the scrollbars.
       m_pBeOSStatusBarView =  _createStatusBarWindow();
	
    if (!m_pBeOSStatusBarView)
	 return (false);

	return(true);	
}


be_DocView::be_DocView(BRect frame, const char *name, uint32 resizeMask, uint32 flags)
	:BView(frame, name, resizeMask, flags | B_FRAME_EVENTS) {
	m_pBPicture = NULL;
}

void be_DocView::FrameResized(float new_width, float new_height) {
	be_Window	*pBWin;
	GR_BeOSGraphics *pG;
	pBWin = (be_Window *)Window();
	if (!pBWin || !pBWin->m_pBeOSFrame)
		return; 
// 	pBWin->DisableUpdates(); 
	pG = (GR_BeOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG)
	{
		pBWin->EnableUpdates();
		return;
	}

	BRect rect = Bounds();
	pG->ResizeBitmap(rect);

	//Only do this after we have resize the Graphics
	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	if (pView) {
		pView->setWindowSize(rect.Width()+1, rect.Height()+1);
		pView->draw(); //TODO do we need this???
 		/* Methinks it can handle itself*/
	}
//	pBWin->EnableUpdates();
	pBWin->Sync(); //Maybe Sync? We'll see

	// Dynamic Zoom Implimentation
	pBWin->m_pBeOSFrame->updateZoom();
}

void be_DocView::Draw(BRect updateRect) {
	be_Window 		*pBWin;

	pBWin = (be_Window*)Window();
	if (!pBWin || !pBWin->m_pBeOSFrame)
		return;
	
	// Okay, there is a bug here, that when the BeOS wants a single line of horizontal text drawn, it (more)
	// passes updateRect.top and updateRect.bottom as an equal value, so how do we get around this (more)
	// other than pre-decreasing the top value.
	// The same thing also occurs for a vertical line..
	/*
	updateRect.top -= 1.0f;
	if( updateRect.top < 0.0f)
	{
		updateRect.bottom += 1.0f;
		updateRect.top = 0.0f;
	}
	
	updateRect.left -= 1.0f;
	if( updateRect.left < 0.0f)
	{
		updateRect.left = 0.0f;
		updateRect.right += 1.0f;
	}
*/
#if defined(USE_BACKING_BITMAP)
	GR_BeOSGraphics 	*pG;
	BBitmap 		*pBitmap;	
 	pBWin->DisableUpdates();
	pG = (GR_BeOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG || !(pBitmap = pG->ShadowBitmap()))
		return;
	
	DrawBitmap(pBitmap);
	pBWin->EnableUpdates();
	pBWin->Flush();
#else
/* 
Things to do to speed this up, make it less flashy:
 - Only invalidate/draw in the update rect 
 - Draw everything to an offscreen buffer/picture
 - Don't erase the background by default 
*/
	pBWin->DisableUpdates();
	//This code path is used for printing
	if (m_pBPicture) {
		printf("In printer path code\n");
		DrawPicture(m_pBPicture, BPoint(0,0));
		pBWin->EnableUpdates();
		return;
	}

	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	if (pView) {
//		BPicture *mypict;
		//BeginPicture(new BPicture);
		
		//Always erase the contents of the rect first
		//NOTE: No need to do this
		//SetHighColor(ViewColor());	
	//	FillRect(updateRect);
		
		//The tell AbiWord to draw us ...
		UT_Rect r;
		r.top = (UT_sint32)updateRect.top;
		r.left = (UT_sint32)updateRect.left;
		r.width = (UT_sint32)updateRect.Width()+1;
		r.height = (UT_sint32)updateRect.Height()+1;
		pView->draw(&r);
/*
		if ((mypict = EndPicture())) {
			DrawPicture(mypict, BPoint(0,0));
			pBWin->EnableUpdates();
			pBWin->Flush();
			delete mypict;
		}*/
	}
	pBWin->EnableUpdates();
	pBWin->Sync();
#endif
}

void be_DocView::WindowActivated(bool activated)
{
	be_Window *pBWin;
	pBWin = (be_Window *)Window();
	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	if(pView)
		pView->focusChange(activated ? AV_FOCUS_HERE : AV_FOCUS_NONE);
}

/*****************************************************************/
bool XAP_BeOSFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{
	bool bResult = true;
	UT_ASSERT(!m_pBeOSPopup);

	m_pBeOSPopup = new EV_BeOSMenuPopup(m_pBeOSApp, this, 
									  szMenuName, 
									  m_szMenuLabelSetName);
									  
	if (m_pBeOSPopup && m_pBeOSPopup->synthesizeMenuPopup(this))
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		
		translateDocumentToScreen(x,y);
	 
		BPoint screenPoint(x,y);
		BPopUpMenu* pMenu = m_pBeOSPopup->GetHandle();
		BMenuItem* selectedItem = pMenu->Go(screenPoint , false , false, false);
	
		// Send the menu item invocation message to the window
		
		if(selectedItem) // The user clicked an item, will be NULL if they click away from the popup.
		{
			if(m_pBeWin) // Should always be valid here, but just in case.
				m_pBeWin->PostMessage(selectedItem->Message());	
		}
		
	}
	
	DELETEP(m_pBeOSPopup);

	return bResult;

}

EV_Toolbar * XAP_BeOSFrame::_newToolbar(XAP_App *app, XAP_Frame *frame,
					const char *szLayout,
					const char *szLanguage)
{
	return (new EV_BeOSToolbar(static_cast<XAP_BeOSApp *>(app), 
							   static_cast<XAP_BeOSFrame *>(frame), 
							   szLayout, szLanguage));
}
