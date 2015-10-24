/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stack>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "xap_Gtk2Compat.h"
#include "ev_UnixMenu.h"
#include "ev_UnixMenuBar.h"
#include "ev_UnixMenuPopup.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "ev_UnixKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"
#include "ut_string_class.h"
#include "xap_UnixDialogHelper.h"
#include "ap_Menu_Id.h"
// hack, icons are in wp
#include "ap_UnixStockIcons.h"
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#include <hildon/hildon-window.h>
#endif

#define ACTIVATE_ACCEL "activate"
#define ACCEL_FLAGS (GtkAccelFlags)(GTK_ACCEL_LOCKED)

/*****************************************************************/

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixMenu * pUnixMenu, XAP_Menu_Id id)
	{
		m_pUnixMenu = pUnixMenu;
		m_id = id;
	}
	
	~_wd(void)
	{
	}

	static void s_onActivate(GtkWidget * widget, gpointer callback_data)
	{
		// Do not do anything when a radio menu item is unchecked, see bug
		// 13734
		if (GTK_IS_RADIO_MENU_ITEM(widget) && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
			return;

		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

		_wd * wd = static_cast<_wd *>(callback_data);
		UT_return_if_fail(wd);

		wd->m_pUnixMenu->menuEvent(wd->m_id);
	}

	static void s_onMenuItemSelect(GtkWidget * /*widget*/, gpointer data)
	{
		UT_ASSERT(data);

		_wd * wd = static_cast<_wd *>(data);
		UT_return_if_fail(wd && wd->m_pUnixMenu);

		// WL_REFACTOR: redundant code
		XAP_Frame * pFrame = wd->m_pUnixMenu->getFrame();
		UT_return_if_fail(pFrame);
		EV_Menu_Label * pLabel = wd->m_pUnixMenu->getLabelSet()->getLabel(wd->m_id);
		if (!pLabel)
		{
			pFrame->setStatusMessage(NULL);
			return;
		}

		const char * szMsg = pLabel->getMenuStatusMessage();
		if (!szMsg || !*szMsg)
			szMsg = "TODO This menu item doesn't have a StatusMessage defined.";	
		pFrame->setStatusMessage(szMsg);
	}
	
	static void s_onMenuItemDeselect(GtkWidget * /*widget*/, gpointer data)
	{
		UT_ASSERT(data);

		_wd * wd = static_cast<_wd *>(data);
		UT_return_if_fail(wd && wd->m_pUnixMenu);

		XAP_Frame * pFrame = wd->m_pUnixMenu->getFrame();
		UT_return_if_fail(pFrame);

		pFrame->setStatusMessage(NULL);
	}

	static void s_onInitMenu(GtkMenuItem * /*menuItem*/, gpointer callback_data)
	{
		_wd * wd = static_cast<_wd *>(callback_data);
		UT_return_if_fail(wd);
		wd->m_pUnixMenu->refreshMenu(wd->m_pUnixMenu->getFrame()->getCurrentView());
	}

	static void s_onDestroyMenu(GtkMenuItem * /*menuItem*/, gpointer callback_data)
	{
		_wd * wd = static_cast<_wd *>(callback_data);
		UT_return_if_fail(wd);

		// we always clear the status bar when a menu goes away, so we don't
		// leave a message behind
		XAP_Frame * pFrame = wd->m_pUnixMenu->getFrame();
		UT_return_if_fail(pFrame);
		pFrame->setStatusMessage(NULL);
	}

	// GTK wants to run popup menus asynchronously, but we want synchronous,
	// so we need to do a gtk_main_quit() on our own to show we're done
	// with our modal work.
	static void s_onDestroyPopupMenu(GtkMenuItem * menuItem, gpointer callback_data)
	{
		// do the grunt work
		s_onDestroyMenu(menuItem, callback_data);
		gtk_main_quit();
	}

	EV_UnixMenu *		m_pUnixMenu;
	XAP_Menu_Id			m_id;
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

static const char ** _ev_GetLabelName(XAP_UnixApp * pUnixApp,
									  XAP_Frame * /*pFrame*/,
									  const EV_Menu_Action * pAction,
									  const EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return data;	// which will be two nulls now

	static UT_String accelbuf;
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();
		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pUnixApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pUnixApp->getEditEventMapper();
			UT_ASSERT(pEEM);

			const char * string = pEEM->getShortcutFor(pEM);
			if (string && *string)
				accelbuf = string;
			else
				// zero it out for this round
				accelbuf = "";
		}
	}

	// set shortcut mnemonic, if any
	if (!accelbuf.empty())
		data[1] = accelbuf.c_str();
	
	if (!pAction->raisesDialog())
	{
		data[0] = szLabelName;
		return data;
	}

	// append "..." to menu item if it raises a dialog
	static char buf[128];
	memset(buf,0,G_N_ELEMENTS(buf));
	strncpy(buf,szLabelName,G_N_ELEMENTS(buf)-4);
	strcat(buf,"...");

	data[0] = buf;
	
	return data;
}

/**
 * This subroutine calcules the gdk accel_key, ac_mods associated to
 * a given string (for instance, str = "Ctrl+A" -> accel_key = 'A'
 * ac_mods = GDK_CONTROL_MASK)
 */
void EV_UnixMenu::_convertStringToAccel(const char *str,
				       guint &accel_key,
				       GdkModifierType &ac_mods)
{
	if (str == NULL || *str == '\0')
		return;

	if (strncmp (str, "Ctrl+", 5) == 0) {
		ac_mods = static_cast<GdkModifierType>(ac_mods | GDK_CONTROL_MASK);
		str += 5;
	}

	if (strncmp (str, "Alt+", 4) == 0) {
		ac_mods = static_cast<GdkModifierType>(ac_mods | GDK_MOD1_MASK);
		str += 4;
	}

	if (strncmp (str, "Shift+", 6) == 0) {
		ac_mods = static_cast<GdkModifierType>(ac_mods | GDK_SHIFT_MASK);
		str += 6;
	}

	if (strncmp (str, "Del", 3) == 0) {
		// Rob: we are not using <del> as accel key, otherwise
		// events are not passed down the widget hierarchy
		// see #1235.
		// accel_key = GDK_Delete;
	}
	else if (str[0] == 'F' &&
			 str[1] >= '0' &&
			 str[1] <= '9') {
		accel_key = 0xFFBD + atoi(str + 1);
	}
	else {
		accel_key = static_cast<guint>(str[0]);
	}
}


/*****************************************************************/

EV_UnixMenu::EV_UnixMenu(XAP_UnixApp * pUnixApp, 
						 XAP_Frame *pFrame, 
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pUnixApp, pUnixApp->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	  m_pUnixApp(pUnixApp),
	  m_pFrame(pFrame),
	  // there are 189 callbacks at the moment. This is a large vector, but we do not want
	  // it to grow too fast (it has the lifespan of the application, and so we do not
	  // want too much empty space in it)
	  m_vecCallbacks(189,4, true)
{
	m_accelGroup = gtk_accel_group_new();
}

EV_UnixMenu::~EV_UnixMenu()
{
	m_vecMenuWidgets.clear();
	UT_VECTOR_PURGEALL(_wd *,m_vecCallbacks);
}

XAP_Frame * EV_UnixMenu::getFrame()
{
	return m_pFrame;
}

bool EV_UnixMenu::menuEvent(XAP_Menu_Id id)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return true if handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_return_val_if_fail(pMenuActionSet, false);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	UT_return_val_if_fail(pAction, false);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pUnixApp->getEditMethodContainer();
	UT_return_val_if_fail(pEMC, false);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	UT_String script_name(pAction->getScriptName());
	invokeMenuMethod(m_pFrame->getCurrentView(), pEM, script_name);
	return true;
}

#if !defined(EMBEDDED_TARGET) || EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
static guint _ev_get_underlined_char(const char * szString)
{

	UT_ASSERT(szString);
	
	// return the keycode right after the underline
	const UT_UCS4String str(szString);
	for (UT_uint32 i = 0; i + 1 < str.length(); )
	{
		if (str[i++] == '_')
			return gdk_unicode_to_keyval(str[i]);
	}

	return GDK_KEY_VoidSymbol;
}
#endif

static void _ev_strip_underline(char * bufResult,
								const char * szString)
{
	UT_ASSERT(szString && bufResult);
	
	const char * pl = szString;
	char * b = bufResult;
	while (*pl)
	{
		if (*pl == '_')
			pl++;
		else
			*b++ = *pl++;
	}
	
	*b = 0;
}

// change the first '&' character, which we assume to be an accelerator character, to a '_' character as used 
// by GTK+. Furthermore, escape all '_' characters with another '_' character for a literal '_'
static void _ev_convert(char * bufResult,
						const char * szString)
{
	UT_ASSERT(szString && bufResult);

	bool foundAmpersand = false;
	const char * src = szString;
	char * dest = bufResult;
	while (*src)
	{
		if (*src == '&' && !foundAmpersand)
		{
			*dest = '_';
			foundAmpersand = true;
		}
		else if (*src == '_')
		{
			*dest = '_';
			dest++;
			*dest = '_';
		}
		else
		{
			*dest = *src;
		}
		dest++;
		src++;
	}
	*dest = 0;
}

bool EV_UnixMenu::synthesizeMenu(GtkWidget * wMenuRoot, bool isPopup)
{
	// create a GTK menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	std::stack<GtkWidget*> stack;
	stack.push(wMenuRoot);

	GSList *group = NULL; // for radio button groups.

	for (UT_uint32 k = 0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);
		
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		// VERY BAD HACK!  It will be here until I fix the const correctness of all the functions
		// using EV_Menu_Action
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
			const char ** data = getLabelName(m_pUnixApp, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			GtkWidget * w;
			
			if (szLabelName && *szLabelName)
			{
				w = s_createNormalMenuEntry(id, pAction->isCheckable(), pAction->isRadio(), 
											isPopup, szLabelName, szMnemonicName);
				if (pAction->isRadio()) {
					gtk_radio_menu_item_set_group(GTK_RADIO_MENU_ITEM(w), group);
					group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));
				} else
					group = NULL; // radio buton items should be consecutive

				// find parent menu item
				GtkWidget * wParent = stack.top();
				UT_ASSERT(wParent);

				// bury in parent
				gtk_menu_shell_append(GTK_MENU_SHELL(wParent), w);
			}
			// give it a fake, with no label, to make sure it passes the
			// test that an empty (to be replaced) item in the vector should
			// have no children
			else 
			{
				  w = gtk_menu_item_new();
				  UT_ASSERT(w);
			}

			m_vecMenuWidgets.addItem(w);			
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = _ev_GetLabelName(m_pUnixApp, m_pFrame, pAction, pLabel);
			szLabelName = data[0];
			group = NULL; // assuming there is no submenu inside a radio button menus list
			
			if (szLabelName && *szLabelName)
			{				
				char buf[1024];
				// convert label into underscored version
				_ev_convert(buf, szLabelName);

				// create the item widget
				GtkWidget * w = gtk_menu_item_new_with_mnemonic(buf);
				//gtk_object_set_user_data(GTK_OBJECT(w), this);
				gtk_widget_show(w);
								
				// create callback info data for action handling
				_wd * wd = new _wd(this, id);
				UT_ASSERT(wd);
				m_vecCallbacks.addItem(static_cast<void *>(wd));

				// find parent menu item
				GtkWidget * wParent = stack.top();
				UT_ASSERT(wParent);

				// bury the widget in parent menu
				gtk_container_add(GTK_CONTAINER(wParent), w);
				
				// since we are starting a new sub menu, create a shell for new items
				GtkWidget * wsub = gtk_menu_new();

				// Here's the tricky part:
				// If the underlined character conflicts with ANY accelerator
				// in the keyboard layer, don't do the underline construction,
				// but instead make a label with no underlines (and no accelerators).
				
				// get the underlined value from the candidate label
				guint keyCode;
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
				keyCode = GDK_VoidSymbol;
#else
				keyCode = _ev_get_underlined_char(buf);
#endif
				
				// GTK triggers the menu accelerators off of MOD1 ***without
				// regard to what XK_ keysym is bound to it.  therefore, if
				// MOD1 is bound to XK_Alt_{L,R}, we do the following.

				bool bAltOnMod1 = (ev_UnixKeyboard::getAltModifierMask() == GDK_MOD1_MASK);
				bool bConflict = false;
				
				// Lookup any bindings cooresponding to MOD1-key and the lower-case
				// version of the underlined char, since all the menus ignore upper
				// case (SHIFT-MOD1-[char]) invokations of accelerators.

				if (keyCode != GDK_KEY_VoidSymbol && bAltOnMod1)
				{
					EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
					UT_ASSERT(pEEM);
					EV_EditMethod * pEM = NULL;
					pEEM->Keystroke(EV_EKP_PRESS|EV_EMS_ALT|keyCode,&pEM);

					// if the pointer is valid, there is a conflict
					bConflict = (pEM != NULL);
				}
				
				if (bConflict)
				{
					// construct the label with NO underlined text and
					// no accelerators bound
					char * dup = NULL;

					// clone string just for the space it gives us, the data
					// is trashed by _ev_strip_underlines()
					dup = g_strdup(buf);

					// get a clean string
					_ev_strip_underline(dup, buf);
					
					GtkWidget * child = gtk_bin_get_child(GTK_BIN(w));
					UT_ASSERT(child);					
					gtk_label_set_text_with_mnemonic(GTK_LABEL(child), dup);

					FREEP(dup);
				}

#ifndef ENABLE_MENUBUTTON
				if ((keyCode != GDK_KEY_VoidSymbol) && !isPopup)
				  {
					  // bind to top level if parent is top level
 					  if (wParent == wMenuRoot)
 					    {
 						    gtk_widget_add_accelerator(w,
 									       ACTIVATE_ACCEL,
 									       m_accelGroup,
 									       keyCode,
 									       GDK_MOD1_MASK,
 									       ACCEL_FLAGS);
 					    }
				  }
#endif
				// we always set an accel group, even if we don't actually bind any
				// to this widget
				GtkAccelGroup *accelGroup = gtk_accel_group_new();
				gtk_menu_set_accel_group(GTK_MENU(wsub),accelGroup);
				g_object_unref(accelGroup);

				// This stuff happens to every label:
				// 
				// menu items with sub menus attached (w) get this signal
				// bound to their children so they can trigger a refresh 
				g_signal_connect(G_OBJECT(wsub),
						 "map",
						 G_CALLBACK(_wd::s_onInitMenu),
						 wd);
				g_signal_connect(G_OBJECT(wsub),
						 "unmap",
						 G_CALLBACK(_wd::s_onDestroyMenu),
								   wd);
				
				// add to menu bar
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
			w = stack.top();
			stack.pop();
			UT_ASSERT(w);
			group = NULL;

			// item is created (albeit empty in this case), add to vector
			m_vecMenuWidgets.addItem(w);
			break;
		}
		case EV_MLF_Separator:
		{				
			GtkWidget * w = gtk_separator_menu_item_new();
			gtk_widget_set_sensitive(w, FALSE);
			group = NULL; // assuming there is no separator inside a radio button menus list

			GtkWidget * wParent = stack.top();
			UT_ASSERT(wParent);

			gtk_widget_show(w);
			gtk_menu_shell_append(GTK_MENU_SHELL(wParent),w);

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

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else

	// make sure our last item on the stack is the one we started with
	GtkWidget * wDbg = stack.top();
	stack.pop();
	UT_UNUSED(wDbg);
	UT_ASSERT(wDbg == wMenuRoot);

	// we also have to bind the top level window to our
	// accelerator group for this menu... it needs to join in
	// on the action.
	if(GTK_IS_WINDOW(static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow()) == TRUE)
	{
		gtk_window_add_accel_group(GTK_WINDOW(static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow()), m_accelGroup);
	}
	else
	{
		gtk_window_add_accel_group(GTK_WINDOW(gtk_widget_get_parent(static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow())), m_accelGroup);
	}
	gtk_accel_group_lock(m_accelGroup);
	
#endif

	return true;
}

bool EV_UnixMenu::_refreshMenu(AV_View * pView, GtkWidget * wMenuRoot)
{
	// update the status of stateful items on menu bar.

	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	UT_sint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	std::stack<GtkWidget*> stack;
	stack.push(wMenuRoot);

	// -1 will catch the case where we're inserting and haven't actually
	// entered into a real menu (only at a top level menu)
	
	gint nPositionInThisMenu = -1;
	GSList *group = NULL; // for radio button groups
	
	for (UT_sint32 k = 0; k < nrLabelItemsInLayout; ++k)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{			
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
			const char ** data = _ev_GetLabelName(m_pUnixApp, m_pFrame, pAction, pLabel);
			const char * szLabelName = data[0];
			const char * szMnemonicName = data[1];

			// First we check to make sure the item exists.  If it does not,
			// we create it and continue on.
			if (!gtk_bin_get_child(GTK_BIN(GTK_WIDGET(m_vecMenuWidgets.getNthItem(k)))))
			{
				// This should be the only place refreshMenu touches
				// callback hooks, since this handles the case a widget doesn't
				// exist for a given layout item
				if (szLabelName && *szLabelName)
				{
					// increment position before continuing
					nPositionInThisMenu++;

					// create the item with the underscored label
					GtkWidget * w = s_createNormalMenuEntry(id, pAction->isCheckable () && bCheck, 
															pAction->isRadio () && bCheck, 
															false, szLabelName, szMnemonicName);
					UT_ASSERT(w);
					if (pAction->isRadio()) {
						// note that this only works if the whole group is created at once
						gtk_radio_menu_item_set_group(GTK_RADIO_MENU_ITEM(w), group);
						group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));
					} else
						group = NULL; // radio buton items should be consecutive

					// find parent menu item
					GtkWidget * wParent = stack.top();
					UT_ASSERT(wParent);

					// bury in parent
					gtk_menu_shell_insert(GTK_MENU_SHELL(gtk_menu_item_get_submenu(GTK_MENU_ITEM(wParent))), 
										  w, (nPositionInThisMenu+1));
					
					// we do NOT add a new item, we point the existing index at our new widget
					// (update the pointers)
					GtkWidget* old = NULL;
					GtkWidget *oldItem = GTK_WIDGET(m_vecMenuWidgets.getNthItem(k));
					if (m_vecMenuWidgets.setNthItem(k, w, &old))
					{
						UT_DEBUGMSG(("Could not update dynamic menu widget vector item %d.\n", k));
						UT_ASSERT(0);
					}
					gtk_widget_destroy(oldItem);
					break;
				}
				else
				{
					// do not create a widget if the label is blank, it should not appear in the
					// menu
				}
			}			
			else
			  {
				  // Keep track of where we are in this menu; we get cut down
				  // to zero on the creation of each new submenu.
				  nPositionInThisMenu++;
			  }
			

			// No dynamic label, check/enable
			if (!pAction->hasDynamicLabel())
			{
				// if no dynamic label, all we need to do
				// is enable/disable and/or check/uncheck it.

				GtkWidget * item = m_vecMenuWidgets.getNthItem(k);
				UT_ASSERT(item);

				// check boxes 
				if (GTK_IS_CHECK_MENU_ITEM(item)) {
				  g_signal_handlers_block_by_func(item, reinterpret_cast<void *>(_wd::s_onActivate), g_object_get_data(G_OBJECT(item), "wd"));
				  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), bCheck);
				  g_signal_handlers_unblock_by_func(item, reinterpret_cast<void *>(_wd::s_onActivate), g_object_get_data(G_OBJECT(item), "wd"));
				}

				// all get the gray treatment
				gtk_widget_set_sensitive(GTK_WIDGET(item), bEnable);

				break;
			}

			// Get the item
			GtkWidget * item = m_vecMenuWidgets.getNthItem(k);

			// if item is null, there is no widget for it, so ignore its attributes for
			// this pass
			if (!item)
				break;
						
			// Dynamic label, check for remove
			bool bRemoveIt = (!szLabelName || !*szLabelName);
			if (bRemoveIt)
			{
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
				GtkWidget* blah = NULL;
				if(m_vecMenuWidgets.setNthItem(k, w, &blah))
				{
					UT_DEBUGMSG(("Could not update dynamic menu widget vector item %d.\n", k));
					UT_ASSERT(0);
				}
				break;
			}

			// Dynamic label, check for add/change
			// We always change the labels every time, it's actually cheaper
			// than doing the test for conditional changes.
			// The first child _should_ be a label 
			GtkWidget * child = gtk_bin_get_child(GTK_BIN(item));
			if (child) 
			  {				  
				  
				  // create a new updated label
				  char labelBuf[1024];
				  // convert label into underscored version
				  _ev_convert(labelBuf, szLabelName);
				  gtk_label_set_text_with_mnemonic(GTK_LABEL(child), labelBuf);

				  
				  // bind to parent item's accel group

				  // finally, enable/disable and/or check/uncheck it.
				  if (GTK_IS_CHECK_MENU_ITEM(item)) {
					g_signal_handlers_block_by_func(item, reinterpret_cast<void *>(_wd::s_onActivate), g_object_get_data(G_OBJECT(item), "wd"));
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), bCheck);
					g_signal_handlers_unblock_by_func(item, reinterpret_cast<void *>(_wd::s_onActivate), g_object_get_data(G_OBJECT(item), "wd"));
				  }
				gtk_widget_set_sensitive(static_cast<GtkWidget *>(item), bEnable);
			  }
			
			// we are done with this menu item

			break;
		}
		case EV_MLF_Separator:
			group = NULL; // assuming there is no separator inside a radio button menus list
			nPositionInThisMenu++;			
			break;

		case EV_MLF_BeginSubMenu:
		{
			nPositionInThisMenu = -1;
			group = NULL; // assuming there is no submenu inside a radio button menus list

			// we need to nest sub menus to have some sort of context so
			// we can parent menu items
			GtkWidget * item = m_vecMenuWidgets.getNthItem(k);
			UT_ASSERT(item);

			bool bEnable = true;
			if (pAction->hasGetStateFunction())
			{
				EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
				if (mis & EV_MIS_Gray)
					bEnable = false;
			}
			gtk_widget_set_sensitive(item, bEnable);

			// must have an entry for each and every layout item in the vector
			stack.push(item);
			break;
		}
		case EV_MLF_EndSubMenu:
			UT_ASSERT(stack.top());
			stack.pop();
			group = NULL;

			break;

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}	
	}

	UT_ASSERT(stack.top() == wMenuRoot);
	stack.pop();

	return true;
}

/*!
 * That will add a new menu entry for the menu item at layout_pos.
 *
 * @param layout_pos UT_uint32 with the relative position of the item in the
 * menu.
 * @return true if there were no problems.  False elsewere.
 */
bool EV_UnixMenu::_doAddMenuItem(UT_uint32 layout_pos)
{
	if (layout_pos > 0)
	{
		UT_sint32 err = m_vecMenuWidgets.insertItemAt(NULL, layout_pos);

		if (err != 0) {
			UT_DEBUGMSG(("Error [%d] inserting NULL item in a ut_vector.\n", err));
		}

		return (err == 0);
	}

	return false;
}

/*****************************************************************/

EV_UnixMenuBar::EV_UnixMenuBar(XAP_UnixApp * pUnixApp,
							   XAP_Frame * pFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_UnixMenu(pUnixApp, pFrame, szMenuLayoutName, szMenuLabelSetName)
{
}

EV_UnixMenuBar::~EV_UnixMenuBar()
{
}

void  EV_UnixMenuBar::destroy(void)
{
	gtk_widget_destroy(m_wMenuBar);
}

bool EV_UnixMenuBar::synthesizeMenuBar()
{

	// Just create, don't show the menu bar yet.  It is later added and shown
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON /* in hildon sdk you have get menu_bar from mainWidonw */
	GtkWidget * wWidget = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	m_wMenuBar = gtk_menu_new ();
    hildon_window_set_menu (HILDON_WINDOW (wWidget), GTK_MENU (m_wMenuBar));
#elif defined (ENABLE_MENUBUTTON)
	m_wMenuBar = gtk_menu_new ();
#else
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();
	m_wMenuBar = gtk_menu_bar_new();
#endif

	synthesizeMenu(m_wMenuBar, false);
	gtk_widget_show_all(m_wMenuBar);

#ifdef EMBEDDED_TARGET
#else
	gtk_box_pack_start(GTK_BOX(wVBox), m_wMenuBar, FALSE, TRUE, 0);
#endif	

	return true;
}


bool EV_UnixMenuBar::rebuildMenuBar()
{
#if !defined(EMBEDDED_TARGET) || EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();

	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
#ifdef ENABLE_MENUBUTTON
	m_wMenuBar = gtk_menu_new();
#else
	m_wMenuBar = gtk_menu_bar_new();
#endif

	synthesizeMenu(m_wMenuBar, false);

	// show up the properly connected menu structure
	gtk_widget_show(m_wMenuBar);

	gtk_box_pack_start(GTK_BOX(wVBox), m_wMenuBar, FALSE, TRUE, 0);
	gtk_box_reorder_child(GTK_BOX(wVBox), m_wMenuBar, 0);
#endif
	return true;
}

bool EV_UnixMenuBar::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuBar);

	return true;
}

/*****************************************************************/

EV_UnixMenuPopup::EV_UnixMenuPopup(XAP_UnixApp * pUnixApp,
								   XAP_Frame * pFrame,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_UnixMenu(pUnixApp, pFrame, szMenuLayoutName, szMenuLabelSetName)
{
}

EV_UnixMenuPopup::~EV_UnixMenuPopup()
{
	UT_VECTOR_PURGEALL(_wd *,m_vecCallbacks);
}

GtkWidget * EV_UnixMenuPopup::getMenuHandle() const
{
	return m_wMenuPopup;
}

bool EV_UnixMenuPopup::synthesizeMenuPopup()
{
	m_wMenuPopup = gtk_menu_new();
	_wd * wd = new _wd(this, 0);
	UT_ASSERT(wd);
	GtkAccelGroup *accelGroup = gtk_accel_group_new();
	gtk_menu_set_accel_group(GTK_MENU(m_wMenuPopup),accelGroup);
	g_object_unref(accelGroup);
	g_signal_connect(G_OBJECT(m_wMenuPopup), "map",
					   G_CALLBACK(_wd::s_onInitMenu), wd);
	g_signal_connect(G_OBJECT(m_wMenuPopup), "unmap",
					   G_CALLBACK(_wd::s_onDestroyPopupMenu), wd);
	m_vecCallbacks.addItem(static_cast<void *>(wd));
	synthesizeMenu(m_wMenuPopup, true);

	return true;
}

bool EV_UnixMenuPopup::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView, m_wMenuPopup);

	return true;
}

GtkWidget * EV_UnixMenu::s_createNormalMenuEntry(int 		id, 
												 bool isCheckable, 
												 bool isRadio, 
												 bool isPopup,
												 const char *szLabelName, 
												 const char *szMnemonicName)
{
	// create the item with the underscored label
	GtkWidget * w = NULL;
	char buf[1024];
	// convert label into underscored version
	_ev_convert(buf, szLabelName);

	// an item can't be both a checkable and a radio option
	UT_return_val_if_fail(!(isCheckable && isRadio), NULL);

	if ( isCheckable )
	{
		  w = gtk_check_menu_item_new_with_mnemonic(buf);
	}
	else if ( isRadio )
	{
		w = gtk_radio_menu_item_new_with_mnemonic (NULL, buf);
	}
	else
	{
		// else create a normal menu item
		w = gtk_menu_item_new_with_mnemonic(buf);
	}
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
    UT_UNUSED(szMnemonicName);
    UT_UNUSED(isPopup);
#else
	if (szMnemonicName && *szMnemonicName && !isPopup)
	  {
		  guint accelKey = 0;
		  GdkModifierType acMods = (GdkModifierType)0;
		  _convertStringToAccel(szMnemonicName, accelKey, acMods);
		  // the accel doesn't actually do anything, because all the keyboard actions
		  // are handled at a lower level (we just get an accel label)
		  if (accelKey) {
			gtk_widget_add_accelerator (w, "activate", m_accelGroup, accelKey, acMods, GTK_ACCEL_VISIBLE);
		  }
	  }
#endif	  
	
	UT_return_val_if_fail(w, NULL);
	gtk_widget_show(w);
	
	// set menu data to relate to class
	
	// create callback info data for action handling
	_wd * wd = new _wd(this, id);
	UT_ASSERT(wd);
	m_vecCallbacks.addItem(static_cast<void *>(wd));
	// connect callbacks
	g_signal_connect(G_OBJECT(w), "activate", G_CALLBACK(_wd::s_onActivate), wd);
	g_object_set_data(G_OBJECT(w), "wd", wd);
	g_signal_connect(G_OBJECT(w), "select", G_CALLBACK(_wd::s_onMenuItemSelect), wd);
	g_signal_connect(G_OBJECT(w), "deselect", G_CALLBACK(_wd::s_onMenuItemDeselect), wd);				

	return w;
}
