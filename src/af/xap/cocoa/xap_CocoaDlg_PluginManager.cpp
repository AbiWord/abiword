/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2003, 2007 Hubert Figuiere
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_FileOpenSaveAs.h"
#include "xap_CocoaDlg_PluginManager.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaModule.h"
#include "xap_CocoaPlugin.h"
#include "xap_Dialog_Id.h"
#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ie_types.h"

XAP_Dialog * XAP_CocoaDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_PluginManager * p = new XAP_CocoaDialog_PluginManager(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_PluginManager::XAP_CocoaDialog_PluginManager(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id dlgid)
	: XAP_Dialog_PluginManager(pDlgFactory, dlgid),
	  m_pFrame(NULL)
{
	// 
}

XAP_CocoaDialog_PluginManager::~XAP_CocoaDialog_PluginManager(void)
{
	// 
}

void XAP_CocoaDialog_PluginManager::event_Load ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_DialogFactory * pDialogFactory = (XAP_DialogFactory *) m_pFrame->getDialogFactory();
	
	XAP_Dialog_FileOpenSaveAs * pDialog = (XAP_Dialog_FileOpenSaveAs *) (pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return;
	
	std::string plugin_path = [[[NSBundle mainBundle] bundlePath] UTF8String];
//	plugin_path += "/Contents/PlugIns/AbiHack.dylib";

	pDialog->setCurrentPathname(plugin_path);
	pDialog->setSuggestFilename(false);
	
	const char * szDescList[2];
	const char * szSuffixList[2];
	IEFileType   nTypeList[2];

	szDescList[0]   = "AbiWord Plugin (.so)";
	szSuffixList[0] = "*.so";
	nTypeList[0]    = (IEFileType) 1;

	szDescList[1]   = 0;
	szSuffixList[1] = 0;
	nTypeList[1]    = 0;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType) 1);

	// todo: cd to the proper plugin directory
	
	pDialog->runModal(m_pFrame);
	
	if (pDialog->getAnswer() == XAP_Dialog_FileOpenSaveAs::a_OK)
	{
		const std::string & resultPathname = pDialog->getPathname();

		if (!resultPathname.empty())
		{
//			bool bIsBundle = false;

//			int length = strlen(szResultPathname);
//			if (length > 4)
//				if (strcmp(szResultPathname + length - 4, ".Abi") == 0)
//					bIsBundle = true;

			bool bActivated = activatePlugin(resultPathname.c_str());
//			if (bIsBundle)
//			{
//				XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
//				XAP_CocoaPlugin * plugin = [pController loadPlugin:[NSString stringWithUTF8String:szResultPathname]];
//				[[plugin delegate] pluginActivate];
//				bActivated =  true; // we don't really know if activation succeeded....
//			}
//			else bActivated = activatePlugin(szResultPathname);

			if (!bActivated)
			{
				/* error message
				 */
				m_pFrame->showMessageBox (pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD),
										  XAP_Dialog_MessageBox::b_O,
										  XAP_Dialog_MessageBox::a_OK);
			}
		}
	}
	pDialogFactory->releaseDialog(pDialog);
}

void XAP_CocoaDialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	m_pFrame = pFrame;	

	XAP_CocoaDlg_PluginManagerController * pDialog = [[XAP_CocoaDlg_PluginManagerController alloc] initFromNib];
	if (pDialog)
	{
		[pDialog setXAPOwner:this];

		NSWindow * window = [pDialog window];

		[NSApp runModalForWindow:window];

		[pDialog close];
		[pDialog release];
	}
}

/*****************************************************************/
/*****************************************************************/

@implementation XAP_CocoaPluginReference

- (id)initWithModule:(XAP_CocoaModule *)module
{
	if (![super init]) {
		return nil;
	}
	m_entry = 0;

	m_name = 0;
	m_author = 0;
	m_version = 0;
	m_description = 0;
	m_usage = 0;

	m_module = module;
	m_plugin = 0;

	if (!m_module) {
		[self release];
		return nil;
	}
	const XAP_ModuleInfo * mi = m_module->getModuleInfo();

	if (mi->name)
		m_name = [NSString stringWithUTF8String:(mi->name)];
	if (mi->author)
		m_author = [NSString stringWithUTF8String:(mi->author)];
	if (mi->version)
		m_version = [NSString stringWithUTF8String:(mi->version)];
	if (mi->desc)
		m_description = [NSString stringWithUTF8String:(mi->desc)];
	if (mi->usage)
		m_usage = [NSString stringWithUTF8String:(mi->usage)];

	if (m_name == 0)
		m_name = @"";
	if (m_author == 0)
		m_author = @"";
	if (m_version == 0)
		m_version = @"";
	if (m_description == 0)
		m_description = @"";
	if (m_usage == 0)
		m_usage = @"";

	[m_name retain];
	[m_author retain];
	[m_version retain];
	[m_description retain];
	[m_usage retain];

	m_entry = [[NSAttributedString alloc] initWithString:m_name];
	return self;
}

- (id)initWithPlugin:(XAP_CocoaPlugin *)plugin
{
	if (![super init]) {
		return nil;
	}
	m_entry = 0;

	m_name = 0;
	m_author = 0;
	m_version = 0;
	m_description = 0;
	m_usage = 0;

	m_module = 0;
	m_plugin = plugin;

	if (!m_plugin)
	{
		[self release];
		return nil;
	}
	else if (![m_plugin delegate])
	{
		[self release];
		return nil;
	}

	id <XAP_CocoaPluginDelegate> delegate = [m_plugin delegate];

	m_name        = [delegate pluginName];
	m_author      = [delegate pluginAuthor];
	m_version     = [delegate pluginVersion];
	m_description = [delegate pluginDescription];
	m_usage       = [delegate pluginUsage];

	if (m_name == 0)
		m_name = @"";
	if (m_author == 0)
		m_author = @"";
	if (m_version == 0)
		m_version = @"";
	if (m_description == 0)
		m_description = @"";
	if (m_usage == 0)
		m_usage = @"";

	[m_name retain];
	[m_author retain];
	[m_version retain];
	[m_description retain];
	[m_usage retain];

	NSDictionary * attr = 0;

	if ([delegate pluginIsActive])
		attr = [NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
	else
		attr = [NSDictionary dictionaryWithObject:[NSColor  grayColor] forKey:NSForegroundColorAttributeName];

	m_entry = [[NSAttributedString alloc] initWithString:m_name attributes:attr];
	return self;
}

- (void)dealloc
{
	[m_name release];
	[m_author release];
	[m_version release];
	[m_description release];
	[m_usage release];
	[m_entry release];
	[super dealloc];
}

- (void)setActive:(BOOL)active
{
	[m_entry release];
	m_entry = nil;

	NSDictionary * attr = 0;

	attr = [NSDictionary dictionaryWithObject:(active ? [NSColor blackColor] : [NSColor  grayColor]) forKey:NSForegroundColorAttributeName];

	m_entry = [[NSAttributedString alloc] initWithString:m_name attributes:attr];
}

- (NSAttributedString *)entry
{
	return m_entry;
}

- (NSString *)name
{
	return m_name;
}

- (NSString *)author
{
	return m_author;
}

- (NSString *)version
{
	return m_version;
}

- (NSString *)description
{
	return m_description;
}

- (NSString *)usage
{
	return m_usage;
}

- (XAP_CocoaModule *)module
{
	return m_module;
}

- (XAP_CocoaPlugin *)plugin
{
	return m_plugin;
}

@end

@implementation XAP_CocoaDlg_PluginManagerController

- (id)initFromNib
{
	if (![super initWithWindowNibName:@"xap_CocoaDlg_PluginManager"]) {
		return nil;
	}
	m_PluginRefs = [[NSMutableArray alloc] initWithCapacity:16];
	return self;
}

- (void)dealloc
{
	[m_PluginRefs release];
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog*)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_PluginManager*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = nil;
}

- (void)reloadPluginList
{
	[m_PluginRefs removeAllObjects];

	const UT_GenericVector<XAP_Module*> * pModuleVec = XAP_ModuleManager::instance().enumModules();

	UT_uint32 count = pModuleVec->getItemCount();

	for (UT_uint32 i = 0; i < count; i++) {
		if (XAP_CocoaModule * pModule = static_cast<XAP_CocoaModule *>(pModuleVec->getNthItem(i))) {
			XAP_CocoaPluginReference * pRef = [[XAP_CocoaPluginReference alloc] initWithModule:pModule];
			if (pRef)
			{
				[m_PluginRefs addObject:pRef];
				[pRef release];
			}
		}
	}

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	NSArray * Plugins = [pController plugins];

	unsigned plugin_count = [Plugins count];

	for (unsigned i = 0; i < plugin_count; i++)
	{
		XAP_CocoaPlugin * pPlugin = (XAP_CocoaPlugin *) [Plugins objectAtIndex:i];
		XAP_CocoaPluginReference * pRef = [[XAP_CocoaPluginReference alloc] initWithPlugin:pPlugin];
		if (pRef)
		{
			[m_PluginRefs addObject:pRef];
			[pRef release];
		}
	}
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	LocalizeControl([self window],		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE);

	LocalizeControl(oInstallBtn,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL);

	LocalizeControl(oCloseBtn,			pSS, XAP_STRING_ID_DLG_Close);

	LocalizeControl(oNameLabel,			pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME);
	LocalizeControl(oAuthorLabel,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR);
	LocalizeControl(oVersionLabel,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION);
	LocalizeControl(oDescriptionLabel,	pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC);
//	LocalizeControl(oUsageLabel,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_ ?? );

	[self reloadPluginList];

	[oPluginList setDelegate:self];
	[oPluginList setDataSource:self];

	[oNameData        setStringValue:@""];
	[oAuthorData      setStringValue:@""];
	[oVersionData     setStringValue:@""];
	[oDescriptionData setStringValue:@""];
	[oUsageData       setStringValue:@""];

	if ([m_PluginRefs count])
	{
		[oPluginList selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
		[self tableViewSelectionDidChange:nil];
	}
}

- (IBAction)closeAction:(id)sender
{
	UT_UNUSED(sender);
	[NSApp stopModal];
}

- (IBAction)installAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Load();

	[self reloadPluginList];

	[oPluginList deselectAll:self];
	[oPluginList reloadData];
}

/* NSTableView delegate method
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	int selection = [oPluginList selectedRow];
	if (selection >= 0)
	{
		XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:selection];

		[oNameData        setStringValue:[pRef name       ]];
		[oAuthorData      setStringValue:[pRef author     ]];
		[oVersionData     setStringValue:[pRef version    ]];
		[oDescriptionData setStringValue:[pRef description]];
		[oUsageData       setStringValue:[pRef usage      ]];
	}
	else
	{
		[oNameData        setStringValue:@""];
		[oAuthorData      setStringValue:@""];
		[oVersionData     setStringValue:@""];
		[oDescriptionData setStringValue:@""];
		[oUsageData       setStringValue:@""];
	}
}

/* NSTableDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	UT_UNUSED(aTableView);
	return (int) [m_PluginRefs count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:rowIndex];

	return [pRef entry];
}

@end
