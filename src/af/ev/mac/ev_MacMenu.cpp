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


EV_MacMenu::EV_MacMenu(XAP_MacApp * pMacApp, XAP_MacFrame * pMacFrame,
						   const char * szMenuLayoutName,
						   const char * szMenuLabelSetName)
	: EV_Menu(pMacApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pMacApp = pMacApp;
	m_pMacFrame = pMacFrame;
	m_hMacMenubar = NULL;
	m_lastSubMenuID = 0;		// submenu have ID between 1-235
}

EV_MacMenu::~EV_MacMenu(void)
{
    if (m_hMacMenubar) {
#if UNIVERSAL_INTERFACES_VERSION <= 0x0330
        ::DisposeHandle (m_hMacMenubar);
#else
        ::DisposeMenuBar (m_hMacMenubar);
#endif
        m_hMacMenubar = NULL;
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


char EV_MacMenu::_getItemCmd (const char * mnemonic, UInt8 & modifiers, SInt16 & glyph)
{
	char cmd = 0;
	glyph = 0;
	modifiers = kMenuNoModifiers;
	char * p;
	if (strstr (mnemonic, "Alt+")) {
		modifiers |= kMenuOptionModifier;
	}
	else if (strstr (mnemonic, "Ctrl+") == NULL) {
		modifiers |= kMenuNoCommandModifier;
	}
	if (modifiers & kMenuNoCommandModifier) {
		p = (char *)mnemonic;
	}
	else {
		p = strchr (mnemonic, '+');
		p++;
	}
	if (strlen (p) == 1) {
		return *p;
	}
	else {
		if (strcmp (p, "Del") == 0) {
			glyph = 0x17;
			cmd = kDeleteCharCode;
		}
		else if (strcmp (p, "F4") == 0) {
			glyph = 0x72;
		}
		else if (strcmp (p, "F7") == 0) {
			glyph = 0x75;
		}
	}
	// convert Alt_F4 to cmd-Q to be consistant with HIG.
	if ((glyph == 0x72) && (modifiers == kMenuOptionModifier)) {
		glyph = 0;
		modifiers = kMenuNoModifiers;
		return 'Q';
	}
	return cmd;
}



bool EV_MacMenu::synthesize(void)
{
	OSStatus err;
	int menuType;
	char buf[1024];
	MenuHandle parentMenu;
	UInt8 menuModifiers;
	SInt16 menuGlyph;

	// create a Mac menu from the info provided.   
	bool bResult;
	UT_uint32 tmp = 0;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pMacApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);
        
        UT_ASSERT(m_hMacMenubar);
        ::SetMenuBar (m_hMacMenubar);

	UT_Stack stack;
	UT_Stack typeStack;
	stack.push(m_hMacMenubar);
	typeStack.push ((void *) EV_MAC_MENUBAR);
	
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
			char menuCmd = 0;
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
					menuCmd = _getItemCmd (szMnemonicName, menuModifiers, menuGlyph);
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
			// we only create non empty menus. Otherwise it does not work... (menu manger do not like it).
			// TODO handle the MRU list as it looks like it is the only case were this happens.
			if (menuLabel [0] != 0) {
				currentItem = ::CountMenuItems (parentMenu);
				currentItem++;
				::InsertMenuItem (parentMenu, menuLabel, currentItem);
				if (menuCmd) {
					err = ::SetMenuItemModifiers (parentMenu, currentItem, menuModifiers);
					UT_ASSERT (err == noErr);
					::SetItemCmd (parentMenu, currentItem, menuCmd);
					if (menuGlyph != 0) {
						err = ::SetMenuItemKeyGlyph (parentMenu, currentItem, menuGlyph);
						UT_ASSERT (err == noErr);
					}
				}
				err = ::SetMenuItemCommandID (parentMenu, currentItem, (UInt32)id);
				UT_ASSERT (err == noErr);
			}
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
					::InsertMenu (subMenu, kInsertHierarchicalMenu);
					currentItem++;
					::InsertMenuItem (parentMenu, menuLabel, currentItem);
					err = ::SetMenuItemHierarchicalID (parentMenu, currentItem, m_lastSubMenuID);
					UT_ASSERT (err == noErr);
				}
				else if (menuType == EV_MAC_MENUBAR) {
					subMenu = ::NewMenu (m_lastSubMenuID, menuLabel);
					::InsertMenu (subMenu, 0);
				}
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


//
// Find the XAP_Menu_Id stored for the item #<item> on the menu ID <menu>.
XAP_Menu_Id EV_MacMenu::findMenuId (short menu, short item)
{
	MenuHandle h;
	UInt32 cmd;
	OSStatus err;
	
	h = ::GetMenuHandle (menu);
	UT_ASSERT (h != NULL);
	err = ::GetMenuItemCommandID (h, item, &cmd);
	UT_ASSERT (err == noErr);
	
	if (err == noErr) {
		return (XAP_Menu_Id)cmd;
	}
	UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	return 0;
}



