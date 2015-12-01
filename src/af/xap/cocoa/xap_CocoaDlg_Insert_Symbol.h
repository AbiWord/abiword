/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2003 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifndef XAP_COCOADIALOG_INSERT_SYMBOL_H
#define XAP_COCOADIALOG_INSERT_SYMBOL_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_Insert_Symbol.h"

#import "xap_CocoaDialog_Utilities.h"

class XAP_CocoaFrame;
class XAP_CocoaDialog_Insert_Symbol;

/*****************************************************************/

@interface XAP_CocoaDlg_Insert_SymbolController
: NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate, NSTableViewDataSource>
{
	IBOutlet NSButton *			oAdd;
	IBOutlet NSButton *			oPreview;
	IBOutlet NSButton *			oRemapGlyphs;

	IBOutlet NSPopUpButton *	oFont;
	IBOutlet NSPopUpButton *	oFontFamily;

	IBOutlet NSTableView *		oSymbolTable;

	NSMutableArray *			m_FontList;
	NSFont *					m_CurrentFont;
	NSString *					m_CurrentFontName;
	NSString *					m_OffsetString[14];
	NSString *					m_SymbolString[224];
	NSString *					m_Remap_String[224];
	UT_sint32					m_SymbolWidth[224];

	int							m_Symbol_lo;
	int							m_Symbol_hi;

	BOOL						m_bRemapGlyphs;

	XAP_CocoaDialog_Insert_Symbol *		_xap;
}
- (IBAction)aSingleClick:(id)sender;
- (IBAction)aDoubleClick:(id)sender;
- (IBAction)aAdd:(id)sender;
- (IBAction)aFont:(id)sender;
- (IBAction)aFontFamily:(id)sender;
- (IBAction)aRemapGlyphs:(id)sender;

- (id)initFromNib;
- (void)dealloc;
- (void)setXAPOwner:(XAP_Dialog *)owner;
- (void)discardXAP;
- (void)windowDidLoad;
- (void)windowWillClose:(NSNotification *)aNotification;
- (void)windowToFront;

- (void)fontFamilyDidChange;
- (void)recalculateSymbolWidths;

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

/* NSTableView delegate methods
 */
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(int)rowIndex;
@end


class XAP_CocoaDialog_Insert_Symbol : public XAP_Dialog_Insert_Symbol
{
public:
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	XAP_CocoaDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_Insert_Symbol(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			notifyActiveFrame(XAP_Frame * pFrame);
	virtual void			notifyCloseFrame(XAP_Frame * ) { };
	virtual void			activate(void);
	virtual void			destroy(void);

	void					insertSymbol(const char * fontFamilyName, UT_UCS4Char symbol);
	void					windowClosed(void);

private:
	XAP_CocoaDlg_Insert_SymbolController *	m_dlg;
};

#endif /* XAP_COCOADIALOG_INSERT_SYMBOL_H */
