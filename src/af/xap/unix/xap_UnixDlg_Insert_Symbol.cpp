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
#include "xap_UnixDialogHelper.h"

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

XAP_Dialog * XAP_UnixDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id id)
{
	XAP_UnixDialog_Insert_Symbol * p = new XAP_UnixDialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_UnixDialog_Insert_Symbol::XAP_UnixDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_unixGraphics = NULL;
	m_unixarea = NULL;
	
	m_SymbolMap = 	NULL;

	m_areaCurrentSym = NULL;
	m_InsertS_Font_list = NULL;
}

XAP_UnixDialog_Insert_Symbol::~XAP_UnixDialog_Insert_Symbol(void)
{
	DELETEP(m_unixGraphics);
	DELETEP(m_unixarea);
}


/*****************************************************************/

static void s_dlg_response ( GtkWidget * widget, gint id,
							 XAP_UnixDialog_Insert_Symbol * dlg )
{
	UT_return_if_fail(widget && dlg);

	switch ( id )
	{
		case XAP_UnixDialog_Insert_Symbol::BUTTON_OK:
			dlg->event_OK();
			break;
		case XAP_UnixDialog_Insert_Symbol::BUTTON_CANCEL:
			dlg->event_Cancel();
			break;
	}
}

static void s_sym_SymbolMap_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT( dlg);
	dlg->SymbolMap_exposed();
}


static void s_Symbolarea_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT( dlg);
	dlg->Symbolarea_exposed();
}

static gboolean  s_SymbolMap_clicked(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->SymbolMap_clicked( e );
        return FALSE; 
}

static gboolean  s_CurrentSymbol_clicked(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->CurrentSymbol_clicked( e );
        return FALSE; 
}

static void s_new_font(GtkWidget * widget, XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->New_Font();
}

static gboolean s_keypressed(GtkWidget * widget, GdkEventKey * e,  XAP_UnixDialog_Insert_Symbol * dlg)
{
	return dlg->Key_Pressed( e );
}



static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_UnixDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

#if 0
// TODO: there must be a better way of doing this
// TODO: it just seems so wasteful to have a callback
// TODO: registered for every time the mouse moves over a widget
static void s_motion_event(GtkWidget * /* widget */,
			   GdkEventMotion *evt,
			   XAP_UnixDialog_Insert_Symbol *dlg)
{
        UT_DEBUGMSG(("DOM: motion event\n"));
        dlg->Motion_event(evt);
}

void XAP_UnixDialog_Insert_Symbol::Motion_event(GdkEventMotion *e)
{
	UT_uint32 x, y;

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	x = (UT_uint32) e->x;
	y = (UT_uint32) e->y;

	UT_UCSChar cSymbol = iDrawSymbol->calcSymbol(x, y);
	
	// only draw if different
	if(m_CurrentSymbol != cSymbol)
	  {
	    m_PreviousSymbol = m_CurrentSymbol;
	    m_CurrentSymbol = cSymbol;
	    iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
	  }
}
#endif		    

/*****************************************************************/

void XAP_UnixDialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
}

void XAP_UnixDialog_Insert_Symbol::activate(void)
{
        UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
        gdk_window_raise(m_windowMain->window);
}

void   XAP_UnixDialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *pFrame)
{
        UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
}


void XAP_UnixDialog_Insert_Symbol::runModeless(XAP_Frame * pFrame)
{
	// First see if the dialog is already running
	UT_sint32 sid =(UT_sint32)  getDialogId();
	  
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	abiSetupModelessDialog(GTK_DIALOG(mainWindow),
						   pFrame, this, BUTTON_CANCEL);

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);

	UT_ASSERT(m_SymbolMap && m_SymbolMap->window);

	// make a new Unix GC
	DELETEP (m_unixGraphics);
#ifndef WITH_PANGO	
	m_unixGraphics = new GR_UnixGraphics(m_SymbolMap->window, unixapp->getFontManager(), m_pApp);
#else
	m_unixGraphics = new GR_UnixGraphics(m_SymbolMap->window, m_pApp);
#endif 	

	// let the widget materialize
	_createSymbolFromGC(m_unixGraphics,
						(UT_uint32) m_SymbolMap->allocation.width,
						(UT_uint32) m_SymbolMap->allocation.height);

	// *** Re use the code to draw into the selected symbol area.
	UT_ASSERT(m_areaCurrentSym && m_areaCurrentSym->window);

	// make a new Unix GC
	DELETEP (m_unixarea);
#ifndef WITH_PANGO	
	m_unixarea = new GR_UnixGraphics(m_areaCurrentSym->window, unixapp->getFontManager(), m_pApp);
#else
	m_unixarea = new GR_UnixGraphics(m_areaCurrentSym->window,m_pApp);
#endif
		
	// let the widget materialize
	_createSymbolareaFromGC(m_unixarea,
							(UT_uint32) m_areaCurrentSym->allocation.width,
							(UT_uint32) m_areaCurrentSym->allocation.height);

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);

	// We use this code to insert the default font name into to static
	// variable "m_Insert_Symbol_font" the first time this dialog is
	// called. Afterwards it is just whatever was left from the last
	// call.

	if ( xap_UnixDlg_Insert_Symbol_first == 0)
	{
		iDrawSymbol->setSelectedFont( (char *) DEFAULT_UNIX_SYMBOL_FONT);
		m_CurrentSymbol = ' ';
		m_PreviousSymbol = ' ';
		xap_UnixDlg_Insert_Symbol_first = 1;
	}

	// Show the top level dialog

	gtk_widget_show(mainWindow);

        // Put the current font in the entry box
	char* iSelectedFont = iDrawSymbol->getSelectedFont();
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry),
					   (gchar *)   iSelectedFont);

	// Show the Previously selected symbol

	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// return to ap_Editmethods and wait for something interesting
	// to happen.
}

void XAP_UnixDialog_Insert_Symbol::event_OK(void)
{
        m_Inserted_Symbol = m_CurrentSymbol;
       	_onInsertButton();
}

void XAP_UnixDialog_Insert_Symbol::event_Cancel(void)
{
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
		modeless_cleanup();
		if(m_windowMain && GTK_IS_WIDGET(m_windowMain))
		  gtk_widget_destroy(m_windowMain);
		m_windowMain= NULL;
	}
}

void XAP_UnixDialog_Insert_Symbol::SymbolMap_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->draw();
	/*
	    Need this to see the blue square after an expose event
	*/
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_UnixDialog_Insert_Symbol::Symbolarea_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

//
// This function allows the symbol to be selected via the keyboard
//

gboolean XAP_UnixDialog_Insert_Symbol::Key_Pressed(GdkEventKey * e)
{
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
        case GDK_Return:
	        gtk_signal_emit_stop_by_name((GTK_OBJECT(m_windowMain)),
										 "key_press_event");
			event_OK();
			return TRUE ;
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

	return FALSE ;
}

void XAP_UnixDialog_Insert_Symbol::SymbolMap_clicked( GdkEvent * event)
{
	UT_uint32 x, y;
	x = (UT_uint32) event->button.x;
	y = (UT_uint32) event->button.y;

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = iDrawSymbol->calcSymbol(x, y);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// double click should also insert the symbol
        if(event->type == GDK_2BUTTON_PRESS)
	    event_OK();
}


void XAP_UnixDialog_Insert_Symbol::New_Font(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	
/*
  Extract the new font string from the combo box, update the current symbol
  font and display the new set of symbols to choose from.

  The text extraction code was stolen from ev_GnomeUnixToolbar.
*/

	const gchar * buffer = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry));

	iDrawSymbol->setSelectedFont( (char *) buffer);
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}


void XAP_UnixDialog_Insert_Symbol::destroy(void)
{

	g_list_free( m_InsertS_Font_list);
	for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++) g_free(m_fontlist[i]);
        modeless_cleanup();

	// Just nuke this dialog
       
	gtk_widget_destroy(m_windowMain);
        m_windowMain = NULL;
}

void XAP_UnixDialog_Insert_Symbol::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;	
	g_list_free( m_InsertS_Font_list);

	for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++)
		g_free(m_fontlist[i]);

        modeless_cleanup();
        gtk_widget_destroy(m_windowMain);
        m_windowMain = NULL;
}


/*****************************************************************/

GtkWidget * XAP_UnixDialog_Insert_Symbol::_constructWindow(void)
{
	GtkWidget * vboxInsertS;
	GtkWidget * vhbox;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;

	ConstructWindowName();

	m_windowMain = abiDialogNew ( TRUE, m_WindowName ) ;

	// Now put in a Vbox to hold our 3 widgets (Font Selector, Symbol Table
	// and OK -Selected Symbol- Cancel
	vboxInsertS = GTK_DIALOG(m_windowMain)->vbox ;

	vhbox = gtk_hbox_new(FALSE, 1);

	// Insert the vhbox into the vbox to hold the combo box
	gtk_widget_show(vhbox);
	gtk_box_pack_start(GTK_BOX(vboxInsertS), vhbox, TRUE, TRUE, 0);

	// Finally construct the combo box
	m_fontcombo = _createComboboxWithFonts ();

	// Now put the font combo box at the top of the dialog 
	gtk_box_pack_start(GTK_BOX(vhbox), m_fontcombo, TRUE, FALSE, 0);

	// Now the Symbol Map. 
	// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
	m_SymbolMap = _previewNew (608, 147);
	gtk_box_pack_start(GTK_BOX(vboxInsertS), m_SymbolMap, FALSE, FALSE, 0);
	

	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_OK, BUTTON_OK) ;
	m_areaCurrentSym = _previewNew (60, 45);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_windowMain)->action_area),
					   m_areaCurrentSym, TRUE, FALSE, 0);
	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL) ;
	
	_connectSignals ();

	return m_windowMain;
}

GtkWidget *XAP_UnixDialog_Insert_Symbol::_previewNew (int w, int h)
{
	GtkWidget *pre = createDrawingArea ();
	gtk_widget_show (pre);
	gtk_widget_set_usize (pre, w, h);
	
	// Enable button press events
	gtk_widget_add_events(pre, GDK_BUTTON_PRESS_MASK);
	return pre;
}
	
/*
  This code is to suck all the available fonts and put them in a GList.
  This can then be displayed on a combo box at the top of the dialog.
  Code stolen from ap_UnixToolbar_FontCombo */
/* Now we remove all the duplicate name entries and create the Glist
   glFonts. This will be used in the font selection combo
   box */

GList *XAP_UnixDialog_Insert_Symbol::_getGlistFonts (void)
{	  
#ifndef WITH_PANGO	
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_Vector * list = unixapp->getFontManager()->getAllFonts();
	UT_uint32 count = list->size();
#endif

	GList *glFonts = NULL;
	gchar currentfont[50] = "\0";
	UT_uint32 j = 0;

#ifndef WITH_PANGO	
	for (UT_uint32 i = 0; i < count; i++)
	{
		XAP_UnixFont * pFont = (XAP_UnixFont *)list->getNthItem(i);
		gchar * lgn  = (gchar *) pFont->getName();
#else
	const XAP_PangoFontManager * pManager = GR_Graphics::getFontManager();
	UT_ASSERT(pManager);

	UT_uint32 iFontCount =  pManager->getAvailableFontFamiliesCount();

	for (UT_uint32 i = 0; i < iFontCount; i++)
	{
		gchar * lgn  = (gchar *) pManager->getNthAvailableFontFamily(i);
#endif	
		
		if((strstr(currentfont,lgn)==NULL) || (strlen(currentfont)!=strlen(lgn)) )
		{
			strncpy(currentfont, lgn, 50);
			m_fontlist[j] = g_strdup(currentfont);
			glFonts = g_list_prepend(glFonts, m_fontlist[j++]);
		}
	}
	

	m_Insert_Symbol_no_fonts = j;

#ifndef WITH_PANGO
	DELETEP(list);
#endif
	
	return g_list_reverse(glFonts);
}

void XAP_UnixDialog_Insert_Symbol::CurrentSymbol_clicked(GdkEvent *event)
{
	// have single-click insert the symbol
        if(event->type == GDK_BUTTON_PRESS)
	    event_OK();
}

GtkWidget *XAP_UnixDialog_Insert_Symbol::_createComboboxWithFonts (void)
{
	GtkWidget *fontcombo = gtk_combo_new();

	m_InsertS_Font_list = _getGlistFonts ();
 
	gtk_widget_set_usize(fontcombo, 200, 25);
	gtk_widget_show(fontcombo);
	gtk_combo_set_value_in_list(GTK_COMBO(fontcombo), TRUE, TRUE);
	gtk_combo_set_use_arrows(GTK_COMBO(fontcombo), FALSE);
	gtk_combo_set_popdown_strings(GTK_COMBO(fontcombo), m_InsertS_Font_list);

	// Put the current font in the entry box.
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(fontcombo)->entry),
					   (gchar *) DEFAULT_UNIX_SYMBOL_FONT);

	// Turn off keyboard entry in the font selection box
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(fontcombo)->entry),FALSE);

	return fontcombo;
}

void XAP_UnixDialog_Insert_Symbol::_connectSignals (void)
{
	//
	g_signal_connect(G_OBJECT(m_windowMain),
					 "response",
					 G_CALLBACK(s_dlg_response),
					 (gpointer)this);
	
	// The event to choose the Symbol!
	g_signal_connect(G_OBJECT(m_SymbolMap),
					   "button_press_event",
				       G_CALLBACK(s_SymbolMap_clicked),
					   (gpointer) this);

	// The event to choose the Symbol!
	g_signal_connect(G_OBJECT(m_areaCurrentSym),
			   "button_press_event",
			   G_CALLBACK(s_CurrentSymbol_clicked),
			   (gpointer) this);

	// Look for keys pressed
	g_signal_connect(G_OBJECT(m_windowMain),
					   "key_press_event",
					   G_CALLBACK(s_keypressed),
					   (gpointer) this);


	// Look for "changed" signal on the entry part of the combo box.
	// Code stolen from ev_UnixGnomeToolbar.cpp
	GtkEntry * blah = GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry);
	g_signal_connect(G_OBJECT(&blah->widget),
					   "changed",
					   G_CALLBACK(s_new_font),
					   (gpointer) this);

	// the catch-alls
	// Dont use g_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
							 "delete_event",
							 G_CALLBACK(s_delete_clicked),
							 (gpointer) this);

	g_signal_connect_after(G_OBJECT(m_windowMain),
							 "destroy",NULL, NULL);
	
	// the expose event of the m_SymbolMap
	g_signal_connect(G_OBJECT(m_SymbolMap),
					   "expose_event",
					   G_CALLBACK(s_sym_SymbolMap_exposed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_areaCurrentSym),
					   "expose_event",
					   G_CALLBACK(s_Symbolarea_exposed),
					   (gpointer) this);
}
