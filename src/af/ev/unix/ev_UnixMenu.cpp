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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_UnixMenu.h"
#include "ev_UnixMenuBar.h"
#include "ev_UnixMenuPopup.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

/*****************************************************************/

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixMenu * pUnixMenu, XAP_Menu_Id id)
	{
		m_pUnixMenu = pUnixMenu;
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

		wd->m_pUnixMenu->menuEvent(wd->m_id);
	};

	static void s_onMenuItemSelect(GtkWidget * widget, gpointer data)
	{
		UT_ASSERT(widget && data);

		_wd * wd = (_wd *) data;
		UT_ASSERT(wd && wd->m_pUnixMenu);

		XAP_UnixFrame * pFrame = wd->m_pUnixMenu->getFrame();
		UT_ASSERT(pFrame);

		EV_Menu_Label * pLabel = wd->m_pUnixMenu->getMenuLabelSet()->getLabel(wd->m_id);
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
		UT_ASSERT(wd && wd->m_pUnixMenu);

		XAP_UnixFrame * pFrame = wd->m_pUnixMenu->getFrame();
		UT_ASSERT(pFrame);

		pFrame->setStatusMessage(NULL);
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

		// we always clear the status bar when a menu goes away, so we don't
		// leave a message behind
		XAP_UnixFrame * pFrame = wd->m_pUnixMenu->getFrame();
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
		szLabelName = pAction->getDynamicLabel(pUnixFrame,pLabel);
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

EV_UnixMenu::EV_UnixMenu(XAP_UnixApp * pUnixApp, XAP_UnixFrame * pUnixFrame,
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

XAP_UnixFrame * EV_UnixMenu::getFrame(void)
{
	return m_pUnixFrame;
}

UT_Bool EV_UnixMenu::menuEvent(XAP_Menu_Id id)
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
	sprintf(buf,"%s%d",sz,k);
	return buf;
}

/*
  This function is quirky because GTK (up to 1.2.0) is.  When
  creating accelerators for menu items, one calls
  "guint gtk_label_parse_uline(string)", which looks for underscores
  in the string, and makes each character after an underscore
  underlined, throws out the underscore, and returns the character
  the FIRST underscore precedes.  I think it should quit at the
  first underscore, since it can only return one char, but it
  doesn't.

  It does know that underscores can be escaped by preceding them
  with another underscore.  This function skips the first underscore,
  but then pads out all the others, so that feeding it to the
  gtk_label_parse_uline() will do the "right" thing.

  You get to free the string this returns.
*/

static char * _ev_skip_first_underscore_pad_rest(const char * szString)
{
	UT_ASSERT(szString);

	// create a GString to operate on, using insert operations
	GString * string = g_string_new(szString);

	gint underscoreCount = 0;
	for (gint a = 0; a < string->len; a++)
	{
		if (string->str[a] == '_')
		{
			if (underscoreCount > 0)
			{
				// insert character and increment index (a)
				g_string_insert_c(string, a, '_');
				a++;
			}
			underscoreCount++;
		}
	}

	char * newstring = NULL;
	UT_cloneString(newstring, string->str);

	// go ahead and free segment; we have a copy
	g_string_free(string, TRUE);

	return newstring;
}
	
static char _ev_get_underlined_char(const char * szString)
{

	UT_ASSERT(szString);
	
	// return the char right after the underline
	const char * p = szString;
	while (*p && *(p+1))
	{
		if (*p == '_')
			return *++p;
		else
			p++;
	}
	return 0;
}

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

static void _ev_convert(char * bufResult,
						const char * szString)
{
	UT_ASSERT(szString && bufResult);
	
	strcpy(bufResult, szString);

	char * pl = bufResult;
	while (*pl)
	{
		if (*pl == '&')
			*pl = '_';
		pl++;
	}
}

UT_Bool EV_UnixMenu::synthesizeMenu(GtkWidget * wMenuRoot)
{
	UT_DEBUGMSG(("EV_UnixMenu::synthesizeMenu\n"));
    // create a GTK menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
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
			const char ** data = _ev_GetLabelName(m_pUnixApp, m_pUnixFrame, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			if (szLabelName && *szLabelName)
			{
				char buf[1024];
				// convert label into underscored version
				_ev_convert(buf, szLabelName);

				// create the item with the underscored label
				GtkWidget * w = gtk_menu_item_new();
				if ( !pAction->isCheckable() ) 
					w = gtk_menu_item_new();
				else
					w = gtk_check_menu_item_new();
		
				GtkWidget * hbox = gtk_hbox_new(FALSE, 20);
				gtk_widget_show(hbox);
				gtk_container_add(GTK_CONTAINER(w), hbox);

				// label widget
				GtkWidget * label;

				label = gtk_accel_label_new("SHOULD NOT APPEAR");

				// get a newly padded underscore version
				char * padString = _ev_skip_first_underscore_pad_rest(buf);
				UT_ASSERT(padString);
				guint keyCode = gtk_label_parse_uline(GTK_LABEL(label), padString);
				FREEP(padString);
				
				// either path above filled out the label pointer, so pack it
				// in the hbox
				gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
				gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
				
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
				gtk_object_set_data(GTK_OBJECT(wMenuRoot), szLabelName, w);
				// bury in parent
				gtk_menu_append(GTK_MENU(wParent), w);
				// connect callbacks
				gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(_wd::s_onActivate), wd);
				gtk_signal_connect(GTK_OBJECT(w), "select", GTK_SIGNAL_FUNC(_wd::s_onMenuItemSelect), wd);
				gtk_signal_connect(GTK_OBJECT(w), "deselect", GTK_SIGNAL_FUNC(_wd::s_onMenuItemDeselect), wd);				

				// we always set the acccelerators in "normal" menu items, but
				// not top-level (begin_submenus), as shown below
				gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), w);

				// bind to parent item's accel group
				if ((keyCode != GDK_VoidSymbol))
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

				// create the item widget
				GtkWidget * w = gtk_menu_item_new();
				gtk_object_set_user_data(GTK_OBJECT(w), this);
				gtk_widget_show(w);
								
				// create callback info data for action handling
				_wd * wd = new _wd(this, id);
				UT_ASSERT(wd);
				// find parent menu item
				GtkWidget * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);
				gtk_object_set_data(GTK_OBJECT(wMenuRoot), szLabelName, w);
				// bury the widget in parent menu
				gtk_container_add(GTK_CONTAINER(wParent), w);
				
				// since we are starting a new sub menu, create a shell for new items
				GtkWidget * wsub = gtk_menu_new();

				// Here's the tricky part:
				// If the underlined character conflicts with ANY accelerator
				// in the keyboard layer, don't do the underline construction,
				// but instead make a label with no underlines (and no accelerators).

				
				// get the underlined value from the candidate label
				guint keyCode = _ev_get_underlined_char(buf);

				// this pointer will be filled conditionally later
				GtkWidget * label = NULL;

				// GTK triggers the menu accelerators off of MOD1 ***without
				// regard to what XK_ keysym is bound to it.  therefore, if
				// MOD1 is bound to XK_Alt_{L,R}, we do the following.

				UT_Bool bAltOnMod1 = (ev_UnixKeyboard::getAltModifierMask() == GDK_MOD1_MASK);
				UT_Bool bConflict = UT_FALSE;
				
				// Lookup any bindings cooresponding to MOD1-key and the lower-case
				// version of the underlined char, since all the menus ignore upper
				// case (SHIFT-MOD1-[char]) invokations of accelerators.

				if (bAltOnMod1)
				{
					EV_EditEventMapper * pEEM = m_pUnixFrame->getEditEventMapper();
					UT_ASSERT(pEEM);
					EV_EditMethod * pEM = NULL;
					pEEM->Keystroke(EV_EKP_PRESS|EV_EMS_ALT|tolower(keyCode),&pEM);

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
					UT_cloneString(dup, buf);

					// get a clean string
					_ev_strip_underline(dup, buf);
					
					label = gtk_accel_label_new(dup);
					
					if (dup)
					{
						free(dup);
						dup = NULL;
					}
				}
				else
				{
					label = gtk_accel_label_new("SHOULD NOT APPEAR");

					// get a newly padded underscore version
					char * padString = _ev_skip_first_underscore_pad_rest(buf);
					UT_ASSERT(padString);
					guint keyCode = gtk_label_parse_uline(GTK_LABEL(label), padString);
					FREEP(padString);

					if ((keyCode != GDK_VoidSymbol))
					{
						// bind to top level if parent is top level
						if (wParent == wMenuRoot)
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

						// TODO: Should these happen for every widget, even if
						// TODO: we don't register an accelerator?
						gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), w);
										 
					}
				}

				UT_ASSERT(label);
				gtk_widget_show(label);

				// we always set an accel group, even if we don't actually bind any
				// to this widget
				wd->m_accelGroup = gtk_accel_group_new();
				gtk_menu_set_accel_group(GTK_MENU(wsub), wd->m_accelGroup);
				
				// This stuff happens to every label:
				
				// set alignment inside the label; add to container
				gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
				gtk_container_add(GTK_CONTAINER(w), label);

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
				gtk_object_set_data(GTK_OBJECT(wMenuRoot), _ev_FakeName(szLabelName, tmp++), wsub);
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

			gtk_object_set_data(GTK_OBJECT(wMenuRoot), _ev_FakeName("separator",tmp++), w);
			gtk_widget_show(w);
			gtk_menu_append(GTK_MENU(wParent),w);

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
	GtkWidget * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

	// we also have to bind the top level window to our
	// accelerator group for this menu... it needs to join in
	// on the action.
	gtk_accel_group_attach(m_accelGroup, GTK_OBJECT(m_pUnixFrame->getTopLevelWindow()));
	gtk_accel_group_lock(m_accelGroup);

	return UT_TRUE;
}

UT_Bool EV_UnixMenu::_refreshMenu(AV_View * pView, GtkWidget * wMenuRoot)
{
	// update the status of stateful items on menu bar.

	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
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

				// No dynamic label, check/enable
				if (!pAction->hasDynamicLabel())
				{
					// if no dynamic label, all we need to do
					// is enable/disable and/or check/uncheck it.

					GtkWidget * item = (GtkWidget *) m_vecMenuWidgets.getNthItem(k);
					UT_ASSERT(item);

					// check boxes 
					if (GTK_IS_CHECK_MENU_ITEM(item))
						// must use this line instead of calling
						// gtk_check_menu_item_set_active(...) because it
						// generates an "activate" signal	-- shack / sterwill
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
						UT_DEBUGMSG(("Could not update dynamic menu widget vector item %s.\n", k));
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

	return UT_TRUE;
}

/*****************************************************************/

EV_UnixMenuBar::EV_UnixMenuBar(XAP_UnixApp * pUnixApp,
							   XAP_UnixFrame * pUnixFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_UNIXBASEMENU(pUnixApp,pUnixFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_UnixMenuBar::~EV_UnixMenuBar(void)
{
}

UT_Bool EV_UnixMenuBar::synthesizeMenuBar(void)
{
	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

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
	
	// put it in the vbox
 	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, TRUE, 0);

	return UT_TRUE;
}

UT_Bool EV_UnixMenuBar::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuBar);

	return UT_TRUE;
}

/*****************************************************************/

EV_UnixMenuPopup::EV_UnixMenuPopup(XAP_UnixApp * pUnixApp,
								   XAP_UnixFrame * pUnixFrame,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_UNIXBASEMENU(pUnixApp,pUnixFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_UnixMenuPopup::~EV_UnixMenuPopup(void)
{
}

GtkWidget * EV_UnixMenuPopup::getMenuHandle(void) const
{
	return m_wMenuPopup;
}

UT_Bool EV_UnixMenuPopup::synthesizeMenuPopup(void)
{
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
	return UT_TRUE;
}

UT_Bool EV_UnixMenuPopup::refreshMenu(AV_View * pView)
{
	// this makes an exception for initialization where a view
	// might not exist... silly to refresh the menu then; it will
	// happen in due course to its first display
	if (pView)
		return _refreshMenu(pView,m_wMenuPopup);

	return UT_TRUE;
}
