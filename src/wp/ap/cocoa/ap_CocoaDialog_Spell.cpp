/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2003-2004, 2009 Hubert Figuiere
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


// TODO: still getting some artifacts when doing highligh/replacements

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_CocoaDialog_Spell.h"


XAP_Dialog * AP_CocoaDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
   AP_CocoaDialog_Spell * p = new AP_CocoaDialog_Spell(pFactory, dlgid);
   return p;
}

AP_CocoaDialog_Spell::AP_CocoaDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid)
  : AP_Dialog_Spell(pDlgFactory, dlgid),
	m_dlg(nil),
	m_suggestionList(nil)
{
}

AP_CocoaDialog_Spell::~AP_CocoaDialog_Spell(void)
{
	[m_suggestionList release];
}

/************************************************************/
void AP_CocoaDialog_Spell::runModal(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("beginning spelling check...\n"));
	
	// class the base class method to initialize some basic xp stuff
	AP_Dialog_Spell::runModal(pFrame);
	
	m_bCancelled = false;
	bool bRes = nextMisspelledWord();
	
	if (bRes) { // we need to prepare the dialog
		m_dlg = [[AP_CocoaDialog_Spell_Controller alloc] initFromNib];
		
		// used similarly to convert between text and numeric arguments
		[m_dlg setXAPOwner:this];
	
		// build the dialog
		NSWindow * window = [m_dlg window];
		UT_ASSERT(window);

		// Populate the window's data items
		_populateWindowData();
				
		// now loop while there are still misspelled words
		while (bRes) {

			// show word in main window
			makeWordVisible();
			
			// update dialog with new misspelled word info/suggestions
			_showMisspelledWord();
			
			// run into the GTK event loop for this window
			[NSApp runModalForWindow:window];
			_purgeSuggestions();
	 
			if (m_bCancelled) {
				break;
			}
	 
			// get the next unknown word
			bRes = nextMisspelledWord();
		}
		
		_storeWindowData();
		
		[m_dlg close];
		[m_dlg release];
		m_dlg = nil;
		[m_suggestionList release];
		m_suggestionList = nil;
	}
   
	// TODO: all done message?
	UT_DEBUGMSG(("spelling check complete.\n"));
}


void AP_CocoaDialog_Spell::_showMisspelledWord(void)
{                                
	NSMutableAttributedString *attrStr;
	NSAttributedString *buffer;
	const UT_UCSChar *p;
	// insert start of sentence
	UT_sint32 iLengthPre;
	UT_sint32 iLength;
	UT_sint32 iLengthPost;
	
	attrStr = [[NSMutableAttributedString alloc] initWithString:@""];
	[m_dlg setMisspelled:nil scroll:0.0f];

	p = m_pWordIterator->getPreWord(iLengthPre);
	if (0 < iLengthPre) {
		UT_UTF8String str(p,iLengthPre);
		buffer = [[[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:str.utf8_str()]] autorelease];
		[attrStr appendAttributedString:buffer];
	}

	// insert misspelled word (in highlight color)
	p = m_pWordIterator->getCurrentWord(iLength);
	{
		UT_UTF8String str(p,iLength);
		NSString * word = [NSString stringWithUTF8String:str.utf8_str()];
		[m_dlg setReplace:word];

		NSDictionary * attr = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];

		buffer = [[[NSAttributedString alloc] initWithString:word attributes:attr] autorelease];
	}
	[attrStr appendAttributedString:buffer];
		
	// insert end of sentence
	p = m_pWordIterator->getPostWord(iLengthPost);
	if (0 < iLengthPost) {
		UT_UTF8String str(p,iLengthPost);
		[attrStr appendAttributedString:
		            [[[NSAttributedString alloc] initWithString:
					         [NSString stringWithUTF8String:str.utf8_str()]] autorelease]];
	}

	float offset = static_cast<float>(iLengthPre + iLength) / static_cast<float>(iLengthPre + iLength + iLengthPost);

	[m_dlg setMisspelled:attrStr scroll:offset];

	[m_suggestionList removeAllStrings];
	for (UT_sint32 i = 0; i < m_Suggestions->getItemCount(); i++) {
		UT_UTF8String str((UT_UCSChar*)m_Suggestions->getNthItem(i));
		[m_suggestionList addCString:str.utf8_str()];
	}
	
	if (!m_Suggestions->getItemCount()) {	
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		[m_suggestionList addCString:pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions)];
	}
	[m_dlg reloadSuggestionList];

	if (!m_Suggestions->getItemCount()) {	
		m_iSelectedRow = -1;
	}
	else {
		[m_dlg selectSuggestion:0];
		[m_dlg suggestionSelected:nil];
	}

	[attrStr release];
}

void AP_CocoaDialog_Spell::_populateWindowData(void)
{
   // TODO: initialize list of user dictionaries
   m_suggestionList = [[XAP_StringListDataSource alloc] init];
   [m_dlg setSuggestionList:m_suggestionList];
}

void AP_CocoaDialog_Spell::_storeWindowData(void)
{
   // TODO: anything to store?
}

/*************************************************************/

void AP_CocoaDialog_Spell::event_Change()
{
	UT_UCSChar * replace = NULL;
	UT_DEBUGMSG(("m_iSelectedRow is %i\n", m_iSelectedRow));
	if (m_iSelectedRow != -1) {
		replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
		UT_DEBUGMSG(("Replacing with %s\n", (char*) replace));
		//fprintf(stderr, "Replacing with %s\n", replace);
		changeWordWith(replace);		
	}
	else {
		UT_UCS4String str([[m_dlg replace] UTF8String], 0);
		changeWordWith(str.ucs4_str());
	}
	
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_ChangeAll()
{
	UT_UCSChar * replace = NULL;
	if (m_iSelectedRow != -1) {
		replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
		addChangeAll(replace);
		changeWordWith(replace);
	}
	else {
		// replacement input by the user. If it is empty, we do nothing, 
		// see bug 8552
		NSString * replaceUser = [m_dlg replace];
		
		if ([replaceUser length] > 0) {
			
			UT_UCS4String str([replaceUser UTF8String], 0);

			addChangeAll(str.ucs4_str());
			changeWordWith(str.ucs4_str());
		}
	}
   
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_Ignore()
{
   ignoreWord();
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_IgnoreAll()
{
   addIgnoreAll();
   ignoreWord();
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_AddToDict()
{
   addToDict();
   
   ignoreWord();
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_Cancel()
{
   m_bCancelled = true;
   [NSApp stopModal];
}

void AP_CocoaDialog_Spell::event_SuggestionSelected(int row, int /*column*/)
{
	if (!m_Suggestions->getItemCount()) {
		return;
	}
	m_iSelectedRow = row;
	
	[m_dlg setReplace:[[m_suggestionList array] objectAtIndex:row]];
}

void AP_CocoaDialog_Spell::event_ReplacementChanged()
{
	m_iSelectedRow = -1;
	event_Change();
}


@implementation AP_CocoaDialog_Spell_Controller

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Spell"];
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
	_xap = dynamic_cast<AP_CocoaDialog_Spell*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Spell_SpellTitle);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);

		LocalizeControl(_addBtn, pSS, AP_STRING_ID_DLG_Spell_AddToDict);
		LocalizeControl(_changeAllBtn, pSS, AP_STRING_ID_DLG_Spell_ChangeAll);
		LocalizeControl(_changeBtn, pSS, AP_STRING_ID_DLG_Spell_Change);
		LocalizeControl(_ignoreAllBtn, pSS, AP_STRING_ID_DLG_Spell_IgnoreAll);
		LocalizeControl(_ignoreBtn, pSS, AP_STRING_ID_DLG_Spell_Ignore);
		LocalizeControl(_replLabel, pSS, AP_STRING_ID_DLG_Spell_ChangeTo);
		LocalizeControl(_unknownLabel, pSS, AP_STRING_ID_DLG_Spell_UnknownWord);
		[_suggestionList setTarget:self];
		[_suggestionList setAction:@selector(suggestionSelected:)];
	}
}


- (IBAction)addToDictAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_AddToDict();
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)changeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Change();
}

- (IBAction)changeAllAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_ChangeAll();
}

- (IBAction)ignoreAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Ignore();
}

- (IBAction)ignoreAllAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_IgnoreAll();
}

- (IBAction)replacementChanged:(id)sender
{
	UT_UNUSED(sender);
	[_suggestionList deselectAll:self];
	_xap->event_ReplacementChanged();
}

- (void)suggestionSelected:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_SuggestionSelected([_suggestionList selectedRow], [_suggestionList selectedColumn]);
}

- (void)setMisspelled:(NSAttributedString*)attr scroll:(float)offset
{
	[_unknownData setString:@""];

	if (attr) {
		[_unknownData setEditable:YES];
		[_unknownData insertText:attr];
		[_unknownData setEditable:NO];

		NSClipView * clipView = (NSClipView *) [_unknownData superview];

		NSRect textFrame = [_unknownData bounds];
		NSRect clipFrame = [clipView bounds];

		textFrame.origin.x = 0.0f;
		textFrame.origin.y = offset * textFrame.size.height - clipFrame.size.height / 2.0f;

		if (textFrame.origin.y > textFrame.size.height - clipFrame.size.height)
			textFrame.origin.y = textFrame.size.height - clipFrame.size.height;

		if (textFrame.origin.y < 0)
			textFrame.origin.y = 0;

		[clipView scrollToPoint:textFrame.origin];
	}
}


- (void)setReplace:(NSString*)str
{
	[_replData setStringValue:str];	
}


- (void)selectSuggestion:(int)idx
{
	[_suggestionList selectRowIndexes:[NSIndexSet indexSetWithIndex:idx] byExtendingSelection:NO];
}

- (void)reloadSuggestionList
{
	[_suggestionList deselectAll:self];
	[_suggestionList reloadData];
	[[_suggestionList window] makeFirstResponder:_replData];
}

- (void)setSuggestionList:(id)list
{
	[_suggestionList setDataSource:list];
}

- (NSString*)replace
{
	return [_replData stringValue];
}

@end
