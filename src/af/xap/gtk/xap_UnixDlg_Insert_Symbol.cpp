/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include "gr_UnixCairoGraphics.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_UnixDlg_Insert_Symbol.h"
#include "xap_Draw_Symbol.h"


/*****************************************************************/
/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

static UT_sint32 s_Insert_Symbol_first = 0;
static std::string s_Prev_Font;

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
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id),
	m_windowMain(NULL),
	m_SymbolMap(NULL)
{
	m_areaCurrentSym = NULL;
	m_unixGraphics = NULL;
	m_unixarea = NULL;
	m_ix = 0;
	m_iy = 0;
}

XAP_UnixDialog_Insert_Symbol::~XAP_UnixDialog_Insert_Symbol(void)
{
	DELETEP(m_unixGraphics);
	DELETEP(m_unixarea);
}

void XAP_UnixDialog_Insert_Symbol::runModal(XAP_Frame * )
{
}

void XAP_UnixDialog_Insert_Symbol::activate(void)
{
	UT_return_if_fail(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	gdk_window_raise(gtk_widget_get_window(m_windowMain));
}

void   XAP_UnixDialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *)
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

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	//XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	//UT_ASSERT(unixapp);
	
	UT_ASSERT(m_SymbolMap && gtk_widget_get_window(m_SymbolMap));

	// make a new Unix GC
	DELETEP (m_unixGraphics);
	
	{
		GR_UnixCairoAllocInfo ai(m_SymbolMap);
		m_unixGraphics =
			(GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);
	}
	// let the widget materialize
	GtkAllocation alloc;
	gtk_widget_get_allocation(m_SymbolMap, &alloc);
	_createSymbolFromGC(m_unixGraphics,
						static_cast<UT_uint32>(alloc.width),
						static_cast<UT_uint32>(alloc.height));
	
	// *** Re use the code to draw into the selected symbol area.
	UT_ASSERT(m_areaCurrentSym && gtk_widget_get_window(m_areaCurrentSym));
	
	// make a new Unix GC
	DELETEP (m_unixarea);
    {
		GR_UnixCairoAllocInfo ai(m_areaCurrentSym);
		m_unixarea =
			(GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);
	}
	// let the widget materialize
	gtk_widget_get_allocation(m_areaCurrentSym, &alloc);
	_createSymbolareaFromGC(m_unixarea,
							static_cast<UT_uint32>(alloc.width),
							static_cast<UT_uint32>(alloc.height));

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);

	// We use this code to insert the default font name into to static
	// called. Afterwards it is just whatever was left from the last
	// call.

	if ( s_Insert_Symbol_first == 0)
	{
		iDrawSymbol->setSelectedFont(DEFAULT_UNIX_SYMBOL_FONT);
		UT_UCSChar c = iDrawSymbol->calcSymbol(0, 0);
		if (c)
		{
			m_PreviousSymbol = c;
		        m_CurrentSymbol = c;
			iDrawSymbol->calculatePosition(m_CurrentSymbol, m_ix, m_iy);
		}
		s_Insert_Symbol_first = 1;
	}
	else
	{
		iDrawSymbol->setSelectedFont(s_Prev_Font.c_str());
	}

	_setScrolledWindow ();

	// Show the top level dialog
	gtk_widget_show(mainWindow);

	// Put the current font in the entry box
	const char* iSelectedFont = iDrawSymbol->getSelectedFont();
	s_Prev_Font = iSelectedFont;
	UT_DEBUGMSG(("Selected Font at startup %s \n",iSelectedFont));
	gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_fontcombo))),
					   iSelectedFont);

	// Show the Previously selected symbol

	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// return to ap_Editmethods and wait for something interesting
	// to happen.
}

void XAP_UnixDialog_Insert_Symbol::event_Insert(void)
{
        m_Inserted_Symbol = m_CurrentSymbol;
       	_onInsertButton();
}

void XAP_UnixDialog_Insert_Symbol::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;	
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
//
// Save last font
//
	if (iDrawSymbol)
		s_Prev_Font = iDrawSymbol->getSelectedFont();
    m_InsertS_Font_list.clear();
	
	modeless_cleanup();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void XAP_UnixDialog_Insert_Symbol::New_Font(void )
{
	const gchar * buffer = gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_fontcombo))));

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);

	// need a fallback value here, as this can get called from one of the gtk
	// callbacks when no font has been set.
	iDrawSymbol->setSelectedFont( buffer && *buffer ? static_cast<const char *>(buffer) :
		                                              "Symbol");
	
	// we get strange things if the previous Symbol does not exists in the
	// new font. Reset it
	UT_UCSChar c = iDrawSymbol->calcSymbol(0, 0);
	if (c)
	{
		m_PreviousSymbol = c;
	        m_CurrentSymbol = c;
		iDrawSymbol->calculatePosition(m_CurrentSymbol, m_ix, m_iy);
	}
 	
	_setScrolledWindow ();
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_UnixDialog_Insert_Symbol::New_Row(void)
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);

	// FIXME rounding is better
	UT_uint32 row = UT_uint32 (gtk_adjustment_get_value(m_vadjust));

	iDrawSymbol->setRow (row);
}

void XAP_UnixDialog_Insert_Symbol::Scroll_Event (int direction)
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);
	double value = gtk_adjustment_get_value(m_vadjust);

	if (direction && gtk_adjustment_get_upper(m_vadjust) > value)
	{
		gtk_adjustment_set_value(m_vadjust, value + 1);
	}
	else if (!direction && gtk_adjustment_get_lower(m_vadjust) <= value - 1)
 	{
		gtk_adjustment_set_value(m_vadjust, value - 1);
    }
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
							 XAP_UnixDialog_Insert_Symbol * /*dlg*/)
{
	abiDestroyWidget(widget);
}

static void s_new_font(GtkWidget * /*widget*/, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->New_Font();
}

static void s_new_row(GtkWidget * /*widget*/, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->New_Row();
}

static void  s_scroll_event(GtkWidget * /*widget*/, GdkEventScroll * event, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->Scroll_Event (static_cast <int> (event->direction));
}

#if GTK_CHECK_VERSION(3,0,0)
static gboolean s_sym_SymbolMap_draw(GtkWidget * /*widget*/, cairo_t * /*cr*/, XAP_UnixDialog_Insert_Symbol * dlg)
#else
static gboolean s_sym_SymbolMap_draw(GtkWidget * /*widget*/, GdkEvent * /*cr*/, XAP_UnixDialog_Insert_Symbol * dlg)
#endif
{
	dlg->SymbolMap_exposed();
	return FALSE;
}

static gboolean s_size_request(GtkWidget *, GtkAllocation* req, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->setSymbolMap_size (req->width, req->height);
	return FALSE;
}


#if GTK_CHECK_VERSION(3,0,0)
static gboolean s_Symbolarea_draw(GtkWidget * , cairo_t * , XAP_UnixDialog_Insert_Symbol * dlg)
#else
static gboolean s_Symbolarea_draw(GtkWidget * , GdkEvent * , XAP_UnixDialog_Insert_Symbol * dlg)
#endif
{
	dlg->Symbolarea_exposed();
	return FALSE;
}

static gboolean  s_SymbolMap_clicked(GtkWidget *, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->SymbolMap_clicked( e );
	return FALSE; 
}

static gboolean  s_CurrentSymbol_clicked(GtkWidget *, GdkEvent * e, XAP_UnixDialog_Insert_Symbol * dlg)
{
	dlg->CurrentSymbol_clicked( e );
	return FALSE; 
}

static gboolean s_keypressed(GtkWidget *, GdkEventKey * e,  XAP_UnixDialog_Insert_Symbol * dlg)
{
	return dlg->Key_Pressed( e );
}

/*****************************************************************/

void XAP_UnixDialog_Insert_Symbol::SymbolMap_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);
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
	UT_return_if_fail(iDrawSymbol);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_UnixDialog_Insert_Symbol::setSymbolMap_size(UT_uint32 width, UT_uint32 height)
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_return_if_fail(iDrawSymbol);
	UT_return_if_fail(m_windowMain);
	UT_return_if_fail(m_SymbolMap);

	
	static UT_uint32 diff_width = 0; 
	static UT_uint32 diff_height = 0;


	// only in the beginnig we can measure the difference
	// between window and drawingarea this show stay constant
	GtkRequisition diff;
	GtkAllocation alloc;
	gtk_widget_get_requisition (m_windowMain, &diff);
	gtk_widget_get_allocation (m_SymbolMap, &alloc);
	if (!diff_width || !diff_height)
	{
		diff_width = diff.width - alloc.width;
		diff_height = diff.height - alloc.height;
	}
	diff.width = width - diff_width;
	diff.height = height - diff_height;

    // set new sizes
    iDrawSymbol->setWindowSize(diff.width, diff.height);
	iDrawSymbol->setFontString ();
}


//
// This function allows the symbol to be selected via the keyboard
//

gboolean XAP_UnixDialog_Insert_Symbol::Key_Pressed(GdkEventKey * e)
{
	int move = 0;
	UT_uint32 ix = m_ix;
	UT_uint32 iy = m_iy;
	UT_DEBUGMSG(("Current Symbol %x \n",m_CurrentSymbol));
	switch (e->keyval)
	{
	case GDK_KEY_Up:
		if(iy > 0)
		{
			iy--;
		}
		else
		{
			Scroll_Event (0);
		}
		move = -32;
		break;
	case GDK_KEY_Down:
		if(iy < 6)
		{
			iy++;
		}
		else
		{
			Scroll_Event (1);
		}
		move = 32;
		break;
	case GDK_KEY_Left:
		if(ix > 0)
		{
			ix--;
		}
		else if(iy > 0)
		{
			iy--;
			ix = 31;
		}
		else
		{
			Scroll_Event (0);
			ix = 31;
			
		}
		move = -1;
		break;
	case GDK_KEY_Right:
		if(ix < 31)
		{
			ix++;
		}
		else if(iy < 6)
		{
			iy++;
			ix = 0;
		}
		else
		{
			Scroll_Event (1);
			ix = 0;
			
		}
		move = 1;
		break;
	case GDK_KEY_Return:
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
		UT_return_val_if_fail(iDrawSymbol,FALSE);
		UT_UCSChar c = iDrawSymbol->calcSymbolFromCoords(ix, iy);
		if (c)
		{ m_PreviousSymbol = m_CurrentSymbol;
		  m_CurrentSymbol = c;
  		  UT_DEBUGMSG(("m_CurrentSymbol %x \n",m_CurrentSymbol));
		  m_ix = ix;
		  m_iy = iy;
		}
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
	UT_return_if_fail(iDrawSymbol);
	UT_UCSChar c = iDrawSymbol->calcSymbol(x, y);
	if (c)
	{
		m_PreviousSymbol = m_CurrentSymbol;
	 	m_CurrentSymbol = c;
		iDrawSymbol->calculatePosition(m_CurrentSymbol, m_ix, m_iy);
		iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

		// double click should also insert the symbol
		if(event->type == GDK_2BUTTON_PRESS)
			event_Insert();
	}
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
	gtk_widget_add_events(pre, GDK_SCROLL_MASK);
	return pre;
}

void XAP_UnixDialog_Insert_Symbol::CurrentSymbol_clicked(GdkEvent *event)
{
	// have single-click insert the symbol
	if(event->type == GDK_BUTTON_PRESS)
	    event_Insert();
}

void XAP_UnixDialog_Insert_Symbol::destroy(void)
{
	UT_DEBUGMSG(("XAP_UnixDialog_Insert_Symbol::destroy()"));
    m_InsertS_Font_list.clear();
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
	gtk_window_set_position(GTK_WINDOW(m_windowMain), GTK_WIN_POS_MOUSE);
#if !GTK_CHECK_VERSION(3,0,0)
	gtk_dialog_set_has_separator(GTK_DIALOG(m_windowMain), FALSE);
#endif	

	// Now put in a Vbox to hold our 3 widgets (Font Selector, Symbol Table
	// and OK -Selected Symbol- Cancel
	tmp = gtk_dialog_get_content_area(GTK_DIALOG(m_windowMain));

	GtkWidget * vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget * vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget * hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_show (hbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(tmp), hbox, FALSE, FALSE, 0);

	// Finally construct the combo box
	m_fontcombo = _createComboboxWithFonts ();

	// Now put the font combo box at the top of the dialog 
	gtk_box_pack_start(GTK_BOX(vbox1), m_fontcombo, FALSE, FALSE, 0);

	// Now the Symbol Map. 
	// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
	//
	GtkWidget * hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_show (hbox1);
	gtk_box_pack_start(GTK_BOX(tmp), hbox1, TRUE, TRUE, 4);


		
	m_SymbolMap = _previewNew (608, 147);
	gtk_box_pack_start (GTK_BOX (hbox1), m_SymbolMap, TRUE, TRUE, 0);

	m_vadjust = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 7, 0, 0, 7));
	GtkWidget *vscroll = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, m_vadjust);
	gtk_widget_show (vscroll);
	gtk_box_pack_start (GTK_BOX (hbox1), vscroll, FALSE, FALSE, 0);

	m_areaCurrentSym = _previewNew (60, 45);
	gtk_box_pack_start(GTK_BOX(vbox2), m_areaCurrentSym, TRUE, FALSE, 0);

	gtk_widget_show_all (hbox);

	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Close, s);
	abiAddButton (GTK_DIALOG(m_windowMain), s, BUTTON_CLOSE);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Insert, s);
	tmp = abiAddButton (GTK_DIALOG(m_windowMain), s, BUTTON_INSERT);

	_connectSignals ();

	return m_windowMain;
}
	
/*
  This code is to suck all the available fonts and put them in a std::list.
  This can then be displayed on a combo box at the top of the dialog.
  Code stolen from ap_UnixToolbar_FontCombo. Now we remove all the 
  duplicate name entries and create the Glist glFonts. This will be 
  used in the font selection combo box 
*/
void XAP_UnixDialog_Insert_Symbol::_getGlistFonts (std::list<std::string> & glFonts)
{
	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	if(!pGF)
	{
		return;
	}

	const std::vector<std::string> & names =
		GR_CairoGraphics::getAllFontNames();
	
	for (std::vector<std::string>::const_iterator i = names.begin(); 
		 i != names.end(); ++i)
	{
        const std::string & lgn = *i;
		glFonts.push_back(lgn);
	}	

    glFonts.sort();

    std::string lastfont;
    for (std::list<std::string>::iterator iter = glFonts.begin();
         iter != glFonts.end(); ) 
    {
		if (lastfont == *iter)
		{
            iter = glFonts.erase(iter);
            continue;
		}
        lastfont = *iter;
        ++iter;
    }
}

GtkWidget *XAP_UnixDialog_Insert_Symbol::_createComboboxWithFonts (void)
{
	GtkWidget *fontcombo = gtk_combo_box_text_new_with_entry();
	gtk_widget_show(fontcombo);

	// ensure we don't override this without freeing...
    m_InsertS_Font_list.clear();
	_getGlistFonts (m_InsertS_Font_list);
	for(std::list<std::string>::const_iterator iter = m_InsertS_Font_list.begin();
        iter != m_InsertS_Font_list.end(); ++iter)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(fontcombo), iter->c_str());
	}

	// Turn off keyboard entry in the font selection box
	gtk_editable_set_editable(GTK_EDITABLE(gtk_bin_get_child(GTK_BIN(fontcombo))), FALSE);

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
	GtkEntry * blah = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_fontcombo)));
	g_signal_connect(G_OBJECT(blah),
					 "changed",
					 G_CALLBACK(s_new_font),
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

	g_signal_connect(G_OBJECT(m_windowMain),
					 "size-allocate",
					 G_CALLBACK(s_size_request),
					 static_cast<gpointer>(this));
	
	// the expose event of the m_SymbolMap
	g_signal_connect(G_OBJECT(m_SymbolMap),
#if GTK_CHECK_VERSION(3,0,0)
					 "draw",
#else
					 "expose_event",
#endif
					 G_CALLBACK(s_sym_SymbolMap_draw),
					 static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_areaCurrentSym),
#if GTK_CHECK_VERSION(3,0,0)
					 "draw",
#else
					 "expose_event",
#endif
					   G_CALLBACK(s_Symbolarea_draw),
					   static_cast<gpointer>(this));
        // VScrollbar events
	g_signal_connect(G_OBJECT(m_vadjust),
	                                 "value-changed",
					 G_CALLBACK(s_new_row),
		                         static_cast<gpointer>(this));
        
	// Mouse wheel events
	g_signal_connect(G_OBJECT(m_SymbolMap),
	                                 "scroll_event",
					 G_CALLBACK(s_scroll_event),
		                         static_cast<gpointer>(this));

}

void XAP_UnixDialog_Insert_Symbol::_setScrolledWindow (void)
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
        UT_return_if_fail(iDrawSymbol);
	UT_return_if_fail(m_vadjust);

	UT_uint32 rows = iDrawSymbol->getSymbolRows () + 1;
	rows = (rows <= 7? 1: rows - 7);
	gtk_adjustment_set_lower(m_vadjust, 0);
	gtk_adjustment_set_upper(m_vadjust, gdouble (rows));
	gtk_adjustment_set_page_size(m_vadjust, 1 + rows / 7);
	gtk_adjustment_set_page_increment(m_vadjust, 1);
	gtk_adjustment_set_step_increment(m_vadjust, 1);
	gtk_adjustment_set_value(m_vadjust, 0);
}
