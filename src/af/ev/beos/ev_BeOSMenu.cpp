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
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_BeOSMenu.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

#include "ut_string.h"

#include <UTF8.h>
#include <Menu.h>
#include <MessageFilter.h>
#include <String.h>
/*********************************************************************/
/*
 EVENT FILTERING
 The user selected something from the menu ... invoke the right method
*/
class MenuFilter: public BMessageFilter {
	public:
		MenuFilter(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame, 
				   EV_Menu *pEVMenu);
		filter_result Filter(BMessage *message, BHandler **target);
	private:
		XAP_BeOSApp 		*m_pBeOSApp;
		XAP_BeOSFrame 		*m_pBeOSFrame;
		EV_Menu			*m_pEVMenu;
};
		
MenuFilter::MenuFilter(XAP_BeOSApp * pBeOSApp, 
		       XAP_BeOSFrame * pBeOSFrame, EV_Menu *pEVMenu)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE, 
                           ABI_BEOS_MENU_EV, NULL) {
	m_pBeOSApp = pBeOSApp;
	m_pBeOSFrame = pBeOSFrame;
	m_pEVMenu = pEVMenu;
}					   

filter_result MenuFilter::Filter(BMessage *message, BHandler **target) { 
	if (message->what != ABI_BEOS_MENU_EV) {
		return(B_DISPATCH_MESSAGE);
	}
	//XAP_Menu_Id id = 0;
	BMenuItem* sourceItem;
	
	int32	id = 0;
	message->FindInt32(ABI_BEOS_MENU_EV_NAME, &id);
	message->FindPointer("source" , (void **)&sourceItem);
	//if(sourceItem->IsMarked())
	//	sourceItem->SetMarked(false);
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pBeOSApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_ASSERT(pAction);

#if 0
	if(m_pBeOSFrame->getCurrentView() && pAction->hasGetStateFunction())
	{
		if(pAction->isCheckable())
		{
		
			EV_Menu_ItemState mis = pAction->getMenuItemState(m_pBeOSFrame->getCurrentView());
	    	if (mis & EV_MIS_Toggled)
        	    sourceItem->SetMarked(false);
        	else
        		sourceItem->SetMarked(true);
		}
	}
#endif
             
	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return(B_SKIP_MESSAGE);
	
	const EV_EditMethodContainer * pEMC = m_pBeOSApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	UT_String script_name(pAction->getScriptName());
	m_pEVMenu->invokeMenuMethod(m_pBeOSFrame->getCurrentView(), pEM, script_name);

	((EV_BeOSMenu *)m_pEVMenu)->synthesize();
	
	return(B_SKIP_MESSAGE);			
}

/*********************************************************************/


EV_BeOSMenu::EV_BeOSMenu(XAP_BeOSApp * pBeOSApp, 
			 XAP_BeOSFrame * pBeOSFrame,
			 const char * szMenuLayoutName,
			 const char * szMenuLabelSetName)
	: EV_Menu(pBeOSApp, pBeOSApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	xxx_UT_DEBUGMSG(("EV:Menu: Name: %s SetName %s \n", 
				szMenuLayoutName, szMenuLabelSetName));
	m_pBeOSApp = pBeOSApp;
	m_pBeOSFrame = pBeOSFrame;
}

EV_BeOSMenu::~EV_BeOSMenu(void)
{
}


/*
 Used for manipulating this silly menu thing
*/
typedef struct _my_stack {
	BMenu 				*pMenu;
	struct _my_stack 		*next;
} my_stack_t;

void print_stack(my_stack_t *head) {
	my_stack_t *tmp;
	tmp = head;
	while (tmp) {
		printf("->0x%x \n", tmp->pMenu);
		tmp = tmp->next;
	}
}

my_stack_t *push(my_stack_t *head, BMenu *item) {
	my_stack_t *tmp = (my_stack_t *)malloc(sizeof(my_stack_t));
	if (!tmp) 
		return(head);
	tmp->pMenu = item;
	tmp->next = head;
	return(tmp);		
}

my_stack_t *pop(my_stack_t *head) {
	my_stack_t *tmpfree, *next;

	tmpfree = head;
	next = (head) ? head->next : NULL;
	if (tmpfree) 
		free(tmpfree);
	return(next);
}

BMenu *top(my_stack_t *head) {
	if (!head)
		return(NULL);
	return(head->pMenu);
}

/*
 Menu strings are punctuated with & in front of the key
 which is to serve as accelerators for that item.  This
 routine zips through and removes those &'s and returns
 the accelerator key as an int (think unicode!), and
 places the real string to be shown in bufResult.
*/
int _ev_convert(char * bufResult, const char * szString) {
	int	 accel = 0;
	const char *psrc = szString;
	char *pdst = bufResult;
	
	while (*psrc) {
		if (*psrc != '&') {
			*(pdst++) = *psrc;
		}
		else {
			accel = *(psrc+1);
		}
		psrc++;
	}
	*pdst = '\0';
	return(accel);
}

// Is there a reason why this couldn't be made generic?
const char ** _ev_GetLabelName(XAP_BeOSApp * pBeOSApp,
							  XAP_Frame * pBeOSFrame,
							  const EV_Menu_Action * pAction,
							  EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pBeOSFrame,pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return data;	// which will be two nulls now

	static char accelbuf[32];
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();

		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pBeOSApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pBeOSFrame->getEditEventMapper();
			UT_ASSERT(pEEM);

			const char * string = pEEM->getShortcutFor(pEM);
			if (string && *string)
				strcpy(accelbuf, string);
			else
				// zero it out for this round
				*accelbuf = 0;
		}
	}

	// set shortcut mnemonic, if any
	if (*accelbuf)
		data[1] = accelbuf;
	
	if (!pAction->raisesDialog())
	{
		data[0] = szLabelName;
		return data;
	}

	// append "..." to menu item if it raises a dialog
	static char buf[128];
	memset(buf,0,NrElements(buf));
	strncpy(buf,szLabelName,NrElements(buf)-4);
	strcat(buf,"...");

	data[0] = buf;
	
	return data;
}

bool EV_BeOSMenu::synthesizeMenuBar()
{
	AV_View* pView = m_pBeOSFrame->getCurrentView();
 	bool bCheck = false;
	BMenu 		*pMenu = NULL;
	be_Window 	*pBWin = NULL;
	//Future reference, use the UT_Stack stack;
	my_stack_t	*stack = NULL;
	int			accel;

	UT_ASSERT(m_pBeOSFrame);
	pBWin = (be_Window*)m_pBeOSFrame->getTopLevelWindow();
	UT_ASSERT(pBWin);
		
    // Get the list of actions, and a count.
	const EV_Menu_ActionSet * pMenuActionSet = m_pBeOSApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	//Create the top level menubar
	BRect all = pBWin->m_winRectAvailable;
	all.bottom = all.top + 19;
	pBWin->m_winRectAvailable.top = all.bottom + 1;
	pMenuBar = new BMenuBar(all, "Menubar");
	UT_ASSERT(pMenuBar);
	
	return true;
}

bool EV_BeOSMenu::synthesize()
 {		
	AV_View* pView = m_pBeOSFrame->getCurrentView();
		
	// Save off the rect we were created with and recreate the whole thing
	BRect frameRect = pMenuBar->Frame();
	pMenuBar->RemoveSelf();
	delete pMenuBar;
	pMenuBar = new BMenuBar(frameRect , "Menubar");
	
 	bool bCheck = false;
	BMenu 		*pMenu = NULL;
	be_Window 	*pBWin = NULL;
	//Future reference, use the UT_Stack stack;
	my_stack_t	*stack = NULL;
	int			accel;

	UT_ASSERT(m_pBeOSFrame);
	pBWin = (be_Window*)m_pBeOSFrame->getTopLevelWindow();
	UT_ASSERT(pBWin);
		
    // Get the list of actions, and a count.
	const EV_Menu_ActionSet * pMenuActionSet = m_pBeOSApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	//Create the top level menubar
//	BRect all = pBWin->m_winRectAvailable;
//	all.bottom = all.top + 18;
//	pBWin->m_winRectAvailable.top = all.bottom + 1;
//	pMenuBar = new BMenuBar(all, "Menubar");
//	UT_ASSERT(pMenuBar);
	
	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName = NULL;
                const char * szMnemonicName = NULL;

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:	{

			bool bEnable = true;

			if(pView && pAction->hasGetStateFunction()) 
			{
				EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
				if (mis & EV_MIS_Gray)
					bEnable = false;
				if (mis & EV_MIS_Toggled)
					bCheck = true;
				else
					bCheck = false;
			}

#if 0
                        bool bCheck = false;

                        if (pAction->hasGetStateFunction()) {
                        	EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
                                if (mis & EV_MIS_Gray)
                                	bEnable = false;
                                if (mis & EV_MIS_Toggled)
                                	bCheck = true;
                        }                 
#endif
			const char **data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
                        //const char ** data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
                        szLabelName = data[0];
                        szMnemonicName = data[1];
			BString betterString(szMnemonicName);
			int index,modifiers;
			char key;
			key=0;
			modifiers=0;
			
			// New -- We map control to the command key and alt to option.
			if (szMnemonicName != NULL)
			{
				if ((index=betterString.FindFirst("Ctrl+")) != B_ERROR)
				{
					modifiers=0; // Command is implied.
					if (betterString.FindFirst("F4") != B_ERROR)
					{
						//key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
						//key=B_F7_KEY;
					}
					else if (betterString.FindFirst("Del") != B_ERROR)
					{
						//key=B_DELETE;
					}
					else
					{
						key=betterString[5];
					}
				}
				else if ((index=betterString.FindFirst("Alt+")) != B_ERROR)
				{
					modifiers=B_OPTION_KEY;
					if (betterString.FindFirst("F4") != B_ERROR)
					{
					//	key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
					//	key=B_F7_KEY;
					}
					else if	(betterString.FindFirst("Del") != B_ERROR)
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
					if (betterString.FindFirst("F4") != B_ERROR)
					{
					//	key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
					//	key=B_F7_KEY;
					}
					else if	(betterString.FindFirst("Del") != B_ERROR)
					{
					//	key=B_DELETE;
					}
					else
					{
						key=betterString[0];
					}	
				}
			}

			xxx_UT_DEBUGMSG(("NORM MENU: L:[%s] MN:[%s] \n", 
				(szLabelName) ? szLabelName : "NULL", 
				(szMnemonicName) ? szMnemonicName : "NULL")); 
			if (szLabelName && *szLabelName) {
			

        
				char buf[1024];
				// convert label into proper version and get accelerators
				
				int32 iLength = strlen(szLabelName);
				char* buffer = new char[2*(iLength+1)];
	        		memset(buffer, 0, 2*(iLength+1));
				accel = _ev_convert(buf, szLabelName);
				
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

				pMenuItem->SetEnabled(bEnable);
					
				pMenu->AddItem(pMenuItem);
				
				delete [] buffer;
			}
			else {
				//We are reserving a spot in the menu for something
				//printf("Spot being reserved \n");
			}
			break;
		}
		case EV_MLF_BeginSubMenu: {
			const char **data= _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
			//char ** data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
                        szLabelName = data[0];

                        szMnemonicName = data[1];           
			xxx_UT_DEBUGMSG(("START SUB MENU: L:[%s] MN:[%s] \n", 
				(szLabelName) ? szLabelName : "NULL", 
				(szMnemonicName) ? szMnemonicName : "NULL")); 

			if (szLabelName && *szLabelName) {
				// convert label into underscored version
				char buf[1024];
				
				int32 iLength = strlen(szLabelName);
				char* buffer = new char[2*(iLength+1)];

				accel = _ev_convert(buf, szLabelName);

				pMenu = new BMenu(buf);		//Accellerator ignored
				if (!pMenu) 
				{
					delete [] buffer;
					break;
				}
				//printf("----- Before push ---\n");
				//print_stack(stack);
				stack = push(stack, pMenu);
				//printf("----- After push ---\n");
				//print_stack(stack);
				delete [] buffer;
			}
			break;
		}
		case EV_MLF_EndSubMenu:	{
			pMenu = top(stack); 
			if (!pMenu)		//Skip bogus first entry
				break;
			//UT_ASSERT(pMenu);
	
			//printf("----- Before pop ---\n");
			//print_stack(stack);
			stack = pop(stack);
			//printf("----- After pop ---\n");
			//print_stack(stack);
			BMenu *parentMenu = top(stack);
			if (!parentMenu) {
				pMenuBar->AddItem(pMenu);
			}
			else { 
				parentMenu->AddItem(pMenu);
			}
			break;
		}
		case EV_MLF_Separator:	{	
			pMenu = top(stack);
			if (pMenu)
				pMenu->AddSeparatorItem();
			break;
		}

		case EV_MLF_BeginPopupMenu:
			xxx_UT_DEBUGMSG(("MENU: Begin popup menu \n"));
                        break;
                case EV_MLF_EndPopupMenu:
			xxx_UT_DEBUGMSG(("MENU: End popup menu \n"));
                        break;

		default:
			UT_ASSERT(0);
			break;
		}
	}
	pBWin->AddChild(pMenuBar);
	pBWin->AddFilter(new MenuFilter(m_pBeOSApp, m_pBeOSFrame, this));	
		
	return true;
}

/*****************************************************************/

EV_BeOSMenuPopup::EV_BeOSMenuPopup(XAP_BeOSApp * pBeOSApp,
									XAP_BeOSFrame * pBeOSFrame,
									 const char * szMenuLayoutName,
									 const char * szMenuLabelSetName)
	: EV_BeOSMenu(pBeOSApp,pBeOSFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_BeOSMenuPopup::~EV_BeOSMenuPopup(void)
{
	if (m_pPopup)
		delete m_pPopup;
}

BPopUpMenu* EV_BeOSMenuPopup::GetHandle()
{
	return m_pPopup;
}

bool EV_BeOSMenuPopup::synthesizeMenuPopup(XAP_BeOSFrame * pFrame)
{
	BPopUpMenu	*pMenuBar = NULL;
	BMenu 		*pMenu = NULL;
//	BMenuBar 	*pMenuBar = NULL;

	//Future reference, use the UT_Stack stack;
	my_stack_t	*stack = NULL;
	int			accel;

	UT_ASSERT(pFrame);
		
    // Get the list of actions, and a count.
	const EV_Menu_ActionSet * pMenuActionSet = m_pBeOSApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	//Create the top level menubar
	//BRect all = pBWin->m_winRectAvailable;
	//all.bottom = all.top + 18;
	//pBWin->m_winRectAvailable.top = all.bottom + 1;
	pMenuBar = new BPopUpMenu("popup" , false, false);//BMenuBar(all, "Menubar");
	m_pPopup = pMenuBar;
	
	UT_ASSERT(pMenuBar);
	
	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();

		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);


		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName = NULL;
		const char * szMnemonicName = NULL;

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:	{
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

			const char **data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
			//const char ** data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];

			BString betterString(szMnemonicName);
			int index,modifiers;
			char key;
			key=0;
			modifiers=0;
			if (szMnemonicName != NULL)
			{
				if ((index=betterString.FindFirst("Ctrl+")) != B_ERROR)
				{
					modifiers=0; // Command is implied.
					if (betterString.FindFirst("F4") != B_ERROR)
					{
						//key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
						//key=B_F7_KEY;
					}
					else if (betterString.FindFirst("Del") != B_ERROR)
					{
						//key=B_DELETE;
					}
					else
					{
						key=betterString[5];
					}
				}
				else if ((index=betterString.FindFirst("Alt+")) != B_ERROR)
				{
					modifiers=B_OPTION_KEY;
					if (betterString.FindFirst("F4") != B_ERROR)
					{
					//	key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
					//	key=B_F7_KEY;
					}
					else if	(betterString.FindFirst("Del") != B_ERROR)
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
					if (betterString.FindFirst("F4") != B_ERROR)
					{
					//	key=B_F4_KEY;
					}
					else if (betterString.FindFirst("F7") !=B_ERROR)
					{
					//	key=B_F7_KEY;
					}
					else if	(betterString.FindFirst("Del") != B_ERROR)
					{
					//	key=B_DELETE;
					}
					else
					{
						key=betterString[0];
					}	
				}
			}

			xxx_UT_DEBUGMSG(("NORM MENU: L:[%s] MN:[%s] \n", 
				(szLabelName) ? szLabelName : "NULL", 
				(szMnemonicName) ? szMnemonicName : "NULL")); 
			if (szLabelName && *szLabelName) {
				char buf[1024];
				// convert label into proper version and get accelerators
				
				int32 iLength = strlen(szLabelName);
				char* buffer = new char[2*(iLength+1)];
			        
				memset(buffer, 0, 2*(iLength+1));

				accel = _ev_convert(buf, szLabelName);
				//UT_ASSERT(pMenu);
				BMessage *newmesg = new BMessage(ABI_BEOS_MENU_EV);
				newmesg->AddInt32(ABI_BEOS_MENU_EV_NAME, id);
                
				BMenuItem *pMenuItem = new BMenuItem(buf, newmesg, key,modifiers);
//				pMenuItem->SetEnabled(bEnable);
				pMenuBar->AddItem(pMenuItem);
				
				delete [] buffer;
			}
			else {
				//We are reserving a spot in the menu for something
				//printf("Spot being reserved \n");
			}
			break;
		}
		case EV_MLF_BeginSubMenu: {
			const char **data= _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
			//char ** data = _ev_GetLabelName(m_pBeOSApp, m_pBeOSFrame, pAction, pLabel);
                        szLabelName = data[0];

                        szMnemonicName = data[1];           
			xxx_UT_DEBUGMSG(("START SUB MENU: L:[%s] MN:[%s] \n", 
				(szLabelName) ? szLabelName : "NULL", 
				(szMnemonicName) ? szMnemonicName : "NULL")); 

			if (szLabelName && *szLabelName) {
				// convert label into underscored version
				char buf[1024];
				
				int32 iLength = strlen(szLabelName);
				char* buffer = new char[2*(iLength+1)];

				accel = _ev_convert(buf, szLabelName);
				pMenu = new BMenu(buf);		//Accellerator ignored
				if (!pMenu) 
				{
					delete [] buffer;
					break;
				}
				//printf("----- Before push ---\n");
				//print_stack(stack);
				stack = push(stack, pMenu);
				//printf("----- After push ---\n");
				//print_stack(stack);
				delete [] buffer;
			}
			break;
		}
		case EV_MLF_EndSubMenu:	{
			pMenu = top(stack); 
			if (!pMenu)		//Skip bogus first entry
				break;
			//UT_ASSERT(pMenu);
	
			//printf("----- Before pop ---\n");
			//print_stack(stack);
			stack = pop(stack);
			//printf("----- After pop ---\n");
			//print_stack(stack);
			BMenu *parentMenu = top(stack);
			if (!parentMenu) {
				pMenuBar->AddItem(pMenu);
			}
			else { 
				parentMenu->AddItem(pMenu);
			}
			break;
		}
		case EV_MLF_Separator:	{	
			pMenu = top(stack);
			if (pMenu)
				pMenu->AddSeparatorItem();
			else
				pMenuBar->AddSeparatorItem();
				
			break;
		}

		case EV_MLF_BeginPopupMenu:
			xxx_UT_DEBUGMSG(("MENU: Begin popup menu \n"));
                        break;
                case EV_MLF_EndPopupMenu:
			xxx_UT_DEBUGMSG(("MENU: End popup menu \n"));
                        break;

		default:
			UT_ASSERT(0);
			break;
		}
	}
		
	return true;
}
