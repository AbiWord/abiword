/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_FONTCHOOSER_H
#define XAP_COCOADIALOG_FONTCHOOSER_H

#import <Cocoa/Cocoa.h>
#include "xap_App.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_misc.h"

#import "xap_CocoaDialog_Utilities.h"
#import "xap_Cocoa_NSTableUtils.h"
#import "xap_CocoaFrame.h"

class GR_CocoaGraphics;
class XAP_CocoaDialog_FontChooser;

@interface XAP_CocoaDialog_FontChooserController 
    : NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate>
{
	IBOutlet NSTableView *		_fontList;
	IBOutlet NSTableView *		_styleList;
	IBOutlet NSTableView *		_sizeList;

	IBOutlet NSTextField *		_fontLabel;
	IBOutlet NSTextField *		_styleLabel;
	IBOutlet NSTextField *		_sizeLabel;
	IBOutlet NSBox *			_effectLabel;
	IBOutlet NSBox *			_textColorLabel;
	IBOutlet NSBox *			_textHighlightColorLabel;

	IBOutlet NSButton *			_okBtn;
	IBOutlet NSButton *			_cancelBtn;
	IBOutlet NSButton *			_strikeButton;
	IBOutlet NSButton *			_underlineButton;
	IBOutlet NSButton *			_overlineButton;
	IBOutlet NSButton *			_hiddenButton;
	IBOutlet NSButton *			_superscriptButton;
	IBOutlet NSButton *			_subscriptButton;
	IBOutlet NSButton *			_noHighlightColorButton;

	IBOutlet NSColorWell *		_textColorWell;
	IBOutlet NSColorWell *		_textHighlightColorWell;

	IBOutlet XAP_CocoaNSView *	_preview;

	IBOutlet NSBox *			_previewBox;

	XAP_StringListDataSource *	m_fontDataSource;
	XAP_StringListDataSource *	m_sizeDataSource;
	XAP_StringListDataSource *	m_stylesDataSource;

	XAP_CocoaDialog_FontChooser *	_xap;
}

-(IBAction)okAction:(id)sender;
-(IBAction)cancelAction:(id)sender;
-(IBAction)colorWellAction:(id)sender;
-(IBAction)underlineAction:(id)sender;
-(IBAction)overlineAction:(id)sender;
-(IBAction)strikeoutAction:(id)sender;
-(IBAction)superscriptAction:(id)sender;
-(IBAction)subscriptAction:(id)sender;
-(IBAction)transparentAction:(id)sender;		// = no higlight color ?

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;

- (XAP_CocoaNSView*)preview;
/* accessors */
-(void)setStrikeout:(bool)value;
-(void)setUnderline:(bool)value;
-(void)setOverline:(bool)value;
-(void)setSuperscript:(bool)value;
-(void)setSubscript:(bool)value;

-(void)selectFont:(char*)value;
-(NSString*)selectedFont;
-(void)selectSize:(char*)value;
-(NSString*)selectedSize;
-(void)selectStyle:(char*)style withWeight:(char*)weight;

-(NSColor*)textColor;
-(void)setTextColor:(NSColor*)color;
-(NSColor*)bgColor;
-(void)setBgColor:(NSColor*)color;

@end

/*****************************************************************/

class XAP_CocoaDialog_FontChooser : public XAP_Dialog_FontChooser
{
public:
	XAP_CocoaDialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_FontChooser(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	void                    underlineChanged(bool);
	void                    overlineChanged(bool);
	void                    strikeoutChanged(bool);
	void                    superscriptChanged(bool);
	void                    subscriptChanged(bool);
	void                    transparencyChanged(bool);
	void 					updatePreview(void);
	void                    fontRowChanged(void);
	void                    styleRowChanged(int);
	void                    sizeRowChanged(void);
	void                    fgColorChanged(void);
	void                    bgColorChanged(void);

	// the state of what data is hidden and what is public is
	// pretty grave here.
	bool					getEntryString(const char ** string);

	/* GUI actions */
	void _okAction(void);
	void _cancelAction(void);

	/* GUI creation */
	void				_createGC(XAP_CocoaNSView* owner);
	void				_deleteGC(void);

private:
	bool		m_bSuperScriptInitialValue;
	bool		m_bSubScriptInitialValue;

	void _colorChanged(NSColor* color, const gchar* attr, char* buf);

	gchar *	m_currentFamily;
	XAP_CocoaDialog_FontChooserController*	m_dlg;
};


#endif /* XAP_COCOADIALOG_FONTCHOOSER_H */
