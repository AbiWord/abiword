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
	m_prevDefaultTabStop = 0;
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
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowTabs = PtCreateWidget(PtWindow, NULL, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtWidget_t *vgroup = PtCreateWidget(PtGroup, windowTabs, n, args);

	PtWidget_t *hcontrolgroup, *htmpgroup, *vtmpgroup;

	n = 0;
	hcontrolgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	/* Lists ... */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vtmpgroup = PtCreateWidget(PtGroup, hcontrolgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Label_TabPosition)), 0);
	PtCreateWidget(PtLabel, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *entryTabEntry = PtCreateWidget(PtText, vtmpgroup, n, args);
	PtAddCallback(entryTabEntry, Pt_CB_ACTIVATE, s_set_clicked, this);
	PtAddCallback(entryTabEntry, Pt_CB_TEXT_CHANGED, s_edit_change, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_VISIBLE_COUNT, 5, 0);
	PtWidget_t *listTabs = PtCreateWidget(PtList, vtmpgroup, n, args);
	PtAddCallback(listTabs, Pt_CB_SELECTION, s_list_select, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Label_TabToClear)), 0);
	PtCreateWidget(PtLabel, vtmpgroup, n, args);

	/* Choices ... */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vtmpgroup = PtCreateWidget(PtGroup, hcontrolgroup, n, args);

	n = 0;
	htmpgroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Label_DefaultTS)), 0);
	PtCreateWidget(PtLabel, htmpgroup, n, args);

	n = 0;
	double d = 0.1;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &d, sizeof(d));
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_PRECISION, 1, 0);
	PtWidget_t *spinbuttonTabstop = PtCreateWidget(PtNumericFloat, htmpgroup, n, args);
	PtAddCallback(spinbuttonTabstop, Pt_CB_ACTIVATE, s_spin_default_changed, this);
	PtAddCallback(spinbuttonTabstop, Pt_CB_NUMERIC_CHANGED, s_spin_default_changed, this);

	PtWidget_t *radgroup;

	PtWidget_t *agroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE, Pt_SHOW_TITLE);
	PtSetArg(&args[n++], Pt_ARG_TITLE, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Label_Alignment)), 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	agroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	radgroup = PtCreateWidget(PtGroup, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Left)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonLeft = PtCreateWidget(PtToggleButton, radgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Decimal)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonDecimal = PtCreateWidget(PtToggleButton, radgroup, n, args);

	n = 0;
	radgroup = PtCreateWidget(PtGroup, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Center)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonCenter = PtCreateWidget(PtToggleButton, radgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Bar)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonBar = PtCreateWidget(PtToggleButton, radgroup, n, args);

	n = 0;
	radgroup = PtCreateWidget(PtGroup, agroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Right)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonRight = PtCreateWidget(PtToggleButton, radgroup, n, args);

	PtWidget_t *lgroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE, Pt_SHOW_TITLE);
	PtSetArg(&args[n++], Pt_ARG_TITLE, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Label_Leader)), 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	lgroup = PtCreateWidget(PtGroup, vtmpgroup, n, args);

	n = 0;
	radgroup = PtCreateWidget(PtGroup, lgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,  UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_None)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonLeaderNone = PtCreateWidget(PtToggleButton, radgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Dash)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonLeaderDash = PtCreateWidget(PtToggleButton, radgroup, n, args);

	n = 0;
	radgroup = PtCreateWidget(PtGroup, lgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Dot)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonLeaderDot = PtCreateWidget(PtToggleButton, radgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Underline)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *radiobuttonLeaderUnderline = PtCreateWidget(PtToggleButton, radgroup, n, args);

	/* Setting buttons */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, Pt_TOP_BEVEL | Pt_TOP_OUTLINE, Pt_ALL);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	htmpgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtLabel, htmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Button_Set)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonSet = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonSet, Pt_CB_ACTIVATE, s_set_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Button_Clear)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonClear = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonClear, Pt_CB_ACTIVATE, s_clear_clicked, this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue( AP_STRING_ID_DLG_Tab_Button_ClearAll)), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonClearAll = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonClearAll, Pt_CB_ACTIVATE, s_clear_all_clicked, this);

	/* Other buttons */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, Pt_TOP_BEVEL | Pt_TOP_OUTLINE, Pt_ALL);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	htmpgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtLabel, htmpgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue( AP_STRING_ID_DLG_Options_Btn_Apply), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonApply = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonApply, Pt_CB_ACTIVATE, s_apply_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue( XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonOK = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue( XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonCancel = PtCreateWidget(PtButton, htmpgroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

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

	UT_uint32 id, userdata;

	// create user data tControl -> stored in widgets 
	for ( id = 0; id < id_last; id++ ) 
	{
		PtWidget_t *w = _lookupWidget( (tControl) id );

		UT_ASSERT( w );

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		PtSetResource(w, Pt_ARG_USER_DATA, &id, sizeof(id));
	}

	for ( id = id_ALIGN_LEFT; id <= id_ALIGN_BAR; id++)
	{
		PtWidget_t *w = _lookupWidget((tControl)id);

		//We want the matching eTabType from fl_BlockLayout.h
		userdata = id - id_ALIGN_LEFT + 1;
		PtSetResource(w, Pt_ARG_USER_DATA, &userdata, sizeof(userdata));
		PtAddCallback(w, Pt_CB_ACTIVATE, s_alignment_change, this);
	}

	for ( id = id_LEADER_NONE; id <= id_LEADER_UNDERLINE; id++)
	{
		PtWidget_t *w = _lookupWidget((tControl)id);

		//We want the matching eTabLeader from fl_BlockLayout.h
		userdata = id - id_LEADER_NONE;
		PtSetResource(w, Pt_ARG_USER_DATA, &userdata, sizeof(userdata));
		PtAddCallback(w, Pt_CB_ACTIVATE, s_leader_change, this);
	}
	
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
	UT_ASSERT(w);
	if (w) {
		int flags = 0;
		flags = (value == UT_TRUE) ? Pt_SELECTABLE : (Pt_BLOCKED | Pt_GHOST);
		PtSetResource(w, Pt_ARG_FLAGS, flags, Pt_SELECTABLE | Pt_BLOCKED | Pt_GHOST);
	}
}

/* TODO
 direction == -1 is down
 direction == 1 is up
 direction == 0 is totally new value
*/
void AP_QNXDialog_Tab::_spinChanged(void)
{
	double *d;
	PtWidget_t *w = _lookupWidget(id_SPIN_DEFAULT_TAB_STOP);

	d = NULL;
	PtGetResource(w, Pt_ARG_NUMERIC_VALUE, &d, 0);

	if (!d) {
		return;
	}

#if 0
	UT_sint32 amt;
	amt = (UT_sint32)(((*d + 0.04) - m_prevDefaultTabStop) / 0.1);

	if (amt == 0) {	//Prevents the continual looping
		return;
	}

	//The increment value is actually internal to this
	//function, so just tell it how many times to spin
	//(and up or down).
	_doSpin(id_SPIN_DEFAULT_TAB_STOP, amt);
#else
	_doSpinValue(id_SPIN_DEFAULT_TAB_STOP, *d);
#endif
}


/*****************************************************************/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Callbacks events
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

/*static*/ int AP_QNXDialog_Tab::s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(dlg); 
	dlg->event_OK(); 
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Cancel(); 
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_apply_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Apply(); 
	return Pt_CONTINUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// WP level events
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
/*static*/ int AP_QNXDialog_Tab::s_spin_default_changed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;

	//Only handle the spinner changes or when the user hits enter
	if (info->reason == Pt_CB_NUMERIC_CHANGED &&
	    info->reason_subtype == Pt_NUMERIC_CHANGED) {
		return Pt_CONTINUE;
	}

	dlg->_spinChanged();
	return Pt_CONTINUE;
}
 
/*static*/ int AP_QNXDialog_Tab::s_set_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_Set();	
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_clear_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_Clear(); 
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_clear_all_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_ClearAll(); 
	return Pt_CONTINUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Listbox stuff

// TODO - This should be moved to XAP code, but the methods in which GTK and
// windows handles selection/deselection differ so much, it's easier just to
// code up the hooks directly.

/*static*/ int AP_QNXDialog_Tab::s_list_select(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	PtListCallback_t * lcb = (PtListCallback_t *)info->cbdata;
	UT_ASSERT(dlg && lcb && widget); 

	//Clear all of our tags first
	int index;
	for (index=id_ALIGN_LEFT; index <= id_ALIGN_BAR; index++) {
		PtSetResource(dlg->_lookupWidget((tControl)index), Pt_ARG_FLAGS, 0, Pt_SET);
	}
	for (index=id_LEADER_NONE; index <= id_LEADER_UNDERLINE; index++) {
		PtSetResource(dlg->_lookupWidget((tControl)index), Pt_ARG_FLAGS, 0, Pt_SET);
	}

	// get the -1, 0.. (n-1) index
	dlg->_event_TabSelected(lcb->item_pos - 1);
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_list_deselect(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	UT_DEBUGMSG(("AP_QNXDialog_Tab::s_list_deselect"));
	return Pt_CONTINUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// edit box stuff

/*static*/ int AP_QNXDialog_Tab::s_edit_change(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	UT_DEBUGMSG(("AP_QNXDialog_Tab::s_edit_change"));
	dlg->_event_TabChange();
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_alignment_change(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	eTabType *id;
	int		  index, *flags;
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 

	// we're only interested in "i'm not toggled"
/*
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;
*/
	PtGetResource(widget, Pt_ARG_FLAGS, &flags, 0);
	if (!(*flags & Pt_SET)) {
		return Pt_CONTINUE;
	}

	id = NULL;
	PtGetResource(widget, Pt_ARG_USER_DATA, &id, 0);
	dlg->m_current_alignment = *id;

	for (index=id_ALIGN_LEFT; index <= id_ALIGN_BAR; index++) {
		if ((*id) + id_ALIGN_LEFT - 1 == index) {
			continue;
		}
		PtSetResource(dlg->_lookupWidget((tControl)index), Pt_ARG_FLAGS, 0, Pt_SET);
	}

	UT_DEBUGMSG(("AP_QNXDialog_Tab::s_alignment_change [%c]", AlignmentToChar(dlg->m_current_alignment)));

	dlg->_event_AlignmentChange();
	return Pt_CONTINUE;
}

/*static*/ int AP_QNXDialog_Tab::s_leader_change(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	eTabLeader *id;
	int 		index;
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 

/*
	// we're only interested in "i'm not toggled"
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;
*/	

	id = NULL;
	PtGetResource(widget, Pt_ARG_USER_DATA, &id, 0);
	dlg->m_current_leader = *id;
	
	for (index=id_LEADER_NONE; index <= id_LEADER_UNDERLINE; index++) {
		if (*id + id_LEADER_NONE == index) {
			continue;
		}
		PtSetResource(dlg->_lookupWidget((tControl)index), Pt_ARG_FLAGS, 0, Pt_SET);
	}

	UT_DEBUGMSG(("AP_QNXDialog_Tab::s_leader_change"));

	dlg->_event_somethingChanged();
	return Pt_CONTINUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// when a window is closed
/*static*/ int AP_QNXDialog_Tab::s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{ 
	AP_QNXDialog_Tab * dlg = (AP_QNXDialog_Tab *)data;
	UT_ASSERT(dlg); 
	UT_DEBUGMSG(("AP_QNXDialog_Tab::s_delete_clicked"));
	dlg->event_WindowDelete(); 
	return Pt_CONTINUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabType AP_QNXDialog_Tab::_gatherAlignment()
{
	return m_current_alignment;
}

void AP_QNXDialog_Tab::_setAlignment( eTabType a )
{
	tControl id = id_ALIGN_LEFT;
	
	
	switch(a)
		{
		case FL_TAB_LEFT:
			id = id_ALIGN_LEFT;
			break;

		case FL_TAB_CENTER:
			id = id_ALIGN_CENTER;
			break;

		case FL_TAB_RIGHT:
			id = id_ALIGN_RIGHT;
			break;

		case FL_TAB_DECIMAL:
			id = id_ALIGN_DECIMAL;
			break;

		case FL_TAB_BAR:
			id = id_ALIGN_BAR;
			break;

		}

	// time to set the alignment radiobutton widget
	PtWidget_t *w = _lookupWidget( id );
	UT_ASSERT(w);

	// tell the change routines to ignore this message
	//m_bInSetCall = UT_TRUE;
	PtSetResource(w, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	//m_bInSetCall = UT_FALSE;

	m_current_alignment = a;
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
	//m_bInSetCall = UT_TRUE;
	PtSetResource(w, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	//m_bInSetCall = UT_FALSE;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const XML_Char * AP_QNXDialog_Tab::_gatherDefaultTabStop()
{
	char *text = NULL;
	PtGetResource(_lookupWidget(id_SPIN_DEFAULT_TAB_STOP), Pt_ARG_TEXT_STRING, &text, 0);
	return text;
}

void AP_QNXDialog_Tab::_setDefaultTabStop( const XML_Char * a )
{
	const XML_Char *suffix;
	double d;
	int len;

	PtWidget_t *w = _lookupWidget(id_SPIN_DEFAULT_TAB_STOP);
	/* There are two parts here ... the number and the suffix,
       we should extract both components */
	sscanf(a, "%lf", &d);
	m_prevDefaultTabStop = d;
	suffix = &a[strlen(a)-1];
	while ((*suffix < '0' || *suffix > '9') && suffix > a) { suffix--; } 

	if (suffix <= a) {
		suffix = NULL;
	} else {
		suffix++;
	}
		
	PtSetResource(w, Pt_ARG_NUMERIC_VALUE, &d, sizeof(d));
	if (suffix) {
		PtSetResource(w, Pt_ARG_NUMERIC_SUFFIX, suffix, 0);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_QNXDialog_Tab::_setTabList( UT_uint32 count )
{
	PtWidget_t *wList = _lookupWidget( id_LIST_TAB );
	UT_uint32 i;

	UT_ASSERT(wList);

	// clear all the items from the list
	if (!wList) {
		return;
	}
	PtListDeleteAllItems(wList);

	for ( i = 0; i < count; i++ )
	{
		char *text = _getTabDimensionString(i);

		if (!text) {
			UT_DEBUGMSG(("Skipping empty "));
			continue;
		}
		UT_DEBUGMSG(("Adding %s", text));

		PtListAddItems(wList, (const char **)&text, 1, 0);
	}
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
	char *text = NULL;

	PtGetResource(_lookupWidget(id_EDIT_TAB), Pt_ARG_TEXT_STRING, &text, 0);
	return (const char *)text;
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


