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

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_CocoaDlg_FileOpenSaveAs.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Strings.h"
#include "xap_Prefs.h"
#include "ut_debugmsg.h"

#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "gr_CocoaGraphics.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"

#include "gr_CocoaImage.h"

#include <sys/stat.h>

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_imp.h"
#include "../../../wp/impexp/xp/ie_impGraphic.h"

#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100


@implementation XAP_OpenSavePanel_AccessoryController

-(id)initWithXAP:(XAP_CocoaDialog_FileOpenSaveAs*)xap
{
	self = [super init];
	_xap = xap;
	return self;
}

-(void)awakeFromNib
{
	
}

-(NSView*)fileTypeAcessoryView
{
	return _fileTypeAcessoryView;
}

-(void)setFileTypeLabel:(NSString*)label
{
	[_fileTypeLabel setStringValue:label];
}

-(void)setSelectedFileType:(int)type
{
	[_fileTypePopup selectItem:[[_fileTypePopup menu] itemWithTag:type]];
}

-(NSMenu*)fileTypesMenu
{
	return [_fileTypePopup menu];
}

-(void)removeItemsOfFileTypesMenu
{
	[_fileTypePopup removeAllItems];
}

-(IBAction)selectFileType:(id)sender
{
	_xap->_setSelectedFileType([[sender selectedItem] tag]);
}


@end
/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FileOpenSaveAs * p = new XAP_CocoaDialog_FileOpenSaveAs(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FileOpenSaveAs::XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id dlgid)
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,dlgid),
	m_accessoryViewsController(nil)
{
}

XAP_CocoaDialog_FileOpenSaveAs::~XAP_CocoaDialog_FileOpenSaveAs(void)
{
	[m_accessoryViewsController release];
}

/*****************************************************************/

void XAP_CocoaDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pCocoaFrame = (XAP_CocoaFrame *)pFrame;
	UT_ASSERT(m_pCocoaFrame);
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	if (m_accessoryViewsController == nil) {
		m_accessoryViewsController = [[XAP_OpenSavePanel_AccessoryController alloc] initWithXAP:this];
		if (![NSBundle loadNibNamed:@"xap_CocoaFileOpen_Views" owner:m_accessoryViewsController]) {
			NSLog (@"Couldn't load nib xap_CocoaFileOpen_Views");
			return;
		}
	}
	// do we want to let this function handle stating the Cocoa
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;

	UT_String szTitle;
	UT_String szFileTypeLabel;
	switch (m_id)
	{
	case XAP_DIALOG_ID_INSERT_PICTURE:
	  {
		m_panel = [NSOpenPanel openPanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;    
	  }
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		m_panel = [NSOpenPanel openPanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		m_panel = [NSSavePanel savePanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel);
		bCheckWritePermission = true;
		break;
	}
	case XAP_DIALOG_ID_FILE_IMPORT:
	  {
	  	m_panel = [NSOpenPanel openPanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
	    break;
	  }
	case XAP_DIALOG_ID_FILE_EXPORT:
	  {
		m_panel = [NSSavePanel savePanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel);
		bCheckWritePermission = true;
	    break;
	  }
	case XAP_DIALOG_ID_INSERT_FILE:
	  {
		m_panel = [NSOpenPanel openPanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
		break;
	  }
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		m_panel = [NSSavePanel savePanel];
		szTitle = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle);
		szFileTypeLabel = pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel);
		bCheckWritePermission = true;
		break;
	}
	default:
		m_panel = nil;
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	UT_ASSERT (m_panel);
	// NOTE: we use our string mechanism to localize the dialog's
	// NOTE: title and the error/confirmation message boxes.  we
	// NOTE: let Cocoa take care of the localization of the actual
	// NOTE: buttons and labels on the FileSelection dialog.

	[m_panel setTitle:[NSString stringWithUTF8String:szTitle.c_str()]];
	[m_panel setExtensionHidden:NO];
	[m_accessoryViewsController setFileTypeLabel:[NSString stringWithUTF8String:szFileTypeLabel.c_str()]];
	[m_accessoryViewsController removeItemsOfFileTypesMenu];
	NSMenuItem*	item;
	NSMenu* fileTypesMenu = [m_accessoryViewsController fileTypesMenu];
	item = [[NSMenuItem alloc]	initWithTitle:
			[NSString stringWithUTF8String:pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect).c_str()]
			action:nil 
			keyEquivalent:@""];
	[item setTag:XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO];
	[fileTypesMenu addItem:item];
	{
		UT_ASSERT(UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szSuffixes))) ==
					UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szDescriptions))));
		UT_uint32 end = UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szDescriptions)));
		
		for (UT_uint32 i = 0; i < end; i++)
		{
			// If this type is default, save its index (i) for later use
			item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:m_szDescriptions[i]]
					action:nil 
					keyEquivalent:@""];
			[item setTag:m_nTypeList[i]];
			[fileTypesMenu addItem:item];
			[item release];
		}
		[m_accessoryViewsController setSelectedFileType:m_nDefaultFileType];
	}
	
	[m_panel setAccessoryView:[m_accessoryViewsController fileTypeAcessoryView]];

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
			// use m_szInitialPathname
			szPersistDirectory = [NSString stringWithUTF8String:m_szInitialPathname];
			szPersistFile = [NSString string];
		}
		else
		{
			// use directory(m_szInitialPathname)
			szPersistDirectory = [NSString stringWithUTF8String:m_szInitialPathname];
			szPersistFile = [NSString string];
		}
	}


	int result = [m_panel runModalForDirectory:szPersistDirectory file:szPersistFile];
	
	if (result == NSFileHandlingPanelOKButton)
	{
		FREEP (m_szFinalPathname);	// free before reassigning
		m_szFinalPathname = UT_strdup([[m_panel filename] UTF8String]);
		m_answer = a_OK;
	}
			  
	FREEP(szPersistDirectory);
	m_pCocoaFrame = NULL;

	return;
}

#if 0
int 
XAP_CocoaDialog_FileOpenSaveAs::previewPicture (void)
{
        UT_ASSERT (m_FS && m_preview);

	XAP_CocoaApp * unixapp = static_cast<XAP_CocoaApp *> (m_pApp);
	UT_ASSERT(unixapp);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// attach and clear the area immediately
	GR_CocoaGraphics* pGr = new GR_CocoaGraphics(m_preview->window, unixapp->getFontManager(), m_pApp);
	pGr->clearArea(0, 0, m_preview->allocation.width, m_preview->allocation.height);

	gchar * buf = gtk_file_selection_get_filename (m_FS);

	GR_Font * fnt = pGr->findFont("Times New Roman", "normal", "", "normal", "", "12pt");
	UT_ASSERT (fnt);		// not good if we don't have it.
	pGr->setFont(fnt);

	const XML_Char * str = pSS->getValue(XAP_STRING_ID_DLG_IP_No_Picture_Label);
	int len = strlen (str);
	UT_UCSChar * ucstext = new UT_UCSChar [len + 1]; 
	UT_UCS4_strcpy_char (ucstext, str);

	int answer = 0;

	UT_ByteBuf *pBB = NULL;
	FG_Graphic * pGraphic = 0;
	IE_ImpGraphic* pIEG = NULL;
	UT_Error errorCode = UT_OK;
	GR_Image *pImage = NULL;

	double		scale_factor = 0.0;
	UT_sint32     scaled_width,scaled_height;
	UT_sint32     iImageWidth,iImageHeight;

	if (!buf)
	  {
	    pGr->drawChars (ucstext, 0, len, 12, 35);
	    goto Cleanup;
	  }

	// are we dealing with a file or directory here?
	struct stat st;
	if (!stat (buf, &st)) {
		if (!S_ISREG(st.st_mode)) {
			pGr->drawChars (ucstext, 0, len, 12, 35);
			goto Cleanup;
		}
	}
	else {
		pGr->drawChars (ucstext, 0, len, 12, 35);
		goto Cleanup;
	}

	// Load File into memory
	pBB     = new UT_ByteBuf(0);
	pBB->insertFromFile(0, buf);

	// Build an Import Graphic based on file type
	errorCode = IE_ImpGraphic::constructImporter(buf, IEGFT_Unknown, &pIEG);
	if ((errorCode != UT_OK) || !pIEG)
	{
		DELETEP(pBB);
		pGr->drawChars (ucstext, 0, len, 12, 35);
		goto Cleanup;
	}

	errorCode = pIEG->importGraphic (pBB, &pGraphic);

	if ((errorCode != UT_OK) || !pGraphic)
	  {
	    pGr->drawChars (ucstext, 0, len, 12, 35);
	    goto Cleanup;
	  }

	if ( FGT_Raster == pGraphic->getType () )
	{
		pImage = new GR_CocoaImage(NULL);
		UT_ByteBuf * png = static_cast<FG_GraphicRaster*>(pGraphic)->getRaster_PNG();
		UT_PNG_getDimensions (png, iImageWidth, iImageHeight);

		if (m_preview->allocation.width >= iImageWidth && m_preview->allocation.height >= iImageHeight)
		  scale_factor = 1.0;
		else
		  scale_factor = MIN( (double) m_preview->allocation.width/iImageWidth,
				      (double) m_preview->allocation.height/iImageHeight);
		
		scaled_width  = (int)(scale_factor * iImageWidth);
		scaled_height = (int)(scale_factor * iImageHeight);

		pImage->convertFromBuffer(png, scaled_width, scaled_height);
		
		pGr->drawImage(pImage,
			       (int)((m_preview->allocation.width  - scaled_width ) / 2),
			       (int)((m_preview->allocation.height - scaled_height) / 2));
		
		answer = 1;
	}
	else // if ( FGT_Vector == pGraphic->getType () )
	{
	  //pImage = new GR_VectorImage(NULL);
	}

 Cleanup:
	DELETEP(pImage);
	DELETEP(pGr);
	DELETEP(fnt);
	DELETEPV(ucstext);

	return answer;
}
#endif

void	XAP_CocoaDialog_FileOpenSaveAs::_setSelectedFileType (UT_sint32 type)
{
	m_nFileType = type;
}
