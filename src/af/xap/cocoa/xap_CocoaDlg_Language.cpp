/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2003 Hubert Figuiere
 * Copyright (c) 2005 Francis James Franklin
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_App.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_Language.h"

/* *************************************************************** */

XAP_Dialog * XAP_CocoaDialog_Language::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Language * p = new XAP_CocoaDialog_Language(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_Language::XAP_CocoaDialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_Language(pDlgFactory, dlgid),
	m_dlg(nil)
{
	// 
}

XAP_CocoaDialog_Language::~XAP_CocoaDialog_Language(void)
{
	// 
}

void XAP_CocoaDialog_Language::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[XAP_CocoaDialog_Language_Controller alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	[NSApp runModalForWindow:[m_dlg window]];

	int index = [m_dlg indexOfSelectedLanguage];

	if (index >= 0)
	{
		m_answer = XAP_Dialog_Language::a_OK;

		setMakeDocumentDefault([m_dlg applyToDocument] == YES);

		_setLanguage(m_ppLanguages[index]);

		m_bChangedLanguage = true;
	}
	else
	{
		m_answer = XAP_Dialog_Language::a_CANCEL;
	}

	[m_dlg close];
	[m_dlg release];

	m_dlg = nil;
}

/* **************************************************************************************** */

@implementation XAP_CocoaDialog_Language_Controller

- (id)initFromNib
{
	if (![super initWithWindowNibName:@"xap_CocoaDlg_Language"]) {
		return nil;
	}
	_xap = 0;

	m_Selection = 0;
	m_SelectionIndex = -1;

	m_bApplyToDocument = NO;

	m_Languages = [[NSMutableArray alloc] initWithCapacity:100];
	return self;
}

- (void)dealloc
{
	[m_Languages release];
	[m_Selection release];
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
if (m_Selection)
	{
		[m_Selection release];
		m_Selection = nil;
	}
	m_SelectionIndex = -1;

	_xap = static_cast<XAP_CocoaDialog_Language *>(owner);
}

- (void)discardXAP
{
	if (m_Selection)
	{
		[m_Selection release];
		m_Selection = nil;
	}
	m_SelectionIndex = -1;

	_xap = 0;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window],     pSS, XAP_STRING_ID_DLG_ULANG_LangTitle);

	LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
	LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
	LocalizeControl(_selectLanguageBox, pSS, XAP_STRING_ID_DLG_ULANG_AvailableLanguages);

	LocalizeControl(_documentDefaultBtn,  pSS, XAP_STRING_ID_DLG_ULANG_DefaultLangChkbox);

	if (_xap)
	{
		UT_UTF8String defaultLanguageString;
		_xap->getDocDefaultLangDescription(defaultLanguageString);
		[_documentCurrentLabel setStringValue:[NSString stringWithUTF8String:(defaultLanguageString.utf8_str())]];

		const char * current_language = _xap->getCurrentLanguage();

		int current_index = -1;

		UT_uint32 count = _xap->getLanguageCount();

		for (UT_uint32 i = 0; i < count; i++)
		{
			const char * language = _xap->getNthLanguage(i);

			[m_Languages addObject:[NSString stringWithUTF8String:language]];

			if (current_language && (g_ascii_strcasecmp(current_language, language) == 0))
			{
				current_index = (int) i;
			}
		}
		[_languageTable reloadData];

		if (current_index >= 0)
		{
			[_languageTable selectRowIndexes:[NSIndexSet indexSetWithIndex:current_index] byExtendingSelection:NO];
			[_languageTable scrollRowToVisible:current_index];
		}
	}
}

- (NSString *)selectedLanguage
{
	return m_Selection;
}

- (int)indexOfSelectedLanguage
{
	return m_SelectionIndex;
}

- (BOOL)applyToDocument
{
	return m_bApplyToDocument;
}

- (IBAction)aCancel:(id)sender
{
	UT_UNUSED(sender);
	if (m_Selection)
	{
		[m_Selection release];
		m_Selection = 0;
	}
	m_SelectionIndex = -1;

	[NSApp stopModal];
}

- (IBAction)aOK:(id)sender
{
	UT_UNUSED(sender);
	m_bApplyToDocument = ([_documentDefaultBtn state] == NSOnState) ? YES : NO;

	[NSApp stopModal];
}

- (IBAction)aLanguageTable:(id)sender
{
	UT_UNUSED(sender);
	if (m_Selection)
	{
		[m_Selection release];
		m_Selection = nil;
	}
	m_SelectionIndex = [_languageTable selectedRow];

	if (m_SelectionIndex >= 0)
	{
		if ((unsigned) m_SelectionIndex < [m_Languages count])
		{
			m_Selection = [[m_Languages objectAtIndex:((unsigned) m_SelectionIndex)] retain];
		}
		else // huh?
		{
			m_SelectionIndex = -1;
		}
	}
}

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	UT_UNUSED(aTableView);
	return (int) [m_Languages count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	return [m_Languages objectAtIndex:((unsigned) rowIndex)];
}

@end
