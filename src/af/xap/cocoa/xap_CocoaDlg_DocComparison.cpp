/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#import <Cocoa/Cocoa.h>

#include "ut_string.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"


#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_CocoaDlg_DocComparison.h"
#include "xap_CocoaDialogFactory.h"

/*****************************************************************/


XAP_Dialog * XAP_CocoaDialog_DocComparison::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_DocComparison * p = new XAP_CocoaDialog_DocComparison(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_DocComparison::XAP_CocoaDialog_DocComparison(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid)
	: XAP_Dialog_DocComparison(pDlgFactory,dlgid)
{
}


XAP_CocoaDialog_DocComparison::~XAP_CocoaDialog_DocComparison(void)
{
}

void XAP_CocoaDialog_DocComparison::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[XAP_CocoaDialog_DocComparisonController alloc] initFromNib];
	
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


void XAP_CocoaDialog_DocComparison::_populateWindowData(void)
{
	[m_dlg populate];
}


@implementation XAP_CocoaDialog_DocComparisonController
- (id)initFromNib
{
	return [super initWithWindowNibName:@"xap_CocoaDlg_DocComparison"];
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
	_xap = dynamic_cast<XAP_CocoaDialog_DocComparison*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_DocComparison_WindowLabel);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);

		LocalizeControl(_docCmpBox, pSS, XAP_STRING_ID_DLG_DocComparison_DocsCompared);
		LocalizeControl(_resultsBox, pSS, XAP_STRING_ID_DLG_DocComparison_Results);
		LocalizeControl(_relationshipLabel, pSS, XAP_STRING_ID_DLG_DocComparison_Relationship);
		LocalizeControl(_contentLabel, pSS, XAP_STRING_ID_DLG_DocComparison_Content);
		LocalizeControl(_formatLabel, pSS, XAP_STRING_ID_DLG_DocComparison_Fmt);
		LocalizeControl(_stylesLabel, pSS, XAP_STRING_ID_DLG_DocComparison_Styles);
	}
}


- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	[NSApp stopModal];
}

- (void)populate
{
	const char * path = _xap->getPath1();
	if(path) {
		[_doc1 setStringValue:[NSString stringWithUTF8String:path]];
	}
	path = _xap->getPath2();
	if(path) {
		[_doc2 setStringValue:[NSString stringWithUTF8String:path]];
	}
	[_relationshipData setStringValue:[NSString stringWithUTF8String:_xap->getResultValue(0)]];
	[_contentData setStringValue:[NSString stringWithUTF8String:_xap->getResultValue(1)]];
	[_formatData setStringValue:[NSString stringWithUTF8String:_xap->getResultValue(2)]];
	[_stylesData setStringValue:[NSString stringWithUTF8String:_xap->getResultValue(3)]];
}


@end
