/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
 * Copyright (C) 2004-2005, 2009 Hubert Figuiere
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_CocoaDialog_Stylist.h"


@interface StyleNode : NSObject
{
	NSString *_value;
	NSMutableArray *_children;
	int _row;
	int _col;
}
-(id)initWithValue:(const char*)value row:(int)row andCol:(int)col;
-(void)dealloc;

-(void)addChild:(id)child;
-(NSArray*)children;
-(NSString*)value;
-(int)row;
-(int)col;
@end

@implementation StyleNode

-(id)initWithValue:(const char*)value row:(int)row andCol:(int)col
{
	if(![super init]) {
		return nil;
	}
	_value = [[NSString alloc] initWithUTF8String:value];
	_row = row;
	_col = col;
	return self;
}


-(void)dealloc
{
	[_value release];
	[_children release];
	[super dealloc];
}

-(void)addChild:(id)child
{
	if (!_children) {
		_children = [[NSMutableArray alloc] init];
	}
	[_children addObject:child];
}

-(NSArray*)children
{
	return _children;
}

-(NSString*) value
{
	return _value;
}

-(int)row
{
	return _row;
}

-(int)col
{
	return _col;
}

@end

XAP_Dialog * AP_CocoaDialog_Stylist::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	return new AP_CocoaDialog_Stylist(pFactory,dlgid);
}

AP_CocoaDialog_Stylist::AP_CocoaDialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_Stylist(pDlgFactory, dlgid),
	m_dlg(nil),
	m_items(nil),
	m_bModal(false),
	m_bDialogClosed(false)
{
	m_items = [[NSMutableArray alloc] init];
}

AP_CocoaDialog_Stylist::~AP_CocoaDialog_Stylist(void)
{
	[m_items release];
}

void AP_CocoaDialog_Stylist::event_Close(void)
{
	if (m_bModal)
	{
		m_bDialogClosed = true; // we may need to destroy()
		setStyleValid(false);
		[NSApp stopModal];
	}
	else
	{
		destroy();
	}
}

void AP_CocoaDialog_Stylist::setStyleInGUI(void)
{
	UT_sint32 row,col;
	UT_UTF8String sCurStyle = *getCurStyle();
	if((getStyleTree() == NULL) || (sCurStyle.size() == 0))
	{
		updateDialog();
	}
	if(isStyleTreeChanged())
	{
		_fillTree();
	}
	getStyleTree()->findStyle(sCurStyle,row,col);
	if (row >= 0)
	{
		StyleNode * parentNode = [m_items objectAtIndex:row];
		if (col >= 0)
		{
			StyleNode * childNode = [[parentNode children] objectAtIndex:col];

			[m_dlg selectStyleNode:childNode childOf:parentNode];
		}
	}

	setStyleChanged(false);
}

void AP_CocoaDialog_Stylist::destroy(void)
{
	finalize();
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Stylist::activate(void)
{
	UT_ASSERT (m_dlg);
	[[m_dlg window] orderFront:m_dlg];
	setAllSensitivities();
}

void AP_CocoaDialog_Stylist::notifyActiveFrame(XAP_Frame */*pFrame*/)
{
    UT_ASSERT(m_dlg);
	setAllSensitivities();
}

/*!
 * Set the style in the XP layer from the selection in the GUI.
 */
void AP_CocoaDialog_Stylist::styleClicked(UT_sint32 row, UT_sint32 col)
{
	UT_DEBUGMSG(("row %d col %d clicked \n",row,col));

	UT_sint32 row_count = getStyleTree()->getNumRows();
	if ((row >= 0) && (row < row_count))
	{
		UT_sint32 col_count = getStyleTree()->getNumCols(row);
		if ((col >= 0) && (col < col_count))
		{
			UT_UTF8String sStyle;

			getStyleTree()->getStyleAtRowCol(sStyle, row, col);

			UT_DEBUGMSG(("StyleClicked row %d col %d style %s \n", (int) row, (int) col, sStyle.utf8_str()));

			setCurStyle(sStyle);
		}
	}
}

void AP_CocoaDialog_Stylist::runModal(XAP_Frame * /*pFrame*/)
{
	bool bUsingModeless = (m_dlg ? true : false);

	if (!bUsingModeless)
	{
		m_dlg = [[AP_CocoaDialog_Stylist_Controller alloc] initFromNib];

		[m_dlg setXAPOwner:this];

		_populateWindowData();
	}

	m_bDialogClosed = false;
	m_bModal = true;

	NSWindow * window = [m_dlg window];

	[window orderFront:m_dlg];

	[NSApp runModalForWindow:window];

	m_bModal = false;

	if (!bUsingModeless)
	{
		[m_dlg discardXAP];
		[m_dlg close];
		[m_dlg release];

		m_dlg = nil;
	}
	else if (m_bDialogClosed)
	{
		// the user closed the dialog...
		destroy();
	}
}

void AP_CocoaDialog_Stylist::runModeless(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_Stylist_Controller alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();
	[window orderFront:m_dlg];	
	startUpdater();
}

void  AP_CocoaDialog_Stylist::setSensitivity(bool bSens)
{
	[m_dlg setSensitivity:bSens];
}


void  AP_CocoaDialog_Stylist::event_Apply(void)
{
	if (m_bModal)
	{
		setStyleValid(true);
		[NSApp stopModal];
	}
	else
	{
		Apply();
	}
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_CocoaDialog_Stylist::_fillTree(void)
{
	int col, row;
	StyleNode *currentChild;
	
	Stylist_tree * pStyleTree = getStyleTree();
	if(pStyleTree == NULL)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	if(pStyleTree->getNumRows() == 0)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	[m_items removeAllObjects];
	UT_DEBUGMSG(("Number of rows of styles in document %d \n",pStyleTree->getNumRows()));

	std::string sTmp; 
	for(row= 0; row < pStyleTree->getNumRows();row++)
	{
		if(!pStyleTree->getNameOfRow(sTmp,row))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		currentChild = [[[StyleNode alloc] initWithValue:sTmp.c_str() row:row andCol:0] autorelease];
		if(pStyleTree->getNumCols(row) > 0)
		{
			UT_DEBUGMSG(("Adding Heading %s at row %d \n",sTmp.c_str(),row));

			[m_items addObject:currentChild];
			UT_UTF8String sTmp2;
			for(col =0 ; col < pStyleTree->getNumCols(row); col++)
			{
				if(!pStyleTree->getStyleAtRowCol(sTmp2,row,col))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				[currentChild addChild:[[[StyleNode alloc] initWithValue:sTmp2.utf8_str() row:row andCol:col] autorelease]];
				UT_DEBUGMSG(("Adding style %s at row %d col %d \n",sTmp2.utf8_str(),row,col+1));
			}
		}
		else
		{
			UT_DEBUGMSG(("Adding style %s at row %d \n",sTmp.c_str(),row));
			[m_items addObject:currentChild];
		}
	}
	[m_dlg refresh];
	setStyleTreeChanged(false);
}

void  AP_CocoaDialog_Stylist::_populateWindowData(void)
{
	_fillTree();
	setStyleInGUI();
}


@implementation AP_CocoaDialog_Stylist_Controller

- (id)initFromNib
{
	if([super initWithWindowNibName:@"ap_CocoaDialog_Stylist"]) {
		_xap = NULL; 
		_enabled = true;
	}
	return self;
}

- (void)discardXAP
{
	_xap = NULL; 
}

- (void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Stylist*>(owner);
}

- (void)windowDidLoad
{
	if (_xap) {
		NSPanel * panel = (NSPanel *) [self window];

		[panel setBecomesKeyOnlyIfNeeded:YES];

		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl(panel,		pSS, AP_STRING_ID_DLG_Stylist_Title);
		LocalizeControl(_applyBtn,	pSS, XAP_STRING_ID_DLG_Apply);

		std::string label;
		if (pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_Styles, label))
		{
			NSArray * columns = [_stylistList tableColumns];

			[[[columns objectAtIndex:0] headerCell] setStringValue:[NSString stringWithUTF8String:(label.c_str())]];
		}

		[_stylistList setDoubleAction:@selector(outlineDoubleAction:)];

		// data source and delegate for style list should be set by the Nib.
	}
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if (_xap)
		_xap->event_Close();
}

- (void)refresh
{
	[_stylistList reloadData];
}

- (void)selectStyleNode:(StyleNode *)childNode childOf:(StyleNode *)parentNode
{
	[_stylistList expandItem:parentNode];

	int row = [_stylistList rowForItem:childNode];
	if (row >= 0)
	{
		[_stylistList selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	}
	else
	{
		[_stylistList deselectAll:self];
	}
	[_applyBtn setEnabled:NO];
}

- (IBAction)applyAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Apply();
}

- (IBAction)outlineAction:(id)sender
{
	BOOL bCanApply = NO;

	if(!_enabled) {
		return;
	}
	int row = [sender selectedRow];
	if (row >= 0)
	{
		StyleNode * node = [sender itemAtRow:row];

		if (![node children])
		{
			_xap->styleClicked([node row], [node col]);
			bCanApply = YES;
		}
	}
	[_applyBtn setEnabled:bCanApply];
}

- (IBAction)outlineDoubleAction:(id)sender
{
	BOOL bCanApply = NO;

	if(!_enabled) {
		return;
	}
	int row = [sender selectedRow];
	if (row >= 0)
	{
		StyleNode * node = [sender itemAtRow:row];

		if (![node children])
		{
			_xap->styleClicked([node row], [node col]);
			_xap->event_Apply();
			bCanApply = YES;
		}
	}
	[_applyBtn setEnabled:bCanApply];
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	UT_UNUSED(outlineView);
	if (item == nil) {
		return [_xap->getItems() count];
	}
	if (![item isKindOfClass:[StyleNode class]]) {
		return 0;
	}
	return [[item children] count];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	UT_UNUSED(outlineView);
	if (![item isKindOfClass:[StyleNode class]]) {
		return NO;
	}
	return ([item children] ? YES : NO);
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
	UT_UNUSED(outlineView);
	if (item == nil) {
		return [_xap->getItems() objectAtIndex:index];
	}
	if (![item isKindOfClass:[StyleNode class]]) {
		return nil;
	}
	return [[item children] objectAtIndex:index];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	UT_UNUSED(outlineView);
	UT_UNUSED(tableColumn);
	return [item value];
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	UT_UNUSED(outlineView);
	UT_UNUSED(tableColumn);
	UT_UNUSED(item);
	[cell setFont:[NSFont systemFontOfSize:10.0f]];
}

- (void)setSensitivity:(bool)bSens
{
	[_applyBtn setEnabled:bSens];
	[_stylistList setEnabled:bSens];
	_enabled = bSens;
}


@end
