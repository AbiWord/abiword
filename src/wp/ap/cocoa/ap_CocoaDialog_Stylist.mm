/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
 * Copyright (C) 2004 Hubert Figuiere
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
	NSMutableArray *_childrens;
	int _row;
	int _col;
}
-(id)initWithValue:(const char*)value row:(int)row andCol:(int)col;
-(void)dealloc;

-(void)addChild:(id)child;
-(NSArray*)childrens;
-(NSString*)value;
-(int)row;
-(int)col;
@end

@implementation StyleNode

-(id)initWithValue:(const char*)value row:(int)row andCol:(int)col
{
	self = [super init];
	if (self) {
		_value = [[NSString alloc] initWithUTF8String:value];
		_row = row;
		_col = col;
	}
	return self;
}


-(void)dealloc
{
	[_value release];
	[_childrens release];
	[super dealloc];
}

-(void)addChild:(id)child
{
	if (!_childrens) {
		_childrens = [[NSMutableArray alloc] init];
	}
	[_childrens addObject:child];
}

-(NSArray*)childrens
{
	return _childrens;
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


#if 0
static gboolean
tree_select_filter (GtkTreeSelection *selection, GtkTreeModel *model,
								  GtkTreePath *path, gboolean path_selected,
								  gpointer data)
{
	if (gtk_tree_path_get_depth (path) > 1)
		return TRUE;
	return FALSE;
}


static void s_delete_clicked(GtkWidget * wid, AP_CocoaDialog_Stylist * me )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}


static void s_destroy_clicked(GtkWidget * wid, AP_CocoaDialog_Stylist * me )
{
   me->event_Close();
}


static void s_response_triggered(GtkWidget * widget, gint resp, AP_CocoaDialog_Stylist * dlg)
{
	UT_return_if_fail(widget && dlg);
	
	if ( resp == GTK_RESPONSE_APPLY )
	  dlg->event_Apply();
	else if ( resp == GTK_RESPONSE_CLOSE )
	  abiDestroyWidget(widget);
}
#endif

XAP_Dialog * AP_CocoaDialog_Stylist::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id dlgid)
{
	return new AP_CocoaDialog_Stylist(pFactory,dlgid);
}

AP_CocoaDialog_Stylist::AP_CocoaDialog_Stylist(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: AP_Dialog_Stylist(pDlgFactory,dlgid),
		m_dlg(nil),
		m_items(nil)
{
	m_items = [[NSMutableArray alloc] init];
}

AP_CocoaDialog_Stylist::~AP_CocoaDialog_Stylist(void)
{
	[m_items release];
}

void AP_CocoaDialog_Stylist::event_Close(void)
{
	destroy();
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
	if (row >= 0) {
		StyleNode *node = [m_items objectAtIndex:row];
		if (col >= 0) {
			node = [[node childrens] objectAtIndex:col];
		}
		[m_dlg selectStyleNode:node];
	}

	setStyleChanged(false);
}

void AP_CocoaDialog_Stylist::destroy(void)
{
	finalize();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Stylist::activate(void)
{
	UT_ASSERT (m_dlg);
	[[m_dlg window] orderFront:m_dlg];
}

void AP_CocoaDialog_Stylist::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_dlg);
}

/*!
 * Set the style in the XP layer fromthe selection in the GUI.
 */
void AP_CocoaDialog_Stylist::styleClicked(UT_sint32 row, UT_sint32 col)
{
	UT_UTF8String sStyle;
	UT_DEBUGMSG(("row %d col %d clicked \n",row,col));
	if((col == 0) && (getStyleTree()->getNumCols(row) == 1))
	{
		return;
	}
	else if(col == 0)
	{
		getStyleTree()->getStyleAtRowCol(sStyle,row,col);
	}
	else
	{
		getStyleTree()->getStyleAtRowCol(sStyle,row,col-1);
	}
	UT_DEBUGMSG(("StyleClicked row %d col %d style %s \n",row,col,sStyle.utf8_str()));
	setCurStyle(sStyle);
}


void AP_CocoaDialog_Stylist::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0);
}

void AP_CocoaDialog_Stylist::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_Stylist_Controller alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();
	[window orderFront:m_dlg];	
	startUpdater();
}


void  AP_CocoaDialog_Stylist::event_Apply(void)
{
	Apply();
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

	UT_UTF8String sTmp(""); 
	for(row= 0; row < pStyleTree->getNumRows();row++)
	{
		if(!pStyleTree->getNameOfRow(sTmp,row))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		currentChild = [[[StyleNode alloc] initWithValue:sTmp.utf8_str() row:row andCol:0] autorelease];
		if(pStyleTree->getNumCols(row) > 0)
		{
			UT_DEBUGMSG(("Adding Heading %s at row %d \n",sTmp.utf8_str(),row));

			[m_items addObject:currentChild];
			for(col =0 ; col < pStyleTree->getNumCols(row); col++)
			{
				if(!pStyleTree->getStyleAtRowCol(sTmp,row,col))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				[currentChild addChild:[[[StyleNode alloc] initWithValue:sTmp.utf8_str() row:row andCol:col] autorelease]];
				UT_DEBUGMSG(("Adding style %s at row %d col %d \n",sTmp.utf8_str(),row,col+1));
			}
		}
		else
		{
			UT_DEBUGMSG(("Adding style %s at row %d \n",sTmp.utf8_str(),row));
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
	self = [super initWithWindowNibName:@"ap_CocoaDialog_Stylist"];
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
		NSArray *columns;
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Stylist_Title);
		LocalizeControl(_applyBtn, pSS, XAP_STRING_ID_DLG_Apply);
		columns = [_stylistList tableColumns];
		[[[columns objectAtIndex:0] headerCell] setStringValue:[NSString stringWithUTF8String:pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_Styles).utf8_str()]];
		[_stylistList setDoubleAction:@selector(outlineDoubleAction:)];
		// data source an delegate for style list should be set by the Nib.
	}
}

- (void)refresh
{
	[_stylistList reloadData];
}

- (void)selectStyleNode:(StyleNode*)node;
{
	[_stylistList expandItem:node];
}


- (IBAction)applyAction:(id)sender
{
	_xap->event_Apply();
}

- (IBAction)outlineAction:(id)sender
{
	int row;
	row = [sender selectedRow];
	StyleNode *node = [sender itemAtRow:row];
	_xap->styleClicked([node row], [node col]);
}

- (IBAction)outlineDoubleAction:(id)sender
{
	[self outlineAction:sender];
	_xap->event_Apply ();
}


- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if (item == nil) {
		return [_xap->getItems() count];
	}
	if (![item isKindOfClass:[StyleNode class]]) {
		return 0;
	}
	return [[item childrens] count];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	if (![item isKindOfClass:[StyleNode class]]) {
		return NO;
	}
	return ([item childrens]?YES:NO);
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
	if (item == nil) {
		return [_xap->getItems() objectAtIndex:index];
	}
	if (![item isKindOfClass:[StyleNode class]]) {
		return nil;
	}
	return [[item childrens] objectAtIndex:index];
}


- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	return [item value];
}


@end
