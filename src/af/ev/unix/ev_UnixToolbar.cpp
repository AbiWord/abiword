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
#include "ut_string.h"
#include "ev_UnixToolbar.h"
#include "xap_Types.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_UnixToolbar_Icons.h"
#include "ev_UnixToolbar_ViewListener.h"
#include "xav_View.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/
#define COMBO_BUF_LEN 256

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
	
		_wd * wd = (_wd *) user_data;
		UT_ASSERT(wd);

		if (!wd->m_blockSignal)
			wd->m_pUnixToolbar->toolbarEvent(wd->m_id, 0, 0);
	};

	// TODO: should this move out of wd?  It's convenient here; maybe I'll make
	// a microclass for combo boxes.
	static void s_combo_changed(GtkEntry * widget, gpointer user_data) //blah, gpointer user_data)
	{
		_wd * wd = (_wd *) user_data;
		UT_ASSERT(wd);

		gchar * buffer = gtk_entry_get_text(widget);
		UT_uint32 length = widget->text_length;
 
		UT_UCSChar * text = (UT_UCSChar *) buffer;
		if (!wd->m_blockSignal)
			if (wd->m_widget)
				wd->m_pUnixToolbar->toolbarEvent(wd->m_id, text, length);

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

UT_Bool EV_UnixToolbar::toolbarEvent(AP_Toolbar_Id id,
									 UT_UCSChar * pData,
									 UT_uint32 dataLength)

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

	invokeToolbarMethod(m_pUnixFrame->getCurrentView(),pEM,1,pData,dataLength);
	return UT_TRUE;
}


UT_Bool EV_UnixToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	AP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = m_pUnixFrame->getTopLevelWindow();
	GtkWidget * wVBox = m_pUnixFrame->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	UT_ASSERT(m_wHandleBox);
	
	m_wToolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
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

			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
				{
					UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
					GtkWidget * wPixmap;
					UT_Bool bFoundIcon =
						m_pUnixToolbarIcons->getPixmapForIcon(wTLW->window,
															  &wTLW->style->bg[GTK_STATE_NORMAL],
															  pLabel->getIconName(),
															  &wPixmap);
					UT_ASSERT(bFoundIcon);

					wd->m_widget = gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
														   pLabel->getToolbarLabel(),
														   szToolTip,(const char *)NULL,
														   wPixmap,
														   GTK_SIGNAL_FUNC(_wd::s_callback),
														   wd);
				}
				break;

			case EV_TBIT_ToggleButton:
				{
					UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
					GtkWidget * wPixmap;
					UT_Bool bFoundIcon =
						m_pUnixToolbarIcons->getPixmapForIcon(wTLW->window,
															  &wTLW->style->bg[GTK_STATE_NORMAL],
															  pLabel->getIconName(),
															  &wPixmap);
					UT_ASSERT(bFoundIcon);

					wd->m_widget = gtk_toolbar_append_element(GTK_TOOLBAR(m_wToolbar),
															  GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
															  (GtkWidget *)NULL,
															  pLabel->getToolbarLabel(),
															  szToolTip,(const char *)NULL,
															  wPixmap,
															  GTK_SIGNAL_FUNC(_wd::s_callback),
															  wd);
				}
				break;

			case EV_TBIT_EditText:
				break;
					
			case EV_TBIT_DropDown:
				break;
					
			case EV_TBIT_ComboBox:
			{
				EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
				UT_ASSERT(pControl);

				int iWidth = 100;

				if (pControl)
				{
					iWidth = pControl->getPixelWidth();
				}

				GtkWidget * comboBox = gtk_combo_new();
				UT_ASSERT(comboBox);

				// set the size of the entry to set the total combo size
				gtk_widget_set_usize(GTK_COMBO(comboBox)->entry, iWidth, 0);

				// the entry is read-only for now
				gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(comboBox)->entry), FALSE);
										 
				// we override lots of signals to effect document layout changes
/*				gtk_signal_connect(GTK_OBJECT(GTK_COMBO(comboBox)->entry),
										 "activate",
										 GTK_SIGNAL_FUNC(_wd::s_combo_changed),
										 wd);
*/
				GtkEntry * blah = GTK_ENTRY(GTK_COMBO(comboBox)->entry);
				GtkEditable * yuck = GTK_EDITABLE(blah);
				gtk_signal_connect(GTK_OBJECT(&yuck->widget),
								   "changed",
								   GTK_SIGNAL_FUNC(_wd::s_combo_changed),
								   wd);
				
				// populate it
				if (pControl)
				{
					pControl->populate();

					const UT_Vector * v = pControl->getContents();
					UT_ASSERT(v);

					if (v)
					{
						UT_uint32 items = v->getItemCount();
						for (UT_uint32 m=0; m < items; m++)
						{
							char * sz = (char *)v->getNthItem(m);
							GtkWidget * li = gtk_list_item_new_with_label(sz);
							gtk_widget_show(li);
							gtk_container_add (GTK_CONTAINER(GTK_COMBO(comboBox)->list), li);
						}
					}
				}
 
				// Give a final show
				gtk_widget_show(comboBox);

				// stick it in the toolbar
				gtk_toolbar_append_widget(GTK_TOOLBAR(m_wToolbar),
										  comboBox,
										  szToolTip,
										  (const char *) NULL);
				wd->m_widget = comboBox;

				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
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
					break;
				case EV_TBIT_DropDown:
					break;
				case EV_TBIT_ComboBox:
				{
					UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
					UT_Bool bString = EV_TIS_ShouldUseString(tis);
					
					_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					GtkCombo * item = GTK_COMBO(wd->m_widget);
					UT_ASSERT(item);
						
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);

					// NOTE: we always update the control even if !bString
					// Is this logic correct at all?
					//if (GTK_ENTRY(item->entry)->text_length > 0)
					//	gtk_entry_select_region(GTK_ENTRY(item->entry), 0, GTK_ENTRY(item->entry)->text_length);
					//else

					// block the signals
					// Block the signal, throw the toggle event
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					gtk_entry_set_text(GTK_ENTRY(item->entry), szState);
					wd->m_blockSignal = wasBlocked;
					
					UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
								 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
								 ((bGrayed) ? "disabled" : "enabled"),
								 ((bString) ? szState : "no state")));
					

					/////////////////////////////////////////////////
#if 0					
					UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
					UT_Bool bString = EV_TIS_ShouldUseString(tis);

					HWND hwndCombo = _getControlWindow(id);
					UT_ASSERT(hwndCombo);

					// NOTE: we always update the control even if !bString
					int idx = SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szState);
					if (idx==CB_ERR)
						SetWindowText(hwndCombo, szState);

					UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
								 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
								 ((bGrayed) ? "disabled" : "enabled"),
								 ((bString) ? szState : "no state")));
#endif
				}
				break;
				case EV_TBIT_StaticLabel:
					break;
				case EV_TBIT_Spacer:
					break;
				case EV_TBIT_BOGUS:
					break;
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
