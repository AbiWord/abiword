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
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

/*****************************************************************/

#if 0
static const char * _ev_GetLabelName(XAP_MacApp * pMacApp,
									 XAP_MacFrame * pMacFrame,
									 EV_Menu_Action * pAction,
									 EV_Menu_Label * pLabel)
{
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pMacFrame,pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return NULL;
	
	if (!pAction->raisesDialog())
		return szLabelName;

	// append "..." to menu item if it raises a dialog

	static char buf[128];
	memset(buf,0,NrElements(buf));
	strncpy(buf,szLabelName,NrElements(buf)-4);
	strcat(buf,"...");
	return buf;
}
#endif

	
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

bool EV_MacMenu::onCommand(AV_View * pView, 
								WindowPtr hWnd, int wParam)
{
	// map the windows WM_COMMAND command-id into one of our XAP_Menu_Id.
	// we don't need to range check it, getAction() will puke if it's
	// out of range.
	
	XAP_Menu_Id id = LoWord(wParam);

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

//	invokeMenuMethod(m_pMacFrame->getCurrentView(),pEM,1,0,0);
	invokeMenuMethod(pView,pEM,0,0);
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


void EV_MacMenu::_convertToMac (char * buf, const char * label)
{
	UT_ASSERT(label && buf);

	/* TODO: Handle charset conversion */
	strcpy (buf, label);

	char * pl = buf;
	while (*pl)
	{
		if (*pl == '&')
			*pl = '_';
		pl++;
	}
}


bool EV_MacMenu::synthesize(void)
{
	
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
	stack.push(m_hMacMenubar);
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++) {
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
			short currentItem;
			Str255 menuLabel;
			const char ** data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024];
				// convert label into underscored version
				_convertToMac(buf, szLabelName);

				
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
			MenuHandle parentMenu;
			bResult = stack.viewTop((void **)&parentMenu);
			UT_ASSERT(bResult);
			currentItem = ::CountMenuItems (parentMenu);
			::InsertMenuItem (parentMenu, menuLabel, currentItem + 1);
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			short currentItem;
			Str255 menuLabel;
			const char ** data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
			szLabelName = data[0];
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024];
				// convert label into underscored version
				_convertToMac(buf, szLabelName);

				MenuHandle subMenu;
				MenuHandle parentMenu;
				bResult = stack.viewTop((void **)&parentMenu);
				UT_ASSERT(bResult);
				currentItem = ::CountMenuItems (parentMenu);

				UT_ASSERT (m_lastSubMenuID < 235);
				subMenu = ::NewMenu (m_lastSubMenuID + 1, "\p");
				UT_ASSERT (subMenu);
				m_lastSubMenuID++;
				::InsertMenu (subMenu, -1);
				::InsertMenuItem (parentMenu, menuLabel, currentItem);
				::SetItemMark (parentMenu, currentItem + 1, m_lastSubMenuID);

				stack.push(subMenu);

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
			MenuHandle menu;
			bResult = stack.pop((void **)&menu);
			UT_ASSERT(bResult);
			break;
		}
		case EV_MLF_Separator:
		{	
			short currentItem;
			MenuHandle parentMenu;
			bResult = stack.viewTop((void **)&parentMenu);
			UT_ASSERT(bResult);
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
	bResult = stack.pop((void **)&menu);
	UT_ASSERT(bResult);
	UT_ASSERT(menu == m_hMacMenubar);
	
	return true;
}


#if 0
bool EV_MacMenu::synthesize(void)
{
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

//	WindowPtr wTLW = m_pMacFrame->getTopLevelWindow();

//	MenuHandle aMenu = NewMenu(anID, "");

	// we keep a stack of the submenus so that we can properly
	// parent the menu items and deal with nested pull-rights.
        // if parent == NULL then it is in the menu bar
        
	UT_Stack stack;
	stack.push(NULL);
	
        
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
                const char * szLabelName = NULL;
                const char * szMnemonicName = NULL;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_BeginSubMenu:
                        {
                                MenuHandle hMenu;

                                const char **data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
                                szLabelName = data[0];
        
                                szMnemonicName = data[1];           
                                UT_DEBUGMSG(("START SUB MENU: L:[%s] MN:[%s] \n", 
                                        (szLabelName) ? szLabelName : "NULL", 
                                        (szMnemonicName) ? szMnemonicName : "NULL")); 
        
                                if (szLabelName && *szLabelName) {
                                        Str255 labelNameStr;
                                        UT_C2PStrWithConversion (szLabelName, labelNameStr,
                                                        kCFStringEncodingISOLatin1, kCFStringEncodingMacRoman);
                                        hMenu = ::NewMenu (id, labelNameStr);
                                        if (!hMenu) {
                                                break;
                                        }
                                        //printf("----- Before push ---\n");
                                        //print_stack(stack);
                                        stack.push((void *)hMenu);
                                        //printf("----- After push ---\n");
                                        //print_stack(stack);
                                }
                                break;
                        }
		case EV_MLF_Normal:
                        {
                            if(pView && pAction->hasGetStateFunction()) 
                            {
                                EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
//	                        if (mis & EV_MIS_Gray)
//                              	bEnable = false;
                                if (mis & EV_MIS_Toggled)
                                	bCheck = true;
                                else
                                	bCheck = false;
                            }
#if 0
                            bool bEnable = true;
                            bool bCheck = false;

                            if (pAction->hasGetStateFunction()) {
                                EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
                                if (mis & EV_MIS_Gray)
                                	bEnable = false;
                                if (mis & EV_MIS_Toggled)
                                	bCheck = true;
                            }                 
#endif
                            const char **data = getLabelName(m_pMacApp, m_pMacFrame, pAction, pLabel);
                            szLabelName = data[0];
                            szMnemonicName = data[1];
                            int index,modifiers;
                            char key;
                            key=0;
                            modifiers=0;
                            if (szMnemonicName != NULL)
                            {
				if (strstr (szMnemonicName, "Ctrl+") != NULL)
				{
					modifiers = B_CONTROL_KEY;
					if (strstr(szMnemonicName, "F4") != NULL)
					{
						//key=B_F4_KEY;
					}
					else if (strstr(szMnemonicName, "F7") != NULL)
					{
						//key=B_F7_KEY;
					}
					else if (strstr(szMnemonicName, "Del") != NULL)
					{
						//key=B_DELETE;
					}
					else
					{
						key=betterString[5];
					}
				}
				else if (strstr(szMnemonicName, "Alt+") != NULL)
				{
					modifiers=B_COMMAND_KEY;
					if (strstr(szMnemonicName, "F4") != NULL)
					{
					//	key=B_F4_KEY;
					}
					else if (strstr(szMnemonicName, "F7") != NULL)
					{
					//	key=B_F7_KEY;
					}
					else if	(strstr(szMnemonicName, "Del") != NULL)
					{
					//	key=B_DELETE;
					}
					else
					{
						key=betterString[4];
					}
				}
				else 
				{
					modifiers=0;
					if (strstr(szMnemonicName, "F4") != NULL)
					{
					//	key=B_F4_KEY;
					}
					else if (strstr(szMnemonicName, "F7") != NULL)
					{
					//	key=B_F7_KEY;
					}
					else if	(strstr(szMnemonicName, "Del") != NULL)
					{
					//	key=B_DELETE;
					}
					else
					{
						key=betterString[0];
					}	
				}
			}

			UT_DEBUGMSG(("NORM MENU: L:[%s] MN:[%s] \n", 
				(szLabelName) ? szLabelName : "NULL", 
				(szMnemonicName) ? szMnemonicName : "NULL")); 
			if (szLabelName && *szLabelName) {
				char buf[1024];
				// convert label into proper version and get accelerators
				
				int32 iLength = strlen(szLabelName);
				char* buffer = new char[2*(iLength+1)];
			        
                                memset(buffer, 0, 2*(iLength+1));

				int32 destLength = 2*(iLength + 1);
				int32 state =0;
			
				convert_to_utf8(B_ISO1_CONVERSION , szLabelName , &iLength ,  buffer , &destLength , &state);
				buffer[destLength] = '\0';
				
				accel = _ev_convert(buf, buffer);
				
				pMenu = top(stack);
				if (!pMenu)			//Skip bogus first item
				{
					delete [] buffer;
					break;
				}
				
				//UT_ASSERT(pMenu);
				BMessage *newmesg = new BMessage(ABI_BEOS_MENU_EV);
				newmesg->AddInt32(ABI_BEOS_MENU_EV_NAME, id);
                
				BMenuItem *pMenuItem = new BMenuItem(buf, newmesg, key,modifiers);
				if( pAction->isCheckable() )
					pMenuItem->SetMarked( (bCheck == true) );
					
				pMenu->AddItem(pMenuItem);
				
				delete [] buffer;
                            }
                            else {
				//We are reserving a spot in the menu for something
				//printf("Spot being reserved \n");
                            }
                        }	
			break;
		case EV_MLF_EndSubMenu:
			{
                            MenuHandle hMenu;
                            hMenu = top(stack); 
                            if (!hMenu)		//Skip bogus first entry
				break;
                            //UT_ASSERT(pMenu);
            
                            //printf("----- Before pop ---\n");
                            //print_stack(stack);
                            stack = pop(stack);
                            //printf("----- After pop ---\n");
                            //print_stack(stack);
                            MenuHandle *parentMenu = top(stack);
                            if (!parentMenu) {
                                ::InsertMenu (hMenu, 0);
                            }
                            else { 
                                    parentMenu->AddItem(pMenu);
                            }
                        }
			break;			
		case EV_MLF_Separator:
			{
				MenuHandle m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);
                                ::AppendMenu (m, "\p-");
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
		return false;
	}
	return true;
}
#endif

bool EV_MacMenu::onInitMenu(AV_View * pView, WindowPtr hWnd, Handle hMenuBar)
{
	// deal with WM_INITMENU.
#if 0
	if (hMenuBar != m_myMenu)			// these are not different when they
		return false;				// right-click on us on the menu bar.
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pMacApp->getMenuActionSet();
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	bool bNeedToRedrawMenu = false;

	UT_uint32 pos = 0;
	bool bResult;
	UT_Stack stackPos;
	stackPos.push((void*)pos);
	UT_Stack stackMenu;
	stackMenu.push(hMenuBar);

	HMENU m = hMenuBar;
		
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id id = pLayoutItem->getMenuId();
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

				const char * szLabelName = getLabelName(m_pMacApp,pAction,pLabel);

				BOOL bRemoveIt = (!szLabelName || !*szLabelName);

				if (bRemoveIt)			// we don't want it to be there
				{
					if (bPresent)
					{
						RemoveMenu(hMenuBar,cmd,MF_BYCOMMAND);
						bNeedToRedrawMenu = true;
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
						bNeedToRedrawMenu = true;
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
					bNeedToRedrawMenu = true;
				}
			}
			break;
	
		case EV_MLF_Separator:
			pos++;
			break;

		case EV_MLF_BeginSubMenu:
			pos++;
			stackMenu.push(m);
			stackPos.push((void*)pos);

			m = GetSubMenu(m, pos-1);
			UT_ASSERT(m);
			pos = 0;
			break;

		case EV_MLF_EndSubMenu:
			bResult = stackMenu.pop((void **)&m);
			UT_ASSERT(bResult);
			bResult = stackPos.pop((void **)&pos);
			UT_ASSERT(bResult);

			// restore the previous menu
			bResult = stackMenu.viewTop((void **)&m);
			UT_ASSERT(bResult);

			break;

		default:
			UT_ASSERT(0);
			break;
		}

	}

	// TODO some of the documentation refers to a need to call DrawMenuBar(hWnd)
	// TODO after any change to it.  other parts of the documentation makes no
	// TODO reference to it.  if you have problems, do something like:
	// TODO
	// TODO if (bNeedToRedrawMenu)
	// TODO 	DrawMenuBar(hWnd);
#endif // 0
	return true;
}

