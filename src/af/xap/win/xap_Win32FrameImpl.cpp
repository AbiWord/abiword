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

#include "ut_debugmsg.h"
#include "xap_ViewListener.h"
#include "ev_EditMethod.h"

#include <limits.h>					/* for INT_MAX */


XAP_Win32FrameImpl::XAP_Win32FrameImpl(XAP_Frame *pFrame) :
	XAP_FrameImpl(pFrame),
	m_hwndFrame(NULL),
	m_dialogFactory(pFrame, pFrame->getApp()),
	m_pWin32Menu(NULL),
	m_pWin32Popup(NULL),
	m_iBarHeight(0),
	m_iRealSizeWidth(0),
	m_iRealSizeHeight(0)
{
}

XAP_Win32FrameImpl::~XAP_Win32FrameImpl()
{
}


#if 0
void XAP_Win32FrameImpl::_startViewAutoUpdater(void) {}
static void XAP_Win32FrameImpl::viewAutoUpdater(UT_Worker *wkr) {}
#endif


bool XAP_Win32FrameImpl::_updateTitle() 
{
	UT_return_val_if_fail(m_hwndFrame, false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	if (!m_pFrame->updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	UT_String sTmp = m_pFrame->getTitle(INT_MAX);
	sTmp += " - ";
	sTmp += m_pFrame->getApp()->getApplicationTitleForTitleBar();
	
	SetWindowText(m_hwndFrame, sTmp.c_str());

	return true;
}

void XAP_Win32FrameImpl::_initialize()
{
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = m_pFrame->getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_Win32Keyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_Win32Mouse(pEEM);
	UT_ASSERT(m_pMouse);
}

bool XAP_Win32FrameImpl::_close()
{
	UT_return_val_if_fail(m_hwndFrame, false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	// NOTE: This may not be the proper place, but it does mean that the
	// last window closed is the one that the window state is stored from.
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
	if (GetWindowPlacement(m_hwndFrame, &wndPlacement))
	{
		m_pFrame->getApp()->setGeometry(wndPlacement.rcNormalPosition.left, 
				wndPlacement.rcNormalPosition.top, 
				wndPlacement.rcNormalPosition.right - wndPlacement.rcNormalPosition.left,
				wndPlacement.rcNormalPosition.bottom - wndPlacement.rcNormalPosition.top,
				wndPlacement.showCmd
				);
	}
	else
	{
		// if failed to get placement then invalidate stored settings
		m_pFrame->getApp()->setGeometry(0,0,0,0,0);
	}
	
	RevokeDragDrop(m_hwndFrame);
	

	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);
	m_hwndFrame = NULL;

	return true;
}

bool XAP_Win32FrameImpl::_raise()
{
	UT_return_val_if_fail(m_hwndFrame, false);
	BringWindowToTop(m_hwndFrame);
	return true;
}

bool XAP_Win32FrameImpl::_show()
{
	UT_return_val_if_fail(m_hwndFrame, false);

	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return true;
}


XAP_DialogFactory * XAP_Win32FrameImpl::_getDialogFactory()
{
	return &m_dialogFactory;
}

EV_Toolbar * XAP_Win32FrameImpl::_newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage)
{
	EV_Win32Toolbar *result = new EV_Win32Toolbar(static_cast<XAP_Win32App *>(app), 
												  static_cast<XAP_Win32Frame *>(frame), 
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

EV_Menu* XAP_Win32FrameImpl::_getMainMenu()
{
	return m_pWin32Menu;
}

// Useful to refresh the size of the Frame.  For instance,
// when the user selects hide statusbar, the Frame has to be
// resized in order to fill the gap leaved by the statusbar
void XAP_Win32FrameImpl::_queue_resize()
{
	::SendMessage(m_hwndFrame, WM_SIZE, 0, MAKELONG(m_iRealSizeWidth, m_iRealSizeHeight));
}


bool XAP_Win32FrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y)
{
	bool bResult = false;

	UT_return_val_if_fail((m_pWin32Popup==NULL), false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	m_pWin32Popup = new EV_Win32MenuPopup(static_cast<XAP_Win32App*>(m_pFrame->getApp()),szMenuName,m_szMenuLabelSetName);
	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup(m_pFrame))
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


void XAP_Win32FrameImpl::_nullUpdate () const
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
//	wndclass.lpfnWndProc		= XAP_Win32FrameImpl::_FrameWndProc;
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

#if 0 // inprogress
LRESULT CALLBACK XAP_Win32Frame::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32Frame * f = (XAP_Win32Frame*)GetWindowLong(hwnd, GWL_USERDATA);

	if (!f)
	{
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	}
	
	AV_View * pView = NULL;

	pView = f->m_pView;

	if(iMsg == f->m_mouseWheelMessage)
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
			SetFocus(f->m_hwndContainer);
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
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onCommand(pView,hwnd,wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onCommand(pView,hwnd,wParam))
		{
			return 0;
		}
		else if (HIWORD(wParam) == 0)
		{
			// after menu passes on it, give each of the toolbars a chance
			UT_uint32 nrToolbars, k;
			nrToolbars = f->m_vecToolbars.getItemCount();
			for (k=0; k < nrToolbars; k++)
			{
				EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
				XAP_Toolbar_Id id = t->ItemIdFromWmCommand(LOWORD(wParam));
				if (t->toolbarEvent(id))
					return 0;
			}
		}
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_INITMENU:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onInitMenu(f,pView,hwnd,(HMENU)wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onInitMenu(f,pView,hwnd,(HMENU)wParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
		
#ifdef MENU_FT
	case WM_MENUSELECT:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
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
				nrToolbars = f->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
					if (t->getToolTip(lParam))
						break;
				}
			}
			break;

		case RBN_HEIGHTCHANGE:
			{
				RECT r;
				GetClientRect(f->m_hwndFrame, &r);
				int nWidth = r.right - r.left;
				int nHeight = r.bottom - r.top;

				GetClientRect(f->m_hwndRebar, &r);
				f->m_iBarHeight = r.bottom - r.top + 6;

				if (f->m_hwndContainer)
				{
					// leave room for the toolbars
					nHeight -= f->m_iBarHeight;

					MoveWindow(f->m_hwndContainer, 0, f->m_iBarHeight, nWidth, nHeight, TRUE);
				}
								
				f->queue_resize();
			}
			break;

		case NM_CUSTOMDRAW:
			{
				LPNMCUSTOMDRAW  pNMcd = (LPNMCUSTOMDRAW)lParam;
				UT_uint32 nrToolbars, k;
				nrToolbars = f->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
					if( pNMcd->hdr.hwndFrom == t->getWindow() )
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
			f->m_iRealSizeHeight = nHeight;
			f->m_iRealSizeWidth = nWidth;
			
			f->_startViewAutoUpdater();

			if (nWidth != (int) f->m_iSizeWidth && f->m_hwndRebar != NULL)
			{
				MoveWindow(f->m_hwndRebar, 0, 0, nWidth, f->m_iBarHeight, TRUE); 
			}

			// leave room for the toolbars and the status bar
			nHeight -= f->m_iBarHeight;

			if (::IsWindowVisible(f->m_hwndStatusBar))
				nHeight -= f->m_iStatusBarHeight;							
			
				
			if (f->m_hwndStatusBar)
				MoveWindow(f->m_hwndStatusBar, 0, f->m_iBarHeight+nHeight, nWidth, f->m_iStatusBarHeight, TRUE);

			if (f->m_hwndContainer)
				MoveWindow(f->m_hwndContainer, 0, f->m_iBarHeight, nWidth, nHeight, TRUE);			
			
			f->m_iSizeWidth = nWidth;
			f->m_iSizeHeight = nHeight;

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
		ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(f->m_pKeyboard);
		pWin32Keyboard->remapKeyboard((HKL)lParam);

		// Do not propagate this message.
		
		return 1; //DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	case WM_MOUSEWHEEL:
	{
		return SendMessage(f->m_hwndContainer, iMsg, wParam, lParam);
	}

	case WM_SYSCOLORCHANGE:
	{
		if (f->m_hwndRebar)
		{
			SendMessage(f->m_hwndRebar,WM_SYSCOLORCHANGE,0,0);

			REBARBANDINFO rbbi = { 0 };
			rbbi.cbSize = sizeof(REBARBANDINFO);
			rbbi.fMask = RBBIM_COLORS;
			rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
			rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

			UT_uint32 nrToolbars = f->m_vecToolbars.getItemCount();
			for (UT_uint32 k=0; k < nrToolbars; k++)
				SendMessage(f->m_hwndRebar, RB_SETBANDINFO,k,(LPARAM)&rbbi);
		}

		if (f->m_hwndContainer)
			SendMessage(f->m_hwndContainer,WM_SYSCOLORCHANGE,0,0);
		if (f->m_hwndStatusBar)
			SendMessage(f->m_hwndStatusBar,WM_SYSCOLORCHANGE,0,0);
		return 0;
	}

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wParam; 
			// How many files were dropped?
			int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			char bufsize[_MAX_PATH];
			int i,pathlength;
			for (i=0; i<count; i++)
			{
				pathlength = DragQueryFile(hDrop, i, NULL, 0);
				if (pathlength < _MAX_PATH)
				{
					DragQueryFile(hDrop, i, bufsize, _MAX_PATH);
					XAP_App * pApp = f->getApp();
					UT_ASSERT(pApp);
					
					XAP_Frame * pNewFrame = 0;

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

					UT_Error error = pNewFrame->loadDocument(bufsize, IEFT_Unknown);
					if (error != UT_OK)
					{
						if (f != pNewFrame)
							pNewFrame->close();
						s_CouldNotLoadFileMessage(f, bufsize, error);
					}
					else
					{
						pNewFrame->show();
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

#endif // inprogress
