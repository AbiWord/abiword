/* AbiSource Program Utilities
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

#include <windows.h>
#include <commctrl.h>   // includes the common control header

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Win32Toolbar.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_Win32Toolbar_Icons.h"
#include "ev_Win32Toolbar_ViewListener.h"
#include "xav_View.h"
#include "xmlparse.h"
#include "xap_Prefs.h"

#ifndef TBSTYLE_AUTOSIZE
#define TBSTYLE_AUTOSIZE  0x10
#endif

/*****************************************************************/

EV_Win32Toolbar::EV_Win32Toolbar(XAP_Win32App * pWin32App, XAP_Win32Frame * pWin32Frame,
								 const char * szToolbarLayoutName,
								 const char * szToolbarLabelSetName)
	: EV_Toolbar(pWin32App->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName)
{
	m_pWin32App = pWin32App;
	m_pWin32Frame = pWin32Frame;
	m_pViewListener = NULL;
	m_lid = 0;							// view listener id
	m_hwnd = NULL;
}

EV_Win32Toolbar::~EV_Win32Toolbar(void)
{
	_releaseListener();
}

UT_Bool EV_Win32Toolbar::toolbarEvent(XAP_Toolbar_Id id,
									  UT_UCSChar * pData,
									  UT_uint32 dataLength)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	if (!pAction)
		return UT_FALSE;

	AV_View * pView = m_pWin32Frame->getCurrentView();

#if 0
	// make sure we ignore presses on "down" group buttons
	if (pAction->getItemType() == EV_TBIT_GroupButton)
	{
		const char * szState = 0;
		EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

		if (EV_TIS_ShouldBeToggled(tis))
		{
			// if this assert fires, you got a click while the button is down
			// if your widget set won't let you prevent this, handle it here
			UT_ASSERT(UT_TODO);
			
			// can safely ignore this event
			return UT_TRUE;
		}
	}
#endif 

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(pView,pEM,pData,dataLength);
	return UT_TRUE;
}

/*****************************************************************/

// HACK: forward declarations for subclassed controls
#ifdef STRICT   
#define WHICHPROC	WNDPROC
#else   
#define WHICHPROC	FARPROC
#endif

WHICHPROC s_lpfnDefCombo; 
WHICHPROC s_lpfnDefComboEdit; 

#define COMBO_BUF_LEN 256

LRESULT CALLBACK EV_Win32Toolbar::_ComboWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			// relay mouse messages from the combo box to get tool tips to work
			// TODO: similar relay needed from edit control, too??
			// ALT:  tell tooltip control to subclass for these events
			MSG msg;
			HWND hWndTT;
			msg.lParam = lParam;
			msg.wParam = wParam;
			msg.message = uMessage;
			msg.hwnd = hWnd;

			HWND hwndToolbar = GetParent(hWnd);

			hWndTT = (HWND)SendMessage(hwndToolbar, TB_GETTOOLTIPS, 0,0);
			SendMessage(hWndTT, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
			break;
		}

		case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{
				case CBN_SELCHANGE:
				{
					// are we currently dropped down?
					UT_Bool bDropped = (UT_Bool) SendMessage(hWnd, CB_GETDROPPEDSTATE, 0, 0);
					
					if (!bDropped)
						break;

					// yep, we're done 
					SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0);

					UT_sint32 iSelected = SendMessage(hWnd, CB_GETCURSEL, 0, 0);
					
					if(iSelected != -1)
						{
						// now that we know dropdown is gone, this should be ok
						PostMessage(hWnd, WM_KEYDOWN, VK_RETURN, 0);
						}
					else
						{
						PostMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
						}
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN:
		{
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hWnd, GWL_USERDATA);
			UT_ASSERT(t);

			switch (wParam)
			{
				case VK_ESCAPE:
				{
					// restore combo state
					UINT u = GetDlgCtrlID(hWnd);
					XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);

					t->_refreshID(id);
				}
				// fall through

				case VK_TAB:
				case VK_RETURN:
				{
					HWND hwndFrame = t->m_pWin32Frame->getTopLevelWindow();
					SetFocus(hwndFrame);
					return 0;
				}
			}
		}
	}
	return (CallWindowProc(s_lpfnDefCombo, hWnd, uMessage, wParam, lParam));
}

LRESULT CALLBACK EV_Win32Toolbar::_ComboEditWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			// forward these to parent (ie, the combo)
			// TODO: figure out why tooltips still don't work over the edit
			// HYP:  may need to tweak coord system for wParam/lParam
			HWND hwndParent = GetParent(hWnd);
			SendMessage(hwndParent, uMessage, wParam, lParam);
			break;
		}


		case WM_KILLFOCUS:
		{
			// for now, we fire an event any time we lose focus
			// TODO: confirm that this gives the desired behavior
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hWnd, GWL_USERDATA);
			UT_ASSERT(t);

			HWND hwndParent = GetParent(hWnd);
			UINT u = GetDlgCtrlID(hwndParent);
			XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);

			static char buf[COMBO_BUF_LEN];

			UT_UCSChar * pData = (UT_UCSChar *) buf;	// HACK: should be void *
			UT_uint32 dataLength = GetWindowText(hWnd, buf, COMBO_BUF_LEN);
			if(dataLength)
				{
				t->toolbarEvent(id, pData, dataLength);
				}
			else
				{
				SendMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
				}
			break;
		}

		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_ESCAPE:
				case VK_RETURN:
				case VK_TAB:
				{
					// forward these to parent (ie, the combo)
					HWND hwndParent = GetParent(hWnd);
					SendMessage(hwndParent, uMessage, wParam, lParam);
					return 0;
				}
			}
			break;
		}

		case WM_KEYUP:
		case WM_CHAR:
		{
			switch (wParam) 
			{ 
                case VK_TAB:
				case VK_ESCAPE: 
                case VK_RETURN:
					// swallow these
					return 0;
			}
			break;
		}
	}
	return (CallWindowProc(s_lpfnDefComboEdit, hWnd, uMessage, wParam, lParam));
}

/*****************************************************************/

UT_Bool EV_Win32Toolbar::synthesize(void)
{
	// create a toolbar from the info provided.

	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////

	UT_Bool bIcons = UT_TRUE;
	UT_Bool bText = UT_FALSE;
	const XML_Char * szValue = NULL;
	m_pWin32App->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));

	if (UT_XML_stricmp(szValue,"icon") == 0)
	{
		bIcons = UT_TRUE;
		bText = UT_FALSE;
	}
	else if (UT_XML_stricmp(szValue,"text") == 0)
	{
		bIcons = UT_FALSE;
		bText = UT_TRUE;
	}
	else if (UT_XML_stricmp(szValue,"both") == 0)
	{
		bIcons = UT_TRUE;
		bText = UT_TRUE;
	}

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	XAP_Toolbar_ControlFactory * pFactory = m_pWin32App->getControlFactory();
	UT_ASSERT(pFactory);

	HWND hwndParent = m_pWin32Frame->getToolbarWindow();
	if(hwndParent == NULL)
		return UT_FALSE;

	// NOTE: this toolbar will get placed later, by frame or rebar

	m_hwnd = CreateWindowEx(0, 
				TOOLBARCLASSNAME,		// window class name
				(LPSTR) NULL,			// window caption
				WS_CHILD | WS_VISIBLE 
				| WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
				| TBSTYLE_TOOLTIPS | TBSTYLE_FLAT
				| ( bText && !bIcons ? TBSTYLE_LIST : 0 )
				| TBSTYLE_CUSTOMERASE
				| CCS_NOPARENTALIGN | CCS_NODIVIDER
				| CCS_NORESIZE
				,						// window style
				0,						// initial x position
				0,						// initial y position
				0,						// initial x size
				0,						// initial y size
				hwndParent,				// parent window handle
				NULL,					// window menu handle
				m_pWin32App->getInstance(),		// program instance handle
				NULL);					// creation parameters

	UT_ASSERT(m_hwnd);

	SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);  

	// the Windows Common Control Toolbar requires that we set
	// a bitmap size in the toolbar window **before** we actually
	// add any of them.  at this point in the code, we haven't
	// loaded any of the bitmaps yet and thus don't know the maximum
	// size.  we could go thru the layout twice and compute the
	// maxium before calling this, but this seems overkill since
	// we know at compile time what all of the bitmaps are....
	//
	// Now bitmaps are cut down to requested size if to large - HB
	
#define MY_MAXIMUM_BITMAP_X		21
#define MY_MAXIMUM_BITMAP_Y		21
	if( bIcons )
		SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0,
					(LPARAM) MAKELONG(MY_MAXIMUM_BITMAP_X,MY_MAXIMUM_BITMAP_Y));
	else
		SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(0,0));

	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor backgroundColor(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));

	// TODO: is there any advantage to building up all the TBBUTTONs at once
	//		 and then adding them en masse, instead of one at a time? 
	UINT last_id=0;
	UT_Bool bControls = UT_FALSE;

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		UINT u = WmCommandFromItemId(id);

		TBBUTTON tbb;
		memset(&tbb, 0, sizeof(tbb));
		
		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				tbb.idCommand = u;
				tbb.dwData = 0;
				
				last_id = u;

				UT_Bool bButton = UT_FALSE;

				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
					bButton = UT_TRUE;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_BUTTON;     
					break;

				case EV_TBIT_ToggleButton:
					bButton = UT_TRUE;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_CHECK;     
					break;

				case EV_TBIT_GroupButton:
					bButton = UT_TRUE;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_CHECKGROUP;     
					break;

				case EV_TBIT_ComboBox:
					{
						EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
						UT_ASSERT(pControl);

						int iWidth = 100;

						if (pControl)
						{
							iWidth = pControl->getPixelWidth();
						}
						
						bControls = UT_TRUE;
						tbb.fsStyle = TBSTYLE_SEP;   
						tbb.iBitmap = iWidth;

						// create a matching child control
						DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
								CBS_HASSTRINGS | CBS_DROPDOWN;

						if ((pControl) && (pControl->shouldSort()))
						{
							dwStyle |= CBS_SORT;
						}

						HWND hwndCombo = CreateWindowEx ( 0L,   // No extended styles.
							"COMBOBOX",                    // Class name.
							"",                            // Default text.
							dwStyle,                       // Styles and defaults.
							0, 2, iWidth, 250,             // Size and position.
                            m_hwnd,                        // Parent window.
							(HMENU) u,                     // ID.
							m_pWin32App->getInstance(),    // Current instance.
							NULL );                        // No class data.

						UT_ASSERT(hwndCombo);
						
						SendMessage(hwndCombo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

						// populate it
						if (pControl)
						{
							pControl->populate();

							const UT_Vector * v = pControl->getContents();
							UT_ASSERT(v);

							if (v)
							{
								UT_uint32 items = v->getItemCount();
								for (UT_uint32 k=0; k < items; k++)
								{
									char * sz = (char *)v->getNthItem(k);
									SendMessage(hwndCombo, CB_ADDSTRING,(WPARAM)0, (LPARAM)sz);
								}
							}
						}

						// override the window procedure for the combo box
						s_lpfnDefCombo = (WHICHPROC)GetWindowLong(hwndCombo, GWL_WNDPROC);
						SetWindowLong(hwndCombo, GWL_WNDPROC, (LONG)_ComboWndProc);
						SetWindowLong(hwndCombo, GWL_USERDATA, (LONG)this);

						// override the window procedure for its edit control, too
						POINT pt;
						pt.x = 4;
						pt.y = 4; 
			            HWND hwndComboEdit = ChildWindowFromPoint(hwndCombo, pt); 
						UT_ASSERT(hwndComboEdit);
						UT_ASSERT(hwndComboEdit != hwndCombo);
						s_lpfnDefComboEdit = (WHICHPROC)GetWindowLong(hwndComboEdit, GWL_WNDPROC);
						SetWindowLong(hwndComboEdit, GWL_WNDPROC, (LONG)_ComboEditWndProc);
						SetWindowLong(hwndComboEdit, GWL_USERDATA, (LONG)this);

						// Get the handle to the tooltip window.
						HWND hwndTT = (HWND)SendMessage(m_hwnd, TB_GETTOOLTIPS, 0, 0);

						if (hwndTT)
						{
							const char * szToolTip = pLabel->getToolTip();
							if (!szToolTip || !*szToolTip)
							{
								szToolTip = pLabel->getStatusMsg();
							}

							// Fill in the TOOLINFO structure.
							TOOLINFO ti;

							ti.cbSize = sizeof(ti);
							ti.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
							ti.lpszText = (char *) szToolTip;
							ti.hwnd = m_hwnd;		// TODO: should this be the frame?
							ti.uId = (UINT)hwndCombo;
							// Set up tooltips for the combo box.
							SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
						}
						
						// bind this separator to its control
						tbb.dwData = (DWORD) hwndCombo;

						// for now, we never repopulate, so can just toss it
						DELETEP(pControl);
					}
					break;
					
				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT(0);
					break;
				}

				if (bButton)
				{
					// TODO figure out who destroys hBitmap...
					// TODO add code to create these once per application
					// TODO and reference them in each toolbar instance
					// TODO rather than create them once for each window....

					if (bIcons)
					{
						HBITMAP hBitmap;
						UT_Bool bFoundIcon = m_pWin32ToolbarIcons->getBitmapForIcon(m_hwnd,
																					MY_MAXIMUM_BITMAP_X,
																					MY_MAXIMUM_BITMAP_Y,
																					&backgroundColor,
																					pLabel->getIconName(),
																					&hBitmap);
						UT_ASSERT(bFoundIcon);
						TBADDBITMAP ab;
						ab.hInst = 0;
						ab.nID = (LPARAM)hBitmap;
						LRESULT iAddedAt = SendMessage(m_hwnd,TB_ADDBITMAP,1,(LPARAM)&ab);
						UT_ASSERT(iAddedAt != -1);

						tbb.iBitmap = iAddedAt;
					}
					
					if (bText)
					{
						const char * szLabel = pLabel->getToolbarLabel();
						// As long as translators don't cut the text short, we need to autosize just to squize anything insize 1280 :)
						tbb.fsStyle |= TBSTYLE_AUTOSIZE;
						tbb.iString = SendMessage(m_hwnd, TB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPSTR) szLabel);
					}
					
				}
			}
			break;
		
		case EV_TLF_Spacer:
			tbb.fsState = TBSTATE_ENABLED; 
			tbb.fsStyle = TBSTYLE_SEP;     
			break;
			
		default:
			UT_ASSERT(0);
		}

		// add this button to the bar
		SendMessage(m_hwnd, TB_ADDBUTTONS, (WPARAM) 1, (LPARAM) (LPTBBUTTON) &tbb);
	}

	// figure out bar dimensions now that buttons are all there
	SendMessage(m_hwnd, TB_AUTOSIZE, 0, 0); 
	
	if (bControls)
	{
		// move each control on top of its associated separator
		for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
		{
			EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
			UT_ASSERT(pLayoutItem);

			XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
			EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
			UT_ASSERT(pAction);

			RECT r;
			HWND hwndCtrl;
			int nHeight, nSep;

			switch (pAction->getItemType())
			{
				case EV_TBIT_ComboBox:
					hwndCtrl = _getControlWindow(id);
					UT_ASSERT(hwndCtrl);
					GetWindowRect(hwndCtrl, &r);
					nHeight = r.bottom - r.top;

					SendMessage(m_hwnd, TB_GETITEMRECT, (WPARAM) k, (LPARAM)(LPRECT) &r);

					nSep = (r.bottom - r.top - nHeight)/2;
					if (nSep < 0)
						nSep = 0;

					MoveWindow(hwndCtrl, r.left, r.top + nSep, r.right - r.left, nHeight, TRUE);

					break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				default:
					break;
			}
		}
	}

	// Get the height of the toolbar.
	DWORD dwBtnSize = SendMessage(m_hwnd, TB_GETBUTTONSIZE, 0,0);

	// HACK: guess the length of the toolbar
	RECT r;
	UT_ASSERT(last_id > 0);
	SendMessage(m_hwnd, TB_GETRECT, (WPARAM) last_id, (LPARAM)(LPRECT) &r);  
	UINT iWidth = r.right + 13;

	// add this bar to the rebar
	REBARBANDINFO  rbbi;
	ZeroMemory(&rbbi, sizeof(rbbi));
	// Initialize REBARBANDINFO
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_COLORS |	// clrFore and clrBack are valid
		RBBIM_CHILD |				// hwndChild is valid
		RBBIM_CHILDSIZE |			// cxMinChild and cyMinChild are valid
		RBBIM_SIZE |				// cx is valid
		RBBIM_STYLE |				// fStyle is valid
		0;
	rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
	rbbi.fStyle = RBBS_NOVERT |	// do not display in vertical orientation
		/* RBBS_CHILDEDGE | */  // NOT using RBBS_CHILDEDGE gives us a slimmer look, better with the flat button style
		RBBS_BREAK |
		0;
	rbbi.hwndChild = m_hwnd;
	rbbi.cxMinChild = LOWORD(dwBtnSize);
	rbbi.cyMinChild = HIWORD(dwBtnSize);
	rbbi.cx = iWidth;

	// Add it at the the end
	SendMessage(hwndParent, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

	return UT_TRUE;
}

HWND EV_Win32Toolbar::getWindow(void) const
{
	return m_hwnd;
}

HWND EV_Win32Toolbar::_getControlWindow(XAP_Toolbar_Id id)
{
	TBBUTTON tbb;
	HWND hwndCtrl;
	UINT u = WmCommandFromItemId(id);

	UT_uint32 k = SendMessage(m_hwnd, TB_COMMANDTOINDEX, (WPARAM) u, 0);
	SendMessage(m_hwnd, TB_GETBUTTON, (WPARAM) k, (LPARAM)(LPTBBUTTON) &tbb);
	UT_ASSERT(tbb.idCommand == (int) u);
	UT_ASSERT(tbb.fsStyle & TBSTYLE_SEP);

	hwndCtrl = (HWND) tbb.dwData;

	return hwndCtrl;
}

void EV_Win32Toolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
UT_Bool EV_Win32Toolbar::bindListenerToView(AV_View * pView)
{
	if(m_hwnd == NULL)
		return UT_FALSE;

	_releaseListener();
	
	m_pViewListener = new EV_Win32Toolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	UT_Bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	refreshToolbar(pView, AV_CHG_ALL);

	return UT_TRUE;
}

UT_Bool EV_Win32Toolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	if(m_hwnd == NULL)
		return UT_FALSE;

	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				_refreshItem(pView, pAction, id);
			}
			break;
			
		case EV_TLF_Spacer:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	return UT_TRUE;
}

UT_Bool EV_Win32Toolbar::_refreshID(XAP_Toolbar_Id id)
{
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	AV_View * pView = m_pWin32Frame->getCurrentView();
	UT_ASSERT(pView);

	return _refreshItem(pView, pAction, id);
}

UT_Bool EV_Win32Toolbar::_refreshItem(AV_View * pView, const EV_Toolbar_Action * pAction, XAP_Toolbar_Id id)
{
	const char * szState = 0;
	EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);
		
	UINT u = WmCommandFromItemId(id);

	switch (pAction->getItemType())
	{
		case EV_TBIT_PushButton:
			{
				UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);

				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed) ;

				//UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
				//			m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			((bGrayed) ? "disabled" : "enabled")));
			}
			break;
	
		case EV_TBIT_ToggleButton:
		case EV_TBIT_GroupButton:
			{
				UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
				UT_Bool bToggled = EV_TIS_ShouldBeToggled(tis);
				
				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed);
				SendMessage(m_hwnd, TB_CHECKBUTTON, u, (LONG)bToggled);

				//UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
				//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			 ((bGrayed) ? "disabled" : "enabled"),
				//			 ((bToggled) ? "pressed" : "not pressed")));
			}
			break;

		case EV_TBIT_ComboBox:
			{
				UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
				UT_Bool bString = EV_TIS_ShouldUseString(tis);

				HWND hwndCombo = _getControlWindow(id);
				UT_ASSERT(hwndCombo);

				// NOTE: we always update the control even if !bString
				int idx = SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szState);
				if (idx==CB_ERR)
					SetWindowText(hwndCombo, szState);

				//UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
				//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			 ((bGrayed) ? "disabled" : "enabled"),
				//			 ((bString) ? szState : "no state")));
			}
			break;

		case EV_TBIT_EditText:
		case EV_TBIT_DropDown:
		case EV_TBIT_StaticLabel:
			// TODO do these later...
			break;
			
		case EV_TBIT_Spacer:
		case EV_TBIT_BOGUS:
		default:
			UT_ASSERT(0);
			break;
	}

	return UT_TRUE;
}

UT_Bool EV_Win32Toolbar::getToolTip(LPARAM lParam)
{
	UT_ASSERT(lParam);
	LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;

	// who's asking?
	UINT idButton = lpttt->hdr.idFrom;
	XAP_Toolbar_Id id = ItemIdFromWmCommand(idButton);
	
	EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
	if (!pLabel)
		return UT_FALSE;

	// ok, gotcha
	const char * szToolTip = pLabel->getToolTip();
	if (!szToolTip || !*szToolTip)
	{
		szToolTip = pLabel->getStatusMsg();
	}

	// here 'tis
	strncpy(lpttt->lpszText, szToolTip, 80);

	return UT_TRUE;
}
