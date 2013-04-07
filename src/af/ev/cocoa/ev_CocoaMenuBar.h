/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef EV_COCOAMENUBAR_H
#define EV_COCOAMENUBAR_H

#import <Cocoa/Cocoa.h>

#include "ev_CocoaMenu.h"

class AV_View;
class AP_CocoaFrame;
class EV_CocoaMenuBar;

#if 0

#define EV_COCOAMENU_MODSHIFT     0x080
#define EV_COCOAMENU_MODCONTROL   0x100
#define EV_COCOAMENU_MODALTERNATE 0x200
#define EV_COCOAMENU_MODCOMMAND   0x400

struct EV_CocoaCommandKeyRef
{
	id		target;
	SEL		action;

	XAP_Menu_Id	menuid;

	UT_uint32	key;
};

@class EV_CocoaPaletteMenuItem;

@interface EV_CocoaMenuDelegate : NSObject
{
	EV_CocoaMenuBar *	m_MenuBar;

	struct EV_CocoaCommandKeyRef	m_KeyRef;
}
- (id)initWithMenuBar:(EV_CocoaMenuBar *)menuBar;
- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)target action:(SEL *)action;
- (BOOL)queryCommandKey:(char)c withModifierFlags:(unsigned int)modifierFlags;
@end

@interface EV_CocoaDockMenu : NSMenu
{
	int	m_numberOfFrames;

	EV_CocoaPaletteMenuItem *	m_pMenuItem_Palette;
	EV_CocoaPaletteMenuItem *	m_pMenuItem_Preview;
}
-(id)initWithNumberOfFrames:(int)numberOfFrames;
-(void)setMenuItem_Palette:(EV_CocoaPaletteMenuItem *)pMenuItem_Palette;
-(void)setMenuItem_Preview:(EV_CocoaPaletteMenuItem *)pMenuItem_Preview;
-(void)menuNeedsUpdate;
@end

#endif

/*****************************************************************/

class EV_CocoaMenuBar : public EV_CocoaMenu
{
public:
	EV_CocoaMenuBar(const char * szMenuLayoutName, const char * szMenuLabelSetName);

	virtual ~EV_CocoaMenuBar();

#if 0
	NSMenu *			getMenuBar() const { return m_wMenuBar; }

	bool				lookupCommandKey (struct EV_CocoaCommandKeyRef * keyRef) const;
	void				addCommandKey (const struct EV_CocoaCommandKeyRef * keyRef);

	static EV_CocoaDockMenu *	synthesizeDockMenu(const UT_Vector & vecDocs);
	static void					releaseDockMenu(EV_CocoaDockMenu * pMenu);
private:
	static NSMenuItem *					s_pMenuItem_FileNew;
	static NSMenuItem *					s_pMenuItem_FileOpen;
	static EV_CocoaPaletteMenuItem *	s_pMenuItem_Palette;
	static EV_CocoaPaletteMenuItem *	s_pMenuItem_Preview;
protected:
	NSMenu *			m_wMenuBar;
	UT_Vector			m_vecKeyRef;
#endif
};

#endif /* EV_COCOAMENUBAR_H */
