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

#include "ut_assert.h"
#include "ev_Win32Toolbar.h"
#include "ap_Toolbar_Id.h"
#include "ap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_EditEventMapper.h"
#include "ap_Win32Toolbar_Icons.h"

/*****************************************************************/

EV_Win32Toolbar::EV_Win32Toolbar(AP_Win32App * pWin32App, AP_Win32Frame * pWin32Frame,
								 const char * szToolbarLayoutName,
								 const char * szToolbarLabelSetName)
	: EV_Toolbar(pWin32App->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName)
{
	m_pWin32App = pWin32App;
	m_pWin32Frame = pWin32Frame;
}

EV_Win32Toolbar::~EV_Win32Toolbar(void)
{
	UT_VECTOR_PURGEALL(_wd,m_vecToolbarWidgets);
}

UT_Bool EV_Win32Toolbar::toolbarEvent(AP_Toolbar_Id id)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return UT_TRUE iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return UT_FALSE;
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(m_pWin32Frame->getCurrentView(),pEM,1,0,0);
	return UT_TRUE;
}


UT_Bool EV_Win32Toolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	HWND * wTLW = m_pWin32Frame->getTopLevelWindow();

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
#if 0
				UT_Bool bFoundIcon =
					m_pWin32ToolbarIcons->getPixmapForIcon(wTLW->window,
														  &wTLW->style->bg[GTK_STATE_NORMAL],
														  pLabel->getIconName(),
														  &wPixmap);
				UT_ASSERT(bFoundIcon);
#endif
				const char * szToolTip = pLabel->getToolTip();
				if (!szToolTip || !*szToolTip)
					szToolTip = pLabel->getStatusMsg();

				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
#if 0
					gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
											pLabel->getToolbarLabel(),
											szToolTip,(const char *)NULL,
											wPixmap,
											GTK_SIGNAL_FUNC(_wd::s_callback),
											wd);
#endif
					break;

				case EV_TBIT_ToggleButton:
#if 0
					gtk_toolbar_append_element(GTK_TOOLBAR(m_wToolbar),
											   GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
											   (GtkWidget *)NULL,
											   pLabel->getToolbarLabel(),
											   szToolTip,(const char *)NULL,
											   wPixmap,
											   GTK_SIGNAL_FUNC(_wd::s_callback),
											   wd);
#endif
					break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_ComboBox:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT(0);
					break;
				}
			}
		
		case EV_TLF_Spacer:
			// TODO do this....
			break;
			
		default:
			UT_ASSERT(0);
		}
	}

	return UT_TRUE;
}
