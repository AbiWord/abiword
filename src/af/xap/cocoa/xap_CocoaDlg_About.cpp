/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_About.h"

#import "xap_Cocoa_ResourceIDs.h"


/*****************************************************************/


XAP_Dialog * XAP_CocoaDialog_About::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_About * p = new XAP_CocoaDialog_About(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_About::XAP_CocoaDialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id dlgid)
	: XAP_Dialog_About(pDlgFactory,dlgid)
{
}

XAP_CocoaDialog_About::~XAP_CocoaDialog_About(void)
{
}

/*****************************************************************/

void XAP_CocoaDialog_About::runModal(XAP_Frame * )
{
	m_dlg = [[XAP_CocoaDlg_AboutController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	NSWindow *win = [m_dlg window];		// force the window to be loaded.

	[NSApp runModalForWindow:win];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
}

void XAP_CocoaDialog_About::event_OK(void)
{
	[NSApp stopModal];
}

void XAP_CocoaDialog_About::event_URL(void)
{
	XAP_App::getApp()->openURL("http://www.abisource.com/");
}

/*****************************************************************/


@implementation XAP_CocoaDlg_AboutController


- (id)initFromNib
{
	return [super initWithWindowNibName:@"xap_CocoaDlg_About"];
}

-(void)discardXAP
{
	m_xap = NULL;
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	m_xap = dynamic_cast<XAP_CocoaDialog_About*>(owner);
}

- (void)windowDidLoad
{
	XAP_App * app = XAP_App::getApp();
	// we get all our strings from the application string set
	const XAP_StringSet * pSS = app->getStringSet();
	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_ABOUT_Title, s);
	[[self window] setTitle:[NSString stringWithFormat:[NSString stringWithUTF8String:s.c_str()], app->getApplicationName()]];	
	[m_okBtn setTitle:[NSString stringWithUTF8String:pSS->getValue(XAP_STRING_ID_DLG_OK)]];
	[m_appName setStringValue:[NSString stringWithUTF8String:app->getApplicationName()]];
	pSS->getValueUTF8(XAP_STRING_ID_DLG_ABOUT_Version, s);
	[m_versionLabel setStringValue:[NSString stringWithFormat:[NSString stringWithUTF8String:s.c_str()], XAP_App::s_szBuild_Version]];
	[m_licenseText insertText:[NSString stringWithFormat:@"%s\n\n%@", XAP_ABOUT_COPYRIGHT,
					[NSString stringWithFormat:@XAP_ABOUT_GPL_LONG_LF, app->getApplicationName()]]];
	[m_licenseText setEditable:NO];

	NSClipView * clipView = (NSClipView *) [m_licenseText superview];
	[clipView scrollToPoint:NSZeroPoint];

	NSScrollView * scrollView = (NSScrollView *) [clipView superview]; // Not sure why this is necessary...
	[scrollView reflectScrolledClipView:clipView];

	NSImage*	image = [NSImage imageNamed:XAP_COCOA_ABOUT_SIDEBAR_RESOURCE_NAME];
	[m_imageView setImage:image];
}

- (IBAction)okBtnAction:(id)sender
{
	UT_UNUSED(sender);
	m_xap->event_OK();
}

- (IBAction)webBtnAction:(id)sender
{
	UT_UNUSED(sender);
	m_xap->event_URL();
}

@end

