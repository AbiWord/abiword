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
#if 0
class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_QNXMenu * pQNXMenu, XAP_Menu_Id id)
	{
		m_pQNXMenu = pQNXMenu;
		m_id = id;
		m_accelGroup = NULL;
	};
	
	~_wd(void)
	{
	};

	static void s_onActivate(GtkWidget * /* widget */, gpointer callback_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pQNXMenu->menuEvent(wd->m_id);
	};

	static void s_onMenuItemSelect(GtkWidget * widget, gpointer data)
	{
		UT_ASSERT(widget && data);

		_wd * wd = (_wd *) data;
		UT_ASSERT(wd && wd->m_pQNXMenu);

		XAP_QNXFrame * pFrame = wd->m_pQNXMenu->getFrame();
		UT_ASSERT(pFrame);

		EV_Menu_Label * pLabel = wd->m_pQNXMenu->getMenuLabelSet()->getLabel(wd->m_id);
		if (!pLabel)
		{
			pFrame->setStatusMessage(NULL);
			return;
		}

		const char * szMsg = pLabel->getMenuStatusMessage();
		if (!szMsg || !*szMsg)
			szMsg = "TODO This menu item doesn't have a StatusMessage defined.";
	
		pFrame->setStatusMessage(szMsg);
	};
	
	static void s_onMenuItemDeselect(GtkWidget * widget, gpointer data)
	{
		UT_ASSERT(widget && data);

		_wd * wd = (_wd *) data;
		UT_ASSERT(wd && wd->m_pQNXMenu);

		XAP_QNXFrame * pFrame = wd->m_pQNXMenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
	};

	static void s_onInitMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pQNXMenu->refreshMenu(wd->m_pQNXMenu->getFrame()->getCurrentView());

		// attach this new menu's accel group to be triggered off itself
		gtk_accel_group_attach(wd->m_accelGroup, GTK_OBJECT(menuItem));
		gtk_accel_group_lock(wd->m_accelGroup);
	};

	static void s_onDestroyMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		// we always clear the status bar when a menu goes away, so we don't
		// leave a message behind
		XAP_QNXFrame * pFrame = wd->m_pQNXMenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
		
		// bind this menuitem to its parent menu
		gtk_accel_group_detach(wd->m_accelGroup, GTK_OBJECT(menuItem));
		gtk_accel_group_unlock(wd->m_accelGroup);
	};

	// GTK wants to run popup menus asynchronously, but we want synchronous,
	// so we need to do a gtk_main_quit() on our own to show we're done
	// with our modal work.
	static void s_onDestroyPopupMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		// do the grunt work
		s_onDestroyMenu(menuItem, callback_data);
		gtk_main_quit();
	};

	GtkAccelGroup *		m_accelGroup;
	EV_QNXMenu *		m_pQNXMenu;
	XAP_Menu_Id			m_id;
};
#endif

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
									  EV_Menu_Action * pAction,
									  EV_Menu_Label * pLabel)
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

UT_Bool EV_QNXMenu::menuEvent(XAP_Menu_Id id)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pQNXApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeMenuMethod(m_pQNXFrame->getCurrentView(),pEM,0,0);
	return UT_TRUE;
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

int menu_activate(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	struct _cb_menu *mcb = (struct _cb_menu *)data;

	mcb->qnxmenu->menuEvent(mcb->id);

	return Pt_CONTINUE;
}

int menu_appear(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	PtWidget_t *menu= (PtWidget_t *)data;

	if (menu) {
		PtPositionMenu(menu, (info) ? info->event : NULL);
		PtRealizeWidget(menu);
	}

	return Pt_CONTINUE;
}


UT_Bool EV_QNXMenu::synthesizeMenu(PtWidget_t * wMenuRoot)
{
	PtArg_t args[10];

	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	UT_uint32 tmp = 0;
	UT_Bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

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
			
			//printf("Normal menu: [%s] \n", 
			//        (szLabelName) ? szLabelName : "NULL");
			if (szLabelName && *szLabelName)
			{
				char buf[1024], accel[2];
				// convert label into underscored version
				accel[0] = _ev_convert(buf, szLabelName);
				accel[1] = '\0';

				if (szMnemonicName && *szMnemonicName) {
					;//Do something here ...	
				}

				int n = 0;
				PtSetArg(&args[n], Pt_ARG_TEXT_STRING, buf, 0); n++;
				PtSetArg(&args[n], Pt_ARG_ACCEL_KEY, accel, 0); n++;
				//PtSetArg(&args[1], Pt_ARG_ACCEL_TEXT, accel, 0);				
 				wbutton = PtCreateWidget(PtMenuButton, wParent, n, args); 
				struct _cb_menu *mcb;
				mcb = (struct _cb_menu *)malloc(sizeof(*mcb));
				mcb->widget = wbutton;
				mcb->id = id;
				mcb->qnxmenu = this;
				PtAddCallback(wbutton, Pt_CB_ACTIVATE, menu_activate, mcb);

				// item is created, add to class vector
				m_vecMenuWidgets.addItem(wbutton);
				break;
			}
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = _ev_GetLabelName(m_pQNXApp, m_pQNXFrame, pAction, pLabel);
			szLabelName = data[0];

			PtWidget_t * wParent, *wbutton, *wmenu;
			stack.viewTop((void **)&wParent);

			//printf("Sub menu: [%s] \n", (szLabelName) ? szLabelName : "NULL");
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
					PtAddCallback(wbutton, Pt_CB_ARM, menu_appear, wmenu);
				}

				stack.push(wmenu);
				m_vecMenuWidgets.addItem(wbutton);
				m_vecMenuWidgets.addItem(wmenu);
				break;
			}
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			// pop and inspect
			PtWidget_t * w;
			bResult = stack.pop((void **)&w);
			UT_ASSERT(bResult);

			// item is created (albeit empty in this case), add to vector
			//m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_Separator:
		{	
			PtWidget_t * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			PtWidget_t * w = PtCreateWidget(PtSeparator, wParent, 0, 0);

			// item is created, add to class vector
			m_vecMenuWidgets.addItem(w);
			break;
		}

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			m_vecMenuWidgets.addItem(NULL);	// reserve slot in vector so indexes will be in sync
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	// make sure our last item on the stack is the one we started with
	return UT_TRUE;
}

UT_Bool EV_QNXMenu::_refreshMenu(AV_View * pView, void * wMenuRoot)
{
#if 0
	// update the status of stateful items on menu bar.

	const EV_Menu_ActionSet * pMenuActionSet = m_pQNXApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	UT_Bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

	// -1 will catch the case where we're inserting and haven't actually
	// entered into a real menu (only at a top level menu)
	
	gint nPositionInThisMenu = -1;
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			{
				// Keep track of where we are in this menu; we get cut down
				// to zero on the creation of each new submenu.
				nPositionInThisMenu++;
			
				// see if we need to enable/disable and/or check/uncheck it.
				
				UT_Bool bEnable = UT_TRUE;
				UT_Bool bCheck = UT_FALSE;
				
				if (pAction->hasGetStateFunction())
				{
					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray)
						bEnable = UT_FALSE;
					if (mis & EV_MIS_Toggled)
						bCheck = UT_TRUE;
				}

				// must have an entry for each and every layout item in the vector
				UT_ASSERT((k < m_vecMenuWidgets.getItemCount() - 1));

				// Get the dynamic label
				const char ** data = _ev_GetLabelName(m_pQNXApp, m_pQNXFrame, pAction, pLabel);
				const char * szLabelName = data[0];
				
				// First we check to make sure the item exits.  If it does not,
				// we create it and continue on.
				GList * testchildren = gtk_container_children(GTK_CONTAINER(m_vecMenuWidgets.getNthItem(k)));
				if (!testchildren)
				{
					// This should be the only place refreshMenu touches
					// callback hooks, since this handles the case a widget doesn't
					// exist for a given layout item
					if (szLabelName && *szLabelName)
					{
						// find parent menu item
						GtkWidget * wParent;
						bResult = stack.viewTop((void **)&wParent);
						UT_ASSERT(bResult);
											
						char labelbuf[1024];
						// convert label into underscored version
						_ev_convert(labelbuf, szLabelName);
						// create a label
						GtkLabel * label = GTK_LABEL(gtk_accel_label_new("SHOULD NOT APPEAR"));
						UT_ASSERT(label);

						// get a newly padded underscore version
						char * padString = _ev_skip_first_underscore_pad_rest(labelbuf);
						UT_ASSERT(padString);
						gtk_label_parse_uline(GTK_LABEL(label), padString);
						FREEP(padString);

						// create the item with the underscored label
						GtkWidget * w = gtk_menu_item_new();
						UT_ASSERT(w);
						// show and add the label to our menu item
						gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
						gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(label));
						gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), w);
						gtk_widget_show(GTK_WIDGET(label));
						gtk_widget_show(w);

						// set menu data to relate to class
						gtk_object_set_user_data(GTK_OBJECT(w),this);
						// create callback info data for action handling
						_wd * wd = new _wd(this, id);
						UT_ASSERT(wd);

						// set parent data stuff
						gtk_object_set_data(GTK_OBJECT(wMenuRoot), szLabelName, w);
						// bury in parent 
						gtk_menu_insert(GTK_MENU(GTK_MENU_ITEM(wParent)->submenu), w, nPositionInThisMenu);
						// connect callbacks
						gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(_wd::s_onActivate), wd);
						gtk_signal_connect(GTK_OBJECT(w), "select", GTK_SIGNAL_FUNC(_wd::s_onMenuItemSelect), wd);
						gtk_signal_connect(GTK_OBJECT(w), "deselect", GTK_SIGNAL_FUNC(_wd::s_onMenuItemDeselect), wd);				
						
						// we do NOT ad a new item, we point the existing index at our new widget
						// (update the pointers)
						void ** old = NULL;
						if (m_vecMenuWidgets.setNthItem(k, w, old))
						{
							UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.", k));
							UT_ASSERT(0);
						}
						
						break;
					}
					else
					{
						// do not create a widget if the label is blank, it should not appear in the
						// menu
					}
				}

				// No dynamic label, check/enable
				if (!pAction->hasDynamicLabel())
				{
					// if no dynamic label, all we need to do
					// is enable/disable and/or check/uncheck it.

					GtkWidget * item = (GtkWidget *) m_vecMenuWidgets.getNthItem(k);
					UT_ASSERT(item);

					// check boxes 
					if (GTK_IS_CHECK_MENU_ITEM(item))
						GTK_CHECK_MENU_ITEM(item)->active = bCheck;
					// all get the gray treatment
					gtk_widget_set_sensitive(GTK_WIDGET(item), bEnable);

					break;
				}

				// Get the item
				GtkWidget * item = (GtkWidget *) m_vecMenuWidgets.getNthItem(k);

				// if item is null, there is no widget for it, so ignore its attributes for
				// this pass
				if (!item)
					break;
						
				// Dynamic label, check for remove
				UT_Bool bRemoveIt = (!szLabelName || !*szLabelName);
				if (bRemoveIt)
				{
					// unbind all accelerators
					gtk_widget_remove_accelerators(item,
												   "activate_item",
												   FALSE);
					// wipe it out
					gtk_widget_destroy(item);

					// we must also mark this item in the vector as "removed",
					// which means setting [k] equal to a fake item as done
					// on creation of dynamic items.
					// give it a fake, with no label, to make sure it passes the
					// test that an empty (to be replaced) item in the vector should
					// have no children
					GtkWidget * w = gtk_menu_item_new();
					UT_ASSERT(w);
					void ** blah = NULL;
					if(m_vecMenuWidgets.setNthItem(k, w, blah))
					{
						UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.", k));
						UT_ASSERT(0);
					}
					break;
				}

				// Dynamic label, check for add/change
				// We always change the labels every time, it's actually cheaper
				// than doing the test for conditional changes.
				{
					// Get a list of children.  If there are any, destroy them
					GList * children = gtk_container_children(GTK_CONTAINER(item));
					if (children)
					{
						// Get the first item in the list
						GList * firstItem = g_list_first(children);
						UT_ASSERT(firstItem);
					
						// First item's data should be the label, since we added it first
						// in construction.
						GtkWidget * labelChild = GTK_WIDGET(firstItem->data);
						UT_ASSERT(labelChild);

						// destroy the current label
						gtk_container_remove(GTK_CONTAINER(item), labelChild);

						// unbind all accelerators
						gtk_widget_remove_accelerators(item,
													   "activate_item",
													   FALSE);
						
						//gtk_widget_destroy(labelChild);
					}
				
					// create a new updated label

					char labelbuf[1024];
					// convert label into underscored version
					_ev_convert(labelbuf, szLabelName);
					// create a label
					GtkLabel * label = GTK_LABEL(gtk_accel_label_new("SHOULD NOT APPEAR"));
					UT_ASSERT(label);

					// get a newly padded underscore version
					char * padString = _ev_skip_first_underscore_pad_rest(labelbuf);
					UT_ASSERT(padString);
					guint keyCode = gtk_label_parse_uline(label, padString);
					FREEP(padString);

					// show and add the label to our menu item
					gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
					gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(label));
					gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), item);
					gtk_widget_show(GTK_WIDGET(label));

					// bind to parent item's accel group
					if ((keyCode != GDK_VoidSymbol))// && parent_accel_group)
					{
						gtk_widget_add_accelerator(item,
												   "activate_item",
												   GTK_MENU(item->parent)->accel_group,
												   keyCode,
												   0,
												   GTK_ACCEL_LOCKED);
					}

					// finally, enable/disable and/or check/uncheck it.
					if (GTK_IS_CHECK_MENU_ITEM(item))
						GTK_CHECK_MENU_ITEM(item)->active = bCheck;
					gtk_widget_set_sensitive((GtkWidget *) item, bEnable);
				}
				
				// we are done with this menu item
			}
			break;
		case EV_MLF_Separator:
			nPositionInThisMenu++;
			
			break;

		case EV_MLF_BeginSubMenu:
		{
			nPositionInThisMenu = -1;

			// we need to nest sub menus to have some sort of context so
			// we can parent menu items
			GtkWidget * item = (GtkWidget *) m_vecMenuWidgets.getNthItem(k);
			UT_ASSERT(item);

			stack.push(item);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			GtkWidget * item = NULL;
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

	GtkWidget * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

#endif
	return UT_TRUE;
}

/*****************************************************************/

EV_QNXMenuBar::EV_QNXMenuBar(XAP_QNXApp * pQNXApp,
							   XAP_QNXFrame * pQNXFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_QNXMenu(pQNXApp,pQNXFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_QNXMenuBar::~EV_QNXMenuBar(void)
{
}

UT_Bool EV_QNXMenuBar::synthesizeMenuBar(void)
{
    PtArg_t args[10];
	int 	n = 0;

	int     width;

	width = m_pQNXFrame->m_AvailableArea.size.w;
	PhPoint_t	pos;
	pos.y = 0; pos.x = 2;

	//printf("Menu: synthesizing menu .... width %d \n", width);
#define _MNU_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
					 Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_TOP)
	PtSetArg(&args[n], Pt_ARG_ANCHOR_FLAGS, _MNU_ANCHOR_, _MNU_ANCHOR_); n++;
	PtSetArg(&args[n], Pt_ARG_RESIZE_FLAGS, 0, Pt_RESIZE_X_BITS); n++;
	//PtSetArg(&args[n], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n], Pt_ARG_WIDTH, width, 0); n++;
	m_wMenuBar = PtCreateWidget(PtMenuBar, 
								m_pQNXFrame->getTopLevelWindow(), 
								n, args);

	synthesizeMenu(m_wMenuBar);
	return UT_TRUE;
}

UT_Bool EV_QNXMenuBar::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuBar);

	return UT_TRUE;
}

/*****************************************************************/

EV_QNXMenuPopup::EV_QNXMenuPopup(XAP_QNXApp * pQNXApp,
								   XAP_QNXFrame * pQNXFrame,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_QNXMenu(pQNXApp,pQNXFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_QNXMenuPopup::~EV_QNXMenuPopup(void)
{
}

void * EV_QNXMenuPopup::getMenuHandle(void) const
{
	return m_wMenuPopup;
}

UT_Bool EV_QNXMenuPopup::synthesizeMenuPopup(void)
{
#if 0
	m_wMenuPopup = gtk_menu_new();
	_wd * wd = new _wd(this, 0);
	UT_ASSERT(wd);
	wd->m_accelGroup = gtk_accel_group_new();
	gtk_menu_set_accel_group(GTK_MENU(m_wMenuPopup), wd->m_accelGroup);
	gtk_signal_connect(GTK_OBJECT(m_wMenuPopup), "map",
					   GTK_SIGNAL_FUNC(_wd::s_onInitMenu), wd);
	gtk_signal_connect(GTK_OBJECT(m_wMenuPopup), "unmap",
					   GTK_SIGNAL_FUNC(_wd::s_onDestroyPopupMenu), wd);
	gtk_object_set_user_data(GTK_OBJECT(m_wMenuPopup),this);

	synthesizeMenu(m_wMenuPopup);
#endif
	return UT_TRUE;
}

UT_Bool EV_QNXMenuPopup::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuPopup);

	return UT_TRUE;
}
