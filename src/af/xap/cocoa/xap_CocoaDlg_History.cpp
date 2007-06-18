/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_History.h"
#include "xap_CocoaDlg_History.h"

/*****************************************************************/

#if 0
static void s_history_selected(GtkTreeView *treeview,
                            XAP_CocoaDialog_History * dlg)
{
	UT_ASSERT(treeview && dlg);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	UT_sint32 item;
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
	if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}

	// Get the row and col number
	GValue value = {0,};
	gtk_tree_model_get_value (model, &iter,3,&value);
	item = g_value_get_int(&value);
	UT_DEBUGMSG(("Vlaue of id selected %d \n",item));
	dlg->setSelectionId(item);
}

#endif
XAP_Dialog * XAP_CocoaDialog_History::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_History * p = new XAP_CocoaDialog_History(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_History::XAP_CocoaDialog_History(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: XAP_Dialog_History(pDlgFactory,dlgid)
{
}

XAP_CocoaDialog_History::~XAP_CocoaDialog_History(void)
{
}

void XAP_CocoaDialog_History::runModal(XAP_Frame * pFrame)
{
	m_dlg = [[XAP_CocoaDialog_HistoryController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];
	
	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	
	_populateWindowData();  

	[NSApp runModalForWindow:window];

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

#if 0
GtkWidget * XAP_CocoaDialog_History::_constructWindow(void)
{


	_fillHistoryTree();

	// set the single selection mode for the TreeView
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_wTreeView)), GTK_SELECTION_SINGLE);	
	gtk_container_add (GTK_CONTAINER (m_wListWindow), m_wTreeView);
#if 0
	g_signal_connect_after(G_OBJECT(m_wTreeView),
						   "cursor-changed",
						   G_CALLBACK(s_history_selected),
						   static_cast<gpointer>(this));
#endif
	gtk_widget_show_all(m_wTreeView);	
	return m_windowMain;
}
#endif

void XAP_CocoaDialog_History::_fillHistoryTree(void)
{
#if 0
	UT_uint32 i;
	
	GtkTreeIter iter;

	GtkTreeStore * model = gtk_tree_store_new (3, // Total number of columns
                                          G_TYPE_STRING,   //Version number
										  G_TYPE_STRING, //           
										  G_TYPE_STRING,
										  G_TYPE_INT); //

	// build a list of all items
    for (i = 0; i < getListItemCount(); i++)
	{
		// Add a new row to the model
		gtk_tree_store_append (model, &iter,NULL);
		gtk_tree_store_set (model, &iter, 0, getListValue(i,0), 
							1,getListValue(i,1) ,
							2,getListValue(i,2) ,
							3,getListItemId(i) ,
							-1);
	}
    m_wTreeView = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	g_object_unref (model);	
//
// Renderer for the view
//
	GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
//
// Now create columns and add them to the tree
//
	GtkTreeViewColumn * column0 = gtk_tree_view_column_new_with_attributes (getListHeader(0), renderer,
                                                      "text", 0,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column0);
	GtkTreeViewColumn * column1 = gtk_tree_view_column_new_with_attributes (getListHeader(1), renderer,
                                                      "text", 1,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column1);
	GtkTreeViewColumn * column2 = gtk_tree_view_column_new_with_attributes (getListHeader(2), renderer,
                                                      "text", 2,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column2);
	
 	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_wTreeView);
#endif
}

void XAP_CocoaDialog_History::_populateWindowData(void)
{
	[m_dlg populate];
}

void XAP_CocoaDialog_History::event_OK()
{
	m_answer = a_OK;
	[NSApp stopModal];
}


void XAP_CocoaDialog_History::event_Cancel()
{
	m_answer = a_CANCEL;
	[NSApp stopModal];
}


@implementation XAP_CocoaDialog_HistoryController
- (id)initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_History"];
	return self;
}

-(void)discardXAP
{
	_xap = NULL; 
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_History*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_History_WindowLabel);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_historyBox, pSS, XAP_STRING_ID_DLG_History_DocumentDetails);
		[_historyList setTarget:self];
		[_historyList setAction:@selector(historySelect:)];
	}
}


- (IBAction)cancelAction:(id)sender
{
	_xap->event_Cancel();
}

- (IBAction)okAction:(id)sender
{
	_xap->event_OK();
}

- (IBAction)historySelect:(id)sender
{
	UT_uint32 item = [_historyList selectedRow];
	_xap->setSelectionId(item);
}


- (void)populate
{
	[_docNameLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(0)]];
	[_docNameData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(0)]];
	[_versionLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(1)]];
	[_versionData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(1)]];
	[_createdLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(2)]];
	[_createdData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(2)]];
	[_lastSavedLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(3)]];
	[_lastSavedData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(3)]];
	[_editTimeLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(4)]];
	[_editTimeData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(4)]];
	[_identifierLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(5)]];
	[_identifierData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(5)]];
//	[_historyBox setStringValue:[NSString stringWithUTF8String:_xap->getListTitle()]];
}

@end

