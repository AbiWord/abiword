/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 2002
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


#include "xap_Win32FrameImpl.h"

#include <commctrl.h>
#include <limits.h>					/* for INT_MAX */

#include "zmouse.h"
#ifdef __MINGW32__
#include "winezmouse.h"
#endif

#include "ut_debugmsg.h"
#include "xap_ViewListener.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xap_prefs.h"
#include "ie_impGraphic.h"
#include "fv_View.h"

#ifdef _MSC_VER
#pragma warning(disable: 4355)	// 'this' used in base member initializer list
#endif

// Where the heck is this function????
// TODO Fix the following header file. It seems to be incomplete
// TODO #include <ap_EditMethods.h>
// TODO In the mean time, define the needed function by hand
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);



XAP_Win32FrameImpl::XAP_Win32FrameImpl(XAP_Frame *pFrame) :
	XAP_FrameImpl(pFrame),
	m_hwndFrame(NULL),
	m_hwndRebar(NULL),
	m_hwndContainer(NULL),
	m_hwndStatusBar(NULL),
	m_dialogFactory(pFrame, XAP_App::getApp()), /* note: pFrame->getApp() not initialized properly yet! */
	m_pWin32Menu(NULL),
	m_pWin32Popup(NULL),
	m_iBarHeight(0),
	m_iStatusBarHeight(0),
	m_iRealSizeWidth(0),
	m_iRealSizeHeight(0),
	m_mouseWheelMessage(0),
	m_iSizeWidth(0),
	m_iSizeHeight(0)
{
}

XAP_Win32FrameImpl::~XAP_Win32FrameImpl(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Menu);
	DELETEP(m_pWin32Popup);
}


#if 0
void XAP_Win32FrameImpl::_startViewAutoUpdater(void) {}
static void XAP_Win32FrameImpl::viewAutoUpdater(UT_Worker *wkr) {}
#endif


bool XAP_Win32FrameImpl::_updateTitle(void) 
{
	UT_return_val_if_fail(m_hwndFrame, false);
	XAP_Frame *pFrame = getFrame();
	UT_return_val_if_fail(pFrame, false);


	if (!XAP_FrameImpl::_updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	UT_String sTmp = pFrame->getTitle(INT_MAX);
	sTmp += " - ";
	sTmp += XAP_App::getApp()->getApplicationTitleForTitleBar();
	
	SetWindowText(m_hwndFrame, sTmp.c_str());

	return true;
}

void XAP_Win32FrameImpl::_initialize(void)
{
	// we assume AP_{FE}Frame has already called XAP_Frame::initialize(...);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = getFrame()->getEditEventMapper();
	UT_return_if_fail(pEEM);

	m_pKeyboard = new ev_Win32Keyboard(pEEM);
	UT_return_if_fail(m_pKeyboard);
	
	m_pMouse = new EV_Win32Mouse(pEEM);
	UT_return_if_fail(m_pMouse);
}

void XAP_Win32FrameImpl::_createTopLevelWindow(void)
{
	RECT r;
	UT_uint32 iHeight, iWidth;
	UT_sint32 iPosX, iPosY;
	static bool firstWindow = true;	/* position only 1st window! */

	// create a top-level window for us.
	// get the default window size from preferences or something.
	// should set size for all, but position only on 1st created
	// TODO determine where to save & restore from Window flag (since
	//      we can't use the geometry flag (its some other junk about validity of pos & size)
	//      so we can properly restore Maximized/Minimized/Normal mode windows

	// get window width & height from preferences
	UT_uint32 t_flag;		// dummy variable
	if ( !(XAP_App::getApp()->getGeometry(&iPosX,&iPosY,&iWidth,&iHeight,&t_flag)) ||
           !((iWidth > 0) && (iHeight > 0)) )
	{
		UT_DEBUGMSG(("Unable to obtain saved geometry, using window defaults!\n"));
		iWidth = CW_USEDEFAULT;
		iHeight = CW_USEDEFAULT;
		iPosX = CW_USEDEFAULT;
		iPosY = CW_USEDEFAULT;
	}
	/* let Windows(R) place the Window for all but 1st one, for stairstep effect */
	if (!firstWindow)
	{
		iPosX = CW_USEDEFAULT;
		iPosY = CW_USEDEFAULT;
	}
	else firstWindow = false;

	UT_DEBUGMSG(("KJD: Window Frame should be %d x %d [width x height]\n", iWidth, iHeight));


	XAP_Win32App *pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());

	m_hwndFrame = CreateWindow(pWin32App->getApplicationName(),
							   pWin32App->getApplicationTitleForTitleBar(),
							   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							   iPosX, iPosY, iWidth, iHeight,
							   NULL, NULL, pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndFrame);


	// bind this frame to its window
	// WARNING: We assume in many places this refers to a XAP_Frame or descendant!!!
	//SetWindowLong(m_hwndFrame, GWL_USERDATA,(LONG)this);
	SetWindowLong(m_hwndFrame, GWL_USERDATA,(LONG)getFrame());

	m_mouseWheelMessage = RegisterWindowMessage(MSH_MOUSEWHEEL);

	// synthesize a menu from the info in our
	// base class and install it into the window.
	m_pWin32Menu = new EV_Win32MenuBar(pWin32App,
							getFrame()->getEditEventMapper(),
							m_szMenuLayoutName,
							m_szMenuLabelSetName);
	UT_ASSERT(m_pWin32Menu);
	bool bResult = m_pWin32Menu->synthesizeMenuBar(getFrame());
	UT_ASSERT(bResult);

	HMENU oldMenu = GetMenu(m_hwndFrame);
	if (SetMenu(m_hwndFrame, m_pWin32Menu->getMenuHandle()))
	{
		DrawMenuBar(m_hwndFrame);
		if (oldMenu)
			DestroyMenu(oldMenu);
	}

	// create a rebar container for all the toolbars
	m_hwndRebar = CreateWindowEx(0L, REBARCLASSNAME, NULL,
								 WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
								 WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
								 RBS_VARHEIGHT | RBS_BANDBORDERS,
								 0, 0, 0, 0,
								 m_hwndFrame, NULL, pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndRebar);

	// create a toolbar instance for each toolbar listed in our base class.

	_createToolbars();

	// figure out how much room is left for the child
	GetClientRect(m_hwndFrame, &r);
	iHeight = r.bottom - r.top;
	iWidth = r.right - r.left;

	m_iSizeWidth = iWidth;
	m_iSizeHeight = iHeight;
	
	// force rebar to resize itself
	// TODO for some reason, we give REBAR the height of the FRAME
	// TODO and let it decide how much it actually needs....
	if( m_hwndRebar != NULL )
	{
		MoveWindow(m_hwndRebar, 0, 0, iWidth, iHeight, TRUE);

		GetClientRect(m_hwndRebar, &r);
		m_iBarHeight = r.bottom - r.top + 6;

		UT_ASSERT(iHeight > m_iBarHeight);
		iHeight -= m_iBarHeight;
	}
	else
		m_iBarHeight = 0;

	m_hwndContainer = _createDocumentWindow(getFrame(), m_hwndFrame, 0, m_iBarHeight, iWidth, iHeight);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_hwndStatusBar = _createStatusBarWindow(getFrame(), m_hwndFrame,0,m_iBarHeight+iHeight,iWidth);
	GetClientRect(m_hwndStatusBar,&r);
	m_iStatusBarHeight = r.bottom;

	
	// Register drag and drop data and files
	m_dropTarget.setFrame(getFrame());
	RegisterDragDrop(m_hwndFrame, &m_dropTarget);	
		
	return;
}

bool XAP_Win32FrameImpl::_close(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);

	// NOTE: This may not be the proper place, but it does mean that the
	// last window closed is the one that the window state is stored from.
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
	if (GetWindowPlacement(m_hwndFrame, &wndPlacement))
	{
		XAP_App::getApp()->setGeometry(wndPlacement.rcNormalPosition.left, 
				wndPlacement.rcNormalPosition.top, 
				wndPlacement.rcNormalPosition.right - wndPlacement.rcNormalPosition.left,
				wndPlacement.rcNormalPosition.bottom - wndPlacement.rcNormalPosition.top,
				/* flag is meant for info about the position & size info stored, not a generic flag
				 * TODO: figure out where to store this then, so we can max/min/normal again
				wndPlacement.showCmd
				*/
				PREF_FLAG_GEOMETRY_POS | PREF_FLAG_GEOMETRY_SIZE
		);
	}
	else
	{
		// if failed to get placement then invalidate stored settings
		XAP_App::getApp()->setGeometry(0,0,0,0,0);
	}
	
	RevokeDragDrop(m_hwndFrame);
	

	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);
	m_hwndFrame = NULL;

	return true;
}

bool XAP_Win32FrameImpl::_raise(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);
	BringWindowToTop(m_hwndFrame);
	return true;
}

bool XAP_Win32FrameImpl::_show(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);

	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return true;
}


XAP_DialogFactory * XAP_Win32FrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

EV_Toolbar * XAP_Win32FrameImpl::_newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage)
{
	EV_Win32Toolbar *result = new EV_Win32Toolbar(static_cast<XAP_Win32App *>(app), 
												  frame, 
												  szLayout, szLanguage);
	// for now, position each one manually
	// TODO: put 'em all in a rebar instead
	HWND hwndBar = result->getWindow();
	
	RECT rcClient;
	GetClientRect(hwndBar, &rcClient);
	const UT_uint32 iHeight = rcClient.bottom - rcClient.top;
	
	m_iBarHeight += iHeight;

	return result;
}

EV_Menu* XAP_Win32FrameImpl::_getMainMenu(void)
{
	return m_pWin32Menu;
}

// Useful to refresh the size of the Frame.  For instance,
// when the user selects hide statusbar, the Frame has to be
// resized in order to fill the gap leaved by the statusbar
void XAP_Win32FrameImpl::_queue_resize(void)
{
	::SendMessage(m_hwndFrame, WM_SIZE, 0, MAKELONG(m_iRealSizeWidth, m_iRealSizeHeight));
}


bool XAP_Win32FrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y)
{
	bool bResult = false;

	UT_return_val_if_fail((m_pWin32Popup==NULL), false);

	m_pWin32Popup = new EV_Win32MenuPopup(static_cast<XAP_Win32App*>(XAP_App::getApp()),szMenuName,m_szMenuLabelSetName);
	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup(getFrame()))
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));

		_translateDocumentToScreen(x,y);

		TrackPopupMenu(m_pWin32Popup->getMenuHandle(),
					   TPM_CENTERALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
					   x,y,0,m_hwndFrame,NULL);

		// the popup steals our capture, so we need to reset our counter.
		EV_Win32Mouse *pWin32Mouse = static_cast<EV_Win32Mouse *>(m_pMouse);
		pWin32Mouse->reset();
	}

	DELETEP(m_pWin32Popup);
	return bResult;
}

void XAP_Win32FrameImpl::_setFullScreen(bool isFullScreen)
{
	// TODO: currently does nothing
}


bool XAP_Win32FrameImpl::_openURL(const char * szURL)
{
	// NOTE: could get finer control over browser window via DDE 
	// NOTE: may need to fallback to WinExec for old NSCP versions

	int res = (int) ShellExecute(m_hwndFrame /*(HWND) top level window */, "open", szURL, NULL, NULL, SW_SHOWNORMAL);

	// TODO: more specific (and localized) error messages ??
	if (res <= 32)	// show error message if failed to launch browser to display URL
	{
		UT_String errMsg = "Error ("; 
		errMsg += res;  errMsg += ") displaying URL: \n";
		errMsg += " [ ";  errMsg += szURL;  errMsg += " ] ";
		MessageBox(m_hwndFrame, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
	}

	return (res>32);
}


void XAP_Win32FrameImpl::_nullUpdate (void) const
{
	MSG msg;
	for( int i = 0 ; i < 10 ; i++ )
	{
		if( PeekMessage( &msg, (HWND) NULL, 0, 0, PM_REMOVE) )
		{
			DispatchMessage(&msg); 
		} 
	}
}

void XAP_Win32FrameImpl::_setCursor(GR_Graphics::Cursor cursor)
{
	// TODO: currently does nothing
}

bool XAP_Win32FrameImpl::_RegisterClass(XAP_Win32App * app)
{
	WNDCLASSEX wndclass;

	// register class for the frame window
	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= CS_DBLCLKS;
	wndclass.lpfnWndProc		= XAP_Win32FrameImpl::_FrameWndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= app->getInstance();
	wndclass.hIcon			= app->getIcon();
	wndclass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wndclass.lpszMenuName		= NULL;
	wndclass.lpszClassName		= app->getApplicationName();
	wndclass.hIconSm			= app->getSmallIcon();

	ATOM a = RegisterClassEx(&wndclass);
	UT_return_val_if_fail(a, false);

	return true;
}

/*****************************************************************/

#ifdef MENU_FT
HFONT GetAFont(int fnFont) 
{ 
    static LOGFONT lf;  // structure for font information  
 
    // Get a handle to the ANSI fixed-pitch font, and copy 
    // information about the font to a LOGFONT structure. 
 
    GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), 
        &lf); 
 
    // Set the font attributes, as appropriate.  
 
    //if (fnFont == BOLD) 
    lf.lfWeight = FW_BOLD; 
    //else 
    //    lf.lfWeight = FW_NORMAL; 
 
    //lf.lfItalic = (fnFont == ITALIC); 
    //lf.lfItalic = 0; 
 
    // Create the font, and then return its handle.  
 
    return CreateFont(lf.lfHeight, lf.lfWidth, 
        lf.lfEscapement, lf.lfOrientation, lf.lfWeight, 
        lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, 
        lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, 
        lf.lfPitchAndFamily, lf.lfFaceName); 
} 

HFONT	gFont = NULL;
LPMEASUREITEMSTRUCT lpmis;  // pointer to item of data             
LPDRAWITEMSTRUCT lpdis;  
SIZE size;                  // menu-item text extents             
    WORD wCheckX;               // check-mark width                   
    int nTextX;                 // width of menu item                 
    int nTextY;                 // height of menu item                
    int i;                      // loop counter                       
    HFONT hfontOld;             // handle to old font      

#endif

LRESULT CALLBACK XAP_Win32FrameImpl::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	XAP_Frame * f = (XAP_Frame*)GetWindowLong(hwnd, GWL_USERDATA);

	if (!f)
	{
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	}
	
	XAP_Win32FrameImpl * fimpl = static_cast<XAP_Win32FrameImpl *>(f->getFrameImpl());
	UT_return_val_if_fail(fimpl, DefWindowProc(hwnd,iMsg,wParam,lParam));

	AV_View * pView = NULL;

	pView = f->getCurrentView();

	if(iMsg == fimpl->m_mouseWheelMessage)
	{
		wParam = MAKEWPARAM(0, (short)(int)wParam);
		return SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
	}

	switch (iMsg)
	{
	case WM_EXITMENULOOP:
	case WM_SETFOCUS:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_HERE);
			SetFocus(fimpl->m_hwndContainer);
		}
		return 0;

	case WM_ENTERMENULOOP:
	case WM_KILLFOCUS:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_NONE);
		}
		return 0;

	case WM_CREATE:
		return 0;

	case WM_COMMAND:
		if (fimpl->m_pWin32Popup)
		{
			if (fimpl->m_pWin32Popup->onCommand(pView,hwnd,wParam))
				return 0;
		}
		else if (fimpl->m_pWin32Menu->onCommand(pView,hwnd,wParam))
		{
			return 0;
		}
		else if (HIWORD(wParam) == 0)
		{
			// after menu passes on it, give each of the toolbars a chance
			UT_uint32 nrToolbars, k;
			nrToolbars = fimpl->m_vecToolbars.getItemCount();
			for (k=0; k < nrToolbars; k++)
			{
				EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
				XAP_Toolbar_Id id = t->ItemIdFromWmCommand(LOWORD(wParam));
				if (t->toolbarEvent(id))
					return 0;
			}
		}
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_INITMENU:
		if (fimpl->m_pWin32Popup)
		{
			if (fimpl->m_pWin32Popup->onInitMenu(f,pView,hwnd,(HMENU)wParam))
				return 0;
		}
		else if (fimpl->m_pWin32Menu->onInitMenu(f,pView,hwnd,(HMENU)wParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
		
#ifdef MENU_FT
	case WM_MENUSELECT:
		if (fimpl->m_pWin32Popup)
		{
			if (fimpl->m_pWin32Popup->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
				return 0;
		}
		else if (fimpl->m_pWin32Menu->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
		
	case WM_MEASUREITEM: 
 	{
        // Retrieve a device context for the main window.  
        HDC hdc = GetDC(hwnd); 

        // Retrieve pointers to the menu item's 
        // MEASUREITEMSTRUCT structure and MYITEM structure. 

        lpmis = (LPMEASUREITEMSTRUCT) lParam; 
        //pmyitem = (MYITEM *) lpmis->itemData; 
        char *pText = (char*) lpmis->itemData;

        // Select the font associated with the item into 
        // the main window's device context. 
        
        if (!gFont)
        	gFont = GetAFont(0); 

        hfontOld = (HFONT) SelectObject(hdc, gFont); 

        // Retrieve the width and height of the item's string, 
        // and then copy the width and height into the 
        // MEASUREITEMSTRUCT structure's itemWidth and 
        // itemHeight members. 

        GetTextExtentPoint32(hdc, pText, 
            lstrlen(pText), &size); 
            
        lpmis->itemWidth = size.cx; 
        lpmis->itemHeight = size.cy; 

        // Select the old font back into the device context, 
        // and then release the device context. 

        SelectObject(hdc, hfontOld); 
        ReleaseDC(hwnd, hdc); 

        return TRUE; 
 	}
 	
 	case WM_DRAWITEM: 
 	{
            // Get pointers to the menu item's DRAWITEMSTRUCT 
            // structure and MYITEM structure. 
 
            lpdis = (LPDRAWITEMSTRUCT) lParam; 
            //pmyitem = (MYITEM *) lpdis->itemData; 
            char *pText = (char*) lpmis->itemData;
            
           	BOOL fSelected = false;
           	
           	COLORREF crText;            // text color of unselected item      
    		COLORREF crBkgnd;           // background color unselected item
    		static COLORREF crSelText;  // text color of selected item        
    		static COLORREF crSelBkgnd;
 
            // If the user has selected the item, use the selected 
            // text and background colors to display the item. 
 
            if (lpdis->itemState & ODS_SELECTED) 
            { 
                crText = SetTextColor(lpdis->hDC, crSelText); 
                crBkgnd = SetBkColor(lpdis->hDC, crSelBkgnd); 
                fSelected = TRUE; 
            } 
 
            // Remember to leave space in the menu item for the 
            // check-mark bitmap. Retrieve the width of the bitmap 
            // and add it to the width of the menu item. 
 
            wCheckX = GetSystemMetrics(SM_CXMENUCHECK); 
            nTextX = wCheckX + lpdis->rcItem.left; 
            nTextY = lpdis->rcItem.top; 
 
            // Select the font associated with the item into the 
            // item's device context, and then draw the string. 
 
            hfontOld = (HFONT) SelectObject(lpdis->hDC, gFont); 
            ExtTextOut(lpdis->hDC, nTextX, nTextY, ETO_OPAQUE, 
                &lpdis->rcItem, pText, 
                lstrlen(pText), NULL); 
 
            // Select the previous font back into the device 
            // context. 
 
            SelectObject(lpdis->hDC, hfontOld); 
 
            // Return the text and background colors to their 
            // normal state (not selected). 
 
            if (fSelected) 
            { 
                SetTextColor(lpdis->hDC, crText); 
                SetBkColor(lpdis->hDC, crBkgnd); 
            } 
 
	         return TRUE; 
		} 
#endif		

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) 
		{
		case TTN_NEEDTEXT:
			{
				UT_uint32 nrToolbars, k;
				nrToolbars = fimpl->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
					if (t->getToolTip(lParam))
						break;
				}
			}
			break;

		case RBN_HEIGHTCHANGE:
			{
				RECT r;
				GetClientRect(fimpl->m_hwndFrame, &r);
				int nWidth = r.right - r.left;
				int nHeight = r.bottom - r.top;

				GetClientRect(fimpl->m_hwndRebar, &r);
				fimpl->m_iBarHeight = r.bottom - r.top + 6;

				if (fimpl->m_hwndContainer)
				{
					// leave room for the toolbars
					nHeight -= fimpl->m_iBarHeight;

					MoveWindow(fimpl->m_hwndContainer, 0, fimpl->m_iBarHeight, nWidth, nHeight, TRUE);
				}
								
				f->queue_resize();
			}
			break;

		case NM_CUSTOMDRAW:
			{
				LPNMCUSTOMDRAW  pNMcd = (LPNMCUSTOMDRAW)lParam;
				UT_uint32 nrToolbars, k;
				nrToolbars = fimpl->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
					if( t && (pNMcd->hdr.hwndFrom == t->getWindow()) )
					{
						if( pNMcd->dwDrawStage == CDDS_PREPAINT )
						{
							return CDRF_NOTIFYPOSTPAINT;
						}
						if( pNMcd->dwDrawStage == CDDS_POSTPAINT )
						{
							RECT  rc;
							HBRUSH	hBr = NULL;

							rc.top    = pNMcd->rc.top;
							rc.bottom = pNMcd->rc.bottom;
							hBr = GetSysColorBrush( COLOR_3DFACE );

							HWND  hWndChild = FindWindowEx( pNMcd->hdr.hwndFrom, NULL, NULL, NULL );
							while( hWndChild != NULL )
							{
								RECT   rcChild;
								POINT  pt;
								GetWindowRect( hWndChild, &rcChild );
								pt.x = rcChild.left;
								pt.y = rcChild.top;
								ScreenToClient( pNMcd->hdr.hwndFrom, &pt );
								rc.left = pt.x;
								pt.x = rcChild.right;
								pt.y = rcChild.bottom;
								ScreenToClient( pNMcd->hdr.hwndFrom, &pt );
								rc.right = pt.x;
								FillRect( pNMcd->hdc, &rc, hBr );
								hWndChild = FindWindowEx( pNMcd->hdr.hwndFrom, hWndChild, NULL, NULL );
							}

							if( hBr != NULL )
							{
								DeleteObject( hBr );
								hBr = NULL;
							}
						}
						break;
					}
				}
			}
			break;

		// Process other notifications here
		default:
			break;
		} 
		break;

	case WM_SIZE:
	{
		
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);

		UT_DEBUGMSG ((("xap_Win32Frame::WM_SIZE %u - %u\n"), nWidth, nHeight));
		
		if( pView && !pView->isLayoutFilling() )
		{
			fimpl->m_iRealSizeHeight = nHeight;
			fimpl->m_iRealSizeWidth = nWidth;
			
			fimpl->_startViewAutoUpdater();

			if (nWidth != (int) fimpl->m_iSizeWidth && fimpl->m_hwndRebar != NULL)
			{
				MoveWindow(fimpl->m_hwndRebar, 0, 0, nWidth, fimpl->m_iBarHeight, TRUE); 
			}

			// leave room for the toolbars and the status bar
			nHeight -= fimpl->m_iBarHeight;

			if (::IsWindowVisible(fimpl->m_hwndStatusBar))
				nHeight -= fimpl->m_iStatusBarHeight;							
			
				
			if (fimpl->m_hwndStatusBar)
				MoveWindow(fimpl->m_hwndStatusBar, 0, fimpl->m_iBarHeight+nHeight, nWidth, fimpl->m_iStatusBarHeight, TRUE);

			if (fimpl->m_hwndContainer)
				MoveWindow(fimpl->m_hwndContainer, 0, fimpl->m_iBarHeight, nWidth, nHeight, TRUE);			
			
			fimpl->m_iSizeWidth = nWidth;
			fimpl->m_iSizeHeight = nHeight;

			f->updateZoom();
		}

		return 0;
	}

	case WM_CLOSE:
	{
		XAP_App * pApp = f->getApp();
		UT_ASSERT(pApp);

		const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
		UT_ASSERT(pEMC);

		EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
		UT_ASSERT(pEM);						// make sure it's bound to something

		if (pEM)
		{
			pEM->Fn(pView,NULL);
			return 0;
		}

		// let the window be destroyed
		break;
	}

	case WM_INPUTLANGCHANGE:
	{
		UT_DEBUGMSG(("Frame received input language change\n"));

		// This will remap the static tables used by all frames.
		// (see the comment in ev_Win32Keyboard.cpp.)
		ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fimpl->m_pKeyboard);
		pWin32Keyboard->remapKeyboard((HKL)lParam);

		// Do not propagate this message.
		
		return 1; //DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	case WM_MOUSEWHEEL:
	{
		return SendMessage(fimpl->m_hwndContainer, iMsg, wParam, lParam);
	}

	case WM_SYSCOLORCHANGE:
	{
		if (fimpl->m_hwndRebar)
		{
			SendMessage(fimpl->m_hwndRebar,WM_SYSCOLORCHANGE,0,0);

			REBARBANDINFO rbbi = { 0 };
			rbbi.cbSize = sizeof(REBARBANDINFO);
			rbbi.fMask = RBBIM_COLORS;
			rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
			rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

			UT_uint32 nrToolbars = fimpl->m_vecToolbars.getItemCount();
			for (UT_uint32 k=0; k < nrToolbars; k++)
				SendMessage(fimpl->m_hwndRebar, RB_SETBANDINFO,k,(LPARAM)&rbbi);
		}

		if (fimpl->m_hwndContainer)
			SendMessage(fimpl->m_hwndContainer,WM_SYSCOLORCHANGE,0,0);
		if (fimpl->m_hwndStatusBar)
			SendMessage(fimpl->m_hwndStatusBar,WM_SYSCOLORCHANGE,0,0);
		return 0;
	}

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wParam; 
			// How many files were dropped?
			int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			char szFileName[_MAX_PATH];
			int i,pathlength;
			for (i=0; i<count; i++)
			{
				pathlength = DragQueryFile(hDrop, i, NULL, 0);
				if (pathlength < _MAX_PATH)
				{
					DragQueryFile(hDrop, i, szFileName, _MAX_PATH);
					XAP_App * pApp = f->getApp();
					UT_ASSERT(pApp);
					FV_View* pView = (FV_View *) f->getCurrentView();
					XAP_Frame * pNewFrame = 0;
					IEGraphicFileType iegft = IEGFT_Unknown;					
					IE_ImpGraphic *pIEG;
					FG_Graphic* pFG;
					UT_Error errorCode;

					/*
						The user may be dropping any kind of file
						Check first if the file is a graphic. If it's a graphic we insert it 
						in the document, if not we assume that it's a document 		
					*/								
					// If there is no import graphic, it's a document...
					errorCode = IE_ImpGraphic::constructImporter(szFileName, iegft, &pIEG);
					if(errorCode == UT_OK)
					{						
						errorCode = pIEG->importGraphic(szFileName, &pFG);
						if(errorCode != UT_OK || !pFG)
						{
							s_CouldNotLoadFileMessage(f, szFileName, errorCode);							
							DELETEP(pIEG);
							return 0;
						}

						DELETEP(pIEG);					

						errorCode = pView->cmdInsertGraphic(pFG, szFileName);
						if (errorCode != UT_OK)
						{
							s_CouldNotLoadFileMessage(f, szFileName, errorCode);							
							DELETEP(pFG);
							return 0;
						}
						
						DELETEP(pFG);
					  }
					else
					{	
						// Check if the current document is empty.
						if (f->isDirty() || f->getFilename() ||
							(f->getViewNumber() > 0))
						{
							pNewFrame = pApp->newFrame();
							if (pNewFrame == NULL)
							{
								f->setStatusMessage("Could not open another window");
								return 0;
							}
						}
						else
						{
							pNewFrame = f;
						}

						UT_Error error = pNewFrame->loadDocument(szFileName, IEFT_Unknown);
						if (error != UT_OK)
						{
							if (f != pNewFrame)
								pNewFrame->close();
							s_CouldNotLoadFileMessage(f, szFileName, error);
						}
						else
						{
							pNewFrame->show();
						}
					  }
				}
				else
				{
				}
			}
			DragFinish(hDrop);
		}
		return 0;

	case WM_DESTROY:
		return 0;

	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

