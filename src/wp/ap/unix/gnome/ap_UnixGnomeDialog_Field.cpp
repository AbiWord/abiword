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
#include <time.h>
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
#include "ap_Dialog_Field.h"
#include "ap_UnixGnomeDialog_Field.h"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Field * p = new AP_UnixGnomeDialog_Field (pFactory, id);
	return p;
}

AP_UnixGnomeDialog_Field::AP_UnixGnomeDialog_Field(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
  : AP_UnixDialog_Field (pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Field::~AP_UnixGnomeDialog_Field(void)
{
}

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_Field::_constructWindow(void)
{
	GtkWidget *contents;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// Start with the main window
	m_windowMain = gnome_dialog_new (pSS->getValue (AP_STRING_ID_DLG_Field_FieldTitle),
									 GNOME_STOCK_BUTTON_OK,
									 GNOME_STOCK_BUTTON_CANCEL, NULL);
	gnome_dialog_set_default (GNOME_DIALOG (m_windowMain), 0);

	// Now the contents of the dialog box
	contents = _constructWindowContents ();
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (m_windowMain)->vbox), contents, FALSE, TRUE, 0);

	// Now the two buttons
   	m_buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (m_windowMain)->buttons)->data);
   	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);

	// connect all the signals
	_connectSignals ();

	// and action!
	gtk_widget_show_all (m_windowMain);

	return m_windowMain;
}
