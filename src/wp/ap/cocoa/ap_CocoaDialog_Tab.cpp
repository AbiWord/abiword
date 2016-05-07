/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "fl_BlockLayout.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_CocoaDialog_Tab.h"


/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_Tab * p = new AP_CocoaDialog_Tab(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_Tab::AP_CocoaDialog_Tab(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id dlgid)
    : AP_Dialog_Tab(pDlgFactory,dlgid),
		m_dlg(nil),
		m_dataSource(nil)
{
	m_current_alignment = FL_TAB_LEFT;
	m_current_leader	= FL_LEADER_NONE;
}

AP_CocoaDialog_Tab::~AP_CocoaDialog_Tab(void)
{
	[m_dataSource release];
}

/*****************************************************************/

void AP_CocoaDialog_Tab::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_dlg = [[AP_CocoaDialog_TabController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	
	NSWindow* win = [m_dlg window];
	if (!m_dataSource) {
		m_dataSource = [[XAP_StringListDataSource alloc] init];
	}
	else {
		[m_dataSource removeAllStrings];
	}
	[[m_dlg _lookupWidget:id_LIST_TAB] setDataSource:m_dataSource];
    _populateWindowData();
	
	[NSApp runModalForWindow:win];

	switch (m_answer)
	{
	case AP_Dialog_Tab::a_OK:
		_storeWindowData();
		break;

	case AP_Dialog_Tab::a_CANCEL:
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	};
	
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// unix specific handlers 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_CocoaDialog_Tab::event_OK(void)
{
    m_answer = AP_Dialog_Tab::a_OK;
    [NSApp stopModal];
}

void AP_CocoaDialog_Tab::event_Cancel(void)
{
    m_answer = AP_Dialog_Tab::a_CANCEL;
    [NSApp stopModal];
}


void AP_CocoaDialog_Tab::event_Leader(id sender)
{
	m_current_leader = (eTabLeader)[sender tag];
	
	UT_DEBUGMSG(("AP_CocoaDialog_Tab::s_leader_change\n"));
	_event_somethingChanged();
}


void AP_CocoaDialog_Tab::event_Alignment(id sender)
{
	m_current_alignment = (eTabType)[sender tag];

	UT_DEBUGMSG(("AP_CocoaDialog_Tab::s_alignment_change [%c]\n", AlignmentToChar(m_current_alignment)));
	_event_AlignmentChange();
}

/*****************************************************************/


void AP_CocoaDialog_Tab::_controlEnable(tControl ctlid, bool value)
{
	NSControl* ctrl = [m_dlg _lookupWidget:ctlid];
	[ctrl setEnabled:(value?YES:NO)];
}



/*****************************************************************/

eTabType AP_CocoaDialog_Tab::_gatherAlignment()
{
	return m_current_alignment;
}

void AP_CocoaDialog_Tab::_setAlignment(eTabType a)
{
	tControl ctlid = id_ALIGN_LEFT;
	
	
	switch (a)
	{
	case FL_TAB_LEFT:
		ctlid = id_ALIGN_LEFT;
		break;

	case FL_TAB_CENTER:
		ctlid = id_ALIGN_CENTER;
		break;

	case FL_TAB_RIGHT:
		ctlid = id_ALIGN_RIGHT;
		break;

	case FL_TAB_DECIMAL:
		ctlid = id_ALIGN_DECIMAL;
		break;

	case FL_TAB_BAR:
		ctlid = id_ALIGN_BAR;
		break;

		// FL_TAB_NONE, __FL_TAB_MAX
	default:
		return;
	}
	// time to set the alignment radiobutton widget
	NSCell *w = [m_dlg _lookupWidget:ctlid]; 
	UT_ASSERT(w);

	// tell the change routines to ignore this message
	[w setState:NSOnState];

	m_current_alignment = a;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_CocoaDialog_Tab::_gatherLeader()
{
	return m_current_leader;
}

void AP_CocoaDialog_Tab::_setLeader( eTabLeader a )
{
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl ctlid = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_ASSERT( ctlid >= id_LEADER_NONE && ctlid <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	NSCell *w = [m_dlg _lookupWidget:ctlid];
	UT_ASSERT(w);

	// tell the change routines to ignore this message
	[w setState:NSOnState];

	m_current_leader = a;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const gchar* AP_CocoaDialog_Tab::_gatherDefaultTabStop()
{
	NSControl* w = [m_dlg  _lookupWidget:id_SPIN_DEFAULT_TAB_STOP];
	return [[w stringValue] UTF8String];
}

void AP_CocoaDialog_Tab::_setDefaultTabStop(const gchar* defaultTabStop)
{
	NSControl* w = [m_dlg  _lookupWidget:id_SPIN_DEFAULT_TAB_STOP];

	// then set the text
	[w setStringValue:[NSString stringWithUTF8String:defaultTabStop]];
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void AP_CocoaDialog_Tab::_setTabList(UT_uint32 count)
{
	NSTableView* w = [m_dlg _lookupWidget:id_LIST_TAB];
	UT_uint32 i;

	// clear all the items from the list
	[m_dataSource removeAllStrings];

	for (i = 0; i < count; i++) {
		[m_dataSource addString:[NSString stringWithUTF8String:_getTabDimensionString(i)]];
	}
	[w reloadData];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_CocoaDialog_Tab::_gatherSelectTab()
{
	NSTableView* w = [m_dlg _lookupWidget:id_LIST_TAB];
	return [w selectedRow];
}

void AP_CocoaDialog_Tab::_setSelectTab( UT_sint32 v )
{
	NSTableView* w = [m_dlg _lookupWidget:id_LIST_TAB];

	if (v == -1) {	// we don't want to select anything
		[w deselectAll:m_dlg];
	}
	else {
		[w selectRowIndexes:[NSIndexSet indexSetWithIndex:v] byExtendingSelection:NO];
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_CocoaDialog_Tab::_gatherTabEdit()
{
	NSControl* w = [m_dlg  _lookupWidget:id_EDIT_TAB];
	return [[w stringValue] UTF8String];
}

void AP_CocoaDialog_Tab::_setTabEdit( const char *pszStr )
{
	NSControl* w = [m_dlg  _lookupWidget:id_EDIT_TAB];
	[w setStringValue:[NSString stringWithUTF8String:pszStr]];
}


void AP_CocoaDialog_Tab::_clearList()
{
	NSTableView* w = [m_dlg _lookupWidget:id_LIST_TAB];
	[m_dataSource removeAllStrings];
	[w reloadData];
}


@implementation AP_CocoaDialog_TabController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Tab"];
}

- (void)dealloc
{
	DELETEP(_proxy);
	[super dealloc];
}


- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Tab*>(owner);
	UT_ASSERT(_xap);
	_proxy = new AP_CocoaDialog_Tab_Proxy(_xap);
}

- (void)discardXAP
{
	_xap = NULL;
	DELETEP(_proxy);
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Tab_TabTitle);
	LocalizeControl(_tabPositionLabel, pSS, AP_STRING_ID_DLG_Tab_Label_TabPosition);
	LocalizeControl(_tabClearLabel, pSS, AP_STRING_ID_DLG_Tab_Label_TabToClear);
	LocalizeControl(_setBtn, pSS, AP_STRING_ID_DLG_Tab_Button_Set);
	LocalizeControl(_clearBtn, pSS, AP_STRING_ID_DLG_Tab_Button_Clear);
	LocalizeControl(_clearAllBtn, pSS, AP_STRING_ID_DLG_Tab_Button_ClearAll);
	LocalizeControl(_alignmentBox, pSS, AP_STRING_ID_DLG_Tab_Label_Alignment);
	LocalizeControl(_leaderBox, pSS, AP_STRING_ID_DLG_Tab_Label_Leader);
	LocalizeControl(_decimalCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Decimal);
	[_decimalCell setTag:(int)FL_TAB_DECIMAL];
	LocalizeControl(_leftCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Left);
	[_leftCell setTag:(int)FL_TAB_LEFT];
	LocalizeControl(_centerCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Center);
	[_centerCell setTag:(int)FL_TAB_CENTER];
	LocalizeControl(_rightCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Right);
	[_rightCell setTag:(int)FL_TAB_RIGHT];
	LocalizeControl(_barCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Bar);
	[_barCell setTag:(int)FL_TAB_BAR];
	LocalizeControl(_leaderDashCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Dash);
	[_leaderDashCell setTag:(int)FL_LEADER_HYPHEN];
	LocalizeControl(_leaderDotCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Dot);
	[_leaderDotCell setTag:(int)FL_LEADER_DOT];
	LocalizeControl(_leaderNoneCell, pSS, AP_STRING_ID_DLG_Tab_Radio_None);
	[_leaderNoneCell setTag:(int)FL_LEADER_NONE];
	LocalizeControl(_leaderUnderlineCell, pSS, AP_STRING_ID_DLG_Tab_Radio_Underline);
	[_leaderUnderlineCell setTag:(int)FL_LEADER_UNDERLINE];
	LocalizeControl(_defaultTabLabel, pSS, AP_STRING_ID_DLG_Tab_Label_DefaultTS);
	[_tabList setAction:@selector(tabListAction:)];
	[_tabList setTarget:self];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->cancelAction();
}

- (IBAction)clearAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->clearAction();
}

- (IBAction)clearAllAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->clearAllAction();
}

- (IBAction)defaultTabAction:(id)sender
{
	UT_UNUSED(sender);
}

- (IBAction)defaultTabStepperAction:(id)sender
{
	UT_UNUSED(sender);
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->okAction();
}

- (IBAction)setAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->setAction();
}

- (IBAction)alignmentAction:(id)sender
{
	_proxy->alignmentAction([sender selectedCell]);
}


- (IBAction)leaderAction:(id)sender
{
	_proxy->leaderAction([sender selectedCell]);
}

- (IBAction)positionEditAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->positionEditAction();
}

- (void)tabListAction:(id)sender
{
	UT_UNUSED(sender);
	_proxy->tabListAction([_tabList selectedRow]);
}

- (id)_lookupWidget:(AP_Dialog_Tab::tControl)ctlid
{
	switch(ctlid) {
	case AP_Dialog_Tab::id_EDIT_TAB:
		return _tabPositionData;
	case AP_Dialog_Tab::id_LIST_TAB:
		return _tabList;
	case AP_Dialog_Tab::id_SPIN_DEFAULT_TAB_STOP:
		return _defaultTabData;
	case AP_Dialog_Tab::id_ALIGN_LEFT:
		return _leftCell;
	case AP_Dialog_Tab::id_ALIGN_CENTER:
		return _centerCell;
	case AP_Dialog_Tab::id_ALIGN_RIGHT:
		return _rightCell;
	case AP_Dialog_Tab::id_ALIGN_DECIMAL:
		return _decimalCell;
	case AP_Dialog_Tab::id_ALIGN_BAR:
		return _barCell;
	case AP_Dialog_Tab::id_LEADER_NONE:
		return _leaderNoneCell;
	case AP_Dialog_Tab::id_LEADER_DOT:
		return _leaderDotCell;
	case AP_Dialog_Tab::id_LEADER_DASH:
		return _leaderDashCell;
	case AP_Dialog_Tab::id_LEADER_UNDERLINE:
		return _leaderUnderlineCell;
	case AP_Dialog_Tab::id_BUTTON_SET:
		return _setBtn;
	case AP_Dialog_Tab::id_BUTTON_CLEAR:
		return _clearBtn;
	case AP_Dialog_Tab::id_BUTTON_CLEAR_ALL:
		return _clearAllBtn;
	case AP_Dialog_Tab::id_BUTTON_OK:
		return _okBtn;
	case AP_Dialog_Tab::id_BUTTON_CANCEL:
		return _cancelBtn;
	case AP_Dialog_Tab::id_last:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return nil;
}

@end


