/* AbiWord
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

#include <stdlib.h>

#include <MacWindows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_MacDialog_Columns.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_MacDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_MacDialog_Columns * p = new AP_MacDialog_Columns(pFactory,id);
	return p;
}

AP_MacDialog_Columns::AP_MacDialog_Columns(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_radioGroup = NULL;
}

AP_MacDialog_Columns::~AP_MacDialog_Columns(void)
{
}

/*****************************************************************/

void AP_MacDialog_Columns::runModal(XAP_Frame * pFrame)
{
#if 0
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_MacFrame * pMacFrame = static_cast<XAP_MacFrame *>(pFrame);
	UT_ASSERT(pMacFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pMacFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	_storeWindowData();
	
	gtk_widget_destroy(mainWindow);
#endif
}

void AP_MacDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
//	gtk_main_quit();
}

void AP_MacDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
//	gtk_main_quit();
}

void AP_MacDialog_Columns::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;	
//	gtk_main_quit();
}

/*****************************************************************/

WindowPtr AP_MacDialog_Columns::_constructWindow(void)
{
    WindowPtr windowColumns = NULL;

    return windowColumns;
}

void AP_MacDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

//	GtkWidget * widget = _findRadioByID(m_break);
//	UT_ASSERT(widget);
	
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

void AP_MacDialog_Columns::_storeWindowData(void)
{
//	m_break = _getActiveRadioItem();
}

void AP_MacDialog_Columns::enableLineBetweenControl(UT_Bool bState)
{
}
