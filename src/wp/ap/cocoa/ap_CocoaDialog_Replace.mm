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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_CocoaDialog_Replace.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	xxx_UT_DEBUGMSG(("AP_CocoaDialog_Replace::static_constructor(...) I've been called\n"));

	AP_CocoaDialog_Replace * p = new AP_CocoaDialog_Replace(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_Replace::AP_CocoaDialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id dlgid)
	: AP_Dialog_Replace(pDlgFactory,dlgid),
	m_dlg(nil)
{
}

AP_CocoaDialog_Replace::~AP_CocoaDialog_Replace(void)
{
	if (m_dlg) {
		[m_dlg release];
	}
}

/*****************************************************************/

void AP_CocoaDialog_Replace::activate(void)
{
	ConstructWindowName();
	NSWindow* window = [m_dlg window];
	[window setTitle:[NSString stringWithUTF8String:m_WindowName]];
	[window orderFront:m_dlg];
}


void AP_CocoaDialog_Replace::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:m_WindowName]];
}

void AP_CocoaDialog_Replace::runModeless(XAP_Frame * pFrame)
{
	// get the Dialog Id number
	UT_sint32 sid =(UT_sint32)  getDialogId();

	m_dlg = [[AP_CocoaDialog_ReplaceController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	// Save dialog the ID number and pointer to the Dialog
	m_pApp->rememberModelessId( sid,  (XAP_Dialog_Modeless *) m_pDialog);

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();
	
	[window orderFront:m_dlg];

	// this dialog needs this
	setView(static_cast<FV_View *> (getActiveFrame()->getCurrentView()));
}

void AP_CocoaDialog_Replace::event_Find(void)
{
	NSString* findWhat;
	
	findWhat = [m_dlg findWhat];
	
	setFindString(UT_UCS4String([findWhat UTF8String]).ucs4_str());
	
	if (!getReverseFind())	{
		findNext();
	}
	else {
		findPrev();
	}
}

		
void AP_CocoaDialog_Replace::event_Replace(void)
{
	NSString* findWhat;
	NSString* replaceWith;
	
	findWhat = [m_dlg findWhat];
	replaceWith = [m_dlg replaceWith];
	
	setFindString(UT_UCS4String([findWhat UTF8String]).ucs4_str());
	setReplaceString(UT_UCS4String([replaceWith UTF8String]).ucs4_str());
	
	if(!getReverseFind()) {	
		findReplace();
	}
	else {
		findReplaceReverse();
	}
}

void AP_CocoaDialog_Replace::event_ReplaceAll(void)
{
	NSString* findWhat;
	NSString* replaceWith;
	
	findWhat = [m_dlg findWhat];
	replaceWith = [m_dlg replaceWith];

	setFindString(UT_UCS4String([findWhat UTF8String]).ucs4_str());
	setReplaceString(UT_UCS4String([replaceWith UTF8String]).ucs4_str());

	findReplaceAll();
}


void AP_CocoaDialog_Replace::event_MatchCaseToggled(void)
{
	setMatchCase([m_dlg matchCase]);
}


void AP_CocoaDialog_Replace::event_WholeWordToggled(void)
{
	setMatchCase([m_dlg wholeWord]);
}


void AP_CocoaDialog_Replace::event_ReverseFindToggled(void)
{
	setReverseFind([m_dlg findReverse]);
}


void AP_CocoaDialog_Replace::event_Cancel(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;
	destroy();
}


void AP_CocoaDialog_Replace::event_CloseWindow(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;
}


void AP_CocoaDialog_Replace::destroy(void)
{
	_storeWindowData();
	modeless_cleanup();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


/*****************************************************************/
void AP_CocoaDialog_Replace::_populateWindowData(void)
{
	// last used find string
	{
		UT_UCS4Char * bufferUnicode = getFindString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		[m_dlg setFindWhat:[NSString stringWithUTF8String:bufferNormal]];
		// select teh whole buffer
		FREEP(bufferNormal);
	}
	
	
	// last used replace string
	if (m_id == AP_DIALOG_ID_REPLACE)
	{		
		UT_UCS4Char * bufferUnicode = getReplaceString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		[m_dlg setReplaceWith:[NSString stringWithUTF8String:bufferNormal]];

		FREEP(bufferNormal);
	}

	// match case button
	[m_dlg setMatchCase:getMatchCase()];
	[m_dlg setWholeWord:getWholeWord()];
	[m_dlg setFindReverse:getReverseFind()];

	// give focus to find what
}


void AP_CocoaDialog_Replace::_updateLists()
{
	[m_dlg updateFindWhat:&m_findList];
	[m_dlg updateReplaceWith:&m_replaceList];
}



@implementation AP_CocoaDialog_ReplaceController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_Replace"];
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
	_xap = dynamic_cast<AP_CocoaDialog_Replace*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		
		_xap->ConstructWindowName();
		[[self window] setTitle:[NSString stringWithUTF8String:_xap->getWindowName()]];
		LocalizeControl(_whatLabel, pSS, AP_STRING_ID_DLG_FR_FindLabel);
		LocalizeControl(_matchCaseBtn, pSS, AP_STRING_ID_DLG_FR_MatchCase);
		LocalizeControl(_wholeWordBtn, pSS, AP_STRING_ID_DLG_FR_WholeWord);
		LocalizeControl(_replaceLabel, pSS, AP_STRING_ID_DLG_FR_ReplaceWithLabel);
		LocalizeControl(_findAndReplaceBtn, pSS, AP_STRING_ID_DLG_FR_ReplaceButton);
		LocalizeControl(_replaceAll, pSS, AP_STRING_ID_DLG_FR_ReplaceAllButton);
		LocalizeControl(_findBtn, pSS, AP_STRING_ID_DLG_FR_FindNextButton);
		LocalizeControl(_findReverseBtn, pSS, AP_STRING_ID_DLG_FR_ReverseFind);
	}
}
	
- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_CloseWindow();
}


- (IBAction)findAction:(id)sender
{
	_xap->event_Find();
}

- (IBAction)findAndReplaceAction:(id)sender
{
	_xap->event_Replace();
}

- (IBAction)matchCaseAction:(id)sender
{
	_xap->event_MatchCaseToggled();
}

- (IBAction)wholeWordAction:(id)sender
{
	_xap->event_WholeWordToggled();
}

- (IBAction)findReverseAction:(id)sender
{
	_xap->event_ReverseFindToggled();
}

- (IBAction)replaceAllAction:(id)sender
{
	_xap->event_ReplaceAll();
}

- (NSString*)findWhat
{
	return [_whatCombo stringValue];
}


- (void)setFindWhat:(NSString*)str
{
	[_whatCombo setStringValue:str];
}


- (NSString*)replaceWith
{
	return [_replaceCombo stringValue];
}


- (void)setReplaceWith:(NSString*)str
{
	[_replaceCombo setStringValue:str];
}


- (bool)matchCase
{
	return ([_matchCaseBtn state] != NSOffState);
}

- (void)setMatchCase:(bool)val
{
	[_matchCaseBtn setState:(val?NSOnState:NSOffState)];
}

- (bool)wholeWord
{
	return ([_wholeWordBtn state] != NSOffState);
}


- (void)setWholeWord:(bool)val
{
	[_wholeWordBtn setState:(val?NSOnState:NSOffState)];
}


- (bool)findReverse
{
	return ([_findReverseBtn state] != NSOffState);
}


- (void)setFindReverse:(bool)val
{
	[_findReverseBtn setState:(val?NSOnState:NSOffState)];
}


- (void)updateFindWhat:(UT_Vector*)list
{
	[self _updateCombo:_whatCombo withList:list];
}


- (void)updateReplaceWith:(UT_Vector*)list
{
	[self _updateCombo:_replaceCombo withList:list];
}

- (void)_updateCombo:(NSComboBox*)combo withList:(UT_Vector*)list
{
	UT_uint32 vecSize, i;
	[combo removeAllItems];
	
	vecSize = list->getItemCount();
	for(i = 0; i < vecSize; i++) {
		[combo addItemWithObjectValue:[NSString stringWithUTF8String:(const char*)(*list)[i]]];
	}
}

@end

