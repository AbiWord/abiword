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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_MacToolbar.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_MacTlbr_Icons.h"
#include "ev_MacTlbr_ViewListener.h"
#include "xav_View.h"

/*****************************************************************/

EV_MacToolbar::EV_MacToolbar(XAP_MacApp * pMacApp, XAP_MacFrame * pMacFrame,
								 const char * szToolbarLayoutName,
								 const char * szToolbarLabelSetName)
	: EV_Toolbar(pMacApp->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName)
{
	m_pMacApp = pMacApp;
	m_pMacFrame = pMacFrame;
	m_pViewListener = NULL;
	m_lid = 0;							// view listener id
//	m_hwnd = NULL;
}

EV_MacToolbar::~EV_MacToolbar(void)
{
//	_releaseListener();
//	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
}

UT_Bool EV_MacToolbar::toolbarEvent(AP_Toolbar_Id id,
									  UT_UCSChar * pData,
									  UT_uint32 dataLength)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pMacApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	if (!pAction)
		return UT_FALSE;

	AV_View * pView = m_pMacFrame->getCurrentView();

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
	
	const EV_EditMethodContainer * pEMC = m_pMacApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(pView,pEM,pData,dataLength);
	return UT_TRUE;
}

/*****************************************************************/

UT_Bool EV_MacToolbar::synthesize(void)
{
	// create a toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pMacApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	AP_Toolbar_ControlFactory * pFactory = m_pMacApp->getControlFactory();
	UT_ASSERT(pFactory);

#if 0
	HWND hwndParent = m_pMacFrame->getToolbarWindow();

	// NOTE: this toolbar will get placed later, by frame or rebar

    m_hwnd = CreateWindowEx(0, 
				TOOLBARCLASSNAME,		// window class name
				(LPSTR) NULL,			// window caption
				WS_CHILD | WS_VISIBLE 
				| WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
				| TBSTYLE_TOOLTIPS | TBSTYLE_FLAT
				| CCS_NOPARENTALIGN | CCS_NODIVIDER
				| CCS_NORESIZE
				,						// window style
				0,						// initial x position
				0,						// initial y position
				0,						// initial x size
				0,						// initial y size
				hwndParent,				// parent window handle
				NULL,					// window menu handle
				m_pMacApp->getInstance(),		// program instance handle
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
	// so, let's just put in the code to assert if someone adds
	// an overly large bitmap to the source....
	
#define MY_MAXIMUM_BITMAP_X		24
#define MY_MAXIMUM_BITMAP_Y		24
	SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0,
				(LPARAM) MAKELONG(MY_MAXIMUM_BITMAP_X,MY_MAXIMUM_BITMAP_Y));

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

		AP_Toolbar_Id id = pLayoutItem->getToolbarId();
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
						DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE |
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
							m_pMacApp->getInstance(),    // Current instance.
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
					
					HBITMAP hBitmap;
					UT_Bool bFoundIcon = m_pMacToolbarIcons->getBitmapForIcon(m_hwnd,
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
#if 0
					const char * szLabel = pLabel->getToolbarLabel();
					tbb.iString = SendMessage(m_hwnd, TB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPSTR) szLabel);
#endif
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

			AP_Toolbar_Id id = pLayoutItem->getToolbarId();
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
		RBBS_CHILDEDGE | 
		RBBS_BREAK |
		0;
	rbbi.hwndChild = m_hwnd;
	rbbi.cxMinChild = 0;
	rbbi.cyMinChild = HIWORD(dwBtnSize);
	rbbi.cx = iWidth;

	// Add it at the the end
	SendMessage(hwndParent, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
#endif // 0

	return UT_TRUE;
}

WindowPtr EV_MacToolbar::getWindow(void) const
{
	return m_hwnd;
}

UT_Bool EV_MacToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pMacApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		AP_Toolbar_Id id = pLayoutItem->getToolbarId();
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

UT_Bool EV_MacToolbar::_refreshID(AP_Toolbar_Id id)
{
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pMacApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	AV_View * pView = m_pMacFrame->getCurrentView();
	UT_ASSERT(pView);

	return _refreshItem(pView, pAction, id);
}

UT_Bool EV_MacToolbar::_refreshItem(AV_View * pView, const EV_Toolbar_Action * pAction, AP_Toolbar_Id id)
{
	const char * szState = 0;
	EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);
		
#if 0
	UINT u = WmCommandFromItemId(id);

	switch (pAction->getItemType())
	{
		case EV_TBIT_PushButton:
			{
				UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);

				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed) ;

				UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
							m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
							((bGrayed) ? "disabled" : "enabled")));
			}
			break;
	
		case EV_TBIT_ToggleButton:
		case EV_TBIT_GroupButton:
			{
				UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
				UT_Bool bToggled = EV_TIS_ShouldBeToggled(tis);
				
				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed);
				SendMessage(m_hwnd, TB_CHECKBUTTON, u, (LONG)bToggled);

				UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
							 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
							 ((bGrayed) ? "disabled" : "enabled"),
							 ((bToggled) ? "pressed" : "not pressed")));
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

				UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
							 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
							 ((bGrayed) ? "disabled" : "enabled"),
							 ((bString) ? szState : "no state")));
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

#endif // 0

	return UT_TRUE;
}

UT_Bool EV_MacToolbar::getToolTip(long lParam)
{
#if 0
	UT_ASSERT(lParam);
	LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;

	// who's asking?
	UINT idButton = lpttt->hdr.idFrom;
	AP_Toolbar_Id id = ItemIdFromWmCommand(idButton);
	
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
#endif // 0

	return UT_TRUE;
}
