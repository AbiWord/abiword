/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_FontChooser.h"
#include "xap_CocoaApp.h"
#include "xap_EncodingManager.h"
#include "gr_CocoaGraphics.h"

// your typographer's standard nonsense latin font phrase
#define PREVIEW_ENTRY_DEFAULT_STRING	"Lorem ipsum dolor sit amet, consectetaur adipisicing..."

//
// For Screen color picker
	enum
	{
		RED,
		GREEN,
		BLUE,
		OPACITY
	};

/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FontChooser * p = new XAP_CocoaDialog_FontChooser(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FontChooser::XAP_CocoaDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_FontChooser(pDlgFactory,dlgid),
		m_currentFamily(NULL),
		m_dlg(nil)
{
}

XAP_CocoaDialog_FontChooser::~XAP_CocoaDialog_FontChooser(void)
{
	FREEP(m_currentFamily);
	DELETEP(m_pGraphics);
}


#if 0
/*****************************************************************/

static gint s_drawing_area_expose(GtkWidget * w,
								  GdkEventExpose * /* pExposeEvent */)
{
	XAP_CocoaDialog_FontChooser * dlg = (XAP_CocoaDialog_FontChooser *)
		                              g_object_get_user_data(G_OBJECT(w));

//
// Look if updates are blocked and quit if they are.
//
	if(dlg->m_blockUpdate)
		return FALSE;
	dlg->updatePreview();
	return FALSE;
}

#endif

/*****************************************************************/

void XAP_CocoaDialog_FontChooser::underlineChanged(bool value)
{
	m_bUnderline = value;
	m_bChangedUnderline = !m_bChangedUnderline;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}


void XAP_CocoaDialog_FontChooser::strikeoutChanged(bool value)
{
	m_bStrikeout = value;
	m_bChangedStrikeOut = !m_bChangedStrikeOut;
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


void XAP_CocoaDialog_FontChooser::transparencyChanged(bool value)
{
	bool bTrans = value;
	if(bTrans)
	{
		addOrReplaceVecProp("bgcolor","transparent");
		UT_DEBUGMSG (("Update background color"));
//		m_currentBGColor[RED]  = -1;
//		m_currentBGColor[GREEN] = -1;
//		m_currentBGColor[BLUE] = -1;
	}
	updatePreview();
}

void XAP_CocoaDialog_FontChooser::fontRowChanged(void)
{
	NSString *fontFamily;
	
	fontFamily = [m_dlg selectedFont];
	NSLog (@"font family is %@", fontFamily);
	FREEP(m_currentFamily);
	m_currentFamily = UT_strdup([fontFamily UTF8String]);
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
				   (XML_Char *)XAP_EncodingManager::fontsizes_mapping.lookupByTarget([fontSize UTF8String]));
	addOrReplaceVecProp("font-size",(XML_Char *)szFontSize);

	updatePreview();
}


void XAP_CocoaDialog_FontChooser::fgColorChanged(void)
{
	NSColor* color;
	static char buf_color[8];
	float r,g,b,a;
	color = [m_dlg bgColor];
	[color getRed:&r green:&g blue:&b alpha:&a];
	snprintf(buf_color, sizeof(buf_color), "%02x%02x%02x",
			(unsigned int) (r 	* (float) 255.0),
			(unsigned int) (g	* (float) 255.0),
			(unsigned int) (b * (float) 255.0));
	addOrReplaceVecProp("color",(XML_Char *)buf_color);
	updatePreview();
}


void XAP_CocoaDialog_FontChooser::bgColorChanged(void)
{
	NSColor* color;
	static char buf_color[8];
	float r,g,b,a;
	color = [m_dlg bgColor];
	[color getRed:&r green:&g blue:&b alpha:&a];
	snprintf(buf_color, sizeof(buf_color), "%02x%02x%02x",
			(unsigned int) (r 	* (float) 255.0),
			(unsigned int) (g	* (float) 255.0),
			(unsigned int) (b * (float) 255.0));
	addOrReplaceVecProp("bgcolor",(XML_Char *)buf_color);
	updatePreview();
}



void XAP_CocoaDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(m_pApp);

	m_dlg = [[XAP_CocoaDialog_FontChooserController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];
	
	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	// freeze updates of the preview


	// Set the defaults in the list boxes according to dialog data
	[m_dlg selectFont:(char*)getVal("font-family")];
	[m_dlg selectStyle:(char*)getVal("font-style") withWeight:(char*)getVal("font-weight")];
	[m_dlg selectSize:(char*)getVal("font-size")];

	// Set color in the color selector
	if (getVal("color"))
	{
		UT_RGBColor c;
		UT_parseColor(getVal("color"), c);

		NSColor *color = GR_CocoaGraphics::_utRGBColorToNSColor(c);
		[m_dlg setTextColor:color];
	}
	else {
		[m_dlg setTextColor:[NSColor blackColor]];	
	}
	
	// Set color in the color selector
	const XML_Char * pszBGCol = getVal("bgcolor");
	if (pszBGCol && strcmp(pszBGCol,"transparent") != 0)
	{
		UT_RGBColor c;
		UT_parseColor(getVal("bgcolor"), c);

		NSColor *color = GR_CocoaGraphics::_utRGBColorToNSColor(c);
		[m_dlg setBgColor:color];
	}
	else
	{
		[m_dlg setBgColor:[NSColor whiteColor]];	
	}

	// set the strikeout and underline check buttons
	[m_dlg setStrikeout:m_bStrikeout];
	[m_dlg setUnderline:m_bUnderline];
	[m_dlg setOverline:m_bOverline];
	
	updatePreview();
	// Run the dialog
	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;

	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((getVal("font-family")) ? getVal("font-family") : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((getVal("font-size")) ? getVal("font-size") : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((getVal("font-weight")) ? getVal("font-weight") : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((getVal("font-style")) ? getVal("font-style") : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((getVal("color")) ? getVal("color") : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeout),							((m_bChangedStrikeOut) ? "(chg)" : "")));

}

bool XAP_CocoaDialog_FontChooser::getEntryString(char ** string)
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
	if (!m_pGraphics)
		return;
	// if a font has been set since this dialog was launched, draw things with it
//	if (m_doneFirstFont)
//	{
		char * entryString;

		if (!getEntryString(&entryString))
			return;

		UT_UCSChar * unicodeString = NULL;
		UT_UCS4_cloneString_char(&unicodeString, entryString);
		event_previewExposed(unicodeString);
//	}
//	else
//	{
//		event_previewClear();
//	}
}

void XAP_CocoaDialog_FontChooser::_okAction(void)
{
	m_answer = XAP_Dialog_FontChooser::a_CANCEL;
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
	m_pGraphics = new GR_CocoaGraphics(owner, m_pApp);
	size = [owner bounds].size;
	_createFontPreviewFromGC(m_pGraphics, lrintf(size.width), lrintf(size.height));
}

void XAP_CocoaDialog_FontChooser::_deleteGC(void)
{
	DELETEP(m_pGraphics);
	m_pGraphics = NULL;
}

@implementation XAP_CocoaDialog_FontChooserController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_FontChooser"];
	return self;
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
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
	[m_sizeDataSource release];
	[m_stylesDataSource release];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_FontChooser*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		_xap->_createGC(_preview);
		_xap->event_previewClear();
		/* localize */
		const XAP_StringSet * pSS = _xap->getApp()->getStringSet();
		[[self window] setTitle:[NSString stringWithUTF8String:pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_FontTitle).c_str()]];
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_fontLabel, pSS, XAP_STRING_ID_DLG_UFS_FontLabel);
		LocalizeControl(_styleLabel, pSS, XAP_STRING_ID_DLG_UFS_StyleLabel);
		LocalizeControl(_sizeLabel, pSS, XAP_STRING_ID_DLG_UFS_SizeLabel);
		LocalizeControl(_effectLabel, pSS, XAP_STRING_ID_DLG_UFS_EffectsFrameLabel);
		LocalizeControl(_strikeButton, pSS, XAP_STRING_ID_DLG_UFS_StrikeoutCheck);
		LocalizeControl(_underlineButton, pSS, XAP_STRING_ID_DLG_UFS_UnderlineCheck);
		LocalizeControl(_overlineButton, pSS, XAP_STRING_ID_DLG_UFS_OverlineCheck);
		LocalizeControl(_strikeButton, pSS, XAP_STRING_ID_DLG_UFS_StrikeoutCheck);
	//	LocalizeControl(_strikeButton, pSS, XAP_STRING_ID_DLG_UFS_EncodingLabel);
		LocalizeControl(_textColorLabel, pSS, XAP_STRING_ID_DLG_UFS_ColorTab);
		LocalizeControl(_textHighlightColorLabel, pSS, XAP_STRING_ID_DLG_UFS_BGColorTab);
		LocalizeControl(_noHighlightColorButton, pSS, XAP_STRING_ID_DLG_UFS_TransparencyCheck);
		
		m_fontDataSource = [[XAP_StringListDataSource alloc] init];
		[m_fontDataSource loadFontList];
		[_fontList setDataSource:m_fontDataSource];
		[_fontList setDelegate:self];
	
		m_stylesDataSource = [[XAP_StringListDataSource alloc] init];
		[m_stylesDataSource addUT_String:pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleRegular)];
		[m_stylesDataSource addUT_String:pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleItalic)];
		[m_stylesDataSource addUT_String:pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleBold)];
		[m_stylesDataSource addUT_String:pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleBoldItalic)];
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
		
		[[NSNotificationCenter defaultCenter] addObserver:self 
						selector:@selector(colorWellDidChange:) 
						name:nil
						object:_textColorWell];
		[[NSNotificationCenter defaultCenter] addObserver:self 
						selector:@selector(colorWellDidChange:) 
						name:nil
						object:_textHighlightColorWell];
	}
	else {
		NSLog(@"Loaded Windows without XAP (%s:%d)", __FILE__, __LINE__);
	}
}

-(IBAction)okAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->_okAction();
}


-(IBAction)cancelAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->_cancelAction();
}


-(IBAction)colorWellAction:(id)sender
{
	NSColorPanel*	panel = [NSColorPanel sharedColorPanel];
	[panel setTarget:sender];
	[panel setColor:[sender color]];
}

-(IBAction)underlineAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->underlineChanged
				([(NSControl*)sender intValue] == NSOffState ? 0 : 1);
}


-(IBAction)overlineAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->overlineChanged
			([(NSControl*)sender intValue] == NSOffState ? 0 : 1);
}


-(IBAction)strikeoutAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->strikeoutChanged
			([(NSControl*)sender intValue] == NSOffState ? 0 : 1);
}


-(IBAction)transparentAction:(id)sender
{
	static_cast<XAP_CocoaDialog_FontChooser*>(_xap)->strikeoutChanged
			([(NSControl*)sender intValue] == NSOffState ? 0 : 1);
}

- (void)colorPanelDidChange:(NSNotification *)aNotification
{
	id obj = [aNotification object];

}


- (void)colorWellDidChange:(NSNotification *)aNotification
{
	id obj = [aNotification object];
	UT_DEBUGMSG(("received notification\n"));
	if (obj == _textColorWell) {
		_xap->fgColorChanged();
	}
	else if (obj == _textHighlightColorWell) {
		_xap->bgColorChanged();
	}
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

-(void)setStrikeout:(bool)value
{
	[_strikeButton setIntValue:value];
}


-(void)setUnderline:(bool)value
{
	[_underlineButton setIntValue:value];
}


-(void)setOverline:(bool)value
{
	[_overlineButton setIntValue:value];
}


-(void)selectFont:(char*)value
{
	if (value) {
		int idx = [(XAP_StringListDataSource*)[_fontList dataSource] rowWithCString:value];
		if (idx >= 0) {
			[_fontList selectRow:idx byExtendingSelection:NO];
		}
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
	
	snprintf(sizeString, 60, "%s", std_size_string(UT_convertToPoints(value)));
	idx = [(XAP_StringListDataSource*)[_sizeList dataSource] rowWithCString:(char *)XAP_EncodingManager::fontsizes_mapping.lookupBySource(sizeString)];
	if (idx >= 0) {
		[_sizeList selectRow:idx byExtendingSelection:NO];
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
	else if (!UT_stricmp(style, "normal") &&
					!UT_stricmp(weight, "normal")) {
		st = 0;
	}
	else if (!UT_stricmp(style, "italic") &&
					!UT_stricmp(weight, "normal")) {
		st = 1;
	}
	else if (!UT_stricmp(style, "normal") &&
					!UT_stricmp(weight, "bold")){
		st = 2;
	}
	else if (!UT_stricmp(style, "italic") &&
					!UT_stricmp(weight, "bold")) {
		st = 3;
	}
	else {
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	if (st != -1) {
		[_styleList selectRow:st byExtendingSelection:NO];
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

