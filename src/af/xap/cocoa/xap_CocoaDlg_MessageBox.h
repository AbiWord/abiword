/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_MESSAGEBOX_H
#define XAP_COCOADIALOG_MESSAGEBOX_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaFrame.h"
#include "xap_Dlg_MessageBox.h"


class XAP_CocoaDialog_MessageBox;

@interface XAP_CocoaDlg_MessageBoxController : NSWindowController
{
	IBOutlet NSButton * m_okBtn;
	IBOutlet NSButton * m_cancelBtn;
	IBOutlet NSButton * m_yesBtn;
	IBOutlet NSButton * m_noBtn;
	IBOutlet NSTextField * m_messageField;
	XAP_Dialog_MessageBox::tButtons	m_buttons;
	XAP_CocoaDialog_MessageBox	* m_xap;
}
+ (XAP_CocoaDlg_MessageBoxController *)loadFromNibWithButtons:(XAP_Dialog_MessageBox::tButtons)buttons;
- (void)windowDidLoad;
- (void)setButtons:(XAP_Dialog_MessageBox::tButtons)buttons;

- (void)setXAPOwner:(XAP_CocoaDialog_MessageBox *)owner;
- (void)setOkBtnLabel:(NSString *)label;
- (void)setCancelBtnLabel:(NSString *)label;
- (void)setYesBtnLabel:(NSString *)label;
- (void)setNoBtnLabel:(NSString *)label;
- (void)setMessage:(NSString *)message;
- (IBAction)okAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)yesAction:(id)sender;
- (IBAction)noAction:(id)sender;
@end


/*****************************************************************/

class XAP_CocoaDialog_MessageBox : public XAP_Dialog_MessageBox
{
public:
	XAP_CocoaDialog_MessageBox(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_MessageBox(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// must let static callbacks read our bindings
	void 				_setAnswer(XAP_Dialog_MessageBox::tAnswer answer);
	
	XAP_Frame *		_getFrame () { return m_pCocoaFrame; };	//accessor for Obj-C
private:
	XAP_Frame *		m_pCocoaFrame;
	XAP_CocoaDlg_MessageBoxController *m_dlg;	
};

#endif /* XAP_COCOADIALOG_MESSAGEBOX_H */
