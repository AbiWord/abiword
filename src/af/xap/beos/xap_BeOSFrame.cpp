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

#define DPRINTF(x) x

/*****************************************************************/

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*********************************************************/
TFScrollBar::TFScrollBar(XAP_BeOSFrame *pFrame, BRect frame, const char *name,
                         BView *target, float min, float max, 
			 orientation direction)
                        :BScrollBar(frame, name, NULL, min, max, direction) {
        m_pBeOSFrame = pFrame;
}

void TFScrollBar::ValueChanged(float newValue) {
        printf("FRAME: Scrollbar changed \n");
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
	  m_dialogFactory(this)
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
	  m_dialogFactory(this)
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

AP_DialogFactory * XAP_BeOSFrame::getDialogFactory(void)
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

#if 0
	m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
						 m_pBeOSApp->getApplicationTitleForTitleBar());
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pBeOSApp->getApplicationName(),
						   m_pBeOSApp->getApplicationName());

	// TODO get the following values from a preferences or something.
	gtk_container_set_border_width(GTK_CONTAINER(m_wTopLevelWindow), 4);

	// TODO These values should probably be set and read from program
	// TODO preferences.
	// NOTE GTK does not automatically parse -geometry for us, but
	// NOTE we should honor it eventually.
	gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700, 650);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
					   GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
					   GTK_SIGNAL_FUNC(_fe::destroy), NULL);

	// create a VBox inside it.
	
	m_wVBox = gtk_vbox_new(FALSE,0);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	// TODO get the following values from a preferences or something.

// HEY! This is commented out because versions of GTK > than 1.1.5
//	call this gtk_container_SET_border_width() and commenting it out
//  makes it build everywhere, and it doesn't look any different.

//	gtk_container_set_border_width(GTK_CONTAINER(m_wVBox), 0);

/* TF NOTE DONE IN be_Window class
	// synthesize a menu from the info in our base class.

	m_pBeOSMenu = new EV_BeOSMenu(m_pBeOSApp,this,
				      m_szMenuLayoutName, m_szMenuLabelSetName);
	UT_ASSERT(m_pBeOSMenu);
	bResult = m_pBeOSMenu->synthesize();
	UT_ASSERT(bResult);
*/

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.
	gtk_widget_realize(m_wTopLevelWindow);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);

	UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_BeOSToolbar * pBeOSToolbar
			= new EV_BeOSToolbar(m_pBeOSApp,this,
								 (const char *)m_vecToolbarLayoutNames.getNthItem(k),
								 m_szToolbarLabelSetName);
		UT_ASSERT(pBeOSToolbar);
		bResult = pBeOSToolbar->synthesize();
		UT_ASSERT(bResult);

		m_vecBeOSToolbars.addItem(pBeOSToolbar);
	}

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).

	m_wSunkenBox = _createDocumentWindow();
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);

	// TODO decide what to do with accelerators
	// gtk_window_add_accelerator_table(GTK_WINDOW(window), accel);

	gtk_widget_show(m_wVBox);
#endif
	
	// we let our caller decide when to show m_wTopLevelWindow.

	return;
}

UT_Bool XAP_BeOSFrame::close()
{
	//Destroy the top level window
	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::raise()
{
	//GtkWidget * tlw = getTopLevelWindow();
	//UT_ASSERT(tlw);
	//gdk_window_raise(tlw->window);

	return UT_TRUE;
}

UT_Bool XAP_BeOSFrame::show()
{
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

/*********************************************************
 Local Window/View Class Stuff 
*********************************************************/

be_Window::be_Window(XAP_BeOSApp *theApp, XAP_BeOSFrame *theFrame,
 	  	BRect r, char *name, 
		window_look look, window_feel feel, 
 		uint32 flags, uint32 workspace) 
       : BWindow(r, name, look, feel, flags, workspace) {
	
	printf("Setting values to %x and %x \n", theApp, theFrame);
	m_pBeOSFrame = theFrame;
	m_pBeOSApp = theApp;
	printf("Set values to %x and %x \n", m_pBeOSFrame, m_pBeOSApp);
}

bool be_Window::QuitRequested(void) {
	//I need to tell the framework here ...
	return(true);
}

bool be_Window::_createWindow(const char *szMenuLayoutName,
			   const char *szMenuLabelSetName) {
	BRect r;
	
	DPRINTF(printf("ABI_WIN: Creating top level \n"));	
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
		DPRINTF(printf("FRAME: Attaching toolbar # %d\n", k));
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

	//Set up some scrollbars 
	printf("Setting up scroll bars \n");
	r = m_winRectAvailable;
	r.bottom -= B_H_SCROLL_BAR_HEIGHT;
	r.left = r.right - B_V_SCROLL_BAR_WIDTH;
	m_vScroll = new TFScrollBar(m_pBeOSFrame, r, 
				    "VertScroll", NULL, 0, 100, B_VERTICAL);
	r = m_winRectAvailable;
	r.top = r.bottom - B_H_SCROLL_BAR_HEIGHT;
	r.right -= B_V_SCROLL_BAR_WIDTH;
	m_hScroll = new TFScrollBar(m_pBeOSFrame, r, 
				    "HortScroll", NULL, 0, 100, B_HORIZONTAL);
	AddChild(m_hScroll);
	AddChild(m_vScroll);
	m_pBeOSFrame->setScrollBars(m_hScroll, m_vScroll);	
	m_winRectAvailable.bottom -= B_H_SCROLL_BAR_HEIGHT +1;
	m_winRectAvailable.right -= B_V_SCROLL_BAR_WIDTH +1;
	printf("Finished with scroll bars \n");

	//Add the document view in the remaining space
	m_winRectAvailable.PrintToStream();
	m_pbe_DocView = new be_DocView(m_winRectAvailable, "MainDocView", 
				       B_FOLLOW_ALL, B_WILL_DRAW);
	m_pbe_DocView->SetViewColor(0,120, 255);
	//Add the view to both frameworks (Be and Abi)
	AddChild(m_pbe_DocView);
	m_pBeOSFrame->setBeDocView(m_pbe_DocView);	
	
	//Without this we never get any key inputs
  	m_pbe_DocView->MakeFocus(true);   

	return(true);	
}

be_DocView::be_DocView(BRect frame, const char *name, uint32 resizeMask, uint32 flags)
	:BView(frame, name, resizeMask, flags | B_FRAME_EVENTS) {
}

void be_DocView::FrameResized(float new_width, float new_height) {
	be_Window	*pBWin;
	GR_BEOSGraphics *pG;
	
	pBWin = (be_Window *)Window();
	if (!pBWin || !pBWin->m_pBeOSFrame)
		return;
 
	pG = (GR_BEOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG)
		return;

	BRect r = Bounds();
	pG->ResizeBitmap(r);

	//Only do this after we have resize the Graphics
	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	//UT_ASSERT(pView);		
	if (pView) {
		pView->setWindowSize(r.Width(), r.Height());
		pView->draw();
	}
}

void be_DocView::Draw(BRect updateRect) {
	be_Window 		*pBWin;

	pBWin = (be_Window*)Window();
	if (!pBWin || !pBWin->m_pBeOSFrame)
		return;
	
	//DPRINTF(printf("FRAME: Draw "); updateRect.PrintToStream());

#if defined(USE_BACKING_BITMAP)
	GR_BEOSGraphics 	*pG;
	BBitmap 		*pBitmap;	

	pG = (GR_BEOSGraphics *)pBWin->m_pBeOSFrame->Graphics();
	if (!pG || !(pBitmap = pG->ShadowBitmap()))
		return;
	
	DrawBitmap(pBitmap);
#else
	AV_View *pView = pBWin->m_pBeOSFrame->getCurrentView();
	//UT_ASSERT(pView);		
	if (pView) {
		//TODO: Make the update more succinct with a rect
		pView->draw();
	}
#endif
}
