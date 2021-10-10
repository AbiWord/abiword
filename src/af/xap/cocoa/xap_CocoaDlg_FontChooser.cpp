/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003, 2009 Hubert Figuiere
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_FontChooser.h"
#include "xap_CocoaApp.h"

#include "xap_EncodingManager.h"
#include "gr_CocoaGraphics.h"

// your typographer's standard nonsense latin font phrase
#define PREVIEW_ENTRY_DEFAULT_STRING	"Lorem ipsum dolor sit amet, consectetaur adipisicing..."



/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FontChooser * p = new XAP_CocoaDialog_FontChooser(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FontChooser::XAP_CocoaDialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_FontChooser(pDlgFactory,dlgid),
	m_bSuperScriptInitialValue(false),
	m_bSubScriptInitialValue(false),
	m_currentFamily(NULL),
	m_dlg(nil)
{
	// 
}

XAP_CocoaDialog_FontChooser::~XAP_CocoaDialog_FontChooser(void)
{
	FREEP(m_currentFamily);
	DELETEP(m_pGraphics);
}


/*****************************************************************/

void XAP_CocoaDialog_FontChooser::strikeoutChanged(bool value)
{
	m_bStrikeout = value;
	m_bChangedStrikeOut = !m_bChangedStrikeOut;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::underlineChanged(bool value)
{
	m_bUnderline = value;
	m_bChangedUnderline = !m_bChangedUnderline;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::overlineChanged(bool value)
{
	m_bOverline = value;
	m_bChangedOverline = !m_bChangedOverline;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::superscriptChanged(bool value)
{
	m_bSuperScript = value;
	m_bSubScript   = false;

	[m_dlg setSubscript:m_bSubScript];

	m_bChangedSuperScript = (m_bSuperScript != m_bSuperScriptInitialValue);
	m_bChangedSubScript   = (m_bSubScript   != m_bSubScriptInitialValue  );

	setSuperScript(m_bSuperScript);
	setSubScript  (m_bSubScript);

	updatePreview();
}

void XAP_CocoaDialog_FontChooser::subscriptChanged(bool value)
{
	m_bSuperScript = false;
	m_bSubScript   = value;

	[m_dlg setSuperscript:m_bSuperScript];

	m_bChangedSuperScript = (m_bSuperScript != m_bSuperScriptInitialValue);
	m_bChangedSubScript   = (m_bSubScript   != m_bSubScriptInitialValue  );

	setSuperScript(m_bSuperScript);
	setSubScript  (m_bSubScript);

	updatePreview();
}

void XAP_CocoaDialog_FontChooser::transparencyChanged(bool value)
{
	bool bTrans = value;
	if(bTrans)
	{
		addOrReplaceVecProp("bgcolor","transparent");
		UT_DEBUGMSG (("Update background color"));
	}
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::fontRowChanged(void)
{
	NSString *fontFamily;
	
	fontFamily = [m_dlg selectedFont];
	FREEP(m_currentFamily);
	m_currentFamily = g_strdup([fontFamily UTF8String]);
	addOrReplaceVecProp("font-family",m_currentFamily);

	updatePreview();
}

void XAP_CocoaDialog_FontChooser::styleRowChanged(int rowNumber)
{
	if (rowNumber == 0)
	{
		addOrReplaceVecProp("font-style","normal");
		addOrReplaceVecProp("font-weight","normal");
	}
	else if (rowNumber == 1)
	{
		addOrReplaceVecProp("font-style","italic");
		addOrReplaceVecProp("font-weight","normal");
	}
	else if (rowNumber == 2)
	{
		addOrReplaceVecProp("font-style","normal");
		addOrReplaceVecProp("font-weight","bold");
	}
	else if (rowNumber == 3)
	{
		addOrReplaceVecProp("font-style","italic");
		addOrReplaceVecProp("font-weight","bold");
	}
	else
	{
		UT_ASSERT(0);
	}
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::sizeRowChanged(void)
{
	NSString* fontSize;
	static char szFontSize[60];
	
	fontSize = [m_dlg selectedSize];
	
	snprintf(szFontSize, sizeof(szFontSize), "%spt",
				   (gchar *)XAP_EncodingManager::fontsizes_mapping.lookupByTarget([fontSize UTF8String]));
	addOrReplaceVecProp("font-size",(gchar *)szFontSize);

	updatePreview();
}

/*!
	Change the color for the property.
	
	\param color the NSColor
	\param attr the CSS attribute
	\param buf the buf to store the color (static in the caller)
 */
void XAP_CocoaDialog_FontChooser::_colorChanged(NSColor * color, const gchar * attr, char * buf)
{
	CGFloat r,g,b,a;
	color = [color colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&r green:&g blue:&b alpha:&a];
	snprintf(buf, 7, "%02x%02x%02x",
			(unsigned int) (r * 255.0),
			(unsigned int) (g * 255.0),
			(unsigned int) (b * 255.0));
	addOrReplaceVecProp(attr,(gchar *)buf);
	updatePreview();
}

/*!
	Called when text color has been changed in the widget
 */
void XAP_CocoaDialog_FontChooser::fgColorChanged(void)
{
	static char buf_color[8];
	_colorChanged([m_dlg textColor], "color", buf_color);
}

/*!
	Called when text background color has been changed in the widget
 */
void XAP_CocoaDialog_FontChooser::bgColorChanged(void)
{
	static char buf_color[8];
	_colorChanged([m_dlg bgColor], "bgcolor", buf_color);
}

void XAP_CocoaDialog_FontChooser::runModal(XAP_Frame * /*pFrame*/)
{
	UT_ASSERT(m_pApp);

	m_dlg = [[XAP_CocoaDialog_FontChooserController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];
	
	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	// freeze updates of the preview
	_createGC([m_dlg preview]);
	event_previewClear();


	// Set the defaults in the list boxes according to dialog data
	[m_dlg selectFont:(char*)getVal("font-family").c_str()];
	[m_dlg selectStyle:(char*)getVal("font-style").c_str() withWeight:(char*)getVal("font-weight").c_str()];
	[m_dlg selectSize:(char*)getVal("font-size").c_str()];

	// Set color in the color selector
	const std::string sColor = getVal("color");
	if (!sColor.empty())
	{
		UT_RGBColor c;
		UT_parseColor(sColor.c_str(), c);

		NSColor *color = GR_CocoaGraphics::_utRGBColorToNSColor(c);
		[m_dlg setTextColor:color];
	}
	else {
		[m_dlg setTextColor:[[NSColor blackColor] colorUsingColorSpaceName:NSDeviceRGBColorSpace]];	
	}
	
	// Set color in the color selector
	const std::string sBGCol = getVal("bgcolor");
	if (!sBGCol.empty() && strcmp(sBGCol.c_str(),"transparent") != 0)
	{
		UT_RGBColor c;
		UT_parseColor(sBGCol.c_str(), c);

		NSColor *color = GR_CocoaGraphics::_utRGBColorToNSColor(c);
		[m_dlg setBgColor:color];
	}
	else
	{
		[m_dlg setBgColor:[[NSColor whiteColor] colorUsingColorSpaceName:NSDeviceRGBColorSpace]];	
	}

	// set the strikeout and underline check buttons
	[m_dlg setStrikeout:m_bStrikeout];
	[m_dlg setUnderline:m_bUnderline];
	[m_dlg setOverline:m_bOverline];

	m_bSuperScriptInitialValue = m_bSuperScript;
	m_bSubScriptInitialValue   = m_bSubScript;

	[m_dlg setSuperscript:m_bSuperScript];
	[m_dlg setSubscript:m_bSubScript];
	
	updatePreview();
	// Run the dialog
	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;

	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 getVal("font-family").c_str(),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 getVal("font-size").c_str(),		((m_bChangedFontSize) ? "(chg)" : ""),
				 getVal("font-weight").c_str(),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 getVal("font-style").c_str(),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 getVal("color").c_str(),				((m_bChangedColor) ? "(chg)" : ""),
				 m_bUnderline,							((m_bChangedUnderline) ? "(chg)" : ""),
				 m_bStrikeout,							((m_bChangedStrikeOut) ? "(chg)" : "")));

}

bool XAP_CocoaDialog_FontChooser::getEntryString(const char ** string)
{
	UT_ASSERT(string);

	// Maybe this will be editable in the future, if one wants to
	// hook up a mini formatter to the entry area.  Probably not.

	*string = PREVIEW_ENTRY_DEFAULT_STRING;

	return true;
}

void XAP_CocoaDialog_FontChooser::updatePreview(void)
{
	// if we don't have anything yet, just ignore this request
	if (!m_pGraphics) {
		return;
	}
	
	// if a font has been set since this dialog was launched, draw things with it
	const UT_UCSChar*  entryString = getDrawString();

	if (!entryString) {
		return;
	}

	event_previewInvalidate(entryString);
}

void XAP_CocoaDialog_FontChooser::_okAction(void)
{
	m_answer = XAP_Dialog_FontChooser::a_OK;
	[NSApp stopModal];
}


void XAP_CocoaDialog_FontChooser::_cancelAction(void)
{
	m_answer = XAP_Dialog_FontChooser::a_CANCEL;
	[NSApp stopModal];
}

/* callbacks to create / release the GR_Graphics when the window is loaded / released */
void	XAP_CocoaDialog_FontChooser::_createGC(XAP_CocoaNSView* owner)
{
	NSSize  size;
	GR_CocoaAllocInfo ai(owner);
	m_pGraphics = static_cast<GR_CocoaGraphics*>(XAP_App::getApp()->newGraphics(ai));

	size = [owner bounds].size;
	_createFontPreviewFromGC(m_pGraphics, lrintf(size.width), lrintf(size.height));
	owner.drawable = m_pFontPreview;
}

void XAP_CocoaDialog_FontChooser::_deleteGC(void)
{
	DELETEP(m_pGraphics);
	m_pGraphics = NULL;
}

@implementation XAP_CocoaDialog_FontChooserController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"xap_CocoaDlg_FontChooser"];
}

-(void)discardXAP
{
	if (_xap) {
		_xap->_deleteGC();
		_xap = NULL;
	}
}

-(void)dealloc
{
	[m_sizeDataSource release];
	[m_stylesDataSource release];
	[m_fontDataSource release];
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_FontChooser*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = _xap->getApp()->getStringSet();

		LocalizeControl([self window],				pSS, XAP_STRING_ID_DLG_UFS_FontTitle);

		LocalizeControl(_okBtn,						pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn,					pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_fontLabel,					pSS, XAP_STRING_ID_DLG_UFS_FontLabel);
		LocalizeControl(_styleLabel,				pSS, XAP_STRING_ID_DLG_UFS_StyleLabel);
		LocalizeControl(_sizeLabel,					pSS, XAP_STRING_ID_DLG_UFS_SizeLabel);
		LocalizeControl(_effectLabel,				pSS, XAP_STRING_ID_DLG_UFS_EffectsFrameLabel);
		LocalizeControl(_strikeButton,				pSS, XAP_STRING_ID_DLG_UFS_StrikeoutCheck);
		LocalizeControl(_underlineButton,			pSS, XAP_STRING_ID_DLG_UFS_UnderlineCheck);
		LocalizeControl(_overlineButton,			pSS, XAP_STRING_ID_DLG_UFS_OverlineCheck);
		LocalizeControl(_strikeButton,				pSS, XAP_STRING_ID_DLG_UFS_StrikeoutCheck);
	//	LocalizeControl(_strikeButton,				pSS, XAP_STRING_ID_DLG_UFS_EncodingLabel);
		LocalizeControl(_superscriptButton,			pSS, XAP_STRING_ID_DLG_UFS_SuperScript);
		LocalizeControl(_subscriptButton,			pSS, XAP_STRING_ID_DLG_UFS_SubScript);
		LocalizeControl(_textColorLabel,			pSS, XAP_STRING_ID_DLG_UFS_ColorTab);
		LocalizeControl(_textHighlightColorLabel,	pSS, XAP_STRING_ID_DLG_UFS_BGColorTab);
		LocalizeControl(_noHighlightColorButton,	pSS, XAP_STRING_ID_DLG_UFS_TransparencyCheck);
		
		m_fontDataSource = [[XAP_StringListDataSource alloc] init];
		[m_fontDataSource loadFontList];
		[_fontList setDataSource:m_fontDataSource];
		[_fontList setDelegate:self];
	
		m_stylesDataSource = [[XAP_StringListDataSource alloc] init];
		std::string label;
		pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleRegular, label);
		[m_stylesDataSource addCString:label.c_str()];
		pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleItalic, label);
		[m_stylesDataSource addCString:label.c_str()];
		pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleBold, label);
		[m_stylesDataSource addCString:label.c_str()];
		pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleBoldItalic, label);
		[m_stylesDataSource addCString:label.c_str()];
		[_styleList setDataSource:m_stylesDataSource];
		[_styleList setDelegate:self];
	
		m_sizeDataSource = [[XAP_StringListDataSource alloc] init];
		{
			int sz = XAP_EncodingManager::fontsizes_mapping.size();
			for (int i = 0; i < sz; ++i)
			{
				char* str = (char*)XAP_EncodingManager::fontsizes_mapping.nth2(i);
				if (str) {
					[m_sizeDataSource addString:[NSString stringWithUTF8String:str]];
				}
				else {
					NSLog(@"attempting to add NULL to string data source (%s:%d)", __FILE__, __LINE__);
				}
			}
		}
		[_sizeList setDataSource:m_sizeDataSource];
		[_sizeList setDelegate:self];
	}
	else {
		NSLog(@"Loaded Windows without XAP (%s:%d)", __FILE__, __LINE__);
	}

	if ([[NSColorPanel sharedColorPanel] isVisible])
		{
			[_textColorWell activate:YES];
		}
}

-(IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	[[NSColorPanel sharedColorPanel] orderOut:self];

	static_cast<XAP_CocoaDialog_FontChooser *>(_xap)->_okAction();
}

-(IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	[[NSColorPanel sharedColorPanel] orderOut:self];

	static_cast<XAP_CocoaDialog_FontChooser *>(_xap)->_cancelAction();
}

-(IBAction)colorWellAction:(id)sender
{
	if (sender == _textColorWell) {
		_xap->fgColorChanged();
	}
	else if (sender == _textHighlightColorWell) {
		_xap->bgColorChanged();
	}
}

-(IBAction)underlineAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->underlineChanged
				([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

-(IBAction)overlineAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->overlineChanged
			([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

-(IBAction)strikeoutAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->strikeoutChanged
			([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

-(IBAction)superscriptAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->superscriptChanged
			([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

-(IBAction)subscriptAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->subscriptChanged
			([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

-(IBAction)transparentAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->transparencyChanged
			([(NSControl*)sender intValue] == NSControlStateValueOff ? 0 : 1);
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	id obj = [aNotification object];
	if (obj == _fontList) {
		_xap->fontRowChanged();
	} 
	else if (obj == _styleList) {
		int idx;
		idx = [_styleList selectedRow];
		_xap->styleRowChanged(idx);
	}
	else if (obj == _sizeList) {
		_xap->sizeRowChanged();	
	}
}

- (XAP_CocoaNSView*)preview
{
	return _preview;
}

-(void)setStrikeout:(bool)value
{
	[_strikeButton setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
}

-(void)setUnderline:(bool)value
{
	[_underlineButton setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
}

-(void)setOverline:(bool)value
{
	[_overlineButton setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
}

-(void)setSuperscript:(bool)value
{
	[_superscriptButton setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
}

-(void)setSubscript:(bool)value
{
	[_subscriptButton setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
}

-(void)selectFont:(char*)value
{
	if (value) {
		int idx = [(XAP_StringListDataSource*)[_fontList dataSource] rowWithCString:value];
		if (idx >= 0) {
			[_fontList selectRowIndexes:[NSIndexSet indexSetWithIndex:idx] byExtendingSelection:NO];
			[_fontList scrollRowToVisible:idx];
		}
	}
	else {
		UT_DEBUGMSG(("selectFont: called with NULL value\n"));
		[_fontList setAllowsEmptySelection:YES];
		[_fontList deselectAll:nil];
		return;
	}

}


-(NSString*)selectedFont
{
	int idx = [_fontList selectedRow];
	return [[(XAP_StringListDataSource*)[_fontList dataSource] array] objectAtIndex:idx];
}


-(void)selectSize:(char*)value
{
	int idx;
	char sizeString[60];

	if (value == NULL) {
		UT_DEBUGMSG(("selectSize: called with NULL value\n"));
		[_sizeList setAllowsEmptySelection:YES];
		[_sizeList deselectAll:nil];
		return;
	}
	snprintf(sizeString, 60, "%s", std_size_string(UT_convertToPoints(value)));
	idx = [(XAP_StringListDataSource*)[_sizeList dataSource] rowWithCString:(char *)XAP_EncodingManager::fontsizes_mapping.lookupBySource(sizeString)];
	if (idx >= 0) {
		[_sizeList selectRowIndexes:[NSIndexSet indexSetWithIndex:idx] byExtendingSelection:NO];
		[_sizeList scrollRowToVisible:idx];
	}
}


-(NSString*)selectedSize
{
	int idx = [_sizeList selectedRow];
	return [[(XAP_StringListDataSource*)[_sizeList dataSource] array] objectAtIndex:idx];
}


-(void)selectStyle:(char*)style withWeight:(char*)weight
{
	// this is pretty messy
	int st = 0;
	if (!style || !weight) {
		st = -1;
	}
	else if (!g_ascii_strcasecmp(style, "normal") &&
					!g_ascii_strcasecmp(weight, "normal")) {
		st = 0;
	}
	else if (!g_ascii_strcasecmp(style, "italic") &&
					!g_ascii_strcasecmp(weight, "normal")) {
		st = 1;
	}
	else if (!g_ascii_strcasecmp(style, "normal") &&
					!g_ascii_strcasecmp(weight, "bold")){
		st = 2;
	}
	else if (!g_ascii_strcasecmp(style, "italic") &&
					!g_ascii_strcasecmp(weight, "bold")) {
		st = 3;
	}
	else {
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	if (st != -1) {
		[_styleList selectRowIndexes:[NSIndexSet indexSetWithIndex:st] byExtendingSelection:NO];
		[_styleList scrollRowToVisible:st];
	}
}


-(NSColor*)textColor
{
	return [_textColorWell color];
}

-(void)setTextColor:(NSColor*)color
{
	[_textColorWell setColor:color];
}

-(NSColor*)bgColor
{
	return [_textHighlightColorWell color];
}

-(void)setBgColor:(NSColor*)color
{
	[_textHighlightColorWell setColor:color];
}

@end

