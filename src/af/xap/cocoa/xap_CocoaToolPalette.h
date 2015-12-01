/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2004 AbiSource, Inc.
 * Copyright (C) 2004 Francis James Franklin <fjf@alinameridon.com>
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


#ifndef XAP_COCOATOOLPALETTE_H
#define XAP_COCOATOOLPALETTE_H

#import <Cocoa/Cocoa.h>

#include "ap_Strings.h"
#include "ap_Toolbar_Id.h"

#include "xav_Listener.h"

class XAP_Frame;
class EV_Menu_ActionSet;
class EV_Toolbar_ActionSet;
class FV_View;

@class XAP_CocoaToolPalette;

struct XAP_CocoaPaletteRef
{
	NSString *		Name; // used internally to identify palette
	NSButton *		Title;
	NSBox *			Box;
};

struct XAP_CocoaToolRef
{
	NSButton *			button;
	XAP_Toolbar_Id		tlbrid;
	AP_String_Id_Enum	ttipid;
};

class XAP_CocoaToolPaletteListener : public AV_Listener
{
public:
	XAP_CocoaToolPaletteListener(XAP_CocoaToolPalette * pPalette);

	virtual ~XAP_CocoaToolPaletteListener();

	virtual bool				notify(AV_View * pView, const AV_ChangeMask mask);
    virtual AV_ListenerType		getType(void);

	void						setCurrentView(AV_View * view);

private:
	XAP_CocoaToolPalette *	m_pPalette;

	AV_View *				m_pView;
	AV_ListenerId			m_lid;
};

@interface XAP_CocoaPaletteView : NSView
{
	NSMutableArray *	m_Palette;
}
- (id)init;
- (void)dealloc;

- (void)addPalette:(const struct XAP_CocoaPaletteRef *)palette;

- (void)sync;
@end

@interface XAP_PaletteProperties_DataSource
	: NSObject<NSOutlineViewDataSource, NSOutlineViewDelegate>
{
	NSOutlineView *		m_OutlineView;
	NSMutableArray *	m_PropertyLevels;
}
- (id)initWithOutlineView:(NSOutlineView *)outlineView;
- (void)dealloc;

- (void)syncWithView:(FV_View *)pView;

/* NSOutlineViewDataSource
 */
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item;
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item;
- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;

/* NSOutlineView delegate method
 */
- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;
- (void)outlineView:(NSOutlineView *)outlineView willDisplayOutlineCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;
@end

@interface XAP_CocoaPreviewPanel : NSWindowController
{
	IBOutlet NSTextField *oPreview;
}
- (id)init;
- (void)dealloc;
- (void)windowDidLoad;
- (void)toggleVisibility:(id)sender;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)setPreviewString:(NSString *)previewString;
@end

@interface XAP_CocoaToolPalette : NSWindowController<NSWindowDelegate>
{
	struct XAP_CocoaToolRef *			m_ToolChest;

	XAP_CocoaPaletteView *				m_PaletteView;

	XAP_PaletteProperties_DataSource *	m_Properties_DataSource;

	IBOutlet XAP_CocoaPreviewPanel *	oPreviewPanel;

	IBOutlet NSButton *			oTitle_Standard;
	IBOutlet NSButton *			oTitle_Format;
	IBOutlet NSButton *			oTitle_Table;
	IBOutlet NSButton *			oTitle_Extra;
	IBOutlet NSButton *			oTitle_Properties;

	IBOutlet NSBox *			oBox_Standard;
	IBOutlet NSBox *			oBox_Format;
	IBOutlet NSBox *			oBox_Table;
	IBOutlet NSBox *			oBox_Extra;
	IBOutlet NSBox *			oBox_Properties;

	IBOutlet NSPopUpButton *	oDocumentStyle;
	IBOutlet NSPopUpButton *	oFontName;
	IBOutlet NSPopUpButton *	oFontMemberName;

	IBOutlet NSComboBox *		oFontSize;
	IBOutlet NSComboBox *		oZoom;

	IBOutlet NSColorWell *		oColor_BG;
	IBOutlet NSColorWell *		oColor_FG;

	IBOutlet NSButton *			oSwitch_BG;
	IBOutlet NSButton *			oSwitch_FG;

	IBOutlet NSTextField *		oPreview;

	IBOutlet NSOutlineView *	oProperties;

	IBOutlet NSPanel *			oPanel;

#ifdef defn
#undef defn
#endif
#define defn(T,X,Y)	IBOutlet NSButton * T;
#include "xap_CocoaTools.h"
#undef defn

	NSMutableArray *				m_pFontFamilies;

	NSString *						m_pCurrentFontFamily;

	const EV_Menu_ActionSet *		m_pMenuActionSet;
	const EV_Toolbar_ActionSet *	m_pToolbarActionSet;
	const EV_EditMethodContainer *	m_pEditMethodContainer;

	XAP_CocoaToolPaletteListener *	m_Listener;

	AV_View *		m_pViewCurrent;
	AV_View *		m_pViewPrevious;

	XAP_Frame *		m_pFrameCurrent;
	XAP_Frame *		m_pFramePrevious;
}
+ (XAP_CocoaToolPalette *)instance:(id)sender;
+ (BOOL)instantiated;

- (id)init;
- (void)dealloc;

- (void)windowDidLoad;
- (void)close;
- (void)windowWillClose:(NSNotification *)aNotification;

- (void)toggleVisibility:(id)sender;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

+ (void)setPreviewText:(id)previewText;
- (void)setPreviewString:(NSString *)previewString;

- (NSWindow *)previewPanel;

- (void)setColor:(XAP_Toolbar_Id)tlbrid;

- (IBAction)aColor_FG:(id)sender;
- (IBAction)aColor_BG:(id)sender;
- (IBAction)aSwitch_FG:(id)sender;
- (IBAction)aSwitch_BG:(id)sender;
- (IBAction)aDocumentStyle:(id)sender;
- (IBAction)aFontName:(id)sender;
- (IBAction)aFontMemberName:(id)sender;
- (IBAction)aFontSize:(id)sender;
- (IBAction)aTitle_click:(id)sender;
- (IBAction)aTB_click:(id)sender;
- (IBAction)aZoom:(id)sender;

- (void)sync;

- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;

- (void)rebuildFontFamilyPopUp;
- (void)syncPopUpsForFont:(NSString *)requestedFontFamilyName;
@end

#endif /* ! XAP_COCOATOOLPALETTE_H */
