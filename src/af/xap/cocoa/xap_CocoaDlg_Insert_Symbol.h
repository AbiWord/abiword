/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2003 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_INSERT_SYMBOL_H
#define XAP_COCOADIALOG_INSERT_SYMBOL_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_Insert_Symbol.h"

#import "xap_CocoaDialog_Utilities.h"

#define DEFAULT_COCOA_SYMBOL_FONT "Symbol"

class XAP_CocoaFrame;
class XAP_CocoaDialog_Insert_Symbol;

/*****************************************************************/
@interface XAP_CocoaDlg_Insert_SymbolController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_addBtn;
    IBOutlet NSButton *_closeBtn;
    IBOutlet NSComboBox *_fontCombo;
    IBOutlet XAP_CocoaNSView *_grid;
    IBOutlet XAP_CocoaNSView *_preview;
	XAP_CocoaDialog_Insert_Symbol* _xap;
}
- (IBAction)addAction:(id)sender;
- (IBAction)closeAction:(id)sender;
- (IBAction)fontSelectAction:(id)sender;

- (XAP_CocoaNSView*)grid;
- (XAP_CocoaNSView*)preview;

- (void)_selectFontByName:(const char*)name;
- (NSString*)_selectedFont;
@end



class XAP_CocoaDialog_Insert_Symbol : public XAP_Dialog_Insert_Symbol
{
public:
	XAP_CocoaDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_Insert_Symbol(void);

	virtual void			runModal(XAP_Frame * pFrame);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame *pFrame){};
	virtual void			destroy(void);
	virtual void			activate(void);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	void 					event_CloseWindow(void);
	void					SymbolMap_exposed( void);
	void					Symbolarea_exposed( void);
	void					SymbolMap_clicked(NSEvent* event);
	void					CurrentSymbol_clicked(void);
	virtual void			Key_Pressed(NSEvent * e);
	virtual void			New_Font( void);

private:

	void		_fillComboboxWithFonts (NSComboBox* combo);

	GR_CocoaGraphics	* 		m_pGRPreview;
	GR_CocoaGraphics *       	m_pGRGrid;
	XAP_CocoaDlg_Insert_SymbolController*	m_dlg;
};

#endif /* XAP_COCOADIALOG_INSERT_SYMBOL_H */
