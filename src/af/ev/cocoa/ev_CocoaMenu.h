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

#ifndef EV_COCOAMENU_H
#define EV_COCOAMENU_H

#include <stack>
#import <Cocoa/Cocoa.h>

#include "ut_types.h"

#include "xap_CocoaAppController.h"
#include "xap_Types.h"

#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"


class AV_View;

class XAP_CocoaApp;

class EV_CocoaMenu;
class EV_CocoaMenuBar;
class EV_Menu_Action;
class EV_Menu_Label;

class AP_CocoaFrame;

/*****************************************************************/

@interface EV_CocoaMenuTarget : NSObject
{
	EV_CocoaMenu *	m_menu;
}
- (id)initWithMenu:(EV_CocoaMenu *)menu;
- (void)menuSelected:(id)sender;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
@end

@interface EV_CocoaFontTarget : NSObject
{
	NSMutableArray *	m_Fonts;
}
- (id)init;
- (void)dealloc;
- (NSMenuItem *)fontMenuItem:(NSString *)title;
- (void)menuSelected:(id)sender;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
@end

class EV_CocoaMenu : public EV_Menu
{
public:
	EV_CocoaMenu(const char * szMenuLayoutName, const char * szMenuLabelSetName, bool bContextMenu);

	virtual ~EV_CocoaMenu();

	void				buildAppMenu();
private:
	void				addToAppMenu(XAP_Menu_Id menuid, const EV_Menu_Action * pAction, const EV_Menu_Label * pLabel, EV_Menu_LayoutFlags flags);
	void				addToAppMenu(NSMenuItem * item);
public:
	bool				menuEvent(XAP_Menu_Id menuid);

	void				validateMenuItem(XAP_Menu_Id menuid, bool & bEnabled, bool & bChecked, const char *& szLabel);

	NSString *			convertToString(const char * label, bool strip_dots = false);

private:
	/*
	static NSString *	_getItemCmd (const char * mnemonic, unsigned int & modifiers, UT_uint32 * keyRefKey = 0);
	 */
	EV_CocoaMenuTarget *	m_menuTarget;
	EV_CocoaFontTarget *	m_fontTarget;
	XAP_CocoaAppMenu_Id		m_AppMenuCurrent;

	std::stack<NSMenu*> *				m_menuStack;

	void					MenuStack_clear();
	void					MenuStack_push(NSMenu * menu);
	NSMenu *				MenuStack_pop();

protected:
	virtual bool			_doAddMenuItem(UT_uint32 layout_pos); // does nothing, returns false

private:
	char *					m_buffer;
	UT_uint32				m_maxlen;

	bool					m_bContextMenu;
	bool					m_bAddSeparator;
	XAP_Menu_Id				m_SeparatorID;
};

#endif /* EV_COCOAMENU_H */
