/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuière
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

#import "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_CocoaFont.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Draw_Symbol.h"
#include "xap_CocoaDlg_Insert_Symbol.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

static UT_uint32 xap_CocoaDlg_Insert_Symbol_first = 0;
static UT_UCSChar m_CurrentSymbol;
static UT_UCSChar m_PreviousSymbol;

XAP_Dialog * XAP_CocoaDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Insert_Symbol * p = new XAP_CocoaDialog_Insert_Symbol(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Insert_Symbol::XAP_CocoaDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id dlgid)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,dlgid),
		m_pGRPreview(NULL),
		m_pGRGrid(NULL)
{
}

XAP_CocoaDialog_Insert_Symbol::~XAP_CocoaDialog_Insert_Symbol(void)
{
	DELETEP(m_pGRGrid);
	DELETEP(m_pGRPreview);
}


/*****************************************************************/

#if 0
static void s_sym_SymbolMap_exposed(GtkWidget * widget, GdkEvent * e, XAP_CocoaDialog_Insert_Symbol * dlg)
{
	UT_ASSERT( dlg);
	dlg->SymbolMap_exposed();
}


static void s_Symbolarea_exposed(GtkWidget * widget, GdkEvent * e, XAP_CocoaDialog_Insert_Symbol * dlg)
{
	UT_ASSERT( dlg);
	dlg->Symbolarea_exposed();
}

static gboolean  s_SymbolMap_clicked(GtkWidget * widget, GdkEvent * e, XAP_CocoaDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->SymbolMap_clicked( e );
        return FALSE; 
}

static gboolean  s_CurrentSymbol_clicked(GtkWidget * widget, GdkEvent * e, XAP_CocoaDialog_Insert_Symbol * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->CurrentSymbol_clicked( e );
        return FALSE; 
}

#endif

#if 0
static gboolean s_keypressed(GtkWidget * widget, GdkEventKey * e,  XAP_CocoaDialog_Insert_Symbol * dlg)
{
	dlg->Key_Pressed( e );
	return TRUE;
}

#endif


#if 0
// TODO: there must be a better way of doing this
// TODO: it just seems so wasteful to have a callback
// TODO: registered for every time the mouse moves over a widget
static void s_motion_event(GtkWidget * /* widget */,
			   GdkEventMotion *evt,
			   XAP_CocoaDialog_Insert_Symbol *dlg)
{
        UT_DEBUGMSG(("DOM: motion event\n"));
        dlg->Motion_event(evt);
}

void XAP_CocoaDialog_Insert_Symbol::Motion_event(GdkEventMotion *e)
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

void XAP_CocoaDialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
}

void XAP_CocoaDialog_Insert_Symbol::activate(void)
{
	UT_ASSERT(m_dlg);
	NSWindow* window = [m_dlg window];
	ConstructWindowName();
	NSString* str = [[NSString alloc] initWithUTF8String:m_WindowName];
	[window setTitle:str];
	[str release];
	[window orderFront:m_dlg];
}

void   XAP_CocoaDialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_ASSERT(m_dlg);
	ConstructWindowName();
	NSString* str = [[NSString alloc] initWithUTF8String:m_WindowName];
	[[m_dlg window] setTitle:str];
	[str release];
}


void XAP_CocoaDialog_Insert_Symbol::runModeless(XAP_Frame * pFrame)
{
	NSWindow* window;
	// First see if the dialog is already running
	UT_sint32 sid =(UT_sint32)  getDialogId();
	  
	// Build the window's widgets and arrange them
	m_dlg = [[XAP_CocoaDlg_Insert_SymbolController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	
	// Save dialog the ID number and pointer to the Dialog
	m_pApp->rememberModelessId( sid,  (XAP_Dialog_Modeless *) m_pDialog);
	window = [m_dlg window];

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area

	// make a new Cocoa GC
	DELETEP (m_pGRGrid);
	m_pGRGrid = new GR_CocoaGraphics([m_dlg grid], m_pApp);

	// let the widget materialize
	NSSize size = [[m_dlg grid] bounds].size;
	_createSymbolFromGC(m_pGRGrid,
						lrintf(size.width),
						lrintf(size.height));

	// make a new Cocoa GC
	DELETEP (m_pGRPreview);
	m_pGRPreview = new GR_CocoaGraphics([m_dlg preview], m_pApp);
		
	// let the widget materialize
	size = [[m_dlg preview] bounds].size;
	_createSymbolareaFromGC(m_pGRPreview,
							lrintf(size.width),
							lrintf(size.height));

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	// We use this code to insert the default font name into to static
	// variable "m_Insert_Symbol_font" the first time this dialog is
	// called. Afterwards it is just whatever was left from the last
	// call.

	if ( xap_CocoaDlg_Insert_Symbol_first == 0)
	{
		iDrawSymbol->setSelectedFont( (char *) DEFAULT_COCOA_SYMBOL_FONT);
		m_CurrentSymbol = ' ';
		m_PreviousSymbol = ' ';
		xap_CocoaDlg_Insert_Symbol_first = 1;
	}

	// Show the top level dialog
	[window orderFront:m_dlg];

        // Put the current font in the entry box
	const char* iSelectedFont = iDrawSymbol->getSelectedFont();
	[m_dlg _selectFontByName:iSelectedFont];
	New_Font();
	
	// Show the Previously selected symbol

	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// return to ap_Editmethods and wait for something interesting
	// to happen.
}

void XAP_CocoaDialog_Insert_Symbol::event_OK(void)
{
	m_Inserted_Symbol = m_CurrentSymbol;
	_onInsertButton();
}

void XAP_CocoaDialog_Insert_Symbol::event_Cancel(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;

	modeless_cleanup();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void XAP_CocoaDialog_Insert_Symbol::event_CloseWindow(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;

	modeless_cleanup();
	[m_dlg release];
	m_dlg = nil;
}

void XAP_CocoaDialog_Insert_Symbol::SymbolMap_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->draw();
	/*
	    Need this to see the blue square after an expose event
	*/
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_CocoaDialog_Insert_Symbol::Symbolarea_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

//
// This function allows the symbol to be selected via the keyboard
//

void XAP_CocoaDialog_Insert_Symbol::Key_Pressed(NSEvent * e)
{
	NSString *characters = [e characters];
	int uLength = [characters length];
	for (int ind = 0; ind < uLength; ind++)
	{
		int move = 0;
		UT_uint32 charData = [characters characterAtIndex:ind]; // can we go faster than that ?

		switch (charData)
		{
		case NSUpArrowFunctionKey:
			move = -32;
			break;
		case NSDownArrowFunctionKey:
			move = 32;
			break;
		case NSLeftArrowFunctionKey:
			move = -1;
			break;
		case NSRightArrowFunctionKey:
			move = 1;
			break;
		case 0x0d:
	//	        g_signal_emit_stop_by_name((G_OBJECT(m_windowMain)),
	//					     "key_press_event");
			event_OK();
			return;
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
	
	//		g_signal_emit_stop_by_name((G_OBJECT(m_windowMain)),
	//									"key_press_event");
		}
	}
}

void XAP_CocoaDialog_Insert_Symbol::SymbolMap_clicked(NSEvent * event)
{
	NSView*	hitView = [m_dlg grid];
	NSPoint pt = [event locationInWindow];

	pt = [hitView convertPoint:pt fromView:nil];
	pt.y = [hitView bounds].size.height - pt.y;
	
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = iDrawSymbol->calcSymbol(lrintf(pt.x), lrintf(pt.y));
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	// double click should also insert the symbol
	if([event clickCount] >= 2) {
	    event_OK();
	}
}


void XAP_CocoaDialog_Insert_Symbol::New_Font(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	
/*
  Extract the new font string from the combo box, update the current symbol
  font and display the new set of symbols to choose from.

  The text extraction code was stolen from ev_GnomeCocoaToolbar.
*/

	const char* buffer = [[m_dlg _selectedFont] UTF8String];

	iDrawSymbol->setSelectedFont(buffer);
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}


void XAP_CocoaDialog_Insert_Symbol::destroy(void)
{
	modeless_cleanup();

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


/*****************************************************************/


void XAP_CocoaDialog_Insert_Symbol::CurrentSymbol_clicked()
{
	event_OK();
}


void XAP_CocoaDialog_Insert_Symbol::_fillComboboxWithFonts (NSComboBox* combo)
{
	id obj;
	NSString* str = nil;
	NSArray * list = [[[NSFontManager sharedFontManager] availableFontFamilies] 
						sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
	NSEnumerator* iter = [list objectEnumerator];

	[combo removeAllItems];
	while (obj = [iter nextObject]) {
		[combo addItemWithObjectValue:obj];
	}

	str = [[NSString alloc] initWithUTF8String:DEFAULT_COCOA_SYMBOL_FONT];
	[combo selectItemWithObjectValue:str];
	[str release];
}

//***************************************************************

@interface SymbolEventDelegate : NSObject <XAP_MouseEventDelegate>
{
	XAP_CocoaDialog_Insert_Symbol*	_xap;
}
- (void)setXAPOwner:(XAP_CocoaDialog_Insert_Symbol *)owner;
@end

//***************************************************************

@implementation SymbolEventDelegate

- (void)setXAPOwner:(XAP_CocoaDialog_Insert_Symbol *)owner
{
	_xap = owner;
}

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
}

- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
}

- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	_xap->CurrentSymbol_clicked();
}

@end

//***************************************************************

@interface CharMapEventDelegate : NSObject <XAP_MouseEventDelegate>
{
	XAP_CocoaDialog_Insert_Symbol*	_xap;
}
- (void)setXAPOwner:(XAP_CocoaDialog_Insert_Symbol *)owner;
@end


//***************************************************************

@implementation CharMapEventDelegate

- (void)setXAPOwner:(XAP_CocoaDialog_Insert_Symbol *)owner
{
	_xap = owner;
}

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
	_xap->SymbolMap_clicked(theEvent);
}

- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
	_xap->SymbolMap_clicked(theEvent);
}

- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	_xap->SymbolMap_clicked(theEvent);
}

@end

//***************************************************************

@implementation XAP_CocoaDlg_Insert_SymbolController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_Insert_Symbol"];
	return self;
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_Insert_Symbol*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = nil;
}


- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	LocalizeControl(_addBtn, pSS, XAP_STRING_ID_DLG_Insert);
	LocalizeControl(_closeBtn, pSS, XAP_STRING_ID_DLG_Close);

	CharMapEventDelegate* charMapDelegate;
	charMapDelegate = [[CharMapEventDelegate alloc] init];
	[charMapDelegate setXAPOwner:_xap];
	[_grid setEventDelegate:charMapDelegate];
	[charMapDelegate release];

	SymbolEventDelegate* symbolDelegate;
	symbolDelegate = [[SymbolEventDelegate alloc] init];
	[symbolDelegate setXAPOwner:_xap];
	[_preview setEventDelegate:symbolDelegate];
	[symbolDelegate release];

	[_fontCombo addItemsWithObjectValues:[[[NSFontManager sharedFontManager] availableFontFamilies] 
			sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)]];
}


- (IBAction)addAction:(id)sender
{
	_xap->event_OK();
}

- (IBAction)closeAction:(id)sender
{
	_xap->event_Cancel();
}

- (IBAction)fontSelectAction:(id)sender
{
	_xap->New_Font();
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_CloseWindow();
}

- (XAP_CocoaNSView*)grid
{
	return _grid;
}

- (XAP_CocoaNSView*)preview;
{
	return _preview;
}

- (void)_selectFontByName:(const char*)name
{
	NSString* string = [[NSString alloc] initWithUTF8String:name];
	[_fontCombo selectItemWithObjectValue:string];
	[string release];
}


- (NSString*)_selectedFont
{
	return [_fontCombo stringValue];
}


@end
