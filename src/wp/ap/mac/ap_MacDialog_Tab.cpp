/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Tab.h"
#include "ap_MacDialog_Tab.h"

/*****************************************************************/

XAP_Dialog * AP_MacDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,													 XAP_Dialog_Id id)
{
	AP_MacDialog_Tab * p = new AP_MacDialog_Tab(pFactory,id);
	return p;
}

AP_MacDialog_Tab::AP_MacDialog_Tab(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id)
{
}

AP_MacDialog_Tab::~AP_MacDialog_Tab(void)
{
}

void AP_MacDialog_Tab::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	
}

eTabType AP_MacDialog_Tab::_gatherAlignment()
{
#if 0
	BRadioButton* pToggledButton;

	pToggledButton = (BRadioButton *)newwin->FindView("alignleft");
	if(pToggledButton->Value())
		return FL_TAB_LEFT;
	
	pToggledButton = (BRadioButton *)newwin->FindView("alignright");
	if(pToggledButton->Value())
		return FL_TAB_RIGHT;
	
	pToggledButton = (BRadioButton *)newwin->FindView("aligncenter");
	if(pToggledButton->Value())
		return FL_TAB_CENTER;
		
	pToggledButton = (BRadioButton *)newwin->FindView("aligndecimal");
	if(pToggledButton->Value())
		return FL_TAB_DECIMAL;
		
	pToggledButton = (BRadioButton *)newwin->FindView("alignBar");
	if(pToggledButton->Value())
		return FL_TAB_BAR;
#endif
	return FL_TAB_NONE;
}

void AP_MacDialog_Tab::_setAlignment( eTabType a )
{
#if 0
	BRadioButton* pToggledButton = NULL;
	
	switch(a)
	{
		case FL_TAB_LEFT:
			pToggledButton = (BRadioButton *)newwin->FindView("alignleft");
			break;
			
		case FL_TAB_RIGHT:
			pToggledButton = (BRadioButton *)newwin->FindView("alignright");
			break;
			
		case FL_TAB_CENTER:
			pToggledButton = (BRadioButton *)newwin->FindView("aligncenter");
			break;
			
		case FL_TAB_DECIMAL:
			pToggledButton = (BRadioButton *)newwin->FindView("aligndecimal");
			break;
			
		case FL_TAB_BAR:
			pToggledButton = (BRadioButton *)newwin->FindView("alignBar");
			break;
	}
	
	if(pToggledButton)
		pToggledButton->SetValue(1);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_MacDialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_MacDialog_Tab::_setLeader( eTabLeader a )
{
#if 0
	BRadioButton* pButton = NULL;
	
	switch(a)
	{
		case id_LEADER_NONE:
			pButton = (BRadioButton *)newwin->FindView("tabNoLeader");
			break;
			
		case id_LEADER_DOT:
			pButton = (BRadioButton *)newwin->FindView("tabDot");
			break;
		
		case id_LEADER_DASH:
			pButton = (BRadioButton *)newwin->FindView("tabDash");	
			break;
		
		case id_LEADER_UNDERLINE:
			pButton = (BRadioButton *)newwin->FindView("tabUnderline");
			break;
	}
	
	if(pButton)
		pButton->SetValue(1);
#endif
}

const XML_Char * AP_MacDialog_Tab::_gatherDefaultTabStop()
{
	const XML_Char* pText = NULL;

#if 0
	BTextControl *pControl = (BTextControl *)newwin->FindView("defaultTabStop");
	if(pControl)
	{
		 pText = pControl->Text();
	}	
#endif
	return pText;
}

void AP_MacDialog_Tab::_setDefaultTabStop( const XML_Char* default_tab )
{
#if 0
	BTextControl *pControl = (BTextControl *)newwin->FindView("defaultTabStop");
	if(pControl)
	{
		pControl->SetText(default_tab);
	}	
#endif
}

void AP_MacDialog_Tab::_setTabList( UT_uint32 count )
{
#if 0
	// Loop through the list box and remove it's contents.
	BStringItem* removedItem;
	UT_uint32 i;
	
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		int numItems = pList->CountItems();
		
		for( i = 0; i < numItems; i ++)
		{
			removedItem = (BStringItem *)pList->RemoveItem((int32)0);
			delete removedItem;
		}
	}
	
	for ( i = 0; i < count; i++ )
	{
		pList->AddItem(new BStringItem(_getTabDimensionString(i)));//_win32Dialog.addItemToList(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST, _getTabDimensionString(i));
	}
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_MacDialog_Tab::_gatherSelectTab()
{
#if 0
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		return pList->CurrentSelection();
	}
	
	return -1;
#endif
}

void AP_MacDialog_Tab::_setSelectTab( UT_sint32 v )
{
#if 0
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		pList->Select(v);
	}
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_MacDialog_Tab::_gatherTabEdit()
{
#if 0
	BTextControl* tabStop = (BTextControl *)newwin->FindView("tabPosition");
	if(tabStop)
		return tabStop->Text();
	else
		return NULL;
#endif
}

void AP_MacDialog_Tab::_setTabEdit( const char *pszStr )
{	
#if 0
	BTextControl* tabStop = (BTextControl *)newwin->FindView("tabPosition");
	if(tabStop)
		tabStop->SetText(pszStr);
#endif
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_MacDialog_Tab::_clearList()
{
#if 0
	// Loop through the list box and remove it's contents.
	BStringItem* removedItem;
	
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		int numItems = pList->CountItems();
		
		for(int i = 0; i < numItems; i ++)
		{
			removedItem = (BStringItem *)pList->RemoveItem((int32)0);
			delete removedItem;
		}
	}
#endif
}
