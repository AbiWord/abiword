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
#include <ctype.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_QNXMenu.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
#include "ev_QNXKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"


/*****************************************************************/

/*
  Unlike the Win32 version, which uses a \t (tab) to seperate the
  feature from the mnemonic in a single label string, this
  function returns two strings, to be put into the two seperate
  labels in a Gtk menu item.

  Oh, and these are static buffers, don't call this function
  twice and expect previous return pointers to have the same
  values at their ends.
*/

static const char ** _ev_GetLabelName(XAP_QNXApp * pQNXApp,
									  XAP_QNXFrame * pQNXFrame,
									  const EV_Menu_Action * pAction,
									  const EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pQNXFrame,pLabel);
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
			const EV_EditMethodContainer * pEMC = pQNXApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pQNXFrame->getEditEventMapper();
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

/*****************************************************************/

EV_QNXMenu::EV_QNXMenu(XAP_QNXApp * pQNXApp, XAP_QNXFrame * pQNXFrame,
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pQNXApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pQNXApp = pQNXApp;
	m_pQNXFrame = pQNXFrame;
}

EV_QNXMenu::~EV_QNXMenu(void)
{
	m_vecMenuWidgets.clear();
}

XAP_QNXFrame * EV_QNXMenu::getFrame(void)
{
	return m_pQNXFrame;
}

bool EV_QNXMenu::menuEvent(XAP_Menu_Id id)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return true iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pQNXApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	//Right away switch the focus back to the document, do this before
	//we invoke the menu method since the menu method could tell us to 
	//self destruct (ie a close selection) on the frame.
	m_pQNXFrame->setDocumentFocus();

	invokeMenuMethod(m_pQNXFrame->getCurrentView(),pEM,0,0);

	return true;
}

static const char * _ev_FakeName(const char * sz, UT_uint32 k)
{
	// construct a temporary string

	static char buf[128];
	UT_ASSERT(strlen(sz)<120);
	sprintf(buf,"%s%d",sz,k);
	return buf;
}

static char _ev_convert(char * bufResult,
			const char * szString)
{
	int i;
	char *p, c;
	UT_ASSERT(szString && bufResult);

	c = 0;
	for (i=0, p = bufResult; szString[i]; i++) {
		if (szString[i] == '&' && szString[i+1] != '&') 
			c = szString[i+1];
		else
			*p++ = szString[i];
	}
	*p = '\0';
	return(c);
}

struct _cb_menu {
	EV_QNXMenu	*qnxmenu;
	PtWidget_t	*widget;
	XAP_Menu_Id id;
};

/*******************************************************************************/

static int s_menu_select(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
		struct _cb_menu *mcb = (struct _cb_menu *)data;
		UT_ASSERT(mcb && mcb->qnxmenu);

		XAP_QNXFrame * pFrame = mcb->qnxmenu->getFrame();
		UT_ASSERT(pFrame);

		EV_Menu_Label * pLabel = mcb->qnxmenu->getMenuLabelSet()->getLabel(mcb->id);
		if (!pLabel) {
			pFrame->setStatusMessage(NULL);
			return Pt_CONTINUE;
		}

		const char * szMsg = pLabel->getMenuStatusMessage();
		if (!szMsg || !*szMsg) {
			szMsg = "TODO This menu item doesn't have a StatusMessage defined.";
		}
	
		pFrame->setStatusMessage(szMsg);
		return Pt_CONTINUE;
}
	
static int s_menu_deselect(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
		struct _cb_menu *mcb = (struct _cb_menu *)data;

		UT_ASSERT(mcb && mcb->qnxmenu);

		XAP_QNXFrame * pFrame = mcb->qnxmenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
		return Pt_CONTINUE;
}


static int s_menu_activate(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	struct _cb_menu *mcb = (struct _cb_menu *)data;

	mcb->qnxmenu->menuEvent(mcb->id);

	return Pt_CONTINUE;
}

static int s_menu_appear(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	PtWidget_t *menu= (PtWidget_t *)data;

	if (menu) {
		PtPositionMenu(menu, (info) ? info->event : NULL);
		PtRealizeWidget(menu);
	}

	return Pt_CONTINUE;
}

static int s_menu_init(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	EV_QNXMenu *pMenu = (EV_QNXMenu *)data;
	UT_ASSERT(pMenu);

	pMenu->refreshMenu(pMenu->getFrame()->getCurrentView());
	return Pt_CONTINUE;
}

/*******************************************************************************/

/* Not perfect since we have FXX keys and Del in this list 
   Really we should have a function to attach a hotkey/label
   to a widget given the widget.
*/
static char get_hotkey_key(const char *str) {
	//Find the next character after the =
	char *p;
	if ((p = strchr(str, '+')) && *++p) {
		//This fixes problems like Alt-F4 for quitting ... for now
		if (*(p+1) == '\0') {
			return tolower(*p);
		}
	}	
	return '\0';
}

static int get_hotkey_code(const char *str) {
	int code = 0;
	if (strstr(str, "Ctrl")) {
		code |= Pk_KM_Ctrl; 
	}
	if (strstr(str, "Shift")) {
		code |= Pk_KM_Shift;
	}
	if (strstr(str, "Alt")) {
		code |= Pk_KM_Alt;
	}
	return code;
}



bool EV_QNXMenu::synthesizeMenu(PtWidget_t * wMenuRoot)
{
	PtArg_t args[10];

	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName;
		const char * szMnemonicName;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{
			const char ** data = _ev_GetLabelName(m_pQNXApp, m_pQNXFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];

			PtWidget_t * wParent, *wbutton;
			stack.viewTop((void **)&wParent);
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024], accel[2];
				// convert label into underscored version
				accel[0] = _ev_convert(buf, szLabelName);
				accel[1] = '\0';

				int n = 0;
				PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buf, 0); 
				PtSetArg(&args[n++], Pt_ARG_ACCEL_KEY, accel, 0); 
				if (szMnemonicName && *szMnemonicName) {
					PtSetArg(&args[n++], Pt_ARG_ACCEL_TEXT, szMnemonicName, 0); 
				}

				if (pAction->isCheckable()) {
					PtSetArg(&args[n++], Pt_ARG_FLAGS,
							Pt_MENU_BUTTON | Pt_AUTOHIGHLIGHT | Pt_SET, 
							Pt_MENU_BUTTON | Pt_AUTOHIGHLIGHT | Pt_SET);
					wbutton = PtCreateWidget(PtToggleButton, wParent, n, args);
				}
				else {
 					wbutton = PtCreateWidget(PtMenuButton, wParent, n, args); 
				}

				struct _cb_menu *mcb;
				mcb = (struct _cb_menu *)malloc(sizeof(*mcb));
				mcb->widget = wbutton;
				mcb->id = id;
				mcb->qnxmenu = this;
				PtAddCallback(wbutton, Pt_CB_ACTIVATE, s_menu_activate, mcb);
				PtAddCallback(wbutton, Pt_CB_ARM, s_menu_select, mcb);
				PtAddCallback(wbutton, Pt_CB_DISARM, s_menu_deselect, mcb);

				if (szMnemonicName && *szMnemonicName && get_hotkey_key(szMnemonicName) != '\0') {
					PtAddHotkeyHandler(PtGetParent(wMenuRoot, PtWindow),
									   get_hotkey_key(szMnemonicName), 
									   get_hotkey_code(szMnemonicName),
										0, mcb, s_menu_activate); 
				}

				// item is created, add to class vector
				m_vecMenuWidgets.addItem(wbutton);
			}
			else {
				// Add a placeholder item in the list
				m_vecMenuWidgets.addItem(NULL);
			}
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = _ev_GetLabelName(m_pQNXApp, m_pQNXFrame, pAction, pLabel);
			szLabelName = data[0];

			PtWidget_t * wParent, *wbutton, *wmenu;
			stack.viewTop((void **)&wParent);

			if (szLabelName && *szLabelName)
			{
				char buf[1024], accel[2];
				// convert label into underscored version
				accel[0] =_ev_convert(buf, szLabelName);
				accel[1] = '\0';

				// create the item widget
				int n = 0;
				PtSetArg(&args[n], Pt_ARG_TEXT_STRING, buf, 0); n++;
				PtSetArg(&args[n], Pt_ARG_ACCEL_KEY, accel, 0); n++;
				//PtSetArg(&args[n], Pt_ARG_ACCEL_KEY, accel, 0); n++;
				if (wParent != wMenuRoot) {
					PtSetArg(&args[n], Pt_ARG_BUTTON_TYPE, Pt_MENU_RIGHT, Pt_MENU_RIGHT); n++;
				}
				wbutton = PtCreateWidget(PtMenuButton, wParent, n, args); 

				n = 0;
				if (wParent != wMenuRoot) {
					PtSetArg(&args[n], Pt_ARG_MENU_FLAGS, 
							Pt_MENU_CHILD|Pt_MENU_AUTO, 
							Pt_MENU_CHILD|Pt_MENU_AUTO); n++;
				}
				wmenu =  PtCreateWidget(PtMenu, wbutton, n, args); 
				if (wParent == wMenuRoot) {
					PtAddCallback(wbutton, Pt_CB_ARM, s_menu_init, this);
					PtAddCallback(wbutton, Pt_CB_ARM, s_menu_appear, wmenu);
					PtAddHotkeyHandler(PtGetParent(wMenuRoot, PtWindow), 
										tolower(accel[0]), Pk_KM_Alt, 
										0, wmenu, s_menu_appear); 
				}

				stack.push(wmenu);
				//If you need the menu button for manipulation, you can always get the parent
				m_vecMenuWidgets.addItem(wmenu /* wbutton */);
			}
			else {
				// Add a placeholder item in the list
				m_vecMenuWidgets.addItem(NULL);
			}
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			PtWidget_t * w;
			bResult = stack.pop((void **)&w);
			UT_ASSERT(bResult);

			//Add the same widget that was the start of the menu
			m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_Separator:
		{	
			PtWidget_t * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			PtWidget_t * w = PtCreateWidget(PtSeparator, wParent, 0, 0);

			m_vecMenuWidgets.addItem(w);
			break;
		}

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			//Reserve slot for later
			m_vecMenuWidgets.addItem(NULL);	
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	// make sure our last item on the stack is the one we started with
	return true;
}

static void set_menu_enabled(PtWidget_t *w, int enabled, int checked) {
	int flags;

/* Photon funniness ... why can't ghosting work? */
	flags =  (enabled == true) ? Pt_SELECTABLE : (Pt_BLOCKED | Pt_GHOST)  ;
	flags |= (checked == true) ? Pt_SET : 0;
	PtSetResource(w, Pt_ARG_FLAGS, flags, Pt_BLOCKED | Pt_GHOST | Pt_SET | Pt_SELECTABLE);
}

//TODO: This code gets called way too often, or maybe we need to be able
//      to whip through all of the non-relevant widgets more quickly.  In
//      any case it is on the TODO list.
bool EV_QNXMenu::_refreshMenu(AV_View * pView, void * wMenuRoot)
{
	PtWidget_t *item, *wParent, *wPrev;

	// update the status of stateful items on menu bar.
	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

	// -1 will catch the case where we're inserting and haven't actually
	// entered into a real menu (only at a top level menu)
	
	int nPositionInThisMenu = -1;

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			{
				// Keep track of where we are in this menu; we get cut down
				// to zero on the creation of each new submenu.
				nPositionInThisMenu++;
			
				// see if we need to enable/disable and/or check/uncheck it.
				
				bool bEnable = true;
				bool bCheck = false;
				
				if (pAction->hasGetStateFunction())
				{
					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray)
						bEnable = false;
					if (mis & EV_MIS_Toggled)
						bCheck = true;
				}

				// must have an entry for each and every layout item in the vector
				UT_ASSERT((k < m_vecMenuWidgets.getItemCount() - 1));

				// Get the dynamic label
				const char ** data = _ev_GetLabelName(m_pQNXApp, m_pQNXFrame, pAction, pLabel);
				const char * szLabelName = data[0];
				
				// Get the item we are pointing at
				item = (PtWidget_t *) m_vecMenuWidgets.getNthItem(k);
				stack.viewTop((void **)&wParent);

				// No dynamic label then just check/enable the item
				if (!pAction->hasDynamicLabel())
				{
					// If there is no item, then skip this placeholder entry		
					if (!item) {
						UT_ASSERT(0);
						break;
					}
					wPrev = item;
					set_menu_enabled(item, bEnable, bCheck);
				}

				// Dynamic label string --> NULL string means remove entry
				else if (!szLabelName || !*szLabelName)
				{
					if (item) {
						PtDestroyWidget(item);
					}

					// we must also mark this item in the vector as "removed",
					// which means setting [k] equal to a fake item as done
					// on creation of dynamic items.
					// give it a fake, with no label, to make sure it passes the
					// test that an empty (to be replaced) item in the vector should
					// have no children

					if(m_vecMenuWidgets.setNthItem(k, NULL /* New */, NULL /* Old */))
					{
						UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.", k));
						UT_ASSERT(0);
					}
					break;
				}

				// Dynamic label string --> Change label and set mode
				else {
					// create a new updated label
					char labelbuf[1024], accel[2];

					// convert label into underscored version
					accel[0] = _ev_convert(labelbuf, szLabelName);
					accel[1] = '\0';


					if (!item) {
						int n;
						PtArg_t args[10];
						
						n = 0;
						PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, labelbuf, 0); 
						PtSetArg(&args[n++], Pt_ARG_ACCEL_KEY, accel, 0); 
						/*
						if (szMnemonicName && *szMnemonicName) {
							PtSetArg(&args[n++], Pt_ARG_ACCEL_TEXT, szMnemonicName, 0); 
						}
						*/

						if (pAction->isCheckable()) {
							PtSetArg(&args[n++], Pt_ARG_FLAGS,
								Pt_MENU_BUTTON | Pt_AUTOHIGHLIGHT | Pt_SET, 
								Pt_MENU_BUTTON | Pt_AUTOHIGHLIGHT | Pt_SET);
							item = PtCreateWidget(PtToggleButton, wParent, n, args);
						} else {
							item = PtCreateWidget(PtMenuButton, wParent, n, args); 
						}
						if(wPrev) {
							PtWidgetInsert(item, wPrev, 0);
						}
						PtRealizeWidget(item);

						struct _cb_menu *mcb;
						mcb = (struct _cb_menu *)malloc(sizeof(*mcb));
						mcb->widget = item;
						mcb->id = id;
						mcb->qnxmenu = this;
						PtAddCallback(item, Pt_CB_ACTIVATE, s_menu_activate, mcb);
						PtAddCallback(item, Pt_CB_ARM, s_menu_select, mcb);
						PtAddCallback(item, Pt_CB_DISARM, s_menu_deselect, mcb);

						/*
						if (szMnemonicName && *szMnemonicName && get_hotkey_key(szMnemonicName) != '\0') {
							PtAddHotkeyHandler(PtGetParent(wMenuRoot, PtWindow),
									   get_hotkey_key(szMnemonicName), 
									   get_hotkey_code(szMnemonicName),
										0, mcb, s_menu_activate); 
						}
						*/

						//Now that we have an item, add it to the vector
						m_vecMenuWidgets.setNthItem(k, item, NULL);

						break;
					}

					PtSetResource(item, Pt_ARG_TEXT_STRING, labelbuf, 0);
					PtSetResource(item, Pt_ARG_ACCEL_KEY, accel, 0); 
					set_menu_enabled(item, bEnable, bCheck);
					wPrev = item;
				}
			}
			break;

		case EV_MLF_Separator:
			nPositionInThisMenu++;
			wPrev = (PtWidget_t *) m_vecMenuWidgets.getNthItem(k);
			break;

		case EV_MLF_BeginSubMenu:
		{
			nPositionInThisMenu = -1;

			// we need to nest sub menus to have some sort of context so
			// we can parent menu items
			item = (PtWidget_t *) m_vecMenuWidgets.getNthItem(k);
			wPrev = NULL;
			UT_ASSERT(item);
			stack.push(item);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			item = NULL;
			bResult = stack.pop((void **)&item);
			UT_ASSERT(bResult);
			break;
		}

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}	
	}

	PtWidget_t * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

	return true;
}

/*!
 * See unix menu code for comment
 */
bool EV_QNXMenu::_doAddMenuItem(UT_uint32 layout_pos)
{
	if (layout_pos > 0)
	{
		UT_sint32 err = m_vecMenuWidgets.insertItemAt(NULL, layout_pos);
		return !err;
	}

	return false;
}



/*****************************************************************/

EV_QNXMenuBar::EV_QNXMenuBar(XAP_QNXApp * pQNXApp,
							   XAP_QNXFrame * pQNXFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_QNXMenu(pQNXApp,pQNXFrame,szMenuLayoutName,szMenuLabelSetName)
{
	m_wMenuBar = NULL;
}

EV_QNXMenuBar::~EV_QNXMenuBar(void)
{
	if (m_wMenuBar) {
		PtDestroyWidget(m_wMenuBar);
	}
	m_wMenuBar = NULL;
	//TODO: Keep track of our alloced strucutres and free them too
}

bool EV_QNXMenuBar::synthesizeMenuBar(void)
{
	m_wMenuBar = PtCreateWidget(PtMenuBar, m_pQNXFrame->getTBGroupWidget(), 0, NULL);
	synthesizeMenu(m_wMenuBar);
	return true;
}

bool EV_QNXMenuBar::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuBar);

	return true;
}

/*****************************************************************/

EV_QNXMenuPopup::EV_QNXMenuPopup(XAP_QNXApp * pQNXApp,
								   XAP_QNXFrame * pQNXFrame,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_QNXMenu(pQNXApp,pQNXFrame,szMenuLayoutName,szMenuLabelSetName)
{
	m_wMenuPopup = NULL;
}

EV_QNXMenuPopup::~EV_QNXMenuPopup(void)
{
	if (m_wMenuPopup) {
		PtDestroyWidget(m_wMenuPopup);
	}
	m_wMenuPopup = NULL;
	//TODO: Keep track of our alloced strucutres and free them too
}

PtWidget_t * EV_QNXMenuPopup::getMenuHandle(void) const
{
	return m_wMenuPopup;
}

static int popup_realized(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	XAP_QNXFrame *pQNXFrame = (XAP_QNXFrame *)data; 

	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_FLAGS, Pt_BLOCKED, Pt_BLOCKED);
	PtSetResources(pQNXFrame->getTopLevelWindow(), 1, &arg);

	return Pt_CONTINUE;
}

static int popup_unrealized(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	XAP_QNXFrame *pQNXFrame = (XAP_QNXFrame *)data; 

	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_FLAGS, 0, Pt_BLOCKED);
	PtSetResources(pQNXFrame->getTopLevelWindow(), 1, &arg);

	pQNXFrame->setPopupDone(1);

	return Pt_CONTINUE;
}


bool EV_QNXMenuPopup::synthesizeMenuPopup()
{
    PtArg_t args[10];
	int 	n = 0;

	m_wMenuPopup = PtCreateWidget(PtMenu, 
								  m_pQNXFrame->getTopLevelWindow(),
								  n, args);
	PtAddCallback(m_wMenuPopup, Pt_CB_REALIZED, popup_realized, m_pQNXFrame);
	PtAddCallback(m_wMenuPopup, Pt_CB_UNREALIZED, popup_unrealized, m_pQNXFrame);

	synthesizeMenu(m_wMenuPopup);

	return true;
}

bool EV_QNXMenuPopup::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuPopup);

	return true;
}
