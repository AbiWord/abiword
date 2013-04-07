/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuière
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

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#import "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaCairoGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaFrame.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Draw_Symbol.h"
#include "xap_CocoaDlg_Insert_Symbol.h"

#include "fv_View.h"

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Insert_Symbol * p = new XAP_CocoaDialog_Insert_Symbol(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Insert_Symbol::XAP_CocoaDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_Insert_Symbol(pDlgFactory,dlgid)
{
	// 
}

XAP_CocoaDialog_Insert_Symbol::~XAP_CocoaDialog_Insert_Symbol(void)
{
	// 
}

void XAP_CocoaDialog_Insert_Symbol::runModal(XAP_Frame * /*pFrame*/)
{
	// 
}

void XAP_CocoaDialog_Insert_Symbol::runModeless(XAP_Frame * /*pFrame*/)
{
	/* First see if the dialog is already running
	 */
	UT_sint32 sid = (UT_sint32) getDialogId();

	/* Build the window's widgets and arrange them
	 */
	m_dlg = [[XAP_CocoaDlg_Insert_SymbolController alloc] initFromNib];

	[m_dlg setXAPOwner:this];
	[m_dlg windowToFront];

	/* Save dialog the ID number and pointer to the Dialog
	 */
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
}

void XAP_CocoaDialog_Insert_Symbol::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	// ...
}

void XAP_CocoaDialog_Insert_Symbol::activate(void)
{
	UT_ASSERT(m_dlg);
	if (!m_dlg)
		return;

	[m_dlg windowToFront];
}

void XAP_CocoaDialog_Insert_Symbol::destroy(void)
{
	modeless_cleanup();

	if (m_dlg)
		{
			[m_dlg close];
			[m_dlg release];
			m_dlg = nil;
		}
}

void XAP_CocoaDialog_Insert_Symbol::insertSymbol(const char * fontFamilyName, UT_UCS4Char symbol)
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	AV_View * view = [pController currentView];

	if (!view)
	{
		view = [pController previousView];
	}
	if (view)
	{
		if (fontFamilyName)
		{
			if (m_pListener)
			{
				m_pListener->setView(view);
				m_pListener->insertSymbol(symbol, (char *) fontFamilyName);
			}
		}
		else // special case - don't insert as symbol; just use current font
		{
			FV_View * pView = static_cast<FV_View *>(view);
			pView->cmdCharInsert(&symbol, 1);
		}
	}
}

void XAP_CocoaDialog_Insert_Symbol::windowClosed(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;

	modeless_cleanup();

	[m_dlg release];
	m_dlg = nil;
}

//***************************************************************

static unichar s_remap[224] = {
	/* TODO: Complete this table & verify...
	 */
	  0x0020, 
	/*0x00A0,*/0x0021, 0x2200, 0x0023,  0x2203,  0x0025, 0x0026, 0x220B,  0x0028, 0x0029, 0x2217, 0x002B,  0x002C,  0x2212, 0x002E, 0x002F, 
	  0x0030,  0x0031, 0x0032, 0x0033,  0x0034,  0x0035, 0x0036, 0x0037,  0x0038, 0x0039, 0x003A, 0x003B,  0x003C,  0x003D, 0x003E, 0x003F, 
	  0x2245,  0x0391, 0x0392, 0x03A7,  0x0394, 
	                                  /*0x2206,*/0x0395, 0x03A6, 0x0393,  0x0397, 0x0399, 0x03D1, 0x039A,  0x039B,  0x039C, 0x039D, 0x039F, 
	  0x03A0,  0x0398, 0x03A1, 0x03A3,  0x03A4,  0x03A5, 0x03C2, 0x03A9, 
	                                                           /*0x2126,*/0x039E, 0x03A8, 0x0396, 0x005B,  0x2234,  0x005D, 0x22A5, 0x005F, 
	  0xF8E5,  0x03B1, 0x03B2, 0x03C7,  0x03B4,  0x03B5, 0x03C6, 0x03B3,  0x03B7, 0x03B9, 0x03D5, 0x03BA,  0x03BB,/*0x00B5,*/ 
	                                                                                                                0x03BC, 0x03BD, 0x03BF, 
	  0x03C0,  0x03B8, 0x03C1, 0x03C3,  0x03C4,  0x03C5, 0x03D6, 0x03C9,  0x03BE, 0x03C8, 0x03B6, 0x007B,  0x007C,  0x007D, 0x223C
	                                                                                                                              , 0x007f,

	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,

	0x20AC, 0x03D2, 0x2032, 0x2264,/*0x2044,*/ 
	                                 0x2215, 0x221E, 0x0192, 0x2663,  0x2666, 0x2665, 0x2660, 0x2194,  0x2190, 0x2191, 0x2192, 0x2193, 
	0x00B0, 0x00B1, 0x2033, 0x2265,  0x00D7, 0x221D, 0x2202, 0x2022,  0x00F7, 0x2260, 0x2261, 0x2248,  0x2026, 0xF8E6, 0xF8E7, 0x21B5, 
	0x2135, 0x2111, 0x211C, 0x2118,  0x2297, 0x2295, 0x2205, 0x2229,  0x222A, 0x2283, 0x2287, 0x2284,  0x2282, 0x2286, 0x2208, 0x2209, 
	0x2220, 0x2207, 0x00AE, 0x00A9,  0x2122, 0x220F, 0x221A, 0x22C5,  0x00AC, 0x2227, 0x2228, 0x21D4,  0x21D0, 0x21D1, 0x21D2, 0x21D3, 
	0x25CA, 0x3008, 0x00AE, 0x00A9,  0x2122, 0x2211, 0xF8EB, 0xF8EC,  0xF8ED, 0xF8EE, 0xF8EF, 0xF8F0,  0xF8F1, 0xF8F2, 0xF8F3, 0xF8F4, 
0xF8FF,     0x3009, 0x222B, 0x2320,  0xF8F5, 0x2321, 0xF8F6, 0xF8F7,  0xF8F8, 0xF8F9, 0xF8FA, 0xF8FB,  0xF8FC, 0xF8FD, 0xF8FE
	                                                                                                                         , 0x00ff
};

@implementation XAP_CocoaDlg_Insert_SymbolController

- (id)initFromNib
{
	if (![super initWithWindowNibName:@"xap_CocoaDlg_Insert_Symbol"]) {
		return nil;
	}
	m_FontList = 0;
	m_CurrentFont = 0;
	m_CurrentFontName = 0;

	m_Symbol_lo = 0;
	m_Symbol_hi = 0;

	char hex[14] = { '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int i;
	unichar uc[3];

	uc[1] = (unichar) ((unsigned char) 'x');
	uc[2] = 0;
	for (i = 0; i < 14; i++)
	{
		uc[0] = (unichar) ((unsigned char) hex[i]);
		m_OffsetString[i] = [[NSString alloc] initWithCharacters:uc length:2];
	}

	uc[1] = 0;
	for (i = 0; i < 224; i++)
	{
		uc[0] = (unichar) (i + 32);
		m_SymbolString[i] = [[NSString alloc] initWithCharacters:uc length:1];
		uc[0] = s_remap[i];
		m_Remap_String[i] = [[NSString alloc] initWithCharacters:uc length:1];
		m_SymbolWidth[i] = 0;
	}

	NSArray * pAvailableFontFamilies = [[NSFontManager sharedFontManager] availableFontFamilies];

	unsigned count = [pAvailableFontFamilies count];
	if (count)
	{
		m_FontList = [[NSMutableArray alloc] initWithCapacity:count];
		if (m_FontList)
		{
			for (unsigned ff = 0; ff < count; ff++)
			{
				NSString * pFontFamily = [pAvailableFontFamilies objectAtIndex:ff];

				/* const char * szFF = [pFontFamily UTF8String]; */

				if (true /* (*szFF != '.') && (*szFF != '#') */) // cf. Bug 6638
				{
					[m_FontList addObject:pFontFamily];
				}
			}
			if ([m_FontList count])
			{
				[m_FontList sortUsingSelector:@selector(compare:)];
			}
			else
			{
				UT_DEBUGMSG(("XAP_CocoaDlg_Insert_Symbol -initFromNib: no usable font families?\n"));

				[m_FontList release];
				m_FontList = 0;
			}
		}
	}
	else
	{
		UT_DEBUGMSG(("XAP_CocoaDlg_Insert_Symbol -initFromNib: no available font families?\n"));
	}
	if (!m_FontList)
	{
		[self dealloc];
		return nil;
	}
	return self;
}

- (void)dealloc
{
	int i;

	for (i = 0; i < 14; i++)
		{
			if ( m_OffsetString[i])
				[m_OffsetString[i] release];
			m_OffsetString[i] = 0;
		}
	for (i = 0; i < 224; i++)
		{
			if ( m_SymbolString[i])
				[m_SymbolString[i] release];
			m_SymbolString[i] = 0;

			if ( m_Remap_String[i])
				[m_Remap_String[i] release];
			m_Remap_String[i] = 0;
		}
	if (m_FontList)
		{
			[m_FontList release];
			m_FontList = 0;
		}
	if (m_CurrentFont)
		{
			[m_CurrentFont release];
			m_CurrentFont = 0;
		}
	if (m_CurrentFontName)
		{
			[m_CurrentFontName release];
			m_CurrentFontName = 0;
		}
	[super dealloc];
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

	std::string sTitle;

	if (pSS->getValueUTF8(XAP_STRING_ID_DLG_Insert_SymbolTitle, sTitle))
		{
			[[self window] setTitle:[NSString stringWithUTF8String:(sTitle.c_str())]];
		}

	LocalizeControl(oAdd,      pSS, XAP_STRING_ID_DLG_Insert);
//	LocalizeControl(_closeBtn, pSS, XAP_STRING_ID_DLG_Close);

	[oRemapGlyphs setState:NSOnState];
	m_bRemapGlyphs = YES;

	[oFontFamily removeAllItems];
	[oFontFamily addItemsWithTitles:m_FontList];

	int index = [oFontFamily indexOfItemWithTitle:@"Symbol"];
	if (index >= 0)
		{
			[oFontFamily selectItemAtIndex:index];
		}
	[self fontFamilyDidChange];

	[oPreview setFont:[NSFont fontWithName:m_CurrentFontName size:36.0f]];
	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[m_Symbol_hi * 16 + m_Symbol_lo]];
	else
		[oPreview setTitle:m_SymbolString[m_Symbol_hi * 16 + m_Symbol_lo]];

	[oSymbolTable setDelegate:self];
	[oSymbolTable setDataSource:self];
	[oSymbolTable setDoubleAction:@selector(aDoubleClick:)];
	[oSymbolTable reloadData];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	[oSymbolTable setDelegate:nil];
	[oSymbolTable setDataSource:nil];
	[oSymbolTable setTarget:nil];

	_xap->windowClosed();
}

- (void)windowToFront
{
	[[self window] makeKeyAndOrderFront:self];
	[[self window] makeFirstResponder:oSymbolTable];
}

- (IBAction)aSingleClick:(id)sender
{
	UT_UNUSED(sender);
	int columnIndex = [oSymbolTable clickedColumn];
	int    rowIndex = [oSymbolTable clickedRow   ];

	if (columnIndex == 0)
		return;

	int index = rowIndex * 16 + (columnIndex - 1);
	if (m_SymbolWidth[index] == 0)
		return;

	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[index]];
	else
		[oPreview setTitle:m_SymbolString[index]];

	[oSymbolTable setNeedsDisplayInRect:[oSymbolTable frameOfCellAtColumn:(m_Symbol_lo + 1) row:m_Symbol_hi]];

	m_Symbol_lo = columnIndex - 1;
	m_Symbol_hi =    rowIndex;

	[oSymbolTable setNeedsDisplayInRect:[oSymbolTable frameOfCellAtColumn:(m_Symbol_lo + 1) row:m_Symbol_hi]];
	[oSymbolTable displayIfNeeded];
}

- (IBAction)aDoubleClick:(id)sender
{
	int columnIndex = [oSymbolTable clickedColumn];
	int    rowIndex = [oSymbolTable clickedRow   ];

	if (columnIndex == 0)
		return;

	int index = rowIndex * 16 + (columnIndex - 1);
	if (m_SymbolWidth[index] == 0)
		return;

	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[index]];
	else
		[oPreview setTitle:m_SymbolString[index]];

	[oSymbolTable setNeedsDisplayInRect:[oSymbolTable frameOfCellAtColumn:(m_Symbol_lo + 1) row:m_Symbol_hi]];

	m_Symbol_lo = columnIndex - 1;
	m_Symbol_hi =    rowIndex;

	[oSymbolTable setNeedsDisplayInRect:[oSymbolTable frameOfCellAtColumn:(m_Symbol_lo + 1) row:m_Symbol_hi]];
	[oSymbolTable displayIfNeeded];

	[self aAdd:sender];
}

- (IBAction)aAdd:(id)sender
{
	UT_UNUSED(sender);
	int index = m_Symbol_hi * 16 + m_Symbol_lo;
	if (m_SymbolWidth[index] == 0)
		return;

	if (m_bRemapGlyphs)
	{
		UT_UCS4Char remapped_index = static_cast<UT_UCS4Char>(s_remap[index]);

		if ((remapped_index & static_cast<UT_UCS4Char>(0xff)) == remapped_index)
		{
			/* okay, this is awkward; the font remapping will treat characters <= 0xff specially,
			 * but for a very few cases we're actually mapping within this range.
			 */
			_xap->insertSymbol(NULL, remapped_index);
			return;
		}
	}
	_xap->insertSymbol([[oFontFamily titleOfSelectedItem] UTF8String], static_cast<UT_UCS4Char>(m_bRemapGlyphs ? s_remap[index] : (index + 32)));
}

- (IBAction)aFontFamily:(id)sender
{
	UT_UNUSED(sender);
	[self fontFamilyDidChange];

	[oPreview setFont:[NSFont fontWithName:m_CurrentFontName size:36.0f]];
	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[m_Symbol_hi * 16 + m_Symbol_lo]];
	else
		[oPreview setTitle:m_SymbolString[m_Symbol_hi * 16 + m_Symbol_lo]];

	[oSymbolTable setNeedsDisplay:YES];
	[oSymbolTable displayIfNeeded];
}

- (IBAction)aFont:(id)sender
{
	UT_UNUSED(sender);
	NSArray * fonts = [[NSFontManager sharedFontManager] availableMembersOfFontFamily:[oFontFamily titleOfSelectedItem]];

	NSArray * font = [fonts objectAtIndex:[oFont indexOfSelectedItem]];

	if (m_CurrentFontName)
		{
			[m_CurrentFontName release];
			m_CurrentFontName = 0;
		}
	m_CurrentFontName = [font objectAtIndex:0];
	[m_CurrentFontName retain];

	if (m_CurrentFont)
		{
			[m_CurrentFont release];
			m_CurrentFont = 0;
		}
	m_CurrentFont = [NSFont fontWithName:m_CurrentFontName size:12.0f];
	[m_CurrentFont retain];

	[self recalculateSymbolWidths];

	[oPreview setFont:[NSFont fontWithName:m_CurrentFontName size:36.0f]];
	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[m_Symbol_hi * 16 + m_Symbol_lo]];
	else
		[oPreview setTitle:m_SymbolString[m_Symbol_hi * 16 + m_Symbol_lo]];

	[oSymbolTable setNeedsDisplay:YES];
	[oSymbolTable displayIfNeeded];
}

- (void)fontFamilyDidChange
{
	[oFont removeAllItems];

	NSArray * fonts = [[NSFontManager sharedFontManager] availableMembersOfFontFamily:[oFontFamily titleOfSelectedItem]];

	unsigned int count = [fonts count];

	for (unsigned int i = 0; i < count; i++)
		{
			NSArray * font = [fonts objectAtIndex:i];

			if (i == 0)
				{
					if (m_CurrentFontName)
						{
							[m_CurrentFontName release];
							m_CurrentFontName = 0;
						}
					m_CurrentFontName = [font objectAtIndex:0];
					[m_CurrentFontName retain];
				}
			[oFont addItemWithTitle:[font objectAtIndex:1]];
		}
	[oFont selectItemAtIndex:0];

	if (m_CurrentFont)
		{
			[m_CurrentFont release];
			m_CurrentFont = 0;
		}
	m_CurrentFont = [NSFont fontWithName:m_CurrentFontName size:12.0f];
	[m_CurrentFont retain];

	[self recalculateSymbolWidths];
}

- (IBAction)aRemapGlyphs:(id)sender
{
	UT_UNUSED(sender);
	m_bRemapGlyphs = ([oRemapGlyphs state] == NSOnState) ? YES : NO;

	[self recalculateSymbolWidths];

	[oPreview setFont:[NSFont fontWithName:m_CurrentFontName size:36.0f]];
	if (m_bRemapGlyphs)
		[oPreview setTitle:m_Remap_String[m_Symbol_hi * 16 + m_Symbol_lo]];
	else
		[oPreview setTitle:m_SymbolString[m_Symbol_hi * 16 + m_Symbol_lo]];

	[oSymbolTable setNeedsDisplay:YES];
	[oSymbolTable displayIfNeeded];
}

- (void)recalculateSymbolWidths
{
	/* should have ns-font & ps-font-name at this point; calculate Abi-font-widths, though [TODO: FIXME??]
	 */
	// XAP_CocoaFont Font(m_CurrentFont); // this is causing a crash

	for (int i = 0; i < 224; i++)
	{
		//UT_UCS4Char ucs4 = static_cast<UT_UCS4Char>(m_bRemapGlyphs ? s_remap[i] : (i + 32));

		m_SymbolWidth[i] = 1; // Font.measureUnremappedCharForCache(ucs4);
	}
}

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	UT_UNUSED(aTableView);
	return 14;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	NSString * identifier = [aTableColumn identifier];

	const char * cid = [identifier UTF8String];

	NSTextFieldCell * cell = (NSTextFieldCell *) [aTableColumn dataCell];

	if (*cid == '-')
		{
			[cell setFont:[NSFont labelFontOfSize:12.0f]];

			[cell setDrawsBackground:YES];
			[cell setBackgroundColor:[NSColor controlHighlightColor]];

			if (m_OffsetString[rowIndex])
				return m_OffsetString[rowIndex];
			return @"?x";
		}

	if (m_CurrentFont)
		[cell setFont:m_CurrentFont];
	else
		[cell setFont:[NSFont labelFontOfSize:12.0f]];

	int columnIndex;

	if (((*cid) >= '0') && ((*cid) <= '9'))
		columnIndex = (int) ((*cid) - '0');
	else
		columnIndex = (int) ((*cid) - 'A') + 10;

	if ((columnIndex == m_Symbol_lo) && (rowIndex == m_Symbol_hi))
		{
			[cell setDrawsBackground:YES];
			[cell setBackgroundColor:[NSColor controlHighlightColor]];
		}
	else
		{
			[cell setDrawsBackground:NO];
		}

	int index = rowIndex * 16 + columnIndex;

	if (m_SymbolWidth[index])
		return m_bRemapGlyphs ? m_Remap_String[index] : m_SymbolString[index];
	return @"";
}

/* NSTableView delegate methods
 */
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aCell);
	UT_UNUSED(aTableColumn);
	UT_UNUSED(rowIndex);
	// ...
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(rowIndex);
	return NO;
}

@end
