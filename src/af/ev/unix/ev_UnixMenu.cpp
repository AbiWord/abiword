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
 
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_UnixMenu.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"


#define DELETEP(p)		do { if (p) delete p; } while (0)
#define NrElements(a)	((sizeof(a) / sizeof(a[0])))

/*****************************************************************/

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixMenu * pUnixMenu, AP_Menu_Id id)
	{
		m_pUnixMenu = pUnixMenu;
		m_id = id;
	};
	
	~_wd(void)
	{
	};

	static void s_onActivate(GtkWidget * widget, gpointer callback_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pUnixMenu->menuEvent(wd->m_id);
	};

	static void s_onInitMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pUnixMenu->refreshMenu(wd->m_pUnixMenu->getFrame()->getCurrentView());

		// attach this new menu's accel group to be triggered off itself
		gtk_accel_group_attach(wd->m_accelGroup, GTK_OBJECT(menuItem));
		gtk_accel_group_lock(wd->m_accelGroup);
	};

	static void s_onDestroyMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		// bind this menuitem to its parent menu
		gtk_accel_group_detach(wd->m_accelGroup, GTK_OBJECT(menuItem));
		gtk_accel_group_unlock(wd->m_accelGroup);
	};

	GtkAccelGroup *		m_accelGroup;
	EV_UnixMenu *		m_pUnixMenu;
	AP_Menu_Id			m_id;
};


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

static const char ** _ev_GetLabelName(AP_UnixApp * pUnixApp,
									  XAP_UnixFrame * pUnixFrame,
									  EV_Menu_Action * pAction,
									  EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pUnixApp,pLabel);
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
			const EV_EditMethodContainer * pEMC = pUnixApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pUnixFrame->getEditEventMapper();
			UT_ASSERT(pEEM);

			const char * string = pEEM->getShortcutFor(pEM);
			if (string && *string)
				strcpy(accelbuf, string);
			else
				// zero it out for this round
				*accelbuf = NULL;
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

EV_UnixMenu::EV_UnixMenu(AP_UnixApp * pUnixApp, XAP_UnixFrame * pUnixFrame,
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pUnixApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pUnixApp = pUnixApp;
	m_pUnixFrame = pUnixFrame;

	m_accelGroup = gtk_accel_group_new();
}

EV_UnixMenu::~EV_UnixMenu(void)
{
	m_vecMenuWidgets.clear();
}

UT_Bool EV_UnixMenu::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView);

	return UT_TRUE;
}

XAP_UnixFrame * EV_UnixMenu::getFrame(void)
{
	return m_pUnixFrame;
}

UT_Bool EV_UnixMenu::menuEvent(AP_Menu_Id id)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pUnixApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeMenuMethod(m_pUnixFrame->getCurrentView(),pEM,0,0);
	return UT_TRUE;
}

static const char * _ev_FakeName(const char * sz, UT_uint32 k)
{
	// construct a temporary string

	static char buf[128];
	UT_ASSERT(strlen(sz)<120);
	sprintf(buf,"%s%ld",sz,k);
	return buf;
}

static void _ev_convert(char * bufResult,
						const char * szString)
{
	strcpy(bufResult, szString);

	char * pl = bufResult;
	while (*pl)
	{
		if (*pl == '&')
			*pl = '_';
		pl++;
	}
}

UT_Bool EV_UnixMenu::synthesize(void)
{
    // create a GTK menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	UT_ASSERT(m_wHandleBox);

	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
	m_wMenuBar = gtk_menu_bar_new();

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	UT_uint32 tmp = 0;
	UT_Bool bResult;
	UT_Stack stack;
	stack.push(m_wMenuBar);

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
		const char * szLabelName;
		const char * szMnemonicName;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{
			const char ** data = _ev_GetLabelName(m_pUnixApp, m_pUnixFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024];
				// convert label into underscored version
				_ev_convert(buf, szLabelName);
				// create a label
				GtkWidget * label = gtk_accel_label_new("SHOULD NOT APPEAR");
				UT_ASSERT(label);
				// trigger the underscore conversion in the menu labels
				guint keyCode = gtk_label_parse_uline(GTK_LABEL(label), buf);

				// create the item with the underscored label
				GtkWidget * w = gtk_menu_item_new();
				UT_ASSERT(w);
				GtkWidget * hbox = gtk_hbox_new(FALSE, 20);
				UT_ASSERT(hbox);
				gtk_widget_show(hbox);
 
				gtk_container_add(GTK_CONTAINER(w), hbox);
				
				// show and add the label to our menu item
				gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
				//gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(label));
				gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
				
				gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), w);

				if (szMnemonicName && *szMnemonicName)
				{
					GtkWidget * mlabel = gtk_accel_label_new(szMnemonicName);
					UT_ASSERT(mlabel);
					gtk_misc_set_alignment(GTK_MISC(mlabel), 0.0, 0.5);
					gtk_box_pack_end(GTK_BOX(hbox), mlabel, FALSE, FALSE, 0);
					gtk_widget_show(mlabel);
				}
				
				gtk_widget_show(GTK_WIDGET(label));
				gtk_widget_show(w);

				// set menu data to relate to class
				gtk_object_set_user_data(GTK_OBJECT(w),this);
				// create callback info data for action handling
				_wd * wd = new _wd(this, id);
				UT_ASSERT(wd);
				// find parent menu item
				GtkWidget * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);
				gtk_object_set_data(GTK_OBJECT(m_wMenuBar), szLabelName, w);
				// bury in parent
				gtk_container_add(GTK_CONTAINER(wParent),w);
				// connect callbacks
				gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(_wd::s_onActivate), wd);

				// bind to parent item's accel group
				if ((keyCode != GDK_VoidSymbol))// && parent_accel_group)
				{
					gtk_widget_add_accelerator(w,
											   "activate_item",
											   GTK_MENU(wParent)->accel_group,
											   keyCode,
											   0,
											   GTK_ACCEL_LOCKED);
				}

				// item is created, add to class vector
				m_vecMenuWidgets.addItem(w);
				break;
			}
			// give it a fake, with no label, to make sure it passes the
			// test that an empty (to be replaced) item in the vector should
			// have no children
			GtkWidget * w = gtk_menu_item_new();
			UT_ASSERT(w);
			m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = _ev_GetLabelName(m_pUnixApp, m_pUnixFrame, pAction, pLabel);
			szLabelName = data[0];
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024];
				// convert label into underscored version
				_ev_convert(buf, szLabelName);
				// create a label
				GtkLabel * label = GTK_LABEL(gtk_accel_label_new("SHOULD NOT APPEAR"));
				UT_ASSERT(label);

				// trigger the underscore conversion in the menu labels
				guint keyCode = gtk_label_parse_uline(label, buf);
				
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
				// find parent menu item
				GtkWidget * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);
				gtk_object_set_data(GTK_OBJECT(m_wMenuBar), szLabelName, w);
				// bury in parent
				gtk_container_add(GTK_CONTAINER(wParent),w);
				
				// since we are starting a new sub menu, create a shell for new items
				GtkWidget * wsub = gtk_menu_new();
				UT_ASSERT(wsub);

				wd->m_accelGroup = gtk_accel_group_new();

				if ((keyCode != GDK_VoidSymbol))
				{
					// bind to top level if parent is top level
					if (wParent == m_wMenuBar)
					{
						gtk_widget_add_accelerator(w,
												   "activate_item",
												   m_accelGroup,
												   keyCode,
												   GDK_MOD1_MASK,
												   GTK_ACCEL_LOCKED);
					}
					else
					{
						// just bind to be triggered by parent
						gtk_widget_add_accelerator(w,
												   "activate_item",
												   GTK_MENU(wParent)->accel_group,
												   keyCode,
												   0,
												   GTK_ACCEL_LOCKED);
					}
				}
				
				gtk_menu_set_accel_group(GTK_MENU(wsub), wd->m_accelGroup);
										 
				// menu items with sub menus attached (w) get this signal
				// bound to their children so they can trigger a refresh 
				gtk_signal_connect(GTK_OBJECT(wsub),
								   "map",
								   GTK_SIGNAL_FUNC(_wd::s_onInitMenu),
								   wd);
				gtk_signal_connect(GTK_OBJECT(wsub),
								   "unmap",
								   GTK_SIGNAL_FUNC(_wd::s_onDestroyMenu),
								   wd);
				
				gtk_object_set_user_data(GTK_OBJECT(wsub),this);

				// add to menu bar
				gtk_object_set_data(GTK_OBJECT(m_wMenuBar), _ev_FakeName(szLabelName, tmp++), wsub);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(w), wsub);
				stack.push(wsub);

                // item is created, add to vector
				m_vecMenuWidgets.addItem(w);
				break;
			}
			// give it a fake, with no label, to make sure it passes the
			// test that an empty (to be replaced) item in the vector should
			// have no children
			GtkWidget * w = gtk_menu_item_new();
			UT_ASSERT(w);
			m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			// pop and inspect
			GtkWidget * w;
			bResult = stack.pop((void **)&w);
			UT_ASSERT(bResult);

			// item is created (albeit empty in this case), add to vector
			m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_Separator:
		{	
			GtkWidget * w = gtk_menu_item_new();
			UT_ASSERT(w);
			gtk_object_set_user_data(GTK_OBJECT(w),this);
			_wd * wd = new _wd(this, id);
			UT_ASSERT(wd);

			GtkWidget * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			gtk_object_set_data(GTK_OBJECT(m_wMenuBar), _ev_FakeName("separator",tmp++), w);
			gtk_widget_show(w);
			gtk_container_add(GTK_CONTAINER(wParent),w);

			// item is created, add to class vector
			m_vecMenuWidgets.addItem(w);
			break;
		}
		default:
			UT_ASSERT(0);
			break;
		}
	}

	// make sure our last item on the stack is the one we started with
	GtkWidget * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == m_wMenuBar);
	
	// show up the properly connected menu structure
	gtk_widget_show(m_wMenuBar);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wMenuBar);
	gtk_widget_show(m_wHandleBox);

	// we also have to bind the top level window to our
	// accelerator group for this menu... it needs to join in
	// on the action.
	gtk_accel_group_attach(m_accelGroup, GTK_OBJECT(m_pUnixFrame->getTopLevelWindow()));
	gtk_accel_group_lock(m_accelGroup);
	
	// put it in the vbox
 	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, TRUE, 0);

	return UT_TRUE;
}

UT_Bool EV_UnixMenu::_refreshMenu(AV_View * pView)
{
	// update the status of stateful items on menu bar.

	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	UT_Bool bResult;
	UT_Stack stack;
	stack.push(m_wMenuBar);

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			{
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
				const char ** data = _ev_GetLabelName(m_pUnixApp, m_pUnixFrame, pAction, pLabel);
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
						// trigger the underscore conversion in the menu labels
						gtk_label_parse_uline(label, labelbuf);
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
						gtk_object_set_data(GTK_OBJECT(m_wMenuBar), szLabelName, w);
						// bury in parent
						gtk_container_add(GTK_CONTAINER(GTK_MENU_ITEM(wParent)->submenu), w);
						// connect callbacks
						gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(_wd::s_onActivate), wd);

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
//					gtk_container_remove(GTK_CONTAINER(item->parent), item);
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
					// trigger the underscore conversion in the menu labels
					guint keyCode = gtk_label_parse_uline(label, labelbuf);

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
			break;

		case EV_MLF_BeginSubMenu:
		{
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
		default:
			UT_ASSERT(0);
			break;
		}	
	}

	GtkWidget * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == m_wMenuBar);
	

	return UT_TRUE;
}
