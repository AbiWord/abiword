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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_BeOSDlg_Insert_Symbol.h"

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Insert_Symbol * p = new XAP_BeOSDialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_BeOSDialog_Insert_Symbol::XAP_BeOSDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id)
{
}

XAP_BeOSDialog_Insert_Symbol::~XAP_BeOSDialog_Insert_Symbol(void)
{
}

void XAP_BeOSDialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

/*
	4.	Replace this useless comment with specific instructions to 
		whoever's porting your dialog so they know what to do.
		Skipping this step may not cost you any donuts, but it's 
		rude.  

OK here goes. Look at xap_UnixDlg_Insert_Symbol.cpp.

Firstly there are local static wrapper functions used to connect GUI events to
their correct class managers. These are:


static void s_ok_clicked(GtkWidget * widget,...
static void s_cancel_clicked(GtkWidget * widget,... 
static void s_sym_SymbolMap_exposed(GtkWidget * widget,.. 
static void s_Symbolarea_exposed(GtkWidget * widget,...
static void s_SymbolMap_clicked(GtkWidget * widget,...
static void s_new_font(GtkWidget * widget,...
static gboolean s_keypressed(GtkWidget * widget,...
static void s_delete_clicked(GtkWidget * /* widget * /,...

I hope their use is obvious from there names. 
Your platform may need these too.I know nothing but gtk/gnome though.

Then there are class specific handlers of the GUI events.

void XAP_UnixDialog_Insert_Symbol::event_OK(void)
  //
  // Quit the dialog with signal that a symbol and font have been selected.
  // The check on the number of fonts present is used to get around the 
  // problem that asking for keypress events (particularly return) activates
  // "OK" and "Cancel" buttons too
  //


void XAP_UnixDialog_Insert_Symbol::event_Cancel(void)
  //
  // Quit the dialog returning no selected symbol.
  // The check on the number of fonts present is used to get around the 
  // problem that asking for keypress events (particularly return) activates
  // "OK" and "Cancel" buttons too
  //

void XAP_UnixDialog_Insert_Symbol::SymbolMap_exposed(void )
  //
  // Update the Symbol Map display on an exposed event.
  //

void XAP_UnixDialog_Insert_Symbol::Symbolarea_exposed(void )
  //
  // Update the selected Symbol area on an expose event
  //

void XAP_UnixDialog_Insert_Symbol::Key_Pressed( GdkEventKey * e)

  //
  // This function allows the symbol to be selected via the keyboard
  // The arrow keys are used to choose a symbol. The return key is equivalent
  // to clicking the OK button.

void XAP_UnixDialog_Insert_Symbol::SymbolMap_clicked( GdkEvent * event)
  // This updates the current character picked from the location of click event
  // within the symbol map window


void XAP_UnixDialog_Insert_Symbol::New_Font(void )

  // This recognizes that a new font has been request from the list and changes
  // the symbol map to display that font

void XAP_UnixDialog_Insert_Symbol::event_WindowDelete(void)
  // Obvious..


GtkWidget * XAP_UnixDialog_Insert_Symbol::_constructWindow(void)
  // This code is used to contruct the GUI and to connect the GUI signals to
  // the appropriate handlers.
  // I used an overall vertical box to hold the font selector, Symbol Map,
  // and selection area regions.
  // In addition there is a horizontal box to hold the buttons and selected
  // Symbol areas.
  // I used a combination list and entry box widget for the font selection
  // and there is a lot of code to populate this list.
  // There are two drawing areas. The Main one in the middle of the dialog 
  // which shows the symbols for selection and a smaller one between the OK 
  // and cancel buttons to show a magnified view of the current selected
  // symbol
 

The runModal function first constructs the GUI then enables and draws the
  two graphics areas (the Symbol Map and the Selected Symbol area). Defines
  the font and current selected symbol from either the defaults (the
Font is platform specific. You should choose an appropriate font name.)
or from the previous invocation of the dialog. It then enters the gtk_main
loop and waits from responses from the user.



	This file should *only* be used for stubbing out platforms which 
	you don't know how to implement.  When implementing a new dialog 
	for your platform, you're probably better off starting with code
	from another working dialog.  
*/	

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}



