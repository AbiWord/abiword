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

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ap_QNXDialog_Tab.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Tab * p = new AP_QNXDialog_Tab(pFactory,id);
	return p;
}

AP_QNXDialog_Tab::AP_QNXDialog_Tab(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id)
{
	m_current_alignment = FL_TAB_LEFT;
	m_current_leader	= FL_LEADER_NONE;
}

AP_QNXDialog_Tab::~AP_QNXDialog_Tab(void)
{
}

void AP_QNXDialog_Tab::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

    // Build the window's widgets and arrange them
    PtWidget_t * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

	connectFocus(mainWindow, pFrame);
	// save for use with event
	m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData();

    // To center the dialog, we need the frame of its parent.
    XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
    UT_ASSERT(pQNXFrame);
    
    // Get the GtkWindow of the parent frame
    PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    // Make it modal, and stick it up top
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

    // Show the top level dialog,
	PtRealizeWidget(mainWindow);

    // Run the event loop for this window.
	int count = PtModalStart();

	do {
		done = 0;
		while(!done) {
    		PtProcessEvent();
		}

		switch ( m_answer )
		{
		case AP_Dialog_Tab::a_OK:
			_storeWindowData();
			break;

		case AP_Dialog_Tab::a_APPLY:
			_storeWindowData();
			break;

		case AP_Dialog_Tab::a_CANCEL:
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		};

	} while ( m_answer == AP_Dialog_Tab::a_APPLY );	

	PtModalEnd(MODAL_END_ARG(count));
	
	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Tab::event_OK(void)
{
	if (!done++) {
	    m_answer = AP_Dialog_Tab::a_OK;
	}
}

void AP_QNXDialog_Tab::event_Cancel(void)
{
	if (!done++) {
    	m_answer = AP_Dialog_Tab::a_CANCEL;
	}
}

void AP_QNXDialog_Tab::event_Apply(void)
{
	if (!done++) {
	    m_answer = AP_Dialog_Tab::a_APPLY;
	}
}

void AP_QNXDialog_Tab::event_WindowDelete(void)
{
	if (!done++) {
	    m_answer = AP_Dialog_Tab::a_CANCEL;    
	}
}

PtWidget_t* AP_QNXDialog_Tab::_constructWindow (void )
{
	
	PtWidget_t *windowTabs;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	PtArg_t args[10];
	int 	n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue( AP_STRING_ID_DLG_Tab_TabTitle), 0);
	windowTabs = PtCreateWidget(PtWindow, NULL, n, args);


	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vgroup = PtCreateWidget(PtGroup, windowTabs, n, args);

	PtWidget_t *hcontrolgroup, *htmpgroup, *vtmpgroup;

	n = 0;
	hcontrolgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	/* Lists ... */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vtmpgroup = PtCreateWidget(PtGroup, hcontrolgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Tab stop position: ", 0);
	PtCreateWidget(PtLabel, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *entryTabEntry = PtCreateWidget(PtText, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *listTabs = PtCreateWidget(PtList, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Tab stops to be cleared: ", 0);
	PtCreateWidget(PtLabel, vtmpgroup, n, args);

	/* Choices ... */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vtmpgroup = PtCreateWidget(PtGroup, hcontrolgroup, n, args);

	n = 0;
	htmpgroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Default tab stops: ", 0);
	PtCreateWidget(PtLabel, htmpgroup, n, args);

	n = 0;
	PtWidget_t *spinbuttonTabstop = PtCreateWidget(PtNumericInteger, htmpgroup, n, args);

	PtWidget_t *agroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE, Pt_SHOW_TITLE);
	PtSetArg(&args[n++], Pt_ARG_TITLE, "Alignment", 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	agroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Left", 0);
	PtWidget_t *radiobuttonLeft = PtCreateWidget(PtToggleButton, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Center", 0);
	PtWidget_t *radiobuttonCenter = PtCreateWidget(PtToggleButton, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Right", 0);
	PtWidget_t *radiobuttonRight = PtCreateWidget(PtToggleButton, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Decimal", 0);
	PtWidget_t *radiobuttonDecimal = PtCreateWidget(PtToggleButton, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Bar", 0);
	PtWidget_t *radiobuttonBar = PtCreateWidget(PtToggleButton, agroup, n, args);


	PtWidget_t *lgroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE, Pt_SHOW_TITLE);
	PtSetArg(&args[n++], Pt_ARG_TITLE, "Leader", 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	lgroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "None", 0);
	PtWidget_t *radiobuttonLeaderNone = PtCreateWidget(PtToggleButton, lgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "..........", 0);
	PtWidget_t *radiobuttonLeaderDot = PtCreateWidget(PtToggleButton, lgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "----------", 0);
	PtWidget_t *radiobuttonLeaderDash = PtCreateWidget(PtToggleButton, lgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "__________", 0);
	PtWidget_t *radiobuttonLeaderUnderline = PtCreateWidget(PtToggleButton, lgroup, n, args);

	/* Setting buttons */
	n = 0;
	htmpgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Set", 0);
	PtWidget_t *buttonSet = PtCreateWidget(PtButton, htmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Clear", 0);
	PtWidget_t *buttonClear = PtCreateWidget(PtButton, htmpgroup, n, args);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Clear All", 0);
	PtWidget_t *buttonClearAll = PtCreateWidget(PtButton, htmpgroup, n, args);

	/* Other buttons */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, Pt_TOP_BEVEL, Pt_ALL);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	htmpgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Apply", 0);
	PtWidget_t *buttonApply = PtCreateWidget(PtButton, htmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "OK", 0);
	PtWidget_t *buttonOK = PtCreateWidget(PtButton, htmpgroup, n, args);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Cancel", 0);
	PtWidget_t *buttonCancel = PtCreateWidget(PtButton, htmpgroup, n, args);

	m_Widgets.setNthItem( id_EDIT_TAB,				entryTabEntry,		NULL);
	m_Widgets.setNthItem( id_LIST_TAB,				listTabs,			NULL);
	m_Widgets.setNthItem( id_SPIN_DEFAULT_TAB_STOP,	spinbuttonTabstop,	NULL);

	m_Widgets.setNthItem( id_ALIGN_LEFT,			radiobuttonLeft,	NULL);
	m_Widgets.setNthItem( id_ALIGN_CENTER,			radiobuttonCenter,	NULL);
	m_Widgets.setNthItem( id_ALIGN_RIGHT,			radiobuttonRight,	NULL);
	m_Widgets.setNthItem( id_ALIGN_DECIMAL,			radiobuttonDecimal,	NULL);
	m_Widgets.setNthItem( id_ALIGN_BAR,				radiobuttonBar,		NULL);

	m_Widgets.setNthItem( id_LEADER_NONE,			radiobuttonLeaderNone,		NULL);
	m_Widgets.setNthItem( id_LEADER_DOT,			radiobuttonLeaderDot,		NULL);
	m_Widgets.setNthItem( id_LEADER_DASH,			radiobuttonLeaderDash,		NULL);
	m_Widgets.setNthItem( id_LEADER_UNDERLINE,		radiobuttonLeaderUnderline,	NULL);

	m_Widgets.setNthItem( id_BUTTON_SET,			buttonSet,					NULL);
	m_Widgets.setNthItem( id_BUTTON_CLEAR,			buttonClear,				NULL);
	m_Widgets.setNthItem( id_BUTTON_CLEAR_ALL,		buttonClearAll,				NULL);

	m_Widgets.setNthItem( id_BUTTON_OK,				buttonOK,					NULL);
	m_Widgets.setNthItem( id_BUTTON_CANCEL,			buttonCancel,				NULL);
	m_Widgets.setNthItem( id_BUTTON_APPLY,			buttonApply,				NULL);
	
	return windowTabs;
}

/* Stolen from the UNIX code */
PtWidget_t *AP_QNXDialog_Tab::_lookupWidget ( tControl id )
{
	UT_ASSERT(m_Widgets.getItemCount() > (UT_uint32)id );

	PtWidget_t *w = (PtWidget_t *)m_Widgets.getNthItem((UT_uint32)id);
	UT_ASSERT(w);

	return w;
}


void AP_QNXDialog_Tab::_controlEnable( tControl id, UT_Bool value )
{
	PtWidget_t *w = _lookupWidget(id);
	if (w) {
		int flags = 0;
		flags = (value == UT_TRUE) ? Pt_SELECTABLE : (Pt_BLOCKED | Pt_GHOST);
		PtSetResource(w, Pt_ARG_FLAGS, flags, Pt_SELECTABLE | Pt_BLOCKED | Pt_GHOST);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabType AP_QNXDialog_Tab::_gatherAlignment()
{
#if 0
	// for ( UT_uint32 i = (UT_uint32)id_ALIGN_LEFT; 
	// 	  i <= (UT_uint32)id_ALIGN_BAR;
	// 	  i++ )
#endif
	return m_current_alignment;
}

void AP_QNXDialog_Tab::_setAlignment( eTabType a )
{
	// NOTE - tControl id_ALIGN_LEFT .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_ALIGN_LEFT + (UT_uint32)a);	
	UT_ASSERT( id >= id_ALIGN_LEFT && id <= id_ALIGN_BAR );

	// time to set the alignment radiobutton widget
	PtWidget_t *w = _lookupWidget( id );
	UT_ASSERT(w); 

	// tell the change routines to ignore this message
	if (w) {
		PtSetResource(w, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_QNXDialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_QNXDialog_Tab::_setLeader( eTabLeader a )
{
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_ASSERT( id >= id_LEADER_NONE && id <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	PtWidget_t *w = _lookupWidget( id );
	UT_ASSERT(w);

	// tell the change routines to ignore this message
	if (w) {
		PtSetResource(w, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const XML_Char * AP_QNXDialog_Tab::_gatherDefaultTabStop()
{
	//return gtk_entry_get_text( GTK_ENTRY( _lookupWidget( id_SPIN_DEFAULT_TAB_STOP ) ) );
	return NULL;
}

void AP_QNXDialog_Tab::_setDefaultTabStop( const XML_Char * a )
{
	UT_UNUSED(a);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_QNXDialog_Tab::_setTabList( UT_uint32 count )
{
	PtWidget_t *wList = _lookupWidget( id_LIST_TAB );
	UT_uint32 i;
	fl_TabStop *pTabInfo;

	UT_ASSERT(wList);

	// clear all the items from the list
	if (!wList) {
		return;
	}
	PtListDeleteAllItems(wList);

#if 0
	for ( i = 0; i < count; i++ )
	{
		GtkWidget *li = gtk_list_item_new_with_label( _getTabDimensionString(i));

		// we want to DO stuff
		gtk_signal_connect(GTK_OBJECT(li),
						   "select",
						   GTK_SIGNAL_FUNC(s_list_select),
						   (gpointer) this);

		// show this baby
		gtk_widget_show(li);

		gList = g_list_append( gList, li );
	}

	for ( i = 0; i < count; i++ )
	{
		pTabInfo = (fl_TabStop *)v.getNthItem(i);

		// this will do for the time being, but if we want 
		UT_DEBUGMSG(("%s:%d need to fix\n", __FILE__,__LINE__));

		const char *ptr;

		//Unix does this ... I think it is wrong!
		//ptr = UT_convertToDimensionlessString( pTabInfo->iPositionLayoutUnits,  pTabInfo->iPosition );
		ptr = UT_convertToDimensionlessString( pTabInfo->iPosition,  NULL);
		PtListAddItems(wList, &ptr, 1, 0);
	}
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_QNXDialog_Tab::_gatherSelectTab()
{
#if 0
	return m_iGtkListIndex;
#endif
	return 0;
}

void AP_QNXDialog_Tab::_setSelectTab( UT_sint32 v )
{
#if 0
	m_iGtkListIndex = v;

	if ( v == -1 )	// we don't want to select anything
	{
		gtk_list_unselect_all(GTK_LIST(_lookupWidget(id_LIST_TAB)));
	}
	else
	{
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_QNXDialog_Tab::_gatherTabEdit()
{
#if 0
	return gtk_entry_get_text( GTK_ENTRY( _lookupWidget( id_EDIT_TAB ) ) );
#endif
	return NULL;
}

void AP_QNXDialog_Tab::_setTabEdit( const char *pszStr )
{
	PtWidget_t *w = _lookupWidget( id_EDIT_TAB );

	UT_ASSERT(w);
	if (!w) {
		return;
	}

	PtSetResource(w, Pt_ARG_TEXT_STRING, pszStr, 0);
}

void AP_QNXDialog_Tab::_clearList()
{
	PtWidget_t *wList = _lookupWidget( id_LIST_TAB );

	UT_ASSERT(wList);
	if (!wList) {
		return;
	}

	PtListDeleteAllItems(wList);
}


