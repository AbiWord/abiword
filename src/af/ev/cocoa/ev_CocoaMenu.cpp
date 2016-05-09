/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2004, 2007, 2009 Hubert Figuiere
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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stack>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaFrameImpl.h"
#import "xap_CocoaCompat.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaToolPalette.h"
#include "xap_Types.h"

#include "ev_CocoaMenu.h"
#include "ev_CocoaMenuBar.h"
#include "ev_CocoaMenuPopup.h"
#include "ev_CocoaKeyboard.h"
#include "ev_EditEventMapper.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_Toolbar_Actions.h"

#include "ap_CocoaFrame.h"
#include "ap_Menu_Id.h"

EV_CocoaMenuBar::EV_CocoaMenuBar(const char * szMenuLayoutName, const char * szMenuLabelSetName) :
	EV_CocoaMenu(szMenuLayoutName, szMenuLabelSetName, false)
{
	// 
}

EV_CocoaMenuBar::~EV_CocoaMenuBar()
{
	// 
}


EV_CocoaMenuPopup::EV_CocoaMenuPopup(const char * szMenuLayoutName, const char * szMenuLabelSetName) :
	EV_CocoaMenu(szMenuLayoutName, szMenuLabelSetName, true)
{
	// 
}

EV_CocoaMenuPopup::~EV_CocoaMenuPopup()
{
	// 
}

NSMenu * EV_CocoaMenuPopup::getMenuHandle() const
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	return [pController contextMenu];
}

bool EV_CocoaMenuPopup::synthesizeMenuPopup()
{
	buildAppMenu();
	return true;
}

bool EV_CocoaMenuPopup::refreshMenu(AV_View * /*pView*/)
{
	return true;
}


@implementation EV_CocoaMenuTarget

- (id)initWithMenu:(EV_CocoaMenu *)menu
{
	if (self = [super init]) {
		m_menu = menu;
	}
	return self;
}

- (void)menuSelected:(id)sender
{
	if ([sender isKindOfClass:[NSMenuItem class]])
	{
		XAP_Menu_Id menuid = static_cast<XAP_Menu_Id>([sender tag]);
		
		UT_DEBUGMSG(("[EV_CocoaMenuTarget -menuSelected:] (menu-id=%d)\n", (int) menuid));
		
		if (!m_menu->menuEvent(menuid))
		{
			UT_DEBUGMSG(("[EV_CocoaMenuTarget -menuSelected:] (menu-id=%d) - failed!\n", (int) menuid));
		}
	}
	else
	{
		UT_DEBUGMSG(("[EV_CocoaMenuTarget -menuSelected:] (ignored)\n"));
	}
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	XAP_Menu_Id menuid = static_cast<XAP_Menu_Id>([menuItem tag]);

	bool bEnabled = true;
	bool bChecked = false;

	const char * szLabel = 0;

	m_menu->validateMenuItem(menuid, bEnabled, bChecked, szLabel);

	if (szLabel)
	{
		[menuItem setTitle:(m_menu->convertToString(szLabel))];
	}
	[menuItem setState:(bChecked ? NSOnState : NSOffState)];

	return bEnabled ? YES : NO;
}

@end

@implementation EV_CocoaFontTarget

- (id)init
{
	if (self = [super init]) {
		m_Fonts = [[NSMutableArray alloc] initWithCapacity:128];
	}
	return self;
}

- (void)dealloc
{
	[m_Fonts release];
	[super dealloc];
}

- (NSMenuItem *)fontMenuItem:(NSString *)title
{
	NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];

	[item setTarget:self];
	// [item setAction:@selector(menuSelected:)];
	[item setTag:((int) (-1))];

	NSMenu * family_menu = [[NSMenu alloc] initWithTitle:title];
	NSFontManager * FM = [NSFontManager sharedFontManager];
	NSArray * Families = [[FM availableFontFamilies] sortedArrayUsingSelector:@selector(compare:)];
	float font_size = [NSFont systemFontSize];
	int tag = 0;
	unsigned family_count = [Families count];

	for (unsigned f_i = 0; f_i < family_count; f_i++)
	{
		NSString * Family = [Families objectAtIndex:f_i];
		NSArray * Members = [FM availableMembersOfFontFamily:Family];
		unsigned member_count = [Members count];
		
		if (member_count == 1)
		{
			NSArray * Member = [Members objectAtIndex:0];
			NSString * font_name = [Member objectAtIndex:0];
			NSMenuItem * member_item = [[NSMenuItem alloc] initWithTitle:Family action:nil keyEquivalent:@""];
			
			if ([member_item respondsToSelector:@selector(setAttributedTitle:)])
			{
				NSFont * font = [NSFont fontWithName:font_name size:font_size];
				NSDictionary * attr = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
				NSAttributedString * attr_title = [[NSAttributedString alloc] initWithString:Family attributes:attr];
				
				[member_item setAttributedTitle:attr_title];
				[attr_title release];
			}
			[member_item setTarget:self];
			[member_item setAction:@selector(menuSelected:)];
			[member_item setTag:tag];
			
			[family_menu addItem:member_item];
			
			[member_item release];
			
			[m_Fonts addObject:font_name];
			
			tag++;
		}
		else if (member_count > 1)
		{
			NSArray * Member = [Members objectAtIndex:0];
			NSString * font_name = [Member objectAtIndex:0];
			NSMenuItem * family_item = [[NSMenuItem alloc] initWithTitle:Family 
										action:nil keyEquivalent:@""];
			
			if ([family_item respondsToSelector:@selector(setAttributedTitle:)])
			{
				NSFont * font = [NSFont fontWithName:font_name size:font_size];
				NSDictionary * attr = [NSDictionary dictionaryWithObject:font 
									   forKey:NSFontAttributeName];
				NSAttributedString * attr_title = [[NSAttributedString alloc] 
												   initWithString:Family attributes:attr];
				
				[family_item setAttributedTitle:attr_title];
				
				[attr_title release];
			}
			[family_item setTarget:self];
			// [family_item setAction:@selector(menuSelected:)];
			[family_item setTag:((int) (-1))];
			
			NSMenu * member_menu = [[NSMenu alloc] initWithTitle:Family];
			
			for (unsigned m_i = 0; m_i < member_count; m_i++)
			{
				Member = [Members objectAtIndex:m_i];
				font_name = [Member objectAtIndex:0];
				NSString * member_name = [Member objectAtIndex:1];
				NSMenuItem * member_item = [[NSMenuItem alloc] initWithTitle:member_name 
											action:nil keyEquivalent:@""];
				
				if ([member_item respondsToSelector:@selector(setAttributedTitle:)])
				{
					NSFont * font = [NSFont fontWithName:font_name size:font_size];
					NSDictionary * attr = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
					NSAttributedString * attr_title = [[NSAttributedString alloc] initWithString:member_name attributes:attr];
					
					[member_item setAttributedTitle:attr_title];
					
					[attr_title release];
				}
				[member_item setTarget:self];
				[member_item setAction:@selector(menuSelected:)];
				[member_item setTag:tag];
				
				[member_menu addItem:member_item];
				
				[member_item release];
				
				[m_Fonts addObject:font_name];
				
				tag++;
			}
			[family_item setSubmenu:member_menu];
			[member_menu release];
			[family_menu addItem:family_item];
			[family_item release];
		}
	}
	[item setSubmenu:family_menu];
	[family_menu release];
	[item autorelease];
	return item;
}

- (void)menuSelected:(id)sender
{
	int tag = [sender tag];
	if (tag >= 0) {
		if (XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame()) {
			if (AV_View * pView = pFrame->getCurrentView()) {
				const XAP_App * app = XAP_App::getApp();
				const EV_Toolbar_ActionSet * pToolbarActionSet = app->getToolbarActionSet();
				UT_ASSERT(pToolbarActionSet);
				if (!pToolbarActionSet)
					return;
				
				const EV_EditMethodContainer * pEditMethodContainer = app->getEditMethodContainer();
				UT_ASSERT(pEditMethodContainer);
				if (!pEditMethodContainer)
					return;
				
				const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_FONT);
				UT_ASSERT(pAction);
				if (!pAction)
					return;
				
				const char * szMethodName = pAction->getMethodName();
				if (!szMethodName)
					return;
				
				EV_EditMethod * pEM = pEditMethodContainer->findEditMethodByName(szMethodName);
				if (!pEM)
					return;
				
				NSString * font_name = [m_Fonts objectAtIndex:tag];
				
				UT_UCS4String selection([font_name UTF8String]);
				
				const UT_UCS4Char * pData = selection.ucs4_str();
				UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());
				
				EV_EditMethodCallData emcd(pData, dataLength);
				pEM->Fn(pView, &emcd);
			}
		}
	}
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	UT_UNUSED(menuItem);
	return XAP_App::getApp()->getLastFocussedFrame() ? YES : NO;
}

@end

EV_CocoaMenu::EV_CocoaMenu(const char * szMenuLayoutName, const char * szMenuLabelSetName, bool bContextMenu) 
: EV_Menu(XAP_App::getApp(), XAP_App::getApp()->getEditMethodContainer(), 
		  szMenuLayoutName, szMenuLabelSetName),
	m_menuTarget(0),
	m_fontTarget(0),
	m_AppMenuCurrent(static_cast<XAP_CocoaAppMenu_Id>(0)),
	m_menuStack(0),
	m_buffer(0),
	m_maxlen(0),
	m_bContextMenu(bContextMenu),
	m_bAddSeparator(false)
{
	m_menuTarget = [[EV_CocoaMenuTarget alloc] initWithMenu:this];
	m_fontTarget = [[EV_CocoaFontTarget alloc] init];
	UT_ASSERT(m_menuTarget);
}

EV_CocoaMenu::~EV_CocoaMenu()
{
	[m_menuTarget release];
	[m_fontTarget release];
	DELETEPV(m_buffer);
}

void EV_CocoaMenu::buildAppMenu()
{
	UT_DEBUGMSG(("EV_CocoaMenu::buildAppMenu()\n"));

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	if (m_bContextMenu)
		[pController clearContextMenu];
	else
		[pController clearAllMenus];

	m_AppMenuCurrent = static_cast<XAP_CocoaAppMenu_Id>(0); // XAP_CocoaAppMenu_AbiWord, technically

	const EV_Menu_ActionSet * pMenuActionSet = XAP_App::getApp()->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	if (!pMenuActionSet)
		return;
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout);
	if (!nrLabelItemsInLayout)
		return;

	std::stack<NSMenu*> stack;
	m_menuStack = &stack;

	for (UT_uint32 k = 0; k < nrLabelItemsInLayout; k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);
		
		EV_Menu_LayoutFlags flags = pLayoutItem->getMenuLayoutFlags();
		
		XAP_Menu_Id menuid = pLayoutItem->getMenuId();
		
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
		UT_continue_if_fail(pAction);
		
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(menuid);
		UT_continue_if_fail(pLabel);
		
		bool bNewAppMenuItem = false;
		
		if (!m_bContextMenu) {
			switch (menuid) // handle top-level menus
			{
			case AP_MENU_ID_FILE:
				m_AppMenuCurrent = XAP_CocoaAppMenu_File;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_EDIT:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Edit;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_VIEW:
				m_AppMenuCurrent = XAP_CocoaAppMenu_View;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_INSERT:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Insert;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_FORMAT:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Format;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_TOOLS:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Tools;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_TABLE:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Table;
				bNewAppMenuItem = true;
				break;
			case AP_MENU_ID_WINDOW:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Window;
				bNewAppMenuItem = true;
				// I want to skip window-menu entries; see below
				break;
			case AP_MENU_ID_HELP:
				m_AppMenuCurrent = XAP_CocoaAppMenu_Help;
				bNewAppMenuItem = true;
				break;
			default:
				break;
			}
		}
		if ((!m_AppMenuCurrent && !m_bContextMenu) 
			|| (m_AppMenuCurrent == XAP_CocoaAppMenu_Window)) // I want to skip window-menu entries
		{
			continue;
		}
		if (bNewAppMenuItem)
		{
			MenuStack_clear();
			
			const char ** data = getLabelName(XAP_App::getApp(), pAction, pLabel);
			UT_ASSERT(data);
			if (!data)
				break; // erk!
			
			const char * szLabelName = data[0];
			UT_ASSERT(szLabelName);
			if (szLabelName)
			{
				[pController setTitle:convertToString(szLabelName) forMenu:m_AppMenuCurrent];
			}
			continue;
		}
		addToAppMenu(menuid, pAction, pLabel, flags);
	}
	MenuStack_clear(); // shouldn't be anything in it, but maybe
	m_menuStack = 0;
}

void EV_CocoaMenu::addToAppMenu(XAP_Menu_Id menuid, const EV_Menu_Action * pAction, 
								const EV_Menu_Label * pLabel, EV_Menu_LayoutFlags flags)
{
	switch (flags)
	{
	case EV_MLF_BeginSubMenu:
	{
		const char ** data = getLabelName(XAP_App::getApp(), pAction, pLabel);
		UT_ASSERT(data);
		if (!data)
			break; // erk!
		
		const char * szLabelName = data[0];
		UT_ASSERT(szLabelName);
		if (!szLabelName)
			break; // erk!
		
		NSString * itemName = convertToString(szLabelName);
		NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:itemName 
							 action:nil keyEquivalent:@""];
		NSMenu * menu = [[NSMenu alloc] initWithTitle:itemName];
		
		[item setSubmenu:menu];
		[item setTag:menuid];
		
		addToAppMenu(item);
		
		MenuStack_push(menu);
		
		[menu release];
		[item release];
	}
	break;
	
	case EV_MLF_EndSubMenu:
		MenuStack_pop();
		break;
	case EV_MLF_Normal:
	{
		const char ** data = getLabelName(XAP_App::getApp(), pAction, pLabel);
		UT_ASSERT(data);
		if (!data)
			break; // erk!
		
		//	const char * szMnemonicName = data[1]; // standard AbiWord-defined short-cut, e.g., Shift+Ctrl+?
		const char * szLabelName    = data[0];
		//	UT_ASSERT(szLabelName); // this happens 5 times atm (possibly more for new users?) in the recent files list
		if (!szLabelName)
			break; // erk!
		
		NSString * itemName = convertToString(szLabelName, (menuid == AP_MENU_ID_HELP_ABOUT));
		
		switch (menuid)
		{
		case AP_MENU_ID_HELP_ABOUT:
		{
			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController setAboutTitle:itemName];
			// TODO: setKeyEquivalent (although none for this usually)
		}
		break;
		
		case AP_MENU_ID_TOOLS_OPTIONS:
		{
			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController setPrefsTitle:itemName];
			// TODO: setKeyEquivalent (usually Cmd-,)
		}
		break;
		case AP_MENU_ID_FILE_EXIT:
			// let's leave the title of this alone
			// TODO: setKeyEquivalent (usually Cmd-Q)
			break;
			// fall through...
			
		default:
		{
			NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:itemName action:nil keyEquivalent:@""];
			
			if (!m_bContextMenu)
			{
				XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
				
				unsigned int mask = 0;
				if (const char * equiv = [pController keyEquivalentForMenuID:menuid modifierMask:&mask])
				{
					[item setKeyEquivalent:[NSString stringWithUTF8String:equiv]];
					[item setKeyEquivalentModifierMask:mask];
				}
			}
			
			[item setTarget:m_menuTarget];
			[item setAction:@selector(menuSelected:)];
			[item setTag:menuid];
			
			addToAppMenu(item);
			
			[item release];
		}
		break;
		}
	}
	break;
	
	case EV_MLF_Separator:
		m_bAddSeparator = true;
		m_SeparatorID = menuid;
		break;
		
	case EV_MLF_BeginPopupMenu:
	case EV_MLF_EndPopupMenu:
		break;
		
	default:
		UT_ASSERT_NOT_REACHED();
		break;
	}
}

void EV_CocoaMenu::addToAppMenu(NSMenuItem * item)
{
	if (!m_menuStack->empty())
	{
		// UT_DEBUGMSG(("EV_CocoaMenu::addToAppMenu(\"%s\") [submenu]\n", [[item title] UTF8String]));
		NSMenu * menu = m_menuStack->top();
		if (m_bAddSeparator)
		{
			m_bAddSeparator = false;
			
			NSMenuItem * separator = [NSMenuItem separatorItem];
			[separator setTag:m_SeparatorID];
			
			[menu addItem:separator];
		}
		[menu addItem:item];
	}
	else
	{
		// UT_DEBUGMSG(("EV_CocoaMenu::addToAppMenu(\"%s\") [appmenu]\n", [[item title] UTF8String]));
		XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
		
		if (m_bAddSeparator)
		{
			m_bAddSeparator = false;
			
			NSMenuItem * separator = [NSMenuItem separatorItem];
			[separator setTag:m_SeparatorID];
			
			if (m_bContextMenu)
				[pController appendContextItem:separator];
			else
				[pController appendItem:separator toMenu:m_AppMenuCurrent];
		}
		if (m_bContextMenu)
			[pController appendContextItem:item];
		else
			[pController appendItem:item toMenu:m_AppMenuCurrent];
	}
}

bool EV_CocoaMenu::menuEvent(XAP_Menu_Id menuid)
{
	const EV_Menu_ActionSet * pMenuActionSet = XAP_App::getApp()->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	if (!pMenuActionSet)
		return false;

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
	UT_ASSERT(pAction);
	if (!pAction)
		return false;

	const char * szMethodName = pAction->getMethodName();
	UT_ASSERT(szMethodName);
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	UT_ASSERT(pEMC);
	if (!pEMC)
		return false;

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);
	if (!pEM)
		return false;

	UT_String script_name(pAction->getScriptName());

	XAP_Frame * frame = XAP_App::getApp()->getLastFocussedFrame();

	AV_View * view = frame ? frame->getCurrentView() : 0;

	return invokeMenuMethod(view, pEM, script_name);
}

void EV_CocoaMenu::validateMenuItem(XAP_Menu_Id menuid, bool & bEnabled, bool & bChecked, const char *& szLabel)
{
	bEnabled = true;
	bChecked = false;

	XAP_App * pApp = XAP_App::getApp();

	XAP_Frame * pFrame = pApp->getLastFocussedFrame();

	AV_View * pView = pFrame ? pFrame->getCurrentView() : 0;

	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	if (!pMenuActionSet)
		return;

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	if (!pEMC)
		return;

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
	if (!pAction)
		return;

	const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(menuid);
	if (!pLabel)
		return;

	EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(m_pMenuLayout->getLayoutIndex(menuid));
	if (!pLayoutItem)
		return;

	switch (pLayoutItem->getMenuLayoutFlags())
	{
	case EV_MLF_Normal:
		if (pAction->hasGetStateFunction())
		{
			if (pView)
			{
				EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
				if (mis & EV_MIS_Gray)
					bEnabled = false;
				if (mis & EV_MIS_Toggled)
					bChecked = true;
			}
			else
			{
				bEnabled = false;
			}
		}
		else if (const char * methodName = pAction->getMethodName()) // TODO FIXME: store this somewhere
		{
			EV_EditMethod * pEM = pEMC->findEditMethodByName(methodName);
			if (pEM)
			{
				if (!(pEM->getType() & EV_EMT_APP_METHOD))
				{
					bEnabled = (pView != NULL);
				}
			}
			else
			{
				bEnabled = false;
			}
		}
		if (pAction->hasDynamicLabel()) {
			if (const char ** data = getLabelName(pApp, pAction, pLabel)) {
				if (data[0])
				{
					szLabel = data[0];
				}
			}
		}
		break;
	case EV_MLF_BeginSubMenu:
		bEnabled = (pView != 0);
		if (menuid == AP_MENU_ID_FILE_RECENT)
		{
			if (!pAction->getMenuItemState(pView))
			{
				bEnabled = true;
			}
		}
		else if (pAction->hasGetStateFunction())
		{
			if (pView)	{
				EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
				if (mis & EV_MIS_Gray)	{
					bEnabled = false;
				}
			}
		}
		break;
		
	case EV_MLF_EndSubMenu:
	case EV_MLF_Separator:
	case EV_MLF_BeginPopupMenu:
	case EV_MLF_EndPopupMenu:
		break;
		
	default:
		UT_ASSERT_NOT_REACHED();
		break;
	}
}

NSString * EV_CocoaMenu::convertToString(const char * label, bool strip_dots)
{
	if (!label)
		return @"";
	if (*label == 0)
		return @"";

	size_t length = strlen(label);

	if (m_maxlen <= length)
	{
		DELETEPV(m_buffer);
		m_buffer = new char[length+1];
		UT_ASSERT(m_buffer);
		if (m_buffer)
			m_maxlen = length + 1;
		else
			return @"";
	}
	_convertLabelToMac(m_buffer, m_maxlen, label);
	
	if (strip_dots)
	{
		length = strlen(m_buffer);
		if (length > 3)
		{
			char * ptr = m_buffer + length - 1;
			if (*ptr-- == '.') {
				if (*ptr-- == '.') {
					if (*ptr == '.') {
						*ptr = 0;
					}
				}
			}
		}
	}
	return [NSString stringWithUTF8String:m_buffer];
}

void EV_CocoaMenu::MenuStack_clear()
{
	while (/* NSMenu * menu = */ MenuStack_pop())
		;
}

void EV_CocoaMenu::MenuStack_push(NSMenu * menu)
{
	[menu retain];
	m_menuStack->push(menu);
}

NSMenu * EV_CocoaMenu::MenuStack_pop()
{
	m_bAddSeparator = false;

	NSMenu * menu = nil;
	if (!m_menuStack->empty()) {
		menu = m_menuStack->top();
		m_menuStack->pop();
	}

	[menu autorelease];
	return menu;
}

bool EV_CocoaMenu::_doAddMenuItem(UT_uint32 /*layout_pos*/)
{
	return false;
}

#if 0

/* **************************************************************** OLD IMPLEMENTATION **************************************************************** */

/*!
	Return the menu shortcut for the mnemonic
	
	\param mnemonic the string for the menmonic
	\returnvalue modifier the modifiers
	\return newly allocated NSString that contains the key equivalent. 
	should be nil on entry.
 */
NSString* EV_CocoaMenu::_getItemCmd (const char * mnemonic, unsigned int & modifiers, UT_uint32 * keyRefKey)
{
	bool needsShift = false;
	modifiers = 0;
	char * p;
	if (strstr (mnemonic, "Alt+")) {
		modifiers |= NSAlternateKeyMask;
	}
	if (strstr (mnemonic, "Ctrl+") != NULL) {
		modifiers |= NSCommandKeyMask;
	}
	if (strstr (mnemonic, "Shift+") != NULL) {
		needsShift = true;
	}
	if ((modifiers & NSCommandKeyMask) == 0) {
		p = (char *)mnemonic;
	}
	else {
		p = strrchr (mnemonic, '+');
		p++;
	}
	if (keyRefKey && (((*p) & 0x7f) == (*p))) {
		char c = *p;
		if ((c >= 'a') && (c <= 'z'))
			c = static_cast<char>(toupper(static_cast<int>(c)));

		*keyRefKey = static_cast<UT_uint32>(c);

		if (modifiers & NSAlternateKeyMask)
			*keyRefKey |= EV_COCOAMENU_MODALTERNATE;
		if (modifiers & NSCommandKeyMask)
			*keyRefKey |= EV_COCOAMENU_MODCOMMAND;
		if (modifiers & NSShiftKeyMask)
			*keyRefKey |= EV_COCOAMENU_MODSHIFT;
	}
	NSString *shortcut = nil;
	if ((p[1] == 0) && needsShift) {
		shortcut = [[NSString stringWithUTF8String:p] uppercaseString];
	}
	else {
		shortcut = [NSString stringWithUTF8String:p];
	}
	xxx_UT_DEBUGMSG(("returning shortcut %s\n", [shortcut UTF8String]));
	return shortcut;
}

#endif

/*****************************************************************/

#if 0

@implementation EV_CocoaMenuDelegate

- (id)initWithMenuBar:(EV_CocoaMenuBar *)menuBar
{
	if (self = [super init])
		{
			m_MenuBar = menuBar;
		}
	return self;
}

- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)target action:(SEL *)action
{
	BOOL bHasEquivalent = NO;

	UT_DEBUGMSG(("[EV_CocoaMenuDelegate -menuHasKeyEquivalent]\n"));
	if ([event type] == NSKeyDown)
	{
		unsigned int modifierFlags = [event modifierFlags];
		
		NSString * str = [event charactersIgnoringModifiers];
		if ([str length] == 1)
		{
			unichar uc;
			[str getCharacters:&uc];
			
			if ((uc & 0x7f) == uc)
			{
				char c = static_cast<char>(uc & 0x7f);
				
				if ((c >= 'a') && (c <= 'z'))
				{
					c = static_cast<char>(toupper (static_cast<int>(c)));
					bHasEquivalent = [self queryCommandKey:c withModifierFlags:modifierFlags];
				}
				else
					bHasEquivalent = [self queryCommandKey:c withModifierFlags:modifierFlags];
			}
			if (bHasEquivalent)
			{
				*target = m_KeyRef.target;
				*action = m_KeyRef.action;
				
				m_MenuBar->menuEvent (m_KeyRef.menuid);
			}
		}
	}
	return bHasEquivalent;
}

- (BOOL)queryCommandKey:(char)c withModifierFlags:(unsigned int)modifierFlags
{
	m_KeyRef.key = static_cast<UT_uint32>(c);

	if (modifierFlags & NSShiftKeyMask    )
		m_KeyRef.key |= EV_COCOAMENU_MODSHIFT;
	if (modifierFlags & NSControlKeyMask  )
		m_KeyRef.key |= EV_COCOAMENU_MODCONTROL;
	if (modifierFlags & NSAlternateKeyMask)
		m_KeyRef.key |= EV_COCOAMENU_MODALTERNATE;
	if (modifierFlags & NSCommandKeyMask  )
		m_KeyRef.key |= EV_COCOAMENU_MODCOMMAND; // I suspect that this one is necessarily true...

	return (m_MenuBar->lookupCommandKey (&m_KeyRef) ? YES : NO);
}

@end

EV_CocoaMenuBar::EV_CocoaMenuBar(XAP_CocoaApp * pCocoaApp, const char * szMenuLayoutName, const char * szMenuLabelSetName) :
	EV_CocoaMenu(pCocoaApp, szMenuLayoutName, szMenuLabelSetName)
{
	char buf[1024];

	if (!s_pMenuItem_FileNew)
		{
			const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(AP_MENU_ID_FILE_NEW);
			_convertLabelToMac(buf, sizeof (buf), pLabel->getMenuLabel());
			NSString * title = [NSString stringWithUTF8String:buf];
			s_pMenuItem_FileNew = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
			[s_pMenuItem_FileNew setAction:@selector(dockFileNew:)];
		}
	if (!s_pMenuItem_FileOpen)
		{
			const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(AP_MENU_ID_FILE_OPEN);
			_convertLabelToMac(buf, sizeof (buf), pLabel->getMenuLabel());
			NSString * title = [NSString stringWithUTF8String:buf];
			s_pMenuItem_FileOpen = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
			[s_pMenuItem_FileOpen setAction:@selector(dockFileOpen:)];
		}
}

EV_CocoaMenuBar::~EV_CocoaMenuBar()
{
	UT_VECTOR_PURGEALL(struct EV_CocoaCommandKeyRef *,m_vecKeyRef);
}

void  EV_CocoaMenuBar::destroy(void)
{
}

bool EV_CocoaMenuBar::synthesizeMenuBar(NSMenu *menu)
{
	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
	m_wMenuBar = menu;

	synthesizeMenu(m_wMenuBar, this);
	
	EV_CocoaMenuDelegate * menuDelegate = [[EV_CocoaMenuDelegate alloc] initWithMenuBar:this];
	if (menuDelegate)
	{
		XAP_CocoaApplication * sharedApplication = (XAP_CocoaApplication *) NSApp;
		[sharedApplication setMenuDelegate:menuDelegate];
		[menuDelegate release];
	}
	return true;
}


bool EV_CocoaMenuBar::rebuildMenuBar()
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	return false;
}

bool EV_CocoaMenuBar::lookupCommandKey (struct EV_CocoaCommandKeyRef * keyRef) const
{
	bool found = false;

	for (UT_uint32 i = 0; i < m_vecKeyRef.getItemCount(); i++)
		{
			const struct EV_CocoaCommandKeyRef * savedKeyRef = 0;
			savedKeyRef = reinterpret_cast<const struct EV_CocoaCommandKeyRef *>(m_vecKeyRef.getNthItem(i));

			if (savedKeyRef->key == keyRef->key)
				{
					keyRef->menuid = savedKeyRef->menuid;
					keyRef->target = savedKeyRef->target;
					keyRef->action = savedKeyRef->action;
					found = true;
					break;
				}
		}
	return found;
}

void EV_CocoaMenuBar::addCommandKey (const struct EV_CocoaCommandKeyRef * keyRef)
{
	struct EV_CocoaCommandKeyRef * newKeyRef = new struct EV_CocoaCommandKeyRef;

	newKeyRef->key    = keyRef->key;
	newKeyRef->menuid = keyRef->menuid;
	newKeyRef->target = keyRef->target;
	newKeyRef->action = keyRef->action;

	m_vecKeyRef.addItem (reinterpret_cast<void *>(newKeyRef));
}

@interface EV_CocoaPaletteMenuItem : NSMenuItem
{
	BOOL	m_bPreview;
}
-(id)initWithTitle:(NSString *)title forPreview:(BOOL)preview;
-(void)togglePanel:(id)sender;
-(void)update;
@end

@implementation EV_CocoaPaletteMenuItem

-(id)initWithTitle:(NSString *)title forPreview:(BOOL)preview
{
	[super initWithTitle:title action:nil keyEquivalent:@""];
	m_bPreview = preview;
	return self;
}

-(void)togglePanel:(id)sender
{
	BOOL bInstantiated = [XAP_CocoaToolPalette instantiated];
	
	if (bInstantiated)
	{
		NSWindow * window = m_bPreview ? [[XAP_CocoaToolPalette instance:self] previewPanel] : [[XAP_CocoaToolPalette instance:self] window];
		
		if ([window isVisible])
			[window orderOut:self];
		else
			[window orderFront:self];
	}
}

-(void)update
{
	BOOL bVisible = NO;

	if ([XAP_CocoaToolPalette instantiated])
	{
		NSWindow * window = m_bPreview 
			? [[XAP_CocoaToolPalette instance:self] previewPanel] 
			: [[XAP_CocoaToolPalette instance:self] window];
		bVisible = [window isVisible];
	}
	
	if (bVisible)
		[self setState:NSOnState];
	else
		[self setState:NSOffState];
}

@end

@interface EV_CocoaFramedMenuItem : NSMenuItem
{
	XAP_Frame *	m_pFrame;
}
-(id)initWithTitle:(NSString *)title forXAPFrame:(XAP_Frame *)frame;
-(id)selectDocument:(id)sender;
-(void)update;
@end

@implementation EV_CocoaFramedMenuItem

-(id)initWithTitle:(NSString *)title forXAPFrame:(XAP_Frame *)frame
{
	[super initWithTitle:title action:nil keyEquivalent:@""];
	m_pFrame = frame;
	return self;
}

-(id)selectDocument:(id)sender
{
	m_pFrame->raise();
	return self;
}

-(void)update
{
	NSWindow * window = static_cast<XAP_CocoaFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();

	if ([window isKeyWindow])
		[self setState:NSOnState];
	else
		[self setState:NSOffState];
}

@end

@implementation EV_CocoaDockMenu

-(id)initWithNumberOfFrames:(int)numberOfFrames
{
	[super initWithTitle:@"Dock"];
	m_numberOfFrames = numberOfFrames;
	m_pMenuItem_Palette = 0;
	m_pMenuItem_Preview = 0;
	return self;
}

-(void)setMenuItem_Palette:(EV_CocoaPaletteMenuItem *)pMenuItem_Palette
{
	m_pMenuItem_Palette = pMenuItem_Palette;
}

-(void)setMenuItem_Preview:(EV_CocoaPaletteMenuItem *)pMenuItem_Preview
{
	m_pMenuItem_Preview = pMenuItem_Preview;
}

-(void)menuNeedsUpdate
{
	for (int i = 0; i < m_numberOfFrames; i++)
	{
		EV_CocoaFramedMenuItem * pMenuItem = (EV_CocoaFramedMenuItem *) [self itemAtIndex:i];
		[pMenuItem update];
	}
	if (m_pMenuItem_Palette)
	{
		[m_pMenuItem_Palette update];
	}
	if (m_pMenuItem_Preview)
	{
		[m_pMenuItem_Preview update];
	}
}

@end

NSMenuItem *				EV_CocoaMenuBar::s_pMenuItem_FileNew  = 0;
NSMenuItem *				EV_CocoaMenuBar::s_pMenuItem_FileOpen = 0;
EV_CocoaPaletteMenuItem *	EV_CocoaMenuBar::s_pMenuItem_Palette  = 0;
EV_CocoaPaletteMenuItem *	EV_CocoaMenuBar::s_pMenuItem_Preview  = 0;

EV_CocoaDockMenu * EV_CocoaMenuBar::synthesizeDockMenu(const UT_Vector & vecDocs)
{
	XAP_CocoaAppController * pAppDelegate = (XAP_CocoaAppController *) [NSApp delegate];
	if (!pAppDelegate)
		return nil;

	EV_CocoaDockMenu * pDockMenu = [[EV_CocoaDockMenu alloc] initWithNumberOfFrames:((int) vecDocs.getItemCount())];
	if (!pDockMenu)
		return nil;

	if (s_pMenuItem_Palette == 0)
	{
		s_pMenuItem_Palette = [[EV_CocoaPaletteMenuItem alloc] initWithTitle:@"Tool Palette" forPreview:NO];
		
		[s_pMenuItem_Palette setTarget:s_pMenuItem_Palette];
		[s_pMenuItem_Palette setAction:@selector(togglePanel:)];
		
		[s_pMenuItem_Palette update];
		
		[pDockMenu setMenuItem_Palette:s_pMenuItem_Palette];
	}
	if (s_pMenuItem_Preview == 0)
	{
		s_pMenuItem_Preview = [[EV_CocoaPaletteMenuItem alloc] initWithTitle:@"Preview Panel" forPreview:YES];
		
		[s_pMenuItem_Preview setTarget:s_pMenuItem_Preview];
		[s_pMenuItem_Preview setAction:@selector(togglePanel:)];
		
		[s_pMenuItem_Preview update];
		
		[pDockMenu setMenuItem_Preview:s_pMenuItem_Preview];
	}
	
	if (vecDocs.getItemCount())
	{
		for (UT_uint32 i = 0; i < vecDocs.getItemCount(); i++)
		{
			XAP_Frame * pFrame = (XAP_Frame *) vecDocs.getNthItem(i);
			NSString * title = [NSString stringWithUTF8String:(pFrame->getTitle(80))];
			EV_CocoaFramedMenuItem * pMenuItem = 0;
			pMenuItem = [[EV_CocoaFramedMenuItem alloc] initWithTitle:title forXAPFrame:pFrame];
			if (pMenuItem)
			{
				[pMenuItem setAction:@selector(selectDocument:)];
				[pMenuItem setTarget:pMenuItem];
				if (pFrame->getCurrentView()->isActive())
				{
					[pMenuItem setState:NSOnState];
				}
				[pDockMenu addItem:pMenuItem];
				[pMenuItem release];
			}
		}
		[pDockMenu addItem:[NSMenuItem separatorItem]];
	}
	if (s_pMenuItem_Palette)
	{
		[pDockMenu addItem:s_pMenuItem_Palette];
	}
	if (s_pMenuItem_Preview)
	{
		[pDockMenu addItem:s_pMenuItem_Preview];
	}
	if (s_pMenuItem_FileNew)
	{
		[s_pMenuItem_FileNew setTarget:pAppDelegate];
		[pDockMenu addItem:s_pMenuItem_FileNew];
	}
	if (s_pMenuItem_FileOpen)
	{
		[s_pMenuItem_FileOpen setTarget:pAppDelegate];
		[pDockMenu addItem:s_pMenuItem_FileOpen];
	}
	return pDockMenu;
}

void EV_CocoaMenuBar::releaseDockMenu(EV_CocoaDockMenu * pMenu)
{
	if (s_pMenuItem_Palette)
	{
		[pMenu removeItem:s_pMenuItem_Palette];
	}
	if (s_pMenuItem_Preview)
	{
		[pMenu removeItem:s_pMenuItem_Preview];
	}
	if (s_pMenuItem_FileNew)
	{
		[pMenu removeItem:s_pMenuItem_FileNew];
	}
	if (s_pMenuItem_FileOpen)
	{
		[pMenu removeItem:s_pMenuItem_FileOpen];
	}
	[pMenu release];
}

#endif
