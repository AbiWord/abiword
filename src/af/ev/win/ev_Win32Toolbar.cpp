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
#include "ap_Toolbar_Id.h"
#include "ap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_EditEventMapper.h"
#include "ap_Win32Toolbar_Icons.h"
#include "ev_Win32Toolbar_ViewListener.h"
#include "fv_View.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

EV_Win32Toolbar::EV_Win32Toolbar(AP_Win32App * pWin32App, AP_Win32Frame * pWin32Frame,
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
//	UT_VECTOR_PURGEALL(_wd,m_vecToolbarWidgets);
}

UT_Bool EV_Win32Toolbar::toolbarEvent(AP_Toolbar_Id id)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(m_pWin32Frame->getCurrentView(),pEM,1,0,0);
	return UT_TRUE;
}


UT_Bool EV_Win32Toolbar::synthesize(void)
{
	// create a toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

#ifdef REBAR
	HWND hwndParent = m_pWin32Frame->getToolbarWindow();
#else
	HWND hwndParent = m_pWin32Frame->getTopLevelWindow();
#endif

	// NOTE: this toolbar will get placed later, by frame or rebar

    m_hwnd = CreateWindowEx(0, 
				TOOLBARCLASSNAME,		// window class name
				(LPSTR) NULL,			// window caption
				WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS
				| CCS_NOPARENTALIGN | CCS_NOMOVEY | CCS_NODIVIDER
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
	SendMessage(m_hwnd, TB_SETBUTTONSIZE, 0, (LPARAM) MAKELONG(16,15));  

	// HACK: stuff in the standard icons for now
	TBADDBITMAP ab;
	ab.hInst = HINST_COMMCTRL;		// hinstCommctrl
	ab.nID   = IDB_STD_SMALL_COLOR;	// std bitmaps
	LRESULT iAddedAt = SendMessage(m_hwnd, TB_ADDBITMAP, 15, (LPARAM)&ab);
	UT_ASSERT(iAddedAt != -1);

	// TODO: is there any advantage to building up all the TBBUTTONs at once
	//		 and then adding them en masse, instead of one at a time? 
	UINT last_id=0;

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
#if 0
				// TODO: actually translate XPM into a HBITMAP here
				// NOTE: if the logic's here, apps don't need it, but can't provide BMPs either
				UT_Bool bFoundIcon =
					m_pWin32ToolbarIcons->getPixmapForIcon(wTLW->window,
														  &wTLW->style->bg[GTK_STATE_NORMAL],
														  pLabel->getIconName(),
														  &wPixmap);
				UT_ASSERT(bFoundIcon);
#endif

				// these don't vary much by type
				tbb.iBitmap = STD_HELP;		// HACK: just map to std. icons for now
				tbb.idCommand = u;     
				tbb.dwData = 0; 
				
				last_id = u;

				const char * szLabel = pLabel->getToolbarLabel();
				tbb.iString = SendMessage(m_hwnd, TB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPSTR) szLabel);

				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_BUTTON;     
					break;

				case EV_TBIT_ToggleButton:
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_CHECK;     
					break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_ComboBox:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT(0);
					break;
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

#ifdef REBAR
	RECT r;
	GetWindowRect(m_hwnd, &r);

#if 0
	UT_ASSERT(last_id > 0);
	SendMessage(m_hwnd, TB_GETRECT, (WPARAM) last_id, (LPARAM)(LPRECT) &r);  
#endif

	// add this bar to the rebar
	REBARBANDINFO  rbbi;
	ZeroMemory(&rbbi, sizeof(rbbi));
	// Initialize REBARBANDINFO
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_COLORS |	// clrFore and clrBack are valid
		RBBIM_CHILD |				// hwndChild is valid
		RBBIM_CHILDSIZE |			// cxMinChild and cyMinChild are valid
		RBBIM_STYLE |				// fStyle is valid
//		RBBIM_ID |					// wID is valid
//		RBBIM_TEXT |				// lpText is valid
//		RBBIM_IMAGE |				// iImage is valid
		/* RBBIM_BACKGROUND */ 0;			// hbmBack is valid
	rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
	rbbi.fStyle = RBBS_NOVERT |	// do not display in vertical orientation
		RBBS_CHILDEDGE |
		/* RBBS_FIXEDBMP */ 0;
//	rbbi.hbmBack = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BACK));
//	rbbi.lpText = TEXT("Cool sites:");
	rbbi.hwndChild = m_hwnd;
	rbbi.cxMinChild = r.right - r.left;
	rbbi.cyMinChild = r.bottom - r.top;

	// Add it at the the end
	SendMessage(hwndParent, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
#endif

	return UT_TRUE;
}

HWND EV_Win32Toolbar::getWindow(void) const
{
	return m_hwnd;
}

void EV_Win32Toolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
UT_Bool EV_Win32Toolbar::bindListenerToView(FV_View * pView)
{
	_releaseListener();
	
	m_pViewListener = new EV_Win32Toolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	UT_Bool bResult = pView->addListener(static_cast<FV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	return UT_TRUE;
}

UT_Bool EV_Win32Toolbar::refreshToolbar(FV_View * pView, FV_ChangeMask mask)
{
	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		AP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		FV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...
		
		UINT u = WmCommandFromItemId(id);

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);
			
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

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_ComboBox:
				case EV_TBIT_StaticLabel:
					// TODO do these later...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT(0);
					break;
				}
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

UT_Bool EV_Win32Toolbar::getToolTip(LPARAM lParam)
{
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

	return UT_TRUE;
}
				  
