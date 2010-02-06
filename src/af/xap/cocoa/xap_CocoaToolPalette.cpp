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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"

#import "xap_CocoaApp.h"
#import "xap_CocoaAppController.h"
#import "xap_CocoaToolPalette.h"
#import "xap_CocoaToolbar_Icons.h"
#import "xap_CocoaCompat.h"

#include "xap_Frame.h"
#include "xap_Toolbar_LabelSet.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "xap_Prefs_SchemeIds.h"

#include "ev_EditEventMapper.h"
#include "ev_Menu_Actions.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"

#include "pd_Document.h"
#include "pd_Style.h"

#include "pp_AttrProp.h"

#include "fv_View.h"

#include "ap_Menu_Id.h"

enum _XAP_CocoaTool_Id
{
#define defn(T,X,Y)			ToolID_##T,
#include "xap_CocoaTools.h"
#undef defn
	XAP_COCOA_TOOL_ID__COUNT_
};

@interface XAP_CocoaPalette : NSObject
{
	NSString *		m_Name;
	NSButton *		m_Title;
	NSBox *			m_Box;

	UT_uint32		m_heightTitle;
	UT_uint32		m_heightBox;
}
- (id)initWithPalette:(const struct XAP_CocoaPaletteRef *)palette;
- (void)dealloc;

- (NSString *)Name;
- (NSButton *)Title;
- (NSBox *)Box;

- (BOOL)isExpanded;

- (UT_uint32)heightTitle;
- (UT_uint32)heightBox;
- (UT_uint32)heightConditional;
@end

@implementation XAP_CocoaPalette

- (id)initWithPalette:(const struct XAP_CocoaPaletteRef *)palette
{
	if(![super init]) {
		return nil;
	}
	UT_ASSERT(palette);
	UT_ASSERT(palette->Name);

	m_Name = [[NSString alloc] initWithString:(palette->Name)];
	UT_ASSERT(palette->Title && palette->Box);

	m_Title = palette->Title;
	m_Box   = palette->Box;

	[m_Title retain];
	[m_Box   retain];

	NSRect frame;

	frame = [m_Title frame];
	m_heightTitle = static_cast<UT_uint32>(frame.size.height);

	frame = [m_Box frame];
	m_heightBox = static_cast<UT_uint32>(frame.size.height);
	return self;
}

- (void)dealloc
{
	if (m_Name)
	{
		[m_Name release];
	}
	if (m_Title)
	{
		[m_Title release];
	}
	if (m_Box)
	{
		[m_Box release];
	}
	[super dealloc];
}

- (NSString *)Name
{
	return m_Name;
}

- (NSButton *)Title
{
	return m_Title;
}

- (NSBox *)Box
{
	return m_Box;
}

- (BOOL)isExpanded
{
	return ([m_Title tag] == 1) ? YES : NO;
}

- (UT_uint32)heightTitle
{
	return m_heightTitle;
}

- (UT_uint32)heightBox
{
	return m_heightBox;
}

- (UT_uint32)heightConditional
{
	UT_uint32 height = m_heightTitle;

	if ([m_Title tag] == 1)
	{
		height += m_heightBox;
	}
	return height;
}

@end

@implementation XAP_CocoaPaletteView

- (id)init
{
	NSRect frame;
	frame.origin.x    =   0;
	frame.origin.y    =   0;
	frame.size.width  = 300;
	frame.size.height = 100;

	self  = [super initWithFrame:frame];
	if (self) // ignore frame here
	{
		m_Palette = [[NSMutableArray alloc] initWithCapacity:4];
	}
	return self;
}

- (void)dealloc
{
	if (m_Palette)
	{
		[m_Palette release];
		m_Palette = 0;
	}
	[super dealloc];
}

- (void)addPalette:(const struct XAP_CocoaPaletteRef *)paletteRef
{
	XAP_CocoaPalette * palette = [[XAP_CocoaPalette alloc] initWithPalette:paletteRef];
	if (palette)
	{
		[m_Palette addObject:palette];
		[palette release];

		[paletteRef->Title removeFromSuperviewWithoutNeedingDisplay];
		[paletteRef->Box   removeFromSuperviewWithoutNeedingDisplay];

		[self sync]; // update frames

		[self addSubview:(paletteRef->Title)];
		[self addSubview:(paletteRef->Box  )];

		[self setNeedsDisplay:YES];
	}
}

#define PALETTE_ELEMENT_ORIGIN_X  -3
#define PALETTE_ELEMENT_WIDTH    219
#define PALETTE_PANEL_WIDTH      213

- (void)sync
{
	unsigned count = [m_Palette count];
	unsigned i;

	if (!count) {
		return; // huh. no palettes
	}

	/* First, calculate total height
	 */
	UT_uint32 view_height = 0;

	for (i = 0; i < count; i++)
	{
		XAP_CocoaPalette * palette = (XAP_CocoaPalette *) [m_Palette objectAtIndex:i];

		view_height += [palette heightConditional];
	}

	NSSize size;
	size.width  = PALETTE_PANEL_WIDTH;
	size.height = static_cast<float>(view_height);

	NSRect frame;

	UT_uint32 vh_remember = view_height;

	/* Adjust frames of individual palettes
	 */
	for (i = 0; i < count; i++)
	{
		XAP_CocoaPalette * palette = (XAP_CocoaPalette *) [m_Palette objectAtIndex:i];

		UT_uint32 height = [palette heightTitle];

		view_height -= height;

		/* Title
		 */

		/* Box
		 */
		height = [palette heightBox];

		if ([palette isExpanded])
		{
			view_height -= height;
		}
		else // move it out of sight
		{
			[self setNeedsDisplayInRect:[[palette Box] frame]];

			frame.origin.x    = PALETTE_ELEMENT_WIDTH;
			frame.origin.y    = static_cast<float>(view_height);
			frame.size.width  = PALETTE_ELEMENT_WIDTH;
			frame.size.height = static_cast<float>(height);
			[[palette Box] setFrame:frame];
		}
	}
	[self displayIfNeeded];

	view_height = vh_remember;

	/* Update panel size, if in a panel, else set the frame size
	 */
	NSWindow * window = [self window];
	if (window)
	{
		// [self setNeedsDisplay:NO];

		NSRect current = [window frame];

		int y = static_cast<int>(current.origin.y) + static_cast<int>(current.size.height);

		NSRect content;
		content.origin.x    = 0;
		content.origin.y    = 0;
		content.size.width  = size.width;
		content.size.height = size.height;

		frame = [NSWindow frameRectForContentRect:content styleMask:(NSTitledWindowMask|NSClosableWindowMask|NSUtilityWindowMask)];
		frame.origin.x = current.origin.x;
		frame.origin.y = static_cast<float>(y - static_cast<int>(frame.size.height));
		[window setFrame:frame display:YES animate:YES];
	}
	else
	{
		frame.origin.x    = 0;
		frame.origin.y    = 0;
		frame.size.width  = size.width;
		frame.size.height = size.height;
		[self setFrame:frame];
	}

	/* Adjust frames of individual palettes
	 */
	for (i = 0; i < count; i++)
	{
		XAP_CocoaPalette * palette = (XAP_CocoaPalette *) [m_Palette objectAtIndex:i];

		UT_uint32 height = [palette heightTitle];

		view_height -= height;

		/* Title
		 */
		frame.origin.x    = PALETTE_ELEMENT_ORIGIN_X;
		frame.origin.y    = static_cast<float>(view_height);
		frame.size.width  = PALETTE_ELEMENT_WIDTH;
		frame.size.height = static_cast<float>(height);
		[[palette Title] setFrame:frame];
		[[palette Title] setNeedsDisplay:YES];
		[[palette Title] displayIfNeeded];

		/* Box
		 */
		height = [palette heightBox];

		if ([palette isExpanded])
		{
			view_height -= height;

			frame.origin.x    = PALETTE_ELEMENT_ORIGIN_X;
			frame.origin.y    = static_cast<float>(view_height);
			frame.size.width  = PALETTE_ELEMENT_WIDTH;
			frame.size.height = static_cast<float>(height);
			[[palette Box] setFrame:frame];
			[[palette Box] setNeedsDisplay:YES];
			[[palette Box] displayIfNeeded];
		}
		else // move it out of sight
		{
			frame.origin.x    = PALETTE_ELEMENT_WIDTH;
			frame.origin.y    = static_cast<float>(view_height);
			frame.size.width  = PALETTE_ELEMENT_WIDTH;
			frame.size.height = static_cast<float>(height);
			[[palette Box] setFrame:frame];
		}
	}
}

@end

@interface XAP_PaletteProperties_Level : NSObject
{
	BOOL				m_IsLevel;

	NSString *			m_Name;
	NSString *			m_Value;

	NSMutableArray *	m_Properties;
}
- (id)initWithAP:(const PP_AttrProp *)pAP levelName:(NSString *)name;
- (id)initWithStyle:(PD_Style *)style levelName:(NSString *)name;
- (id)initWithPropertyName:(NSString *)name propertyValue:(NSString *)value;
- (void)dealloc;

- (int)numberOfChildrenOfItem;
- (BOOL)isItemExpandable;
- (id)child:(int)index;
- (id)objectValueForTableColumn:(NSTableColumn *)tableColumn;
@end

@implementation XAP_PaletteProperties_Level

- (id)initWithAP:(const PP_AttrProp *)pAP levelName:(NSString *)pname
{
	if (![super init])
	{
		return  nil;
	}
	
	m_IsLevel = YES;

	m_Name  = pname;
	m_Value = @"";

	[m_Name  retain];
	[m_Value retain];

	m_Properties = nil;

	UT_sint32 count = pAP->getPropertyCount();
	if (count) {
		m_Properties = [[NSMutableArray alloc] initWithCapacity:((unsigned) count)];
		if (!m_Properties) {
			[self release];
			return nil;
		}
		for (UT_sint32 i = 0; i < count; i++) {
			const char * szName  = NULL;
			const char * szValue = NULL;

			if (pAP->getNthProperty(i, szName, szValue)) {
				NSString * name  = [NSString stringWithUTF8String:(szName )];
				NSString * value = [NSString stringWithUTF8String:(szValue)];

				XAP_PaletteProperties_Level * property = [[XAP_PaletteProperties_Level alloc] initWithPropertyName:name propertyValue:value];
				if (property) {
					[m_Properties addObject:property];
					[property release];
				}
			}
		}
	}
	return self;
}

- (id)initWithStyle:(PD_Style *)style levelName:(NSString *)pname
{
	if (![super init]) {
		return nil;
	}
	m_IsLevel = YES;

	m_Name  = pname;
	m_Value = @"";

	[m_Name  retain];
	[m_Value retain];

	m_Properties = nil;

	UT_sint32 count = static_cast<int>(style->getPropertyCount());
	if (count) {
		m_Properties = [[NSMutableArray alloc] initWithCapacity:((unsigned) count)];
		if (!m_Properties) {
			[self release];
			return nil;
		}
		for (UT_sint32 i = 0; i < count; i++) {
			const char * szName  = NULL;
			const char * szValue = NULL;

			if (style->getNthProperty(i, szName, szValue)) {
				NSString * name  = [NSString stringWithUTF8String:(szName )];
				NSString * value = [NSString stringWithUTF8String:(szValue)];

				XAP_PaletteProperties_Level * property = [[XAP_PaletteProperties_Level alloc] initWithPropertyName:name propertyValue:value];
				if (property) {
					[m_Properties addObject:property];
					[property release];
				}
			}
		}
	}
	return self;
}

- (id)initWithPropertyName:(NSString *)name propertyValue:(NSString *)value
{
	if(![super init]) {
		return nil;
	}
	m_IsLevel = NO;

	m_Name  = name;
	m_Value = value;

	[m_Name  retain];
	[m_Value retain];

	m_Properties = nil;
	return self;
}

- (void)dealloc
{
	if (m_Name)
	{
		[m_Name  release];
	}
	if (m_Value)
	{
		[m_Value release];
	}
	if (m_Properties)
	{
		[m_Properties release];
	}
	[super dealloc];
}

- (int)numberOfChildrenOfItem
{
	if (m_IsLevel && m_Properties)
	{
		return [m_Properties count];
	}
	return 0;
}

- (BOOL)isItemExpandable
{
	return m_IsLevel;
}

- (id)child:(int)index
{
	if (m_IsLevel /* && m_Properties */)
	{
		return [m_Properties objectAtIndex:((unsigned) index)];
	}
	return 0; // ??
}

- (id)objectValueForTableColumn:(NSTableColumn *)tableColumn
{
	NSTableHeaderCell * headerCell = [tableColumn headerCell];

	if ([[headerCell stringValue] isEqualToString:@"Value"])
	{
		return m_Value;
	}
	return m_Name;
}

@end

static PD_Style * _getStyle(const PP_AttrProp * pAttrProp, PD_Document * pDoc)
{
	PD_Style * pStyle = 0;

	const gchar * szValue = 0;

// This is where the style/name split gets really hairy. This index AP MIGHT be
// from a style definition in which case the name of the style is PT_NAME_ATTRIBUTE_NAME
// or it might be from the document in which case the attribute is
// PT_STYLE_ATTRIBUTE_NAME. - MES.

	if (pAttrProp->getAttribute(PT_NAME_ATTRIBUTE_NAME, szValue))
	{
		UT_return_val_if_fail (szValue && szValue[0], 0);
		if (pDoc) {
			pDoc->getStyle(reinterpret_cast<const char*>(szValue), &pStyle);
		}
		
		// NOTE: we silently fail if style is referenced, but not defined
	}
    else if(pAttrProp->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
	{
		UT_return_val_if_fail (szValue && szValue[0], 0);
		if (pDoc) {
			pDoc->getStyle(reinterpret_cast<const char*>(szValue), &pStyle);
		}
		
		// NOTE: we silently fail if style is referenced, but not defined
	}
	return pStyle;
}

@implementation XAP_PaletteProperties_DataSource

- (id)initWithOutlineView:(NSOutlineView *)outlineView
{
	if (![super init])
	{
		return nil;
	}
	m_OutlineView = outlineView;

	m_PropertyLevels = [[NSMutableArray alloc] initWithCapacity:6];
	return self;
}

- (void)dealloc
{
	[m_OutlineView setDelegate:nil];
	[m_OutlineView setDataSource:nil];

	if (m_PropertyLevels)
	{
		[m_PropertyLevels release];
		m_PropertyLevels = 0;
	}
	[super dealloc];
}

- (void)syncWithView:(FV_View *)pView
{
	if (m_PropertyLevels)
	{
		[m_PropertyLevels removeAllObjects];
	}
	if (pView)
	{
		const PP_AttrProp * pSpanAP    = 0;
		const PP_AttrProp * pBlockAP   = 0;
		const PP_AttrProp * pSectionAP = 0;
		const PP_AttrProp * pDocAP     = 0;

		if (pView->getAllAttrProp(pSpanAP, pBlockAP, pSectionAP, pDocAP))
		{
			if (pSpanAP)
			{
				if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithAP:pSpanAP levelName:@"Span"])
				{
					[m_PropertyLevels addObject:pPPLevel];
					[pPPLevel release];
				}
				if (PD_Style * style = _getStyle(pSpanAP, pView->getDocument()))
				{
					NSString * name = [NSString stringWithUTF8String:(style->getName())];
					if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithStyle:style levelName:name])
					{
						[m_PropertyLevels addObject:pPPLevel];
						[pPPLevel release];
					}
				}
			}

			if (pBlockAP)
			{
				if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithAP:pBlockAP levelName:@"Block"])
				{
					[m_PropertyLevels addObject:pPPLevel];
					[pPPLevel release];
				}
				if (PD_Style * style = _getStyle(pBlockAP, pView->getDocument()))
				{
					NSString * name = [NSString stringWithUTF8String:(style->getName())];
					if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithStyle:style levelName:name])
					{
						[m_PropertyLevels addObject:pPPLevel];
						[pPPLevel release];
					}
				}
			}

			if (pSectionAP) 
			{
				if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithAP:pSectionAP levelName:@"Section"])
				{
					[m_PropertyLevels addObject:pPPLevel];
					[pPPLevel release];
				}
			}
			if (pDocAP) 
			{
				if (XAP_PaletteProperties_Level * pPPLevel = [[XAP_PaletteProperties_Level alloc] initWithAP:pDocAP levelName:@"Document"])
				{
					[m_PropertyLevels addObject:pPPLevel];
					[pPPLevel release];
				}
			}
		}
	}
	[m_OutlineView reloadData];
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	UT_UNUSED(outlineView);
	if (item)
	{
		XAP_PaletteProperties_Level * pPPLevel = (XAP_PaletteProperties_Level *) item;
		return [pPPLevel numberOfChildrenOfItem];
	}
	return (int) [m_PropertyLevels count];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	UT_UNUSED(outlineView);
	if (item)
	{
		XAP_PaletteProperties_Level * pPPLevel = (XAP_PaletteProperties_Level *) item;
		return [pPPLevel isItemExpandable];
	}
	return YES; // ??
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
	UT_UNUSED(outlineView);
	if (item)
	{
		XAP_PaletteProperties_Level * pPPLevel = (XAP_PaletteProperties_Level *) item;
		return [pPPLevel child:index];
	}
	return [m_PropertyLevels objectAtIndex:((unsigned) index)];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	UT_UNUSED(outlineView);
	if (item)
	{
		XAP_PaletteProperties_Level * pPPLevel = (XAP_PaletteProperties_Level *) item;
		return [pPPLevel objectValueForTableColumn:tableColumn];
	}
	return @"??"; // ??
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	UT_UNUSED(outlineView);
	UT_UNUSED(tableColumn);
	UT_UNUSED(item);
	[cell setFont:[NSFont systemFontOfSize:10.0f]];
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayOutlineCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	UT_UNUSED(outlineView);
	UT_UNUSED(tableColumn);
	UT_UNUSED(item);
	[cell setFont:[NSFont systemFontOfSize:10.0f]];
}

@end

@implementation XAP_CocoaPreviewPanel

- (id)init
{
	return [super init];
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)windowDidLoad
{
	NSPanel * panel = (NSPanel *) [self window];

	[panel setBecomesKeyOnlyIfNeeded:YES];

	[oPreview setStringValue:@"Preview"];

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	if (NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:@"Preview Panel" action:@selector(toggleVisibility:) keyEquivalent:@"I"])
	{
		[item setTarget:self];
		[pController appendPanelItem:item];
		[item release];
	}
}

- (void)toggleVisibility:(id)sender
{
	UT_UNUSED(sender);
	NSPanel * panel = (NSPanel *) [self window];

	if ([panel isVisible]) {
		[panel orderOut:self];
	}
	else {
		[panel orderFront:self];
	}
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem
{
	NSPanel * panel = (NSPanel *) [self window];

	[menuItem setState:([panel isVisible] ? NSOnState : NSOffState)];

	return YES;
}

- (void)setPreviewString:(NSString *)previewString
{
	[oPreview setStringValue:previewString];
}

@end

static XAP_CocoaToolPalette * s_instance = nil;

@implementation XAP_CocoaToolPalette

+ (XAP_CocoaToolPalette *)instance:(id)sender
{
	if (!s_instance) {
		s_instance = [[XAP_CocoaToolPalette alloc] init];
		if (!s_instance)
		{
			return nil;
		}
		bool visible = false;
		XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
		UT_ASSERT(pPrefs);

		XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
		UT_ASSERT(pPrefsScheme);
		
		pPrefsScheme->getValueBool(XAP_PREF_KEY_ToolPaletteVisible, &visible);
		if (visible) {
			[s_instance showWindow:sender];
		}
	}
	return s_instance;
}

+ (BOOL)instantiated
{
	return (s_instance ? YES : NO);
}

- (id)init
{
	if(![super initWithWindowNibName:@"xap_CocoaToolPalette"]) {
		return nil;
	}
	m_ToolChest = 0;
	m_PaletteView = 0;
	m_Properties_DataSource = 0;

	m_pMenuActionSet = 0;
	m_pToolbarActionSet = 0;
	m_pEditMethodContainer = 0;
	m_pFontFamilies = 0;
	m_pCurrentFontFamily = 0;
	m_Listener = 0;

	XAP_CocoaApp * pCocoaApp = dynamic_cast<XAP_CocoaApp *>(XAP_App::getApp());
	UT_ASSERT(pCocoaApp);

	m_pMenuActionSet       = pCocoaApp->getMenuActionSet();
	m_pToolbarActionSet    = pCocoaApp->getToolbarActionSet();
	m_pEditMethodContainer = pCocoaApp->getEditMethodContainer();

	if (!m_pToolbarActionSet || !m_pMenuActionSet || !m_pEditMethodContainer)
	{
		UT_ASSERT(m_pToolbarActionSet && m_pMenuActionSet && m_pEditMethodContainer);
	}
	NSArray * pAvailableFontFamilies = [[NSFontManager sharedFontManager] availableFontFamilies];

	unsigned count = [pAvailableFontFamilies count];
	if (count)
	{
		m_pFontFamilies = [[NSMutableArray alloc] initWithCapacity:count];
		if (m_pFontFamilies) {
			for (unsigned ff = 0; ff < count; ff++) {
				NSString * pFontFamily = [pAvailableFontFamilies objectAtIndex:ff];

				/* const char * szFF = [pFontFamily UTF8String]; */

				if (true /* (*szFF != '.') && (*szFF != '#') */) // cf. Bug 6638
				{
					[m_pFontFamilies addObject:pFontFamily];
				}
			}
			if ([m_pFontFamilies count]) {
				[m_pFontFamilies sortUsingSelector:@selector(compare:)];
			}
			else {
				UT_DEBUGMSG(("XAP_CocoaToolPalette -init: no usable font families?\n"));

				[m_pFontFamilies release];
				m_pFontFamilies = 0;
			}
		}
	}
	else
	{
		UT_DEBUGMSG(("XAP_CocoaToolPalette -init: no available font families?\n"));
	}
	try
	{
		m_ToolChest = new struct XAP_CocoaToolRef[XAP_COCOA_TOOL_ID__COUNT_];
	}
	catch(...)
	{
		m_ToolChest = nil;
	}
	m_PaletteView = [[XAP_CocoaPaletteView alloc] init];
	try
	{
		m_Listener = new XAP_CocoaToolPaletteListener(self);
	}
	catch(...)
	{
		m_Listener = 0;
	}
	return self;
}

- (void)dealloc
{
	if (m_Properties_DataSource)
	{
		[m_Properties_DataSource release];
		m_Properties_DataSource = 0;
	}
	if (m_PaletteView) {
		if (![m_PaletteView superview])
		{
			[m_PaletteView dealloc];
			m_PaletteView = 0;
		}
	}
	if (m_pFontFamilies)
	{
		[m_pFontFamilies release];
		m_pFontFamilies = 0;
	}
	if (m_pCurrentFontFamily)
	{
		[m_pCurrentFontFamily release];
		m_pCurrentFontFamily = 0;
	}
	DELETEP(m_ToolChest);
	DELETEP(m_Listener);

	[super dealloc];
}

- (void)windowDidLoad
{
	UT_DEBUGMSG(("XAP_CocoaToolPalette -windowDidLoad\n"));

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	if (m_PaletteView)
	{
		// TODO: localise palette titles based on toolbar names

		struct XAP_CocoaPaletteRef palette;

		[oTitle_Standard setTag:1];
		palette.Name  = @"Standard";
		palette.Title = oTitle_Standard;
		palette.Box   = oBox_Standard;
		[m_PaletteView addPalette:&palette];

		[oTitle_Format setTag:1];
		palette.Name  = @"Format";
		palette.Title = oTitle_Format;
		palette.Box   = oBox_Format;
		[m_PaletteView addPalette:&palette];

		[oTitle_Table setTag:0];
		palette.Name  = @"Table";
		palette.Title = oTitle_Table;
		palette.Box   = oBox_Table;
		[m_PaletteView addPalette:&palette];

		[oTitle_Extra setTag:0];
		palette.Name  = @"Extra";
		palette.Title = oTitle_Extra;
		palette.Box   = oBox_Extra;
		[m_PaletteView addPalette:&palette];

		[oTitle_Properties setTag:0];
		palette.Name  = @"Properties";
		palette.Title = oTitle_Properties;
		palette.Box   = oBox_Properties;
		[m_PaletteView addPalette:&palette];

		NSRect content = [m_PaletteView frame];
		NSRect frame = [NSWindow frameRectForContentRect:content styleMask:(NSTitledWindowMask|NSClosableWindowMask|NSUtilityWindowMask)];

		NSRect visible_frame = [[oPanel screen] visibleFrame];
		int x = static_cast<int>(visible_frame.size.width ) + static_cast<int>(visible_frame.origin.x);
		int y = static_cast<int>(visible_frame.size.height) + static_cast<int>(visible_frame.origin.y);

		frame.origin.x = static_cast<float>(x - static_cast<int>(frame.size.width ));
		frame.origin.y = static_cast<float>(y - static_cast<int>(frame.size.height));

		[oPanel setFrame:frame display:NO];
		[oPanel setContentView:m_PaletteView];

		[m_PaletteView setNeedsDisplay:YES];

		[oPanel setFloatingPanel:YES];
		[oPanel setBecomesKeyOnlyIfNeeded:YES];
		[oPanel setReleasedWhenClosed:YES];
		[oPanel setDelegate:self];

		[self setWindow:oPanel];

		if (NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:@"Tool Palette" action:@selector(toggleVisibility:) keyEquivalent:@"T"])
		{
			[item setTarget:self];
			[pController appendPanelItem:item];
			[item release];
		}
	}

#define defn(T,X,Y)			m_ToolChest[ToolID_##T].button = T; \
	m_ToolChest[ToolID_##T].tlbrid = X;							\
	m_ToolChest[ToolID_##T].ttipid = Y;
#include "xap_CocoaTools.h"
#undef defn

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	const char * tooltip = 0;

	for (int i = 0; i < XAP_COCOA_TOOL_ID__COUNT_; i++)
	{
		[m_ToolChest[i].button setTag:i];
		[m_ToolChest[i].button setEnabled:NO];
		[m_ToolChest[i].button setBordered:NO];

		if (m_ToolChest[i].ttipid != AP_STRING_ID__FIRST__) {
			tooltip = pSS->getValue(m_ToolChest[i].ttipid);
			if (tooltip)
			{
				[m_ToolChest[i].button setToolTip:[NSString stringWithUTF8String:tooltip]];
			}
		}
		
		if ([m_ToolChest[i].button isKindOfClass:[XAP_CocoaToolbarButton class]])
		{
			NSButtonCell * cell = (NSButtonCell *) [m_ToolChest[i].button cell];

			[cell setShowsStateBy:NSNoCellMask];
			[cell setHighlightsBy:NSNoCellMask];
		}
	}

	tooltip = pSS->getValue(AP_STRING_ID_TOOLBAR_TOOLTIP_FMT_STYLE);
	if (tooltip)
	{
		[oDocumentStyle setToolTip:[NSString stringWithUTF8String:tooltip]];
	}
	tooltip = pSS->getValue(AP_STRING_ID_TOOLBAR_TOOLTIP_FMT_FONT);
	if (tooltip)
	{
		[oFontName setToolTip:[NSString stringWithUTF8String:tooltip]];
	}
	tooltip = pSS->getValue(AP_STRING_ID_TOOLBAR_TOOLTIP_FMT_SIZE);
	if (tooltip)
	{
		[oFontSize setToolTip:[NSString stringWithUTF8String:tooltip]];
	}
	tooltip = pSS->getValue(AP_STRING_ID_TOOLBAR_TOOLTIP_ZOOM);
	if (tooltip)
	{
		[oZoom setToolTip:[NSString stringWithUTF8String:tooltip]];
	}

	[self rebuildFontFamilyPopUp];

	[[oFontName       menu] setAutoenablesItems:NO];
	[[oFontMemberName menu] setAutoenablesItems:NO];

	[[oDocumentStyle menu] setAutoenablesItems:NO];

	m_Properties_DataSource = [[XAP_PaletteProperties_DataSource alloc] initWithOutlineView:oProperties];
	if (m_Properties_DataSource)
	{
		[oProperties setIndentationPerLevel:10.0f];
		[oProperties setDataSource:m_Properties_DataSource];
		[oProperties setDelegate:m_Properties_DataSource];
	}

	[oPreviewPanel windowDidLoad];

	[self setCurrentView:[pController currentView] inFrame:[pController currentFrame]];
	[self sync];
}

- (void)close
{
	UT_DEBUGMSG(("XAP_CocoaToolPalette -close\n"));
	s_instance = nil;
	[super close];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
	UT_return_if_fail (pPrefs);

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_if_fail (pPrefsScheme);
	
	pPrefsScheme->setValueBool(XAP_PREF_KEY_ToolPaletteVisible, false);


	UT_DEBUGMSG(("XAP_CocoaToolPalette -windowWillClose\n"));
}

- (void)toggleVisibility:(id)sender
{
	UT_UNUSED(sender);
	if ([oPanel isVisible]) {
		[oPanel orderOut:self];
	}
	else {
		[oPanel orderFront:self];
	}
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem
{
	[menuItem setState:([oPanel isVisible] ? NSOnState : NSOffState)];

	return YES;
}

+ (void)setPreviewText:(id)previewText
{
	if (s_instance) {
		if ([previewText isKindOfClass:[NSString class]]) {
			NSString * str = (NSString *) previewText;
			[s_instance setPreviewString:str];
		}
		if ([previewText isKindOfClass:[NSAttributedString class]]) {
			NSAttributedString * str = (NSAttributedString *) previewText;
			[s_instance setPreviewString:[str string]];
		}
	}
}

- (void)setPreviewString:(NSString *)previewString
{
	[oPreview setStringValue:previewString];

	[oPreviewPanel setPreviewString:previewString];
}

- (NSWindow *)previewPanel
{
	return [oPreviewPanel window];
}

- (void)setColor:(XAP_Toolbar_Id)tlbrid
{
	if (!m_pViewCurrent) {
		return;
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(tlbrid);
	UT_ASSERT(pAction);
	if (!pAction) {
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName) {
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM) {
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -setColor: have edit method\n"));

	XAP_CGFloat red;
	XAP_CGFloat green;
	XAP_CGFloat blue;
	XAP_CGFloat alpha;

	NSColor * color = [[NSColorPanel sharedColorPanel] color];

	[color getRed:&red green:&green blue:&blue alpha:&alpha]; // TODO: is color necessarily RGBA? if not, could be a problem...

	int r = static_cast<int>(lrintf(red   * 255));	r = (r < 0) ? 0 : r;	r = (r > 255) ? 255 : r;
	int g = static_cast<int>(lrintf(green * 255));	g = (g < 0) ? 0 : g;	g = (g > 255) ? 255 : g;
	int b = static_cast<int>(lrintf(blue  * 255));	b = (b < 0) ? 0 : b;	b = (b > 255) ? 255 : b;

	UT_HashColor hash;

	const char * color_string = hash.setColor(static_cast<unsigned char>(r),
											  static_cast<unsigned char>(g),
											  static_cast<unsigned char>(b));
	if (color_string)
	{
		UT_UCS4String color_data(color_string);

		const UT_UCS4Char * pData = color_data.ucs4_str();
		UT_uint32 dataLength = static_cast<UT_uint32>(color_data.length());

		EV_EditMethodCallData emcd(pData,dataLength);
		pEM->Fn(m_pViewCurrent, &emcd);
	}
}

- (IBAction)aColor_FG:(id)sender
{
	UT_UNUSED(sender);
	[self setColor:AP_TOOLBAR_ID_COLOR_FORE];
}

- (IBAction)aColor_BG:(id)sender
{
	UT_UNUSED(sender);
	[self setColor:AP_TOOLBAR_ID_COLOR_BACK];
}

- (IBAction)aSwitch_FG:(id)sender
{
	UT_UNUSED(sender);
	// TODO
}

- (IBAction)aSwitch_BG:(id)sender
{
	UT_UNUSED(sender);
	// TODO
}

- (IBAction)aTitle_click:(id)sender
{
	NSButton * title = (NSButton *) sender;

	if ([title tag] == 0) {
		[title setTag:1];
	}
	else {
		[title setTag:0];
	}

	[m_PaletteView sync];
	[m_PaletteView setNeedsDisplay:YES];
}

- (IBAction)aDocumentStyle:(id)sender
{
	UT_UNUSED(sender);
	if (!m_pViewCurrent) {
		return;
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_STYLE);
	UT_ASSERT(pAction);
	if (!pAction) {
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName) {
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM) {
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -aDocumentStyle: have edit method\n"));

	UT_UCS4String selection([[oDocumentStyle titleOfSelectedItem] UTF8String]);

	const UT_UCS4Char * pData = selection.ucs4_str();
	UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

	EV_EditMethodCallData emcd(pData,dataLength);
	pEM->Fn(m_pViewCurrent, &emcd);
}

- (IBAction)aFontName:(id)sender
{
	UT_UNUSED(sender);
	if (!m_pViewCurrent) {
		return;
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_FONT);
	UT_ASSERT(pAction);
	if (!pAction) {
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName) {
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM) {
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -aFontName: have edit method\n"));

	UT_UCS4String selection([[oFontName titleOfSelectedItem] UTF8String]);

	const UT_UCS4Char * pData = selection.ucs4_str();
	UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

	EV_EditMethodCallData emcd(pData,dataLength);
	pEM->Fn(m_pViewCurrent, &emcd);
}

- (IBAction)aFontMemberName:(id)sender
{
	UT_UNUSED(sender);
	if (!m_pViewCurrent) {
		return;
	}

	if (!m_pCurrentFontFamily)
	{
		return;
	}

//	int index = [[oFontMemberName selectedItem] tag];

	NSString * fontFamilyName = m_pCurrentFontFamily;

	BOOL bBold   = ([oTB_text_bold   state] == NSOnState) ? YES : NO; // current state
	BOOL bItalic = ([oTB_text_italic state] == NSOnState) ? YES : NO;

	BOOL bSetBold   = NO;
	BOOL bSetItalic = NO;

#if 0
	if (index == [m_pCurrentFontFamilyHelper indexBoldItalic])
	{
		bSetBold   = YES;
		bSetItalic = YES;
	}
	else if (index == [m_pCurrentFontFamilyHelper indexBold])
	{
		bSetBold   = YES;
	}
	else if (index == [m_pCurrentFontFamilyHelper indexItalic])
	{
		bSetItalic = YES;
	}
	else if (index == [m_pCurrentFontFamilyHelper indexRegular])
	{
		// 
	}
	else // need to set an explicit font name rather than a font family name...
	{
		NSArray * fontNames = [m_pCurrentFontFamilyHelper fontNames];

		fontFamilyName = [fontNames objectAtIndex:index];
	}
#endif
	/* fontFamilyName gets corrupted for some probably obvious reason, so copy it here and now...
	 */
	UT_UCS4String selection([fontFamilyName UTF8String]);

	if (bBold != bSetBold)
	{
		[oTB_text_bold setState:(bSetBold ? NSOnState : NSOffState)];
		[self aTB_click:oTB_text_bold];
	}

	if (bItalic != bSetItalic)
	{
		[oTB_text_italic setState:(bSetItalic ? NSOnState : NSOffState)];
		[self aTB_click:oTB_text_italic];
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_FONT);
	UT_ASSERT(pAction);
	if (!pAction) {
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName) {
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM) {
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -aFontMemberName: have edit method\n"));

	const UT_UCS4Char * pData = selection.ucs4_str();
	UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

	EV_EditMethodCallData emcd(pData,dataLength);
	pEM->Fn(m_pViewCurrent, &emcd);
}

- (IBAction)aFontSize:(id)sender
{
	UT_UNUSED(sender);
	if (!m_pViewPrevious)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_SIZE);
	UT_ASSERT(pAction);
	if (!pAction)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -aFontSize: have edit method\n"));

	int size = [oFontSize intValue];

	if (size < 1) {
		size = 1;   // TODO: ??
	}
	if (size > 100) {
		size = 100; // TODO: ??
	}

	char buf[8];
	sprintf(buf, "%d", size);

	UT_UCS4String selection(buf);

	const UT_UCS4Char * pData = selection.ucs4_str();
	UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

	EV_EditMethodCallData emcd(pData,dataLength);
	pEM->Fn(m_pViewPrevious, &emcd);

	if (m_pFramePrevious) {
		m_pFramePrevious->raise();
	}
}

- (IBAction)aTB_click:(id)sender
{
	NSButton * button = (NSButton *) sender;

	int tag = [button tag];

	if ((tag < 0) || (tag >= XAP_COCOA_TOOL_ID__COUNT_))
	{
		UT_DEBUGMSG(("XAP_CocoaToolPalette -aTB_click: invalid tag\n"));
		return;
	}
	// button == m_ToolChest[tag].button;

	// TODO: other special buttons

	XAP_Toolbar_Id tlbrid = m_ToolChest[tag].tlbrid;

	if (!m_pViewCurrent)
	{
		if ((tlbrid == AP_TOOLBAR_ID_FILE_NEW ) ||
			(tlbrid == AP_TOOLBAR_ID_FILE_OPEN) ||
			(tlbrid == AP_TOOLBAR_ID_HELP))
		{
			// Only these are okay
		}
		else
		{
			return;
		}
	}

	/* Toolbar fore- & background color buttons don't actually set the color, they just ensure that the
	 * color-chooser is visible & correctly configured:
	 */
	if ((tlbrid == AP_TOOLBAR_ID_COLOR_FORE) || (tlbrid == AP_TOOLBAR_ID_COLOR_BACK))
	{
		NSColorPanel * colorPanel = [NSColorPanel sharedColorPanel];

		// ?? [NSColorPanel setPickerMask:(NSColorPanelRGBModeMask|NSColorPanelWheelModeMask|NSColorPanelGrayModeMask)];

		[colorPanel setAction:0];
		[colorPanel setTarget:0];

		if (tlbrid == AP_TOOLBAR_ID_COLOR_FORE)
		{
			// [colorPanel setTitle:@"Foreground Color"]; // TODO: Localize
			[colorPanel setColor:[oColor_FG color]];
			[colorPanel setAction:@selector(aColor_FG:)];
		}
		else
		{
			// [colorPanel setTitle:@"Background Color"]; // TODO: Localize
			[colorPanel setColor:[oColor_BG color]];
			[colorPanel setAction:@selector(aColor_BG:)];
		}
		[colorPanel orderFront:self];
		[colorPanel setTarget:self];

		return;
	}

	if (tlbrid)
	{
		const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(tlbrid);
		UT_ASSERT(pAction);
		if (!pAction) {
			return;
		}

		const char * szMethodName = pAction->getMethodName();
		if (!szMethodName) {
			return;
		}

		EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
		if (!pEM) {
			return;
		}

		UT_DEBUGMSG(("XAP_CocoaToolPalette -aTB_click: have edit method\n"));

		EV_EditMethodType t = pEM->getType();

		if (t & EV_EMT_REQUIREDATA) // This method requires character data
		{
			UT_DEBUGMSG(("XAP_CocoaToolPalette -aTB_click: method requires data\n"));
		}
		else
		{
			const UT_UCS4Char * pData = NULL;
			UT_uint32 dataLength = 0;

			EV_EditMethodCallData emcd(pData,dataLength);
			pEM->Fn(m_pViewCurrent, &emcd);
		}
	}
	else
	{
		switch (tag) {
		case ToolID_oTB_LineAll:
		case ToolID_oTB_LineBottom:
		case ToolID_oTB_LineLeft:
		case ToolID_oTB_LineNone:
		case ToolID_oTB_LineRight:
		case ToolID_oTB_LineTop:
			// TODO: this is something else. use menu methods, perhaps
			break;

		case ToolID_oTB_search:
		{
			bool okay = false;

			const EV_Menu_Action * pAction = m_pMenuActionSet->getAction(AP_MENU_ID_EDIT_FIND);
			if (m_pViewCurrent && pAction) {
				if (const char * szMethodName = pAction->getMethodName())
				{
					EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
					if (pEM)
					{
						EV_EditMethodCallData data;
						okay = pEM->Fn(m_pViewCurrent, &data);
					}
				}
			}
			if (!okay)
			{
				UT_DEBUGMSG(("XAP_CocoaToolPalette -aTB_click: ToolID_oTB_search: failed!\n"));
			}
			break;
		}
		case ToolID_oTB_search_replace:
		{
			bool okay = false;

			const EV_Menu_Action * pAction = m_pMenuActionSet->getAction(AP_MENU_ID_EDIT_REPLACE);
			if (m_pViewCurrent && pAction) {
				if (const char * szMethodName = pAction->getMethodName())
				{
					EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
					if (pEM)
					{
						EV_EditMethodCallData data;
						okay = pEM->Fn(m_pViewCurrent, &data);
					}
				}
			}
			if (!okay)
			{
				UT_DEBUGMSG(("XAP_CocoaToolPalette -aTB_click: ToolID_oTB_search_replace: failed!\n"));
			}
			break;
		}

		case ToolID_oTB_SplitAbove:
		case ToolID_oTB_SplitBelow:
		case ToolID_oTB_SplitHoriMid:
		case ToolID_oTB_SplitLeft:
		case ToolID_oTB_SplitRight:
		case ToolID_oTB_SplitVertMid:
			// TODO: this is something else. use menu methods, perhaps
			break;

		default:
			UT_DEBUGMSG (("XAP_CocoaToolPalette -aTB_click: Unimplemented toolbar button!\n"));
			break;
		}
	}
}

- (IBAction)aZoom:(id)sender
{
	UT_UNUSED(sender);
	if (!m_pViewPrevious)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_ZOOM);
	UT_ASSERT(pAction);
	if (!pAction)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	EV_EditMethod * pEM = m_pEditMethodContainer->findEditMethodByName(szMethodName);
	if (!pEM)
	{
		if (m_pFramePrevious) {
			m_pFramePrevious->raise();
		}
		return;
	}

	UT_DEBUGMSG(("XAP_CocoaToolPalette -aZoom: have edit method\n"));

	NSString * value = [oZoom stringValue];

	if ([value isEqualToString:@"Whole Page"] || [value isEqualToString:@"Page Width"])
	{
		UT_UCS4String selection([value UTF8String]);

		const UT_UCS4Char * pData = selection.ucs4_str();
		UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

		EV_EditMethodCallData emcd(pData,dataLength);
		pEM->Fn(m_pViewPrevious, &emcd);
	}
	else
	{
		int size = [oZoom intValue];

		if (size < 1) {
			size = 1;    // TODO: ??
		}
		if (size > 1000) {
			size = 1000; // TODO: ??
		}

		char buf[8];
		sprintf(buf, "%d", size);

		UT_UCS4String selection(buf);

		const UT_UCS4Char * pData = selection.ucs4_str();
		UT_uint32 dataLength = static_cast<UT_uint32>(selection.length());

		EV_EditMethodCallData emcd(pData,dataLength);
		pEM->Fn(m_pViewPrevious, &emcd);
	}
	if (m_pFramePrevious) {
		m_pFramePrevious->raise();
	}
}

- (void)sync
{
	/* toolbar buttons
	 */
	for (UT_uint32 i = 0; i < XAP_COCOA_TOOL_ID__COUNT_; i++)
	{
		if (m_ToolChest[i].tlbrid)
		{
			if (!m_pViewCurrent)
			{
				if ((m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_FILE_NEW ) ||
					(m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_FILE_OPEN) ||
					(m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_HELP))
				{
					[m_ToolChest[i].button setEnabled:YES];
				}
				else
				{
					[m_ToolChest[i].button setEnabled:NO];
				}
				continue;
			}

			if ((m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_MERGEABOVE) ||
				(m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_MERGEBELOW) ||
				(m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_MERGELEFT ) ||
				(m_ToolChest[i].tlbrid == AP_TOOLBAR_ID_MERGERIGHT))
			{
				/* WARNING: MERGELEFT/RIGHT/ABOVE/BELOW need to be implemented properly in toolbar actions!
				 */
				[m_ToolChest[i].button setEnabled:NO];
				continue;
			}

			const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(m_ToolChest[i].tlbrid);
			UT_ASSERT(pAction);
			if (!pAction) {
				continue;
			}

			const char * szState = 0;
			EV_Toolbar_ItemState tis = pAction->getToolbarItemState(m_pViewCurrent, &szState);

			switch (pAction->getItemType()) {
			
			case EV_TBIT_PushButton:
			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				[m_ToolChest[i].button setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];
			}
			break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
			{
				[m_ToolChest[i].button setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];
				[m_ToolChest[i].button setState:(EV_TIS_ShouldBeToggled(tis) ? NSOnState : NSOffState)];
			}
			break;

			case EV_TBIT_StaticLabel:
			case EV_TBIT_Spacer:
			case EV_TBIT_BOGUS:
			case EV_TBIT_EditText:
			case EV_TBIT_DropDown:
			case EV_TBIT_ComboBox: // handled elsewhere
				break;

			default:
				UT_DEBUGMSG (("XAP_CocoaToolPalette -sync: Unexpected toolbar layout flags!\n"));
				UT_ASSERT(0);
				break;
			}
		}
		else
		{
			switch (i) {// well, okay, a little insane to switch this, but...
			case ToolID_oTB_LineAll:
			case ToolID_oTB_LineBottom:
			case ToolID_oTB_LineLeft:
			case ToolID_oTB_LineNone:
			case ToolID_oTB_LineRight:
			case ToolID_oTB_LineTop:
				// TODO: this is something else. use menu methods, perhaps
				[m_ToolChest[i].button setEnabled:NO];
				break;

			case ToolID_oTB_search:
			case ToolID_oTB_search_replace:
				[m_ToolChest[i].button setEnabled:(m_pViewCurrent ? YES : NO)];
				break;

			case ToolID_oTB_SplitAbove:
			case ToolID_oTB_SplitBelow:
			case ToolID_oTB_SplitHoriMid:
			case ToolID_oTB_SplitLeft:
			case ToolID_oTB_SplitRight:
			case ToolID_oTB_SplitVertMid:
				// TODO: this is something else. use menu methods, perhaps
				[m_ToolChest[i].button setEnabled:NO];
				break;

			default:
				UT_DEBUGMSG (("XAP_CocoaToolPalette -sync: Unimplemented toolbar button!\n"));
				[m_ToolChest[i].button setEnabled:NO];
				break;
			}
		}
	}

	/* Document styles
	 */
	[oDocumentStyle removeAllItems];
	[oDocumentStyle setEnabled:NO];

	if (m_pFrameCurrent) {
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrameCurrent->getCurrentDoc())) {
			const char * szName = 0;
			const PD_Style * pStyle = 0;

			NSMutableArray * styles = [NSMutableArray arrayWithCapacity:32];

			for (UT_uint32 k = 0; (pDoc->enumStyles(k, &szName, &pStyle)); k++) {
				if (szName) {
#if 1
					[styles addObject:[NSString stringWithUTF8String:szName]];
#else
					// TODO: Make style names reflect properties such as: font, size, alignment ??
					NSString * name = [NSString stringWithUTF8String:szName];

					NSDictionary * attr = TODO

						[oDocumentStyle addItemWithTitle:[NSAttributedString initWithString:name attributes:attr]];
#endif
				}
			}
			[styles sortUsingSelector:@selector(compare:)];

			[oDocumentStyle addItemsWithTitles:styles];

			const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_STYLE);
			UT_ASSERT(pAction);
			if (pAction) {
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(m_pViewCurrent, &szState);

				[oDocumentStyle setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];

				UT_ASSERT(szState);
				if (szState) {
					[oDocumentStyle selectItemWithTitle:[NSString stringWithUTF8String:szState]];
				}
			}
		}
	}
	/* Font name
	 */
	[oFontName setEnabled:NO];

	if (m_pViewCurrent)
	{
		const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_FONT);
		UT_ASSERT(pAction);
		if (pAction)
		{
			const char * szState = 0;
			EV_Toolbar_ItemState tis = pAction->getToolbarItemState(m_pViewCurrent, &szState);

			[oFontName setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];

			// UT_ASSERT(szState); // This assert is triggered if the selection contains multiple fonts?
			if (szState)
			{
				[self syncPopUpsForFont:[NSString stringWithUTF8String:szState]];
			}
		}
	}

	/* Font size
	 */
	[oFontSize setEnabled:NO];

	if (m_pViewCurrent)
	{
		const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_FMT_SIZE);
		UT_ASSERT(pAction);
		if (pAction)
		{
			const char * szState = 0;
			EV_Toolbar_ItemState tis = pAction->getToolbarItemState(m_pViewCurrent, &szState);

			[oFontSize setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];

			if (szState)
			{
				NSString * selection = [NSString stringWithUTF8String:szState];

				int index = [oFontSize indexOfItemWithObjectValue:selection];

				if (index == NSNotFound)
				{
					index = [oFontSize indexOfSelectedItem];

					if (index >= 0)
					{
						[oFontSize deselectItemAtIndex:index];
					}
				}
				else
				{
					[oFontSize selectItemAtIndex:index];
				}
				[oFontSize setStringValue:selection];
			}
			else // mixed selection
			{
				[oFontSize setStringValue:@""];
			}
		}
	}

	/* Zoom
	 */
	[oZoom setEnabled:NO];

	if (m_pViewCurrent)
	{
		const EV_Toolbar_Action * pAction = m_pToolbarActionSet->getAction(AP_TOOLBAR_ID_ZOOM);
		UT_ASSERT(pAction);
		if (pAction)
		{
			const char * szState = 0;
			EV_Toolbar_ItemState tis = pAction->getToolbarItemState(m_pViewCurrent, &szState);

			[oZoom setEnabled:(EV_TIS_ShouldBeGray(tis) ? NO : YES)];

			UT_ASSERT(szState);
			if (szState)
			{
				NSString * selection = [NSString stringWithUTF8String:szState];

				int index = [oZoom indexOfItemWithObjectValue:selection];

				if (index == NSNotFound)
				{
					index = [oZoom indexOfSelectedItem];

					if (index >= 0)
					{
						[oZoom deselectItemAtIndex:index];
					}
				}
				else
				{
					[oZoom selectItemAtIndex:index];
				}
				[oZoom setStringValue:selection];
			}
		}
	}

	/* Color buttons
	 */
	[oSwitch_FG setEnabled:NO]; // TODO
	[oSwitch_BG setEnabled:NO]; // TODO

	if (m_pViewCurrent)
	{
		FV_View * pFView = static_cast<FV_View *>(m_pViewCurrent);

		bool bMixedSelection;
		bool bExplicitlyDefined;

		UT_UTF8String szColorValue;

		if (pFView->queryCharFormat("color", szColorValue, bExplicitlyDefined, bMixedSelection))
		{
			[oSwitch_FG setState:(bExplicitlyDefined ? NSOnState : NSOffState)];

			UT_HashColor hash;

			bool bValid = (hash.setColor(szColorValue.utf8_str()) != 0);
			if (!bValid) {
				bValid = (hash.setHashIfValid(szColorValue.utf8_str()) != 0);
			}
			
			if (bValid)
			{
				UT_RGBColor rgb = hash.rgb();

				float r = static_cast<float>(rgb.m_red) / 255;
				float g = static_cast<float>(rgb.m_grn) / 255;
				float b = static_cast<float>(rgb.m_blu) / 255;
				float a = 1;

				[oColor_FG setColor:[NSColor colorWithDeviceRed:r green:g blue:b alpha:a]];
			}
			else
			{
				UT_DEBUGMSG(("XAP_CocoaToolPalette -sync: fg=\"%s\"?\n", szColorValue.utf8_str()));
			}
		}
		if (pFView->queryCharFormat("bgcolor", szColorValue, bExplicitlyDefined, bMixedSelection))
		{
			[oSwitch_BG setState:(bExplicitlyDefined ? NSOnState : NSOffState)];

			if (strcmp (szColorValue.utf8_str(), "transparent") == 0)
			{
				float r = 1;
				float g = 1;
				float b = 1;
				float a = 0;

				[oColor_BG setColor:[NSColor colorWithDeviceRed:r green:g blue:b alpha:a]];
			}
			else
			{
				UT_HashColor hash;

				bool bValid = (hash.setColor(szColorValue.utf8_str()) != 0);
				if (!bValid) {
					bValid = (hash.setHashIfValid(szColorValue.utf8_str()) != 0);
				}
				if (bValid)
				{
					UT_RGBColor rgb = hash.rgb();

					float r = static_cast<float>(rgb.m_red) / 255;
					float g = static_cast<float>(rgb.m_grn) / 255;
					float b = static_cast<float>(rgb.m_blu) / 255;
					float a = 1;

					[oColor_BG setColor:[NSColor colorWithDeviceRed:r green:g blue:b alpha:a]];
				}
				else
				{
					UT_DEBUGMSG(("XAP_CocoaToolPalette -sync: bg=\"%s\"?\n", szColorValue.utf8_str()));
				}
			}
		}
	}

	/* Properties OutlineView
	 */
	[m_Properties_DataSource syncWithView:(static_cast<FV_View *>(m_pViewCurrent))];
}

- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame
{
	if (!self) return;

	/* Need to be careful. A frame/view can switch out because keyboard
	 * focus has changed (e.g., because a combo-box in the panel has stolen
	 * focus) or it may have been killed...
	 */
	m_pViewPrevious  = m_pViewCurrent;
	m_pFramePrevious = m_pFrameCurrent;

	bool bSync = false;

	if (frame && view)
	{
		m_pViewCurrent  = view;
		m_pFrameCurrent = frame;

		bSync = true;
	}
	else if (!frame && !view)
	{
		m_pViewCurrent  = 0;
		m_pFrameCurrent = 0;

		// should we sync in this case?
	}
	else
	{
		UT_ASSERT(view && frame);

		m_pViewCurrent  = 0;
		m_pFrameCurrent = 0;

		bSync = true;
	}

	m_Listener->setCurrentView(m_pViewCurrent);

	if (bSync) {
		[self sync];
	}
}

- (void)rebuildFontFamilyPopUp
{
//	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	[oFontName removeAllItems];

	unsigned count = [m_pFontFamilies count];

	BOOL bAttributedTitles = YES;

	for (unsigned i = 0; i < count; i++)
	{
		NSString * Family = [m_pFontFamilies objectAtIndex:i];

		[oFontName addItemWithTitle:Family];

		NSMenuItem * item = [oFontName lastItem];

		[item setTag:((int) i)];

		if (i == 0)
		{
			bAttributedTitles = [item respondsToSelector:@selector(setAttributedTitle:)];
		}
//		if (bAttributedTitles)
//		{
//			XAP_CocoaFontFamilyHelper * helper = [pController helperForFontFamily:Family];
//
//			[item setAttributedTitle:[helper attributedFontFamilyName]];
//		}
	}
}

- (void)syncPopUpsForFont:(NSString *)requestedFontFamilyName
{
	/* requestedFontFamilyName is one of:
	 * 
	 * (a) a recognized font family name
	 * (b) really a font name
	 * (c) completely unknown, so we pretend it's a new font family name
	 */

	NSString * fontFamilyName = 0;

//	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

//	XAP_CocoaFontReference * fontRef = 0;

//	XAP_CocoaFontFamilyHelper * helper = [pController helperForFontFamily:requestedFontFamilyName];

//	if (helper)
//	{
//		// (a) a recognized font family name
//
//		fontFamilyName = requestedFontFamilyName;
//	}
//	else if ((fontRef = [pController helperReferenceForFont:requestedFontFamilyName]))
//	{
//		// (b) really a font name
//
//		fontFamilyName = [fontRef fontFamily];
//
//		helper = [fontRef fontFamilyHelper];
//	}
//	else
//	{
//		// (c) completely unknown, so we pretend it's a new font family name
//
		fontFamilyName = requestedFontFamilyName;
//
//		helper = [pController helperForUnknownFontFamily:requestedFontFamilyName];
//
		/* In this case we need to modify the list of font families
		 */

		[m_pFontFamilies addObject:fontFamilyName];
		[m_pFontFamilies sortUsingSelector:@selector(compare:)];

		[self rebuildFontFamilyPopUp];
//	}

	BOOL bRebuildMemberPopUp = YES;

	if (m_pCurrentFontFamily)
	{
		if ([m_pCurrentFontFamily isEqualToString:fontFamilyName])
		{
			bRebuildMemberPopUp = NO;
		}
		[m_pCurrentFontFamily release];
	}
	m_pCurrentFontFamily = fontFamilyName;
	[m_pCurrentFontFamily retain];

#if 0
	if (m_pCurrentFontFamilyHelper)
	{
		[m_pCurrentFontFamilyHelper release];
	}
	m_pCurrentFontFamilyHelper = helper;
	[m_pCurrentFontFamilyHelper retain];

	[oFontName selectItemWithTitle:m_pCurrentFontFamily];

	unsigned count = [helper count];

	NSArray * fontMembers = [helper fontMembers];

	if (bRebuildMemberPopUp)
	{
		[oFontMemberName removeAllItems];

		NSArray * attributedFontMembers = [helper attributedFontMembers];

		BOOL bAttributedTitles = YES;

		for (unsigned i = 0; i < count; i++)
		{
			NSString * fontMember = [fontMembers objectAtIndex:i];

			[oFontMemberName addItemWithTitle:fontMember];

			NSMenuItem * item = [oFontMemberName lastItem];

			[item setTag:((int) i)];

			if (i == 0)
			{
				bAttributedTitles = [item respondsToSelector:@selector(setAttributedTitle:)];
			}
			if (bAttributedTitles)
			{
				NSAttributedString * attr_title = [attributedFontMembers objectAtIndex:i];

				[item setAttributedTitle:attr_title];
			}
		}
		[oFontMemberName setEnabled:((count > 1) ? YES : NO)];
	}
	if (count == 1)
	{
		return;
	}
	if (fontRef)
	{
		[oFontMemberName selectItemAtIndex:((int) [fontRef index])];
		return;
	}
	/* We have a font family name but we need to choose a particular member of the font...
	 */
	BOOL bBold   = ([oTB_text_bold   state] == NSOnState) ? YES : NO;
	BOOL bItalic = ([oTB_text_italic state] == NSOnState) ? YES : NO;

	if (bBold && bItalic)
	{
		int index = [helper indexBoldItalic];
		if (index >= 0)
		{
			[oFontMemberName selectItemAtIndex:index];
			return;
		}
	}
	else if (bBold)
	{
		int index = [helper indexBold];
		if (index >= 0)
		{
			[oFontMemberName selectItemAtIndex:index];
			return;
		}
	}
	else if (bItalic)
	{
		int index = [helper indexItalic];
		if (index >= 0)
		{
			[oFontMemberName selectItemAtIndex:index];
			return;
		}
	}
	else
	{
		int index = [helper indexRegular];

		[oFontMemberName selectItemAtIndex:((index >= 0) ? index : 0)];
	}
#endif
}

@end

XAP_CocoaToolPaletteListener::XAP_CocoaToolPaletteListener(XAP_CocoaToolPalette * pPalette) :
	m_pPalette(pPalette),
	m_pView(0)
{
	// 
}

XAP_CocoaToolPaletteListener::~XAP_CocoaToolPaletteListener()
{
	setCurrentView(0);
}

void XAP_CocoaToolPaletteListener::setCurrentView(AV_View * view)
{
	if (m_pView)
	{
		if (!m_pView->removeListener(m_lid))
		{
			UT_DEBUGMSG(("XAP_CocoaToolPaletteListener::setCurrentView: failed to remove listener!\n"));
		}
		m_pView = 0;
	}
	if (view)
	{
		if (view->addListener(this, &m_lid))
		{
			m_pView = view;
		}
		else
		{
			UT_DEBUGMSG(("XAP_CocoaToolPaletteListener::setCurrentView: failed to add listener!\n"));
		}
	}
}

bool XAP_CocoaToolPaletteListener::notify(AV_View * pView, const AV_ChangeMask /*mask*/)
{
	if (pView == m_pView)
	{
		[m_pPalette sync];
	}
	return true; // why is this a bool function?
}

AV_ListenerType XAP_CocoaToolPaletteListener::getType()
{
	return AV_LISTENER_TOOLBAR;
}
