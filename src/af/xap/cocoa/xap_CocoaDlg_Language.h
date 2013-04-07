/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2005 Francis James Franklin
 * Copyright (C) 2001, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_LANGUAGE_H
#define XAP_COCOADIALOG_LANGUAGE_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_Language.h"

@class XAP_CocoaDialog_Language_Controller;

/*****************************************************************/

class XAP_CocoaDialog_Language : public XAP_Dialog_Language
{
public:
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	XAP_CocoaDialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_Language(void);

	virtual void			runModal(XAP_Frame * pFrame);

	const char * getNthLanguage (UT_uint32 n) const
	{
		return (n < m_iLangCount) ? ((const char *) m_ppLanguages[n]) : 0;
	}
	UT_uint32				getLanguageCount() const { return m_iLangCount; }

	const char *			getCurrentLanguage() const { return (const char *) m_pLanguage; }

private:
	XAP_CocoaDialog_Language_Controller *	m_dlg;
};

@interface XAP_CocoaDialog_Language_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSBox    *     _selectLanguageBox;
	IBOutlet NSButton *		_cancelBtn;
	IBOutlet NSButton *		_okBtn;
	IBOutlet NSButton *		_documentDefaultBtn;

	IBOutlet NSTextField *	_documentCurrentLabel;

	IBOutlet NSTableView *	_languageTable;

	NSMutableArray *		m_Languages;

	NSString *				m_Selection;
	int						m_SelectionIndex;

	BOOL					m_bApplyToDocument;

	XAP_CocoaDialog_Language *	_xap;
}

- (id)initFromNib;
- (void)dealloc;
- (void)setXAPOwner:(XAP_Dialog *)owner;
- (void)discardXAP;
- (void)windowDidLoad;

- (NSString *)selectedLanguage;
- (int)indexOfSelectedLanguage;
- (BOOL)applyToDocument;

- (IBAction)aCancel:(id)sender;
- (IBAction)aOK:(id)sender;
- (IBAction)aLanguageTable:(id)sender;

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

@end

#endif /* XAP_COCOADIALOG_LANGUAGE_H */
