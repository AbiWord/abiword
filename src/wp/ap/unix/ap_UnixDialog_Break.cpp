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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_UnixDialogBuilder.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_UnixDialog_Break.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_Break * p = new AP_UnixDialog_Break(pFactory,id);
	return p;
}

AP_UnixDialog_Break::AP_UnixDialog_Break(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id),
	  m_wMainWindow(NULL), m_pXML (NULL)
{
}

AP_UnixDialog_Break::~AP_UnixDialog_Break(void)
{
  if (m_pXML)
    gtk_object_unref (GTK_OBJECT (m_pXML));
}

/*****************************************************************/

static void s_ok_clicked(GtkButton * widget, gpointer data)
{
	UT_ASSERT(widget);
	AP_UnixDialog_Break *me = static_cast<AP_UnixDialog_Break*>(data);
	UT_ASSERT(me);
	me->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, gpointer data)
{
	UT_ASSERT(widget);
	AP_UnixDialog_Break *me = static_cast<AP_UnixDialog_Break*>(data);
	UT_ASSERT(me);
	me->event_Cancel();
}

static void s_delete_clicked(GtkWidget * widget,
			     GdkEvent /* event */,
			     gpointer data)
{
	UT_ASSERT(widget);
	AP_UnixDialog_Break *me = static_cast<AP_UnixDialog_Break*>(data);
	UT_ASSERT(me);
	me->event_Cancel();
}

/*****************************************************************/

void AP_UnixDialog_Break::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	_constructWindow();
	UT_ASSERT(m_wMainWindow);

	connectFocus(GTK_WIDGET(m_wMainWindow),pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(parentWindow, m_wMainWindow);

	// Show the top level dialog,
	gtk_widget_show(m_wMainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(m_wMainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	_storeWindowData();
	
	gtk_widget_destroy(m_wMainWindow);
}

void AP_UnixDialog_Break::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Break::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Break::event_Cancel(void)
{
	m_answer = AP_Dialog_Break::a_CANCEL;
	gtk_main_quit();
}

/*****************************************************************/

void AP_UnixDialog_Break::_init(void)
{
	GtkWidget * w = NULL;

	w = glade_xml_get_widget (m_pXML, "ok_button");
	gtk_signal_connect (GTK_OBJECT (w), "clicked",
			    GTK_SIGNAL_FUNC(s_ok_clicked),
			    static_cast<void *>(this));

	w = glade_xml_get_widget (m_pXML, "cancel_button");
	gtk_signal_connect (GTK_OBJECT (w), "clicked",
			    GTK_SIGNAL_FUNC(s_cancel_clicked),
			    static_cast<void *>(this));

	gtk_signal_connect_after (GTK_OBJECT (m_wMainWindow), "delete_event",
				  GTK_SIGNAL_FUNC(s_delete_clicked),
				  static_cast<void *>(this));

	gtk_signal_connect_after (GTK_OBJECT(m_wMainWindow), "destroy",
				  NULL, NULL);

	gtk_widget_show(m_wMainWindow);
}

GtkWidget * AP_UnixDialog_Break::_constructWindow (void)
{
	XAP_UnixDialogBuilder *builder = XAP_UnixDialogBuilder::instance();
	m_pXML = builder->build(_getGladeName(), m_pApp);
	m_wMainWindow = glade_xml_get_widget(m_pXML, "main_window");
	_init();

	return m_wMainWindow;
}

void AP_UnixDialog_Break::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
	// JCA: glade will take care of the details
}

void AP_UnixDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}

AP_Dialog_Break::breakType AP_UnixDialog_Break::_getActiveRadioItem(void)
{
	breakType retval = b_PAGE;

	if (_isActive("page_break_rb"))
		return b_PAGE;
	else if (_isActive("next_page_rb"))
		return b_NEXTPAGE;
	else if (_isActive("continuous_rb"))
		return b_CONTINUOUS;
	else if (_isActive("column_break_rb"))
		return b_COLUMN;
	else if (_isActive("even_page_rb"))
		return b_EVENPAGE;
	else if (_isActive("odd_page_rb"))
		return b_ODDPAGE;
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return retval;
}


