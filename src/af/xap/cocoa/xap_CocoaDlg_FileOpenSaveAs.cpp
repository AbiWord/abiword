/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2009 Hubert Figuiere
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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_string.h"

#include "xap_CocoaDlg_FileOpenSaveAs.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"
#include "xap_Strings.h"

#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_types.h"


#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100

@implementation XAP_OpenSavePanel_AccessoryController

- (id)initWithXAP:(XAP_CocoaDialog_FileOpenSaveAs*)xap
{
	if (![super init]) {
		return nil;
	}
	_xap = xap;
	return self;
}

- (NSView *)fileTypeAccessoryView
{
	return oFTAccessoryView;
}

- (void)setFileTypeLabel:(const std::string &)label
{
	SetNSControlLabel(oFTLabel, label);
}

- (void)setSelectedFileType:(int)type
{
	[oFTPopUp selectItem:[[oFTPopUp menu] itemWithTag:type]];
}

- (void)removeItemsOfFileTypesMenu
{
	[oFTPopUp removeAllItems];
}

- (void)addItemWithTitle:(NSString *)title fileType:(int)type
{
	[oFTPopUp addItemWithTitle:title];
	[[oFTPopUp lastItem] setTag:type];
}

- (IBAction)selectFileType:(id)sender
{
	UT_UNUSED(sender);
	_xap->_setSelectedFileType([[sender selectedItem] tag]);
}

// delegate method
#if 0
- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename
{
	
	return YES;
}
#endif

@end

/*****************************************************************/
static void addSuffixesToFileTypes(NSMutableArray * fileTypes, const char * suffix_list)
{
	std::string suffix;
	while (const char * ptr1 = strstr(suffix_list, "*.")) {
		ptr1 += 2;
		bool bSingleExtension = true;
		const char * ptr2 = ptr1;
		while (*ptr2 && (*ptr2 != ';')) {
			if (*ptr2++ == '.') {
				bSingleExtension = false;
			}
		}
		suffix.assign(ptr1, ptr2 - ptr1);
		if (bSingleExtension) {
			UT_DEBUGMSG(("added suffix %s\n", suffix.c_str()));
			[fileTypes addObject:[NSString stringWithUTF8String:(suffix.c_str())]];
		}
		suffix_list = ptr2;
	}
}


XAP_Dialog * XAP_CocoaDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FileOpenSaveAs * p = new XAP_CocoaDialog_FileOpenSaveAs(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FileOpenSaveAs::XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_FileOpenSaveAs(pDlgFactory,dlgid),
	m_accessoryViewsController(nil),
	m_panel(nil),
	m_fileTypes(nil),
	m_szFileTypeDescription(0),
	m_szFileTypeCount(0)
{
	// 
}

XAP_CocoaDialog_FileOpenSaveAs::~XAP_CocoaDialog_FileOpenSaveAs(void)
{
	if (m_accessoryViewsController)
	{
		[m_accessoryViewsController release];
	}
	if (m_fileTypes)
	{
		[m_fileTypes release];
	}
}


NSSavePanel * XAP_CocoaDialog_FileOpenSaveAs::_makeOpenPanel()
{
	NSOpenPanel *openPanel = [[NSOpenPanel openPanel] retain];

	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setCanChooseDirectories:NO];
	[openPanel setCanChooseFiles:YES];
	// this call is 10.3 only.
	if ([openPanel respondsToSelector:@selector(setCanCreateDirectories:)])
		[openPanel setCanCreateDirectories:NO];

	[openPanel setCanSelectHiddenExtension:NO];
	[openPanel setExtensionHidden:NO];
	
	for (UT_uint32 i = 0; i < m_szFileTypeCount; i++) {
		addSuffixesToFileTypes(m_fileTypes,  m_szSuffixes[i]);
	}
	if([m_fileTypes count]) {
		[openPanel setAllowedFileTypes:m_fileTypes];
	}
	else {
		[openPanel setAllowedFileTypes:nil];	
	}
	return openPanel;
}



NSSavePanel * XAP_CocoaDialog_FileOpenSaveAs::_makeSavePanel(const std::string & fileTypeLabel)
{
	NSSavePanel * savePanel = [[NSSavePanel savePanel] retain];

	// this call is 10.3 only.
	if ([savePanel respondsToSelector:@selector(setCanCreateDirectories:)])
		[savePanel setCanCreateDirectories:YES];

	[savePanel setCanSelectHiddenExtension:YES];
	[savePanel setExtensionHidden:NO];
	[m_accessoryViewsController setFileTypeLabel:fileTypeLabel];
	
	/* File-types PopUp:
	 */
	[m_accessoryViewsController removeItemsOfFileTypesMenu];

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	std::string label;
	if (pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect, label)) {
		NSString * title = [NSString stringWithUTF8String:(label.c_str())];
		int type = (int) XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO;
		[m_accessoryViewsController addItemWithTitle:title fileType:type];
	}

	UT_sint32 defaultFileType = m_nDefaultFileType;

	for (UT_uint32 i = 0; i < m_szFileTypeCount; i++) {
		NSString * title = [NSString stringWithUTF8String:m_szDescriptions[i]];

		int type = m_nTypeList[i];
		
		[m_accessoryViewsController addItemWithTitle:title fileType:type];
	}

	[m_accessoryViewsController setSelectedFileType:defaultFileType];
	_setSelectedFileType(defaultFileType);
	
	[savePanel setAccessoryView:[m_accessoryViewsController fileTypeAccessoryView]];
	return savePanel;
}

/*****************************************************************/

void XAP_CocoaDialog_FileOpenSaveAs::runModal(XAP_Frame * /*pFrame*/)
{
	if (!m_accessoryViewsController) {
		m_accessoryViewsController = [[XAP_OpenSavePanel_AccessoryController alloc] initWithXAP:this];
		if (m_accessoryViewsController) {
			if (![NSBundle loadNibNamed:@"xap_CocoaFileOpen_Views" owner:m_accessoryViewsController]) {
				NSLog (@"Couldn't load nib xap_CocoaFileOpen_Views");
				[m_accessoryViewsController release];
				m_accessoryViewsController = 0;
			}
		}
	}
	UT_ASSERT(m_accessoryViewsController);
	if (!m_accessoryViewsController)
		return;

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	// do we want to let this function handle stating the Cocoa
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;
	bool bOpenPanel = false;
	bool bSavePanel = false;

	std::string szTitle;
	std::string szFileTypeLabel;

	switch (m_id)
	{
	case XAP_DIALOG_ID_INSERTMATHML:
		bOpenPanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertMath, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertMath, szFileTypeLabel);

		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_INSERT_PICTURE:
		bOpenPanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_FILE_OPEN:
		bOpenPanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_FILE_SAVEAS:
	case XAP_DIALOG_ID_FILE_SAVE_IMAGE:
		bSavePanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel, szFileTypeLabel);

		bCheckWritePermission = true;
		break;
	case XAP_DIALOG_ID_FILE_IMPORT:
		bOpenPanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_FILE_EXPORT:
		bSavePanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel, szFileTypeLabel);

		bCheckWritePermission = true;
		break;
	case XAP_DIALOG_ID_INSERT_FILE:
		bOpenPanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_PRINTTOFILE:
		bSavePanel = true;

		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle, szTitle);
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel, szFileTypeLabel);

		bCheckWritePermission = true;
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (!m_fileTypes)
		m_fileTypes = [[NSMutableArray alloc] initWithCapacity:4];
	UT_ASSERT(m_fileTypes);
	if (!m_fileTypes)
		return;

	m_panel = nil;

	// TODO WTF is this not in XP land?
	m_szFileTypeCount = g_strv_length((gchar **) m_szDescriptions);

	UT_ASSERT(g_strv_length((gchar **) m_szSuffixes) == m_szFileTypeCount);

	if (bOpenPanel) {
		m_panel = _makeOpenPanel();
	}
	else if (bSavePanel) {
		m_panel = _makeSavePanel(szFileTypeLabel);
	}
	UT_ASSERT(m_panel);
	if (!m_panel)
		return;


	[m_panel setTitle:[NSString stringWithUTF8String:(szTitle.c_str())]];
	[m_panel setDelegate:m_accessoryViewsController];


	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	NSString * szPersistDirectory = nil;
	NSString * szPersistFile = nil;

	if (m_initialPathname.empty())
	{
		// the caller did not supply initial pathname
		// (or supplied an empty one).  see if we have
		// some persistent info.
		
		UT_ASSERT(!m_bSuggestName);
		if (!m_persistPathname.empty())
		{
			// we have a pathname from a previous use,
			// extract the directory portion and start
			// the dialog there (but without a filename).

			szPersistDirectory = [NSString stringWithUTF8String:m_persistPathname.c_str()];
		}
		else
		{
			// no initial pathname given and we don't have
			// a pathname from a previous use, so just let
			// it come up in the current working directory.
		}
	}
	else
	{
		// we have an initial pathname (the name of the document
		// in the frame that we were invoked on).  if the caller
		// wanted us to suggest a filename, use the initial
		// pathname as is.  if not, use the directory portion of
		// it.

		if (m_bSuggestName)
		{
			/* use m_initialPathname
			 */
			NSString * path = [NSString stringWithUTF8String:m_initialPathname.c_str()];

			szPersistDirectory = [path stringByDeletingLastPathComponent];

			if ([szPersistDirectory isEqualToString:path]) // shouldn't happen
			{
				szPersistFile = [NSString string];
			}
			else
			{
				szPersistFile = [path lastPathComponent];
			}

			/* Slightly odd case where we're saving a file whose original document type
			 * (guessing by the suffix) is not supported for saving to.
			 */
			if ((m_id == XAP_DIALOG_ID_FILE_SAVEAS) && [szPersistFile length])
			{
				NSString * extension = [szPersistFile pathExtension];

				std::string szSaveTypeSuffix = UT_pathSuffix(m_szSuffixes[m_nDefaultFileType]);

				if ([extension length] && !szSaveTypeSuffix.empty())
				{
					if (*(szSaveTypeSuffix.c_str()) == '.')
					{
						NSString * new_suffix = [NSString stringWithUTF8String:(szSaveTypeSuffix.c_str() + 1)];

						if (![new_suffix isEqualToString:extension])
						{
							/* okay, the suggested extension doesn't match the desired extension
							 */
							NSString * filename = [szPersistFile stringByDeletingPathExtension];
							szPersistFile = [filename stringByAppendingPathExtension:new_suffix];
						}
					}
				}
			}
		}
		else
		{
			/* use directory(m_szInitialPathname)
			 */
			NSString * path = [NSString stringWithUTF8String:m_initialPathname.c_str()];

			szPersistDirectory = [path stringByDeletingLastPathComponent];
			szPersistFile = [NSString string];
		}
	}

	int result;

	m_bOpenPanel   = bOpenPanel;
	m_bPanelActive = true;

	do
	{
		m_bIgnoreCancel = false;

		if (m_bOpenPanel)
		{
//			[m_panel setPanelCanOrderOut:YES];
			NSOpenPanel * openPanel = (NSOpenPanel*)m_panel;
			if ([m_fileTypes count])
			{
				NSString * type = (NSString *) [m_fileTypes objectAtIndex:0];
				if (szPersistFile)
				{
					NSString * extension = [szPersistFile pathExtension];
					if (![extension isEqualToString:type]) {
						szPersistFile = nil;
					}
				}
			}

			if ([m_fileTypes count]) {
				result = [openPanel runModalForDirectory:szPersistDirectory file:szPersistFile types:m_fileTypes];
			}
			else {
				result = [openPanel runModalForDirectory:szPersistDirectory file:szPersistFile types:nil];
			}

			result = (result == NSOKButton) ? NSFileHandlingPanelOKButton : NSFileHandlingPanelCancelButton;
		}
		else
		{
			if ([m_fileTypes count])
			{
				NSString * type = (NSString *) [m_fileTypes objectAtIndex:0];
				[m_panel setRequiredFileType:type];

				if (szPersistFile)
				{
					NSString * extension = [szPersistFile pathExtension];
					if ([extension length])
					{
						if (![extension isEqualToString:type])
						{
							szPersistFile = [szPersistFile stringByDeletingPathExtension];
							szPersistFile = [szPersistFile stringByAppendingPathExtension:type];
						}
					}
					else
					{
						szPersistFile = [szPersistFile stringByAppendingPathExtension:type];
					}
				}
			}
			else
			{
				[m_panel setRequiredFileType:nil];
			}

//			[m_panel setPanelCanOrderOut:YES];
			result = [m_panel runModalForDirectory:szPersistDirectory file:szPersistFile];
		}

		szPersistDirectory = [m_panel directory];
		szPersistFile      = [m_panel filename];

		if (szPersistFile)
			szPersistFile = [szPersistFile lastPathComponent];
	}
	while (m_bIgnoreCancel);

	m_bPanelActive = false;

	szPersistFile = [m_panel filename];

	if ((result == NSFileHandlingPanelOKButton) && szPersistFile)
	{
		m_finalPathname = [szPersistFile UTF8String];
		m_answer = a_OK;
	}
}



void XAP_CocoaDialog_FileOpenSaveAs::_setSelectedFileType (UT_sint32 type)
{
	m_nFileType = type;
	m_szFileTypeDescription = 0;

	[m_fileTypes removeAllObjects];

	if (m_nFileType != XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO) {
		for (UT_uint32 i = 0; i < m_szFileTypeCount; i++) {
			if (m_nFileType == m_nTypeList[i]) {
				m_szFileTypeDescription = m_szDescriptions[i];

				addSuffixesToFileTypes(m_fileTypes,  m_szSuffixes[i]);
			}
		}
	}

	if([m_fileTypes count]) {
		[m_panel setAllowedFileTypes:m_fileTypes];
	}
	else {
		[m_panel setAllowedFileTypes:nil];	
	}
	[m_panel update];
//	if (m_bPanelActive) {
//		[m_panel setPanelCanOrderOut:NO];

//		m_bIgnoreCancel = true;
//		[NSApp stopModal];
//	}
}

