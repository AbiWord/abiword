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

	static void s_onActivate(gpointer * callback_data, guint callback_action, GtkWidget * widget)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		EV_UnixMenu * menu = (EV_UnixMenu *) callback_data;

		_wd * wd = (_wd *) menu->_get_MenuVector(callback_action);
		UT_ASSERT(wd);

		wd->m_pUnixMenu->menuEvent(wd->m_id);
	};

	static void s_onInitMenu(GtkMenuItem * menuItem, gpointer user_data)
	{
		_wd * wd = (_wd *)user_data;
		UT_ASSERT(wd);

		wd->m_pUnixMenu->refreshMenu(wd->m_pUnixMenu->getFrame()->getCurrentView());
	};
		
	EV_UnixMenu *		m_pUnixMenu;
	AP_Menu_Id			m_id;
};


/*****************************************************************/

static const char * _ev_GetLabelName(AP_UnixApp * pUnixApp,
									 EV_Menu_Action * pAction,
									 EV_Menu_Label * pLabel)
{
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pUnixApp,pLabel);
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

static const char * _ev_gtkMenuFactoryName(const char * szMenuName)
{
	// construct the menu bar factory name for gtk.
	// return pointer to static string.  caller must use or copy before next call.
	
	static char buf[128];

	sprintf(buf,"<%s>",szMenuName);
	return buf;
}

/*****************************************************************/

EV_UnixMenu::EV_UnixMenu(AP_UnixApp * pUnixApp, AP_UnixFrame * pUnixFrame,
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pUnixApp->getEditMethodContainer(),szMenuLayoutName,szMenuLabelSetName)
{
	m_pUnixApp = pUnixApp;
	m_pUnixFrame = pUnixFrame;
}

EV_UnixMenu::~EV_UnixMenu(void)
{
	UT_VECTOR_PURGEALL(_wd *,m_vecMenuWidgets);
}

UT_Bool EV_UnixMenu::refreshMenu(AV_View * pView)
{
	return _refreshMenu(pView);
}

AP_UnixFrame * EV_UnixMenu::getFrame(void)
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

	invokeMenuMethod(m_pUnixFrame->getCurrentView(),pEM,1,0,0);
	return UT_TRUE;
}

UT_Bool EV_UnixMenu::synthesize(void)
{
    // create a GTK menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = m_pUnixFrame->getTopLevelWindow();
	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	UT_ASSERT(m_wHandleBox);
	
	m_wAccelGroup = gtk_accel_group_new();
	const char * szMenuBarName = _ev_gtkMenuFactoryName(m_pMenuLayout->getName());
	m_wMenuBarItemFactory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,szMenuBarName,m_wAccelGroup);

	// we allocate space for nrLabelItemsInLayout even though our
	// format includes entries not found in the corresponding gtk
	// menu factory layout.  we'll just leave the extra ones zero.
	
	m_menuFactoryItems = (GtkItemFactoryEntry *)calloc(sizeof(GtkItemFactoryEntry),nrLabelItemsInLayout);
	UT_ASSERT(m_menuFactoryItems);
	m_nrActualFactoryItems = 0;

	char bufMenuPathname[1024];
	*bufMenuPathname = 0;

	UT_Stack callbackPathStack;
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		const char * szLabelName;
		char * tempPath = NULL;
		
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			szLabelName = _ev_GetLabelName(m_pUnixApp, pAction, pLabel);
			if (szLabelName && *szLabelName)
				_append_NormalItem(bufMenuPathname,szLabelName,id,pAction->isCheckable());
			break;	
		
		case EV_MLF_BeginSubMenu:
			szLabelName = _ev_GetLabelName(m_pUnixApp, pAction, pLabel);
			if (szLabelName && *szLabelName)
				_append_SubMenu(bufMenuPathname,szLabelName,id);
			// push our pathnames on to a stack to be popped and deleted later
			tempPath = strdup((const char *) bufMenuPathname);
			callbackPathStack.push((void *) tempPath);
			break;
	
		case EV_MLF_EndSubMenu:
			_end_SubMenu(bufMenuPathname);
			break;
			
		case EV_MLF_Separator:
			_append_Separator(bufMenuPathname,id);
			break;

		default:
			UT_ASSERT(0);
			break;
		}
	}
 
	gtk_item_factory_create_items(m_wMenuBarItemFactory,
								  m_nrActualFactoryItems,
								  m_menuFactoryItems,
								  this); // "window" pointer lets us resolve indexes
	
	gtk_accel_group_attach(m_wAccelGroup,GTK_OBJECT(wTLW));

	m_wMenuBar = gtk_item_factory_get_widget(m_wMenuBarItemFactory, szMenuBarName);

	/*
	  Pop strings off our stack and grab a pointer to the
	  widget they represent.
	*/
	char ** ppString = new char *;
	while (callbackPathStack.pop((void **) ppString))
	{
		GtkWidget * tlMenuItem = gtk_item_factory_get_widget(m_wMenuBarItemFactory,
															 *ppString);
		if (tlMenuItem)
		{
			// tip:  "map" is the signal that mimics WM_INITMENU.  This signal
			// is fired before any painting is done (right on click)
			char ourSignal[] = "map";
			_wd * wd = new _wd(this, (AP_Menu_Id) NULL);
			UT_ASSERT(wd);
			if(!gtk_signal_connect(GTK_OBJECT(tlMenuItem),
								   ourSignal,
								   GTK_SIGNAL_FUNC(_wd::s_onInitMenu),
								   wd))
			{
				UT_DEBUGMSG(("*** Failed menu signal [%s] connect to [%p], menu item [%s].\n",
							 ourSignal, tlMenuItem, *ppString));
			}
		}
		delete *ppString;
	}
	delete ppString;
	
	// show up the properly connected menu structure
	gtk_widget_show(m_wMenuBar);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wMenuBar);
	gtk_widget_show(m_wHandleBox);
	
	// put it in the vbox
 	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, TRUE, 0);

	return UT_TRUE;
}

UT_Vector * EV_UnixMenu::_get_MenuVector(UT_uint32 n)
{
	UT_ASSERT(m_vecMenuWidgets.getNthItem(n));
       
	return (UT_Vector *) m_vecMenuWidgets.getNthItem(n);
}

static void _ev_strip_accel(char * bufResult,
							const char * szString)
{
	int i = 0;
	int j = 0;
	while (szString[i] != 0)
	{
		if (szString[i] != '_')
		{
			bufResult[j] = szString[i];
			j++;
		}
		i++;
	}
	bufResult[j++] = NULL;
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

static void _ev_concat_and_convert(char * bufResult,
								   const char * szPrefix,
								   const char * szLabelName)
{
	// Win32 uses '&' in it's menu strings to denote an accelerator.
	// GTK MenuFactory uses '_'.  We do the conversion.

	strcpy(bufResult,szPrefix);
	char * pb = bufResult + strlen(bufResult);
	*pb++ = '/';

	const char * pl = szLabelName;
	while (*pl)
	{
		if (*pl == '&')
			*pb++ = '_';
		else
			*pb++ = *pl;
		pl++;
	}
	*pb = 0;
}

void EV_UnixMenu::_append_NormalItem(char * bufMenuPathname,const char * szLabelName,AP_Menu_Id id,UT_Bool bCheckable)
{
	GtkItemFactoryEntry * p = &m_menuFactoryItems[m_nrActualFactoryItems++];

	char buf[1024];
	_ev_concat_and_convert(buf,bufMenuPathname,szLabelName);
	UT_cloneString(p->path,buf);

	// TODO search our keybindings and see if there's one or more
	// TODO keys set to the function that this menu item is bound
	// TODO to.  if so, add an accelerator to the following.
	// TODO note that this is independent of any mnemonic given on
	// TODO the menu label.
	
	p->accelerator = NULL;
	p->callback = (GtkItemFactoryCallback)_wd::s_onActivate;

	_wd * wd = new _wd(this,id);
	UT_ASSERT(wd);
	m_vecMenuWidgets.addItem(wd);

	p->callback_action = m_vecMenuWidgets.findItem(wd);
	p->item_type = NULL;
	if (bCheckable)
		p->item_type = "<CheckItem>";
}

void EV_UnixMenu::_append_SubMenu(char * bufMenuPathname,const char * szLabelName,AP_Menu_Id id)
{
	GtkItemFactoryEntry * p = &m_menuFactoryItems[m_nrActualFactoryItems++];

	// we are given 2 strings -- something of the form: "/Edit" and "&Align"
	// build string for the sub-menu (with accelerator) and then yank the "&"
	// and extend the prefix.

	// TODO if this is a top-level sub-menu (like "File" or "Edit")
	// TODO and there's an accelerator (mnemonic) defined in the label
	// TODO string ("&File"), verify that the we haven't already bound
	// TODO that particular keyboard action (Alt-F) to some EditMethod
	// TODO in the keybindings.  if so, we should strip off the '&',
	// TODO so that GTK will ignore the key sequence and let our
	// TODO keyboard handler take care of it.
	
	char buf[1024];
	_ev_concat_and_convert(buf,bufMenuPathname,szLabelName);
	UT_cloneString(p->path,buf);

	p->accelerator = NULL;
	p->callback = NULL;

	_wd * wd = new _wd(this,id);
	UT_ASSERT(wd);
	m_vecMenuWidgets.addItem(wd);

	p->callback_action = m_vecMenuWidgets.findItem(wd);
	p->item_type = "<Branch>";

	// append this name to the prefix, omitting the "&".

	char * pb = bufMenuPathname + strlen(bufMenuPathname);
	*pb++ = '/';
	const char * pl = szLabelName;
	while (*pl)
	{
		if (*pl != '&')
			*pb++ = *pl;
		pl++;
	}
	*pb = 0;
}

void EV_UnixMenu::_end_SubMenu(char * bufMenuPathname)
{
	// trim trailing component from pathname

	char * pLastSlash = strrchr(bufMenuPathname,'/');
	if (pLastSlash && *pLastSlash)
		*pLastSlash = 0;
}

void EV_UnixMenu::_append_Separator(char * bufMenuPathname, AP_Menu_Id id)
{
	GtkItemFactoryEntry * p = &m_menuFactoryItems[m_nrActualFactoryItems++];

	static int tmp = 0;

	char buf[1024];
	sprintf(buf,"%s/separator%d",bufMenuPathname,tmp++);
	UT_cloneString(p->path,buf);

	p->accelerator = NULL;
	p->callback = (GtkItemFactoryCallback)_wd::s_onActivate;

	_wd * wd = new _wd(this,id);
	UT_ASSERT(wd);
	m_vecMenuWidgets.addItem(wd);

	p->callback_action = m_vecMenuWidgets.findItem(wd);
	p->item_type = "<Separator>";
}

UT_Bool EV_UnixMenu::_refreshMenu(AV_View * pView)
{
	// update the status of stateful items on menu bar.

    // This function is a big hack, and is tied to the Window menu
	// in the "new item" stage.  Must fix.
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();

	gchar buf[1024];
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		const char * szMenuFactoryItemPath = _getItemPath(id);
		UT_Bool bPresent = _isItemPresent(id);

		GtkWidget * item;
		
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

				// we can only get an item pointer if the widget already has a
				// path.  If it's missing one, code to create one happens later.
				if (szMenuFactoryItemPath)
				{
					// Get a pointer to the current item for later queries

					// Strip out the underscores from the path
					// or else the lookup in the hash will fail.
					_ev_strip_accel(buf, szMenuFactoryItemPath);
					item = gtk_item_factory_get_widget(m_wMenuBarItemFactory, buf);

					if (item)
						bPresent = UT_TRUE;
			
					if (!pAction->hasDynamicLabel())
					{
						if (bPresent)
						{
							// if no dynamic label, all we need to do
							// is enable/disable and/or check/uncheck it.

							// Strip out the underscores from the path
							// or else the lookup in the hash will fail.
							_ev_strip_accel(buf, szMenuFactoryItemPath);
							item = gtk_item_factory_get_widget(m_wMenuBarItemFactory,
															   buf);
							UT_ASSERT(item);

							// check boxes 
							if (GTK_IS_CHECK_MENU_ITEM(item))
								GTK_CHECK_MENU_ITEM(item)->active = bCheck;
							// all get the gray treatment

							gtk_widget_set_sensitive(GTK_WIDGET(item), bEnable);
							break;
						}
					}
				}
				
				// this item has a dynamic label...
				// compute the value for the label.
				// if it is blank, we remove the item from the menu.

				const char * szLabelName = _ev_GetLabelName(m_pUnixApp,pAction,pLabel);

				UT_Bool bRemoveIt = (!szLabelName || !*szLabelName);

				if (bRemoveIt)			// we don't want it to be there
				{
					// happens only if the item has a path and is found
					// in the item factory
					if (bPresent)
					{
						_ev_strip_accel(buf, szMenuFactoryItemPath);
						gtk_item_factory_delete_item(m_wMenuBarItemFactory,
													 buf);
					}
					break;
				}

				// we want the item in the menu.
				if (bPresent)			// just update the label on the item.
				{
					if (strcmp(szLabelName,szMenuFactoryItemPath)==0)
					{
						// dynamic label has not changed, all we need to do
						// is enable/disable and/or check/uncheck it.

						if (GTK_IS_CHECK_MENU_ITEM(item))
							GTK_CHECK_MENU_ITEM(item)->active = bCheck;
						gtk_widget_set_sensitive((GtkWidget *) item, bEnable);

					}
					else
					{
						// dynamic label has changed, do the complex modify.

						/****************************************************************
						  BIG HACK:  We update the menu label directly, which means we
						  do not update the mnemonics.  This will work for the Window
						  menu only, since the mnemonics are always the first character,
						  which is always the same since it was created.  This needs to
						  be fixed!
						****************************************************************/

						// bail if we don't have an item (widget) yet, but are created
						if (!item)
							break;
						
						// Get a list of children, one of which is our label
						GList * children = gtk_container_children(GTK_CONTAINER(item));
						UT_ASSERT(children);
						// First item should be the label (this might be unsafe)
						GList * labelChild = g_list_first(children);
						UT_ASSERT(labelChild);

						gchar buf2[1024];
						_ev_convert(buf, szLabelName);
						_ev_strip_accel(buf2, buf);
						gtk_label_set(GTK_LABEL((GtkTypeObject *)labelChild->data), buf2);
					}
				}
				else
				{
					GtkItemFactoryEntry * p = &m_menuFactoryItems[m_nrActualFactoryItems++];

					// BAD!
					_ev_concat_and_convert(buf, "/Window", szLabelName);
					UT_cloneString(p->path,buf);

					p->accelerator = NULL;
					p->callback = (GtkItemFactoryCallback)_wd::s_onActivate;

					_wd * wd = new _wd(this,id);
					UT_ASSERT(wd);

					p->callback_action = m_vecMenuWidgets.findItem(wd);
					p->item_type = NULL;

					// Currently no checkable dynamic items
                    //if (bCheckable)
					//p->item_type = "<CheckItem>";

					// Why am I passing a 1?  Gtk's item factory code does it when
					// given multiple items to create.
					gtk_item_factory_create_item(m_wMenuBarItemFactory, p, this, 1); 
				}
			}
			break;
		case EV_MLF_Separator:
			break;

		case EV_MLF_BeginSubMenu:
			break;

		case EV_MLF_EndSubMenu:
			break;

		default:
			UT_ASSERT(0);
			break;
		}	
	}

	return UT_TRUE;
}

const char * EV_UnixMenu::_getItemPath(AP_Menu_Id id) const
{
	for (UT_uint32 k=0; (k < m_nrActualFactoryItems); k++)
	{
		_wd * wd = (_wd *)m_vecMenuWidgets.getNthItem(m_menuFactoryItems[k].callback_action);
		if (wd && (wd->m_id==id))
			return m_menuFactoryItems[k].path;
	}

	return NULL;
}

UT_Bool EV_UnixMenu::_isItemPresent(AP_Menu_Id id) const
{
	for (UT_uint32 k=0; (k < m_nrActualFactoryItems); k++)
	{
		_wd * wd = (_wd *)m_vecMenuWidgets.getNthItem(m_menuFactoryItems[k].callback_action);
		if (wd && (wd->m_id==id))
			return UT_TRUE;
	}

	return UT_FALSE;
}
