/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaWidget.h"
#include "xap_Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Strings.h"

#include "ap_CocoaDialog_Latex.h"

XAP_Dialog * AP_CocoaDialog_Latex::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	return new AP_CocoaDialog_Latex(pFactory,dlgid);
}

AP_CocoaDialog_Latex::AP_CocoaDialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
		 AP_Dialog_Latex(pDlgFactory,dlgid)
{
	// ...
}

AP_CocoaDialog_Latex::~AP_CocoaDialog_Latex(void)
{
	// ...
}

void AP_CocoaDialog_Latex::runModeless(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_LatexController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	[m_dlg window]; // just to make sure the nib is loaded.

	/* Save dialog the ID number and pointer to the widget
	 */
	UT_sint32 sid = static_cast<UT_sint32>(getDialogId());
	m_pApp->rememberModelessId (sid, (XAP_Dialog_Modeless *) m_pDialog);

	activate();
}

void  AP_CocoaDialog_Latex::activate(void)
{
	if (m_dlg)
		{
			[[m_dlg window] orderFront:m_dlg];
		}
}

void AP_CocoaDialog_Latex::notifyActiveFrame(XAP_Frame */*pFrame*/)
{
	// ...
}

void AP_CocoaDialog_Latex::event_Insert(void)
{
	getLatexFromGUI();

	if (convertLatexToMathML())
		{
			insertIntoDoc();
		}
}

void AP_CocoaDialog_Latex::event_Close(void)
{
	destroy();
}

void AP_CocoaDialog_Latex::destroy(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	

	modeless_cleanup();

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Latex::setLatexInGUI(void)
{
	if (m_dlg)
		{
			UT_UTF8String latex;

			getLatex(latex);

			[m_dlg setEditorText:[NSString stringWithUTF8String:(latex.utf8_str())]];
		}
}

bool AP_CocoaDialog_Latex::getLatexFromGUI(void)
{
	bool bOkay = false;

	if (m_dlg)
		if (NSString * editorText = [m_dlg editorText])
			if ([editorText length])
				{
					bOkay = true;

					UT_UTF8String latex([editorText UTF8String]);

					setLatex(latex);
				}
	return bOkay;
}

/*****************************************************************/

@implementation AP_CocoaDialog_LatexController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Latex"]) {

	}
	return self;
}

- (void)dealloc
{
	// ...
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_Latex *>(owner);
}

- (void)discardXAP
{
	_xap = nil;
}

- (void)windowDidLoad
{
	if (_xap)
		{
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

			LocalizeControl([self window],	pSS, AP_STRING_ID_DLG_Latex_LatexTitle);

			LocalizeControl(oClose,			pSS, AP_STRING_ID_DLG_CloseButton);
			LocalizeControl(oInsert,		pSS, AP_STRING_ID_DLG_InsertButton);

			LocalizeControl(oHeadingText,	pSS, AP_STRING_ID_DLG_Latex_LatexEquation);
			LocalizeControl(oExampleText,	pSS, AP_STRING_ID_DLG_Latex_Example);
		}
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if (_xap)
		_xap->event_Close();
}

- (IBAction)aClose:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		_xap->event_Close();
}

- (IBAction)aInsert:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		_xap->event_Insert();
}

- (void)setEditorText:(NSString *)text
{
    [oEditor setString:text];
}

- (NSString *)editorText
{
	return [oEditor string];
}

@end
