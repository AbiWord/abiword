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

void TFScrollBar::ValueChanged(float newValue) {
        AV_View * pView = m_pBeOSFrame->getCurrentView();
        if (pView && Orientation() == B_VERTICAL) {
                pView->setYScrollOffset((UT_sint32) newValue);
        }
        else if (pView && Orientation() == B_HORIZONTAL) {
                pView->setXScrollOffset((UT_sint32) newValue);
        }
}


/*********************************************************/ 

XAP_BeOSFrame::XAP_BeOSFrame(XAP_BeOSApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
	  m_dialogFactory(this, static_cast<XAP_App *>(app))
{
	m_pBeOSApp = app;
	m_pBeOSKeyboard = NULL;
	m_pBeOSMouse = NULL;
	m_pBeOSMenu = NULL;
	m_pView = NULL;
	m_pBeWin = NULL;
	m_pBeDocView = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_BeOSFrame::XAP_BeOSFrame(XAP_BeOSFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this, static_cast<XAP_App *>(f->m_pBeOSApp))
{
	m_pBeOSApp = f->m_pBeOSApp;
	m_pBeOSKeyboard = NULL;
	m_pBeOSMouse = NULL;
	m_pBeOSMenu = NULL;
	m_pView = NULL;
	m_pBeWin = NULL;
	m_pBeDocView = NULL;
}

XAP_BeOSFrame::~XAP_BeOSFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pBeOSKeyboard);
	DELETEP(m_pBeOSMouse);
	DELETEP(m_pBeOSMenu);
	UT_VECTOR_PURGEALL(EV_BeOSToolbar *, m_vecBeOSToolbars);
}

UT_Bool XAP_BeOSFrame::initialize(const char * szKeyBindingsKey, 
				  const char * szKeyBindingsDefaultValue,
				  const char * szMenuLayoutKey, 
				  const char * szMenuLayoutDefaultValue,
				  const char * szMenuLabelSetKey, 
				  const char * szMenuLabelSetDefaultValue,
				  const char * szToolbarLayoutsKey, 
				  const char * szToolbarLayoutsDefaultValue,
				  const char * szToolbarLabelSetKey, 
				  const char * szToolbarLabelSetDefaultValue) {
	UT_Bool bResult;

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
	m_pBeOSKeyboard = new ev_BeOSKeyboard(pEEM);
	UT_ASSERT(m_pBeOSKeyboard);

	m_pBeOSMouse = new ev_BeOSMouse(pEEM);
	UT_ASSERT(m_pBeOSMouse);

	return UT_TRUE;
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

ev_BeOSMouse * XAP_BeOSFrame::getBeOSMouse(void)
{
	return m_pBeOSMouse;
}

ev_BeOSKeyboard * XAP_BeOSFrame::getBeOSKeyboard(void)
{
	return m_pBeOSKeyboard;
}

XAP_DialogFactory * XAP_BeOSFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_BeOSFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	UT_Bool bResult;

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

UT_Bool XAP_BeOSFrame::close()
{
	m_pBeWin->Lock();
	m_pBeWin->Close();
	m_pBeWin->Unlock();
	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::raise()
{
	
	m_pBeWin->Lock();
	m_pBeWin->Show();
	m_pBeWin->Unlock();
	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::show()
{
	m_pBeWin->Lock();
	m_pBeWin->Show();
	m_pBeWin->Unlock();
	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::openURL(const char * szURL)
{
	char *url;
	url = (char*)strdup(szURL);
	be_roster->Launch("text/html", 1, &url);
	free(url);
	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return UT_FALSE;
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

	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName, UT_sint32 x, UT_sint32 y) {
#if 0
	UT_Bool bResult = UT_TRUE;

	UT_ASSERT(!m_pBeOSPopup);

	m_pBeOSPopup = new EV_BeOSMenuPopup(m_pBeOSApp,this,szMenuName,m_szMenuLabelSetName);
	if (m_pBeOSPopup && m_pBeOSPopup->synthesizeMenuPopup())
	{
		// the popup will steal the mouse and so we won't get the
		// button_release_event and we won't know to release our
		// grab.  so let's do it here.  (when raised from a keyboard
		// context menu, we may not have a grab, but that should be ok.

		GtkWidget * w = gtk_grab_get_current();
		if (w)
		{
			UT_DEBUGMSG(("Ungrabbing mouse [before popup].\n"));
			gtk_grab_remove(w);
		}

		translateDocumentToScreen(x,y);

		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		UT_Point pt;
		pt.x = x;
		pt.y = y;
		gtk_menu_popup(GTK_MENU(m_pBeOSPopup->getMenuHandle()), NULL, NULL,
					   s_gtkMenuPositionFunc, &pt, 3, 0);
	}

	DELETEP(m_pBeOSPopup);
	return bResult;
#endif
	return(UT_FALSE);
}

UT_Vector * XAP_BeOSFrame::VecBeOSToolbars() { 
	return(&m_vecBeOSToolbars); 
};      

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
	return(true);
}

bool be_Window::_createWindow(const char *szMenuLayoutName,
			   const char *szMenuLabelSetName) {
	BRect r;
	
	m_winRectAvailable = Bounds();
	//printf("Initial Available Rect: "); m_winRectAvailable.PrintToStream();

	//Add the menu
	m_pBeOSMenu = new EV_BeOSMenu(m_pBeOSApp, m_pBeOSFrame,
				      szMenuLayoutName, szMenuLabelSetName);
	UT_ASSERT(m_pBeOSMenu);
	UT_Bool bResult = m_pBeOSMenu->synthesize();
	UT_ASSERT(bResult);
	//printf("After Adding Menu Available Rect: "); m_winRectAvailable.PrintToStream();
	
	//Add the toolbars
	UT_ASSERT(m_pBeOSFrame);
	UT_uint32 nrToolbars = m_pBeOSFrame->VecToolbarLayoutNames()->getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_BeOSToolbar * pBeOSToolbar
			= new EV_BeOSToolbar(m_pBeOSApp, 
					 m_pBeOSFrame,
			 		(const char *)m_pBeOSFrame->VecToolbarLayoutNames()->getNthItem(k),
					m_pBeOSFrame->ToolbarLabelSetName());
		UT_ASSERT(pBeOSToolbar);
		bResult = pBeOSToolbar->synthesize();
		UT_ASSERT(bResult);
		m_pBeOSFrame->VecBeOSToolbars()->addItem(pBeOSToolbar);
	}
	//printf("After Adding Toolbars: "); m_winRectAvailable.PrintToStream();


	// Let the app-specific frame code create the contents of
        // the child area of the window (between the toolbars and
        // the status bar).

        _createDocumentWindow();

#if 0

        // Let the app-specific frame code create the status bar
        // if it wants to.  we will put it below the document
        // window (a peer with toolbars and the overall sunkenbox)
        // so that it will appear outside of the scrollbars.
	m_wStatusBar = _createStatusBarWindow();
        if (m_wStatusBar) {
                gtk_widget_show(m_wStatusBar);
                gtk_box_pack_end(GTK_BOX(m_wVBox), m_wStatusBar, FALSE, FALSE, 0
);
        }                    
#endif

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
 
	pG = (GR_BeOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG)
		return;

	BRect rect = Bounds();
	pG->ResizeBitmap(rect);

	//Only do this after we have resize the Graphics
	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	if (pView) {
		pView->setWindowSize(rect.Width(), rect.Height());
		pView->draw();
	}
}

void be_DocView::Draw(BRect updateRect) {
	be_Window 		*pBWin;

	pBWin = (be_Window*)Window();
	if (!pBWin || !pBWin->m_pBeOSFrame)
		return;
	
#if defined(USE_BACKING_BITMAP)
	GR_BeOSGraphics 	*pG;
	BBitmap 		*pBitmap;	

	pG = (GR_BeOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG || !(pBitmap = pG->ShadowBitmap()))
		return;
	
	DrawBitmap(pBitmap);
#else
/* 
Things to do to speed this up, make it less flashy:
 - Only invalidate/draw in the update rect 
 - Draw everything to an offscreen buffer/picture
 - Don't erase the background by default 
*/

	//This code path is used for printing
	if (m_pBPicture) {
		DrawPicture(m_pBPicture, BPoint(0,0));
		return;
	}

	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	if (pView) {
		BPicture *mypict;
		BeginPicture(new BPicture);
		
		//Always erase the contents of the rect first
		//NOTE: No need to do this
		//SetHighColor(ViewColor());	
		//FillRect(updateRect);
		
		//The tell AbiWord to draw us ...
		UT_Rect r;
		r.top = updateRect.top;
		r.left = updateRect.left;
		r.width = updateRect.Width();
		r.height = updateRect.Height();
		pView->draw(&r);

		if ((mypict = EndPicture())) {
			DrawPicture(mypict, BPoint(0,0));
			delete mypict;
		}
	}
#endif
}

