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
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_QNXDialog_Lists.h"

#include "ut_qnxHelper.h"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Lists::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Lists * p = new AP_QNXDialog_Lists(pFactory,id);
	return p;
}

AP_QNXDialog_Lists::AP_QNXDialog_Lists(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Lists(pDlgFactory,id)
{
}

AP_QNXDialog_Lists::~AP_QNXDialog_Lists(void)
{
}

void AP_QNXDialog_Lists::activate()
{
// Standard Modeless Activate. Update dialog and raise it
}

void AP_QNXDialog_Lists::destroy()
{
  // Standard Modeless destroy.
}

void AP_QNXDialog_Lists::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	UT_ASSERT(UT_NOT_IMPLEMENTED);

/*
//
//---------------------------------------------------------------------
//
// Hints from the unix code. It is a Modeless dialog with two extra wrinkles.
// Firstly it autoupdates every 0.5 seconds to reflect the List status of
// the current View. Second upon a focus event it also does an immediate 
// Auto update. This means it is impossible for the dialog to be out of sync
// with the current view. To do the update on focus it was neccessary to create
// a new function "connectFocusModelessOther()" to run a second static function
// after doinf the usual Modeless chores upon a focus event. See 
// ut_unixdialoghelper.cpp for details.
//
// As you would expect
	_constructWindow ();
	UT_ASSERT (m_wMainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
 
	// This magic command displays the frame that characters will be
	// inserted into.
        // This variation runs the additional static function shown afterwards.
        // Only use this if you need to to update the dialog upon focussing.

//
// !!!!! IMPORTANT NEW FUNCTION NEEDED FOR OTHER PLATFORMS !!!!!!!!!!!!
//
	connectFocusModelessOther (GTK_WIDGET (m_wMainWindow), m_pApp, (gboolean (*)(void)) s_update);

	// Populate the dialog
	updateDialog();

	// Now Display the dialog
	gtk_widget_show_all (m_wMainWindow);

	// Next construct a timer for auto-updating the dialog
	GR_Graphics * pG = NULL;
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this,pG);
	m_bDestroy_says_stopupdating = UT_FALSE;

	// OK fire up the auto-updater for 0.5 secs

	m_pAutoUpdateLists->set(500);
	
*/
	//------------------------------------------------------------
	// End of QNX code hints for runModeless
    //------------------------------------------------------------
}

/*
//
// ------------------------------------------------------------------------
//
// !!!!!!!! MORE HINTS !!!!!!!!!!!!!!!!!!!

OK Here are some more hints from the QNX build for lists. 
First here are the static
callbacks used to connect events to useful code.
*/


static int s_startChanged(PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->startChanged ();
	return Pt_CONTINUE;
}

static int s_stopChanged (PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->stopChanged ();
	return Pt_CONTINUE;
}

static int s_startvChanged (PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->startvChanged ();
	return Pt_CONTINUE;
}

static int s_applyClicked (PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->applyClicked();
	return Pt_CONTINUE;
}

static int s_closeClicked (PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->destroy();
	return Pt_CONTINUE;
}

static int s_deleteClicked (PtWidget_t * widget, void * data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists *dlg = (AP_QNXDialog_Lists *)data; 
	dlg->destroy();
	return Pt_CONTINUE;
}

static int s_update (void)
{
/*
	Current_Dialog->updateDialog();
*/
	return UT_TRUE;
}

// Next comes brief descriptions of the memmber functions used by the unix 
// Code


         
void    AP_QNXDialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
  // The autoupdate code. Borrows heavily from WordCount
}


void  AP_QNXDialog_Lists::applyClicked(void)
{
  // Gather all the info from the dialog and run the xp code in "Apply" to
  // implement it all
}


void  AP_QNXDialog_Lists::startChanged(void)
{
  // Code that implements all the stuff needed once the start toggle button is
  // clicked
}


void  AP_QNXDialog_Lists::stopChanged(void)
{
  // Code that implements all the stuff needed once the stop toggle button is
  // clicked
}


void  AP_QNXDialog_Lists::startvChanged(void)
{
  // Code that implements all the stuff needed once the change list toggle 
  // button is clicked
}


void AP_QNXDialog_Lists::updateDialog(void)
{
  // Update the dialog
  //
	_populateWindowData();
	setAllSensitivity();
}


void AP_QNXDialog_Lists::setAllSensitivity(void)
{ 
  // Code to allow the user to change only those parameters that make
  // sense in the current list context
}


PtWidget_t * AP_QNXDialog_Lists::_constructWindow (void)
{
  // Code to construct the dialog window
  //
}


PtWidget_t *AP_QNXDialog_Lists::_constructWindowContents (void)
{
  // Code to put in the majority of the labels and buttons. Everything but
  // apply and close
}


void AP_QNXDialog_Lists::_populateWindowData (void) 
{
  // Code to fill in the labels from the current list context in the current 
  // view
}


void AP_QNXDialog_Lists::_connectSignals(void)
{
  // Connect the signals from the GUI elements to the code to implement them
}

//
// ------------------------------------------------------------------------
//
// !!!!!!!! END OF HINTS !!!!!!!!!!!!!!!!!!!







