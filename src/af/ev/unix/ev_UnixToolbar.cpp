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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_UnixToolbar.h"
#include "ap_Types.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_EditEventMapper.h"
#include "ap_UnixToolbar_Icons.h"
#include "ev_UnixToolbar_ViewListener.h"
#include "av_View.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixToolbar * pUnixToolbar, AP_Toolbar_Id id, GtkWidget * widget = NULL)
	{
		m_pUnixToolbar = pUnixToolbar;
		m_id = id;
		m_widget = widget;
		m_blockSignal = false;
	};
	
	~_wd(void)
	{
	};

	static void s_callback(GtkWidget * widget, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = (_wd *)user_data;
		UT_ASSERT(wd);

		if (!wd->m_blockSignal)
			wd->m_pUnixToolbar->toolbarEvent(wd->m_id);
	};

	EV_UnixToolbar *	m_pUnixToolbar;
	AP_Toolbar_Id		m_id;
	GtkWidget *			m_widget;
	bool				m_blockSignal;
};

/*****************************************************************/

EV_UnixToolbar::EV_UnixToolbar(AP_UnixApp * pUnixApp, AP_UnixFrame * pUnixFrame,
							   const char * szToolbarLayoutName,
							   const char * szToolbarLabelSetName)
	: EV_Toolbar(pUnixApp->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName)
{
	m_pUnixApp = pUnixApp;
	m_pUnixFrame = pUnixFrame;
	m_pViewListener = 0;
	m_lid = 0;							// view listener id
}

EV_UnixToolbar::~EV_UnixToolbar(void)
{
	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
	_releaseListener();
}

UT_Bool EV_UnixToolbar::toolbarEvent(AP_Toolbar_Id id)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pUnixApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(m_pUnixFrame->getCurrentView(),pEM,1,0,0);
	return UT_TRUE;
}


UT_Bool EV_UnixToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = m_pUnixFrame->getTopLevelWindow();
	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	UT_ASSERT(m_wHandleBox);
	
	m_wToolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH);
	UT_ASSERT(m_wToolbar);
	
	gtk_toolbar_set_button_relief(GTK_TOOLBAR(m_wToolbar), GTK_RELIEF_NONE);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(m_wToolbar), TRUE);
	gtk_toolbar_set_space_size(GTK_TOOLBAR(m_wToolbar), 10);

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		AP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
		{
			_wd * wd = new _wd(this,id);
			UT_ASSERT(wd);


			GtkWidget * wPixmap;
			UT_Bool bFoundIcon =
				m_pUnixToolbarIcons->getPixmapForIcon(wTLW->window,
													  &wTLW->style->bg[GTK_STATE_NORMAL],
													  pLabel->getIconName(),
													  &wPixmap);
			UT_ASSERT(bFoundIcon);

			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
				wd->m_widget = gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
													   pLabel->getToolbarLabel(),
													   szToolTip,(const char *)NULL,
													   wPixmap,
													   GTK_SIGNAL_FUNC(_wd::s_callback),
													   wd);

				break;

			case EV_TBIT_ToggleButton:
				wd->m_widget = gtk_toolbar_append_element(GTK_TOOLBAR(m_wToolbar),
														  GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
														  (GtkWidget *)NULL,
														  pLabel->getToolbarLabel(),
														  szToolTip,(const char *)NULL,
														  wPixmap,
														  GTK_SIGNAL_FUNC(_wd::s_callback),
														  wd);
				break;

			case EV_TBIT_EditText:
				break;
					
			case EV_TBIT_DropDown:
				break;
					
			case EV_TBIT_ComboBox:
				break;
					
			case EV_TBIT_StaticLabel:
				// TODO do these...
				break;
					
			case EV_TBIT_Spacer:
				break;
					
			case EV_TBIT_BOGUS:
			default:
				UT_ASSERT(0);
				break;
			}

			// add item after bindings to catch widget returned to us
			m_vecToolbarWidgets.addItem(wd);
		}
		break;
			
		case EV_TLF_Spacer:
		{
			// Append to the vector even if spacer, to sync up with refresh
			// which expects each item in the layout to have a place in the
			// vector.
			_wd * wd = new _wd(this,id);
			UT_ASSERT(wd);
			m_vecToolbarWidgets.addItem(wd);

			gtk_toolbar_append_space(GTK_TOOLBAR(m_wToolbar));
			break;
		}
		
		default:
			UT_ASSERT(0);
		}
	}

	// show the complete thing
	gtk_widget_show(m_wToolbar);

	// an arbitrary padding to make our document not run into our buttons
	gtk_container_border_width(GTK_CONTAINER(m_wToolbar), 2);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wToolbar);
	gtk_widget_show(m_wHandleBox);
	
	// put it in the vbox
	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, FALSE, 0);

	return UT_TRUE;
}

void EV_UnixToolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
UT_Bool EV_UnixToolbar::bindListenerToView(AV_View * pView)
{
	_releaseListener();
	
	m_pViewListener = new EV_UnixToolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	UT_Bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	refreshToolbar(pView, AV_CHG_ALL);

	return UT_TRUE;
}

UT_Bool EV_UnixToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		AP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
					{
						UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);

						_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
						UT_ASSERT(wd);
						GtkButton * item = GTK_BUTTON(wd->m_widget);
						UT_ASSERT(item);
						
						// Disable/enable toolbar item
						gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);

						UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
									m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
									((bGrayed) ? "disabled" : "enabled")));
					}
					break;
			
				case EV_TBIT_ToggleButton:
					{
						UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
						UT_Bool bToggled = EV_TIS_ShouldBeToggled(tis);

						_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
						UT_ASSERT(wd);
						GtkToggleButton * item = GTK_TOGGLE_BUTTON(wd->m_widget);
						UT_ASSERT(item);
						
						// Press/unpress the item
						//item->active = bToggled;

						// Block the signal, throw the toggle event
						bool wasBlocked = wd->m_blockSignal;
						wd->m_blockSignal = true;
						gtk_toggle_button_set_state(item, bToggled);
						//gtk_toggle_button_toggled(item);
						wd->m_blockSignal = wasBlocked;
						
						// Disable/enable toolbar item
						//gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);
						
						UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
									 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
									 ((bGrayed) ? "disabled" : "enabled"),
									 ((bToggled) ? "pressed" : "not pressed")));
					}
					break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_ComboBox:
				case EV_TBIT_StaticLabel:
					// TODO do these later...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT(0);
					break;
				}
			}
			break;
			
		case EV_TLF_Spacer:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	return UT_TRUE;
}
