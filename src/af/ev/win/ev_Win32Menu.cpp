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
#include <stdio.h>
#include <string.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_Win32Menu.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"


#define DELETEP(p)		do { if (p) delete p; } while (0)
#define NrElements(a)	((sizeof(a) / sizeof(a[0])))

/*****************************************************************/

static const char * _ev_GetLabelName(AP_Win32App * pWin32App,
									 XAP_Win32Frame * pWin32Frame,
									 EV_Menu_Action * pAction,
									 EV_Menu_Label * pLabel)
{
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pWin32App,pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return NULL;

	const char * szShortcut = NULL;
	int len = 0;
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();

		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pWin32App->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pWin32Frame->getEditEventMapper();
			UT_ASSERT(pEEM);

			szShortcut = pEEM->getShortcutFor(pEM);
			if (szShortcut && *szShortcut)
				len = strlen(szShortcut) + 1;	// extra char is for the tab

		}
	}
	
	if (pAction->raisesDialog())
		len += 4;

	if (!len)
		return szLabelName;

	static char buf[128];
	memset(buf,0,NrElements(buf));
	strncpy(buf,szLabelName,NrElements(buf)-len);

	// append "..." to menu item if it raises a dialog
	if (pAction->raisesDialog())
		strcat(buf,"...");

	// append shortcut mnemonic, if any
	if (szShortcut && *szShortcut)
	{
		strcat(buf, "\t");
		strcat(buf, szShortcut);
	}

	return buf;
}
	
/*****************************************************************/

EV_Win32Menu::EV_Win32Menu(AP_Win32App * pWin32App, XAP_Win32Frame * pWin32Frame,
						   const char * szMenuLayoutName,
						   const char * szMenuLabelSetName)
	: EV_Menu(pWin32App->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pWin32App = pWin32App;
	m_pWin32Frame = pWin32Frame;
	m_myMenu = NULL;
}

EV_Win32Menu::~EV_Win32Menu(void)
{
}

UT_Bool EV_Win32Menu::onCommand(AV_View * pView,
								HWND hWnd, WPARAM wParam)
{
	// map the windows WM_COMMAND command-id into one of our AP_Menu_Id.
	// we don't need to range check it, getAction() will puke if it's
	// out of range.
	
	AP_Menu_Id id = MenuIdFromWmCommand(LOWORD(wParam));

	// user selected something from the menu.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	if (!pAction)
		return UT_FALSE;

	const char * szMethodName = pAction->getMethodName();
	UT_ASSERT(szMethodName);
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

//	invokeMenuMethod(m_pWin32Frame->getCurrentView(),pEM,0,0);
	invokeMenuMethod(pView,pEM,0,0);
	return UT_TRUE;
}

UT_Bool EV_Win32Menu::synthesize(void)
{
	// create a Win32 menu from the info provided.

	UT_Bool bResult;
	UT_uint32 tmp = 0;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	HWND wTLW = m_pWin32Frame->getTopLevelWindow();

	HMENU menuBar = CreateMenu();

	// we keep a stack of the submenus so that we can properly
	// parent the menu items and deal with nested pull-rights.
	
	UT_Stack stack;
	stack.push(menuBar);
	UT_DEBUGMSG(("MenuBar::synthesize [menuBar 0x%08lx]\n",menuBar));
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item

		const char * szLabelName = _ev_GetLabelName(m_pWin32App,m_pWin32Frame,pAction,pLabel);
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_BeginSubMenu:
			UT_ASSERT(szLabelName && *szLabelName);
			// Fall Thru Intended
		case EV_MLF_Normal:
			{
				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);

				// set standard flags on the item, we'll update the
				// state on an onInitMenu().
				// map our AP_Menu_Id into a windows WM_COMMAND id.
				
				UINT flags = MF_STRING | MF_ENABLED | MF_UNCHECKED;
				UINT u = WmCommandFromMenuId(id);

				if (pLayoutItem->getMenuLayoutFlags() == EV_MLF_BeginSubMenu)
				{
					HMENU sub = CreateMenu();
					UT_ASSERT(sub);

					flags |= MF_POPUP;
					stack.push(sub);
					u = (UINT) sub;
					UT_DEBUGMSG(("menuBar::synthesize [name %s][subMenu 0x%08lx]\n",szLabelName,u));
				}
				else
				{
					UT_DEBUGMSG(("menuBar::synthesize [name %s]\n",szLabelName));
				}

				if (szLabelName && *szLabelName)
					AppendMenu(m, flags, u, szLabelName);
			}
			break;
	
		case EV_MLF_EndSubMenu:
			{
				HMENU m = NULL;
				bResult = stack.pop((void **)&m);
				UT_ASSERT(bResult);
				UT_DEBUGMSG(("menuBar::synthesize [endSubMenu 0x%08lx]\n",m));
			}
			break;
			
		case EV_MLF_Separator:
			{
				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);

				AppendMenu(m, MF_SEPARATOR, 0, NULL);
				UT_DEBUGMSG(("menuBar::synthesize [separator appended to submenu 0x%08lx]\n",m));
			}
			break;

		default:
			UT_ASSERT(0);
			break;
		}
	}

#ifdef UT_DEBUG
	HMENU wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == menuBar);
#endif

	// swap menus for top-level window
	HMENU oldMenu = GetMenu(wTLW);

	if (SetMenu(wTLW, menuBar))
	{
		DrawMenuBar(wTLW);
		m_myMenu = menuBar;

		if (oldMenu)
			DestroyMenu(oldMenu);
	}
	else
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}
	
	return UT_TRUE;
}

UT_Bool EV_Win32Menu::onInitMenu(AV_View * pView, HWND hWnd, HMENU hMenuBar)
{
	// deal with WM_INITMENU.

	if (hMenuBar != m_myMenu)			// these are not different when they
		return UT_FALSE;				// right-click on us on the menu bar.
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_Bool bNeedToRedrawMenu = UT_FALSE;

	UT_uint32 pos = 0;
	UT_Bool bResult;
	UT_Stack stackPos;
	stackPos.push((void*)pos);
	UT_Stack stackMenu;
	stackMenu.push(hMenuBar);

	HMENU mTemp;
	HMENU m = hMenuBar;
	UT_DEBUGMSG(("menuBar::onInitMenu: [menubar 0x%08lx]\n",m));
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);

		UINT cmd = WmCommandFromMenuId(id);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			{
				// see if we need to enable/disable and/or check/uncheck it.
				
				UINT uEnable = MF_BYCOMMAND | MF_ENABLED;
				UINT uCheck = MF_BYCOMMAND | MF_UNCHECKED;
				if (pAction->hasGetStateFunction())
				{
					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray)
						uEnable |= MF_GRAYED;
					if (mis & EV_MIS_Toggled)
						uCheck |= MF_CHECKED;
				}

				if (!pAction->hasDynamicLabel())
				{
					// if no dynamic label, all we need to do
					// is enable/disable and/or check/uncheck it.
					pos++;

					EnableMenuItem(hMenuBar,cmd,uEnable);
					CheckMenuItem(hMenuBar,cmd,uCheck);
					break;
				}

				// get the current menu info for this item.
				
				MENUITEMINFO mif;
				char bufMIF[128];
				mif.cbSize = sizeof(mif);
				mif.dwTypeData = bufMIF;
				mif.cch = NrElements(bufMIF)-1;
				mif.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID;
				BOOL bPresent = GetMenuItemInfo(hMenuBar,cmd,FALSE,&mif);

				// this item has a dynamic label...
				// compute the value for the label.
				// if it is blank, we remove the item from the menu.

				const char * szLabelName = _ev_GetLabelName(m_pWin32App,m_pWin32Frame,pAction,pLabel);

				BOOL bRemoveIt = (!szLabelName || !*szLabelName);

				if (bRemoveIt)			// we don't want it to be there
				{
					if (bPresent)
					{
						RemoveMenu(hMenuBar,cmd,MF_BYCOMMAND);
						bNeedToRedrawMenu = UT_TRUE;
					}
					break;
				}

				// we want the item in the menu.
				pos++;

				if (bPresent)			// just update the label on the item.
				{
					if (strcmp(szLabelName,mif.dwTypeData)==0)
					{
						// dynamic label has not changed, all we need to do
						// is enable/disable and/or check/uncheck it.

						EnableMenuItem(hMenuBar,cmd,uEnable);
						CheckMenuItem(hMenuBar,cmd,uCheck);
					}
					else
					{
						// dynamic label has changed, do the complex modify.
						
						mif.fState = uCheck | uEnable;
						mif.fType = MFT_STRING;
						mif.dwTypeData = (LPTSTR)szLabelName;
						SetMenuItemInfo(hMenuBar,cmd,FALSE,&mif);
						bNeedToRedrawMenu = UT_TRUE;
					}
				}
				else
				{
					// insert new item at the correct location

					mif.fState = uCheck | uEnable;
					mif.fType = MFT_STRING;
					mif.wID = cmd;
					mif.dwTypeData = (LPTSTR)szLabelName;
					InsertMenuItem(m,pos-1,TRUE,&mif);
					bNeedToRedrawMenu = UT_TRUE;
				}
			}
			break;
	
		case EV_MLF_Separator:
			pos++;
			break;

		case EV_MLF_BeginSubMenu:
			mTemp = m;
			pos++;
			stackMenu.push(mTemp);
			stackPos.push((void*)pos);

			m = GetSubMenu(mTemp, pos-1);
			UT_DEBUGMSG(("menuBar::onInitMenu: [menu 0x%08lx] at [pos %d] has [submenu 0x%08lx]\n",
						 mTemp,pos-1,m));
			UT_ASSERT(m);
			pos = 0;
			break;

		case EV_MLF_EndSubMenu:
			bResult = stackMenu.pop((void **)&mTemp);
			UT_ASSERT(bResult);
			bResult = stackPos.pop((void **)&pos);
			UT_ASSERT(bResult);

			UT_DEBUGMSG(("menuBar::onInitMenu: endSubMenu [popping to menu 0x%08lx pos %d] from 0x%08lx\n",
						 mTemp,pos,m));
			m = mTemp;
			break;

		default:
			UT_ASSERT(0);
			break;
		}

	}

#ifdef UT_DEBUG
	HMENU wDbg = NULL;
	bResult = stackMenu.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == hMenuBar);
#endif

	// TODO some of the documentation refers to a need to call DrawMenuBar(hWnd)
	// TODO after any change to it.  other parts of the documentation makes no
	// TODO reference to it.  if you have problems, do something like:
	// TODO
	// TODO if (bNeedToRedrawMenu)
	// TODO 	DrawMenuBar(hWnd);
		
	return UT_TRUE;
}


