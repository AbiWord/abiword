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
#include <gnome.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixGnomeDialog_WordCount.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_WordCount * p = new AP_UnixGnomeDialog_WordCount(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_WordCount::AP_UnixGnomeDialog_WordCount(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: AP_UnixDialog_WordCount(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_buttonOK = NULL;
	m_wContent = NULL;
}

AP_UnixGnomeDialog_WordCount::~AP_UnixGnomeDialog_WordCount(void)
{
}

/*****************************************************************/

void AP_UnixGnomeDialog_WordCount::runModal(XAP_Frame * pFrame)
{
	gint button;

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
    centerDialog(parentWindow, mainWindow);
	gnome_dialog_set_parent (GNOME_DIALOG (mainWindow), GTK_WINDOW (parentWindow));
	button = gnome_dialog_run_and_close (GNOME_DIALOG (mainWindow));

	switch (button) {
	case 0:
		m_answer = AP_Dialog_WordCount::a_OK;
		break;
	case -1:
		m_answer = AP_Dialog_WordCount::a_CANCEL;
		break;
	default:
		UT_ASSERT (0);
	}
}

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_WordCount::_constructWindow(void)
{
	GtkWidget * windowWordCount;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr;	// used for conversions

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle));
	windowWordCount = gnome_dialog_new (unixstr, GNOME_STOCK_BUTTON_OK, NULL);

	_constructWindowContents();
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (windowWordCount)->vbox), m_wContent);
	m_windowMain = windowWordCount;
   	m_buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (windowWordCount)->buttons)->data);

	return windowWordCount;
}
