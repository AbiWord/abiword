 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <windows.h>
#include <stdio.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ap_Menu_Id.h"
#include "ev_Win32Menu.h"
#include "ap_Win32Ap.h"
#include "ap_Win32Frame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

/*****************************************************************/

static const char * _ev_FakeName(const char * sz, UT_uint32 k)
{
	// construct a temporary string

	static char buf[128];
	sprintf(buf,"%s%ld",sz,k);
	return buf;
}

/*****************************************************************/

EV_Win32Menu::EV_Win32Menu(AP_Win32Ap * pWin32Ap, AP_Win32Frame * pWin32Frame)
	: EV_Menu(pWin32Ap->getEditMethodContainer())
{
	m_pWin32Ap = pWin32Ap;
	m_pWin32Frame = pWin32Frame;
}

EV_Win32Menu::~EV_Win32Menu(void)
{
}

UT_Bool EV_Win32Menu::onCommand(FV_View * pView,
					  HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
// TODO: move this logic out a level?
	WORD cmd = LOWORD(wParam);

	// convert command to menu id
	WORD wid = cmd - WM_USER;
	AP_Menu_Id id = (AP_Menu_Id) wid;
// ==>: then can just pass id like Unix does

	UT_ASSERT(id > AP_MENU_ID__BOGUS1__);
	UT_ASSERT(id < AP_MENU_ID__BOGUS2__);

	// user selected something from the menu.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32Ap->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	UT_ASSERT(szMethodName);
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pWin32Ap->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

//	invokeMenuMethod(m_pWin32Frame->getCurrentView(),pEM,1,0,0);
	invokeMenuMethod(pView,pEM,1,0,0);
	return UT_TRUE;
}

UT_Bool EV_Win32Menu::synthesize(void)
{
	// create a Win32 menu from the info provided.

	UT_Bool bResult;
	UT_uint32 tmp = 0;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32Ap->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	const EV_Menu_LabelSet * pMenuLabelSet = m_pWin32Frame->getMenuLabelSet();
	UT_ASSERT(pMenuLabelSet);
	
	const EV_Menu_Layout * pMenuLayout = m_pWin32Frame->getMenuLayout();
	UT_ASSERT(pMenuLayout);
	
	UT_uint32 nrLabelItemsInLayout = pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	HWND wTLW = m_pWin32Frame->getTopLevelWindow();

	HMENU menuBar = CreateMenu();

	// we keep a stack of the submenus so that we can properly
	// parent the menu items and deal with nested pull-rights.
	
	UT_Stack stack;
	stack.push(menuBar);
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		const char * szLabelName = pAction->getDynamicLabel(m_pWin32Ap);
		if (!szLabelName || !*szLabelName)
			szLabelName = pLabel->getMenuLabel();

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		case EV_MLF_BeginSubMenu:
			{
				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);

				// TODO: do we disable/check here or elsewhere?
				UINT flags = MF_STRING | MF_ENABLED;
				UINT u = id + WM_USER;

				if (pLayoutItem->getMenuLayoutFlags() == EV_MLF_BeginSubMenu)
				{
					HMENU sub = CreateMenu();
					UT_ASSERT(sub);

					flags |= MF_POPUP;

					stack.push(sub);

					u = (UINT) sub;
				}

				AppendMenu(m, flags, u, szLabelName);
			}
			break;
	
		case EV_MLF_EndSubMenu:
			{
				HMENU m = NULL;
				bResult = stack.pop((void **)&m);
				UT_ASSERT(bResult);
			}
			break;
			
		case EV_MLF_Separator:
			{
				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);

				AppendMenu(m, MF_SEPARATOR, 0, NULL);
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

