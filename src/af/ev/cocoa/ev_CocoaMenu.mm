/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2004 Hubert Figuiere
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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_exception.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"

#include "xap_Types.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaToolPalette.h"

#include "ev_CocoaMenu.h"
#include "ev_CocoaMenuBar.h"
#include "ev_CocoaMenuPopup.h"
#include "ev_CocoaKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

#include "ap_Menu_Id.h"
#include "ap_CocoaFrame.h"

#import "xap_CocoaFrameImpl.h"

@implementation EV_NSMenu

-(void)dealloc
{
	[_virtualItems release];
	[super dealloc];
}


-(id)initWithXAP:(EV_CocoaMenu*)owner andTitle:(NSString*)title
{
	self = [super initWithTitle:title];
	_virtualItems = [[NSMutableArray alloc] init];
	_xap = owner;
	return self;
}

// NSMenu overrides

-(void)update
{
	xxx_UT_DEBUGMSG(("-[EV_NSMenu update: %s]\n", [[self title] UTF8String]));
	_xap->_refreshMenu(self);
	[super update];
}

- (void)addVirtualItem:(id <NSMenuItem>)newItem
{
	[_virtualItems addObject:newItem];
}


- (NSEnumerator*)virtualItemsEnumerator
{
	return [_virtualItems objectEnumerator];
}

@end


@implementation EV_CocoaMenuTarget

- (void)setXAPOwner:(EV_CocoaMenu*)owner
{
	_xap = owner;
}


- (id)menuSelected:(id)sender
{
	UT_DEBUGMSG (("@EV_CocoaMenuTarget (id)menuSelected:(id)sender\n"));

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


void
EV_CocoaMenu::_refreshMenu(EV_NSMenu *menu)
{
	XAP_App* app = XAP_App::getApp();
	AV_View* pView;
	XAP_Frame* frame = app->getLastFocussedFrame();
	if (frame) {
		pView = frame->getCurrentView();
	}
	else {
		pView = NULL;
	}
	const EV_Menu_ActionSet * pMenuActionSet = app->getMenuActionSet();
	const EV_EditMethodContainer * pEMC = app->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	NSMenuItem *menuItem;
	NSEnumerator * enumerator = [[menu itemArray] objectEnumerator];
	while (menuItem = [enumerator nextObject]) {
		[menu removeItem:menuItem];
	}
	
	enumerator = [menu virtualItemsEnumerator];
	
	while (menuItem = [enumerator nextObject]) {
		XAP_Menu_Id cmd = [menuItem tag];
		
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(cmd);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(cmd);
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(m_pMenuLayout->getLayoutIndex(cmd));
		
		switch (pLayoutItem->getMenuLayoutFlags()) {
		case EV_MLF_Normal:
		{			
			// see if we need to enable/disable and/or check/uncheck it.
			bool bEnable = true;
			bool bCheck = false;
			
			if (pAction->hasGetStateFunction())
			{
				if (pView) {
					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray)
						bEnable = false;
					if (mis & EV_MIS_Toggled)
						bCheck = true;
				}
				else {
					bEnable = false;
					bCheck = false;				
				}
			}
			else {
				// TODO FIXME: store this somewhere
				const char *methodName = pAction->getMethodName();

				EV_EditMethod * pEM = pEMC->findEditMethodByName(methodName);
				if (pEM) {
					EV_EditMethodType t = pEM->getType();
					if (!(t & EV_EMT_APP_METHOD)) {
						bEnable = (pView != NULL);
					}
					else {
						bEnable = true;
					}
				}
				else {
					bEnable = false;
				}
			}

			// Get the dynamic label
			const char ** data = getLabelName(app, pAction, pLabel);
			const char * szLabelName = data[0];
			const char * szMnemonicName = data[1];		
			bool bHasDynamicLabel = pAction->hasDynamicLabel();
			
			/* 
			 we need to set the memnomic if current is empty even if it is not dynamic. Because we
			 may be running the first update after building the menu bar in the application without
			 having a frame. No frame = no shortcut actuallly because event mapper is tied to the frame.
			*/
			if (szMnemonicName && *szMnemonicName 
						&& (bHasDynamicLabel || [[menuItem keyEquivalent] isEqualToString:@""])) {
				NSString* shortCut;
				unsigned int modifier = 0;
				UT_DEBUGMSG(("changing menu shortcut\n"));
				shortCut = _getItemCmd (szMnemonicName, modifier);
				[menuItem setKeyEquivalent:shortCut];
			}

			// No dynamic label, check/enable
			if (!bHasDynamicLabel)
			{
				[menu addItem:menuItem];
			}
			else {
				szLabelName = pAction->getDynamicLabel(pLabel);
				if (szLabelName && *szLabelName) {
					char buf[1024];
					_convertLabelToMac(buf, sizeof (buf), szLabelName);
					[menuItem setTitle:[NSString stringWithUTF8String:buf]];
					[menu addItem:menuItem];
				}
			}
			
			[menuItem setState:(bCheck?NSOnState:NSOffState)];
			[menuItem setEnabled:(bEnable?YES:NO)];
			break;
		}
		case EV_MLF_Separator:
			[menu addItem:menuItem];
			break;

		case EV_MLF_BeginSubMenu:
		{
			bool bEnable = (pView != NULL);
			if (cmd == AP_MENU_ID_FILE_RECENT) {
					if (!pAction->getMenuItemState(pView)) {
						bEnable = true;
					}
			}
			else if (pAction->hasGetStateFunction()) {
				if (pView) {
					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray) {
						bEnable = false;
					}
				}
				else {
					bEnable = false;
				}
			}
			[menu addItem:menuItem];
			[menuItem setEnabled:(bEnable?YES:NO)];
			break;
		}
		case EV_MLF_EndSubMenu:
		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;
			
		default:
			UT_ASSERT_NOT_REACHED();
			break;
		}
	}
}


/*****************************************************************/


EV_CocoaMenu::EV_CocoaMenu(XAP_CocoaApp * pCocoaApp,
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pCocoaApp, pCocoaApp->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	  m_pCocoaApp(pCocoaApp)
{
	m_menuTarget = [[EV_CocoaMenuTarget alloc] init];
	[m_menuTarget setXAPOwner:this];
}

EV_CocoaMenu::~EV_CocoaMenu()
{
	[m_menuTarget release];
}

bool EV_CocoaMenu::menuEvent(XAP_Menu_Id menuid)
{
	// user selected something from the menu.
	// invoke the appropriate function.
	// return true if handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pCocoaApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pCocoaApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	UT_String script_name(pAction->getScriptName());
	XAP_Frame* frame = NULL;
	AV_View* view = NULL;
	frame = static_cast<XAP_CocoaApp*>(XAP_App::getApp())->_getFrontFrame();
	if (frame) {
		view =  frame->getCurrentView();
	}
	invokeMenuMethod(view, pEM, script_name);
	return true;
}

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	XAP_Frame * frame = m_pCocoaApp->getLastFocussedFrame();

bool EV_CocoaMenu::synthesizeMenu(NSMenu * wMenuRoot, EV_CocoaMenuBar * pMenuBar)
{
	UT_DEBUGMSG(("EV_CocoaMenu::synthesizeMenu\n"));
	const EV_Menu_ActionSet * pMenuActionSet = m_pCocoaApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	bool bResult;
	UT_Stack stack;
	stack.push(wMenuRoot);

	for (UT_uint32 k = 0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);
		
		XAP_Menu_Id menuid = pLayoutItem->getMenuId();
		// VERY BAD HACK!  It will be here until I fix the const correctness of all the functions
		// using EV_Menu_Action
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(menuid);
		UT_ASSERT(pAction);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(menuid);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName;
		const char * szMnemonicName;
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
		{
			unsigned int modifier = 0;
			const char ** data = getLabelName(m_pCocoaApp, pAction, pLabel);
			szLabelName = data[0];
			szMnemonicName = data[1];
			
			NSString * shortCut = nil;
			UT_uint32 keyRefKey = 0;

			if (data[1] && *(data[1])) {
				shortCut = _getItemCmd (data[1], modifier, &keyRefKey);
			}
			else {
				shortCut = [NSString string];
			}
			char buf[1024];
			// convert label into underscored version

			// create the item with the underscored label
			EV_NSMenu * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			NSMenuItem * menuItem = nil;
			NSString * str = nil;
			if (szLabelName) {
				_convertLabelToMac(buf, sizeof (buf), szLabelName);
				str = [[NSString alloc] initWithUTF8String:buf];
			}
			else {
				str = [[NSString alloc] init];
			}
			switch (menuid) {
			
			case AP_MENU_ID_HELP_ABOUT:
				menuItem = [XAP_AppController_Instance _aboutMenu];
				[menuItem setTitle:str];
				[menuItem setKeyEquivalent:shortCut];
				break;
			case AP_MENU_ID_TOOLS_OPTIONS:
				menuItem = [XAP_AppController_Instance _preferenceMenu];
				[menuItem setTitle:str];
				[menuItem setKeyEquivalent:shortCut];
				break;
			case AP_MENU_ID_FILE_EXIT:
				menuItem = [XAP_AppController_Instance _quitMenu];
				[menuItem setTitle:str];
				[menuItem setKeyEquivalent:shortCut];					
				break;
			default:
				menuItem = [[NSMenuItem alloc] initWithTitle:str action:nil
								keyEquivalent:shortCut];
				[wParent addVirtualItem:menuItem];
				[menuItem release];
			}
			if (pMenuBar && keyRefKey) {
				struct EV_CocoaCommandKeyRef keyRef;
				keyRef.key    = keyRefKey;
				keyRef.menuid = pLayoutItem->getMenuId();
				keyRef.target = m_menuTarget;
				keyRef.action = @selector(menuSelected:);
				pMenuBar->addCommandKey (&keyRef);
			}
			[menuItem setTarget:m_menuTarget];
			[menuItem setAction:@selector(menuSelected:)];
			[menuItem setTag:pLayoutItem->getMenuId()];
			[str release];
			break;
		}
		case EV_MLF_BeginSubMenu:
		{
			const char ** data = getLabelName(m_pCocoaApp, pAction, pLabel);
			szLabelName = data[0];
			
			char buf[1024];
			// convert label into underscored version
			// create the item with the underscored label
			EV_NSMenu * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);

			NSMenuItem * menuItem = nil;
			NSString * str = nil;
			if (szLabelName) {
				_convertLabelToMac(buf, sizeof (buf), szLabelName);
				str = [NSString stringWithUTF8String:buf];	
			}
			else {
				str = [NSString string]; // autoreleased
			}
			if ([wParent isKindOfClass:[EV_NSMenu class]]) {
				menuItem = [[NSMenuItem alloc] initWithTitle:str action:nil keyEquivalent:@""];
				[wParent addVirtualItem:menuItem];
				[menuItem release];
			}
			else {
				menuItem = [wParent addItemWithTitle:str action:nil keyEquivalent:@""];
			}
			
			// item is created, add to class vector
			[menuItem setTag:(int)pLayoutItem->getMenuId()];

			EV_NSMenu * subMenu = [[EV_NSMenu alloc] initWithXAP:this andTitle:str];
			[menuItem setSubmenu:subMenu];
			[subMenu setAutoenablesItems:NO];
			[subMenu release];
			stack.push((void **)subMenu);
			break;
		}
		case EV_MLF_EndSubMenu:
		{
			// pop and inspect
			EV_NSMenu * menu;
			bResult = stack.pop((void **)&menu);
			UT_ASSERT(bResult);

			break;
		}
		case EV_MLF_Separator:
		{	
			NSMenuItem * menuItem = nil;
			menuItem = [NSMenuItem separatorItem];

			EV_NSMenu * wParent;
			bResult = stack.viewTop((void **)&wParent);
			UT_ASSERT(bResult);
			[wParent addVirtualItem:menuItem];

			[menuItem setTag:(int)pLayoutItem->getMenuId()];
			break;
		}

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;
			
		default:
			UT_ASSERT_NOT_REACHED();
			break;
		}
	}

	// make sure our last item on the stack is the one we started with
	EV_NSMenu * wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == wMenuRoot);

	return true;
}


bool EV_CocoaMenu::_doAddMenuItem(UT_uint32 layout_pos)
{	
	UT_ASSERT_NOT_REACHED();
	return false;
}


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


/*****************************************************************/

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

static EV_CocoaMenuBar * s_cocoa_menu_bar = 0;

EV_CocoaMenuBar * EV_CocoaMenuBar::instance()
{
	return s_cocoa_menu_bar;
}

EV_CocoaMenuBar * EV_CocoaMenuBar::instantiate(XAP_CocoaApp * pCocoaApp, const char * szMenuLayoutName, const char * szMenuLabelSetName)
{
	if (!s_cocoa_menu_bar)
		{
			UT_TRY
				{
					s_cocoa_menu_bar = new EV_CocoaMenuBar(pCocoaApp, szMenuLayoutName, szMenuLabelSetName);
				}
			UT_CATCH(...)
				{
					s_cocoa_menu_bar = 0;
				}
		}
	return s_cocoa_menu_bar;
}

EV_CocoaMenuBar::EV_CocoaMenuBar(XAP_CocoaApp * pCocoaApp,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_CocoaMenu(pCocoaApp, szMenuLayoutName, szMenuLabelSetName)
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

bool EV_CocoaMenuBar::refreshMenu(AV_View * pView)
{
	return true;
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
	if (self = [super initWithTitle:title action:nil keyEquivalent:@""])
		{
			m_bPreview = preview;
		}
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
			NSWindow * window = m_bPreview ? [[XAP_CocoaToolPalette instance:self] previewPanel] : [[XAP_CocoaToolPalette instance:self] window];
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
	if (self = [super initWithTitle:title action:nil keyEquivalent:@""])
		{
			m_pFrame = frame;
		}
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
	if (self = [super initWithTitle:@"Dock"])
		{
			m_numberOfFrames = numberOfFrames;
			m_pMenuItem_Palette = 0;
			m_pMenuItem_Preview = 0;
		}
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

/*****************************************************************/

EV_CocoaMenuPopup::EV_CocoaMenuPopup(XAP_CocoaApp * pCocoaApp,
								   const char * szMenuLayoutName,
								   const char * szMenuLabelSetName)
	: EV_CocoaMenu(pCocoaApp, szMenuLayoutName, szMenuLabelSetName),
		m_wMenuPopup(nil)
{
}

EV_CocoaMenuPopup::~EV_CocoaMenuPopup()
{
	[m_wMenuPopup release];
}

NSMenu * EV_CocoaMenuPopup::getMenuHandle() const
{
	return m_wMenuPopup;
}

bool EV_CocoaMenuPopup::synthesizeMenuPopup()
{
	m_wMenuPopup = [[EV_NSMenu alloc] initWithXAP:this andTitle:@""];
	[m_wMenuPopup setAutoenablesItems:NO];
	synthesizeMenu(m_wMenuPopup);
	return true;
}

bool EV_CocoaMenuPopup::refreshMenu(AV_View * pView)
{
	return true;
}
