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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Tab.h"
#include "ap_Win32Dialog_Tab.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Tab * p = new AP_Win32Dialog_Tab(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

AP_Win32Dialog_Tab::AP_Win32Dialog_Tab(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id), _win32Dialog(this)
{
}

AP_Win32Dialog_Tab::~AP_Win32Dialog_Tab(void)
{
}

void AP_Win32Dialog_Tab::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	// raise the dialog

	_win32Dialog.runModal(pFrame, AP_DIALOG_ID_TAB, AP_RID_DIALOG_TABS, this);
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Tab::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Tab_TabTitle));

	// localize controls
	_DSX(TABS_OK_BUTTON,				DLG_OK);
	_DSX(TABS_CANCEL_BUTTON,			DLG_Cancel);
										
	_DS(TABS_TAB_STOP_POSITION_LABEL,	DLG_Tab_Label_TabPosition);
	_DS(TABS_TAB_STOPS_CLEARED_LABEL,	DLG_Tab_Label_TabToClear);
	_DS(TABS_DEFAULT_TAB_STOPS_LABEL,	DLG_Tab_Label_DefaultTS);

	_DS(TABS_ALIGNMENT_LABEL,			DLG_Tab_Label_Alignment);
	_DS(TABS_LEFT_RADIO,				DLG_Tab_Radio_Left);
	_DS(TABS_CENTER_RADIO,				DLG_Tab_Radio_Center);
	_DS(TABS_RIGHT_RADIO,				DLG_Tab_Radio_Right);
	_DS(TABS_DECIMAL_RADIO,				DLG_Tab_Radio_Decimal);
	_DS(TABS_BAR_RADIO,					DLG_Tab_Radio_Bar);

	_DS(TABS_LEADER_LABEL,				DLG_Tab_Label_Leader);
	_DS(TABS_NONE_RADIO,				DLG_Tab_Radio_None);
	_DS(TABS_DOTS_RADIO,				DLG_Tab_Radio_Dot);
	_DS(TABS_DASH_RADIO,				DLG_Tab_Radio_Dash);
	_DS(TABS_UNDERLINE_RADIO,			DLG_Tab_Radio_Underline);

	_DS(TABS_SET_BUTTON,				DLG_Tab_Button_Set);
	_DS(TABS_CLEAR_BUTTON,				DLG_Tab_Button_Clear);
	_DS(TABS_CLEAR_ALL_BUTTON,			DLG_Tab_Button_ClearAll);

	_populateWindowData();
	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Tab::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_TABS_OK_BUTTON:
		m_answer = a_OK;
		_storeWindowData();
		EndDialog(hWnd, 1);
		return 1;

	case AP_RID_DIALOG_TABS_CANCEL_BUTTON:
		m_answer = a_CANCEL;
		EndDialog(hWnd, 0);
		return 1;

	case AP_RID_DIALOG_TABS_TAB_STOP_POSITION_EDIT:
		if(wNotifyCode == EN_CHANGE)
		{
			_event_TabChange();
		}
		return 1;

	case AP_RID_DIALOG_TABS_SET_BUTTON:
		_event_Set();
		return 1;

	case AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST:
		if(wNotifyCode == LBN_SELCHANGE)
		{
			UT_uint32 Index = (UT_uint32)_win32Dialog.getListSelectedIndex(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST);
			_event_TabSelected(Index);
		}
		return 1;

	case AP_RID_DIALOG_TABS_LEFT_RADIO:
		_event_AlignmentChange();
		return 1;
		
	case AP_RID_DIALOG_TABS_CENTER_RADIO:
		_event_AlignmentChange();
		return 1;
		
	case AP_RID_DIALOG_TABS_RIGHT_RADIO:
		_event_AlignmentChange();
		return 1;
		
	case AP_RID_DIALOG_TABS_DECIMAL_RADIO:
		_event_AlignmentChange();
		return 1;
		
	case AP_RID_DIALOG_TABS_BAR_RADIO:
		_event_AlignmentChange();
		return 1;

	case AP_RID_DIALOG_TABS_CLEAR_ALL_BUTTON:
		_event_ClearAll();
		return 1;

	case AP_RID_DIALOG_TABS_CLEAR_BUTTON:
		_event_Clear();
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}

}

BOOL AP_Win32Dialog_Tab::_onDeltaPos(NM_UPDOWN * pnmud)
{
	// respond to WM_NOTIFY/UDN_DELTAPOS message
	// return TRUE to prevent the change from happening
	// return FALSE to allow it to occur
	// we may alter the change by changing the fields in pnmud.

	UT_DEBUGMSG(("onDeltaPos: [idFrom %d][iPos %d][iDelta %d]\n",
				 pnmud->hdr.idFrom,pnmud->iPos,pnmud->iDelta));

	_doSpin(id_SPIN_DEFAULT_TAB_STOP, (0 - (UT_sint32) pnmud->iDelta));
				
	return 1;
}

void AP_Win32Dialog_Tab::_controlEnable( tControl id, bool value )
{
	int WinControlID;

	switch(id)
	{
	case id_EDIT_TAB:
		WinControlID = AP_RID_DIALOG_TABS_TAB_STOP_POSITION_EDIT;
		break;

	case id_LIST_TAB:
		WinControlID = AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST;
		break;

	case id_SPIN_DEFAULT_TAB_STOP:
		WinControlID = AP_RID_DIALOG_TABS_DEFAULT_TAB_STOPS_EDIT;
		break;

	case id_ALIGN_LEFT:
		WinControlID = AP_RID_DIALOG_TABS_LEFT_RADIO;
		break;
		
	case id_ALIGN_CENTER:
		WinControlID = AP_RID_DIALOG_TABS_CENTER_RADIO;
		break;
		
	case id_ALIGN_RIGHT:
		WinControlID = AP_RID_DIALOG_TABS_RIGHT_RADIO;
		break;
		
	case id_ALIGN_DECIMAL:
		WinControlID = AP_RID_DIALOG_TABS_DECIMAL_RADIO;
		break;
		
	case id_ALIGN_BAR:
		WinControlID = AP_RID_DIALOG_TABS_BAR_RADIO;
		break;

	case id_LEADER_NONE:
		WinControlID = AP_RID_DIALOG_TABS_NONE_RADIO;
		break;
		
	case id_LEADER_DOT:
		WinControlID = AP_RID_DIALOG_TABS_DOTS_RADIO;
		break;
		
	case id_LEADER_DASH:
		WinControlID = AP_RID_DIALOG_TABS_DASH_RADIO;
		break;
		
	case id_LEADER_UNDERLINE:
		WinControlID = AP_RID_DIALOG_TABS_UNDERLINE_RADIO;
		break;

	case id_BUTTON_SET:
		WinControlID = AP_RID_DIALOG_TABS_SET_BUTTON;
		break;
		
	case id_BUTTON_CLEAR:
		WinControlID = AP_RID_DIALOG_TABS_CLEAR_BUTTON;
		break;
		
	case id_BUTTON_CLEAR_ALL:
		WinControlID = AP_RID_DIALOG_TABS_CLEAR_ALL_BUTTON;
		break;

	case id_BUTTON_OK:
		WinControlID = AP_RID_DIALOG_TABS_OK_BUTTON;
		break;
		
	case id_BUTTON_CANCEL:
		WinControlID = AP_RID_DIALOG_TABS_CANCEL_BUTTON;
		break;

	default:
		WinControlID = 0;
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if(WinControlID)
	{
		_win32Dialog.enableControl(WinControlID, value);
	}

}


eTabType AP_Win32Dialog_Tab::_gatherAlignment()
{
	if(_win32Dialog.isChecked(AP_RID_DIALOG_TABS_LEFT_RADIO))
		return FL_TAB_LEFT;

	if(_win32Dialog.isChecked(AP_RID_DIALOG_TABS_RIGHT_RADIO))
		return FL_TAB_RIGHT;

	if(_win32Dialog.isChecked(AP_RID_DIALOG_TABS_CENTER_RADIO))
		return FL_TAB_CENTER;

	if(_win32Dialog.isChecked(AP_RID_DIALOG_TABS_DECIMAL_RADIO))
		return FL_TAB_DECIMAL;

	if(_win32Dialog.isChecked(AP_RID_DIALOG_TABS_BAR_RADIO))
		return FL_TAB_BAR;

	return FL_TAB_NONE;

}

void AP_Win32Dialog_Tab::_setAlignment( eTabType a )
{
	
	_win32Dialog.checkButton(AP_RID_DIALOG_TABS_LEFT_RADIO, a == FL_TAB_LEFT);

	
	_win32Dialog.checkButton(AP_RID_DIALOG_TABS_RIGHT_RADIO, a == FL_TAB_RIGHT);

	
	_win32Dialog.checkButton(AP_RID_DIALOG_TABS_CENTER_RADIO, a == FL_TAB_CENTER);

	
	_win32Dialog.checkButton(AP_RID_DIALOG_TABS_DECIMAL_RADIO, a == FL_TAB_DECIMAL);

	
	_win32Dialog.checkButton(AP_RID_DIALOG_TABS_BAR_RADIO, a == FL_TAB_BAR);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_Win32Dialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_Win32Dialog_Tab::_setLeader( eTabLeader a )
{
/*
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_ASSERT( id >= id_LEADER_NONE && id <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id );
	UT_ASSERT(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message

	m_bInSetCall = true;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = false;
*/
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const XML_Char * AP_Win32Dialog_Tab::_gatherDefaultTabStop()
{
	_win32Dialog.getControlText(AP_RID_DIALOG_TABS_DEFAULT_TAB_STOPS_EDIT, Buffer, 128);

	return Buffer;
}

void AP_Win32Dialog_Tab::_setDefaultTabStop( const XML_Char* default_tab )
{
	_win32Dialog.setControlText(AP_RID_DIALOG_TABS_DEFAULT_TAB_STOPS_EDIT, default_tab);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void AP_Win32Dialog_Tab::_setTabList( UT_uint32 count )
{
	UT_uint32 i;

	// clear all the items from the list
	_win32Dialog.resetContent(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST);

	for ( i = 0; i < count; i++ )
	{
		_win32Dialog.addItemToList(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST, _getTabDimensionString(i));
	}
	

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_Win32Dialog_Tab::_gatherSelectTab()
{
	return _win32Dialog.getListSelectedIndex(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST);
}

void AP_Win32Dialog_Tab::_setSelectTab( UT_sint32 v )
{
	_win32Dialog.selectListItem(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST, v);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_Win32Dialog_Tab::_gatherTabEdit()
{
	_win32Dialog.getControlText(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_EDIT, Buffer, 128);

	return Buffer;
}

void AP_Win32Dialog_Tab::_setTabEdit( const char *pszStr )
{
	_win32Dialog.setControlText(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_EDIT, pszStr);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_Win32Dialog_Tab::_clearList()
{
	_win32Dialog.resetContent(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST);
}
