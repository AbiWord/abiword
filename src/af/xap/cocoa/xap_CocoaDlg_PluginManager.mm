/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
	
	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);
	
	const char * szDescList[3];
	const char * szSuffixList[3];
	IEFileType   nTypeList[3];

	szDescList[0]   = "AbiWord Plugin (.so-abi)";
	szSuffixList[0] = "*.so-abi";
	nTypeList[0]    = (IEFileType) 1;

	szDescList[1]   = "AbiWord Bundle-Plugin (.Abi)";
	szSuffixList[1] = "*.Abi";
	nTypeList[1]    = (IEFileType) 1;
	
	szDescList[2]   = 0;
	szSuffixList[2] = 0;
	nTypeList[2]    = 0;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType) 1);

	// todo: cd to the proper plugin directory
	
	pDialog->runModal(m_pFrame);
	
	if (pDialog->getAnswer() == XAP_Dialog_FileOpenSaveAs::a_OK)
		{
			const char * szResultPathname = pDialog->getPathname();

			if (szResultPathname && *szResultPathname)
				{
					bool bIsBundle = false;

					int length = strlen(szResultPathname);
					if (length > 4)
						if (strcmp(szResultPathname + length - 4, ".Abi") == 0)
							bIsBundle = true;

					bool bActivated;
					if (bIsBundle)
						{
							XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
							XAP_CocoaPlugin * plugin = [pController loadPlugin:[NSString stringWithUTF8String:szResultPathname]];
							bActivated = [[plugin delegate] pluginActivate] ? true : false;
						}
					else bActivated = activatePlugin(szResultPathname);

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
	if (self = [super init])
		{
			m_entry = 0;

			m_name = 0;
			m_author = 0;
			m_version = 0;
			m_description = 0;
			m_usage = 0;

			m_module = module;
			m_plugin = 0;

			if (!m_module)
				{
					[self release];
					self = 0;
				}
		}
	if (self)
		{
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
		}
	return self;
}

- (id)initWithPlugin:(XAP_CocoaPlugin *)plugin
{
	if (self = [super init])
		{
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
					self = 0;
				}
			else if (![m_plugin delegate])
				{
					[self release];
					self = 0;
				}
		}
	if (self)
		{
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
		}
	return self;
}

- (void)dealloc
{
	if (m_name)
		{
			[m_name release];
			m_name = 0;
		}
	if (m_author)
		{
			[m_author release];
			m_author = 0;
		}
	if (m_version)
		{
			[m_version release];
			m_version = 0;
		}
	if (m_description)
		{
			[m_description release];
			m_description = 0;
		}
	if (m_usage)
		{
			[m_usage release];
			m_usage = 0;
		}
	if (m_entry)
		{
			[m_entry release];
			m_entry = 0;
		}
	[super dealloc];
}

- (void)setActive:(BOOL)active
{
	if (m_entry)
		{
			[m_entry release];
			m_entry = 0;
		}

	NSDictionary * attr = 0;

	if (active)
		attr = [NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
	else
		attr = [NSDictionary dictionaryWithObject:[NSColor  grayColor] forKey:NSForegroundColorAttributeName];

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
	if (self = [super initWithWindowNibName:@"xap_CocoaDlg_PluginManager"])
		{
			m_PluginRefs = [[NSMutableArray alloc] initWithCapacity:16];
			if (!m_PluginRefs)
				{
					[self dealloc];
					self = 0;
				}
		}
	return self;
}

- (void)dealloc
{
	if (m_PluginRefs)
		{
			[m_PluginRefs release];
			m_PluginRefs = 0;
		}
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

	for (UT_uint32 i = 0; i < count; i++)
		if (XAP_CocoaModule * pModule = static_cast<XAP_CocoaModule *>(pModuleVec->getNthItem(i)))
			if (XAP_CocoaPluginReference * pRef = [[XAP_CocoaPluginReference alloc] initWithModule:pModule])
				{
					[m_PluginRefs addObject:pRef];
				}

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	NSArray * Plugins = [pController plugins];

	unsigned plugin_count = [Plugins count];

	for (unsigned i = 0; i < plugin_count; i++)
		{
			XAP_CocoaPlugin * pPlugin = (XAP_CocoaPlugin *) [Plugins objectAtIndex:i];

			if (XAP_CocoaPluginReference * pRef = [[XAP_CocoaPluginReference alloc] initWithPlugin:pPlugin])
				{
					[m_PluginRefs addObject:pRef];
				}
		}

	if ([m_PluginRefs count])
		[oDeactivateAllBtn setEnabled:YES];
	else
		[oDeactivateAllBtn setEnabled:NO];
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	LocalizeControl([self window],		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE);

//	LocalizeControl(oActivateBtn,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVATE /* TODO */);
	LocalizeControl(oDeactivateBtn,		pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE);
	LocalizeControl(oDeactivateAllBtn,	pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL);
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

	[oActivateBtn   setEnabled:NO];
	[oDeactivateBtn setEnabled:NO];

	[oForceDeactivation setEnabled:NO];
	[oForceDeactivation setState:NSOffState];

	[oNameData        setStringValue:@""];
	[oAuthorData      setStringValue:@""];
	[oVersionData     setStringValue:@""];
	[oDescriptionData setStringValue:@""];
	[oUsageData       setStringValue:@""];

	if ([m_PluginRefs count])
		{
			[oPluginList selectRow:0 byExtendingSelection:NO];
			[self tableViewSelectionDidChange:nil];
		}
}

- (IBAction)closeAction:(id)sender
{
	[NSApp stopModal];
}

- (IBAction)activateAction:(id)sender
{
	int selection = [oPluginList selectedRow];
	if (selection >= 0)
		{
			XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:selection];

			if (XAP_CocoaPlugin * pPlugin = [pRef plugin])
				if ([[pPlugin delegate] pluginActivate])
					{
						[oForceDeactivation setEnabled:YES];

						[pRef setActive:YES];

						[oPluginList setNeedsDisplay:YES];
						[oPluginList displayIfNeeded];

						[oActivateBtn   setEnabled:NO];
						[oDeactivateBtn setEnabled:YES];
					}
		}
}

- (IBAction)deactivateAction:(id)sender
{
	int selection = [oPluginList selectedRow];
	if (selection >= 0)
		{
			XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:selection];

			if (XAP_CocoaPlugin * pPlugin = [pRef plugin])
				{
					XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

					BOOL bForce = ([oForceDeactivation state] == NSOnState) ? YES : NO;

					if ([pController deactivatePlugin:pPlugin overridePlugin:bForce])
						{
							[oForceDeactivation setEnabled:NO];
							[oForceDeactivation setState:NSOffState];

							[pRef setActive:NO];

							[oPluginList setNeedsDisplay:YES];
							[oPluginList displayIfNeeded];

							[oActivateBtn   setEnabled:YES];
							[oDeactivateBtn setEnabled:NO];
						}
				}
			else if (XAP_CocoaModule * pModule = [pRef module])
				{
					XAP_ModuleManager::instance().unloadModule(pModule);

					[self reloadPluginList];

					[oPluginList deselectAll:self];
					[oPluginList reloadData];

					[self tableViewSelectionDidChange:nil];
				}
		}
}

- (IBAction)deactivateAllAction:(id)sender
{
	XAP_ModuleManager::instance().unloadAllPlugins();

	[self reloadPluginList];

	[oPluginList deselectAll:self];
	[oPluginList reloadData];

	[self tableViewSelectionDidChange:nil];
}

- (IBAction)installAction:(id)sender
{
	_xap->event_Load();

	[self reloadPluginList];

	[oPluginList deselectAll:self];
	[oPluginList reloadData];
}

/* NSTableView delegate method
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	int selection = [oPluginList selectedRow];
	if (selection >= 0)
		{
			XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:selection];

			[oNameData        setStringValue:[pRef name       ]];
			[oAuthorData      setStringValue:[pRef author     ]];
			[oVersionData     setStringValue:[pRef version    ]];
			[oDescriptionData setStringValue:[pRef description]];
			[oUsageData       setStringValue:[pRef usage      ]];

			if (XAP_CocoaPlugin * pPlugin = [pRef plugin])
				{
					if ([[pPlugin delegate] pluginIsActive])
						{
							[oActivateBtn   setEnabled:NO];
							[oDeactivateBtn setEnabled:YES];

							[oForceDeactivation setEnabled:YES];
						}
					else
						{
							[oActivateBtn   setEnabled:YES];
							[oDeactivateBtn setEnabled:NO];

							[oForceDeactivation setEnabled:NO];
						}
				}
			else
				{
					[oActivateBtn   setEnabled:NO];
					[oDeactivateBtn setEnabled:YES];

					[oForceDeactivation setEnabled:NO];
				}
			[oForceDeactivation setState:NSOffState];
		}
	else
		{
			[oNameData        setStringValue:@""];
			[oAuthorData      setStringValue:@""];
			[oVersionData     setStringValue:@""];
			[oDescriptionData setStringValue:@""];
			[oUsageData       setStringValue:@""];

			[oActivateBtn   setEnabled:NO];
			[oDeactivateBtn setEnabled:NO];

			[oForceDeactivation setEnabled:NO];
			[oForceDeactivation setState:NSOffState];
		}
}

/* NSTableDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return (int) [m_PluginRefs count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	XAP_CocoaPluginReference * pRef = (XAP_CocoaPluginReference *) [m_PluginRefs objectAtIndex:rowIndex];

	return [pRef entry];
}

@end
