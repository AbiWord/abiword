/* AbiSource Application Framework
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
#include <string.h>
#include <gnome.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_hash.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_UnixDlg_Insert_Symbol.h"
#include "xap_UnixGnomeDlg_Insert_Symbol.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * XAP_UnixGnomeDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
																   XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_Insert_Symbol * p = new XAP_UnixGnomeDialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_Insert_Symbol::XAP_UnixGnomeDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
																	 XAP_Dialog_Id id)
	: XAP_UnixDialog_Insert_Symbol(pDlgFactory, id)
{
}

XAP_UnixGnomeDialog_Insert_Symbol::~XAP_UnixGnomeDialog_Insert_Symbol(void)
{
}

static void
cb_close (GtkWidget * w, XAP_UnixGnomeDialog_Insert_Symbol * dlg)
{
  UT_ASSERT (dlg);
  dlg->event_Cancel();
}

/*****************************************************************/

GtkWidget * XAP_UnixGnomeDialog_Insert_Symbol::_constructWindow(void)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;

        ConstructWindowName();
	m_windowMain = gnome_dialog_new (m_WindowName, NULL);

	vbox = GTK_WIDGET (GNOME_DIALOG (m_windowMain)->vbox);

	hbox = gtk_hbox_new (FALSE, 1);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	
// Finally construct the combo box
	m_fontcombo = _createComboboxWithFonts ();
	gtk_object_set_data (GTK_OBJECT (m_windowMain), "fontcombo", m_fontcombo);
	gtk_box_pack_start (GTK_BOX (hbox), m_fontcombo, TRUE, FALSE, 0);

	// And the area for the current symbol
	m_areaCurrentSym = _previewNew (60, 45);
	gtk_object_set_data (GTK_OBJECT (m_windowMain), 
						 "areaCurrentSym", m_areaCurrentSym);
	gtk_box_pack_start(GTK_BOX(hbox), m_areaCurrentSym, TRUE, FALSE, 0);

	// Now the Symbol Map. 
	// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
	m_SymbolMap = _previewNew (608, 147);
	gtk_object_set_data (GTK_OBJECT (m_windowMain), "SymbolMap", m_SymbolMap);
	gtk_box_pack_start(GTK_BOX(vbox), m_SymbolMap, FALSE, FALSE, 0);
	
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(XAP_STRING_ID_DLG_Insert));
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (m_windowMain), tmp,
											GNOME_STOCK_PIXMAP_ADD);
   	m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);
	FREEP(tmp);

	gnome_dialog_append_button (GNOME_DIALOG (m_windowMain), GNOME_STOCK_BUTTON_CLOSE);
   	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);

	gtk_object_set_data (GTK_OBJECT (m_windowMain), "buttonOK", m_buttonOK);
	gtk_object_set_data (GTK_OBJECT (m_windowMain), "buttonCancel", m_buttonCancel);

	gtk_signal_connect (GTK_OBJECT(m_windowMain), "close",
			    GTK_SIGNAL_FUNC(cb_close), (gpointer)this);

	_connectSignals ();

	setDefaultButton (GNOME_DIALOG(m_windowMain), 0); // insert button
	
	return m_windowMain;
}

