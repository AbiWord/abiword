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
#include "ut_types.h"
#include "ut_stack.h"
#include "ap_Menu_Id.h"
#include "ev_UnixMenu.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"


#define DELETEP(p)		do { if (p) delete p; } while (0)

/*****************************************************************/

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(AP_Menu_Id id, GtkWidget * widget)
	{
		m_id = id;
		m_widget = widget;
	};
	
	~_wd(void)
	{
		// TODO do we need to destroy our widget or will gtk take care of this ??
	};

	static void			s_onActivate(GtkMenuItem * menuItem, gpointer user_data);
	
protected:
	AP_Menu_Id			m_id;
	GtkWidget *			m_widget;
};

void _wd::s_onActivate(GtkMenuItem * menuItem, gpointer user_data)
{
	// this is a static callback method and does not have a 'this' pointer.
	// map the user_data into an object and dispatch the event.
	
	_wd * wd = (_wd *)user_data;
	UT_ASSERT(wd);

	EV_UnixMenu * pUnixMenu = (EV_UnixMenu *)gtk_object_get_user_data(GTK_OBJECT(menuItem));

	pUnixMenu->menuEvent(wd->m_id);
}

/*****************************************************************/

static const char * _ev_FakeName(const char * sz, UT_uint32 k)
{
	// construct a temporary string

	static char buf[128];
	UT_ASSERT(strlen(sz)<120);
	sprintf(buf,"%s%ld",sz,k);
	return buf;
}

/*****************************************************************/

EV_UnixMenu::EV_UnixMenu(AP_UnixApp * pUnixApp, AP_UnixFrame * pUnixFrame)
	: EV_Menu(pUnixApp->getEditMethodContainer())
{
	m_pUnixApp = pUnixApp;
	m_pUnixFrame = pUnixFrame;
}

EV_UnixMenu::~EV_UnixMenu(void)
{
	UT_VECTOR_PURGEALL(_wd,m_vecMenuWidgets);
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

	// TODO decide if we like this lookup here or would rather do the
	// TODO lookup when we synthesize the menu and store the pEM in
	// TODO the _wd entry.  by doing the lookup here, the application
	// TODO will startup a little faster....
	
	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeMenuMethod(m_pUnixFrame->getCurrentView(),pEM,1,0,0);
	return UT_TRUE;
}

UT_Bool EV_UnixMenu::synthesize(void)
{
	// create a GTK menu from the info provided.

	UT_Bool bResult;
	UT_uint32 tmp = 0;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pUnixApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	const EV_Menu_LabelSet * pMenuLabelSet = m_pUnixFrame->getMenuLabelSet();
	UT_ASSERT(pMenuLabelSet);
	
	const EV_Menu_Layout * pMenuLayout = m_pUnixFrame->getMenuLayout();
	UT_ASSERT(pMenuLayout);
	
	UT_uint32 nrLabelItemsInLayout = pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = m_pUnixFrame->getTopLevelWindow();
	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

	m_wMenuBar = gtk_menu_bar_new();
	gtk_object_set_data(GTK_OBJECT(wTLW), "menubar", m_wMenuBar);
	gtk_widget_show(m_wMenuBar);
	gtk_box_pack_start(GTK_BOX(wVBox),m_wMenuBar,FALSE,TRUE,0);
	gtk_widget_set_usize(m_wMenuBar, -1, 30);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	
	UT_Stack stack;
	stack.push(m_wMenuBar);
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		AP_Menu_Id id = pLayoutItem->getMenuId();
		EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		
		const char * szLabelName = pAction->getDynamicLabel(m_pUnixApp);
		if (!szLabelName || !*szLabelName)
			szLabelName = pLabel->getMenuLabel();

		// append "..." to menu item if it raises a dialog
		
		char * buf = NULL;
		if (pAction->raisesDialog())
		{
			buf = new char[strlen(szLabelName) + 4];
			UT_ASSERT(buf);
			strcpy(buf,szLabelName);
			strcat(buf,"...");
			szLabelName = buf;
		}

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		case EV_MLF_BeginSubMenu:
			{
				GtkWidget * w = gtk_menu_item_new_with_label(szLabelName);
				UT_ASSERT(w);
				gtk_object_set_user_data(GTK_OBJECT(w),this);
				_wd * wd = new _wd(id,w);
				UT_ASSERT(wd);
				m_vecMenuWidgets.addItem(wd);
				GtkWidget * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);

				gtk_object_set_data(GTK_OBJECT(m_wMenuBar), szLabelName, w);
				gtk_widget_show(w);
				gtk_container_add(GTK_CONTAINER(wParent),w);
				gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(_wd::s_onActivate),wd);

				if (pLayoutItem->getMenuLayoutFlags() == EV_MLF_BeginSubMenu)
				{
					GtkWidget * wsub = gtk_menu_new();
					UT_ASSERT(wsub);
					gtk_object_set_user_data(GTK_OBJECT(wsub),this);
					_wd * wdsub = new _wd(id,wsub);
					UT_ASSERT(wdsub);
					m_vecMenuWidgets.addItem(wdsub);
					gtk_object_set_data(GTK_OBJECT(m_wMenuBar), _ev_FakeName(szLabelName,tmp++), wsub);
					// note: for GTK 1.0.6, we don't call "gtk_widget_show(wsub);"
					// note: on the popup's (else we get weird stuff on the screen).
					// note: this doesn't seem to matter for 1.1.2....
					gtk_menu_item_set_submenu(GTK_MENU_ITEM(w),wsub);
					stack.push(wsub);
				}
			}
			break;
	
		case EV_MLF_EndSubMenu:
			{
				GtkWidget * wsub = NULL;
				bResult = stack.pop((void **)&wsub);
				UT_ASSERT(bResult);
			}
			break;
			
		case EV_MLF_Separator:
			{
				GtkWidget * w = gtk_menu_item_new();
				UT_ASSERT(w);
				gtk_object_set_user_data(GTK_OBJECT(w),this);
				_wd * wd = new _wd(id,w);
				UT_ASSERT(wd);
				m_vecMenuWidgets.addItem(wd);
				GtkWidget * wParent;
				bResult = stack.viewTop((void **)&wParent);
				UT_ASSERT(bResult);

				gtk_object_set_data(GTK_OBJECT(m_wMenuBar), _ev_FakeName("separator",tmp++), w);
				gtk_widget_show(w);
				gtk_container_add(GTK_CONTAINER(wParent),w);
			}
			break;

		default:
			UT_ASSERT(0);
			break;
		}

		DELETEP(buf);
	}

#ifdef UT_DEBUG
	GtkWidget * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == m_wMenuBar);
#endif
	
	return UT_TRUE;
}

