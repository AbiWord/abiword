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

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

static UT_uint32 xap_UnixDlg_Insert_Symbol_first = 0;
static UT_UCSChar m_CurrentSymbol;
static UT_UCSChar m_PreviousSymbol;

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


/*****************************************************************/

static gboolean s_ok_clicked(GtkWidget * widget, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return FALSE;
}

static gboolean s_cancel_clicked(GtkWidget * widget, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return FALSE;
}

static void s_sym_SymbolMap_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT( dlg);
	dlg->SymbolMap_exposed();
}

static void s_Symbolarea_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(dlg);
	dlg->Symbolarea_exposed();
}

static gboolean  s_SymbolMap_clicked(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->SymbolMap_clicked( e );
	return FALSE; 
}

static void s_new_font(GtkWidget * widget, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->New_Font();
}

static gboolean s_keypressed(GtkWidget * widget, GdkEventKey * e,  XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->Key_Pressed(e);
	return TRUE;
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}


/*****************************************************************/

GtkWidget * XAP_UnixGnomeDialog_Insert_Symbol::_constructWindow(void)
{
	GtkWidget * mwindow;
	GtkWidget * vbox;
	GtkWidget * vhbox;
	GtkWidget * fontcombo;
	GtkWidget * SymbolMap;
	GtkWidget * areaCurrentSym;
	GtkWidget * hboxInsertS;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;
	XML_Char * tmp = NULL;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(XAP_STRING_ID_DLG_Insert_SymbolTitle));
	mwindow = gnome_dialog_new (tmp, );
	FREEP(tmp);
//	gtk_widget_set_usize(windowInsertS, 610, 245);

	// Now put in a Vbox to hold our 3 widgets (Font Selector, Symbol Table
	// and OK -Selected Symbol- Cancel

	vbox = gtk_vbox_new (FALSE, 4);
	gtk_container_add(GTK_CONTAINER(mwindow), vbox);

	// Now Build the font combo box into the frame

	/* First though we have to grab the fonts!

	   This code is to suck all the available fonts and put them in a GList.
	   This can then be displayed on a combo box at the top of the dialog.
	   Code stolen from ap_UnixToolbar_FontCombo */

	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_uint32 count = unixapp->getFontManager()->getCount();
	XAP_UnixFont ** list = unixapp->getFontManager()->getAllFonts();

	/* Now we remove all the duplicate name entries and create the Glist
	   m_InsertS_Font_list. This will be used in the font selection combo
	   box */

	gchar currentfont[50] = "\0";
	UT_uint32 j = 0;
	m_InsertS_Font_list = NULL;

	for (UT_uint32 i = 0; i < count; i++)
	{
		gchar * lgn  = (gchar *) list[i]->getName();
		if(strstr(currentfont, lgn) == NULL)
		{
			strncpy(currentfont, lgn, 50);
			m_fontlist[j] = g_strdup(currentfont);
			m_InsertS_Font_list = g_list_prepend(m_InsertS_Font_list, m_fontlist[j++]);
		}
	}
	m_Insert_Symbol_no_fonts = j;
	DELETEP(list);
  
	m_InsertS_Font_list = g_list_reverse(m_InsertS_Font_list);

	vhbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vboxInsertS), vhbox, TRUE, TRUE, 0);

	// Finally construct the combo box
	fontcombo = gtk_combo_new();
 
	gtk_object_set_data (GTK_OBJECT(windowInsertS), "fontcombo", fontcombo);
	gtk_widget_set_usize(fontcombo, 200, 25);
	gtk_widget_show(fontcombo);
	gtk_combo_set_value_in_list(GTK_COMBO(fontcombo), TRUE, TRUE);
	gtk_combo_set_use_arrows(GTK_COMBO(fontcombo), FALSE);
	gtk_combo_set_popdown_strings(GTK_COMBO(fontcombo), m_InsertS_Font_list);

	// Put the current font in the entry box.
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(fontcombo)->entry),
					   (gchar *) DEFAULT_UNIX_SYMBOL_FONT);

	// Turn off keyboard entry in the font selection box
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(fontcombo)->entry), FALSE);

	// Now put the font combo box and the preview at the top of the dialog 
	gtk_box_pack_start(GTK_BOX(vhbox), fontcombo, TRUE, FALSE, 0);

	// Now the Symbol Map. 

	// *** Code Stolen from the preview widget ***
	{
		SymbolMap = gtk_drawing_area_new ();
		// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
		gtk_widget_set_usize (SymbolMap, 608, 147);
		gtk_box_pack_start (GTK_BOX (vbox), SymbolMap, FALSE, FALSE, 0);
		gtk_widget_add_events(SymbolMap, GDK_BUTTON_PRESS_MASK);
   	}
	

	gtk_object_set_data (GTK_OBJECT (windowInsertS), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_box_pack_start(GTK_BOX(hboxInsertS), buttonOK, TRUE, FALSE, 4);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	// *** Code Stolen from the preview widget again! ***
	{
		areaCurrentSym = gtk_drawing_area_new ();
		gtk_object_set_data (GTK_OBJECT (windowInsertS), 
							 "areaCurrentSym", areaCurrentSym);
		gtk_widget_show (areaCurrentSym);
		gtk_widget_set_usize (areaCurrentSym, 60,45);
		gtk_box_pack_start(GTK_BOX(vhbox), areaCurrentSym, TRUE, FALSE, 0);
   	}

	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(XAP_STRING_ID_DLG_Insert));
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (mwindow), tmp, GNOME_STOCK_PIXMAP_ADD);
	FREEP(tmp);

	gnome_dialog_append_button (GNOME_DIALOG (mwindow), GNOME_STOCK_BUTTON_CLOSE);
   	buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (mwindow)->buttons)->data);
   	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (mwindow)->buttons)->data);

	// I'M HERE!!!!!!!!!!!! XXXXXXXXXXX
	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowInsertS;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_SymbolMap = SymbolMap;
	m_fontcombo = fontcombo;
	m_areaCurrentSym = areaCurrentSym;

	// Now connect the signals
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	// The event to choose the Symbol!
	gtk_signal_connect(GTK_OBJECT(SymbolMap),
					   "button_press_event",
				       GTK_SIGNAL_FUNC(s_SymbolMap_clicked),
					   (gpointer) this);

	// Look for keys pressed
	gtk_signal_connect(GTK_OBJECT(windowInsertS),
					   "key_press_event",
					   GTK_SIGNAL_FUNC(s_keypressed),
					   (gpointer) this);


	// Look for "changed" signal on the entry part of the combo box.
	// Code stolen from ev_UnixGnomeToolbar.cpp

	GtkEntry * blah = GTK_ENTRY(GTK_COMBO(fontcombo)->entry);
	GtkEditable * yuck = GTK_EDITABLE(blah);
	gtk_signal_connect(GTK_OBJECT(&yuck->widget),
					   "changed",
					   GTK_SIGNAL_FUNC(s_new_font),
					   (gpointer) this);

	// the catch-alls
	gtk_signal_connect_after(GTK_OBJECT(windowInsertS),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowInsertS),
							 "destroy",NULL, NULL);
	
	// the expose event of the SymbolMap
	gtk_signal_connect(GTK_OBJECT(SymbolMap),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_sym_SymbolMap_exposed),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(areaCurrentSym),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_Symbolarea_exposed),
					   (gpointer) this);

       	gtk_widget_grab_focus (buttonOK);
        gtk_widget_grab_default (buttonOK);
// We have to wait a little...
//	gtk_widget_show(windowInsertS);
	
	return windowInsertS;
}


