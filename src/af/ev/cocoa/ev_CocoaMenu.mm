/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
#include "ut_string_class.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_CocoaMenu.h"
#include "ev_CocoaMenuBar.h"
#include "ev_CocoaMenuPopup.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaDialog_Utilities.h"
#include "ev_CocoaKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"
#include "ut_string_class.h"
#include "ap_Menu_Id.h"

#import <Cocoa/Cocoa.h>
#import <AppKit/NSNibControlConnector.h>

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_CocoaMenu * pCocoaMenu, XAP_Menu_Id menuid, NSMenuItem *item = nil);
	~_wd(void) {};

	NSMenuItem *        m_item;
	EV_CocoaMenu *		m_pCocoaMenu;
	XAP_Menu_Id			m_id;
};


@implementation EV_CocoaMenuTarget
- (id)menuSelected:(id)sender
{
	UT_DEBUGMSG (("@EV_CocoaMenuTarget (id)menuSelected:(id)sender\n"));

	UT_ASSERT ([sender class] == [NSMenuItem class]);
	_wd *wd = (_wd *)[sender tag];
	UT_ASSERT  (wd);
	wd->m_pCocoaMenu->menuEvent(wd->m_id);
}

@end

/*****************************************************************/
_wd::_wd(EV_CocoaMenu * pCocoaMenu, XAP_Menu_Id menuid, NSMenuItem *item)
{
	m_pCocoaMenu = pCocoaMenu;
	m_id = menuid;
	m_item = item;
}


#if 0
class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	static void s_onActivate(GtkWidget * /* widget */, gpointer callback_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pCocoaMenu->menuEvent(wd->m_id);
	};

	static void s_onMenuItemSelect(GtkWidget * widget, gpointer data)
	{
		UT_ASSERT(widget && data);

		_wd * wd = (_wd *) data;
		UT_ASSERT(wd && wd->m_pCocoaMenu);

		XAP_CocoaFrame * pFrame = wd->m_pCocoaMenu->getFrame();
		UT_ASSERT(pFrame);
		EV_Menu_Label * pLabel = wd->m_pCocoaMenu->getLabelSet()->getLabel(wd->m_id);
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
		UT_ASSERT(wd && wd->m_pCocoaMenu);

		XAP_CocoaFrame * pFrame = wd->m_pCocoaMenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
	};

	static void s_onInitMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		wd->m_pCocoaMenu->refreshMenu(wd->m_pCocoaMenu->getFrame()->getCurrentView());

		// attach this new menu's accel group to be triggered off itself
		gtk_accel_group_attach(wd->m_accelGroup, G_OBJECT(menuItem));
		gtk_accel_group_lock(wd->m_accelGroup);
	};

	static void s_onDestroyMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		_wd * wd = (_wd *) callback_data;
		UT_ASSERT(wd);

		// we always clear the status bar when a menu goes away, so we don't
		// leave a message behind
		XAP_CocoaFrame * pFrame = wd->m_pCocoaMenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
		
		// bind this menuitem to its parent menu
		gtk_accel_group_detach(wd->m_accelGroup, G_OBJECT(menuItem));
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
	EV_CocoaMenu *		m_pCocoaMenu;
	XAP_Menu_Id			m_id;
};
#endif

/*****************************************************************/


EV_CocoaMenu::EV_CocoaMenu(XAP_CocoaApp * pCocoaApp, XAP_CocoaFrame * pCocoaFrame,
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pCocoaApp, pCocoaApp->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	  m_pCocoaApp(pCocoaApp),
	  m_pCocoaFrame(pCocoaFrame)
{
	m_menuTarget = [[EV_CocoaMenuTarget alloc] init];
}

EV_CocoaMenu::~EV_CocoaMenu()
{
	UT_VECTOR_SPARSEPURGEALL(_wd *, m_vecMenuWidgets);
	[m_menuTarget release];
}

XAP_CocoaFrame * EV_CocoaMenu::getFrame()
{
	return m_pCocoaFrame;
}

bool EV_CocoaMenu::menuEvent(XAP_Menu_Id menuid)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return true if handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pCocoaApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pCocoaApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	UT_String script_name(pAction->getScriptName());
	invokeMenuMethod(m_pCocoaFrame->getCurrentView(), pEM, script_name);
	return true;
}


bool EV_CocoaMenu::synthesizeMenu(NSMenu * wMenuRoot)
{
	UT_DEBUGMSG(("EV_CocoaMenu::synthesizeMenu\n"));
    // create a GTK menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pCocoaApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	UT_uint32 tmp = 0;
	bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

	NSNibControlConnector * conn = [[NSNibControlConnector alloc] init];
	for (UT_uint32 k = 0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id menuid = pLayoutItem->getMenuId();
		// VERY BAD HACK!  It will be here until I fix the const correctness of all the functions
		// using EV_Menu_Action
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
		UT_ASSERT(pAction);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(menuid);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName;
		const char * szMnemonicName;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{
			unsigned int modifier = 0;
			const char ** data = getLabelName(m_pCocoaApp, m_pCocoaFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			if (data[0] && *(data[0])) {
				NSString * shortCut = nil;
				
				if (data[1] && *(data[1])) {
					_getItemCmd (data[1], modifier, shortCut);
				}
				else {
					shortCut = [NSString string];
				}
				char buf[1024];
				// convert label into underscored version
	
				// create the item with the underscored label
				NSMenu * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);
	
				NSMenuItem * menuItem = nil;
				NSString * str = nil;
				if (szLabelName) {
					_convertLabelToMac(buf, sizeof (buf), szLabelName);
					str = [NSString stringWithCString:buf];	// autoreleased
				}
				else {
					str = [NSString string];	// autoreleased
				}
				switch (menuid) {
				
				case AP_MENU_ID_HELP_ABOUT:
					menuItem = m_pCocoaFrame->_getAboutMenuItem();
					[menuItem setTitle:str];
					[menuItem setKeyEquivalent:shortCut];
					break;
				case AP_MENU_ID_TOOLS_OPTIONS:
					menuItem = m_pCocoaFrame->_getPreferenceMenuItem();
					[menuItem setTitle:str];
					[menuItem setKeyEquivalent:shortCut];
					break;
				case AP_MENU_ID_FILE_EXIT:
					menuItem = m_pCocoaFrame->_getQuitMenuItem();
					[menuItem setTitle:str];
					[menuItem setKeyEquivalent:shortCut];					
					break;
				default:
					menuItem = [wParent addItemWithTitle:str action:nil
									keyEquivalent:shortCut];
				}
				[conn setDestination:m_menuTarget];
				[conn setSource:menuItem];
				[conn setLabel:@"menuSelected:"];
				[conn establishConnection];
				
				// TODO check that this does not leak when we destroy the menu
				_wd * wd = new _wd (this, pLayoutItem->getMenuId(), menuItem);
				[menuItem setTag:(int)wd];
				[shortCut release];
		
				// item is created, add to class vector
				m_vecMenuWidgets.addItem(wd);
			}
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = getLabelName(m_pCocoaApp, m_pCocoaFrame, pAction, pLabel);
			szLabelName = data[0];
			
			char buf[1024];
			// convert label into underscored version
			// create the item with the underscored label
			NSMenu * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			NSMenuItem * menuItem = nil;
			NSString * str = nil;
			if (szLabelName) {
				_convertLabelToMac(buf, sizeof (buf), szLabelName);
				str = [NSString stringWithCString:buf];	// autoreleased
			}
			else {
				str = [NSString string]; // autoreleased
			}
			menuItem = [wParent addItemWithTitle:str action:nil keyEquivalent:@""];
			
			
			// item is created, add to class vector
			_wd * wd = new _wd (this, pLayoutItem->getMenuId(), menuItem);
			[menuItem setTag:(int)wd];
			m_vecMenuWidgets.addItem(wd);

			NSMenu * subMenu = [[NSMenu alloc] initWithTitle:str];
			[menuItem setSubmenu:subMenu];
			[subMenu setAutoenablesItems:NO];
			stack.push((void **)subMenu);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			// pop and inspect
			NSMenu * menu;
			bResult = stack.pop((void **)&menu);
			UT_ASSERT(bResult);

			// item is created (albeit empty in this case), add to vector
			m_vecMenuWidgets.addItem(NULL);
			break;
		}
		case EV_MLF_Separator:
		{	
			NSMenuItem * menuItem = nil;
			menuItem = [NSMenuItem separatorItem];

			NSMenu * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);
			[wParent addItem:menuItem];

			_wd * wd = new _wd (this, pLayoutItem->getMenuId(), menuItem);
			[menuItem setTag:(int)wd];
			// item is created, add to class vector
			m_vecMenuWidgets.addItem(wd);
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

	[conn release];
	// make sure our last item on the stack is the one we started with
	NSMenu * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

	return true;
}

bool EV_CocoaMenu::_refreshMenu(AV_View * pView, NSMenu * wMenuRoot)
{
	// update the status of stateful items on menu bar.

	const EV_Menu_ActionSet * pMenuActionSet = m_pCocoaApp->getMenuActionSet();
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
	
	for (UT_uint32 k = 0; k < nrLabelItemsInLayout; ++k)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id menuid = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(menuid);

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
			const char ** data = getLabelName(m_pCocoaApp, m_pCocoaFrame, pAction, pLabel);
			const char * szLabelName = data[0];
				
			// First we check to make sure the item exists.  If it does not,
			// we create it and continue on.
UT_ASSERT(UT_NOT_IMPLEMENTED);
#if 0
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
					g_object_set_user_data(G_OBJECT(w),this);
					// create callback info data for action handling
					_wd * wd = new _wd(this, id);
					UT_ASSERT(wd);

					// set parent data stuff
					g_object_set_data(G_OBJECT(wMenuRoot), szLabelName, w);
					// bury in parent 
					gtk_menu_insert(GTK_MENU(GTK_MENU_ITEM(wParent)->submenu), w, nPositionInThisMenu);
					// connect callbacks
					g_signal_connect(G_OBJECT(w), "activate", G_CALLBACK(_wd::s_onActivate), wd);
					g_signal_connect(G_OBJECT(w), "select", G_CALLBACK(_wd::s_onMenuItemSelect), wd);
					g_signal_connect(G_OBJECT(w), "deselect", G_CALLBACK(_wd::s_onMenuItemDeselect), wd);				
						
					// we do NOT add a new item, we point the existing index at our new widget
					// (update the pointers)
					void ** old = NULL;
					if (m_vecMenuWidgets.setNthItem(k, w, old))
					{
						UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.\n", k));
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
#endif
			// No dynamic label, check/enable
			if (!pAction->hasDynamicLabel())
			{
				// if no dynamic label, all we need to do
				// is enable/disable and/or check/uncheck it.

				NSMenuItem * item = (NSMenuItem *) m_vecMenuWidgets.getNthItem(k);
				UT_ASSERT(item);
				UT_ASSERT([item isKindOfClass:[NSMenuItem class]]);

				// all get the gray treatment
				[item setState:(bCheck ? NSOnState : NSOffState)];
				[item setEnabled:(bEnable ? YES : NO)];

				break;
			}

			// Get the item
			NSMenuItem * item = (NSMenuItem *) m_vecMenuWidgets.getNthItem(k);

			// if item is null, there is no widget for it, so ignore its attributes for
			// this pass
			if (!item) {
				break;
			}
			UT_ASSERT([item isKindOfClass:[NSMenuItem class]]);
						
				// Dynamic label, check for remove
			bool bRemoveIt = (!szLabelName || !*szLabelName);
			if (bRemoveIt)
			{
				NSMenu * parentMenu = [item menu];
				[parentMenu removeItem:item];
				// TODO check that we don't leak the menu item. I think it is autoreleased, but I'm not sure 

				// we must also mark this item in the vector as "removed",
				// which means setting [k] equal to a fake item as done
				// on creation of dynamic items.
				// give it a fake, with no label, to make sure it passes the
				// test that an empty (to be replaced) item in the vector should
				// have no children
				NSMenuItem * fakeItem = [[NSMenuItem alloc] init];
				UT_ASSERT(fakeItem);
				void ** blah = NULL;
				if(m_vecMenuWidgets.setNthItem(k, fakeItem, blah))
				{
					UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.\n", k));
					UT_ASSERT(0);
				}
				break;
			}
#if 0
			// Dynamic label, check for add/change
			// We always change the labels every time, it's actually cheaper
			// than doing the test for conditional changes.
			// TODO : check that it is cheaper in Cocoa. Obj-C method call is expensive !! -- Hub
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
#endif
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
			NSMenu * item = (NSMenu *) m_vecMenuWidgets.getNthItem(k);
			UT_ASSERT(item);
			UT_ASSERT([item isKindOfClass:[NSMenu class]]);

			stack.push(item);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			NSMenu * item = NULL;
			bResult = stack.pop((void **)&item);
			UT_ASSERT(bResult);
			UT_ASSERT ([item isKindOfClass:[NSMenu class]]);

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

	NSMenu * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

	return true;
}

/*!
 * That will add a new menu entry for the menu item at layout_pos.
 *
 * @param layout_pos UT_uint32 with the relative position of the item in the
 * menu.
 * @return true if there were no problems.  False elsewere.
 */
bool EV_CocoaMenu::_doAddMenuItem(UT_uint32 layout_pos)
{
	UT_DEBUGMSG(("JCA: layout_pos = [%d]\n", layout_pos));
	if (layout_pos > 0)
	{
		UT_DEBUGMSG(("Trying to insert at [%d] in a vector of size [%d].\n", layout_pos, m_vecMenuWidgets.size()));
		UT_sint32 err = m_vecMenuWidgets.insertItemAt(NULL, layout_pos);

		if (err != 0)
			UT_DEBUGMSG(("Error [%d] inserting NULL item in a ut_vector.\n", err));

		return (err == 0);
	}

	return false;
}




/*!
	Return the menu shortcut for the mnemonic
	
	\param mnemonic the string for the menmonic
	\returnvalue modifier the modifiers
	\returnvalue key a newly allocated NSString that contains the key equivalent. 
	should be nil on entry.
 */
void EV_CocoaMenu::_getItemCmd (const char * mnemonic, unsigned int & modifiers, NSString * & key)
{
	NSString * nsMnemonic;
	modifiers = 0;
	char * p;
	if (strstr (mnemonic, "Alt+")) {
		modifiers |= NSAlternateKeyMask;
	}
	else if (strstr (mnemonic, "Ctrl+") != NULL) {
		modifiers |= NSCommandKeyMask;
	}
	if ((modifiers & NSCommandKeyMask) == 0) {
		p = (char *)mnemonic;
	}
	else {
		p = strchr (mnemonic, '+');
		p++;
	}
	nsMnemonic = [NSString stringWithCString:p];	// autoreleased
	key = [nsMnemonic lowercaseString];
}


/*****************************************************************/

EV_CocoaMenuBar::EV_CocoaMenuBar(XAP_CocoaApp * pCocoaApp,
							   XAP_CocoaFrame * pCocoaFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_CocoaMenu(pCocoaApp, pCocoaFrame, szMenuLayoutName, szMenuLabelSetName)
{
}

EV_CocoaMenuBar::~EV_CocoaMenuBar()
{
}

void  EV_CocoaMenuBar::destroy(void)
{
}

bool EV_CocoaMenuBar::synthesizeMenuBar(NSMenu *menu)
{
	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
	m_wMenuBar = menu;

	synthesizeMenu(m_wMenuBar);
	
	[[NSApplication sharedApplication] setMainMenu:m_wMenuBar];
	return true;
}


bool EV_CocoaMenuBar::rebuildMenuBar()
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	return false;
#if 0
	GtkWidget * wVBox = m_pCocoaFrame->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	UT_ASSERT(m_wHandleBox);

	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
	m_wMenuBar = gtk_menu_bar_new();

	synthesizeMenu(m_wMenuBar);
	
	// show up the properly connected menu structure
	gtk_widget_show(m_wMenuBar);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wMenuBar);
	gtk_widget_show(m_wHandleBox);
	
	// put it at position 1 in the vbox
 	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, TRUE, 0); // was start
	gtk_box_reorder_child(GTK_BOX(wVBox), m_wHandleBox,0);

	return true;
#endif
}

bool EV_CocoaMenuBar::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuBar);

	return true;
}

/*****************************************************************/

EV_CocoaMenuPopup::EV_CocoaMenuPopup(XAP_CocoaApp * pCocoaApp,
								   XAP_CocoaFrame * pCocoaFrame,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_CocoaMenu(pCocoaApp, pCocoaFrame, szMenuLayoutName, szMenuLabelSetName),
		m_wMenuPopup(nil)
{
}

EV_CocoaMenuPopup::~EV_CocoaMenuPopup()
{
	[m_wMenuPopup release];
}

NSMenu * EV_CocoaMenuPopup::getMenuHandle() const
{
	return m_wMenuPopup;
}

bool EV_CocoaMenuPopup::synthesizeMenuPopup()
{
	m_wMenuPopup = [[NSMenu alloc] initWithTitle:@""];
	[m_wMenuPopup setAutoenablesItems:NO];
	synthesizeMenu(m_wMenuPopup);
	return true;
}

bool EV_CocoaMenuPopup::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView, m_wMenuPopup);

	return true;
}
