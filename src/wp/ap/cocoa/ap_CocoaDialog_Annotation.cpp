/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_CocoaDialog_Annotation.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Annotation::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Annotation * p = new AP_CocoaDialog_Annotation(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Annotation::AP_CocoaDialog_Annotation(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_Annotation(pDlgFactory, dlgid)
	, m_dlg(nil)
{
}

AP_CocoaDialog_Annotation::~AP_CocoaDialog_Annotation(void)
{
}

void AP_CocoaDialog_Annotation::runModal(XAP_Frame *)
{
	m_dlg = [[AP_CocoaDialog_AnnotationController alloc] initFromNib];
	
	[m_dlg setXAPOwner:this];
	NSWindow * window = [m_dlg window];

	[NSApp runModalForWindow:window];
	
	[m_dlg discardXAP];
	[m_dlg release];
	m_dlg = nil;
}

@implementation AP_CocoaDialog_AnnotationController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Annotation"];
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
	_xap = dynamic_cast<AP_CocoaDialog_Annotation*>(owner);
}

- (void)loadData
{
	[_titleText setStringValue:[NSString stringWithUTF8String:_xap->getTitle().c_str()]];
	[_authorText setStringValue:[NSString stringWithUTF8String:_xap->getAuthor().c_str()]];
	[_descText setString:[NSString stringWithUTF8String:_xap->getDescription().c_str()]];
}

-(void)windowDidLoad
{
	if(_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Annotation_Title);

		LocalizeControl(_titleLabel, pSS, AP_STRING_ID_DLG_Annotation_Title_LBL);
		LocalizeControl(_authorLabel, pSS, AP_STRING_ID_DLG_Annotation_Author_LBL);
		LocalizeControl(_descLabel, pSS, AP_STRING_ID_DLG_Annotation_Description_LBL);

		LocalizeControl(_replaceBtn, pSS, AP_STRING_ID_DLG_Annotation_Replace_LBL);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		[self loadData];
	}
}


- (void)saveData
{
	_xap->setTitle([[_titleText stringValue] UTF8String]);
	_xap->setAuthor([[_authorText stringValue] UTF8String]);
	_xap->setDescription([[_descText string] UTF8String]);
}


- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setAnswer ( AP_Dialog_Annotation::a_OK ) ;
	[self saveData];
	[NSApp stopModal];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setAnswer ( AP_Dialog_Annotation::a_CANCEL ) ;
	[NSApp stopModal];
}

- (IBAction)replaceAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setAnswer ( AP_Dialog_Annotation::a_APPLY ) ;
	[self saveData];
	[NSApp stopModal];
}

@end

