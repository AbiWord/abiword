/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
 * Copyright (C) 2005 Francis Franklin
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_CocoaDialog_New.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"

/*************************************************************************/

XAP_Dialog * AP_CocoaDialog_New::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_New * p = new AP_CocoaDialog_New(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_New::AP_CocoaDialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_New(pDlgFactory, dlgid), m_pFrame(0)
{
}

AP_CocoaDialog_New::~AP_CocoaDialog_New(void)
{
}

void AP_CocoaDialog_New::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_dlg = [[AP_CocoaDialog_NewController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	NSWindow* win = [m_dlg window];

	// Populate the window's data items
//	_populateWindowData();
	
	[NSApp runModalForWindow:win];

//	_storeWindowData();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	m_pFrame = NULL;
}

/*************************************************************************/
/*************************************************************************/

void AP_CocoaDialog_New::event_Ok ()
{
	setAnswer (AP_Dialog_New::a_OK);

	if ([m_dlg existingBtnState])
	{
		setOpenType(AP_Dialog_New::open_Existing);
	}
	else
	{
		setFileName([[m_dlg newBtnState] UTF8String]);
		setOpenType(AP_Dialog_New::open_Template);
	}

	[NSApp stopModal];
}

void AP_CocoaDialog_New::event_Cancel ()
{
	setAnswer (AP_Dialog_New::a_CANCEL);
	[NSApp stopModal];
}

void AP_CocoaDialog_New::event_ToggleOpenExisting ()
{
	XAP_Dialog_Id dlgid = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) XAP_App::getApp()->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(dlgid));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname("");
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IE_Imp::fileTypeForSuffix(".abw"));

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty())
		{
			// update the entry box
			[m_dlg setFileName:[NSString stringWithUTF8String:resultPathname.c_str()]];
			setFileName (resultPathname.c_str());
		}

		[m_dlg setExistingBtnState:YES];
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

void AP_CocoaDialog_New::event_ToggleStartNew ()
{	
	// nada
}

/*************************************************************************/
/*************************************************************************/

@implementation AP_CocoaDialog_NewController

- (void)dealloc
{
	[m_templates release];
	[_dataSource release];
	[super dealloc];
}


- (id)initFromNib
{
	if(![super initWithWindowNibName:@"ap_CocoaDialog_New"]) {
		return nil;
	}
	m_templates = [[NSMutableArray alloc] init];
	return self;
}


- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_New*>(owner);
	UT_ASSERT (_xap);
}

- (void)discardXAP
{
	_xap = nil;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_NEW_Title);
	LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
	LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
	LocalizeControl(_chooseFileBtn, pSS, AP_STRING_ID_DLG_NEW_Choose);
	LocalizeControl(_createNewBtn, pSS, AP_STRING_ID_DLG_NEW_Create);
	LocalizeControl(_openBtn, pSS, AP_STRING_ID_DLG_NEW_Open);
	[self synchronizeGUI:_createNewBtn];	// TODO check what is the default
	
	_dataSource = [[XAP_StringListDataSource alloc] init];
	NSMutableArray *templateDirs = [[NSMutableArray alloc] init];
	
	[templateDirs addObject:[NSString stringWithFormat:@"%s/templates/", XAP_App::getApp()->getUserPrivateDirectory()]];
	[templateDirs addObject:[NSString stringWithFormat:@"%s/templates/", XAP_App::getApp()->getAbiSuiteLibDir()]];
	[templateDirs addObject:[NSString stringWithFormat:@"%s/templates/", [[[NSBundle mainBundle] resourcePath] UTF8String]]];

	NSEnumerator* e = [templateDirs objectEnumerator];
    while(NSString *dirPath = [e nextObject])
    {
        NSDirectoryEnumerator* dirEnumerator = [[NSFileManager defaultManager] enumeratorAtPath:dirPath];
        while(NSString *file = [dirEnumerator nextObject])
        {
			if([[file pathExtension] isEqualToString:@"awt"]) {
                [_dataSource addString:file];
                [m_templates addObject:[dirPath stringByAppendingPathComponent:file]];
            }
        }
    }

	[templateDirs release];
	
	[_templateList setDataSource:_dataSource];
	[_templateList setDelegate:self];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if ([_templateList selectedRow] < 0) // I don't think this happens
		[self synchronizeGUI:_openBtn];
	else
		[self synchronizeGUI:_createNewBtn];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)radioButtonAction:(id)sender
{
	[self synchronizeGUI:sender];
}

- (IBAction)chooseAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_ToggleOpenExisting();
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Ok();
}

- (void)synchronizeGUI:(NSControl*)control
{
	enum { NONE, NEW, OPEN } selected;
	
	if (control == _createNewBtn) {
		selected = NEW;
	}
	else if (control == _openBtn) {
		selected = OPEN;
	}
	else {
		selected = NONE;
	}
	switch (selected) {
	case NEW:
		[_createNewBtn setState:NSOnState];
		[_templateList setEnabled:YES];
		[_openBtn setState:NSOffState];
		[_documentNameData setEnabled:NO];
		[_chooseFileBtn setEnabled:NO];
		break;
	case OPEN:
		[_createNewBtn setState:NSOffState];
		[_templateList setEnabled:NO];
		[_openBtn setState:NSOnState];
		[_documentNameData setEnabled:YES];
		[_chooseFileBtn setEnabled:YES];
		break;
	default:
		break;
	}
}


- (BOOL)existingBtnState
{
	return ([_openBtn state] == NSOnState);
}

- (void)setExistingBtnState:(BOOL)state
{
	if (state) {
		[_openBtn setState:NSOnState];
	}
	else {
		[_openBtn setState:NSOffState];
	}
}

- (NSString *)newBtnState
{
	NSString * path = 0;

	if ([_createNewBtn state] == NSOnState) {
		int index = [_templateList selectedRow];
		if (index < 0) {
			UT_DEBUGMSG(("AP_CocoaDialog_NewController -newBtnState: no template selection from list?\n"));
		}
		else {
			path = (NSString *) [m_templates objectAtIndex:((unsigned) index)];
			UT_DEBUGMSG(("AP_CocoaDialog_NewController -newBtnState: template \"%s\" selected from list.\n", [path UTF8String]));
		}
	}
	return path;
}

- (void)setFileName:(NSString*)name
{
	[_documentNameData setStringValue:name];
}

@end
