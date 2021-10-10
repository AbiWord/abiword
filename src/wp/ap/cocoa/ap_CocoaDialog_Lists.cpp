/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003-2016 Hubert Figuiere
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_CocoaGraphics.h"
#include "xap_Dialog_Id.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_CocoaDialog_Lists.h"
#include "fp_Line.h"
#include "fp_Column.h"

/*****************************************************************/

enum {
	GUI_LIST_NONE = 0,
	GUI_LIST_BULLETED = 1,
	GUI_LIST_NUMBERED = 2
};


static AP_CocoaDialog_Lists * Current_Dialog;


AP_CocoaDialog_Lists::AP_CocoaDialog_Lists(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_Lists(pDlgFactory,dlgid)
{
	Current_Dialog = this;
	m_pPreviewWidget = NULL;
	m_pAutoUpdateLists = NULL;
	m_bDontUpdate = false;
}

XAP_Dialog * AP_CocoaDialog_Lists::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Lists * p = new AP_CocoaDialog_Lists(pFactory, dlgid);
	return p;
}


AP_CocoaDialog_Lists::~AP_CocoaDialog_Lists(void)
{
	if(m_pPreviewWidget != NULL)
		DELETEP (m_pPreviewWidget);
}


void AP_CocoaDialog_Lists::runModal( XAP_Frame * /*pFrame*/)
{
	FL_ListType  savedListType;
	setModal();
	m_dlg = [[AP_CocoaDialog_ListsController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = m_dlg.window;

	clearDirty();

	// Populate the dialog
	m_bDontUpdate = false;
	loadXPDataIntoLocal();
//
// Need this to stop this being stomped during the contruction of preview
// widget
//
	savedListType = getNewListType();


	XAP_CocoaNSView* preview = m_dlg.preview;
	UT_ASSERT([preview isKindOfClass:[XAP_CocoaNSView class]]);
	GR_CocoaAllocInfo ai(preview);
	m_pPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

	// let the widget materialize

	NSRect rect = preview.bounds;
	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) rect.size.width,
						 (UT_uint32) rect.size.height);
	preview.drawable = getListsPreview();
//
// Restore our value
//
	setNewListType(savedListType);
	previewInvalidate();
	[NSApp runModalForWindow:window];
//
//  We've finished here.
//
	[m_dlg release];
	m_dlg = nil;
	DELETEP (m_pPreviewWidget);
}


void AP_CocoaDialog_Lists::runModeless (XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_ListsController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	
	clearDirty();
	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);

	NSPanel* window = (NSPanel*)m_dlg.window;

	// Populate the dialog
	updateDialog();
	m_bDontUpdate = false;

	// Now Display the dialog
	[window setBecomesKeyOnlyIfNeeded:YES];
	[window orderFront:m_dlg];

	// make a new Cocoa GC
	XAP_CocoaNSView* preview = [m_dlg preview];
	UT_ASSERT([preview isKindOfClass:[XAP_CocoaNSView class]]);
	GR_CocoaAllocInfo ai(preview);
	m_pPreviewWidget = static_cast<GR_CocoaGraphics*>(XAP_App::getApp()->newGraphics(ai));

	// let the widget materialize

	NSRect bounds = preview.bounds;
	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) bounds.size.width,
						 (UT_uint32) bounds.size.height);
	preview.drawable = getListsPreview();

	// Next construct a timer for auto-updating the dialog
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this);
	m_bDestroy_says_stopupdating = false;

	// OK fire up the auto-updater for 0.5 secs

	m_pAutoUpdateLists->set(500);
}


void AP_CocoaDialog_Lists::autoupdateLists(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);
	// this is a static callback method and does not have a 'this' pointer.
	AP_CocoaDialog_Lists * pDialog =  (AP_CocoaDialog_Lists *) pWorker->getInstanceData();
	// Handshaking code. Plus only update if something in the document
	// changed.

	AP_Dialog_Lists * pList = ( AP_Dialog_Lists *) pDialog;

	if(pList->isDirty())
		return;
	if(pDialog->getAvView()->getTick() != pDialog->getTick())
	{
		pDialog->setTick(pDialog->getAvView()->getTick());
		if( pDialog->m_bDestroy_says_stopupdating != true)
		{
			pDialog->m_bAutoUpdate_happening_now = true;
			pDialog->updateDialog();
			pDialog->previewInvalidate();
			pDialog->m_bAutoUpdate_happening_now = false;
		}
	}
}

void AP_CocoaDialog_Lists::setFoldLevelInGUI(void)
{
	[m_dlg selectFolding:getCurrentFold()];
}

bool AP_CocoaDialog_Lists::isPageLists(void)
{
	if(isModal())
	{
		return true;
	}

	return [m_dlg selectedTab] == 0;
}

void AP_CocoaDialog_Lists::previewInvalidate(void)
{
	if(m_pPreviewWidget)
	{
		setbisCustomized(true);
		event_PreviewAreaExposed();
	}
}

void AP_CocoaDialog_Lists::destroy(void)
{
	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_QUIT);
		[NSApp stopModal];
	}
	else
	{
		m_bDestroy_says_stopupdating = true;
		m_pAutoUpdateLists->stop();
		setAnswer(AP_Dialog_Lists::a_CLOSE);

		modeless_cleanup();

		[m_dlg close];
		[m_dlg release];
		m_dlg = 0;

		DELETEP(m_pAutoUpdateLists);
		DELETEP(m_pPreviewWidget);
	}
}

void AP_CocoaDialog_Lists::activate (void)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];
	m_bDontUpdate = false;
	updateDialog();
	[[m_dlg window] orderFront:m_dlg];
}

void AP_CocoaDialog_Lists::notifyActiveFrame(XAP_Frame */*pFrame*/)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];
	m_bDontUpdate = false;
	updateDialog();
	previewInvalidate();
}

void  AP_CocoaDialog_Lists::typeChanged(int type)
{
	//
	// code to change list list
	//
	[m_dlg setStyleMenu:type];
	
	switch (type) {
	case GUI_LIST_NONE:
		setNewListType(NOT_A_LIST);
		break;
	case GUI_LIST_BULLETED:
		setNewListType(BULLETED_LIST);
		break;
	case GUI_LIST_NUMBERED:
		setNewListType(NUMBERED_LIST);
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}
//
// This methods needs to be called from loadXPDataIntoLocal to set the correct
// list style. However if we are doing this we definately don't want to call
// loadXPDataIntoLocal again! Luckily we can just check this to make sure this is
// not happenning.
//
	if(!dontUpdate())
	{
		fillUncustomizedValues(); // Set defaults
		loadXPDataIntoLocal(); // load them into the widget
		previewInvalidate(); // Show current setting
	}
}

/*!
 * This method just sets the value of m_newListType. This is needed to
 * make fillUncustomizedValues work.
 */
void  AP_CocoaDialog_Lists::setListTypeFromWidget(void)
{
	NSMenuItem* item = [m_dlg selectedListStyle];

	setNewListType((FL_ListType)[item tag]);
}

/*!
 * This method reads out all the elements of the GUI and sets the XP member
 * variables from them
 */
void  AP_CocoaDialog_Lists::setXPFromLocal(void)
{
	// Read m_newListType

	setListTypeFromWidget();
//
// Read out GUI stuff in the customize box and load their values into the member
// variables.
//
	_gatherData();
//
// Now read the toggle button state and set the member variables from them
//
	switch ([m_dlg listAction]) {
	case GUI_LIST_NONE:
		setbStartNewList(true);
		setbApplyToCurrent(false);
		setbResumeList(false);
		break;
	case GUI_LIST_BULLETED:
		setbStartNewList(false);
		setbApplyToCurrent(true);
		setbResumeList(false);
		break;
	case GUI_LIST_NUMBERED:
		setbStartNewList(false);
		setbApplyToCurrent(false);
		setbResumeList(true);
		break;
	default:
		UT_ASSERT_NOT_REACHED();
		break;
	}
}


void  AP_CocoaDialog_Lists::applyClicked(void)
{
	setXPFromLocal();
	previewInvalidate();
	Apply();
	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_OK);
		[NSApp stopModal];
	}
}

void  AP_CocoaDialog_Lists::customChanged(void)
{
	fillUncustomizedValues();
	loadXPDataIntoLocal();
}


void AP_CocoaDialog_Lists::updateFromDocument(void)
{
	PopulateDialogData();
	_setRadioButtonLabels();
	setNewListType(getDocListType());
	loadXPDataIntoLocal();
}

void AP_CocoaDialog_Lists::updateDialog(void)
{
	if(!isDirty())
	{
		updateFromDocument();
	}
	else
	{
		setXPFromLocal();
	}
}

void AP_CocoaDialog_Lists::setAllSensitivity(void)
{
	PopulateDialogData();
	if(getisListAtPoint())
	{
	}
}


void AP_CocoaDialog_Lists::_fillFontMenu(NSPopUpButton* menu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	[menu removeAllItems];
	AppendLocalizedMenuItem(menu, pSS, AP_STRING_ID_DLG_Lists_Current_Font, 0);

	NSArray * list = [[[NSFontManager sharedFontManager] availableFontFamilies] 
	                      sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
	NSUInteger nfonts = list.count;

	for (NSUInteger i = 0; i < nfonts; i++)
	{
		[menu addItemWithTitle:[list objectAtIndex:i]];
		[[menu lastItem] setTag:(i + 1)];
#if 0
		NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:[list objectAtIndex:i]
				action:nil keyEquivalent:@""];
		[item setTag:i+1];
		[[menu menu] addItem:item];
#endif
	}
}

void AP_CocoaDialog_Lists::_fillNoneStyleMenu(NSMenu *listmenu)
{
	NSMenuItem*	item;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	[listmenu removeItemAtIndex:0];
	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Type_none)
			action:nil keyEquivalent:@""];
	[item setTag:(int)NOT_A_LIST];
	[listmenu addItem:item];
	[item release];
}

void AP_CocoaDialog_Lists::_fillNumberedStyleMenu(NSMenu *listmenu)
{
	NSMenuItem*	item;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	[listmenu removeItemAtIndex:0];
	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Numbered_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)NUMBERED_LIST];
	[listmenu addItem:item];
	[item release];
	
	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Lower_Case_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)LOWERCASE_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Upper_Case_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)UPPERCASE_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Lower_Roman_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)LOWERROMAN_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Upper_Roman_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)UPPERROMAN_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Hebrew_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)HEBREW_LIST];
	[listmenu addItem:item];
	[item release];
}


void AP_CocoaDialog_Lists::_fillBulletedStyleMenu(NSMenu *listmenu)
{
	NSMenuItem*	item;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	[listmenu removeItemAtIndex:0];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Bullet_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)BULLETED_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Dashed_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)DASHED_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Square_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)SQUARE_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Triangle_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)TRIANGLE_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Diamond_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)DIAMOND_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Star_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)STAR_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Implies_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)IMPLIES_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Tick_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)TICK_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Box_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)BOX_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Hand_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)HAND_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Heart_List)
			action:nil keyEquivalent:@""];
	[item setTag:(int)HEART_LIST];
	[listmenu addItem:item];
	[item release];

	item = [[NSMenuItem alloc] initWithTitle:LocalizedString(pSS, AP_STRING_ID_DLG_Lists_Arrowhead_List)  
			action:nil keyEquivalent:@""];
	[item setTag:(int)ARROWHEAD_LIST];
	[listmenu addItem:item];
	[item release];
}

void AP_CocoaDialog_Lists::_setRadioButtonLabels(void)
{
//	const XAP_StringSet * pSS = m_pApp->getStringSet();
	PopulateDialogData();
	// Button 0 is Start New List, button 2 is resume list
//	gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
//	gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
}

void AP_CocoaDialog_Lists::loadXPDataIntoLocal(void)
{
	//
	// This function reads the various memeber variables and loads them into
	// into the dialog variables.
	//
	m_bDontUpdate = true;

	UT_DEBUGMSG(("loadXP newListType = %d \n",getNewListType()));
	[m_dlg->_textAlignData setFloatValue:getfAlign()];
	float indent = getfAlign() + getfIndent();
	[m_dlg->_labelAlignData setFloatValue:indent];
	if( (getfIndent() + getfAlign()) < 0.0)
	{
		setfIndent( - getfAlign());
		[m_dlg->_labelAlignData setFloatValue:0.0];
	}
	//
	// Code to work out which is active Font
	//
	if(getFont() == "NULL")
	{
		[m_dlg->_fontPopup selectItemAtIndex:0];
	}
	else
	{
		[m_dlg->_fontPopup selectItemWithTitle:[NSString stringWithUTF8String:getFont().c_str()]];
	}
	[m_dlg->_startAtData setIntValue:getiStartValue()];

	[m_dlg->_levelDelimData setStringValue:[NSString stringWithUTF8String:getDecimal().c_str()]];
	[m_dlg->_formatData setStringValue:[NSString stringWithUTF8String:getDelim().c_str()]];

	//
	// Now set the list type and style
	FL_ListType save = getNewListType();
	if(getNewListType() == NOT_A_LIST)
	{
		typeChanged(GUI_LIST_NONE);
		setNewListType(save);
		[m_dlg->_typePopup selectItemAtIndex:GUI_LIST_NONE];
		[m_dlg->_stylePopup selectItemAtIndex:[m_dlg->_stylePopup indexOfItemWithTag:(int)NOT_A_LIST]];
	}
	else if(getNewListType() >= BULLETED_LIST)
	{
		typeChanged(GUI_LIST_BULLETED);
		setNewListType(save);
		[m_dlg->_typePopup selectItemAtIndex:GUI_LIST_BULLETED];
		[m_dlg->_stylePopup selectItemAtIndex:[m_dlg->_stylePopup indexOfItemWithTag:(int)getNewListType()]];
	}
	else
	{
		typeChanged(GUI_LIST_NUMBERED);
	    setNewListType(save);
		[m_dlg->_typePopup selectItemAtIndex:GUI_LIST_NUMBERED];
		[m_dlg->_stylePopup selectItemAtIndex:[m_dlg->_stylePopup indexOfItemWithTag:(int)getNewListType()]];
	}

	m_bDontUpdate = false;
}

bool    AP_CocoaDialog_Lists::dontUpdate(void)
{
        return m_bDontUpdate;
}

/*!
 * This method reads the various elements in the Customize box and loads
 * the XP member variables with them
 */
void AP_CocoaDialog_Lists::_gatherData(void)
{
	UT_sint32 maxWidth = getBlock()->getFirstContainer()->getContainer()->getWidth();

	float fmaxWidthIN = ((float) maxWidth / GR_CocoaGraphics::_getScreenResolution()) - 0.6;
	setiLevel(1);
	float f = [m_dlg->_textAlignData floatValue];
	if(f >   fmaxWidthIN)
	{
		f = fmaxWidthIN;
		[m_dlg->_textAlignData setFloatValue:f];
	}
	setfAlign(f);
	float indent = 		[m_dlg->_labelAlignData floatValue];
	if((indent - f) > fmaxWidthIN )
	{
		indent = fmaxWidthIN + f;
		[m_dlg->_labelAlignData setFloatValue:indent];
	}
	setfIndent(indent - getfAlign());
	if( (getfIndent() + getfAlign()) < 0.0)
	{
		setfIndent(- getfAlign());
		[m_dlg->_labelAlignData setFloatValue:0.0];
	}
	int ifont =  [m_dlg->_fontPopup indexOfSelectedItem];
	if(ifont == 0)
	{
		copyCharToFont("NULL");
	}
	else
	{
		copyCharToFont([[[m_dlg->_fontPopup selectedItem] title] UTF8String]);
	}
	copyCharToDecimal([[m_dlg->_levelDelimData stringValue] UTF8String]);
	setiStartValue([m_dlg->_startAtData intValue]);
	copyCharToDelim([[m_dlg->_formatData stringValue] UTF8String]);
}




@implementation AP_CocoaDialog_ListsController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Lists"];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Lists*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = nil;
	_preview.drawable = nil;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

//	[[self window] setTitle:[NSString stringWithUTF8String:_xap->getWindowName()]];
	if(!_xap->isModal()) {
		LocalizeControl(_applyBtn, pSS, XAP_STRING_ID_DLG_Apply);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Close);
	}
	else {
		LocalizeControl(_applyBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
	}
	[_typePopup removeAllItems];
	AppendLocalizedMenuItem(_typePopup, pSS, AP_STRING_ID_DLG_Lists_Type_none, GUI_LIST_NONE);
	AppendLocalizedMenuItem(_typePopup, pSS, AP_STRING_ID_DLG_Lists_Type_bullet, GUI_LIST_BULLETED);
	AppendLocalizedMenuItem(_typePopup, pSS, AP_STRING_ID_DLG_Lists_Type_numbered, GUI_LIST_NUMBERED);
	[_typePopup selectItemAtIndex:GUI_LIST_NONE];
	_xap->_fillNoneStyleMenu(m_listStyleNone_menu);
	_xap->_fillNumberedStyleMenu(m_listStyleNumbered_menu);
	_xap->_fillBulletedStyleMenu(m_listStyleBulleted_menu);
	[_stylePopup setMenu:m_listStyleNone_menu];
	
	LocalizeControl(_typeLabel, pSS, AP_STRING_ID_DLG_Lists_Type);
	LocalizeControl(_styleLabel, pSS, AP_STRING_ID_DLG_Lists_Style);
	LocalizeControl(_setDefaultBtn, pSS, AP_STRING_ID_DLG_Lists_SetDefault);
	_xap->_fillFontMenu(_fontPopup);
	LocalizeControl(_formatLabel, pSS, AP_STRING_ID_DLG_Lists_Format);
	LocalizeControl(_fontLabel, pSS, AP_STRING_ID_DLG_Lists_Font);
	LocalizeControl(_levelDelimLabel, pSS, AP_STRING_ID_DLG_Lists_DelimiterString);
	LocalizeControl(_startAtLabel, pSS, AP_STRING_ID_DLG_Lists_Start);
	LocalizeControl(_textAlignLabel, pSS, AP_STRING_ID_DLG_Lists_Align);
	LocalizeControl(_labelAlignLabel, pSS, AP_STRING_ID_DLG_Lists_Indent);
	LocalizeControl(_previewBox, pSS, AP_STRING_ID_DLG_Lists_Preview);
	LocalizeControl(_startNewListBtn, pSS, AP_STRING_ID_DLG_Lists_Start_New);
	LocalizeControl(_applyToCurrentBtn, pSS, AP_STRING_ID_DLG_Lists_Apply_Current);
	LocalizeControl(_attachToPreviousBtn, pSS, AP_STRING_ID_DLG_Lists_Resume);

	LocalizeControl([_mainTab tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_Lists_PageProperties);
	LocalizeControl([_mainTab tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_Lists_PageFolding);
	
	LocalizeControl(_noFoldingBtn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevelexp);
	LocalizeControl(_noFoldingBtn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevel0);
	LocalizeControl(_foldLevel1Btn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevel1);
	LocalizeControl(_foldLevel2Btn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevel2);
	LocalizeControl(_foldLevel3Btn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevel3);
	LocalizeControl(_foldLevel4Btn, pSS, AP_STRING_ID_DLG_Lists_FoldingLevel4);
}

- (XAP_CocoaNSView*)preview
{
	return _preview;
}

- (NSMenuItem*)selectedListType
{
	return [_typePopup selectedItem];
}


- (NSMenuItem*)selectedListStyle
{
	return [_stylePopup selectedItem];
}

- (void)setStyleMenu:(int)type
{
	switch(type) {
	case GUI_LIST_NONE:
		if ([_stylePopup menu] != m_listStyleNone_menu) {
			[_stylePopup selectItemAtIndex:0];	// make sure first item is selected before changing.
			[_stylePopup setMenu:m_listStyleNone_menu];
		}
		break;
	case GUI_LIST_BULLETED:
		if ([_stylePopup menu] != m_listStyleBulleted_menu) {
			[_stylePopup selectItemAtIndex:0];	// make sure first item is selected before changing.
			[_stylePopup setMenu:m_listStyleBulleted_menu];
		}
		break;
	case GUI_LIST_NUMBERED:
		if ([_stylePopup menu] != m_listStyleNumbered_menu) {
			[_stylePopup selectItemAtIndex:0];	// make sure first item is selected before changing.
			[_stylePopup setMenu:m_listStyleNumbered_menu];
		}
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}
}

- (NSInteger)listAction
{
	return [_listActionMatrix selectedColumn];
}

- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if(Current_Dialog->isDirty()) {
		return;
	}
	if(Current_Dialog->getAvView()->getTick() != Current_Dialog->getTick()) {
		Current_Dialog->setTick(Current_Dialog->getAvView()->getTick());
		Current_Dialog->updateDialog();
	}
}

- (void)selectFolding:(int)folding
{
	[_foldingMatrix selectCellWithTag:folding];
}


// return the tab that is selected.
- (NSInteger)selectedTab
{
	return [_mainTab indexOfTabViewItem: [_mainTab selectedTabViewItem]];
}

- (IBAction)applyAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->applyClicked();
	// [NSApp stopModal];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->destroy();
}

- (IBAction)labelAlignAction:(id)sender
{
	[_labelAlignData setFloatValue:[_labelAlignStepper floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)labelAlignActionStepper:(id)sender
{
	[_labelAlignStepper setFloatValue:[_labelAlignData floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)setDefaultAction:(id)sender
{
	UT_UNUSED(sender);
  	_xap->setDirty();
	_xap->customChanged();
}

- (IBAction)startAtAction:(id)sender
{
	[_startAtStepper setFloatValue:[_startAtData floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)startAtStepperAction:(id)sender
{
	[_startAtData setFloatValue:[_startAtStepper floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)styleChangedAction:(id)sender
{
	UT_UNUSED(sender);
	if(_xap->dontUpdate()) {
		return;
	}
  	_xap->setDirty();
	_xap->setListTypeFromWidget(); // Use this to set m_newListType
	_xap->fillUncustomizedValues(); // Use defaults to start.
	_xap->loadXPDataIntoLocal(); // Load them into our member variables
	_xap->previewInvalidate();
}

- (IBAction)textAlignAction:(id)sender
{
	[_textAlignStepper setFloatValue:[_textAlignData floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)textAlignActionStepper:(id)sender
{
	[_textAlignData setFloatValue:[_textAlignStepper floatValue]];
	[self valueChangedAction:sender];
}

- (IBAction)typeChangedAction:(id)sender
{
	int style = [[sender selectedItem] tag];
	[self setStyleMenu:style];
	/* force the update.*/
	[self styleChangedAction:_stylePopup];
}

- (IBAction)valueChangedAction:(id)sender
{
	UT_UNUSED(sender);
	if(_xap->dontUpdate()) {
		return;
	}
  	_xap->setDirty();
	_xap->setXPFromLocal(); // Update member Variables
	_xap->previewInvalidate();
}


- (IBAction)foldingChanged:(id)sender
{
	NSMatrix *m = sender;
	
	_xap->_foldingChanged([m selectedRow]);
}


@end





