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

#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_QNXDlg_Insert_Symbol.h"
#include "ut_qnxHelper.h"


/*****************************************************************/


XAP_Dialog * XAP_QNXDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id id)
{
	XAP_QNXDialog_Insert_Symbol * p = new XAP_QNXDialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_QNXDialog_Insert_Symbol::XAP_QNXDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_unixGraphics = NULL;
	m_unixarea = NULL;
	
	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_SymbolMap = 	NULL;

	m_areaCurrentSym = NULL;
}

XAP_QNXDialog_Insert_Symbol::~XAP_QNXDialog_Insert_Symbol(void)
{
	DELETEP(m_unixGraphics);
	DELETEP(m_unixarea);
}


/*****************************************************************/

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_sym_SymbolMap_exposed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT( dlg);
	dlg->SymbolMap_exposed();
	return Pt_CONTINUE;
}

static int s_Symbolarea_exposed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT( dlg);
	dlg->Symbolarea_exposed();
	return Pt_CONTINUE;
}

static int s_SymbolMap_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->SymbolMap_clicked( NULL );
	return Pt_CONTINUE;
}

static int s_new_font(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->New_Font();
	return Pt_CONTINUE;
}

static int s_keypressed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	dlg->Key_Pressed( NULL );

	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{

	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();

	return Pt_CONTINUE;
}

/*****************************************************************/

void XAP_QNXDialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
#if 0
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);

	// *** this is how we add the gc for symbol table ***
#if 0
	// attach a new graphics context to the drawing area
	XAP_QNXApp * app = static_cast<XAP_QNXApp *> (m_pApp);
	UT_ASSERT(app);

	UT_ASSERT(m_SymbolMap && m_SymbolMap->window);

	// make a new QNX GC
	DELETEP (m_unixGraphics);
	m_unixGraphics = new GR_QNXGraphics(m_SymbolMap->window, unixapp->getFontManager());

	// let the widget materialize
	_createSymbolFromGC(m_unixGraphics,
						(UT_uint32) m_SymbolMap->allocation.width,
						(UT_uint32) m_SymbolMap->allocation.height);

	// *** Re use the code to draw into the selected symbol area.
	UT_ASSERT(m_areaCurrentSym && m_areaCurrentSym->window);

	// make a new QNX GC
	DELETEP (m_unixarea);
	m_unixarea = new GR_QNXGraphics(m_areaCurrentSym->window, unixapp->getFontManager());
		
	// let the widget materialize
	_createSymbolareaFromGC(m_unixarea,
							(UT_uint32) m_areaCurrentSym->allocation.width,
							(UT_uint32) m_areaCurrentSym->allocation.height);
#endif

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	// We use this code to insert the default font name into to static
        // variable "m_Insert_Symbol_font" the first time this dialog is
        // called. Afterwards it is just whatever was left from the last
        // call.

	if ( xap_QNXDlg_Insert_Symbol_first == 0) //BOGUS: the gcs need a font.
	{
		iDrawSymbol->setSelectedFont( (UT_UCSChar *) DEFAULT_QNX_SYMBOL_FONT);
		m_CurrentSymbol = ' ';
		m_PreviousSymbol = ' ';
		xap_QNXDlg_Insert_Symbol_first = 1;
	}

	// Show the top level dialog

	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

        // Put the current font in the entry box
	UT_UCSChar* iSelectedFont = iDrawSymbol->getSelectedFont();
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry),
					   (gchar *)   iSelectedFont);

	// Show the Previously selected symbol

	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// Run into the GTK event loop for this window.
#endif

	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;
}

void XAP_QNXDialog_Insert_Symbol::event_OK(void)
{
#if 0
	if (m_Insert_Symbol_no_fonts > 0 )
	{ 
		m_answer = XAP_Dialog_Insert_Symbol::a_OK;
		m_Inserted_Symbol = m_CurrentSymbol;
		g_list_free( m_InsertS_Font_list);

		for (UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++) 
		{
			if (m_fontlist[i] != NULL)
				g_free(m_fontlist[i]);
		}

		m_Insert_Symbol_no_fonts = 0;
		gtk_main_quit();
	}
#endif
}

void XAP_QNXDialog_Insert_Symbol::event_Cancel(void)
{
#if 0
	if(m_Insert_Symbol_no_fonts > 0 )
	{ 
		m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;
		g_list_free( m_InsertS_Font_list);

		for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++) 
		{
			if(m_fontlist[i] != NULL)
				g_free (m_fontlist[i]);
		}

		m_Insert_Symbol_no_fonts = 0;
		gtk_main_quit();
	}
#endif
}

void XAP_QNXDialog_Insert_Symbol::SymbolMap_exposed(void )
{
#if 0
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->draw();
//	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
#endif
}

void XAP_QNXDialog_Insert_Symbol::Symbolarea_exposed(void )
{
#if 0
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
#endif
}

//
// This function allows the symbol to be selected via the keyboard
//

void XAP_QNXDialog_Insert_Symbol::Key_Pressed(void * e)
{
#if 0
	int move = 0;

	switch (e->keyval)
	{
	case GDK_Up:
		move = -32;
		break;
	case GDK_Down:
		move = 32;
		break;
	case GDK_Left:
		move = -1;
		break;
	case GDK_Right:
		move = 1;
		break;
	}

	if (move != 0)
	{
		if ((m_CurrentSymbol + move) >= 32 && (m_CurrentSymbol + move) <= 255)
		{ 
			XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
			UT_ASSERT(iDrawSymbol);
			m_PreviousSymbol = m_CurrentSymbol;
			m_CurrentSymbol = m_CurrentSymbol + move;
			iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
		}

		gtk_signal_emit_stop_by_name((GTK_OBJECT(m_windowMain)),
									 "key_press_event");
	}
#endif
}

void XAP_QNXDialog_Insert_Symbol::SymbolMap_clicked( void * event)
{
#if 0
	UT_uint32 x, y;
	x = (UT_uint32) event->button.x;
	y = (UT_uint32) event->button.y;

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = iDrawSymbol->calcSymbol(x, y);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
#endif
}


void XAP_QNXDialog_Insert_Symbol::New_Font(void )
{
#if 0
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	
/*
  Extract the new font string from the combo box, update the current symbol
  font and display the new set of symbols to choose from.

  The text extraction code was stolen from ev_GnomeQNXToolbar.
*/

	gchar * buffer = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry));

	iDrawSymbol->setSelectedFont( (UT_UCSChar *) buffer);
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
#endif
}

void XAP_QNXDialog_Insert_Symbol::event_WindowDelete(void)
{
#if 0
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;	
	g_list_free( m_InsertS_Font_list);
	for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++)
		g_free(m_fontlist[i]);
	gtk_main_quit();
#endif
}


/*****************************************************************/

PtWidget_t * XAP_QNXDialog_Insert_Symbol::_constructWindow(void)
{
#if 0
	PtWidget_t * windowInsertS;

	PtWidget_t * vboxInsertS;
	PtWidget_t * vhbox;

	PtWidget_t * fontcombo;
	PtWidget_t * SymbolMap;

	PtWidget_t * areaCurrentSym;

	PtWidget_t * hboxInsertS;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XML_Char * tmp = NULL;

	windowInsertS = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowInsertS), "windowInsertS", windowInsertS);
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(XAP_STRING_ID_DLG_Insert_SymbolTitle));
	gtk_window_set_title (GTK_WINDOW (windowInsertS), tmp);
	FREEP(tmp);
	gtk_widget_set_usize(windowInsertS, 610, 245);

	gtk_window_set_policy (GTK_WINDOW (windowInsertS), FALSE, FALSE, FALSE);

	// Now put in a Vbox to hold our 3 widgets (Font Selector, Symbol Table
	// and OK -Selected Symbol- Cancel

	vboxInsertS = gtk_vbox_new( FALSE, 1);
	gtk_widget_show(vboxInsertS);
	// Insert the vbox into the dialog window

	gtk_container_add(GTK_CONTAINER(windowInsertS),vboxInsertS);

	// Now Build the font combo box into the frame

	/* First though we have to grab the fonts!

	   This code is to suck all the available fonts and put them in a GList.
	   This can then be displayed on a combo box at the top of the dialog.
	   Code stolen from ap_QNXToolbar_FontCombo */

	XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (m_pApp);
	UT_uint32 count = unixapp->getFontManager()->getCount();
	XAP_QNXFont ** list = unixapp->getFontManager()->getAllFonts();

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

	vhbox = gtk_hbox_new(FALSE, 1);

	// Insert the vhbox into the vbox to hold the combo box
	gtk_widget_show(vhbox);
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
					   (gchar *) DEFAULT_QNX_SYMBOL_FONT);

	// Turn off keyboard entry in the font selection box
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(fontcombo)->entry),FALSE);

	// Now put the font combo box at the top of the dialog 
	gtk_box_pack_start(GTK_BOX(vhbox), fontcombo, TRUE, FALSE, 0);

	// Now the Symbol Map. 

	// *** Code Stolen from the preview widget ***
	{
		SymbolMap = gtk_drawing_area_new ();
		gtk_object_set_data (GTK_OBJECT (windowInsertS), "SymbolMap", SymbolMap);
		gtk_widget_show (SymbolMap);

		// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
		gtk_widget_set_usize (SymbolMap, 608, 147);
		// Put this in the dialog under the font selection widget

		gtk_box_pack_start(GTK_BOX(vboxInsertS), SymbolMap, FALSE, FALSE, 0);

		// Enable button press events
		gtk_widget_add_events(SymbolMap, GDK_BUTTON_PRESS_MASK);
   	}
	
	// Now make a Hbox to hold  OK, Current Selection and Cancel
	hboxInsertS = gtk_hbox_new (FALSE, 1);

	// Insert the hbox into the dialog window
	gtk_object_set_data (GTK_OBJECT (windowInsertS), "hboxInsertS", hboxInsertS);
	gtk_widget_show (hboxInsertS);
	gtk_box_pack_start (GTK_BOX (vboxInsertS), hboxInsertS, TRUE, TRUE, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
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
		gtk_box_pack_start(GTK_BOX(hboxInsertS), areaCurrentSym, TRUE, FALSE, 0);
   	}

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_object_set_data (GTK_OBJECT (windowInsertS), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_box_pack_start(GTK_BOX(hboxInsertS), buttonCancel, TRUE, FALSE, 4);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	gtk_widget_show (hboxInsertS);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowInsertS;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_SymbolMap = 	SymbolMap;
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
	// Code stolen from ev_QNXGnomeToolbar.cpp

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
#endif
	return NULL;
}


