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

	PtArg_t args[10];
	int     n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
				UT_XML_transNoAmpersands(pSS->getValue(XAP_STRING_ID_DLG_Insert_SymbolTitle)), 0); 
	windowInsertS = PtCreateWidget(PtWindow, NULL, n, args);

	//Create a vertical group to contain the font selector, 
	// raw drawing area and then a horizontal group of buttons
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vboxInsertS = PtCreateWidget(PtGroup, windowInsertS, n, args);

	// First put in a combo box so the user can select fonts
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	fontcombo = PtCreateWidget(PtComboBox, windowInsertS, n, args);
	const char *sz = "Symbol";
	PtListAddItems(fontcombo, &sz, 1, 0);

	// Then put the main symbol area in the center vertically 
	// *** Code Stolen from the preview widget ***
	{
		// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 608, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 147, 0);
		PtWidget_t *symgroup = PtCreateWidget(PtGroup, windowInsertS, n, args);

		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 608, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 147, 0);
		SymbolMap = PtCreateWidget(PtRaw, symgroup, n, args);

		// Enable button press events
		//gtk_widget_add_events(SymbolMap, GDK_BUTTON_PRESS_MASK);
   	}
	
	// Then horizontally group the OK, Preview, Cancel widgets
	n = 0;
	hboxInsertS = PtCreateWidget(PtGroup, windowInsertS, n, args);

	n = 0;
 	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, hboxInsertS, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_ok_clicked, this);

	// *** Code Stolen from the preview widget again! ***
	{
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 60, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 45, 0);
		PtWidget_t *symgroup = PtCreateWidget(PtGroup, hboxInsertS, n, args);

		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 60, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 45, 0);
		areaCurrentSym = PtCreateWidget(PtRaw, symgroup, n, args);
   	}

	n = 0;
 	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hboxInsertS, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowInsertS;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_SymbolMap = 	SymbolMap;
	m_fontcombo = fontcombo;
	m_areaCurrentSym = areaCurrentSym;

	return windowInsertS;
}


