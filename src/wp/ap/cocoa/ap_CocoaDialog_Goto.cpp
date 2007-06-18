/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaDialog_Utilities.h"

#include "xap_Dialog_Id.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_CocoaDialog_Goto.h"

#include "fv_View.h"

/*****************************************************************/
XAP_Dialog * AP_CocoaDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Goto * p = new AP_CocoaDialog_Goto(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Goto::AP_CocoaDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id dlgid)
	: AP_Dialog_Goto(pDlgFactory, dlgid),
		m_iRow(-1),
		m_dlg(nil)
{
}

AP_CocoaDialog_Goto::~AP_CocoaDialog_Goto(void)
{

}
#if 0
void AP_CocoaDialog_Goto::s_blist_clicked(GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_CocoaDialog_Goto *me)
{
	gtk_entry_set_text(GTK_ENTRY(me->m_wEntry), (gchar*)me->m_pBookmarks[row]);
}
#endif
void AP_CocoaDialog_Goto::doGoto(const char *number)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) g_try_malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS4_strcpy_char (ucsnumber, number);
	int target = getSelectedRow ();
	getView()->gotoTarget (static_cast<AP_JumpTarget>(target), ucsnumber);
	g_free (ucsnumber);
}

void AP_CocoaDialog_Goto::event_goto (const char* number)
{
	if (number && *number) {
		doGoto(number);
	}
}

void AP_CocoaDialog_Goto::event_forward ()
{
	doGoto ("+1");
}

void AP_CocoaDialog_Goto::event_backward ()
{
	doGoto("-1");
}


void AP_CocoaDialog_Goto::event_targetChanged(int row)
{
	setSelectedRow (row);
}

void AP_CocoaDialog_Goto::event_valueChanged()
{
/*
	gchar *text = gtk_entry_get_text (GTK_ENTRY (widget));

	if (text[0] == '\0')
	{
		gtk_widget_grab_default (me->m_wClose);
		gtk_widget_set_sensitive (me->m_wGoto, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (me->m_wGoto, TRUE);
		gtk_widget_grab_default (me->m_wGoto);
	}
*/
}

void AP_CocoaDialog_Goto::setSelectedRow (int row)
{
	m_iRow = row;
/*	if(row == (int) AP_JUMPTARGET_BOOKMARK)
	{
		gtk_widget_hide(m_dlabel);
		gtk_widget_show(m_swindow);
	}
	else
	{
		gtk_widget_hide(m_swindow);
		gtk_widget_show(m_dlabel);
	}*/
}

int AP_CocoaDialog_Goto::getSelectedRow (void)
{
	return (m_iRow);
}

void AP_CocoaDialog_Goto::runModeless (XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_GotoController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];

	[m_dlg windowToFront];

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
}

void AP_CocoaDialog_Goto::destroy (void)
{
	modeless_cleanup();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Goto::activate (void)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];

	[m_dlg windowToFront];
}

void AP_CocoaDialog_Goto::notifyActiveFrame(XAP_Frame * pFrame)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];
}


@implementation AP_CocoaDialog_GotoController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_Goto"];
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
	_xap = dynamic_cast<AP_CocoaDialog_Goto*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl(jumpToBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Goto);
		LocalizeControl(forwardBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Next);
		LocalizeControl(backBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Prev);
		LocalizeControl(whatLabel,	pSS, AP_STRING_ID_DLG_Goto_Label_What);
		LocalizeControl(valueLabel,	pSS, AP_STRING_ID_DLG_Goto_Label_Number);

		[whatPopup removeAllItems];

		if (char ** tmp2 = _xap->getJumpTargets()) {
			for (int i = 0; tmp2[i] != NULL; i++) {
				[whatPopup addItemWithTitle:[NSString stringWithUTF8String:tmp2[i]]];
			}
			for(UT_uint32 i = 0; i < _xap->getExistingBookmarksCount(); i++) {
				[valueCombo addItemWithObjectValue:[NSString stringWithUTF8String:_xap->getNthExistingBookmark(i)]];
			}
		}
	}
}

- (void)windowToFront
{
	[[self window] makeKeyAndOrderFront:self];
	[[self window] makeFirstResponder:valueCombo];
}

- (NSString*)stringValue
{
	return [valueCombo stringValue];
}


- (IBAction)backAction:(id)sender
{
	_xap->event_backward();
}

- (IBAction)closeAction:(id)sender
{
}

- (IBAction)forwardAction:(id)sender
{
	_xap->event_forward();
}

- (IBAction)jumpToAction:(id)sender
{
	_xap->event_goto([[valueCombo stringValue] UTF8String]);
}

- (IBAction)valueComboAction:(id)sender
{
	_xap->event_valueChanged();
}

- (IBAction)whatPopupAction:(id)sender
{
	NSMenuItem * item = [sender selectedItem];
	if (item) {
		_xap->event_targetChanged([sender indexOfItem:item]);
	}
}

@end
