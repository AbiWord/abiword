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

#include <stdio.h>
#include <string.h>

#include <Menus.h>

#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_MacString.h"
#include "xap_Types.h"
#include "ev_MacMenu.h"
#include "xap_Mac_ResID.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

/*****************************************************************/

class _menuCallback
{
public:
	_menuCallback (short id, MenuHandle handle);
	~_menuCallback ();

	UT_Vector * items;
	short mId;
	MenuHandle mHandle;	
};

/*****************************************************************/
_menuCallback::_menuCallback (short id, MenuHandle handle)
{
	mId = id;
	mHandle = handle;
	items = new UT_Vector;
}

/*****************************************************************/
_menuCallback::~_menuCallback ()
{
	delete items;
}
	

/*****************************************************************/

EV_MacMenu::EV_MacMenu(XAP_MacApp * pMacApp, XAP_MacFrame * pMacFrame,
						   const char * szMenuLayoutName,
						   const char * szMenuLabelSetName)
	: EV_Menu(pMacApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pMacApp = pMacApp;
	m_pMacFrame = pMacFrame;
	m_hMacMenubar = NULL;
	m_lastSubMenuID = 0;		// submenu have ID between 1-235
	m_lastMenuID = 255;			// menu have > 0 ID, above 256 to not conflict with sub menus.
	
	m_callbacks = new UT_Vector;
}

EV_MacMenu::~EV_MacMenu(void)
{
	_menuCallback * callback;
    if (m_hMacMenubar) {
#if UNIVERSAL_INTERFACES_VERSION <= 0x0330
        ::DisposeHandle (m_hMacMenubar);
#else
        ::DisposeMenuBar (m_hMacMenubar);
#endif
        m_hMacMenubar = NULL;
    }
	if (m_callbacks) {
		while (m_callbacks->getItemCount() > 0) {
			delete (_menuCallback *) (m_callbacks->getFirstItem ());	
			m_callbacks->deleteNthItem (0);
		}
		delete m_callbacks;
	}
}

bool EV_MacMenu::onCommand(XAP_Menu_Id id)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return true iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pMacApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	if (!pAction)
		return false;

	const char * szMethodName = pAction->getMethodName();
	UT_ASSERT(szMethodName);
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pMacApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeMenuMethod(m_pMacFrame->getCurrentView(),pEM,0,0);
	return true;
}



bool EV_MacMenu::synthesizeMenuBar(void)
{
    UT_ASSERT (m_pMacFrame);

	const EV_Menu_ActionSet * pMenuActionSet = m_pMacApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

#if TARGET_API_MAC_CARBON
	OSErr err = ::DuplicateMenuBar (::GetMenuBar(), &m_hMacMenubar);
    UT_ASSERT (err == noErr);
#else
	m_hMacMenubar = ::GetMenuBar();
	::HandToHand (&m_hMacMenubar);
#endif

       
    UT_ASSERT (m_hMacMenubar);

//	::ClearMenuBar (m_hMacMenubar);	

	synthesize ();

	return true;
}


void EV_MacMenu::_convertToMac (char * buf, size_t bufSize, const char * label)
{
	UT_ASSERT(label && buf);
	UT_ASSERT(strlen (label) < bufSize);

	/* TODO: Handle charset conversion */
	strcpy (buf, label);

	char * src, *dst;
	src = dst = buf;
	while (*src)
	{
		*dst = *src;
		src++;
		if (*dst != '&')
			dst++;
	}
	*dst = 0;
}


bool EV_MacMenu::synthesize(void)
{
	int menuType;
	char buf[1024];
	MenuHandle parentMenu;
	_menuCallback * cb;

	// create a Mac menu from the info provided.
	AV_View* pView = m_pMacFrame->getCurrentView();
    
	bool bResult;
    bool bCheck;
	UT_uint32 tmp = 0;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pMacApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);
        
    UT_ASSERT(m_hMacMenubar);
    ::SetMenuBar (m_hMacMenubar);

	UT_Stack stack;
	UT_Stack typeStack;
	UT_Stack cbStack;
	stack.push(m_hMacMenubar);
	typeStack.push ((void *) EV_MAC_MENUBAR);
	cbStack.push (NULL);
	
	m_lastSubMenuID++;
	
	parentMenu = ::GetMenu (RES_MENU_APPLE);
	UT_ASSERT (parentMenu);
	::InsertMenu (parentMenu, 0);			

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++) {
		short currentItem;
		Str255 menuLabel;

		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName;
		const char * szMnemonicName;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{
			const char ** data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			if (szLabelName && *szLabelName)
			{
				// convert label into underscored version
				_convertToMac(buf, sizeof (buf), szLabelName);

				
				if (szMnemonicName && *szMnemonicName)
				{
					/* MAC TODO add the accelerator */
				}
				C2PStr (menuLabel, buf);
			}
			else {
				C2PStr (menuLabel, "");
			}				
			// find parent menu item
			bResult = stack.viewTop((void **)&parentMenu);
			UT_ASSERT(bResult);
			bResult = typeStack.viewTop ((void **)&menuType);
			UT_ASSERT(bResult);
			bResult = cbStack.viewTop ((void **)&cb);
			UT_ASSERT(bResult);
			currentItem = ::CountMenuItems (parentMenu);
			::InsertMenuItem (parentMenu, menuLabel, currentItem + 1);
			cb->items->addItem ((void *)id);
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
			szLabelName = data[0];
			
			if (szLabelName && *szLabelName)
			{
				// convert label into underscored version
				MenuHandle subMenu;
	
				_convertToMac(buf, sizeof (buf), szLabelName);
				C2PStr (menuLabel, buf);
				bResult = stack.viewTop((void **)&parentMenu);
				UT_ASSERT(bResult);
				bResult = typeStack.viewTop ((void **)&menuType);
				UT_ASSERT(bResult);
				UT_ASSERT (m_lastSubMenuID < 235);
				m_lastSubMenuID++;

				if (menuType == EV_MAC_MENU) {
					currentItem = ::CountMenuItems (parentMenu);
					subMenu = ::NewMenu (m_lastSubMenuID, "\p");
					UT_ASSERT (subMenu);
					::InsertMenu (subMenu, -1);
					::InsertMenuItem (parentMenu, menuLabel, currentItem);
					::SetItemMark (parentMenu, currentItem + 1, m_lastSubMenuID);
				}
				else if (menuType == EV_MAC_MENUBAR) {
					subMenu = ::NewMenu (m_lastSubMenuID, menuLabel);
					::InsertMenu (subMenu, 0);			
				}
	
				cb = new _menuCallback (m_lastSubMenuID, subMenu);

				cbStack.push (cb);
				m_callbacks->addItem (cb);
				cb = NULL;
				stack.push(subMenu);
				typeStack.push ((void *)EV_MAC_MENU);
				break;
			}
			// give it a fake, with no label, to make sure it passes the
			// test that an empty (to be replaced) item in the vector should
			// have no children
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			// pop to go on level up
			bResult = stack.pop((void **)&parentMenu);
			UT_ASSERT(bResult);
			bResult = typeStack.pop ((void **)&menuType);
			UT_ASSERT(bResult);
			UT_ASSERT(menuType == EV_MAC_MENU);
			break;
		}
		case EV_MLF_Separator:
		{	
			bResult = stack.viewTop((void **)&parentMenu);
			UT_ASSERT(bResult);
			bResult = typeStack.viewTop ((void **)&menuType);
			UT_ASSERT(bResult);
			UT_ASSERT(menuType == EV_MAC_MENU);
			currentItem = ::CountMenuItems (parentMenu);
			::InsertMenuItem (parentMenu, "\p-", currentItem + 1);
			cb->items->addItem (NULL);
			break;
		}

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	// make sure our last item on the stack is the one we started with
	MenuBarHandle menu = NULL;
	bResult = typeStack.pop ((void **)&menuType);
	UT_ASSERT(bResult);
	bResult = stack.pop((void **)&menu);
	UT_ASSERT(bResult);
	UT_ASSERT(menu == m_hMacMenubar);
	return true;
}


XAP_Menu_Id EV_MacMenu::findMenuId (short menu, short item)
{
	UT_uint32 count = m_callbacks->getItemCount ();
	UT_uint32 i;
	_menuCallback * cb;
	
	for (i = 0; i < count; i++) {
		cb = (_menuCallback *)m_callbacks->getNthItem (i);
		if (cb->mId == menu) {
			XAP_Menu_Id id = (XAP_Menu_Id)cb->items->getNthItem (item - 1);
			return id;
		}
	}
	UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	return 0;
}



