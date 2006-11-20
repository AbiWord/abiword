/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#ifdef USE_GUCHARMAP
#include <gucharmap/gucharmap.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include "gr_UnixGraphics.h"
#include "gr_UnixPangoGraphics.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_UnixDlg_Insert_Symbol.h"
#include "xap_Draw_Symbol.h"

/*****************************************************************/
/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

static UT_sint32 xap_UnixDlg_Insert_Symbol_first =0;
static UT_String xap_Unix_Prev_Font;

#ifndef USE_GUCHARMAP
static UT_UCSChar m_CurrentSymbol;
static UT_UCSChar m_PreviousSymbol;
#endif

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
	m_SymbolMap = 	NULL;
	m_InsertS_Font_list = NULL;
	m_Insert_Symbol_no_fonts = 0;
	m_fontlist.clear();

#ifndef USE_GUCHARMAP
	m_areaCurrentSym = NULL;
	m_unixGraphics = NULL;
	m_unixarea = NULL;
	m_ix = 0;
	m_iy = 0;
#endif
}

XAP_UnixDialog_Insert_Symbol::~XAP_UnixDialog_Insert_Symbol(void)
{
#ifndef USE_GUCHARMAP
	DELETEP(m_unixGraphics);
	DELETEP(m_unixarea);
#endif
}

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
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	abiSetupModelessDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_INSERT);

#ifndef USE_GUCHARMAP

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);
	
	UT_ASSERT(m_SymbolMap && m_SymbolMap->window);

	// make a new Unix GC
	DELETEP (m_unixGraphics);
	
	{
		//m_unixGraphics = new GR_UnixGraphics(m_SymbolMap->window, unixapp->getFontManager(), m_pApp);
		GR_UnixAllocInfo ai(m_SymbolMap->window, unixapp->getFontManager());
		m_unixGraphics = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);
	}
	// let the widget materialize
	_createSymbolFromGC(m_unixGraphics,
						static_cast<UT_uint32>(m_SymbolMap->allocation.width),
						static_cast<UT_uint32>(m_SymbolMap->allocation.height));
	
	// *** Re use the code to draw into the selected symbol area.
	UT_ASSERT(m_areaCurrentSym && m_areaCurrentSym->window);
	
	// make a new Unix GC
	DELETEP (m_unixarea);
    {
		//m_unixarea = new GR_UnixGraphics(m_areaCurrentSym->window, unixapp->getFontManager(), m_pApp);
		GR_UnixAllocInfo ai(m_areaCurrentSym->window, unixapp->getFontManager());
		m_unixarea = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);
	}
	// let the widget materialize
	_createSymbolareaFromGC(m_unixarea,
							static_cast<UT_uint32>(m_areaCurrentSym->allocation.width),
							static_cast<UT_uint32>(m_areaCurrentSym->allocation.height));

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);

	// We use this code to insert the default font name into to static
	// variable "m_Insert_Symbol_font" the first time this dialog is
	// called. Afterwards it is just whatever was left from the last
	// call.

	if ( xap_UnixDlg_Insert_Symbol_first == 0)
	{
		iDrawSymbol->setSelectedFont(DEFAULT_UNIX_SYMBOL_FONT);
		const gchar * buffer = UT_strdup(DEFAULT_UNIX_SYMBOL_FONT);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry),buffer);
		m_CurrentSymbol = ' ';
		m_PreviousSymbol = ' ';
		xap_UnixDlg_Insert_Symbol_first = 1;
	}
	else
	{
		iDrawSymbol->setSelectedFont(xap_Unix_Prev_Font.c_str());
	}
	// Show the top level dialog

	gtk_widget_show(mainWindow);

	// Put the current font in the entry box
	const char* iSelectedFont = UT_strdup(iDrawSymbol->getSelectedFont());
	xap_Unix_Prev_Font = iSelectedFont;
	UT_DEBUGMSG(("Selected Font at startup %s \n",iSelectedFont));
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry),
					   iSelectedFont);

	// Show the Previously selected symbol

	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// return to ap_Editmethods and wait for something interesting
	// to happen.
#else
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry),
					   DEFAULT_UNIX_SYMBOL_FONT);

	gucharmap_charmap_set_font (GUCHARMAP_CHARMAP (m_SymbolMap), DEFAULT_UNIX_SYMBOL_FONT);
#endif /* USE_GUCHARMAP */
}

void XAP_UnixDialog_Insert_Symbol::event_Insert(void)
{
#ifndef USE_GUCHARMAP
        m_Inserted_Symbol = m_CurrentSymbol;
       	_onInsertButton();
#else
		UT_ASSERT(m_pListener);
		const char * symfont = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry));
		m_Inserted_Symbol = gucharmap_table_get_active_character(gucharmap_charmap_get_chartable(GUCHARMAP_CHARMAP(m_SymbolMap)));
        m_pListener->setView(getActiveFrame()->getCurrentView());
		m_pListener->insertSymbol(m_Inserted_Symbol, const_cast<char*>(symfont));
#endif
}

void XAP_UnixDialog_Insert_Symbol::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;	
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
//
// Save last font
//
	if (iDrawSymbol)
		xap_Unix_Prev_Font = iDrawSymbol->getSelectedFont();
	g_list_free(m_InsertS_Font_list);
	
	for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++) 
	{
		free(m_fontlist.getNthItem(i));
	}
	m_fontlist.clear();
	modeless_cleanup();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void XAP_UnixDialog_Insert_Symbol::New_Font(void )
{
	const gchar * buffer = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry));

#ifndef USE_GUCHARMAP
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	// need a fallback value here, as this can get called from one of the gtk
	// callbacks when no font has been set.
	iDrawSymbol->setSelectedFont( buffer && *buffer ? static_cast<const char *>(buffer) :
		                                              "Symbol");
	
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
#else
	gucharmap_charmap_set_font (GUCHARMAP_CHARMAP (m_SymbolMap), buffer);
#endif
}

/*****************************************************************/

static void s_dlg_response ( GtkWidget * widget, gint id,
							 XAP_UnixDialog_Insert_Symbol * dlg )
{
	UT_return_if_fail(widget && dlg);

	switch ( id )
	  {
	  case XAP_UnixDialog_Insert_Symbol::BUTTON_INSERT:
		  dlg->event_Insert();
		  break;
		  
	  case XAP_UnixDialog_Insert_Symbol::BUTTON_CLOSE:
		  abiDestroyWidget(widget); // emit the destroy signal
		  break;
	  }
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->event_WindowDelete();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     XAP_UnixDialog_Insert_Symbol * dlg)
{
	abiDestroyWidget(widget);
}

static void s_new_font(GtkWidget * widget, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->New_Font();
}

#ifdef USE_GUCHARMAP

static void s_charmap_activate (GucharmapCharmap *charmap, gunichar ch, XAP_UnixDialog_Insert_Symbol * pDlg)
{
	pDlg->event_Insert ();
}

#else

static gboolean s_sym_SymbolMap_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->SymbolMap_exposed();
	return FALSE;
}

static gboolean s_Symbolarea_exposed(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->Symbolarea_exposed();
	return FALSE;
}

static gboolean  s_SymbolMap_clicked(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->SymbolMap_clicked( e );
	return FALSE; 
}

static gboolean  s_CurrentSymbol_clicked(GtkWidget * widget, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->CurrentSymbol_clicked( e );
	return FALSE; 
}

static gboolean s_keypressed(GtkWidget * widget, GdkEventKey * e,  XAP_UnixDialog_Insert_Symbol * dlg)
{
	return dlg->Key_Pressed( e );
}

/*****************************************************************/

void XAP_UnixDialog_Insert_Symbol::SymbolMap_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->draw();
	UT_DEBUGMSG(("main symbol area exposed \n"));
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
	UT_DEBUGMSG(("Current Symbol %x \n",m_CurrentSymbol));
	switch (e->keyval)
	{
	case GDK_Up:
		if(m_iy > 0)
		{
			m_iy--;
		}
		move = -32;
		break;
	case GDK_Down:
		if(m_iy < 6)
		{
			m_iy++;
		}
		move = 32;
		break;
	case GDK_Left:
		if(m_ix > 0)
		{
			m_ix--;
		}
		else if(m_iy > 0)
		{
			m_iy--;
			m_ix= 31;
		}
		move = -1;
		break;
	case GDK_Right:
		if(m_ix < 31)
		{
			m_ix++;
		}
		else if(m_iy < 6)
		{
			m_iy++;
			m_ix= 0;
		}
		move = 1;
		break;
	case GDK_Return:
		g_signal_stop_emission (G_OBJECT(m_windowMain), 
								g_signal_lookup ("key_press_event", 
												 G_OBJECT_TYPE (m_windowMain)), 0);
		event_Insert();
		return TRUE ;
		break;
	}
	UT_DEBUGMSG(("m_ix %d m_iy %d \n",m_ix,m_iy));
	if (move != 0)
	{
		XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
		UT_ASSERT(iDrawSymbol);
		m_PreviousSymbol = m_CurrentSymbol;
		m_CurrentSymbol = iDrawSymbol->calcSymbolFromCoords(m_ix, m_iy);
		UT_DEBUGMSG(("m_CurrentSymbol %x \n",m_CurrentSymbol));
		iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

		g_signal_stop_emission (G_OBJECT(m_windowMain), 
								g_signal_lookup ("key_press_event", 
												 G_OBJECT_TYPE (m_windowMain)), 0);
	}

	return FALSE ;
}

void XAP_UnixDialog_Insert_Symbol::SymbolMap_clicked( GdkEvent * event)
{
	UT_uint32 x,y;
	x = static_cast<UT_uint32>(event->button.x);
	y = static_cast<UT_uint32>(event->button.y);
	
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = iDrawSymbol->calcSymbol(x, y);
	iDrawSymbol->calculatePosition(m_CurrentSymbol,m_ix,m_iy);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// double click should also insert the symbol
	if(event->type == GDK_2BUTTON_PRESS)
	    event_Insert();
}

GtkWidget *XAP_UnixDialog_Insert_Symbol::_previewNew (int w, int h)
{
	GtkWidget *pre = createDrawingArea ();
	gtk_widget_show (pre);
	gtk_widget_set_size_request (pre, w, h);
	
	// Enable button press events
	gtk_widget_add_events(pre, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(pre, GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(pre, GDK_KEY_PRESS_MASK);
	gtk_widget_add_events(pre, GDK_KEY_RELEASE_MASK);
	gtk_widget_add_events(pre, GDK_EXPOSURE_MASK);
	gtk_widget_add_events(pre, GDK_ENTER_NOTIFY_MASK);
	gtk_widget_add_events(pre, GDK_LEAVE_NOTIFY_MASK);
	return pre;
}

void XAP_UnixDialog_Insert_Symbol::CurrentSymbol_clicked(GdkEvent *event)
{
	// have single-click insert the symbol
	if(event->type == GDK_BUTTON_PRESS)
	    event_Insert();
}

#endif /* USE_GUCHARMAP */

void XAP_UnixDialog_Insert_Symbol::destroy(void)
{
	g_list_free( m_InsertS_Font_list);
	for(UT_uint32 i = 0; i < m_Insert_Symbol_no_fonts; i++) 
		 free( m_fontlist.getNthItem(i));

	modeless_cleanup();
	
	// Just nuke this dialog
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_Insert_Symbol::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *tmp;

	ConstructWindowName();

	m_windowMain = abiDialogNew ("insert symbol dialog", TRUE, m_WindowName);

	// Now put in a Vbox to hold our 3 widgets (Font Selector, Symbol Table
	// and OK -Selected Symbol- Cancel
	tmp = GTK_DIALOG(m_windowMain)->vbox ;

	GtkWidget * hbox = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox);
	gtk_box_pack_start(GTK_BOX(tmp), hbox, FALSE, FALSE, 0);

	// Finally construct the combo box
	m_fontcombo = _createComboboxWithFonts ();

	// Now put the font combo box at the top of the dialog 
	gtk_box_pack_start(GTK_BOX(hbox), m_fontcombo, FALSE, FALSE, 0);

#ifndef USE_GUCHARMAP
	// Now the Symbol Map. 
	// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
	m_SymbolMap = _previewNew (608, 147);
	gtk_box_pack_start(GTK_BOX(tmp), m_SymbolMap, FALSE, FALSE, 0);
	
	m_areaCurrentSym = _previewNew (60, 45);
	gtk_box_pack_start(GTK_BOX(hbox), m_areaCurrentSym, TRUE, FALSE, 0);
#else
	m_SymbolMap = gucharmap_charmap_new (
		GUCHARMAP_CHAPTERS(gucharmap_block_chapters_new ()));
	gtk_widget_show (m_SymbolMap);
	gtk_box_pack_start (GTK_BOX (tmp), m_SymbolMap, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (m_SymbolMap), 5);

	gtk_widget_set_size_request (m_windowMain, 700, 300);
#endif

	gtk_widget_show_all (hbox);

	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_CLOSE, BUTTON_CLOSE);
	tmp = abiAddButton (GTK_DIALOG(m_windowMain), "&Insert" /* not used */, BUTTON_INSERT);
	localizeButtonUnderline (tmp, pSS, XAP_STRING_ID_DLG_Insert);
	
	_connectSignals ();

	return m_windowMain;
}
	
/*
  This code is to suck all the available fonts and put them in a GList.
  This can then be displayed on a combo box at the top of the dialog.
  Code stolen from ap_UnixToolbar_FontCombo. Now we remove all the 
  duplicate name entries and create the Glist glFonts. This will be 
  used in the font selection combo box 
*/
GList *XAP_UnixDialog_Insert_Symbol::_getGlistFonts (void)
{
	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	if(!pGF)
	{
		return NULL;
	}

	UT_uint32 iGR = pGF->getDefaultClass(true);
	
	UT_GenericVector<XAP_UnixFont*>* fonts = NULL;
	UT_GenericVector<const char*>* names = NULL;
	UT_uint32 iCount = 0;
	
	if(iGR != GRID_UNIX_PANGO)
	{
		fonts = XAP_UnixFontManager::pFontManager->getAllFonts();
		UT_return_val_if_fail( fonts,false );
		iCount = fonts->size();
	}
#ifdef USE_PANGO
	else
	{
		names = GR_UnixPangoGraphics::getAllFontNames();
		UT_return_val_if_fail( names, false );
		iCount = names->size();
	}
#endif
	GList *glFonts = NULL;
	UT_String currentfont;
	UT_uint32 j = 0;

	for (UT_uint32 i = 0; i < iCount; i++)
	{
		const gchar * lgn = NULL;
		if(iGR != GRID_UNIX_PANGO)
		{
			XAP_UnixFont * pFont = fonts->getNthItem(i);
			lgn = static_cast<const gchar *>(pFont->getName());
		}
		else
		{
			lgn = static_cast<const gchar *>(names->getNthItem(i));
		}
		
		if((strstr(currentfont.c_str(),lgn)==NULL) || (currentfont.size() !=strlen(lgn)) )
		{
			currentfont = lgn;
			m_fontlist.addItem(static_cast<gchar*>(UT_strdup(currentfont.c_str())));
			glFonts = g_list_prepend(glFonts, m_fontlist.getNthItem(j));
			j++;
		}
	}	

	m_Insert_Symbol_no_fonts = j;

	DELETEP(fonts);
	DELETEP(names);
	return g_list_reverse(glFonts);
}

GtkWidget *XAP_UnixDialog_Insert_Symbol::_createComboboxWithFonts (void)
{
	GtkWidget *fontcombo = gtk_combo_new();
	gtk_widget_show(fontcombo);

	m_InsertS_Font_list = _getGlistFonts ();
	gtk_combo_set_popdown_strings(GTK_COMBO(fontcombo), m_InsertS_Font_list);

	// Turn off keyboard entry in the font selection box
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(fontcombo)->entry), FALSE);

	return fontcombo;
}

void XAP_UnixDialog_Insert_Symbol::_connectSignals (void)
{
	g_signal_connect(G_OBJECT(m_windowMain),
					 "response",
					 G_CALLBACK(s_dlg_response),
					 static_cast<gpointer>(this));

	// Look for "changed" signal on the entry part of the combo box.
	// Code stolen from ev_UnixGnomeToolbar.cpp
	GtkEntry * blah = GTK_ENTRY(GTK_COMBO(m_fontcombo)->entry);
	g_signal_connect(G_OBJECT(&blah->widget),
					   "changed",
					   GTK_SIGNAL_FUNC(s_new_font),
					   static_cast<gpointer>(this));

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   static_cast<gpointer>(this));

#ifndef USE_GUCHARMAP

	// The event to choose the Symbol!
	g_signal_connect(G_OBJECT(m_SymbolMap),
					 "button_press_event",
					 G_CALLBACK(s_SymbolMap_clicked),
					 static_cast<gpointer>(this));

	// The event to choose the Symbol!
	g_signal_connect(G_OBJECT(m_areaCurrentSym),
					 "button_press_event",
					 G_CALLBACK(s_CurrentSymbol_clicked),
					 static_cast<gpointer>(this));

	// Look for keys pressed
	g_signal_connect(G_OBJECT(m_windowMain),
					 "key_press_event",
					 G_CALLBACK(s_keypressed),
					 static_cast<gpointer>(this));
	
	// the expose event of the m_SymbolMap
	g_signal_connect(G_OBJECT(m_SymbolMap),
					 "expose_event",
					 G_CALLBACK(s_sym_SymbolMap_exposed),
					 static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_areaCurrentSym),
					   "expose_event",
					   G_CALLBACK(s_Symbolarea_exposed),
					   static_cast<gpointer>(this));
#else
	g_signal_connect (G_OBJECT (gucharmap_charmap_get_chartable (GUCHARMAP_CHARMAP (m_SymbolMap))), "activate",
					  G_CALLBACK(s_charmap_activate), static_cast<gpointer>(this));
#endif

}
