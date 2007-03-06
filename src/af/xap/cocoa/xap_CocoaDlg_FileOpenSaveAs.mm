/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_png.h"
#include "ut_string.h"
#include "ut_svg.h"

#include "xap_CocoaDlg_FileOpenSaveAs.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"
#include "xap_Strings.h"

#include "gr_CocoaGraphics.h"
#include "gr_CocoaImage.h"

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"

#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_types.h"

@implementation XAP_CocoaOpenPanel

- (id)init
{
	if (self = [super init])
		{
			m_bPanelCanOrderOut = YES;
		}
	return self;
}

- (BOOL)panelCanOrderOut
{
	return m_bPanelCanOrderOut;
}

- (void)setPanelCanOrderOut:(BOOL)panelCanOrderOut
{
	m_bPanelCanOrderOut = panelCanOrderOut;
}

- (void)orderOut:(id)sender
{
	if ([self panelCanOrderOut])
		{
			[super orderOut:sender];
		}
}

@end

@implementation XAP_CocoaSavePanel

- (id)init
{
	if (self = [super init])
		{
			m_bPanelCanOrderOut = YES;
		}
	return self;
}

- (BOOL)panelCanOrderOut
{
	return m_bPanelCanOrderOut;
}

- (void)setPanelCanOrderOut:(BOOL)panelCanOrderOut
{
	m_bPanelCanOrderOut = panelCanOrderOut;
}

- (void)orderOut:(id)sender
{
	if ([self panelCanOrderOut])
		{
			[super orderOut:sender];
		}
}

@end

#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100

@implementation XAP_OpenSavePanel_AccessoryController

- (id)initWithXAP:(XAP_CocoaDialog_FileOpenSaveAs*)xap
{
	if (self = [super init])
		{
			_xap = xap;
			m_bInsertGraphic = NO;
		}
	return self;
}

- (void)setInsertGraphic:(BOOL)insertGraphic
{
	m_bInsertGraphic = insertGraphic;
}

- (void)setPreviewImage:(NSImage *)image
{
	if (m_bInsertGraphic)
		{
			if (image)
				[oFTIImageView setImage:image];
			else
				[oFTIImageView setImage:[NSImage imageNamed:@"NSApplicationIcon"]];
		}
}

- (NSSize)previewSize
{
	return [oFTIImageView frame].size;
}

- (NSView *)fileTypeAccessoryView
{
	return (m_bInsertGraphic ? oFTIAccessoryView : oFTAccessoryView);
}

- (void)setFileTypeLabel:(NSString*)label
{
	if (m_bInsertGraphic)
		[oFTILabel setStringValue:label];
	else
		[oFTLabel setStringValue:label];
}

- (void)setSelectedFileType:(int)type
{
	if (m_bInsertGraphic)
		[oFTIPopUp selectItem:[[oFTIPopUp menu] itemWithTag:type]];
	else
		[oFTPopUp selectItem:[[oFTPopUp menu] itemWithTag:type]];
}

- (void)removeItemsOfFileTypesMenu
{
	if (m_bInsertGraphic)
		[oFTIPopUp removeAllItems];
	else
		[oFTPopUp removeAllItems];
}

- (void)addItemWithTitle:(NSString *)title fileType:(int)type
{
	if (m_bInsertGraphic)
		{
			[oFTIPopUp addItemWithTitle:title];
			[[oFTIPopUp lastItem] setTag:type];
		}
	else
		{
			[oFTPopUp addItemWithTitle:title];
			[[oFTPopUp lastItem] setTag:type];
		}
}

- (IBAction)selectFileType:(id)sender
{
	_xap->_setSelectedFileType([[sender selectedItem] tag]);
}

- (void)panelSelectionDidChange:(id)sender
{
	if (m_bInsertGraphic)
		_xap->_updatePreview();
}

@end

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FileOpenSaveAs * p = new XAP_CocoaDialog_FileOpenSaveAs(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FileOpenSaveAs::XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_FileOpenSaveAs(pDlgFactory,dlgid),
	m_accessoryViewsController(0),
	m_OpenPanel(0),
	m_SavePanel(0),
	m_panel(0),
	m_FileTypes(0),
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
			m_accessoryViewsController = 0;
		}
	if (m_OpenPanel)
		{
			[m_OpenPanel release];
			m_OpenPanel = 0;
		}
	if (m_SavePanel)
		{
			[m_SavePanel release];
			m_SavePanel = 0;
		}
	if (m_FileTypes)
		{
			[m_FileTypes release];
			m_FileTypes = 0;
		}
}

/*****************************************************************/

void XAP_CocoaDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	if (!m_accessoryViewsController)
		if (m_accessoryViewsController = [[XAP_OpenSavePanel_AccessoryController alloc] initWithXAP:this]) {
			if (![NSBundle loadNibNamed:@"xap_CocoaFileOpen_Views" owner:m_accessoryViewsController]) {
				NSLog (@"Couldn't load nib xap_CocoaFileOpen_Views");
				[m_accessoryViewsController release];
				m_accessoryViewsController = 0;
			}
		}
	UT_ASSERT(m_accessoryViewsController);
	if (!m_accessoryViewsController)
		return;

	if (!m_OpenPanel)
		m_OpenPanel = [[XAP_CocoaOpenPanel alloc] init];
	UT_ASSERT(m_OpenPanel);
	if (!m_OpenPanel)
		return;

	if (!m_SavePanel)
		m_SavePanel = [[XAP_CocoaSavePanel alloc] init];
	UT_ASSERT(m_SavePanel);
	if (!m_SavePanel)
		return;

	if (!m_FileTypes)
		m_FileTypes = [[NSMutableArray alloc] initWithCapacity:4];
	UT_ASSERT(m_FileTypes);
	if (!m_FileTypes)
		return;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the Cocoa
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;
	bool bOpenPanel = false;
	bool bSavePanel = false;

	UT_UTF8String szTitle;
	UT_UTF8String szFileTypeLabel;

	switch (m_id)
	{
	case XAP_DIALOG_ID_INSERTMATHML:
		{
			bOpenPanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertMath, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertMath, szFileTypeLabel);

			bCheckWritePermission = false;
			break;
		}
	case XAP_DIALOG_ID_INSERT_PICTURE:
		{
			bOpenPanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

			bCheckWritePermission = false;
			break;
		}
	case XAP_DIALOG_ID_FILE_OPEN:
		{
			bOpenPanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

			bCheckWritePermission = false;
			break;
		}
	case XAP_DIALOG_ID_FILE_SAVEAS:
		{
			bSavePanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel, szFileTypeLabel);

			bCheckWritePermission = true;
			break;
		}
	case XAP_DIALOG_ID_FILE_IMPORT:
		{
			bOpenPanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

			bCheckWritePermission = false;
			break;
		}
	case XAP_DIALOG_ID_FILE_EXPORT:
		{
			bSavePanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel, szFileTypeLabel);

			bCheckWritePermission = true;
			break;
		}
	case XAP_DIALOG_ID_INSERT_FILE:
		{
			bOpenPanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);

			bCheckWritePermission = false;
			break;
		}
	case XAP_DIALOG_ID_PRINTTOFILE:
		{
			bSavePanel = true;

			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle, szTitle);
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel, szFileTypeLabel);

			bCheckWritePermission = true;
			break;
		}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	m_panel = 0;

	if (bOpenPanel) {
		m_panel = m_OpenPanel;

		// cast it id to avoid warning.
		if ([m_panel respondsToSelector:@selector(setAllowsMultipleSelection:)])
			[(id)m_panel setAllowsMultipleSelection:NO];
		if ([m_panel respondsToSelector:@selector(setCanChooseDirectories:)])
			[(id)m_panel setCanChooseDirectories:NO];
		if ([m_panel respondsToSelector:@selector(setCanChooseFiles:)])
			[(id)m_panel setCanChooseFiles:YES];
		if ([m_panel respondsToSelector:@selector(setCanCreateDirectories:)])
			[(id)m_panel setCanCreateDirectories:NO];

		[m_panel setCanSelectHiddenExtension:NO];
		[m_panel setExtensionHidden:NO];
	}
	else if (bSavePanel) {
		m_panel = m_SavePanel;

		if ([m_panel respondsToSelector:@selector(setCanCreateDirectories:)])
			[m_panel setCanCreateDirectories:YES];

		[m_panel setCanSelectHiddenExtension:YES];
		[m_panel setExtensionHidden:NO];
	}
	UT_ASSERT(m_panel);
	if (!m_panel)
		return;

	if (m_id == XAP_DIALOG_ID_INSERT_PICTURE) {
		[m_accessoryViewsController setInsertGraphic:YES];
		[m_accessoryViewsController setPreviewImage:nil];
	}
	else {
		[m_accessoryViewsController setInsertGraphic:NO];
	}

	[m_accessoryViewsController setFileTypeLabel:[NSString stringWithUTF8String:(szFileTypeLabel.utf8_str())]];

	[m_panel setTitle:[NSString stringWithUTF8String:(szTitle.utf8_str())]];
	[m_panel setDelegate:m_accessoryViewsController];

	/* File-types PopUp:
	 */

	[m_accessoryViewsController removeItemsOfFileTypesMenu];

	UT_UTF8String label;
	if (pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect, label)) {
		NSString * title = [NSString stringWithUTF8String:(label.utf8_str())];
		int type = (int) XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO;
		[m_accessoryViewsController addItemWithTitle:title fileType:type];
	}

	UT_ASSERT(g_strv_length((gchar **) m_szSuffixes) == g_strv_length((gchar **) m_szDescriptions));

	m_szFileTypeCount = g_strv_length((gchar **) m_szDescriptions);

	UT_sint32 defaultFileType = m_nDefaultFileType;

	for (UT_uint32 i = 0; i < m_szFileTypeCount; i++) {
		NSString * title = [NSString stringWithUTF8String:m_szDescriptions[i]];
		int type = m_nTypeList[i];
		if (m_id == XAP_DIALOG_ID_INSERT_PICTURE) {
			if ([title isEqualToString:@"Cocoa-Readable Image"])
				defaultFileType = m_nTypeList[i];
		}
		[m_accessoryViewsController addItemWithTitle:title fileType:type];
	}

	[m_accessoryViewsController setSelectedFileType:defaultFileType];
	_setSelectedFileType(defaultFileType);
	
	[m_panel setAccessoryView:[m_accessoryViewsController fileTypeAccessoryView]];

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	NSString * szPersistDirectory = nil;
	NSString * szPersistFile = nil;

	if (!m_szInitialPathname || !*m_szInitialPathname)
	{
		// the caller did not supply initial pathname
		// (or supplied an empty one).  see if we have
		// some persistent info.
		
		UT_ASSERT(!m_bSuggestName);
		if (m_szPersistPathname)
		{
			// we have a pathname from a previous use,
			// extract the directory portion and start
			// the dialog there (but without a filename).

			szPersistDirectory = [NSString stringWithUTF8String:m_szPersistPathname];
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
			/* use m_szInitialPathname
			 */
			NSString * path = [NSString stringWithUTF8String:m_szInitialPathname];

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

				const char * szSaveTypeSuffix = UT_pathSuffix(m_szSuffixes[m_nDefaultFileType]);

				if ([extension length] && szSaveTypeSuffix)
				{
					if (*szSaveTypeSuffix == '.')
					{
						NSString * new_suffix = [NSString stringWithUTF8String:(szSaveTypeSuffix + 1)];

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
			NSString * path = [NSString stringWithUTF8String:m_szInitialPathname];

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
			[m_OpenPanel setPanelCanOrderOut:YES];

			if ([m_FileTypes count])
			{
				NSString * type = (NSString *) [m_FileTypes objectAtIndex:0];
				if (szPersistFile)
				{
					NSString * extension = [szPersistFile pathExtension];
					if (![extension isEqualToString:type])
						szPersistFile = nil;
				}
			}

			if ([m_FileTypes count])
				result = [m_OpenPanel runModalForDirectory:szPersistDirectory file:szPersistFile types:m_FileTypes];
			else
				result = [m_OpenPanel runModalForDirectory:szPersistDirectory file:szPersistFile types:nil];

			result = (result == NSOKButton) ? NSFileHandlingPanelOKButton : NSFileHandlingPanelCancelButton;
		}
		else
		{
			if ([m_FileTypes count])
			{
				NSString * type = (NSString *) [m_FileTypes objectAtIndex:0];
				[m_SavePanel setRequiredFileType:type];

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
				[m_SavePanel setRequiredFileType:nil];
			}

			[m_SavePanel setPanelCanOrderOut:YES];
			result = [m_SavePanel runModalForDirectory:szPersistDirectory file:szPersistFile];
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
		FREEP (m_szFinalPathname);	// g_free before reassigning
		m_szFinalPathname = g_strdup([szPersistFile UTF8String]);
		m_answer = a_OK;
	}
}

void XAP_CocoaDialog_FileOpenSaveAs::_setSelectedFileType (UT_sint32 type)
{
	m_nFileType = type;
	m_szFileTypeDescription = 0;

	[m_FileTypes removeAllObjects];

	if (m_nFileType != XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO) {
		for (UT_uint32 i = 0; i < m_szFileTypeCount; i++) {
			if (m_nFileType == m_nTypeList[i]) {
				m_szFileTypeDescription = m_szDescriptions[i];

				UT_UTF8String suffix;
				const char * suffix_list = m_szSuffixes[i];
				while (const char * ptr1 = strstr(suffix_list, "*.")) {
					ptr1 += 2;
					bool bSingleExtension = true;
					const char * ptr2 = ptr1;
					while (*ptr2 && (*ptr2 != ';'))
						if (*ptr2++ == '.')
							bSingleExtension = false;
					suffix.assign(ptr1, ptr2 - ptr1);
					if (bSingleExtension)
						[m_FileTypes addObject:[NSString stringWithUTF8String:(suffix.utf8_str())]];
					suffix_list = ptr2;
				}
			}
		}
	}

	if (m_bPanelActive) {
		if (m_bOpenPanel)
			[m_OpenPanel setPanelCanOrderOut:NO];
		else
			[m_SavePanel setPanelCanOrderOut:NO];

		m_bIgnoreCancel = true;
		[NSApp stopModal];
	}
}

void XAP_CocoaDialog_FileOpenSaveAs::_updatePreview ()
{
	if (m_id != XAP_DIALOG_ID_INSERT_PICTURE)
		return;

	NSImage * image = nil;
	NSArray * array = [m_OpenPanel filenames];

	if ([array count]) {
		NSString * filename = (NSString *) [array objectAtIndex:0];

		UT_ByteBuf * pBB = new UT_ByteBuf(0);

		if (pBB->insertFromFile(0, [filename UTF8String])) {
			IE_ImpGraphic * pIEG = 0;

			UT_Error errorCode;
			if (m_szFileTypeDescription) { // i.e., if (m_nFileType != XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO)
				errorCode = IE_ImpGraphic::constructImporterWithDescription(m_szFileTypeDescription, &pIEG);
			}
			else {
				errorCode = IE_ImpGraphic::constructImporter([filename UTF8String], IEGFT_Unknown, &pIEG);
			}
			if ((errorCode == UT_OK) && pIEG) {
				FG_Graphic * pGraphic = 0;

				errorCode = pIEG->importGraphic(pBB, &pGraphic);
				pBB = 0;

				if ((errorCode == UT_OK) && pGraphic) {
					if (FGT_Raster == pGraphic->getType()) {

						const UT_ByteBuf * png = static_cast<FG_GraphicRaster *>(pGraphic)->getRaster_PNG();

						UT_sint32 iImageWidth;
						UT_sint32 iImageHeight;

						UT_PNG_getDimensions(png, iImageWidth, iImageHeight);

						double dImageWidth  = static_cast<double>(iImageWidth);
						double dImageHeight = static_cast<double>(iImageHeight);

						NSSize previewSize = [m_accessoryViewsController previewSize];

						double dPreviewWidth  = static_cast<double>(previewSize.width);
						double dPreviewHeight = static_cast<double>(previewSize.height);

						if ((dPreviewWidth < dImageWidth) || (dPreviewHeight < dImageHeight)) {
							double factor_width  = dPreviewWidth  / dImageWidth;
							double factor_height = dPreviewHeight / dImageHeight;

							double scale_factor = MIN(factor_width, factor_height);

							iImageWidth  = static_cast<UT_sint32>(scale_factor * dImageWidth);
							iImageHeight = static_cast<UT_sint32>(scale_factor * dImageHeight);
						}

						GR_CocoaImage * pImage = new GR_CocoaImage(0);

						pImage->convertFromBuffer(png, iImageWidth, iImageHeight); // this flips the NSImage but doesn't actually scale it
		
						if (image = pImage->getNSImage()) {
							NSSize imageSize;
							imageSize.width  = static_cast<float>(iImageWidth);
							imageSize.height = static_cast<float>(iImageHeight);

							[image setScalesWhenResized:YES];
							[image setSize:imageSize];
							[image setFlipped:NO];
							[image retain];
						}
						DELETEP(pImage);
					}
					else { // if (FGT_Vector == pGraphic->getType())
						// pImage = new GR_VectorImage(NULL);
					}
					DELETEP(pGraphic);
				}
				DELETEP(pIEG);
			}
		}
		DELETEP(pBB);
	}
	[m_accessoryViewsController setPreviewImage:image];
	if (image)
		[image release];
}
