/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_CocoaDlg_PluginManager.h"
#include "xap_CocoaDlg_FileOpenSaveAs.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ie_types.h"

#import "xap_Cocoa_NSTableUtils.h"

/*****************************************************************/
/*****************************************************************/

static void _errorMessage (XAP_Frame * pFrame, const char * msg)
{
	// just a little simple error message box

	pFrame->showMessageBox (msg,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);
}

/*****************************************************************/
/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_PluginManager * p = new XAP_CocoaDialog_PluginManager(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_PluginManager::XAP_CocoaDialog_PluginManager(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_PluginManager(pDlgFactory, dlgid),
		m_pFrame(NULL),
		m_dlg(nil),
		m_dataSource(nil)
{
}

XAP_CocoaDialog_PluginManager::~XAP_CocoaDialog_PluginManager(void)
{
	[m_dataSource release];
}

/*****************************************************************/
/*****************************************************************/
void XAP_CocoaDialog_PluginManager::event_Close()
{
	[NSApp stopModal];
}


void XAP_CocoaDialog_PluginManager::event_DeactivateAll()
{
	deactivateAllPlugins ();
	_refreshAll ();
}

void XAP_CocoaDialog_PluginManager::event_Deactivate()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_Module * pModule = NULL;

	int selectedRow = [m_dlg selectedPlugin];
	if (selectedRow != -1) {
		pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(selectedRow);
	} 
	else {
		// error message box - didn't select a plugin
		_errorMessage (m_pFrame, 
					   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED));
		return;
	}

	if (pModule) {
		if (deactivatePlugin (pModule)) {
			// worked
			_refreshAll ();
		}
		else {
			// error message box
			_errorMessage (m_pFrame, 
						   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD));
		}
	}
	else {
		// error message box
		_errorMessage (m_pFrame, 
					   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD));
	}
}

void XAP_CocoaDialog_PluginManager::event_Load ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_ASSERT(pDialog);
	
	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);
	
	UT_uint32 filterCount = 1;
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	
	// we probably shouldn't hardcode this
	// HP-UX uses .sl, for instance
	szDescList[0] = "AbiWord Plugin (.so)";
	szSuffixList[0] = "*.so";
	nTypeList[0] = (IEFileType)1;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType)1);

	// todo: cd to the proper plugin directory
	
	pDialog->runModal(m_pFrame);
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			if (activatePlugin (szResultPathname))
			{
				// worked!
				_refreshAll ();
			}
			else
			{
				// error message
				_errorMessage (m_pFrame, 
							   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD));
			}
		}
	}
	
	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

void XAP_CocoaDialog_PluginManager::event_Select1 ()
{
	_refreshTab2 ();
}

/*****************************************************************/
/*****************************************************************/

void XAP_CocoaDialog_PluginManager::_refreshAll ()
{
	_refreshTab1();

	[m_dlg setSelectedPlugin:0];

	_refreshTab2();
}

void XAP_CocoaDialog_PluginManager::_refreshTab1 ()
{
	XAP_Module * pModule = NULL;

	const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();

	[(NSMutableArray*)[m_dataSource array] removeAllObjects];
	for (UT_uint32 i = 0; i < pVec->size(); i++) {
		pModule = (XAP_Module *)pVec->getNthItem (i);
		NSString* str = [[NSString alloc ] initWithUTF8String:pModule->getModuleInfo()->name];
		[m_dataSource addString:str];
		[str release];
	}
}

void XAP_CocoaDialog_PluginManager::_refreshTab2 ()
{
	XAP_Module * pModule = 0;
	int selectedRow = [m_dlg selectedPlugin];
	if (selectedRow != -1) {
		pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(selectedRow);
	}

	if (pModule)
	{
		const XAP_ModuleInfo * mi = pModule->getModuleInfo ();
		[m_dlg setModuleInfo:mi];
	}
	else {
		[m_dlg setModuleInfo:NULL];
	}
}

/*****************************************************************/
/*****************************************************************/


void XAP_CocoaDialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	NSWindow* window;
	m_pFrame = pFrame;	

	m_dlg = [[XAP_CocoaDlg_PluginManagerController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	m_dataSource = [[XAP_StringListDataSource alloc] init];

	window = [m_dlg window];
	[m_dlg setDataSource:m_dataSource];	// not retained
	[m_dlg setSelectedPlugin:0];
	
	_refreshAll();

	[NSApp runModalForWindow:window];

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	[m_dataSource release];
	m_dataSource = nil;
}

/*****************************************************************/
/*****************************************************************/

@implementation XAP_CocoaDlg_PluginManagerController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_PluginManager"];
	return self;
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


- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	LocalizeControl(_deactivateBtn, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE);
	LocalizeControl(_deactivateAllBtn, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL);
	LocalizeControl(_installBtn, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL);
	LocalizeControl(_nameData, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME);
	LocalizeControl(_descriptionData, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC);
	LocalizeControl(_authorData, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR);
	LocalizeControl(_versionData, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION);
	LocalizeControl(_detailsBox, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS);
	LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE);
	LocalizeControl(_closeBtn, pSS, XAP_STRING_ID_DLG_Close);
}


- (IBAction)closeAction:(id)sender
{
	_xap->event_Close();
}

- (IBAction)deactivateAction:(id)sender
{
	_xap->event_Deactivate();
}

- (IBAction)deactivateAllAction:(id)sender
{
	_xap->event_DeactivateAll();
}

- (IBAction)installAction:(id)sender
{
	_xap->event_Load();
}

- (IBAction)selectAction:(id)sender
{
	_xap->event_Select1();
}

- (int)selectedPlugin
{
	return [_pluginList selectedRow];
}


- (void)setSelectedPlugin:(int)idx
{
	[_pluginList selectRow:idx byExtendingSelection:NO];
}


- (void)setModuleInfo:(const XAP_ModuleInfo*)info
{
	if (info) {
		[_nameData setStringValue:[NSString stringWithUTF8String:info->name]];
		[_authorData setStringValue:[NSString stringWithUTF8String:info->author]];
		[_descriptionData setStringValue:[NSString stringWithUTF8String:info->desc]];
		[_versionData setStringValue:[NSString stringWithUTF8String:info->version]];	
	}
	else {
		[_nameData setStringValue:@""];
		[_authorData setStringValue:@""];
		[_descriptionData setStringValue:@""];
		[_versionData setStringValue:@""];
	}
}

- (void)setDataSource:(XAP_StringListDataSource*)source
{
	UT_ASSERT(_pluginList);
	[_pluginList setDataSource:source];
}

@end
