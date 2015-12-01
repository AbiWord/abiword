/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2007 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifndef XAP_COCOADIALOG_PLUGIN_MANAGER_H
#define XAP_COCOADIALOG_PLUGIN_MANAGER_H

#include <Cocoa/Cocoa.h>

#include "xap_App.h"
#include "xap_CocoaPlugin.h"
#include "xap_Dlg_PluginManager.h"
#include "xap_Module.h"

#import "xap_Cocoa_NSTableUtils.h"

class XAP_Frame;
class XAP_CocoaDialog_PluginManager;
class XAP_CocoaModule;

@interface XAP_CocoaPluginReference : NSObject
{
	NSAttributedString *	m_entry;

	NSString *				m_name;
	NSString *				m_author;
	NSString *				m_version;
	NSString *				m_description;
	NSString *				m_usage;

	XAP_CocoaModule *		m_module;
	XAP_CocoaPlugin *		m_plugin;
}
- (id)initWithModule:(XAP_CocoaModule *)module;
- (id)initWithPlugin:(XAP_CocoaPlugin *)plugin;
- (void)dealloc;

- (void)setActive:(BOOL)active;

- (NSAttributedString *)entry;

- (NSString *)name;
- (NSString *)author;
- (NSString *)version;
- (NSString *)description;
- (NSString *)usage;

- (XAP_CocoaModule *)module;
- (XAP_CocoaPlugin *)plugin;
@end

@interface XAP_CocoaDlg_PluginManagerController
    : NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate, NSTableViewDataSource>
{
	IBOutlet NSButton *		oInstallBtn;
	IBOutlet NSButton *		oCloseBtn;

	IBOutlet NSTableView *	oPluginList;

	IBOutlet NSTextField *	oNameData;
	IBOutlet NSTextField *	oNameLabel;
	IBOutlet NSTextField *	oAuthorData;
	IBOutlet NSTextField *	oAuthorLabel;
	IBOutlet NSTextField *	oVersionData;
	IBOutlet NSTextField *	oVersionLabel;
	IBOutlet NSTextField *	oDescriptionData;
	IBOutlet NSTextField *	oDescriptionLabel;
	IBOutlet NSTextField *	oUsageData;
	IBOutlet NSTextField *	oUsageLabel;

	XAP_CocoaDialog_PluginManager*	_xap;

	NSMutableArray *	m_PluginRefs;
}
- (id)initFromNib;
- (void)dealloc;

- (IBAction)closeAction:(id)sender;
- (IBAction)installAction:(id)sender;

/* NSTableView delegate method
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;

/* NSTableDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

@end

class XAP_CocoaDialog_PluginManager : public XAP_Dialog_PluginManager
{
public:
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	XAP_CocoaDialog_PluginManager(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_PluginManager(void);

	virtual void	runModal (XAP_Frame * pFrame);

	void			event_Load ();

private:
	XAP_Frame *		m_pFrame;
};

#endif /* PLUGIN_MANAGER_H */
